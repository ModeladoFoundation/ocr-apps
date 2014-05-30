#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "HTA.h"
#include "Tuple.h"
#include "Distribution.h"
#include "util.h"

void HTA_partial_reduce_pil(ReduceOp fr_op, HTA* h1, int dim_reduc, void* s1, HTA** ph2);

static void _reduce_two_tiles(ReduceOp op, HTA* dest, HTA* src)
{
    int num_scalars = dest->leaf.num_elem; 

#ifdef DEBUG
    ASSERT(dest->height == 1 && src->height == 1);
    ASSERT(num_scalars == src->leaf.num_elem);
#endif

    size_t sz = HTA_get_scalar_size(dest);
    void* dest_ptr = HTA_get_ptr_raw_data(dest);
    void* src_ptr =  HTA_get_ptr_raw_data(src);
    for(int i = 0; i < num_scalars; i++)
    {
        op(dest->scalar_type, dest_ptr + i * sz , src_ptr + i * sz);
    }
}

static void _partial_reduce_scalar(ReduceOp op, HTA* src, HTA* dest, int dim_reduc)
{
    int i, cont;
    int dim_trav, tile_size_src;
    void* dest_ptr = HTA_get_ptr_raw_data(dest);
    void* src_ptr = HTA_get_ptr_raw_data(src);
    
    ASSERT(dim_reduc < src->dim && dim_reduc >=0);
    
    // Source Tile
    Tuple nd_size_src = src->flat_size;
    tile_size_src = Tuple_product(&nd_size_src);
    Tuple nd_idx_src; // = Tuple_create_empty(src->dim);
    Tuple_init_zero(&nd_idx_src, src->dim);
    size_t scalar_size_src = HTA_get_scalar_size(src);
    // Destination Tile
    Tuple nd_size_dest = dest->flat_size;
    Tuple nd_idx_dest; // = Tuple_create_empty(dest->dim);    
    Tuple_init_zero(&nd_idx_dest, dest->dim);
    size_t scalar_size_dest = HTA_get_scalar_size(dest);
    ASSERT(scalar_size_dest == scalar_size_src);
    
    if(dim_reduc == src->dim-1) dim_trav = dim_reduc -1;
    else dim_trav = src->dim-1;
    
    cont = 0;
    do {	
        int offset_src = Tuple_nd_to_1d_index(&nd_idx_src, &nd_size_src) * scalar_size_src;
	int offset_dest = Tuple_nd_to_1d_index(&nd_idx_dest, &nd_size_dest) *scalar_size_dest; 

        op(dest->scalar_type, dest_ptr + offset_dest, src_ptr + offset_src); 

	nd_idx_src.values[dim_reduc]++;
	if(nd_idx_src.values[dim_reduc] ==  nd_size_src.values[dim_reduc]) {  
	    nd_idx_src.values[dim_reduc] = 0;
	    nd_idx_src.values[dim_trav]++;
	    if(nd_idx_src.values[dim_trav] == nd_size_src.values[dim_trav]) {
	      nd_idx_src.values[dim_trav] =  0;
	      for(i = dim_trav-1; i >=0; i--) {
		if(i != dim_reduc) {
		  nd_idx_src.values[i]++;
		  if(nd_idx_src.values[i] < nd_size_src.values[i]) break;
		  nd_idx_src.values[i] =  0;
		}
	      }
	    }
	    for(i = 0; i < dest->dim; i++) nd_idx_dest.values[i] = nd_idx_src.values[i];
	    nd_idx_dest.values[dim_reduc] = 0; 
	}
	cont++;
    }
    while (cont < tile_size_src);
}

static HTA *_map_to_reduced_tile(HTA *h, Tuple *h_iter, HTA *r, int dim_reduc) 
{
    Tuple r_iter[r->height-1];
    Tuple_clone_array(r_iter, h_iter);
    for(int i = 0; i < r_iter->height; i++)
        r_iter[i].values[dim_reduc] = 0;
    return HTA_iterator_to_hta(r, r_iter);
}

HTA* HTA_partial_reduce(ReduceOp fr_op, HTA* h1, int dim_reduc, void* s1)
{
    HTA* result; // placeholder for the newly created partial reduction result HTA
    HTA_partial_reduce_pil(fr_op, h1, dim_reduc, s1, &result);
    return result;
}

HTA* HTA_allocate_partial_reduce_storage(HTA *h, int dim_reduc, void *initval) {
    ASSERT(h && initval);
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;
    Dist dist;
    Dist_init(&dist, 0);
    Tuple tsa[h->height-1];
    Tuple *ts = tsa;
    Tuple flat_size;
    if(h->height > 1) {
        Tuple_clone_array(ts, h->tiling);
        Tuple_set_tuples_dim(ts, dim_reduc, 1);   
    }
    else // hc is a leaf
    {
        ts = NULL;
    }
    flat_size = h->flat_size;
    flat_size.values[dim_reduc] = 1;
    HTA *ret = HTA_create_with_ts(h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, ts);
    HTA_init_all_scalars(ret, initval);

    return ret;
}

HTA** HTA_allocate_partial_reduce_temporary(HTA *h, int dim_reduc, void *initval) {
    HTA **ha;

    ASSERT(h->height > 1 && "only support non-leaf hta for now");
    // Creation of the final resulting HTA (for tile reduction)
    ha = (HTA**) malloc(sizeof(HTA*) * h->num_tiles);
    for(int i = 0; i < h->num_tiles; i++) {
        ha[i] = HTA_allocate_partial_reduce_storage(h->tiles[i], dim_reduc, initval);
    }
    return ha;
}

void HTA_sequential_partial_reduce(ReduceOp r_op, HTA* h, HTA* r, int dim_reduc)
{
    ASSERT(h->height == r->height);
    // Iterator each leaf tiles of h and reduce them
    if(h->height > 1) {
        Tuple h_iter[h->height-1];
        Tuple_iterator_begin(h->dim, h->height-1, h_iter);
        do {
            HTA *h_leaf = HTA_iterator_to_hta(h, h_iter);
            HTA *r_leaf = _map_to_reduced_tile(h, h_iter, r, dim_reduc);
            _partial_reduce_scalar(r_op, h_leaf, r_leaf, dim_reduc);
        } while(Tuple_iterator_next(h->tiling, h_iter));
    }
    else {
        _partial_reduce_scalar(r_op, h, r, dim_reduc);
    }
}

void HTA_merge_partial_reduce_results(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA** ha1, void * initval) 
{
    int i;
    
    // map the tiles to the result array
    //Tuple root_size = Tuple_clone_one(r->tiling);
    // FIXME: work around clone one height problem
    Tuple root_size = r->tiling[0];
    root_size.height = 1;
    //Tuple_overwrite_values(root_size, r->tiling); 
    
    ASSERT(root_size.values[dim_reduc] == 1);
    Tuple root_iter;
    Tuple_init_zero(&root_iter, h->dim);
    Tuple orig_root_size = h->tiling[0]; 

    do {
        ASSERT(root_iter.values[dim_reduc] == 0);
        Tuple selected = root_iter;
        // for each result tile, find out which source tiles are used for the result
        int r_idx = Tuple_nd_to_1d_index(&root_iter, &root_size);
        HTA *tree = r->tiles[r_idx];
	int tree_height = tree->height;
	if(tree_height > 1) { // It's not a leaf tile
	  Tuple leaf_iter[tree_height-1];
          Tuple_iterator_begin(h->dim, tree_height-1, leaf_iter);
	  do{
	    HTA* r_leaf = HTA_iterator_to_hta(tree, leaf_iter);
	    for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
	      {
		selected.values[dim_reduc] = i;
		int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                HTA* s_leaf = HTA_iterator_to_hta(ha1[s_idx], leaf_iter);
                _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
            }
	  } while(Tuple_iterator_next(tree->tiling, leaf_iter)); 
	}
	else { // It's a leaf tile
	  HTA* r_leaf = r->tiles[r_idx];
	  for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
	    {
	      selected.values[dim_reduc] = i;
	      int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
              HTA* s_leaf = ha1[s_idx];
              _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
	    }
	}
    } while(Tuple_iterator_next(&root_size, &root_iter));
}
# if 0
HTA* HTA_sequential_partial_reduce(ReduceOp scalar_op, HTA* h, int dim_reduc, void* initval)
{
    int i;
    int temp, leaf_level, num_leaves;
    int dest_leaf_level, dest_num_leaves;
    int result_leaf_level, result_num_leaves;
    Tuple dest_tsa[h->height-1];
    Tuple *dest_ts = NULL;
    Tuple temp_tuple; // = Tuple_create_empty(h->dim); 
    Tuple_init_zero(&temp_tuple, h->dim);
    Tuple dest_flat_size;
    HTA *g, *r, *src, *dest, **hta1, **hta2, **hta3;
    HTA_SCALAR_TYPE scalar_type;
    
    scalar_type = h->scalar_type;
    
    // Creation of the temporary resulting HTA (for scalar reduction)
    // hta1 is an array of HTA leaf tiles of the source HTA
    if(h->height > 1) { // h is not a leaf tile  
      leaf_level = HTA_LEAF_LEVEL(h);
      num_leaves = Tuple_count_elements(h->tiling, leaf_level);
      hta1 = (HTA**) malloc(num_leaves * sizeof(HTA*)); // safe to use malloc here
      temp = 0;
      HTA_collect_tiles(leaf_level, h, hta1, &temp);
      ASSERT(num_leaves == temp);

      Tuple_clone_array(dest_tsa, h->tiling);
      dest_ts = dest_tsa;
      Tuple_get_size_of_dimensions(h->tiling, leaf_level, &temp_tuple);
      dest_flat_size = h->flat_size;
      dest_flat_size.values[dim_reduc] = temp_tuple.values[dim_reduc];
    }
    else { // h is a leaf tile
      dest_ts = NULL;
      dest_flat_size = h->flat_size;
      dest_flat_size.values[dim_reduc] = 1;
      num_leaves = 1;
      hta1 = (HTA**) malloc(num_leaves * sizeof(HTA*)); // safe to use malloc here
      hta1[0] = h;
    }
    
    
    Dist dist;
    Dist_init(&dist, 0);
    // FIXME: Build temporary HTA storage. It will have to be send back to main EDT and needs
    //        to be allocated as unhashed data block?
    g = HTA_create_with_ts(h->dim, h->height, &dest_flat_size, 0, &dist, scalar_type, h->height-1, dest_ts);
    
    HTA_init_all_scalars(g, initval);
    
    // hta2 is an array of HTA leaf tiles of the new destination HTA
    if(h->height > 1) {
      dest_leaf_level = HTA_LEAF_LEVEL(g);
      dest_num_leaves = Tuple_count_elements(g->tiling, dest_leaf_level);
      hta2 = (HTA**) malloc(dest_num_leaves * sizeof(HTA*));
      temp = 0;
      HTA_collect_tiles(dest_leaf_level, g, hta2, &temp);
      ASSERT(dest_num_leaves == temp && dest_num_leaves == num_leaves);
    }
    else {
      dest_num_leaves =  1;
      hta2 = (HTA**) malloc(dest_num_leaves * sizeof(HTA*));
      hta2[0] = g;
    

    // Iterate through all the leaf tiles
    for(i = 0; i < num_leaves; i++) {
      _partial_reduce_scalar(scalar_op, hta1[i], hta2[i], dim_reduc);
    }
    
    if(h->height > 1) { // If h is not a leaf tile
      // Creation of the final resulting HTA (for tile reduction)
      Tuple result_ts[h->height-1];
      Tuple_clone_array(result_ts, h->tiling);
      Tuple_set_tuples_dim(result_ts, dim_reduc, 1);   
      Tuple result_flat_size = h->flat_size;
      result_flat_size.values[dim_reduc] = 1;
    
      // FIXME: Build temporary HTA storage. It will have to be send back to main EDT and needs
      //        to be allocated as unhashed data block?
      r = HTA_create_with_ts(h->dim, h->height, &result_flat_size, 0, &dist, scalar_type, h->height-1, result_ts);
     
      HTA_init_all_scalars(r, initval);
 
      result_leaf_level = HTA_LEAF_LEVEL(r);
      result_num_leaves = Tuple_count_elements(r->tiling, result_leaf_level);
      hta3 = (HTA**) malloc(result_num_leaves * sizeof(HTA*));
      temp = 0;
      HTA_collect_tiles(result_leaf_level, r, hta3, &temp);
    
      ASSERT(result_num_leaves == temp);
    
      Tuple iterator[h->height-1];
      Tuple iterator_r[h->height-1];
      Tuple_iterator_begin(h->dim, h->height-1, iterator);
    
      do
      {
	  src = HTA_iterator_to_hta(g, iterator);
	  Tuple_clone_array(iterator_r, iterator); // Mapping src to dest tile
	  Tuple_set_tuples_dim(iterator_r, dim_reduc, 0);
	  dest = HTA_iterator_to_hta(r, iterator_r);
	  _reduce_two_tiles(scalar_op, dest, src);
      } 
      while(Tuple_iterator_next(dest_ts, iterator));
      HTA_destroy(g);
    }
    else r = g; // In case h is a leaf tile
    
    return(r);
}


#endif
