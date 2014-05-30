#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "HTA.h"
#include "Accessor.h"
#include "Region.h"
#include "RefCount.h"
#include "CSR.h"

/* ================================================
 *
 * Static variables 
 *
 * ===============================================*/
const static size_t _hta_scalar_size[HTA_SCALAR_TYPE_MAX] = {
    sizeof(int32_t),
    sizeof(int64_t),
    sizeof(uint32_t),
    sizeof(uint64_t),
    sizeof(float),
    sizeof(double),
    sizeof(dcomplex)
};

/* ================================================
 *
 * Static functions
 *
 * ===============================================*/

static void _create_dense_leaf(Leaf* leaf, int num_elem, void *block_ptr)
{
    leaf->num_elem = num_elem;
    leaf->raw = block_ptr;
}

static void _create_sparse_leaf(Leaf* leaf)
{
    leaf->num_elem = 0;
    leaf->raw = NULL;
}

static void _free_leaf_tile(HTA *h)
{
    ASSERT(h->height == 1);
    switch(h->type) {
        case HTA_TYPE_DENSE:
            Alloc_free_block(h->leaf.raw);
            h->leaf.raw = NULL;
        break;
        case HTA_TYPE_SPARSE:
            if(h->leaf.raw) { // not empty tile
                Alloc_free_block(h->leaf.raw);
                h->leaf.raw = NULL;
            }
        break;
        default:
            ASSERT(0 && "unimplemented");
    }
}

// Recursively construct HTA metadata tree structure
// given the specification. This is for homogeneous
// HTA only
// @param dim Dimension
// @param levels The number of levels to go
// @param tiling The dimension tuples of each level
// @param mmap The mapping information of leaf tiles to data block GUIDs
// @param scalar_type The primitive data type of scalars
// @param idx The 1D linear index of a tile at its level. It's used for mapping 
//            to data block GUIDs.
static HTA* _recursive_create(HTA_TYPE type, int dim, int levels, 
        const Tuple *element_offset, const Tuple *flat_size, 
        Tuple* tiling, const Mapping * mmap, 
        HTA_SCALAR_TYPE scalar_type, int* idx, Tuple* nd_size_at_level) 
{
    if(levels != 1)
        ASSERT(levels == tiling->height+1 
                && "Levels specified should be equal to the height of tiling");

    // Allocate memory space for metadata
    HTA *h =  Alloc_acquire_block(0, sizeof(struct hta));

    h->type = type;
    h->dim = dim;
    h->height = levels;
    h->scalar_type = scalar_type;
    int cur_idx = idx[levels-1]++;
    h->rank = cur_idx;
    h->nd_rank.dim = dim;
    h->nd_tile_dimensions = nd_size_at_level[levels-1]; // it's actually size at "height"
    Tuple_1d_to_nd_index(cur_idx, &h->nd_tile_dimensions, &h->nd_rank);
    h->nd_element_offset = *element_offset;
    h->is_selection= 0;
    
    // recursive tree walk to lower levels
    if(levels == 1) // Leaf level
    {
        h->flat_size = *flat_size;
        h->num_tiles = 1;
        h->tiles = NULL;
        h->tiling = NULL;
        switch(type) {
            case HTA_TYPE_DENSE: 
                _create_dense_leaf(&h->leaf, Tuple_product(flat_size), 
                        mmap->map_blocks[cur_idx]);
                h->home = Dist_get_home(&mmap->dist, cur_idx);
            break;
            case HTA_TYPE_SPARSE:
                _create_sparse_leaf(&h->leaf);
                h->home = -1;
            break;
            default:
                ASSERT(0 && "not supported");
        }
    }
    else // Non-leaf node
    {
        h->num_tiles = Tuple_product(tiling);
        h->leaf.num_elem = 0;
        h->leaf.raw = NULL;
        h->home = -1;
        // copy the whole tiling information into a newly allocated object
        h->tiling = Alloc_acquire_block(0, sizeof(Tuple) * tiling->height);
        Tuple_clone_array(h->tiling, tiling);
        h->tiles = (HTA**) Alloc_acquire_block(0, sizeof(HTA*) * h->num_tiles); 
        h->flat_size = *flat_size;

        Tuple nd_idx;
        Tuple_init_zero(&nd_idx, dim); 
        Tuple tile_size;
        Tuple_init_zero(&tile_size, dim); 
        Tuple ll_offset;
        for(int i=0; i < h->num_tiles; i++)
        {
            Tuple_init_zero(&ll_offset, dim);
            // 1D to ND index of the lower level tile
            Tuple_1d_to_nd_index(i, tiling, &nd_idx);
            // Compute flatten tile size of the lower level tile
            Tuple_get_lower_tile_size(&nd_idx, tiling, flat_size, &tile_size, &ll_offset);
            Tuple_add_dimensions(&ll_offset, &h->nd_element_offset);
            h->tiles[i] = _recursive_create(type, dim, levels-1, &ll_offset, &tile_size, tiling + 1, mmap, scalar_type, idx, nd_size_at_level);
        }
    }
    return h;
}

/// Recursively free the memory allocated for this HTA
/// @param h The pointer to the HTA to be destroyed
static void _recursive_destroy(HTA *h)
{
    ASSERT(h);
    if(h->height == 1) // Leaf level
    {
        if(!h->is_selection) {
            //Tuple_destroy_one(h->flat_size);
            _free_leaf_tile(h);
        //}
        }
    }
    else // Non-leaf node
    {
        if(!h->is_selection) {
            for(int i=0; i < h->num_tiles; i++) {
                _recursive_destroy(h->tiles[i]);
                h->tiles[i] = NULL;
            }
        }
        //if(RefCount_get_counter(h) == 1) { // Last one to release
            //Tuple_destroy_one(h->flat_size);
            Alloc_free_block(h->tiles);
            h->tiles = NULL;
            Alloc_free_block(h->tiling);
            h->tiling = NULL;
        //}
    }
    //RefCount_release(h);
    Alloc_free_block(h);
}

// Compute the tile index given a specific global index
static int _1d_global_to_tile_index(int size, int num_tiles, int global_index, int* local_index) 
{
    int i;

    i = (num_tiles * ((long long)global_index + 1) - 1) / size;
    *local_index = global_index - (i * (long long)size)/num_tiles; // overflow prevention

    return i;
}

/// @param flat_size Number of scalar elements
/// @param nd_num_tiles Number of tiles in each dimension
/// @param nd_global_index Flattened nd global index of scalar elements
/// @param tile_index The tile index where the nd_index is located
/// @param nd_local_index Flattened nd local index of scalar elements within the located tile
static void _nd_global_to_tile_index(const Tuple *flat_size, const Tuple *nd_num_tiles, const Tuple *nd_global_index, Tuple *tile_index, Tuple *nd_local_index)
{
    for(int dim = 0; dim < flat_size->dim; dim++)
        tile_index->values[dim] = _1d_global_to_tile_index(flat_size->values[dim], nd_num_tiles->values[dim], nd_global_index->values[dim], &(nd_local_index->values[dim]));
}

/// Compute the scalar offset of the tile
static int _1d_tile_to_elem_offset(int tile_index, int num_tiles, int size)
{
    int offset = (tile_index * (long long) size) / num_tiles; // overflow prevention
    return offset;
}

/// Compute the scalar offset of the tile for all N dimensions
static void _nd_tile_to_elem_offset(const Tuple *nd_tile_index, const Tuple *nd_num_tiles, const Tuple *flat_size, Tuple *nd_offset)
{
    for(int dim = 0; dim < flat_size->dim; dim++)
        nd_offset->values[dim] += _1d_tile_to_elem_offset(nd_tile_index->values[dim], nd_num_tiles->values[dim], flat_size->values[dim]);
}

// Compute the max possible leaf tile size
//static void _max_leaf_tile_size(HTA *h, Tuple *nd_size) 
//{
//    Tuple_overwrite_values(nd_size, &h->flat_size);
//    Tuple* cur_t = h->tiling;
//    int dim = h->dim;
//    int height = h->height - 1;
//
//    for(int j = 0; j < height; j++)
//    {
//        for(int i = 0; i < dim; i++)
//            nd_size->values[i] = (nd_size->values[i] + (cur_t->values[i] - 1)) / cur_t->values[i];
//        cur_t++;
//    }
//}

/* ================================================
 *
 * Global functions
 *
 * ===============================================*/

HTA* HTA_create(int dim, int levels, const Tuple *flat_size, int order, 
        Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...)
{
    va_list argp;
    Tuple tiling[num_tuples];
    Tuple *ts = NULL;
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");

    // merge tuples into one big chunk
    if(num_tuples != 0) {
        va_start(argp, num_tuples);
        for(int i = 0; i < num_tuples; i++){
            tiling[i] = va_arg(argp, Tuple);
            tiling[i].height = num_tuples - i;
            ASSERT(tiling[i].dim == dim && "All tuples have to have the same dimensions with dim");
        }
        va_end(argp);
        ts = tiling;
    } // else ts = NULL;

    DBG(5, "HTA_create(): dim = %d, levels = %d, scalar_type = %d\n", dim, levels, scalar_type); 
    HTA* ret = HTA_create_with_ts(dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

HTA* HTA_create_with_ts(int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts)
{
    ASSERT(flat_size && dist);
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");
    
    int idx[levels]; // to count tile index at each level 
    for(int i = 0; i < levels; i++)
        idx[i] = 0;

    Tuple nd_size_at_level[levels]; // the number of tiles at each level
    for(int i= 0; i < levels; i++)
        Tuple_init_zero(&nd_size_at_level[i], dim);
    for(int j = 0; j < dim; j++)
    {
        int v = 1;
        for(int i = 0; i < levels; i++) {
            if(i != 0)
                v *= ts[i-1].values[j];
            nd_size_at_level[levels-i-1].values[j] = v;
        }
    }

    // TODO: this can be parallel
    Mapping* mmap = Mapping_create(dim, levels, order, flat_size, ts, dist, _hta_scalar_size[scalar_type]); 
    Tuple nd_element_offset;
    Tuple_init_zero(&nd_element_offset, dim);
    HTA* ret = _recursive_create(HTA_TYPE_DENSE, dim, levels, &nd_element_offset, flat_size, ts, mmap, scalar_type, idx, nd_size_at_level);
    Mapping_destroy(mmap);

    return ret;
}


void HTA_destroy(HTA *h) {
    _recursive_destroy(h);
}

#if 0
HTA HTA_retain_one(HTA h)
{
    ASSERT(h);
    RefCount_retain(h);
    return h;
}

HTA HTA_retain_all(HTA h)
{
    ASSERT(h);
    ASSERT(h->height >= 1);
    if(h->height > 1) // Non-leaf node
    {
        int n = Tuple_count_elements(h->tiling, 1); 
        for(int i=0; i<n; i++) {
            HTA_retain_all(h->tiles[i]);
        }
    }
    RefCount_retain(h);
    return h;
}
#endif

size_t HTA_get_scalar_size(const HTA *h)
{
    ASSERT(h->scalar_type < HTA_SCALAR_TYPE_MAX && "Unsupported scalar type!");
    return _hta_scalar_size[h->scalar_type];
}

void HTA_print(const HTA* h, int recursively) {
    printf("HTA %p (%s)\n", h, (h->type == HTA_TYPE_DENSE)?"DENSE":"SPARSE");
    printf("dim: %d, ", h->dim);
    printf("height: %d, ", h->height);

    Tuple* tiling = h->tiling;
    HTA** tiles = h->tiles;

    if(h->height != 1)
        Tuple_print(tiling);
    else
        Tuple_print(&h->flat_size);

    printf("home: %d, ", h->home);
    printf("rank: %d, ", h->rank);
    printf("scalar_type: %d, ", h->scalar_type);
    //printf("ref_count: %d ", RefCount_get_counter(h));
    printf("\n");

    if(recursively && h->height > 1) {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_print(tiles[i], 1);
    }
}

/// Pick out the lower level tile specified by the accessor
/// @param h Current level HTA
/// @param accessor The accessor to select the lower level tile
/// @return The pointer to the lower level tile
HTA* HTA_pick_one_tile(const HTA* h, const Tuple *nd_idx) {
    ASSERT(h);
    ASSERT(nd_idx);
    int idx = Tuple_nd_to_1d_index(nd_idx, h->tiling);
    return h->tiles[idx];
}

static HTA* _clone_root_node(HTA *h) {
    HTA *clone =  Alloc_acquire_block(0, sizeof(struct hta));
    *clone = *h;
    if(h->height > 1)
    {
        clone->tiles = (HTA**) Alloc_acquire_block(0, sizeof(HTA*) * h->num_tiles); 
        clone->tiling = Alloc_acquire_block(0, sizeof(Tuple) * (h->height-1));
        memcpy(clone->tiles, h->tiles, sizeof(HTA*) * h->num_tiles);
        Tuple_clone_array(clone->tiling, h->tiling);
    }
    else
    {
        // no need to allocate anything here
    }
    clone->is_selection = 1;
    return clone;
}

/// This function selects tiles according to an array of accessors
/// and creates new HTA metadata when needed
/// @param h Pointer to the HTA
/// @param accessors An array of accessors
/// @param num_accessors The total number of accessors
/// @param cur Current accessor
//  Reference to HTAImpl.h: tileAt(Shape)
HTA* HTA_select(HTA* h, void** accessor, int togo) {
    ASSERT(h); 
    ASSERT(h->height > togo);
    ASSERT(togo > 0);
    //ASSERT(h->height > 1 && "This function sould not be applied on leaf tile");

    // if(togo == 0) {
    //    if(h->height > 1)
    //        Tuple_retain_all(h->tiling);
    //    return HTA_retain_all(h);
    //    return h;
    // }

    HTA* target;
    Region* r = (Region*) *accessor;
    Tuple* acc = (Tuple*) *accessor;
    unsigned int num_tiles;
    Tuple iter;
    Tuple nd_size;
    int watch_dim;

    switch(ACCESSOR_TYPE(*accessor)) {
        case (ACCESSOR_TUPLE): 
            target = HTA_pick_one_tile(h, acc); 
            if(togo == 1)
                return _clone_root_node(target);
            else
                return HTA_select(target, accessor + 1, togo - 1);
            break;
        case (ACCESSOR_REGION): 
            ASSERT(togo == 1 && "LIMITATION: region accessor must be the last one" );
            //target = (HTA) RefCount_alloc(sizeof(struct hta)); 
            target =  Alloc_acquire_block(0, sizeof(struct hta));
            target->dim = h->dim;
            target->height = h->height;

            num_tiles = Region_cardinality(r);
            //target->tiles = (HTA*) malloc(sizeof(HTA) * num_tiles); 
            target->tiles = (HTA**) Alloc_acquire_block(0, sizeof(HTA*) * num_tiles); 
            target->tiling = Alloc_acquire_block(0, sizeof(Tuple) * (h->height-1));
            //target->tiling = Region_get_dimension(r); // the function call *malloc* a new tuple
            *target->tiling = Region_get_dimension(r);
            target->tiling->height = h->height;
            // Use iterator to walk through selected tiles to correctly compute flat size of the new root
            nd_size = *target->tiling;
            nd_size.height = 1;
            Tuple_init_zero(&iter, target->dim);
            Tuple_init_zero(&target->flat_size, target->dim);
            watch_dim = target->dim - 1; // start from the least significant dimension
            for(int i = 0; i < num_tiles; i++) {
                int idx = Region_idx_to_tile_idx(r, h->tiling, i); // the index of the original HTA
                target->tiles[i] = h->tiles[idx];
                
                // update flat_size of the watched dimension
                int old_idx = iter.values[watch_dim];
                if(Tuple_iterator_next(&nd_size, &iter)) {
                    int new_idx = iter.values[watch_dim];
                    if(old_idx != new_idx)
                    {
                        target->flat_size.values[watch_dim] += target->tiles[i]->flat_size.values[watch_dim];
                        if(new_idx == 0)
                        {
                            watch_dim--;
                            target->flat_size.values[watch_dim] += target->tiles[i]->flat_size.values[watch_dim]; // add the flat size of the new watched dim
                        }
                    }
                }
                else // last selected tile
                {
                    target->flat_size.values[watch_dim] += target->tiles[i]->flat_size.values[watch_dim];
                }
                //HTA_retain_all(target->tiles[i]);
                //printf("refcount total alive = %d\n", RefCount_num_alive());
            }
            ASSERT(watch_dim == 0);
            if(h->height > 2) {// next level is not leaf
                Tuple_clone_array(target->tiling + 1, h->tiling + 1);
            }
            target->num_tiles = Tuple_product(target->tiling);
            target->home = -1;
            target->type = h->type;
            target->leaf.num_elem = 0;
            target->leaf.raw = NULL;
            target->rank = -1;
            target->scalar_type = h->scalar_type;
            target->is_selection = 1;
            return target;
            break;
        default:
           ASSERT(0 && "Unknown accessor type");
    }
    return NULL;
}

HTA* HTA_iterator_to_hta(HTA *h, Tuple *it) 
{
    ASSERT(h);
    HTA *ret = h;
    // walk through each level
    for(int i = 0; i < h->height-1; i++)
        ret = HTA_pick_one_tile(ret, &it[i]);
    return ret;
}

/// Return the pointer to the leaf where the nd_index element is located, 
/// and also output the local offset within the leaf.
HTA* HTA_locate_leaf_for_nd_index(HTA *h, const Tuple *nd_index, 
        int *leaf_offset)
{
    HTA* p = h;
    Tuple global_nd_index = *nd_index;
    Tuple next_tile_index; 
    Tuple_init_zero(&next_tile_index, h->dim);
    Tuple local_nd_index; 
    Tuple_init_zero(&local_nd_index, h->dim);

    while(p->height > 1) {        
        _nd_global_to_tile_index(&p->flat_size, p->tiling, &global_nd_index, &next_tile_index, &local_nd_index);

        // swap the two tuples for next iteration
        Tuple tmp = local_nd_index;
        local_nd_index = global_nd_index;
        global_nd_index = tmp;

        p = HTA_pick_one_tile(p, &next_tile_index);
    }

    ASSERT(p->height == 1);
    *leaf_offset = Tuple_nd_to_1d_index(&global_nd_index, &p->flat_size);
    return p;
}

/// Convert a leaf local nd index into a global nd index
void HTA_leaf_local_to_global_nd_index(HTA *h, Tuple *tiling_iter, 
        const Tuple *leaf_local_iter, Tuple *global_nd_index)
{
    Tuple_clear_value(global_nd_index);
    Tuple *cur_idx = tiling_iter;
    HTA* cur_h = h;

    while(cur_h->height > 1)
    {
        _nd_tile_to_elem_offset(cur_idx, cur_h->tiling, &cur_h->flat_size, global_nd_index);
        cur_h = HTA_pick_one_tile(cur_h, cur_idx);
        cur_idx += 1; // next level of iterator
    }
   
    Tuple_add_dimensions(global_nd_index, leaf_local_iter);
    return;
}


void HTA_init_with_array(HTA *h, void* array) {
    ASSERT(array);
    ASSERT(h);

    int dim = h->dim;
    size_t unit_size = HTA_get_scalar_size(h);

    Tuple global_nd_idx;
    Tuple_init_zero(&global_nd_idx, dim);
    do{
        int leaf_offset;
        int mat_offset = Tuple_nd_to_1d_index(&global_nd_idx, &h->flat_size);

        HTA* leaf = HTA_locate_leaf_for_nd_index(h, &global_nd_idx, &leaf_offset);
        void *ptr = HTA_get_ptr_raw_data(leaf);
        ASSERT(ptr);
        memcpy(((char*)ptr) + leaf_offset*unit_size, ((char*)array) + mat_offset*unit_size, unit_size);
    } while(Tuple_iterator_next(&h->flat_size, &global_nd_idx));
}

void HTA_to_array(HTA *h, void* array) {
    ASSERT(array);
    ASSERT(h);

    int dim = h->dim;
    size_t unit_size = HTA_get_scalar_size(h);

    Tuple global_nd_idx;
    Tuple_init_zero(&global_nd_idx, dim);
    int mat_offset = 0;
    int leaf_offset;

    do{
        mat_offset = Tuple_nd_to_1d_index(&global_nd_idx, &h->flat_size);
        HTA *leaf = HTA_locate_leaf_for_nd_index(h, &global_nd_idx, &leaf_offset);
        void *ptr = HTA_get_ptr_raw_data(leaf);
        memcpy(((char*)array) + mat_offset*unit_size, ((char*)ptr) + leaf_offset*unit_size, unit_size);
    } while(Tuple_iterator_next(&h->flat_size, &global_nd_idx));
}


/// Recursively collects pointers to HTA data structure in the same level
/// @param level The target level of HTA tiles in the tree
/// @param h The pointer to the current HTA tile
/// @param collect An array of pointers to tile HTAs
/// @param count A temporary counter to index the collect array
void HTA_collect_tiles(int level, HTA *h, HTA** collect, int *count)
{
    if(level == 0)
    {
        collect[*count] = h;
        (*count)++;
    }
    else
    {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_collect_tiles(level-1, h->tiles[i], collect, count);
    } 
}

void* HTA_access_element(HTA* h, Tuple** acc) {            
   int depth = h->height;                                       
   size_t unit_size = HTA_get_scalar_size(h);
   HTA* cur_h = h;                                               
   while(depth > 1)                                             
   {                                                            
       Tuple *t = acc[h->height - depth];                        
       ASSERT(t);                                               
       cur_h = HTA_pick_one_tile(cur_h, t);                     
       ASSERT(h);                                               
       depth--;                                          
   }                                                         
   Tuple* nd_idx = acc[h->height-1];
   int idx = Tuple_nd_to_1d_index(nd_idx, &cur_h->flat_size); 
   if(cur_h->type == HTA_TYPE_DENSE)
   {
       return (HTA_get_ptr_raw_data(cur_h) + idx*unit_size); 
   }
#if 0
   else if(cur_h->type == HTA_TYPE_SPARSE)
   {
       ASSERT(cur_h->leaf && "leaf tile is not initialized");
       HTA_SPARSE_LEAF leaf = (HTA_SPARSE_LEAF) cur_h->leaf;
       if(leaf->num_elem == 0) // empty tile, return immediately
           return NULL;

       int row = nd_idx->values[0];
       int col = nd_idx->values[1];
       void* val = Alloc_get_block_ptr(leaf->val_id);
       int* col_ind = (int*) Alloc_get_block_ptr(leaf->col_ind_id);
       int* row_ptr = (int*) Alloc_get_block_ptr(leaf->row_ptr_id);

       return CSR_get_ptr_to_value(row, col, val, col_ind, row_ptr, unit_size);
   }
#endif
   else
   {
       ASSERT(0 && "Unsupported HTA type");
       return NULL;
   }
   return NULL;
}

void* HTA_flat_access(HTA *h, const Tuple *global_nd_idx) {
    int offset;
    size_t sz = HTA_get_scalar_size(h);
    HTA *leaf = HTA_locate_leaf_for_nd_index(h, global_nd_idx, &offset);
    void* ptr = HTA_get_ptr_raw_data(leaf);
    return (ptr + offset * sz);
}

void HTA_init_all_scalars(HTA *h, void* initval) {
    ASSERT(h->type == HTA_TYPE_DENSE && "only works for dense HTA");
  
    if(h->height > 1) { // It's not a leaf tile
        Tuple iter[h->height-1];
        Tuple_iterator_begin(h->dim, h->height-1, iter);
        do{
            HTA* leaf = HTA_iterator_to_hta(h, iter);
            int num_elements = leaf->leaf.num_elem;
            int scalar_size = HTA_get_scalar_size(leaf);
            void* ptr = HTA_get_ptr_raw_data(leaf);
            for(int i = 0; i < num_elements; i++) {
                memcpy(ptr, initval, scalar_size);
                ptr += scalar_size;
            }
        }while(Tuple_iterator_next(h->tiling, iter));
    }
    else { // It's a leaf tile
	HTA* leaf = h;
        int num_elements = leaf->leaf.num_elem;
	int scalar_size = HTA_get_scalar_size(leaf);
	void* ptr = HTA_get_ptr_raw_data(leaf);
	for(int i = 0; i < num_elements; i++) {
	  memcpy(ptr, initval, scalar_size);
	  ptr += scalar_size;
	}
    }
}

void * HTA_get_ptr_raw_data(const HTA *h) {
    ASSERT(h->height == 1 && "Must be leaf tile");
    switch(h->type)
    {
        case HTA_TYPE_DENSE:
        case HTA_TYPE_SPARSE:
            return h->leaf.raw; 
        default:
            ASSERT(0 && "unimplemented");
    }
    return NULL;
}

void HTA_nd_global_to_tile_index(const Tuple *flat_size, const Tuple *nd_num_tiles, const Tuple *nd_global_index, Tuple *tile_index, Tuple *nd_local_index)
{
    _nd_global_to_tile_index(flat_size, nd_num_tiles, nd_global_index, tile_index, nd_local_index);
}

// ========================================================================
// Sparse HTAs
// ========================================================================
HTA* HTA_sparse_create(int dim, int levels, const Tuple *flat_size, int order, 
        Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...)
{
    va_list argp;
    Tuple tiling[num_tuples];
    Tuple *ts = NULL;
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");

    // merge tuples into one big chunk
    if(num_tuples != 0) {
        va_start(argp, num_tuples);
        for(int i = 0; i < num_tuples; i++){
            tiling[i] = va_arg(argp, Tuple);
            tiling[i].height = num_tuples - i;
            ASSERT(tiling[i].dim == dim && "All tuples have to have the same dimensions with dim");
        }
        va_end(argp);
        ts = tiling;
    } // else ts = NULL;

    DBG(5, "HTA_create(): dim = %d, levels = %d, scalar_type = %d\n", dim, levels, scalar_type); 
    HTA* ret = HTA_sparse_create_with_ts(dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

HTA* HTA_sparse_create_with_ts(int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts)
{
    ASSERT(flat_size && dist);
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");
    
    int idx[levels]; // to count tile index at each level 
    for(int i = 0; i < levels; i++)
        idx[i] = 0;

    Tuple nd_size_at_level[levels]; // the number of tiles at each level
    for(int i= 0; i < levels; i++)
        Tuple_init_zero(&nd_size_at_level[i], dim);
    for(int j = 0; j < dim; j++)
    {
        int v = 1;
        for(int i = 0; i < levels; i++) {
            if(i != 0)
                v *= ts[i-1].values[j];
            nd_size_at_level[levels-i-1].values[j] = v;
        }
    }
    Tuple nd_element_offset;
    Tuple_init_zero(&nd_element_offset, dim);
    HTA* ret = _recursive_create(HTA_TYPE_SPARSE, dim, levels, &nd_element_offset, flat_size, ts, NULL, scalar_type, idx, nd_size_at_level);
    return ret;
}

void HTA_init_sparse_leaf(HTA *h, int nnz, void* val, int* col_ind, int* row_ptr) {
    ASSERT(h->dim == 2 && "only 2D matrix is supported");
    ASSERT(h->height == 1 && "the input HTA has to be leaf tile"); 
    if(nnz == 0) // special case return immediately
        return;

    int row_dim = h->flat_size.values[0]; // the row dimension
    size_t sz = HTA_get_scalar_size(h);
    Leaf* leaf = &(h->leaf);

    // allocate space
    leaf->num_elem = nnz;
    leaf->raw = Alloc_acquire_block(h->home, nnz*sz + nnz*sizeof(int) + (row_dim+1)*sizeof(int));
    void * val_ptr = leaf->raw;
    int * col_ind_ptr = (int*)(val_ptr + nnz*sz);
    int * row_ptr_ptr = (int*)(val_ptr + nnz*sz + nnz*sizeof(int));

    memcpy(val_ptr, val, nnz*sz);
    memcpy(col_ind_ptr, col_ind, nnz*sizeof(int));
    memcpy(row_ptr_ptr, row_ptr, (row_dim+1)*sizeof(int)); 
}

void HTA_init_with_sparse_matrix(HTA *hs, int num_nz, int num_rows, void* val, int* col_ind, int* row_ptr)
{
    void *tile_val;
    int *tile_col_ind;
    int *tile_row_ptr;

    ASSERT(hs->dim == 2 && "only support 2D for now");
    Tuple iter[hs->height-1];
    Tuple_iterator_begin(hs->dim, hs->height-1, iter);
    //Tuple max_leaf_size;
    //Tuple_init_zero(&max_leaf_size, hs->dim);
    //_max_leaf_tile_size(hs, &max_leaf_size);
    //int max_size = Tuple_product(&max_leaf_size);
    int max_size = num_nz;
    int scalar_size = HTA_get_scalar_size(hs);

    // temporary storage
    tile_val = malloc(max_size * scalar_size);
    tile_col_ind = (int*) malloc(max_size * sizeof(int));
    tile_row_ptr = (int*) malloc((num_rows + 1) * sizeof(int));
    ASSERT(tile_val && tile_col_ind && tile_row_ptr);
    
    Tuple global_idx; //= Tuple_create_empty(hs->dim);    // global index in the matrix
    Tuple_init_zero(&global_idx, hs->dim);
    Tuple local_iter; //= Tuple_create_empty(hs->dim); // local index with leaf tile
    Tuple_init_zero(&local_iter, hs->dim);

    do {
        int nnz = 0;
        int prev_row = -1;

        HTA *leaf = HTA_iterator_to_hta(hs, iter);
        // acquire global index of each element in the tile
        do {
            // a new row starts
            if(local_iter.values[0] != prev_row) 
            {
                tile_row_ptr[local_iter.values[0]] = nnz;
                prev_row = local_iter.values[0];
            }

            HTA_leaf_local_to_global_nd_index(hs, iter, &local_iter, &global_idx); 
            int start = row_ptr[global_idx.values[0]];
            int end = row_ptr[global_idx.values[0]+1];
            for(int i = start; i < end; i++)
            {
                if(col_ind[i] == global_idx.values[1]) // found
                {
                    //tile_val[nnz] = val[i];
                    memcpy(tile_val + nnz * scalar_size, val + i * scalar_size, scalar_size);
                    tile_col_ind[nnz] = local_iter.values[1];
                    nnz++;
                    break;
                }
            }
        } while (Tuple_iterator_next(&leaf->flat_size, &local_iter));
        tile_row_ptr[leaf->flat_size.values[0]] = nnz;
        HTA_init_sparse_leaf(leaf, nnz, tile_val, tile_col_ind, tile_row_ptr);

        // reset
        Tuple_clear_value(&local_iter);
        Tuple_clear_value(&global_idx);
    } while(Tuple_iterator_next(hs->tiling, iter));

    // cleanup
    free(tile_val);
    free(tile_col_ind);
    free(tile_row_ptr);
}

void hta_init() {
    Alloc_init();
}

