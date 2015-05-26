#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pil_mem.h"
#include "pil_nw.h"

#include "HTA.h"
#include "Tuple.h"
#include "Distribution.h"
#include "util.h"
#include "Comm.h"

void HTA_partial_reduce_pil(ReduceOp fr_op, HTA* h1, int dim_reduc, void* s1, HTA** ph2);

// reduce two tiles of the same size element by element
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

int _get_stride(Tuple *tiling, int dim_reduc)
{
    int stride = 1;
    for(int i = tiling->dim - 1; i != dim_reduc; i--) {
        stride *= tiling->values[i];
    }
    return(stride);
}

// partial reduce src tile and store to dest tile
static void _partial_reduce_scalar(ReduceOp op, HTA* src, HTA* dest, int dim_reduc, void *initval)
{

    void* dest_ptr = HTA_get_ptr_raw_data(dest);
    void* src_ptr = HTA_get_ptr_raw_data(src);

    ASSERT(dim_reduc < src->dim && dim_reduc >=0);

    // Source Tile
    Tuple nd_size_src = src->flat_size;
    Tuple nd_idx_src; // = Tuple_create_empty(src->dim);
    Tuple_init_zero(&nd_idx_src, src->dim);
    size_t scalar_size_src = HTA_get_scalar_size(src);
    // Destination Tile
    Tuple nd_size_dest = dest->flat_size;
    Tuple nd_idx_dest; // = Tuple_create_empty(dest->dim);
    Tuple_init_zero(&nd_idx_dest, dest->dim);
    size_t scalar_size_dest = HTA_get_scalar_size(dest);

    ASSERT(scalar_size_dest == scalar_size_src);

    HTA_init_all_scalars(dest, initval);

#if (REDUCE_OPT & PARTIAL_REDUCE_SCALAR_OPT)
    Tuple it[1];
    int offset_src = 0, offset_dest = 0;
    HTA_SCALAR_TYPE scalar_type = dest->scalar_type;
    int src_elems = nd_size_src.values[dim_reduc];
    int stride = _get_stride(&nd_size_src, dim_reduc) * scalar_size_src;
    //printf("dimreduc = %d, stride = %d\n", dim_reduc, stride);

    if (nd_size_src.values[dim_reduc] == 1) { // size is the same but still need to copy data
      memcpy(dest_ptr, src_ptr, src->leaf.num_elem*scalar_size_src);
    }
    else {
      Tuple_iterator_begin(dest->dim, 1, it); // iterate over the dest tile elements
      do {
	offset_src = Tuple_nd_to_1d_index(it, &nd_size_src) * scalar_size_src;
	for (int i = 0; i < src_elems; i++) {
	  //printf("Offset src: %d, Offset dest %d\n", offset_src, offset_dest);
	  op(scalar_type, dest_ptr + offset_dest, src_ptr + offset_src);
	  offset_src = offset_src + stride;
	}
	offset_dest = offset_dest + scalar_size_dest;
      } while(Tuple_iterator_next(&nd_size_dest, it));
    }
#else
     int i, cont = 0;
     int dim_trav, tile_size_src;

     tile_size_src = Tuple_product(&nd_size_src);
     if(dim_reduc == src->dim-1) dim_trav = dim_reduc -1;
     else dim_trav = src->dim-1;
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
#endif
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

void HTA_partial_reduce_with_preallocated(ReduceOp fr_op, HTA* h1, int dim_reduc, void* s1, HTA** ha, HTA *h2)
{
    ASSERT(ha && h2);
    HTA_partial_reduce_with_preallocated_pil(fr_op, h1, dim_reduc, s1, ha, h2);
}

void _set_reduce_ts(HTA *h, Tuple* ts, int dim_reduc) {
    if(h->height > 1) {
        //Tuple_clone_array(ts, h->tiling);
        // FIXME: this will not work for irregular HTAs
        HTA* cur_h = h;
        Tuple* t = ts;
        while(cur_h->height > 1) {
            *t = cur_h->tiling;
            t->height = cur_h->height - 1;
            cur_h = cur_h->tiles[0];
            t++;
        }
        Tuple_set_tuples_dim(ts, dim_reduc, 1);
    }
    else // hc is a leaf
    {
        ts = NULL;
    }
}

HTA* HTA_allocate_partial_reduce_storage(HTA *h, int dim_reduc, void *initval) {
    ASSERT(h && initval);
    HTA *ret;
    Tuple flat_size;
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;
    Dist dist;
    //Dist_init(&dist, 0); // FIXME!! Distribution is redefined
    if (h->height == 1) { // Leaf tile
	flat_size = h->flat_size;
	flat_size.values[dim_reduc] = 1;
	ret = HTA_create_impl(-1, NULL, h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, 0);
    }
    else {
	Tuple tsa[h->height-1];
	//Tuple *ts = tsa;
	_set_reduce_ts(h, tsa, dim_reduc);
	//if(h->height > 1) {
	//    //Tuple_clone_array(ts, h->tiling);
	//    // FIXME: this will not work for irregular HTAs
	//    HTA* cur_h = h;
	//    Tuple* t = ts;
	//    while(cur_h->height > 1) {
	//        *t = cur_h->tiling;
	//        t->height = cur_h->height - 1;
	//        cur_h = cur_h->tiles[0];
	//        t++;
	//    }
	//    Tuple_set_tuples_dim(ts, dim_reduc, 1);
	//}
	//else // hc is a leaf
	//{
	//    ts = NULL;
	//}
	flat_size = h->flat_size;
	flat_size.values[dim_reduc] = 1;
	// FIXME!!
	ret = HTA_create_impl(-1, NULL, h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, tsa);
    }

    HTA_init_all_scalars(ret, initval);

    return ret;
}

HTA* HTA_allocate_partial_reduce_storage_spmd(HTA *h, int dim_reduc, void *initval) {
    ASSERT(h && initval);
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;

    //---------------------------------------------------
    //Compute the shape of the HTA for partial reduce results
    //---------------------------------------------------
    ASSERT(h->height > 1);
    Tuple tsa[h->height-1];
    //Tuple *ts = tsa;
    Tuple flat_size;
    _set_reduce_ts(h, tsa, dim_reduc);
    flat_size = h->flat_size;
    flat_size.values[dim_reduc] = 1;
    //---------------------------------------------------
    // Storage allocation
    //---------------------------------------------------
    Dist dist;
    Dist_init_projected(&dist, &h->dist, dim_reduc);
    HTA *ret = HTA_create_impl(h->pid, NULL, h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, tsa);
    HTA_init_all_scalars(ret, initval);

    return ret;
}
HTA** HTA_allocate_partial_reduce_temporary(HTA *h, int dim_reduc, void *initval) {
    HTA **ha;

    ASSERT(h->height > 1 && "only support non-leaf hta for now");
    // Creation of the final resulting HTA (for tile reduction)
    // FIXME: malloc here potential problem?
    ha = (HTA**) malloc(sizeof(HTA*) * h->num_tiles);
    for(int i = 0; i < h->num_tiles; i++) {
        if(h->pid == -1) {// fork-join mode, use the original implementation
            ha[i] = HTA_allocate_partial_reduce_storage(h->tiles[i], dim_reduc, initval);
	}
	else // h->pid != -1
        {
            HTA* t = h->tiles[i];
            if(t->pid == t->home) // this prorcess owns the tile
            { // Allocate a local HTA for partial results (use the original method to allocate local temporary HTA conditionally)
                ha[i] = HTA_allocate_partial_reduce_storage(h->tiles[i], dim_reduc, initval);
            }
        }
    }
    return ha;
}

HTA* HTA_allocate_partial_reduce_temporary_spmd(HTA *h, int dim_reduc, void *initval) {
    ASSERT(h && initval);
    ASSERT(h->height > 1 && "only support non-leaf hta for now");
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;
    //---------------------------------------------------
    //Compute the shape of the HTA for partial reduce results
    //---------------------------------------------------
    ASSERT(h->height > 1);
    Tuple tsa[h->height-1];
    Tuple flat_size;
    tsa[0] = h->tiling;
    _set_reduce_ts(h->tiles[0], &tsa[1], dim_reduc);
    flat_size = h->flat_size;
    flat_size.values[dim_reduc] = h->tiling.values[dim_reduc];
    //---------------------------------------------------
    // Storage allocation
    //---------------------------------------------------
    Dist dist = h->dist;
    HTA *ret = HTA_create_impl(h->pid, NULL, h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, tsa);
    HTA_init_all_scalars(ret, initval);

    return ret;
}

void HTA_sequential_partial_reduce(ReduceOp r_op, HTA* h, HTA* r, int dim_reduc, void* initval)
{
    ASSERT(h->height == r->height);
    // Iterator each leaf tiles of h and reduce them
    if(h->height > 1) {
        Tuple h_iter[h->height-1];
        Tuple_iterator_begin(h->dim, h->height-1, h_iter);
        do {
            HTA *h_leaf = HTA_iterator_to_hta(h, h_iter);
            HTA *r_leaf = _map_to_reduced_tile(h, h_iter, r, dim_reduc);
            _partial_reduce_scalar(r_op, h_leaf, r_leaf, dim_reduc, initval);
        } while(Tuple_iterator_next(&h->tiling, h_iter));
    }
    else {
        _partial_reduce_scalar(r_op, h, r, dim_reduc, initval);
    }
}

void HTA_merge_partial_reduce_results(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA** ha1, void * initval)
{
    int i;

    // map the tiles to the result array
    //Tuple root_size = Tuple_clone_one(r->tiling);
    // FIXME: work around clone one height problem
    Tuple root_size = r->tiling;
    root_size.height = 1;
    //Tuple_overwrite_values(root_size, r->tiling);

    ASSERT(root_size.values[dim_reduc] == 1);
    Tuple root_iter;
    Tuple_init_zero(&root_iter, h->dim);
    Tuple orig_root_size = h->tiling;

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
            HTA_init_all_scalars(r_leaf, initval);
	    for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
	    {
		selected.values[dim_reduc] = i;
		int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                HTA* s_leaf = HTA_iterator_to_hta(ha1[s_idx], leaf_iter);
                _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
            }
	  } while(Tuple_iterator_next(&tree->tiling, leaf_iter));
	}
	else { // It's a leaf tile
	  HTA* r_leaf = r->tiles[r_idx];
          HTA_init_all_scalars(r_leaf, initval);
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

void HTA_merge_partial_reduce_results_spmd(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA *ht, void * initval)
{
    int i;

    // map the tiles to the result array
    //Tuple root_size = Tuple_clone_one(r->tiling);
    // FIXME: work around clone one height problem
    Tuple root_size = r->tiling;
    root_size.height = 1;
    //Tuple_overwrite_values(root_size, r->tiling);

    ASSERT(root_size.values[dim_reduc] == 1);
    Tuple root_iter;
    Tuple_init_zero(&root_iter, h->dim);
    Tuple orig_root_size = h->tiling;

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
		if(r_leaf->home == r_leaf->pid) { // owner r_leaf
		  HTA_init_all_scalars(r_leaf, initval);
		  for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
		  {
                    selected.values[dim_reduc] = i;
                    int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                    HTA* s_leaf = HTA_iterator_to_hta(ht->tiles[s_idx], leaf_iter);
		    if(s_leaf->pid != -1 && s_leaf->pid != s_leaf->home) { // receive s_leaf
		      size_t sz = HTA_get_scalar_size(s_leaf);
		      gpp_t buf;
		      int s_num_elems = s_leaf->leaf.num_elem;
		      pil_alloc(&buf, sz * s_num_elems);
		      comm_recv(s_leaf->pid, buf, s_leaf->home, sz * s_num_elems, 0);
		      s_leaf->leaf.raw = buf.ptr;
		      _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		      pil_free(buf);
		      //printf(" (%d) received leaf from (%d)\n", s_leaf->pid, s_leaf->home);
		      //printf(" (%d) received %d elems\n", s_leaf->pid, s_num_elems);
		    }
		    else { // owner r_leaf and s_leaf
		      _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		    }
		  }
		}
		else { // not owner r_leaf
		  for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
		  {
                    selected.values[dim_reduc] = i;
                    int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                    HTA* s_leaf = HTA_iterator_to_hta(ht->tiles[s_idx], leaf_iter);
		    if(s_leaf->pid == s_leaf->home) { // send s_leaf
		      void* s_leaf_ptr = HTA_get_ptr_raw_data(s_leaf);
		      size_t sz = HTA_get_scalar_size(s_leaf);
		      gpp_t buf;
		      int s_num_elems = s_leaf->leaf.num_elem;
		      pil_alloc(&buf, sz * s_num_elems);
		      memcpy(buf.ptr, s_leaf_ptr, s_num_elems * sz);
		      comm_send(s_leaf->pid, buf, r_leaf->home, sz * s_num_elems, 0);
		      pil_free(buf);
		      //printf(" (%d) sent leaf to (%d)\n", s_leaf->pid, r_leaf->home);
		      //printf(" (%d) sent %d elems\n", s_leaf->pid, s_num_elems);
		    }
		    else {
		      // do nothing
		    }
		  }
		}
              } while(Tuple_iterator_next(&tree->tiling, leaf_iter));
	}
	else { // It's a leaf tile
	    HTA* r_leaf = r->tiles[r_idx];
            if(r_leaf->pid == -1 || (r_leaf->pid != -1 && r_leaf->home == r_leaf->pid)) { // owner r_leaf
                HTA_init_all_scalars(r_leaf, initval);
                for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
                {
                  selected.values[dim_reduc] = i;
                  int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                  HTA* s_leaf = ht->tiles[s_idx];
		  if(s_leaf->pid != -1 && s_leaf->pid != s_leaf->home) { // receive s_leaf
		    size_t sz = HTA_get_scalar_size(s_leaf);
		    gpp_t buf;
		    int s_num_elems = s_leaf->leaf.num_elem;
		    pil_alloc(&buf, sz * s_num_elems);
		    comm_recv(s_leaf->pid, buf, s_leaf->home, sz * s_num_elems, 0);
		    s_leaf->leaf.raw = buf.ptr;
		    _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		    pil_free(buf);
		    //printf(" (%d) received leaf from (%d)\n", s_leaf->pid, s_leaf->home);
		    //printf(" (%d) received %d elems\n", s_leaf->pid, s_num_elems);
		  }
		  else { // owner r_leaf and s_leaf
		    _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		  }
                }
            }
            else { // not owner r_leaf
	      for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
                {
                  selected.values[dim_reduc] = i;
                  int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                  HTA* s_leaf = ht->tiles[s_idx];
		  if(s_leaf->pid == s_leaf->home) { // send s_leaf
		    void* s_leaf_ptr = HTA_get_ptr_raw_data(s_leaf);
		    size_t sz = HTA_get_scalar_size(s_leaf);
		    gpp_t buf;
		    int s_num_elems = s_leaf->leaf.num_elem;
		    pil_alloc(&buf, sz * s_num_elems);
		    memcpy(buf.ptr, s_leaf_ptr, s_num_elems * sz);
		    comm_send(s_leaf->pid, buf, r_leaf->home, sz * s_num_elems, 0);
		    pil_free(buf);
		    //printf(" (%d) sent leaf to (%d)\n", s_leaf->pid, r_leaf->home);
		    //printf(" (%d) sent %d elems\n", s_leaf->pid, s_num_elems);
		  }
		  else {
		    // do nothing
		  }
                }
	    }
	}
    } while(Tuple_iterator_next(&root_size, &root_iter));
}
void HTA_reduce_two_tiles(ReduceOp op, HTA* dest, HTA* src)
{
    _reduce_two_tiles(op, dest, src);
}

