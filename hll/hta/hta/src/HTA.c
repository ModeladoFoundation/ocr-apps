#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>

#include "pil_mem.h"
#include "pil_nw.h"

#include "HTA.h"
#include "Accessor.h"
#include "Region.h"
#include "RefCount.h"
#include "CSR.h"
#include "Comm.h"

#define DEF_SHARED_LEAVES (1)
// remove if not debugging
// #define DEVDEBUG

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


static int isPowerOfTwo(int n) {
    return ((n != 0) && !(n & (n-1)));
}

// This function assumes n is power of two
static int logint2(int n) {
    ASSERT(isPowerOfTwo(n));
    int x = 0;
    while(!(n & 0x1)) {
        x++;
        n >>= 1;
    }
    return x;
}

static void _create_dense_leaf(Leaf* leaf, int order, int num_elem, void *block_ptr)
{
    leaf->order = order;
    leaf->num_elem = num_elem;
    leaf->raw = block_ptr;
}

static void _create_sparse_leaf(Leaf* leaf)
{
    leaf->num_elem = 0;
    leaf->raw = NULL;
    leaf->order = ORDER_TILE;
}

static void _free_leaf_tile(HTA *h)
{
    ASSERT(h->height == 1);
    switch(h->type) {
        case HTA_TYPE_DENSE:
	// For distributed
            if((!HTA_is_distributed() && (h->pid == h->home)) || (HTA_is_distributed() && h->home == LOCAL)) {
                if(h->leaf.order != ORDER_ROW && h->leaf.order != ORDER_COL)
                    Alloc_free_block(h->leaf.raw);
                h->leaf.raw = NULL;
            }
        break;
        case HTA_TYPE_SPARSE:
            if((h->pid == h->home) && h->leaf.raw) { // not empty tile
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
static HTA* _recursive_create(int pid, HTA_TYPE type, int dim, int levels, int order,
        const Tuple *element_offset, const Tuple *flat_size,
        Tuple* tiling, const Mapping * mmap,
        HTA_SCALAR_TYPE scalar_type, int* idx, Tuple* nd_size_at_level, const Tuple* global_dimensions)
{
    if(levels != 1)
        ASSERT(levels == tiling->height+1
                && "Levels specified should be equal to the height of tiling");

    // Allocate memory space for metadata
    HTA *h =  Alloc_acquire_block(sizeof(struct hta));

    h->type = type;
    h->dim = dim;
    h->height = levels;
    h->scalar_type = scalar_type;
    h->pid = pid;
    int cur_idx = idx[levels-1]++;
    h->rank = cur_idx;
    h->nd_rank.dim = dim;
    h->nd_tile_dimensions = nd_size_at_level[levels-1]; // it's actually size at "height"
    h->global_dimensions = *global_dimensions;
    Tuple_1d_to_nd_index(cur_idx, &h->nd_tile_dimensions, &h->nd_rank);
    h->nd_element_offset = *element_offset;
    h->dist = mmap->dist;
    h->is_selection= 0;
#if ENABLE_PREPACKING
    h->prepacked = 0;
    h->num_gpps = 0;
    h->prepacked_data_array = NULL;
#endif

    // recursive tree walk to lower levels
    if(levels == 1) // Leaf level
    {
        h->flat_size = *flat_size;
        h->num_tiles = 1;
        h->tiles = NULL;
        switch(type) {
            case HTA_TYPE_DENSE:
                h->home = mmap->owners[cur_idx];
                _create_dense_leaf(&h->leaf, order, Tuple_product(flat_size),
                        mmap->map_blocks[cur_idx]);
            break;
            case HTA_TYPE_SPARSE:
                h->home = mmap->owners[cur_idx];
                _create_sparse_leaf(&h->leaf);
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
        h->tiling = *tiling;
        h->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * h->num_tiles);
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
            h->tiles[i] = _recursive_create(pid, type, dim, levels-1, order, &ll_offset, &tile_size, tiling + 1, mmap, scalar_type, idx, nd_size_at_level, global_dimensions);
        }
        // for SPMD, set the home of non-root tiles to be the same as their children
        // FIXME: a cleaner way to do this?
        h->home = h->tiles[0]->home;
    }
    return h;
}

static int _get_mapping(int idx, int num_tiles) {
    int np = HTA_get_num_processes();
    if( np > num_tiles ) // if np > num_tiles, use only num_tiles processes
        np = num_tiles;

    // Assume cyclic mapping here
    return (idx % np);
}

static HTA* _recursive_create_distributed(HTA_TYPE type, int dim, int levels, int order,
        const Tuple *element_offset, const Tuple *flat_size,
        Tuple* tiling, const Mapping * mmap,
        HTA_SCALAR_TYPE scalar_type, int* idx, Tuple* nd_size_at_level, int isTop, int owner, const Tuple *global_dimensions)
{
    if(levels != 1)
        ASSERT(levels == tiling->height+1
                && "Levels specified should be equal to the height of tiling");

    // Allocate memory space for metadata
    HTA *h = (HTA*) Alloc_acquire_block(sizeof(struct hta));

    h->type = type;
    h->dim = dim;
    h->height = levels;
    h->scalar_type = scalar_type;
    int cur_idx = idx[levels-1]++;
    h->rank = cur_idx;
    h->nd_rank.dim = dim;
    h->nd_tile_dimensions = nd_size_at_level[levels-1]; // it's actually size at "height"
    h->global_dimensions = *global_dimensions;
    Tuple_1d_to_nd_index(cur_idx, &h->nd_tile_dimensions, &h->nd_rank);
    h->nd_element_offset = *element_offset;
    h->is_selection= 0;

    // recursive tree walk to lower levels
    if(levels == 1) // Leaf level
    {
        int num_elem;
        size_t sz;
        void* ptr;

        h->flat_size = *flat_size;
        h->num_tiles = 1;
        h->tiles = NULL;
        //h->tiling = NULL;
        switch(type) {
            case HTA_TYPE_DENSE:
                num_elem = Tuple_product(flat_size);
                sz = num_elem * HTA_size_of_scalar_type(scalar_type);
                ptr = (HTA_get_rank() == owner) ? Alloc_acquire_block(sz): NULL;
                _create_dense_leaf(&h->leaf, order, num_elem, ptr);
                h->home = owner;
            break;
            case HTA_TYPE_SPARSE:
                ASSERT(0 && "not supported");
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
        h->home = owner;
        h->tiling = *tiling;
        h->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * h->num_tiles);
        h->flat_size = *flat_size;

	//if(h->height == 3) printf("(%d) Non-leaf height: %d rank: %d owner: %d \n", HTA_get_rank(), h->height, h->rank, h->home);

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
            if(isTop) {
                // FIXME: here we assume np <= (number of top level tiles)
                //        and the distribution doesn't matter (it's just tiles
                //        distributed 1-to-1 to all processes)
                h->tiles[i] = _recursive_create_distributed(type, dim, levels-1, order, &ll_offset, &tile_size, tiling + 1, mmap, scalar_type, idx, nd_size_at_level,
                        FALSE, _get_mapping(i, h->num_tiles), global_dimensions);
            } else {
                h->tiles[i] = _recursive_create_distributed(type, dim, levels-1, order, &ll_offset, &tile_size, tiling + 1, mmap, scalar_type, idx, nd_size_at_level,
                        FALSE, owner, global_dimensions);
            }

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
            //Alloc_free_block(h->tiling);
            //h->tiling = NULL;
        //}
    }
    //RefCount_release(h);
#if ENABLE_PREPACKING
    if(h->prepacked) free(h->prepacked_data_array);
#endif
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

/// FIXME: this function works only for REGULAR tiles because it assumes the tile size at the same
/// level are the same. A search is required if the HTA is not regular.
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

/* ================================================
 *
 * Global functions
 *
 * ===============================================*/

// Create an HTA with uninitialized memory storage in fork-join mode
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
    HTA* ret = HTA_create_impl(-1, NULL, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

// Create an HTA with uninitialized memory storage in SPMD mode
HTA* HTA_create_with_pid(int pid, int dim, int levels, const Tuple *flat_size, int order,
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
    HTA* ret = HTA_create_impl(pid, NULL, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);

    return ret;
}

// Create an HTA shell for pre-allocated array in fork-join mode
HTA* HTA_create_shell(void* prealloc, int dim, int levels, const Tuple *flat_size, int order,
        Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...)
{
    va_list argp;
    Tuple tiling[num_tuples];
    Tuple *ts = NULL;
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");
    ASSERT(order != ORDER_TILE); // not supported for tile layout for now

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
    HTA* ret = HTA_create_impl(-1, prealloc, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

// Create an HTA shell for pre-allocated array in SPMD mode
HTA* HTA_create_shell_with_pid(int pid, void* prealloc, int dim, int levels, const Tuple *flat_size, int order,
        Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...)
{
    va_list argp;
    Tuple tiling[num_tuples];
    Tuple *ts = NULL;
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");
    ASSERT(order != ORDER_TILE); // not supported for tile layout for now

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
    HTA* ret = HTA_create_impl(pid, prealloc, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

// The low-level implementation of HTA_create
HTA* HTA_create_impl(int pid, void* prealloc, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts)
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
#ifdef DEVDEBUG
    printf("nd_size_at_level:\n");
    for(int i = 0; i < levels; i++)
        Tuple_print(&nd_size_at_level[i]);
#endif

    HTA *ret;
    if(!HTA_is_distributed()) {
        Tuple nd_element_offset;
        Tuple_init_zero(&nd_element_offset, dim);

        Mapping* mmap = Mapping_create_impl(pid, 0, dim, levels, order, flat_size, ts, dist, _hta_scalar_size[scalar_type], prealloc);
        ret = _recursive_create(pid, HTA_TYPE_DENSE, dim, levels, order, &nd_element_offset, flat_size, ts, mmap, scalar_type, idx, nd_size_at_level, flat_size);
        // FIXME: set the root node's home to -1 (for SPMD)
        ret->home = -1;

        Mapping_destroy(mmap);
    } else { // For MPI version
        Tuple nd_element_offset;
        Tuple_init_zero(&nd_element_offset, dim);
        // FIXME: a tile is locally owned if the height is 1?
        ret = _recursive_create_distributed(HTA_TYPE_DENSE, dim, levels, order,  &nd_element_offset, flat_size, ts, NULL, scalar_type, idx, nd_size_at_level, TRUE, HTA_get_rank(), flat_size);
    }

    if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
        //for spmd: share all leaves between processes
        if(ret->pid != -1)
          HTA_make_shared_all_leaves(ret);
    }

    return ret;
}


void HTA_destroy(HTA *h) {
    _recursive_destroy(h);
}


size_t HTA_get_scalar_size(const HTA *h)
{
    ASSERT(h->scalar_type < HTA_SCALAR_TYPE_MAX && "Unsupported scalar type!");
    return _hta_scalar_size[h->scalar_type];
}

size_t HTA_size_of_scalar_type(HTA_SCALAR_TYPE t)
{
    ASSERT(t < HTA_SCALAR_TYPE_MAX && "Unsupported scalar type!");
    return _hta_scalar_size[t];
}

void HTA_print(HTA* h, int recursively) {
    printf("HTA %p (%s)\n", h, (h->type == HTA_TYPE_DENSE)?"DENSE":"SPARSE");
    printf("dim: %d, ", h->dim);
    printf("height: %d, ", h->height);

    Tuple* tiling = &h->tiling;
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
    int idx = Tuple_nd_to_1d_index(nd_idx, &h->tiling);
    return h->tiles[idx];
}

static HTA* _clone_root_node(HTA *h) {
    HTA *clone =  Alloc_acquire_block(sizeof(struct hta));
    *clone = *h;
    if(h->height > 1)
    {
        clone->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * h->num_tiles);
        //clone->tiling = Alloc_acquire_block(sizeof(Tuple) * (h->height-1));
        memcpy(clone->tiles, h->tiles, sizeof(HTA*) * h->num_tiles);
        Tuple_clone_array(&clone->tiling, &h->tiling);
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
            target =  Alloc_acquire_block(sizeof(struct hta));
            target->dim = h->dim;
            target->height = h->height;

            num_tiles = Region_cardinality(r);
            //target->tiles = (HTA*) malloc(sizeof(HTA) * num_tiles);
            target->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * num_tiles);
            //target->tiling = Alloc_acquire_block(sizeof(Tuple) * (h->height-1));
            //target->tiling = Region_get_dimension(r); // the function call *malloc* a new tuple
            target->tiling = Region_get_dimension(r);
            target->tiling.height = h->height;
            // Use iterator to walk through selected tiles to correctly compute flat size of the new root
            nd_size = target->tiling;
            nd_size.height = 1;
            Tuple_init_zero(&iter, target->dim);
            Tuple_init_zero(&target->flat_size, target->dim);
            watch_dim = target->dim - 1; // start from the least significant dimension
            for(int i = 0; i < num_tiles; i++) {
                int idx = Region_idx_to_tile_idx(r, &h->tiling, i); // the index of the original HTA
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
                Tuple_clone_array(&target->tiling + 1, &h->tiling + 1); // FIXME: these are not correct anymore
            }
            target->num_tiles = Tuple_product(&target->tiling);
            target->home = -1;
            target->type = h->type;
            target->leaf.num_elem = 0;
            target->leaf.raw = NULL;
            target->rank = -1;
            target->scalar_type = h->scalar_type;
            target->is_selection = 1;
#if ENABLE_PREPACKING
            target->prepacked = 0;
            target->num_gpps = 0;
            target->prepacked_data_array = NULL;
#endif
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
#ifdef DEVDEBUG
    printf("=> Locating element in HTA:");
    Tuple_print(nd_index);
#endif
    // The implementation is changed to meet the requirement for irregular HTAs
    // TODO: this is a linear search and can be MUCH SLOWER
    // Maybe it's  better to use a binary search
    HTA* p = h;
    Tuple local_nd_index = *nd_index;
    while(p->height > 1) {
        Tuple iter[1];
        Tuple_init_zero(iter, p->dim);
        // for each dimension, search linearly
        for(int i = 0; i < p->dim; i++) {
            int idx = local_nd_index.values[i]; // the index to find at dimension i
            int accu = 0;

            // go through the tiles in this dimension until the element is located
            for(int j = 0; j < p->tiling.values[i]; j++) {
                int sz = p->tiles[Tuple_nd_to_1d_index(iter, &p->tiling)]->flat_size.values[i];
                if(accu + sz > idx) {
                    local_nd_index.values[i] = idx - accu;
                    break;
                }
                else {
                    accu += sz;
                    iter->values[i]++;
                }
            }
        }
#ifdef DEVDEBUG
        printf("The element is located (height = %d) at:", p->height);
        Tuple_print(iter);
#endif
        p = HTA_pick_one_tile(p, iter);
    }

    ASSERT(p->height == 1);
    *leaf_offset = Tuple_nd_to_1d_index(&local_nd_index, &p->flat_size);
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
        _nd_tile_to_elem_offset(cur_idx, &cur_h->tiling, &cur_h->flat_size, global_nd_index);
        cur_h = HTA_pick_one_tile(cur_h, cur_idx);
        cur_idx += 1; // next level of iterator
    }

    Tuple_add_dimensions(global_nd_index, leaf_local_iter);
    return;
}


void HTA_init_with_array(HTA *h, const void* array) {
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
	// For distributed
	if((!HTA_is_distributed() && leaf->pid == leaf->home) || (HTA_is_distributed() && leaf->home == LOCAL)) {
	  void *ptr = HTA_get_ptr_raw_data(leaf);
	  ASSERT(ptr);
	  memcpy(((char*)ptr) + leaf_offset*unit_size, ((char*)array) + mat_offset*unit_size, unit_size);
	}
    } while(Tuple_iterator_next(&h->flat_size, &global_nd_idx));
}

void HTA_to_array(HTA *h, void* array) {
   ASSERT(array);
   ASSERT(h);

   // For distributed
   if(HTA_is_distributed()) {
      HTA_to_array_distributed(h, array);
   }
   else {
       int dim = h->dim;
       size_t unit_size = HTA_get_scalar_size(h);

       Tuple global_nd_idx;
       Tuple_init_zero(&global_nd_idx, dim);
       int mat_offset = 0;
       int leaf_offset;

       do{
           HTA *leaf = HTA_locate_leaf_for_nd_index(h, &global_nd_idx, &leaf_offset);
           void *ptr = HTA_get_ptr_raw_data(leaf);
           ASSERT(ptr);
           mat_offset = Tuple_nd_to_1d_index_by_order(leaf->leaf.order, &global_nd_idx, &h->flat_size); // offset in the source array
           switch(leaf->leaf.order) {
                case ORDER_TILE: // destination address = ptr + offset within the leaf tile
                    // no recaculationo of offset within the leaf needed
                    break;
                case ORDER_ROW: // ptr + offset of element - offset of tile start
                case ORDER_COL:
                    leaf_offset = mat_offset - Tuple_nd_to_1d_index_by_order(leaf->leaf.order, &leaf->nd_element_offset, &h->flat_size);
                    break;
                default:
                    assert(0 && "unknown layout");
           }
           memcpy(((char*)array) + mat_offset*unit_size, ((char*)ptr) + leaf_offset*unit_size, unit_size);
       } while(Tuple_iterator_next(&h->flat_size, &global_nd_idx));
   }
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
        if(h->pid == -1 // fork join mode
                || h->pid == h->home) { // spmd mode collect only owned tiles
            collect[*count] = h;
            (*count)++;
        }
    }
    else
    {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_collect_tiles(level-1, h->tiles[i], collect, count);
    }
}

void HTA_collect_set_tiles(int level, HTA *h, HTA** collect, int *count, SelecOp op, Tuple k)
{
    if(level == 0)
    {
	//FIXME: Only applied selection at leaf level
	if(op(h, k) && (h->pid == -1 || h->pid == h->home)) {
	  collect[*count] = h;
	  (*count)++;
	  //Tuple_print(&h->nd_rank);
	}
    }
    else
    {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_collect_set_tiles(level-1, h->tiles[i], collect, count, op, k);
    }
}

// For spmd: returns leaves owned by pid, and its number for the rest of processes
void HTA_collect_leaves_pid(int level, HTA *h, Leaf *collect, int *count, int pid)
{
    if(level == 0)
    {
        if(h->pid == -1 // fork join mode
                || ((h->pid == pid) && (h->pid == h->home)) ) { // spmd mode pid collect owned tiles
            if(h->type == HTA_TYPE_DENSE) assert(h->leaf.raw);
            collect[*count] = h->leaf;
            (*count)++;
        }
        if(h->home == pid && h->pid != h->home) (*count)++;
    }
    else
    {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_collect_leaves_pid(level-1, h->tiles[i], collect, count, pid);
    }
}

void HTA_set_ptr_to_leaves_pid(int level, HTA *h, Leaf *collect, int *count, int pid)
{
    assert(h->pid != -1);

    if(level == 0)
    {
        if(h->home == pid) { // tiles owned by pid, and i'm not the owner
	    if(h->type == HTA_TYPE_DENSE) assert(collect[*count].raw);
            h->leaf = collect[*count];
	    (*count)++;
        }
    }
    else
    {
        for(int i = 0; i < h->num_tiles; i++)
            HTA_set_ptr_to_leaves_pid(level-1, h->tiles[i], collect, count, pid);
    }
}

void HTA_read_element(HTA* h, Tuple** acc, void* buf) {
   int depth = h->height;
   size_t unit_size = HTA_get_scalar_size(h);
   HTA* cur_h = h;
   while(depth > 1)
   {
       Tuple *t = acc[h->height - depth];
       ASSERT(t);
       cur_h = HTA_pick_one_tile(cur_h, t);
       ASSERT(cur_h);
       depth--;
   }
   Tuple* nd_idx = acc[h->height-1];
   int idx = Tuple_nd_to_1d_index(nd_idx, &cur_h->flat_size);
   if(cur_h->type == HTA_TYPE_DENSE)
   {
       int pid = cur_h->pid;

       if(pid == -1) { // fork-join mode
           memcpy(buf, HTA_get_ptr_raw_data(cur_h) + idx*unit_size, unit_size);
       } else {        // SPMD mode
           int owner = cur_h->home;
           if(pid == owner) {
               memcpy(buf, HTA_get_ptr_raw_data(cur_h) + idx*unit_size, unit_size);
           }
           comm_bcast(pid, owner, buf, unit_size);
       }
   } else {
       ASSERT(0 && "Unsupported HTA type");
   }
}

void HTA_write_element(HTA* h, Tuple** acc, void* buf) {
   int depth = h->height;
   size_t unit_size = HTA_get_scalar_size(h);
   HTA* cur_h = h;
   while(depth > 1)
   {
       Tuple *t = acc[h->height - depth];
       ASSERT(t);
       cur_h = HTA_pick_one_tile(cur_h, t);
       ASSERT(cur_h);
       depth--;
   }
   Tuple* nd_idx = acc[h->height-1];
   int idx = Tuple_nd_to_1d_index(nd_idx, &cur_h->flat_size);
   if(cur_h->type == HTA_TYPE_DENSE)
   {
       int pid = cur_h->pid;
       if(pid == -1 || pid == cur_h->home)
           memcpy(HTA_get_ptr_raw_data(cur_h) + idx*unit_size, buf, unit_size);
   } else {
       ASSERT(0 && "Unsupported HTA type");
   }
}

void HTA_flat_read(HTA *h, const Tuple *global_nd_idx, void* buf) {
    int offset;
    int pid = h->pid;
    size_t sz = HTA_get_scalar_size(h);
    HTA *leaf = HTA_locate_leaf_for_nd_index(h, global_nd_idx, &offset);
    if(pid == -1) { // fork join mode, read the data directly
        void* src = HTA_get_ptr_raw_data(leaf);
        memcpy(buf, src + offset * sz, sz);
    } else {
        int owner = leaf->home;
        //printf("(pid=%d) HTA_flat_read source %d\n", pid, owner);
        //printf("(pid=%d) HTA_flat_read index %d\n", pid, global_nd_idx->values[0]);
        if(pid == owner) {
            void* src = HTA_get_ptr_raw_data(leaf);
            memcpy(buf, src + offset * sz, sz);
        }
        comm_bcast(pid, owner, buf, sz);
    }
}

void HTA_flat_write(HTA *h, const Tuple *global_nd_idx, void* val) {
    ASSERT(h && global_nd_idx && val);
    int offset;
    int pid = h->pid;
    size_t sz = HTA_get_scalar_size(h);
    HTA *leaf = HTA_locate_leaf_for_nd_index(h, global_nd_idx, &offset);
    if(pid == -1) { // fork join mode, read the data directly
        void* src = HTA_get_ptr_raw_data(leaf);
        memcpy(src + offset * sz, val, sz);
    } else {
        int owner = leaf->home;
        if(pid == owner) {
            void* src = HTA_get_ptr_raw_data(leaf);
            memcpy(src + offset * sz, val, sz);
        }
    }
}

void HTA_init_all_scalars(HTA *h, void* initval) {
    ASSERT(h->type == HTA_TYPE_DENSE && "only works for dense HTA");

    if(h->height > 1) { // It's not a leaf tile
        Tuple iter[h->height-1];
        HTA* leaf = HTA_iterator_begin(h, iter);
        do {
	// For distributed
	  if((!HTA_is_distributed() && leaf->pid == leaf->home) || (HTA_is_distributed() && leaf->home == HTA_get_rank())) {
            int num_elements = leaf->leaf.num_elem;
            int scalar_size = HTA_get_scalar_size(leaf);
            void* ptr = HTA_get_ptr_raw_data(leaf);
            for(int i = 0; i < num_elements; i++) {
                memcpy(ptr, initval, scalar_size);
                ptr += scalar_size;
            }
	  }
        } while((leaf = HTA_iterator_next(h, iter)));
    }
    else { // It's a leaf tile
	HTA* leaf = h;
	// For distributed
	if((!HTA_is_distributed() && leaf->pid == leaf->home) || (HTA_is_distributed() && leaf->home == HTA_get_rank())) {
            int num_elements = leaf->leaf.num_elem;
            int scalar_size = HTA_get_scalar_size(leaf);
            void* ptr = HTA_get_ptr_raw_data(leaf);
            for(int i = 0; i < num_elements; i++) {
                memcpy(ptr, initval, scalar_size);
                ptr += scalar_size;
            }
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

Tuple HTA_get_nd_num_leaves(HTA* h)
{
//    int dim = h->dim;
//    Tuple ret;
//    ret.dim = dim;
//    for(int i = 0; i < ret.dim; i++)
//        ret.values[i] = 1;
//
    //HTA *cur_h = h;
    //while(cur_h->height > 1) {
    //    for(int i = 0; i < dim; i++)
    //        ret.values[i] *= cur_h->tiling.values[i];
    //    cur_h = cur_h->tiles[0]; // go to lower level
    //}

    HTA *cur_h = h;
    while(cur_h->height > 1) {
        cur_h = cur_h->tiles[0];
    }

    return cur_h->nd_tile_dimensions;
}

HTA* _recursive_find_leaf_tile_from_flat_nd_idx(HTA *h, Tuple *flat_nd_idx, Tuple *flat_nd_num_tiles)
{
    if(h->height == 1) {
        return h;
    } else {
        int dim = h->dim;
        // compute the number of leaf tiles in the lower level tile
        Tuple lower_flat_nd_num_tiles;
        Tuple_init_zero(&lower_flat_nd_num_tiles, dim);
        for(int i = 0; i < dim; i++)
            lower_flat_nd_num_tiles.values[i] = flat_nd_num_tiles->values[i] / h->tiling.values[i];

        // compute the lower level tile where the selected leaf is located, and the new offset within
        Tuple lower_tile_nd_idx;
        Tuple_init_zero(&lower_tile_nd_idx, dim);
        Tuple lower_flat_nd_offset;
        Tuple_init_zero(&lower_flat_nd_offset, dim);
        for(int i = 0; i < dim; i++) {
            lower_tile_nd_idx.values[i] = flat_nd_idx->values[i] / lower_flat_nd_num_tiles.values[i];
            lower_flat_nd_offset.values[i] = flat_nd_idx->values[i] % lower_flat_nd_num_tiles.values[i];
        }

        // convert to 1d idx to select the lower tile
        int lower_tile_1d_idx = Tuple_nd_to_1d_index(&lower_tile_nd_idx, &h->tiling);

        return _recursive_find_leaf_tile_from_flat_nd_idx(h->tiles[lower_tile_1d_idx], &lower_flat_nd_offset, &lower_flat_nd_num_tiles);
    }
}

static HTA * _find_tile_from_flat_1d_idx(HTA* h, int flat_1d_idx, Tuple* flat_nd_num_tiles) {
    Tuple flat_nd_idx;
    Tuple_init_zero(&flat_nd_idx, h->dim);
    Tuple_1d_to_nd_index(flat_1d_idx, flat_nd_num_tiles, &flat_nd_idx);

    return _recursive_find_leaf_tile_from_flat_nd_idx(h, &flat_nd_idx, flat_nd_num_tiles);
}

static HTA * _find_tile_from_flat_nd_idx(HTA* h, Tuple* flat_nd_idx, Tuple* flat_nd_num_tiles) {
    return _recursive_find_leaf_tile_from_flat_nd_idx(h, flat_nd_idx, flat_nd_num_tiles);
}

void HTA_set_abs_region_with_tuple(Tuple* nd_num_elems, Region* elem_sel) {
    ASSERT(nd_num_elems);
    ASSERT(elem_sel);

    if(Region_cardinality(elem_sel) <= 0) { // refresh only if UNDETERMINED
        int dim = nd_num_elems->dim;

        // find undetermined range
        for(int i = 0; i < dim; i++) {
            if(elem_sel->ranges[i].card <= 0) {
                int dim_size = nd_num_elems->values[i];
                Range *r = &elem_sel->ranges[i];

                // set absolute values for the UNDETERMINED range
                //printf("before high = %d, low = %d, dim_size = %d\n", r->high, r->low, dim_size);
                r->high %= dim_size;
                if(r->high < 0) r->high += dim_size;
                r->low %= dim_size;
                if(r->low < 0) r->low += dim_size;
                //printf("after high = %d, low = %d, dim_size = %d\n", r->high, r->low, dim_size);
                ASSERT(r->high >= 0 && r->high < dim_size);
                ASSERT(r->low >= 0 && r->low < dim_size);

                r->mod = dim_size;
                //ASSERT(r->high >= r->low);
                r->card = ((r->high >= r->low) ? (r->high - r->low) : (r->high - r->low + r->mod))/(r->step) + 1;
            }
        }
        // FIXME update region card if it is stored in a field
    }
}

void HTA_set_abs_region_with_hta(HTA* h, Region* elem_sel) {
    ASSERT(h);
    ASSERT(elem_sel);

    if(Region_cardinality(elem_sel) <= 0) { // refresh only if UNDETERMINED
        //int dim = h->dim;
        Tuple nd_num_elems;

        if(h->height == 1) {// it's a leaf
            nd_num_elems = h->flat_size;
        } else {
            nd_num_elems = h->tiling;
        }

        HTA_set_abs_region_with_tuple(&nd_num_elems, elem_sel);
        // FIXME update region card if it is stored in a field
    }
}

void HTA_assign_custom(int* lhs_sel, AssignOp Tuple_iter_lhs_elems, HTA* lhs_hta, int* rhs_sel,
		       AssignOp Tuple_iter_rhs_elems, HTA* rhs_hta) {
    //ASSERT(lhs_sel && rhs_sel);
    ASSERT(lhs_hta && rhs_hta);

    Tuple iterator_src;
    Tuple iterator_dest;
    int idx_src, idx_dest;


    int mypid = lhs_hta->pid;

    HTA* lhs_leaf;
    HTA* rhs_leaf;
    int rhs_idx = 0;

    size_t sz = HTA_get_scalar_size(lhs_hta);

    // retrieve number of leaf tiles
    Tuple lhs_nd_num_tiles = HTA_get_nd_num_leaves(lhs_hta);
    int lhs_num_tiles = Tuple_product(&lhs_nd_num_tiles);
    ASSERT(lhs_num_tiles >= 1);
    Tuple rhs_nd_num_tiles = HTA_get_nd_num_leaves(rhs_hta);
    int rhs_num_tiles = Tuple_product(&rhs_nd_num_tiles);
    ASSERT(rhs_num_tiles >= 1);

    rhs_idx = 0;
    int avoid = 0, alloc_index = 1, num_elems = 0;
    HTA* rhs_leaf_old = NULL;
    HTA* lhs_leaf_old = NULL;
    int *index_elems = NULL;
    int *index_elems2 = NULL;
    //int counter = 0, counter2 = 0;

    // *******************************************
    // SENDS
    // *******************************************
    for(int lhs_idx = 0; lhs_idx < lhs_num_tiles; lhs_idx++) {
        // find lhs_leaf
        if(lhs_sel != NULL && lhs_sel[lhs_idx] != 1)
            continue;
        else {
            lhs_leaf = _find_tile_from_flat_1d_idx(lhs_hta, lhs_idx, &lhs_nd_num_tiles);
            ASSERT(lhs_leaf);
        }

        // find matching rhs_leaf
        while(rhs_sel != NULL && rhs_sel[rhs_idx] != 1) {
            rhs_idx++;

            if(rhs_idx == rhs_num_tiles)
                rhs_idx = 0;
        }
        rhs_leaf = _find_tile_from_flat_1d_idx(rhs_hta, rhs_idx, &rhs_nd_num_tiles);
        ASSERT(rhs_leaf);

        ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");

	// Iterate one time to allocate index array
	if(alloc_index == 1) {
	  idx_src = -1;
	  Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src);
	  do {
		num_elems++;
	  } while(Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src));
	  index_elems = malloc(sizeof(int) * 2 * num_elems);
	  index_elems2 = malloc(sizeof(int) * 2 * num_elems);
	  alloc_index = 0;
	  num_elems = -1;
	}

        if (lhs_leaf->home != rhs_leaf->home) {
	  if (mypid == rhs_leaf->home) { // I own the RHS. Initiate a send
                //printf("pid = %d sends for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                //size_t sz = HTA_get_scalar_size(lhs_leaf);
                gpp_t buf;

		// Optimization: check if it's necessary to recalculate rhs_num_elems
		if(num_elems > 0) {
		  avoid = 1;
		  if(rhs_leaf->leaf.num_elem != rhs_leaf_old->leaf.num_elem) avoid = 0;
		}
		if(avoid == 0) {
		  //if(rhs_leaf->pid == 0) printf("Recalculating number of elems to send.\n");
		  num_elems = 0;
		  idx_src = -1;
		  Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src);
		  do {
		    index_elems[num_elems] = sz * idx_src ;
		    num_elems++;
		  } while(Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src));
		  //counter2++;
		  rhs_leaf_old = rhs_leaf;
		}
// 		else {
// 		  printf(" (%d) Avoiding calculation (SEND).\n", rhs_leaf->pid);
// 		  counter++;
// 		}
		//printf(" (%d) Sending %d elements.\n", mypid, rhs_num_elems);

                // allocate network buffer and aggregate data to be sent
                pil_alloc(&buf, sz * num_elems);

// 		idx_src = -1;
// 		Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src);
// 		idx_dest = 0;
// 		do {
// 		  memcpy(buf.ptr + sz * idx_dest, rhs_leaf_ptr + sz * idx_src, sz);
// 		  idx_dest++;
// 		} while(Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src));

		for(int j = 0; j < num_elems; j++) memcpy(buf.ptr + sz * j, rhs_leaf_ptr + index_elems[j], sz);
		  //memcpy(buf.ptr + sz * j, rhs_leaf_ptr + sz * index_elems[j], sz);

		// initiate a send
                comm_send(mypid, buf, lhs_leaf->home, sz * num_elems, 0);

                pil_free(buf);
            }
        }

      rhs_idx++;
      if(rhs_idx == rhs_num_tiles)
            rhs_idx = 0;

      //rhs_leaf_old = rhs_leaf;

    }

    rhs_idx = 0;
    avoid = 0, num_elems = -1;
    //rhs_leaf_old = NULL;
    //lhs_leaf_old = NULL;

    // *******************************************
    // LOCAL COMMUNICATION
    // *******************************************
    for(int lhs_idx = 0; lhs_idx < lhs_num_tiles; lhs_idx++) {
        // find lhs_leaf
        if(lhs_sel != NULL && lhs_sel[lhs_idx] != 1)
            continue;
        else {
            lhs_leaf = _find_tile_from_flat_1d_idx(lhs_hta, lhs_idx, &lhs_nd_num_tiles);
            ASSERT(lhs_leaf);
        }

        // find matching rhs_leaf
        while(rhs_sel != NULL && rhs_sel[rhs_idx] != 1) {
            rhs_idx++;

            if(rhs_idx == rhs_num_tiles)
                rhs_idx = 0;
        }
        rhs_leaf = _find_tile_from_flat_1d_idx(rhs_hta, rhs_idx, &rhs_nd_num_tiles);
        ASSERT(rhs_leaf);

        ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");

        if(lhs_leaf->home == rhs_leaf->home) { // pure local assignment (this should work for fork-join since all these fields will be -1
            if(mypid == lhs_leaf->home) { // I am the owner of the computation

                void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                //size_t sz = HTA_get_scalar_size(lhs_leaf);

		// Iterate through boudary elements of the leaf tiles
		// Initialize iterator idx < 0
		// Optimization: check if it's necessary to recalculate rhs_num_elems
		if(num_elems > 0) {
		  avoid = 1;
		  if((rhs_leaf->leaf.num_elem != rhs_leaf_old->leaf.num_elem) ||
		    (lhs_leaf->leaf.num_elem != lhs_leaf_old->leaf.num_elem)) avoid = 0;
		}
		if(avoid == 0) {
		  idx_dest = -1;
		  idx_src = -1;
		  num_elems = 0;
		    Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
		    Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src);
		    do {
		      index_elems[num_elems] = sz * idx_src;
		      index_elems2[num_elems] = sz * idx_dest;
		      memcpy(lhs_leaf_ptr + sz * idx_dest, rhs_leaf_ptr + sz * idx_src, sz);
		      Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
		      num_elems++;
		    } while(Tuple_iter_rhs_elems(&rhs_leaf->flat_size, &iterator_src, &idx_src));
		    rhs_leaf_old = rhs_leaf;
		    lhs_leaf_old = lhs_leaf;
		    //counter2++;
		}
		else {
		   //idx_dest = -1;
		   //Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
		   for(int j = 0; j < num_elems; j++) {
		     memcpy(lhs_leaf_ptr + index_elems2[j], rhs_leaf_ptr + index_elems[j], sz);
		     //memcpy(lhs_leaf_ptr + sz * index_elems2[j], rhs_leaf_ptr + sz * index_elems[j], sz);
		     //Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
		   }
		   //printf(" (%d) Avoiding calculation (LOCAL).\n", rhs_leaf->pid);
		   //counter++;
		}

            }
        }

        rhs_idx++;
        if(rhs_idx == rhs_num_tiles)
            rhs_idx = 0;

	//rhs_leaf_old = rhs_leaf;
	//lhs_leaf_old = lhs_leaf;

    } // for

    rhs_idx = 0;
    avoid = 0, num_elems = -1;
    lhs_leaf_old = NULL;

    // *******************************************
    // RECEIVES
    // *******************************************
    for(int lhs_idx = 0; lhs_idx < lhs_num_tiles; lhs_idx++) {
        // find lhs_leaf
        if(lhs_sel != NULL && lhs_sel[lhs_idx] != 1)
            continue;
        else {
            lhs_leaf = _find_tile_from_flat_1d_idx(lhs_hta, lhs_idx, &lhs_nd_num_tiles);
            ASSERT(lhs_leaf);
        }

        // find matching rhs_leaf
        while(rhs_sel != NULL && rhs_sel[rhs_idx] != 1) {
            rhs_idx++;

            if(rhs_idx == rhs_num_tiles)
                rhs_idx = 0;
        }
        rhs_leaf = _find_tile_from_flat_1d_idx(rhs_hta, rhs_idx, &rhs_nd_num_tiles);
        ASSERT(rhs_leaf);

        ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");

        if (lhs_leaf->home != rhs_leaf->home) { // remote assignment
            if(mypid == lhs_leaf->home) { // I own the LHS. Initiate a receive
                //printf("pid = %d receives for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
		// Optimization: check if it's necessary to recalculate rhs_num_elems
		if(num_elems > 0) {
		  avoid = 1;
		  if(lhs_leaf->leaf.num_elem != lhs_leaf_old->leaf.num_elem) avoid = 0;
		}
		// WARNING: Assuming that rhs_num_elems = lhs_num_elems
		if(avoid == 0) {
		  num_elems = 0;
		  idx_dest = -1;
		  Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
		  do {
		    index_elems[num_elems] = sz * idx_dest;
		    num_elems++;
		  } while(Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest));
		  lhs_leaf_old = lhs_leaf;
		  //counter2++;
		}
// 		else {
// 		  printf(" (%d) Avoiding calculation (RECV).\n", rhs_leaf->pid);
// 		  counter++;
// 		}

		//printf(" (%d) Receiving %d elements.\n", mypid, rhs_num_elems);

                void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                //size_t sz = HTA_get_scalar_size(lhs_leaf);
                gpp_t buf;

                // receive aggregated data from network buffer
                pil_alloc(&buf, sz * num_elems);
                comm_recv(mypid, buf, rhs_leaf->home, sz * num_elems, 0);

// 		//Iterate through boudary elements of the leaf tile
// 		idx_dest = -1;
// 		Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest);
// 		idx_src = 0;
// 		do {
// 		  memcpy(lhs_leaf_ptr + sz * idx_dest, buf.ptr + sz * idx_src, sz);
// 		  idx_src++;
// 		} while(Tuple_iter_lhs_elems(&lhs_leaf->flat_size, &iterator_dest, &idx_dest));

		for(int j = 0; j < num_elems; j++)
		  memcpy(lhs_leaf_ptr + index_elems[j], buf.ptr + sz * j, sz);

                pil_free(buf);
            }
        } // perform assignment

        rhs_idx++;
        if(rhs_idx == rhs_num_tiles)
            rhs_idx = 0;

	//lhs_leaf_old = lhs_leaf;

    } // for

    free(index_elems);
    free(index_elems2);
    //printf(" (%d) %d iterators used, %d avoided.\n", lhs_hta->pid, counter2, counter);
}

// HTA assign assumes (1) row-major traverse (2) starts from 0
void HTA_assign(int* lhs_sel, Region* lhs_elem_sel, HTA* lhs_hta, int* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta) {
    //ASSERT(lhs_sel && rhs_sel);
    ASSERT(lhs_hta && rhs_hta);

    int mypid = lhs_hta->pid;

    HTA* lhs_leaf;
    HTA* rhs_leaf;
    int rhs_idx = 0;

    // retrieve number of leaf tiles
    Tuple lhs_nd_num_tiles = HTA_get_nd_num_leaves(lhs_hta);
    int lhs_num_tiles = Tuple_product(&lhs_nd_num_tiles);
    ASSERT(lhs_num_tiles >= 1);
    Tuple rhs_nd_num_tiles = HTA_get_nd_num_leaves(rhs_hta);
    int rhs_num_tiles = Tuple_product(&rhs_nd_num_tiles);
    ASSERT(rhs_num_tiles >= 1);

    // Go through leaf tiles of both HTAs and match them 1-to-1 for assignments
    for(int lhs_idx = 0; lhs_idx < lhs_num_tiles; lhs_idx++) {
        // find lhs_leaf
        if(lhs_sel != NULL && lhs_sel[lhs_idx] != 1)
            continue;
        else {
            lhs_leaf = _find_tile_from_flat_1d_idx(lhs_hta, lhs_idx, &lhs_nd_num_tiles);
            ASSERT(lhs_leaf);
        }

        // find matching rhs_leaf
        while(rhs_sel != NULL && rhs_sel[rhs_idx] != 1) {
            rhs_idx++;

            if(rhs_idx == rhs_num_tiles)
                rhs_idx = 0;
        }
        rhs_leaf = _find_tile_from_flat_1d_idx(rhs_hta, rhs_idx, &rhs_nd_num_tiles);
        ASSERT(rhs_leaf);

        Region abs_lhs_elem_sel;
        Region abs_rhs_elem_sel;
        if(lhs_elem_sel == NULL) {
            abs_lhs_elem_sel = Region_create_full_elem_selection(lhs_leaf->dim);
        } else {
            abs_lhs_elem_sel = *lhs_elem_sel;
        }
        if(rhs_elem_sel == NULL) {
            abs_rhs_elem_sel = Region_create_full_elem_selection(rhs_leaf->dim);
        } else {
            abs_rhs_elem_sel = *rhs_elem_sel;
        }
        HTA_set_abs_region_with_hta(lhs_leaf, &abs_lhs_elem_sel);
        HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);

        // Matching lhs_leaf and rhs_leaf found, perform assignment
        // Check scalar region selection conformatility
        //printf("lhs.card = %d, rhs.card = %d\n", Region_cardinality(&abs_lhs_elem_sel), Region_cardinality(&abs_rhs_elem_sel));
        ASSERT((Region_cardinality(&abs_lhs_elem_sel) % Region_cardinality(&abs_rhs_elem_sel)) == 0);
        //if(lhs_leaf->pid == 0) {
        //    printf("lhs_idx = %d, rhs_idx = %d\n", lhs_idx, rhs_idx);
        //    printf("lhs_elem region:\n");
        //    Region_print(&abs_lhs_elem_sel);
        //    printf("rhs_elem region:\n");
        //    Region_print(&abs_rhs_elem_sel);
        //}

        ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");
        if(lhs_leaf->home == rhs_leaf->home) { // pure local assignment (this should work for fork-join since all these fields will be -1
            if(mypid == lhs_leaf->home) { // I am the owner of the computation
                int lhs_num_elems = Region_cardinality(&abs_lhs_elem_sel);
                int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                size_t sz = HTA_get_scalar_size(lhs_leaf);
                //printf("pid = %d perform local assignment for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);

                //printf("local assignment pid = %d, lhs: %d elements selected, rhs: %d elements selected\n", mypid, lhs_num_elems, rhs_num_elems);
                //printf("lhs leaf pointer %p, rhs leaf pointer %p\n", lhs_leaf_ptr, rhs_leaf_ptr);

                // iterate through selected elements to perform assignment directly
                for(int lhs_elem_idx = 0, rhs_elem_idx = 0; lhs_elem_idx < lhs_num_elems; lhs_elem_idx++, rhs_elem_idx++) {
                    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                        rhs_elem_idx = 0;

                    memcpy(lhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_lhs_elem_sel, &lhs_leaf->flat_size, lhs_elem_idx),
                            rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                            sz);
                }
            }
        } else if (lhs_leaf->home != rhs_leaf->home) { // remote assignment
            if(mypid == lhs_leaf->home) { // I own the LHS. Initiate a receive
                //printf("pid = %d receives for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                int lhs_num_elems = Region_cardinality(&abs_lhs_elem_sel);
                int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                size_t sz = HTA_get_scalar_size(lhs_leaf);
                gpp_t buf;

                // receive aggregated data from network buffer
                pil_alloc(&buf, sz * rhs_num_elems);
                comm_recv(mypid, buf, rhs_leaf->home, sz * rhs_num_elems, 0);

                // perform write to the actual location
                for(int lhs_elem_idx = 0, rhs_elem_idx = 0; lhs_elem_idx < lhs_num_elems; lhs_elem_idx++, rhs_elem_idx++) {
                    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                        rhs_elem_idx = 0;

                    memcpy(lhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_lhs_elem_sel, &lhs_leaf->flat_size, lhs_elem_idx),
                            buf.ptr + sz * rhs_elem_idx,
                            sz);
                }

                pil_free(buf);
            } else if (mypid == rhs_leaf->home) { // I own the RHS. Initiate a send
                //printf("pid = %d sends for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                size_t sz = HTA_get_scalar_size(lhs_leaf);
                gpp_t buf;
                // allocate network buffer and aggregate data to be sent
                pil_alloc(&buf, sz * rhs_num_elems);

                for(int rhs_elem_idx = 0; rhs_elem_idx < rhs_num_elems; rhs_elem_idx++) {
                    memcpy(buf.ptr + sz * rhs_elem_idx,
                           rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                           sz);
                }

                // initiate a send
                comm_send(mypid, buf, lhs_leaf->home, sz * rhs_num_elems, 0);

                pil_free(buf);
            } else { // I don't own either one
                // do nothing
            }
        } // perform assignment
        rhs_idx++;
        if(rhs_idx == rhs_num_tiles)
            rhs_idx = 0;
    } // for
}

// This function assumes traversing the least significant dimension first and it can accept negative ranges
void HTA_assign_with_region(Region* lhs_sel, Region* lhs_elem_sel, HTA* lhs_hta, Region* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta) {
    ASSERT(lhs_hta && rhs_hta);

    int np = HTA_get_num_processes();
    int mypid = lhs_hta->pid;
    size_t sz = HTA_get_scalar_size(lhs_hta);

    // retrieve number of leaf tiles
    Tuple lhs_nd_num_tiles = HTA_get_nd_num_leaves(lhs_hta);
    Tuple rhs_nd_num_tiles = HTA_get_nd_num_leaves(rhs_hta);

    Region abs_lhs_sel;
    Region abs_rhs_sel;
    if(lhs_sel == NULL) {
        abs_lhs_sel = Region_create_full_elem_selection(lhs_hta->dim);
    } else {
        abs_lhs_sel = *lhs_sel;
    }
    if(rhs_sel == NULL) {
        abs_rhs_sel = Region_create_full_elem_selection(rhs_hta->dim);
    } else {
        abs_rhs_sel = *rhs_sel;
    }
    HTA_set_abs_region_with_tuple(&lhs_nd_num_tiles, &abs_lhs_sel);
    HTA_set_abs_region_with_tuple(&rhs_nd_num_tiles, &abs_rhs_sel);

    Tuple lhs_iter, rhs_iter;
    //Region_print(&abs_lhs_sel);
    Region_iterator_init(&lhs_iter, &abs_lhs_sel);
    //Tuple_print(&lhs_iter);
    HTA* lhs_leaf; // = _find_tile_from_flat_nd_idx(lhs_hta, &lhs_iter, &lhs_nd_num_tiles);
    HTA* rhs_leaf = NULL;
    Region real_rhs_elem_sel, real_lhs_elem_sel;
    if(lhs_elem_sel == NULL)
        real_lhs_elem_sel = Region_create_full_elem_selection(lhs_hta->dim);
    else
        real_lhs_elem_sel = *lhs_elem_sel;
    if(rhs_elem_sel == NULL)
        real_rhs_elem_sel = Region_create_full_elem_selection(rhs_hta->dim);
    else
        real_rhs_elem_sel = *rhs_elem_sel;

    if(CFG_get(CFG_OPT_AGGREGATED_ASSIGN)) {
        typedef struct msg_desc {
            int    pid;     // if it is descriptor for sends, pid indicates destination else it is the source of the message
            int    num;     // store the number of elements to prevent recomputation in the second phase
            size_t size;    // size 0 is used to indicate that this message is not communicated by this processor
            size_t offset;
        } Msg_desc;
        int lhs_num_sel_tiles = Region_cardinality(&abs_lhs_sel);
        Msg_desc send_descs[lhs_num_sel_tiles]; // descriptor of message being sent by this process for each tile
        Msg_desc recv_descs[lhs_num_sel_tiles]; // descriptor of message being received by this process for each tile
        HTA* lhs_leaves[lhs_num_sel_tiles];     // store the lhs_leaves matched to save some searching
        HTA* rhs_leaves[lhs_num_sel_tiles];     // store the rhs_leaves matched to save some searching
        for(int i = 0; i < lhs_num_sel_tiles; i++) { // initialize message descriptors
            send_descs[i].num = -1; // use -1 to indicate unintialized descriptor
            send_descs[i].size = 0;
            recv_descs[i].num = -1;
            recv_descs[i].size = 0;
        }
        size_t sendbuf_size = 0;
        size_t recvbuf_size = 0;
        int lhs_sel_tile_count = 0;

        // Phase 1: scan through every tile and create descriptors of messages for buffer allocation and aggregate messages
        do {
            lhs_leaf = _find_tile_from_flat_nd_idx(lhs_hta, &lhs_iter, &lhs_nd_num_tiles);
            lhs_leaves[lhs_sel_tile_count] = lhs_leaf;
            // find matching rhs leaf
            if(rhs_leaf == NULL) { // first time (or wrap around)
                Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            } else {
                if(!Region_iterator_next(&rhs_iter, &abs_rhs_sel)) // NOTICE: the predicate has side-effect and MUST be performed
                    Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            }
            rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);
            rhs_leaves[lhs_sel_tile_count] = rhs_leaf;

            Region abs_rhs_elem_sel = real_rhs_elem_sel;
            HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);

            ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");
            int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
            size_t msg_size = rhs_num_elems * sz;
            if(mypid == lhs_leaf->home) { // I am the owner of the computation
                Msg_desc *desc = &recv_descs[lhs_sel_tile_count];
                desc->pid = rhs_leaf->home;
                desc->num = rhs_num_elems;
                desc->size = msg_size;
                recvbuf_size += msg_size;
                //if(mypid == 0) {
                //    Tuple_print(&rhs_leaf->flat_size);
                //    printf("selected leaf idx %d to receive data of size %zu (%d)\n", lhs_sel_tile_count, msg_size, rhs_num_elems);
                //}
            }
            if (mypid == rhs_leaf->home) { // I own the RHS. Initiate a send
                Msg_desc *desc = &send_descs[lhs_sel_tile_count];
                desc->pid = lhs_leaf->home;
                desc->num = rhs_num_elems;
                desc->size = msg_size;
                sendbuf_size += msg_size;
                //if(mypid == 0) {
                //    printf("selected leaf idx %d to send data of size %zu (%d)\n", lhs_sel_tile_count, msg_size, rhs_num_elems);
                //}
            }
            lhs_sel_tile_count++;
        } while(Region_iterator_next(&lhs_iter, &abs_lhs_sel));
        ASSERT(lhs_sel_tile_count == lhs_num_sel_tiles);
        // Allocate buffer
        void *sendbuf = malloc(sendbuf_size);
        void *recvbuf = malloc(recvbuf_size);
        ASSERT(sendbuf && recvbuf);
        lhs_sel_tile_count = 0;

        // Phase 2: Offset computation - scan through the descriptors and figure out the offsets of each chunk of messages in the buffers
        size_t send_buf_cur_offset = 0;
        size_t send_offsets[np];
        size_t send_sizes[np];
        size_t recv_buf_cur_offset = 0;
        size_t recv_offsets[np];
        size_t recv_sizes[np];
        for(int p = 0; p < np; p++) {
            size_t send_size_p = 0;
            size_t recv_size_p = 0;
            send_offsets[p] = send_buf_cur_offset;
            recv_offsets[p] = recv_buf_cur_offset;
            for(int i = 0; i < lhs_num_sel_tiles; i++) {
                if(send_descs[i].size != 0 && (send_descs[i].pid == p || send_descs[i].pid == -1)) {
                    send_descs[i].offset = send_buf_cur_offset;
                    send_buf_cur_offset += send_descs[i].size;
                    send_size_p += send_descs[i].size;
                }
                if(recv_descs[i].size != 0 && (recv_descs[i].pid == p || recv_descs[i].pid == -1)) {
                    recv_descs[i].offset = recv_buf_cur_offset;
                    recv_buf_cur_offset += recv_descs[i].size;
                    recv_size_p += recv_descs[i].size;
                }
            }
            send_sizes[p] = send_size_p;
            recv_sizes[p] = recv_size_p;
            //if(mypid == 0) {
            //    printf("to send %zu bytes from %d\n", send_size_p, p);
            //    printf("to recv %zu bytes from %d\n", recv_size_p, p);
            //}
        }

        // Phase 3: Scan through tiles again and aggregate messages into the send buffer
        for(int i = 0; i < lhs_num_sel_tiles; i++) {
            lhs_leaf = lhs_leaves[i];
            rhs_leaf = rhs_leaves[i];
            Region abs_lhs_elem_sel = real_lhs_elem_sel;
            Region abs_rhs_elem_sel = real_rhs_elem_sel;
            HTA_set_abs_region_with_hta(lhs_leaf, &abs_lhs_elem_sel);
            HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);
            if (mypid == rhs_leaf->home) { // I own the RHS. Put the data in the aggregated message buffer
                //printf("pid = %d sends for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                Msg_desc* desc = &send_descs[i];
                ASSERT(desc->size != 0 && desc->num != -1);
                int rhs_num_elems = desc->num;
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);

                //for(int rhs_elem_idx = 0; rhs_elem_idx < rhs_num_elems; rhs_elem_idx++) {
                //    memcpy(sendbuf + desc->offset + sz * rhs_elem_idx,
                //           rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                //           sz);
                //}

                // Use region iterator to iterate through elements within the leaf tile
                Tuple rhs_elem_iter;
                Region_iterator_init(&rhs_elem_iter, &abs_rhs_elem_sel);
                int rhs_elem_idx = 0;
                do {
                    int rhs_leaf_offset = Tuple_nd_to_1d_index(&rhs_elem_iter, &rhs_leaf->flat_size);
                    // TODO: algebraic optimization can be applied here
                    memcpy(sendbuf + desc->offset + sz * rhs_elem_idx,
                           rhs_leaf_ptr + sz * rhs_leaf_offset,
                           sz);
                    rhs_elem_idx++;
                } while (Region_iterator_next(&rhs_elem_iter, &abs_rhs_elem_sel));
                ASSERT(rhs_elem_idx == rhs_num_elems);
            }
        } // end for

        // Phase 4: Start all-to-all communication
        comm_alltoallv(mypid, sendbuf, send_sizes, send_offsets, recvbuf, recv_sizes, recv_offsets);

        // Phase 5: Copy received data into correct location
        for(int i = 0; i < lhs_num_sel_tiles; i++) {
            lhs_leaf = lhs_leaves[i];
            rhs_leaf = rhs_leaves[i];
            Region abs_lhs_elem_sel = real_lhs_elem_sel;
            Region abs_rhs_elem_sel = real_rhs_elem_sel;
            HTA_set_abs_region_with_hta(lhs_leaf, &abs_lhs_elem_sel);
            HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);
            if (mypid == lhs_leaf->home) { // I own the RHS. Put the data in the aggregated message buffer
                Msg_desc* desc = &recv_descs[i];
                ASSERT(desc->size != 0 && desc->num != -1);
                int lhs_num_elems = Region_cardinality(&abs_lhs_elem_sel);
                int rhs_num_elems = desc->num;
                void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);

                //for(int lhs_elem_idx = 0, rhs_elem_idx = 0; lhs_elem_idx < lhs_num_elems; lhs_elem_idx++, rhs_elem_idx++) {
                //    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                //        rhs_elem_idx = 0;
                //    memcpy(lhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_lhs_elem_sel, &lhs_leaf->flat_size, lhs_elem_idx),
                //            recvbuf + desc->offset + sz * rhs_elem_idx,
                //            sz);
                //}

                // Use region iterator to iterate through elements within the leaf tile
                Tuple lhs_elem_iter;
                Region_iterator_init(&lhs_elem_iter, &abs_lhs_elem_sel);
                int rhs_elem_idx = 0;
                do {
                    int lhs_leaf_offset = Tuple_nd_to_1d_index(&lhs_elem_iter, &lhs_leaf->flat_size);
                    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                        rhs_elem_idx = 0;
                    // TODO: algebraic optimization can be applied here
                    memcpy(lhs_leaf_ptr + sz * lhs_leaf_offset,
                            recvbuf + desc->offset + sz * rhs_elem_idx,
                            sz);
                    rhs_elem_idx++;
                } while (Region_iterator_next(&lhs_elem_iter, &abs_lhs_elem_sel));
            }
        } // end for
    } else { //CFG_get(CFG_OPT_AGGREGATED_ASSIGN)
        do {
            lhs_leaf = _find_tile_from_flat_nd_idx(lhs_hta, &lhs_iter, &lhs_nd_num_tiles);
            // find matching rhs leaf
            if(rhs_leaf == NULL) { // first time (or wrap around)
                Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            } else {
                if(!Region_iterator_next(&rhs_iter, &abs_rhs_sel)) // NOTICE: the predicate has side-effect and MUST be performed
                    Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            }
            rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);
            //if(mypid == 0) {
            //    printf("lhs_iter:"); Tuple_print(&lhs_iter);
            //    printf("rhs_iter:"); Tuple_print(&rhs_iter);
            //}
            Region abs_lhs_elem_sel = real_lhs_elem_sel;
            Region abs_rhs_elem_sel = real_rhs_elem_sel;
            HTA_set_abs_region_with_hta(lhs_leaf, &abs_lhs_elem_sel);
            HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);

            // Matching lhs_leaf and rhs_leaf found, perform assignment
            // Check scalar region selection conformatility
            //printf("lhs.card = %d, rhs.card = %d\n", Region_cardinality(&abs_lhs_elem_sel), Region_cardinality(&abs_rhs_elem_sel));
            ASSERT((Region_cardinality(&abs_lhs_elem_sel) % Region_cardinality(&abs_rhs_elem_sel)) == 0);
            //int lhs_idx = Tuple_nd_to_1d_index(&lhs_iter, &lhs_nd_num_tiles);
            //int rhs_idx = Tuple_nd_to_1d_index(&rhs_iter, &rhs_nd_num_tiles);
            //if(lhs_leaf->pid == 0) {
            //    printf("lhs_idx = %d, rhs_idx = %d\n", lhs_idx, rhs_idx);
            //    printf("lhs_elem region:\n");
            //    Region_print(&abs_lhs_elem_sel);
            //    printf("rhs_elem region:\n");
            //    Region_print(&abs_rhs_elem_sel);
            //}

            ASSERT(lhs_leaf->leaf.order == ORDER_TILE && rhs_leaf->leaf.order == ORDER_TILE && "Only tile order layout is supported for now");
            if(lhs_leaf->home == rhs_leaf->home) { // pure local assignment (this should work for fork-join since all these fields will be -1
                if(mypid == lhs_leaf->home) { // I am the owner of the computation
                    int lhs_num_elems = Region_cardinality(&abs_lhs_elem_sel);
                    int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                    void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                    void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                    size_t sz = HTA_get_scalar_size(lhs_leaf);
                    //printf("pid = %d perform local assignment for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);

                    //printf("local assignment pid = %d, lhs: %d elements selected, rhs: %d elements selected\n", mypid, lhs_num_elems, rhs_num_elems);
                    //printf("lhs leaf pointer %p, rhs leaf pointer %p\n", lhs_leaf_ptr, rhs_leaf_ptr);

                    // iterate through selected elements to perform assignment directly
                    //for(int lhs_elem_idx = 0, rhs_elem_idx = 0; lhs_elem_idx < lhs_num_elems; lhs_elem_idx++, rhs_elem_idx++) {
                    //    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                    //        rhs_elem_idx = 0;

                    //    memcpy(lhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_lhs_elem_sel, &lhs_leaf->flat_size, lhs_elem_idx),
                    //            rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                    //            sz);
                    //}
                    Tuple lhs_elem_iter;
                    Region_iterator_init(&lhs_elem_iter, &abs_lhs_elem_sel);
                    int rhs_elem_idx = 0;
                    do {
                        int lhs_leaf_offset = Tuple_nd_to_1d_index(&lhs_elem_iter, &lhs_leaf->flat_size);
                        if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                            rhs_elem_idx = 0;
                        // TODO: algebraic optimization can be applied here
                        memcpy(lhs_leaf_ptr + sz * lhs_leaf_offset,
                                rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                                sz);
                        rhs_elem_idx++;
                    } while (Region_iterator_next(&lhs_elem_iter, &abs_lhs_elem_sel));
                }
            } else if (lhs_leaf->home != rhs_leaf->home) { // remote assignment
                if(mypid == lhs_leaf->home) { // I own the LHS. Initiate a receive
                    //printf("pid = %d receives for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                    int lhs_num_elems = Region_cardinality(&abs_lhs_elem_sel);
                    int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                    void* lhs_leaf_ptr = HTA_get_ptr_raw_data(lhs_leaf);
                    size_t sz = HTA_get_scalar_size(lhs_leaf);
                    gpp_t buf;

                    // receive aggregated data from network buffer
                    pil_alloc(&buf, sz * rhs_num_elems);
                    comm_recv(mypid, buf, rhs_leaf->home, sz * rhs_num_elems, 0);

                    // perform write to the actual location
                    //for(int lhs_elem_idx = 0, rhs_elem_idx = 0; lhs_elem_idx < lhs_num_elems; lhs_elem_idx++, rhs_elem_idx++) {
                    //    if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                    //        rhs_elem_idx = 0;

                    //    memcpy(lhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_lhs_elem_sel, &lhs_leaf->flat_size, lhs_elem_idx),
                    //            buf.ptr + sz * rhs_elem_idx,
                    //            sz);
                    //}
                    Tuple lhs_elem_iter;
                    Region_iterator_init(&lhs_elem_iter, &abs_lhs_elem_sel);
                    int rhs_elem_idx = 0;
                    do {
                        int lhs_leaf_offset = Tuple_nd_to_1d_index(&lhs_elem_iter, &lhs_leaf->flat_size);
                        if(rhs_elem_idx == rhs_num_elems) // wrap around rhs
                            rhs_elem_idx = 0;
                        // TODO: algebraic optimization can be applied here
                        memcpy(lhs_leaf_ptr + sz * lhs_leaf_offset,
                                buf.ptr + sz * rhs_elem_idx,
                                sz);
                        rhs_elem_idx++;
                    } while (Region_iterator_next(&lhs_elem_iter, &abs_lhs_elem_sel));

                    pil_free(buf);
                } else if (mypid == rhs_leaf->home) { // I own the RHS. Initiate a send
                    //printf("pid = %d sends for the pair lhs[%d], rhs[%d]\n", mypid, lhs_idx, rhs_idx);
                    int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
                    void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                    size_t sz = HTA_get_scalar_size(lhs_leaf);
                    gpp_t buf;
                    // allocate network buffer and aggregate data to be sent
                    pil_alloc(&buf, sz * rhs_num_elems);

                    //for(int rhs_elem_idx = 0; rhs_elem_idx < rhs_num_elems; rhs_elem_idx++) {
                    //    memcpy(buf.ptr + sz * rhs_elem_idx,
                    //           rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                    //           sz);
                    //}
                    Tuple rhs_elem_iter;
                    Region_iterator_init(&rhs_elem_iter, &abs_rhs_elem_sel);
                    int rhs_elem_idx = 0;
                    do {
                        int rhs_leaf_offset = Tuple_nd_to_1d_index(&rhs_elem_iter, &rhs_leaf->flat_size);
                        // TODO: algebraic optimization can be applied here
                        memcpy(buf.ptr + sz * rhs_elem_idx,
                               rhs_leaf_ptr + sz * rhs_leaf_offset,
                               sz);
                        rhs_elem_idx++;
                    } while (Region_iterator_next(&rhs_elem_iter, &abs_rhs_elem_sel));
                    ASSERT(rhs_elem_idx == rhs_num_elems);

                    // initiate a send
                    comm_send(mypid, buf, lhs_leaf->home, sz * rhs_num_elems, 0);

                    pil_free(buf);
                } else { // I don't own either one
                    // do nothing
                }
            } // perform assignment
        } while(Region_iterator_next(&lhs_iter, &abs_lhs_sel));
    } // end if
}

// Flatten selected region and store it to user specified location.
// The memory location being written has to be preallocated by the user before calling the function
void HTA_flatten(void* out, Region* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta) {
    ASSERT(out);
    ASSERT(rhs_hta);

    int np = HTA_get_num_processes();
    int dim = rhs_hta->dim;
    void* tmp_ptr = NULL;
    int capacity = 0;
    int mypid = rhs_hta->pid;
    HTA* rhs_leaf;
    Tuple rhs_nd_num_tiles = HTA_get_nd_num_leaves(rhs_hta);

    Region abs_rhs_sel;
    if(rhs_sel == NULL) {
        abs_rhs_sel = Region_create_full_elem_selection(dim);
    } else {
        abs_rhs_sel = *rhs_sel;
    }
    HTA_set_abs_region_with_tuple(&rhs_nd_num_tiles, &abs_rhs_sel);

    // Create temporary array of tuples that records the flat sizes
    // of selected tiles at the RHS
    int sel_num_tiles = Region_cardinality(&abs_rhs_sel);
    Tuple sel_tile_flat_sizes[sel_num_tiles]; // the flat size of each selected region
    Tuple sel_tile_nd_offsets[sel_num_tiles]; // start offset of each selected regions in the flattened array
    Tuple sel_flat_size; // flat size of the whole output array
    Tuple_init_zero(&sel_flat_size, dim);

    // Iterate through selected RHS leaf tiles and compute selected
    // flat size of elements
    Tuple rhs_iter;
    Region abs_rhs_elem_sel;
    Region_iterator_init(&rhs_iter, &abs_rhs_sel);
    Region real_rhs_elem_sel;
    int accu_done[dim];
    int first_tile = 1;
    Tuple last;
    Tuple cur_nd_offset;
    Tuple_init_zero(&cur_nd_offset, dim);
    int sel_tiles_idx = 0;
    int is_full_tile = 0;
    if(rhs_elem_sel == NULL) {
        is_full_tile = 1;
        real_rhs_elem_sel = Region_create_full_elem_selection(dim);
    }
    else
        real_rhs_elem_sel = *rhs_elem_sel;
    for(int i = 0; i < dim; i++) {
        accu_done[i] = 0;
        last.values[i] = -1;
    }

    // ^^^ FOR 1D ARRAY FLATTENING
    size_t sz = HTA_get_scalar_size(rhs_hta);
    size_t proc_buf_sizes[np];
    size_t proc_buf_offsets[np];
    int    proc_own_num_tiles[np];
    for(int i = 0; i < np; i++) {
        proc_buf_sizes[i] = 0;
        proc_own_num_tiles[i] = 0;
    }
    // ^^^ FOR 1D ARRAY FLATTENING

    do {
        // compute offsets
        if(sel_tiles_idx != 0) {
            for(int i = 0; i < dim; i++) {
                if(last.values[i] != rhs_iter.values[i]) { // changed
                    if(rhs_iter.values[i] == abs_rhs_sel.ranges[i].low) // reset
                        cur_nd_offset.values[i] = 0;
                    else
                        cur_nd_offset.values[i] += sel_tile_flat_sizes[sel_tiles_idx-1].values[i]; // offset += last tile's flat_size along changed dimension
                }
            }
        }
        sel_tile_nd_offsets[sel_tiles_idx] = cur_nd_offset;

        // update local/global flat size
        rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);

        abs_rhs_elem_sel = real_rhs_elem_sel;
        HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);
        sel_tile_flat_sizes[sel_tiles_idx] = Region_flatten(&abs_rhs_elem_sel);
        // ^^^ FOR 1D ARRAY FLATTENING
        // record accumulated tile sizes of each processes
        proc_buf_sizes[rhs_leaf->home] = Tuple_product(&sel_tile_flat_sizes[sel_tiles_idx]) * sz;
        proc_own_num_tiles[rhs_leaf->home]++;
        // ^^^ FOR 1D ARRAY FLATTENING

        // accumulate sel_flat_size for the flattened result array
        for(int i = 0; i < dim; i++) { // start checking from the higher dimension
            if(last.values[i] != rhs_iter.values[i]) { // changed
                if(!accu_done[i]) { // accumulation of global flat size
                    if(!first_tile && i != dim-1 && !accu_done[i+1]) //turn off accumulation of lower dimension
                        accu_done[i+1] = 1;
                    // accumulate this dimension
                    sel_flat_size.values[i] += sel_tile_flat_sizes[sel_tiles_idx].values[i];
                }
            }
        }

        first_tile = 0;
        sel_tiles_idx++;
        last = rhs_iter;
    } while(Region_iterator_next(&rhs_iter, &abs_rhs_sel));

    ASSERT(sel_tiles_idx == sel_num_tiles);

    if(dim == 1 && is_full_tile == 1) { // special case optimization
        // ^^^ FOR 1D ARRAY FLATTENING
        proc_buf_offsets[0] = 0;
        //printf("thread(%d): buf[%d] size %zu, offset %zu\n", mypid, 0, proc_buf_sizes[0], proc_buf_offsets[0]);
        int use_allgather = 1;
        for(int i = 1; i < np; i++) {
            if(proc_own_num_tiles[i] > 1) {
                use_allgather = 0;
                break;
            }
            proc_buf_offsets[i] = proc_buf_offsets[i-1] + proc_buf_sizes[i-1];
            //printf("thread(%d): buf[%d] size %zu, offset %zu\n", mypid, i, proc_buf_sizes[i], proc_buf_offsets[i]);
        }
        // ^^^ FOR 1D ARRAY FLATTENING
        if(use_allgather && mypid != -1) {
            // FIXME: this will only work if the tiles are mapped 1-to-1 to processes
            //        THIS WILL FAIL if each process owns more than 1 tile!
            Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            sel_tiles_idx = 0;
            do {
                rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);
                int rhs_num_elems = rhs_leaf->leaf.num_elem;
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                size_t sz = HTA_get_scalar_size(rhs_leaf);
                if(mypid == rhs_leaf->home) { // source process, prepare data
                    //printf("pid = %d starts allgather sending tile[%d]\n", Tuple_nd_to_1d_index(&rhs_iter, &rhs_nd_num_tiles));
                    comm_allgatherv(mypid, rhs_leaf_ptr, sz * rhs_num_elems, 0, out, proc_buf_sizes, proc_buf_offsets);
                    break;
                }
            } while(Region_iterator_next(&rhs_iter, &abs_rhs_sel));
        } else {
            Region_iterator_init(&rhs_iter, &abs_rhs_sel);
            sel_tiles_idx = 0;
            do {
                rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);
                int rhs_num_elems = rhs_leaf->leaf.num_elem;
                void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
                size_t sz = HTA_get_scalar_size(rhs_leaf);
                tmp_ptr = out + sz * rhs_leaf->nd_element_offset.values[0];
                if(mypid == rhs_leaf->home) { // source process, prepare data
                    memcpy(tmp_ptr, rhs_leaf_ptr, sz * rhs_num_elems);
                }
                if(mypid != -1) { // SPMD mode
                    comm_bcast(mypid, rhs_leaf->home, tmp_ptr, sz * rhs_num_elems);
                }
                sel_tiles_idx++;
            } while(Region_iterator_next(&rhs_iter, &abs_rhs_sel));
        }
    } else { // general case
        // collect data elements being selected
        //      into consecutive memory locations in the buffer to be sent
        Region_iterator_init(&rhs_iter, &abs_rhs_sel);
        sel_tiles_idx = 0;
        do {
            rhs_leaf = _find_tile_from_flat_nd_idx(rhs_hta, &rhs_iter, &rhs_nd_num_tiles);
            ASSERT(rhs_leaf->leaf.order == ORDER_TILE);
            abs_rhs_elem_sel = real_rhs_elem_sel;
            HTA_set_abs_region_with_hta(rhs_leaf, &abs_rhs_elem_sel);
            int rhs_num_elems = Region_cardinality(&abs_rhs_elem_sel);
            void* rhs_leaf_ptr = HTA_get_ptr_raw_data(rhs_leaf);
            size_t sz = HTA_get_scalar_size(rhs_leaf);

            // buffer allocation
            if(tmp_ptr == NULL) { // first time allocation
                capacity = rhs_num_elems;
                tmp_ptr = malloc(capacity * sz);
            } else if(capacity < rhs_num_elems) { // reallocation
                free(tmp_ptr);
                capacity = rhs_num_elems;
                tmp_ptr = malloc(capacity * sz);
            }

            // broadcast aggregated data from network buffer
            if(mypid == rhs_leaf->home) { // source process, prepare data
                for(int rhs_elem_idx = 0; rhs_elem_idx < rhs_num_elems; rhs_elem_idx++) {
                    memcpy(tmp_ptr + sz * rhs_elem_idx,
                            rhs_leaf_ptr + sz * Region_idx_to_tile_idx(&abs_rhs_elem_sel, &rhs_leaf->flat_size, rhs_elem_idx),
                            sz);
                }
            }
            if(mypid != -1) { // SPMD mode
                comm_bcast(mypid, rhs_leaf->home, tmp_ptr, sz * rhs_num_elems);
            }

            // fill in data to the result array
            Tuple* sel_tile_flat_size = &sel_tile_flat_sizes[sel_tiles_idx];
            Tuple sel_tile_local_nd_idx;
            Tuple_iterator_begin(dim, 1, &sel_tile_local_nd_idx);
            int elem_idx = 0;
            // iterate through the buffer and fill-in data
            do {
                Tuple elem_nd_offset = sel_tile_nd_offsets[sel_tiles_idx]; // nd_offset (global to the out array)
                Tuple_add_dimensions(&elem_nd_offset, &sel_tile_local_nd_idx);
                int out_idx = Tuple_nd_to_1d_index(&elem_nd_offset, &sel_flat_size);
                memcpy(out + out_idx * sz, tmp_ptr + elem_idx * sz, sz);
                elem_idx++;
            } while (Tuple_iterator_next(sel_tile_flat_size, &sel_tile_local_nd_idx));

            sel_tiles_idx++;
        } while(Region_iterator_next(&rhs_iter, &abs_rhs_sel));
        free(tmp_ptr);
    }
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
    ASSERT(!HTA_is_distributed() && "Distributed sparse HTAs not supported");

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
    HTA* ret = HTA_sparse_create_impl(-1, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

HTA* HTA_sparse_create_with_pid(int pid, int dim, int levels, const Tuple *flat_size, int order,
        Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...)
{
    va_list argp;
    Tuple tiling[num_tuples];
    Tuple *ts = NULL;
    ASSERT(levels == num_tuples+1 && "Levels should be (the number of tuples in tiling) + 1");
    ASSERT(dim > 0 && "Invalid dimension value specified");
    ASSERT(!HTA_is_distributed() && "Distributed sparse HTAs not supported");

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
    HTA* ret = HTA_sparse_create_impl(pid, dim, levels, flat_size, order, dist, scalar_type, num_tuples, ts);
    return ret;
}

HTA* HTA_sparse_create_impl(int pid, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts)
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
    Mapping* mmap = Mapping_create_impl(pid, 1, dim, levels, order, flat_size, ts, dist, _hta_scalar_size[scalar_type], NULL);
    HTA* ret = _recursive_create(pid, HTA_TYPE_SPARSE, dim, levels, order, &nd_element_offset, flat_size, ts, mmap, scalar_type, idx, nd_size_at_level, flat_size);
    // FIXME: set the root node's home to -1 (for SPMD)
    ret->home = -1;
    Mapping_destroy(mmap);
    return ret;
}

void HTA_init_sparse_leaf(HTA *h, int nnz, void* val, int* col_ind, int* row_ptr) {
    ASSERT(h->dim == 2 && "only 2D matrix is supported");
    ASSERT(h->height == 1 && "the input HTA has to be leaf tile");
    ASSERT(h->pid == h->home);
    if(nnz == 0) // special case return immediately
        return;

    int row_dim = h->flat_size.values[0]; // the row dimension
    size_t sz = HTA_get_scalar_size(h);
    Leaf* leaf = &(h->leaf);

    // allocate space
    leaf->num_elem = nnz;
    leaf->raw = Alloc_acquire_block(nnz*sz + nnz*sizeof(int) + (row_dim+1)*sizeof(int));
    void * val_ptr = leaf->raw;
    int * col_ind_ptr = (int*)(val_ptr + nnz*sz);
    int * row_ptr_ptr = (int*)(val_ptr + nnz*sz + nnz*sizeof(int));

    memcpy(val_ptr, val, nnz*sz);
    memcpy(col_ind_ptr, col_ind, nnz*sizeof(int));
    memcpy(row_ptr_ptr, row_ptr, (row_dim+1)*sizeof(int));

#if ENABLE_PREPACKING
    h->prepacked = 0;
#endif
}

void HTA_init_with_sparse_matrix(HTA *hs, int num_nz, int num_rows, void* val, int* col_ind, int* row_ptr)
{
    void *tile_val;
    int *tile_col_ind;
    int *tile_row_ptr;

    ASSERT(hs->dim == 2 && "only support 2D for now");
    Tuple iter[hs->height-1];
    //Tuple_iterator_begin(hs->dim, hs->height-1, iter);
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

    HTA *leaf = HTA_iterator_begin(hs, iter);
    do {
        int nnz = 0;
        int prev_row = -1;

        if(leaf->pid == -1 ||(leaf->pid == leaf->home)) {
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
        }
    } while((leaf = HTA_iterator_next(hs, iter)));

    // cleanup
    free(tile_val);
    free(tile_col_ind);
    free(tile_row_ptr);
}

// ========================================================================
// Iterators (for irregular HTAs)
// ========================================================================
HTA* HTA_iterator_begin(HTA* h, Tuple* iter)
{
    Tuple_iterator_begin(h->dim, h->height-1, iter);
    return HTA_iterator_to_hta(h, iter);
}

HTA* HTA_iterator_next(HTA* h, Tuple* iter)
{
    if(iter->height > 1) {
        // try to traverse lower levels
        int idx = Tuple_nd_to_1d_index(iter, &h->tiling);
        HTA* t = HTA_iterator_next(h->tiles[idx], iter+1);
        if(t != NULL) // found
            return t;
        else {
            // try to go to the next tile at this level
            if(Tuple_inc(&h->tiling, iter)) {
                return HTA_iterator_to_hta(h, iter);
            }
            else
                return NULL;
        }
    }
    else { // height == 1
        if(Tuple_inc(&h->tiling, iter)) {
            return HTA_iterator_to_hta(h, iter);
        }
        else
            return NULL;
    }
}

// ------------------------------
// For distributed
// ------------------------------
int _my_rank;
int _num_processes = 1;
int _is_distributed = 0;

int HTA_is_distributed() {
    return _is_distributed;
}

int HTA_get_rank() {
    assert(HTA_is_distributed() && "should never call this function in shared memory HTA");
    return _my_rank;
}

int HTA_get_num_processes() {
    return _num_processes;
}

// ------------------------------
// For SPMD
// ------------------------------
// Fill boolean array selecting elements for a wavefront iteration
// Type = 0 --> RHS, Type = 1 --> LHS
// Propagation >= 0 --> increase along dimension
// Propagation < 0 --> decrease along dimension
// Iteration of the wavefront computation
void HTA_fill_boolean_array(HTA *h, int dimension, int type, int propagation, int iteration, int *sel)
{
    ASSERT(h);
    ASSERT(h->height > 1);
    ASSERT(dimension >= 0 && iteration >= 0);
    ASSERT(sel);

    Tuple iter[h->height-1];
    HTA* leaf = HTA_iterator_begin(h, iter);
    int sum = 0;
    int num_tiles_dimension = leaf->nd_tile_dimensions.values[dimension];
    do {
      int idx = leaf->rank;
      int idx_dim = leaf->nd_rank.values[dimension];
      for(int i = 0; i < leaf->dim; i++) sum += leaf->nd_rank.values[i];
      sel[idx] = 0;
      if (sum == iteration) { // this tile is considered in this iteration
	if(type == 0) { // RHS
	  if(propagation >= 0 && idx_dim < num_tiles_dimension-1) {
	    sel[idx] = 1;
	    //if(leaf->pid == 0) Tuple_print(&leaf->nd_rank);
	  }
	  if(propagation < 0 && idx_dim > 0) {
	    sel[idx] = 1;
	    //if(leaf->pid == 0) Tuple_print(&leaf->nd_rank);
	  }
	}
	else { // LHS
	  if(propagation >= 0 && idx_dim > 0 && idx_dim <= num_tiles_dimension-1) {
	    sel[idx] = 1;
	    //if(leaf->pid == 0) Tuple_print(&leaf->nd_rank);
	  }
	  if(propagation < 0 && idx_dim >= 0 && idx_dim < num_tiles_dimension-1) {
	    sel[idx] = 1;
	    //if(leaf->pid == 0) Tuple_print(&leaf->nd_rank);
	  }
	}
      }
      sum = 0;
    }
    while((leaf = HTA_iterator_next(h, iter)));

}

// Get a tuple that contains the dimension of virtual process mesh
Tuple HTA_get_vp_mesh(int dim) {
    Tuple mesh;
    mesh.height = 1;

    if(dim == 1) {
       Tuple_init(&mesh, 1, _num_processes);
    }
    else if (dim == 2) {
        if(isPowerOfTwo(_num_processes)) {
            int logp = logint2(_num_processes);
            int rows, cols;
            rows = 0x1 << (logp/2);
            cols = _num_processes / rows;
            Tuple_init(&mesh, 2, rows, cols);
        } else {
            Tuple_init(&mesh, 2, 1, _num_processes); // FIXME: find a better mesh shape for non-power of two numbers
        }
    }
    else {
        assert(0 && "mesh of dim >= 3 is not supported");
    }

    return mesh;
}

void HTA_barrier(int pid) {
    if(pid != -1) { // SPMD mode
        pil_barrier_all(pid);
    } else { // fork join mode
        /* no action */
    }
}
// -----------------------------
// HTA init function
// -----------------------------
void hta_init(int pid) {
    if(pid != -1) // SPMD mode
        _num_processes = pil_get_nwCount();
    Alloc_init();
}

