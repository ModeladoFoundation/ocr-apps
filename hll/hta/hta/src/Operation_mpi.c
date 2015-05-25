#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"
#include "HTA.h"
#include "HTA_operations.h"


extern int _is_distributed;
extern int _my_rank;
extern int _num_processes;

//#define DEVDEBUG (1)

// Function prototypes
HTA* HTA_allocate_partial_reduce_storage_distributed(HTA *h, int dim_reduc, void *initval);
void HTA_to_array_distributed(HTA *h, void* array);
void HTA_merge_partial_reduce_results_distributed(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA* ha1, void * initval);
void HTA_get_leaf_nd_elem_offset(HTA *h, Tuple *tiling_iter, Tuple *nd_elem_offset);
void _cast_MPI_buffer(HTA_SCALAR_TYPE stype, void* result, void* buf);
MPI_Datatype _get_MPI_Datatype(HTA_SCALAR_TYPE t);
MPI_Datatype _get_MPI_reduce_op(ReduceOp fr_op);

void HTA_partial_reduce_with_preallocated_pil(ReduceOp fr_op, HTA * h1, int dim_reduc,
					      void* s1, HTA** ha, HTA* h2)
{
    // Dummy function
    assert(0 && "Not implemented in the MPI version");
}

// this function computes the offset of the first element in the leaf tile
void HTA_get_leaf_nd_elem_offset(HTA *h, Tuple *tiling_iter, Tuple *nd_elem_offset) {
    Tuple leaf_local;
    Tuple_init_zero(&leaf_local, h->dim);
    HTA_leaf_local_to_global_nd_index(h, tiling_iter, &leaf_local, nd_elem_offset);
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

void HTA_reduce_two_tiles(ReduceOp op, HTA* dest, HTA* src)
{
    _reduce_two_tiles(op, dest, src);
}

//#include "Operation_util.h"
void HTA_map_h1(int level, H1Op h1op, HTA *h1)
{
  ASSERT(h1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h1op(h1);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  // Assume each 1st level tile is distributed to a single process
  do{
         HTA* tile = HTA_pick_one_tile(h1, iter);
         int owner = tile->home;
#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner is %d\n", owner);
#endif
	if(HTA_get_rank() == owner) { // iterate through the leaf tiles
          if(tile->height > 1) { // It's not a leaf tile
	    int subtree_height = HTA_LEAF_LEVEL(h1);
	    Tuple leaf_iter[subtree_height];
	    HTA* leaf = HTA_iterator_begin(tile, leaf_iter);
	    do {
		h1op(leaf);
	    } while((leaf = HTA_iterator_next(tile, leaf_iter)));
	  }
	 else h1op(tile); // It's a leaf tile
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}

void HTA_map_h2(int level, H2Op h2op, HTA *h1, HTA *h2)
{

  ASSERT(h1 && h2);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h2op(h1, h2);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 1;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];
#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif

  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
        int owner_dest = tile_dest->home;
	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);

	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h2op(leaf_dest, leaf_src[0]);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	   if(HTA_get_rank() == owner_dest) h2op(tile_dest, tile_src[0]);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}
void HTA_map_h3(int level, H3Op h3op, HTA *h1, HTA *h2, HTA *h3)
{

  ASSERT(h1 && h2 && h3);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h3op(h1, h2, h3);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 2;

  HTA *tile_dest, *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];
#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif
  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2
  // Assume Owner-Computes rule
  do{
        tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif

	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
#if 0
	    if(HTA_get_rank() == owner_dest) {
	      if(HTA_get_rank() != owner_src[i]) {
 		buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
 		// Receive src tile
		MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
 		leaf_src[i]->leaf.raw = buffer[i];
 	      }
 	    }
	    if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
	      // Send src tile
 	      MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	    }
#endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h3op(leaf_dest, leaf_src[0], leaf_src[1]);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // It's a leaf tile
	  if(HTA_get_rank() == owner_dest) h3op(tile_dest, tile_src[0], tile_src[1]);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));
}

void HTA_map_h4(int level, H4Op h4op, HTA *h1, HTA *h2, HTA *h3, HTA *h4)
{

  ASSERT(h1 && h2 && h3 && h4);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h4op(h1, h2, h3, h4);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 3;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif


  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2, h4 src3
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
	tile_src[2] = HTA_pick_one_tile(h4, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h4op(leaf_dest, leaf_src[0], leaf_src[1], leaf_src[2]);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);

	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	  if(HTA_get_rank() == owner_dest) h4op(tile_dest, tile_src[0], tile_src[1], tile_src[2]);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}
void HTA_map_h5(int level, H5Op h5op, HTA *h1, HTA *h2, HTA *h3, HTA *h4, HTA *h5)
{

  ASSERT(h1 && h2 && h3 && h4 && h5);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h5op(h1, h2, h3, h4, h5);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 4;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif

  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2, h4 src3, h5 src4
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
	tile_src[2] = HTA_pick_one_tile(h4, iter);
	tile_src[3] = HTA_pick_one_tile(h4, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h5op(leaf_dest, leaf_src[0], leaf_src[1], leaf_src[2], leaf_src[3]);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);

	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	  if(HTA_get_rank() == owner_dest)
	    h5op(tile_dest, tile_src[0], tile_src[1], tile_src[2], tile_src[3]);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}
void HTA_map_h1s1(int level, H1S1Op h1s1op, HTA *h1, void *s1)
{

  ASSERT(h1 && s1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h1s1op(h1, s1);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  // Assume each 1st level tile is distributed to a single process
  do{
         HTA* tile = HTA_pick_one_tile(h1, iter);
         int owner = tile->home;
#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner is %d\n", owner);
#endif
	if(HTA_get_rank() == owner) { // iterate through the leaf tiles
	  if(tile->height > 1) { // It's not a leaf tile
	    int subtree_height = HTA_LEAF_LEVEL(h1);
	    Tuple leaf_iter[subtree_height];
	    HTA* leaf = HTA_iterator_begin(tile, leaf_iter);
	    do {
		  h1s1op(leaf, s1);
	    } while((leaf = HTA_iterator_next(tile, leaf_iter)));
	  }
	  else { // it's a leaf tile
	    if(HTA_get_rank() == owner) h1s1op(tile, s1);
	  }
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}
void HTA_map_h2s1(int level, H2S1Op h2s1op, HTA *h1, HTA *h2, void *s1)
{

  ASSERT(h1 && h2 && s1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
        h2s1op(h1, h2, s1);
        return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 1;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif

  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
        int owner_dest = tile_dest->home;
	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);

	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h2s1op(leaf_dest, leaf_src[0], s1);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else{ // it's a leaf tile
	  if(HTA_get_rank() == owner_dest)
	    h2s1op(tile_dest, tile_src[0], s1);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));
}
void HTA_map_h3s1(int level, H3S1Op h3s1op, HTA *h1, HTA *h2, HTA *h3, void *s1)
{

  ASSERT(h1 && h2 && h3 && s1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
      h3s1op(h1, h2, h3, s1);
      return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 2;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif


  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h3s1op(leaf_dest, leaf_src[0], leaf_src[1], s1);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	  if(HTA_get_rank() == owner_dest)
	    h3s1op(tile_dest, tile_src[0], tile_src[1], s1);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}
void HTA_map_h4s1(int level, H4S1Op h4s1op, HTA *h1, HTA *h2, HTA *h3, HTA *h4, void *s1)
{

  ASSERT(h1 && h2 && h3 && h4 && s1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
      h4s1op(h1, h2, h3, h4, s1);
      return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 3;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif

  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2, h4 src3
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
	tile_src[2] = HTA_pick_one_tile(h4, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h4s1op(leaf_dest, leaf_src[0], leaf_src[1], leaf_src[2], s1);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	   if(HTA_get_rank() == owner_dest)
	     h4s1op(tile_dest, tile_src[0], tile_src[1], tile_src[2], s1);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));


}
void HTA_map_h5s1(int level, H5S1Op h5s1op, HTA *h1, HTA *h2, HTA *h3, HTA *h4, HTA *h5, void *s1)
{

  ASSERT(h1 && h2 && h3 && h4 && h5 && s1);
  ASSERT(level <= HTA_LEAF_LEVEL(h1) && "Mapped level is limited to less than leaf level");

  if(level == 0) { // map at one tile only
      h5s1op(h1, h2, h3, h4, h5, s1);
      return;
  }

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int num_src_hta = 4;

  HTA *tile_src[num_src_hta], *leaf_src[num_src_hta];
  int i, owner_src[num_src_hta];

#if 0
  void *buffer[num_src_hta];
  size_t size_of_scalar = HTA_size_of_scalar_type(h1->scalar_type);
  MPI_Status status;
#endif

  // Assume each 1st level tile is distributed to a single process
  // h1 dest, h2 src1, h3 src2, h4 src3, h5 src4
  // Assume Owner-Computes rule
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
	tile_src[0] = HTA_pick_one_tile(h2, iter);
	tile_src[1] = HTA_pick_one_tile(h3, iter);
	tile_src[2] = HTA_pick_one_tile(h4, iter);
	tile_src[3] = HTA_pick_one_tile(h4, iter);
        int owner_dest = tile_dest->home;

	for(i=0; i<num_src_hta; i++) {
	  owner_src[i] = tile_src[i]->home;
	  ASSERT(owner_src[i] == owner_dest && "owner of the tiles in map are different");
	  (void) owner_src[i]; // Trick to avoid warning of variable set but not used
	}

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif
	if(tile_dest->height > 1) { // It's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {

	    for(i=0; i<num_src_hta; i++) { // Iterate through the src tiles
	      leaf_src[i] = HTA_iterator_to_hta(tile_src[i], leaf_iter);
  #if 0
	      if(HTA_get_rank() == owner_dest) {
		if(HTA_get_rank() != owner_src[i]) {
		  buffer[i] = malloc(size_of_scalar * leaf_src[i]->leaf.num_elem);
		  // Receive src tile
		  MPI_Recv(buffer[i], leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_src[i], 0, MPI_COMM_WORLD, &status);
		  leaf_src[i]->leaf.raw = buffer[i];
		}
	      }
	      if(HTA_get_rank() == owner_src[i] && HTA_get_rank() != owner_dest) {
		// Send src tile
		MPI_Send(leaf_src[i]->leaf.raw, leaf_src[i]->leaf.num_elem, _get_MPI_Datatype(leaf_src[i]->scalar_type), owner_dest, 0, MPI_COMM_WORLD);
	      }
  #endif
	    }
	    if(HTA_get_rank() == owner_dest) { // All tiles available
	      h5s1op(leaf_dest, leaf_src[0], leaf_src[1], leaf_src[2], leaf_src[3], s1);
	      //for(i=0; i<num_src_hta; i++) if(HTA_get_rank() != owner_src[i]) free(buffer[i]);
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	  if(HTA_get_rank() == owner_dest)
	    h5s1op(tile_dest, tile_src[0], tile_src[1], tile_src[2], tile_src[3], s1);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

}

//// ==========================================================================
//// NON MAP FUNCTIONS START HERE
//// ==========================================================================

//// HTA_tile_to_hta
//// goes down the hierarchiy of h2 to the specific level and
//// for all tiles t in that level, map the custom operator op(t, h3)
//// in parallel
void HTA_tile_to_hta(int level, H3Op h3op, HTA * h1, HTA * h2, HTA * h3)
{
  assert( 0 && "Not supported in the MPI implementation");
}

static void _get_tuple_target(Tuple *target, Tuple *nd_idx, Tuple *tiling, Tuple *direction)
{
    ASSERT(nd_idx->dim == tiling->dim && tiling->dim == direction->dim);

    // FIXME: this function will not work for irregular HTAs

    int i, j, dim;
    Tuple num_tiles_dim;
    Tuple nd_offset, dest;

    dim = tiling->dim;
    Tuple_init_zero(&num_tiles_dim, dim);
    Tuple_init_zero(&nd_offset, dim);

    Tuple_overwrite_values(&num_tiles_dim, tiling);

    for(i = 1; i < tiling->height; i++) {
      for(j = 0; j < dim; j++) {
	num_tiles_dim.values[j] *= tiling->values[j]; // Be careful with overflow
      }
    }

    Tuple_get_tile_start_offset(&num_tiles_dim, tiling, nd_idx, &nd_offset);

    Tuple temp;
    Tuple_init_zero(&dest, dim);
    Tuple_init_zero(&temp, dim);

    // Calculate target tile
    for(i = 0; i < dim; i++){
        int size = num_tiles_dim.values[i];
        int dir = direction->values[i] % size;
        dir = (dir < 0) ? (size + dir) : dir; // make dir be positive
        dest.values[i] = (nd_offset.values[i] + dir) % size;
    }

     for(i = 0; i < dim; i++) temp.values[i] = num_tiles_dim.values[i];

     Tuple *tt2;
     // Convert target to adequate nd_index sequence
     for(i = 0; i < tiling->height; i++) {
        Tuple *tt;
	tt = &tiling[i];
	tt2 = &target[i];
	tt2->height = tiling->height - i;
	tt2->dim = dim;
	for(j = 0; j < dim; j++){
	  temp.values[j] = temp.values[j]/tt->values[j];
	  tt2->values[j] = dest.values[j]/temp.values[j];
	  dest.values[j] = dest.values[j] % temp.values[j];
	}
    }
}

void HTA_circshift(HTA *h, Tuple *dir)
{
    ASSERT(h->dim == dir->dim);

    if(h->height == 1) // a leaf tile
        return;

    int dim = h->dim;
    int my_rank;
    void *buffer;
    size_t size_of_scalar = HTA_size_of_scalar_type(h->scalar_type);
    MPI_Status status;

    my_rank = HTA_get_rank();

    // Temporary HTA
    Dist dist;
    Dist_init(&dist, 0);
    Tuple tsa[h->height-1];
    Tuple *ts = tsa;

    if(h->height > 1) {
        // FIXME: this will not work for irregular HTAs
        HTA* cur_h = h;
        Tuple* t = ts;
        while(cur_h->height > 1) {
            *t = cur_h->tiling;
            t->height = cur_h->height - 1;
            cur_h = cur_h->tiles[0];
            t++;
        }
    }

    Tuple flat_size = h->flat_size;
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;
    HTA *temp_hta = HTA_create_with_ts(NULL, dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, ts);

    Tuple iter[h->height-1];
    Tuple target[h->height-1];
    HTA *leaf_src = HTA_iterator_begin(h, iter);
    do {
        _get_tuple_target(target, iter, ts, dir); // get the nd index of shift target position
 	HTA *leaf_temp_dest = HTA_iterator_to_hta(temp_hta, target);
 	int owner_src = leaf_src->home;
 	int owner_dest_temp = leaf_temp_dest->home;
 	if(owner_src == my_rank) {
 	  if(owner_dest_temp == my_rank) {
	    // Copy to temporary HTA
#ifdef DEVDEBUG
	    printf("Copying tile...\n");
#endif
	    memcpy(leaf_temp_dest->leaf.raw, leaf_src->leaf.raw, size_of_scalar * leaf_src->leaf.num_elem);
	  }
	  else {
	    // Send tile
#ifdef DEVDEBUG
	    printf("(%d) Sending tile to %d....\n", my_rank, owner_dest_temp);
#endif
	    MPI_Send(leaf_src->leaf.raw, leaf_src->leaf.num_elem, _get_MPI_Datatype(leaf_src->scalar_type), owner_dest_temp, 0, MPI_COMM_WORLD);
	  }
 	}
	else if(owner_dest_temp == my_rank) {
	  // Receive tile
#ifdef DEVDEBUG
	  printf("(%d) Receiving tile from %d....\n", my_rank, owner_src);
#endif
	  buffer = malloc(size_of_scalar * leaf_src->leaf.num_elem);
	  MPI_Recv(buffer, leaf_src->leaf.num_elem, _get_MPI_Datatype(leaf_src->scalar_type), owner_src, 0, MPI_COMM_WORLD, &status);
	  memcpy(leaf_temp_dest->leaf.raw, buffer, size_of_scalar * leaf_src->leaf.num_elem);
	  free(buffer);
	}
    } while((leaf_src = HTA_iterator_next(h, iter)));

    // Final copy
    HTA_map_h2(HTA_LEAF_LEVEL(h), H2_COPY, h, temp_hta);

}

// Assume s1's type is the same as h1->scalar_type
void HTA_full_reduce(ReduceOp fr_op, void* s1, HTA * h1)
{

  ASSERT(h1 && s1);

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  int i, num_elements = 0;
  double result[1];

  // Assume each 1st level tile is distributed to a single process
  do{
        HTA* tile_dest = HTA_pick_one_tile(h1, iter);
        int owner_dest = tile_dest->home;

#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner_dest is %d\n", owner_dest);
#endif

	if(tile_dest->height > 1) { // it's not a leaf tile
	  int subtree_height = HTA_LEAF_LEVEL(h1);
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf_dest = HTA_iterator_begin(tile_dest, leaf_iter);
	  do {
	    if(HTA_get_rank() == owner_dest) {
	      num_elements = leaf_dest->leaf.num_elem;
	      void *ptr = HTA_get_ptr_raw_data(leaf_dest);
	      for(i = 0; i < num_elements; i++) {
		fr_op(leaf_dest->scalar_type, s1, ptr + HTA_get_scalar_size(leaf_dest) * i);
	      }
	    }
	  } while((leaf_dest = HTA_iterator_next(tile_dest, leaf_iter)));
	}
	else { // it's a leaf tile
	  if(HTA_get_rank() == owner_dest) {
	      num_elements = tile_dest->leaf.num_elem;
	      void *ptr = HTA_get_ptr_raw_data(tile_dest);
	      for(i = 0; i < num_elements; i++) {
		fr_op(tile_dest->scalar_type, s1, ptr + HTA_get_scalar_size(tile_dest) * i);
	      }
	    }
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

  // MPI Allreduce
  MPI_Allreduce(s1, result, 1, _get_MPI_Datatype(h1->scalar_type), _get_MPI_reduce_op(fr_op),
		MPI_COMM_WORLD);

  _cast_MPI_buffer(h1->scalar_type, s1, result);

}

// HTA HTA_partial_reduce(ReduceOp scalar_op, H2Op hta_op, HTA h, int dim_reduc, void* initval)
void HTA_partial_reduce_pil(ReduceOp fr_op, HTA * h1, int dim_reduc, void* s1, HTA **ph2)
{

  int dim = h1->dim;
  Tuple iter[1];
  Tuple_init_zero(iter, dim);
  Tuple top_tiling = h1->tiling;
  top_tiling.height = 1;

  HTA *h2 = HTA_allocate_partial_reduce_storage_distributed(h1, dim_reduc, s1);

  // Assume each 1st level tile is distributed to a single process
  do{
         HTA* tile = HTA_pick_one_tile(h1, iter);
	 HTA* tile_dest = HTA_pick_one_tile(h2, iter);
         int owner = tile->home;
	 int owner_dest = tile_dest->home;
	 ASSERT(owner == owner_dest && "Source and dest tiles belong to different proc.");
	 (void) owner_dest; // Trick to avoid warning of variable set but not used
#if DEVDEBUG
        HTA_master_printf("Top level tile: ");
        if(HTA_get_rank() == 0) Tuple_print(iter);
        HTA_master_printf("\t owner is %d\n", owner);
#endif
	if(HTA_get_rank() == owner) {
	  HTA_sequential_partial_reduce(fr_op, tile, tile_dest, dim_reduc, s1);
	}
  } while(Tuple_iterator_next(&top_tiling, iter));

  HTA *h3 = HTA_allocate_partial_reduce_storage(h1, dim_reduc, s1);
  HTA_merge_partial_reduce_results_distributed(fr_op, h3, h1, dim_reduc, h2, s1);

  *ph2 = h3;

}

void HTA_merge_partial_reduce_results_distributed(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA* ha1, void * initval)
{
    int i;

    void *buffer = NULL;
    size_t size_of_scalar = HTA_size_of_scalar_type(h->scalar_type);
    MPI_Status status;

    // map the tiles to the result array
    //Tuple root_size = Tuple_clone_one(r->tiling);
    // FIXME: work around clone one height problem
    Tuple root_size = r->tiling;
    root_size.height = 1;
    //Tuple_overwrite_values(root_size, r->tiling);

    ASSERT(root_size.values[dim_reduc] == 1);
    Tuple root_iter;
    Tuple_init_zero(&root_iter, h->dim);
    Tuple orig_root_size = h->tiling; // Modified to merge

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
	    int r_leaf_owner = r_leaf->home;
	    for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
	      {
		selected.values[dim_reduc] = i;
		int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
                HTA* s_leaf = HTA_iterator_to_hta(ha1->tiles[s_idx], leaf_iter);
		int s_leaf_owner = s_leaf->home;
		if(HTA_get_rank() == r_leaf_owner) {
		  if(HTA_get_rank() != s_leaf_owner) {
		    buffer = malloc(size_of_scalar * s_leaf->leaf.num_elem);
		    // Receive src tile
		    MPI_Recv(buffer, s_leaf->leaf.num_elem, _get_MPI_Datatype(s_leaf->scalar_type),
			     s_leaf_owner, 0, MPI_COMM_WORLD, &status);
		    s_leaf->leaf.raw = buffer;
		  }
		  // Both tiles are available locally
		  _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		  if(HTA_get_rank() != s_leaf_owner) free(buffer);
		}
		if(HTA_get_rank() == s_leaf_owner && HTA_get_rank() != r_leaf_owner) {
		  // Send src tile
		  MPI_Send(s_leaf->leaf.raw, s_leaf->leaf.num_elem, _get_MPI_Datatype(s_leaf->scalar_type),
			   r_leaf_owner, 0, MPI_COMM_WORLD);
		}
            }
	  } while(Tuple_iterator_next(&tree->tiling, leaf_iter));
	}
	else { // It's a leaf tile
	  HTA* r_leaf = r->tiles[r_idx];
	  int r_leaf_owner = r_leaf->home;
	  for(i = 0; i < orig_root_size.values[dim_reduc]; i++)
	    {
	      selected.values[dim_reduc] = i;
	      int s_idx = Tuple_nd_to_1d_index(&selected, &orig_root_size);
              HTA* s_leaf = ha1->tiles[s_idx];
	      int s_leaf_owner = s_leaf->home;
	      if(HTA_get_rank() == r_leaf_owner) {
		  if(HTA_get_rank() != s_leaf_owner) {
		    buffer = malloc(size_of_scalar * s_leaf->leaf.num_elem);
		    // Receive src tile
		    MPI_Recv(buffer, s_leaf->leaf.num_elem, _get_MPI_Datatype(s_leaf->scalar_type),
			     s_leaf_owner, 0, MPI_COMM_WORLD, &status);
		    s_leaf->leaf.raw = buffer;
		  }
		  // Both tiles are available locally
		  _reduce_two_tiles(scalar_op, r_leaf, s_leaf);
		  if(HTA_get_rank() != s_leaf_owner) free(buffer);
	      }
	      if(HTA_get_rank() == s_leaf_owner && HTA_get_rank() != r_leaf_owner) {
		  // Send src tile
		  MPI_Send(s_leaf->leaf.raw, s_leaf->leaf.num_elem, _get_MPI_Datatype(s_leaf->scalar_type),
			   r_leaf_owner, 0, MPI_COMM_WORLD);
	      }
	    }
	}
    } while(Tuple_iterator_next(&root_size, &root_iter));

}

HTA* HTA_allocate_partial_reduce_storage_distributed(HTA *h, int dim_reduc, void *initval)
{
    ASSERT(h && initval);
    HTA_SCALAR_TYPE scalar_type = h->scalar_type;
    Dist dist;
    Dist_init(&dist, 0);
    Tuple tsa[h->height-1];
    Tuple *ts = tsa;
    Tuple flat_size;
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
	// New for the distributed case
	Tuple *t2 = &h->tiling;
	ts[0].values[dim_reduc] = t2[0].values[dim_reduc];
	flat_size = h->flat_size;
	flat_size.values[dim_reduc] = t2[0].values[dim_reduc];
    }
    else // hc is a leaf
    {
        ts = NULL;
    }

    HTA *ret = HTA_create_with_ts(NULL, h->dim, h->height, &flat_size, 0, &dist, scalar_type, h->height-1, ts);
    HTA_init_all_scalars(ret, initval);

    return ret;
}


void HTA_reduce_h2(ReduceOp fr_op, H2S1Op h2s1op, void *s1, HTA * h1, HTA * h2)
{
  assert(0 && "Not supported in MPI implementation");
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
        if(leaf->home == LOCAL) {
            void *ptr = HTA_get_ptr_raw_data(leaf);
            ASSERT(ptr);
            memcpy(((char*)ptr) + leaf_offset*unit_size, ((char*)array) + mat_offset*unit_size, unit_size);
        }
    } while(Tuple_iterator_next(&h->flat_size, &global_nd_idx));
}

static void _copy_from_buffer_to_array(void* buffer, Tuple* leaf_flat_size, Tuple* offset, Tuple* global_flat_size, void* array, size_t size_of_element) {
    int dim = leaf_flat_size->dim;
    Tuple iter[1];
    Tuple_iterator_begin(dim, 1, iter);
    // iterate through all elements
    // FIXME: possible to speed up by copying contiguous chunks at the least significant dimension
    do {
        int buffer_offset = Tuple_nd_to_1d_index(iter, leaf_flat_size);
        Tuple global_nd_idx = *offset;
        Tuple_add_dimensions(&global_nd_idx, iter);
        int array_offset = Tuple_nd_to_1d_index(&global_nd_idx, global_flat_size);
        memcpy(array + (array_offset*size_of_element), buffer + (buffer_offset * size_of_element), size_of_element);
    } while(Tuple_iterator_next(leaf_flat_size, iter));
}

// mapping HTA_SCALAR_TYPE to MPI types
MPI_Datatype _get_MPI_Datatype(HTA_SCALAR_TYPE t) {
    switch(t) {
        case HTA_SCALAR_TYPE_DCOMPLEX:
            return MPI_DOUBLE_COMPLEX;
        case HTA_SCALAR_TYPE_DOUBLE:
            return MPI_DOUBLE;
        case HTA_SCALAR_TYPE_FLOAT:
            return MPI_FLOAT;
        case HTA_SCALAR_TYPE_INT32:
            return MPI_INT;
        case HTA_SCALAR_TYPE_INT64:
            return MPI_LONG_LONG;
        case HTA_SCALAR_TYPE_UINT32:
            return MPI_UNSIGNED;
        case HTA_SCALAR_TYPE_UINT64:
            return MPI_UNSIGNED_LONG_LONG;
        default:
            assert(0 && "UNKNOWN HTA scalar type");
    }
}

MPI_Datatype _get_MPI_reduce_op(ReduceOp fr_op)
{
  if(fr_op == REDUCE_SUM)
    return MPI_SUM;
  else if(fr_op == REDUCE_PRODUCT)
    return MPI_PROD;
  else if(fr_op == REDUCE_MAX)
    return MPI_MAX;
  else assert(0 && "Reduce function not supported");
}

#define CAST_MPI_BUFFER_CASE(type) \
{ \
    type *r = (type*) result; \
    type d = *((type*) buf); \
    *r = d; \
}

void _cast_MPI_buffer(HTA_SCALAR_TYPE stype, void* result, void* buf)
{
    ASSERT(result && buf);

    switch(stype)
    {
        case(HTA_SCALAR_TYPE_INT32):
            CAST_MPI_BUFFER_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            CAST_MPI_BUFFER_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            CAST_MPI_BUFFER_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            CAST_MPI_BUFFER_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            CAST_MPI_BUFFER_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            CAST_MPI_BUFFER_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

void HTA_to_array_distributed(HTA *h, void* array) {
    ASSERT(array);
    ASSERT(h);

    int dim = h->dim;
    size_t size_of_scalar = HTA_size_of_scalar_type(h->scalar_type);

    Tuple iter[1];
    Tuple_init_zero(iter, dim);
    Tuple top_tiling = h->tiling;
    top_tiling.height = 1;
    // Assume each 1st level tile is distributed to a single process
    do{
        Tuple nd_element_offset;
        Tuple_init_zero(&nd_element_offset, dim);
        HTA* tile = HTA_pick_one_tile(h, iter);
        int owner = tile->home;
#if DEVDEBUG
        rank0_printf("Top level tile: ");
        Tuple_print(iter);
        rank0_printf("\t owner is %d\n", owner);
#endif
        // Get the flattened offset of this tile (FIXME: only work for regular HTA)
        _nd_tile_to_elem_offset(iter, &h->tiling, &h->flat_size, &nd_element_offset);

        // iterate through the leaf tiles and broadcast them
	if(tile->height > 1) { // it's not a leaf tile
	  int subtree_height = h->height - 1;
	  Tuple leaf_iter[subtree_height];
	  HTA* leaf = HTA_iterator_begin(tile, leaf_iter);
	  do {
	      // Broadcast the leaf raw data
	      // FIXME: very costly to allocate the communication buffer every time
	      // FIXME: may be able to broadcast asynchronously?
	      void* buffer;
	      if(HTA_get_rank() == owner) {
		  MPI_Bcast( leaf->leaf.raw, leaf->leaf.num_elem, _get_MPI_Datatype(h->scalar_type), owner, MPI_COMM_WORLD );
		  buffer = leaf->leaf.raw;
	      } else {
		  buffer = malloc(size_of_scalar * leaf->leaf.num_elem);
		  MPI_Bcast( buffer, leaf->leaf.num_elem, _get_MPI_Datatype(h->scalar_type), owner, MPI_COMM_WORLD );
	      }

	      // offset calculation
	      Tuple offset = nd_element_offset;
	      Tuple leaf_nd_elem_offset;
	      Tuple_init_zero(&leaf_nd_elem_offset, dim);
	      HTA_get_leaf_nd_elem_offset(tile, leaf_iter, &leaf_nd_elem_offset);
	      Tuple_add_dimensions(&offset, &leaf_nd_elem_offset);

	      // data copy from leaf raw to the array
	      _copy_from_buffer_to_array(buffer, &leaf->flat_size, &offset, &h->flat_size, array, size_of_scalar);

	      // clean up
	      if(HTA_get_rank() != owner)
		  free(buffer);
	  } while((leaf = HTA_iterator_next(tile, leaf_iter)));
      }
      else { // it's a leaf tile
	// Broadcast the leaf raw data
	      // FIXME: very costly to allocate the communication buffer every time
	      // FIXME: may be able to broadcast asynchronously?
	      void* buffer;
	      if(HTA_get_rank() == owner) {
		  MPI_Bcast( tile->leaf.raw, tile->leaf.num_elem, _get_MPI_Datatype(h->scalar_type), owner, MPI_COMM_WORLD );
		  buffer = tile->leaf.raw;
	      } else {
		  buffer = malloc(size_of_scalar * tile->leaf.num_elem);
		  MPI_Bcast( buffer, tile->leaf.num_elem, _get_MPI_Datatype(h->scalar_type), owner, MPI_COMM_WORLD );
	      }

	      // offset calculation
	      Tuple offset = nd_element_offset;
	      Tuple leaf_nd_elem_offset;
	      Tuple_init_zero(&leaf_nd_elem_offset, dim);
	      Tuple leaf_iter;
	      Tuple_init_zero(&leaf_iter, dim);
	      Tuple_1d_to_nd_index(tile->rank, &top_tiling, &leaf_iter);

	      HTA_get_leaf_nd_elem_offset(tile, &leaf_iter, &leaf_nd_elem_offset);
	      Tuple_add_dimensions(&offset, &leaf_nd_elem_offset);

	      // data copy from leaf raw to the array
	      _copy_from_buffer_to_array(buffer, &tile->flat_size, &offset, &h->flat_size, array, size_of_scalar);

	      // clean up
	      if(HTA_get_rank() != owner)
		  free(buffer);
      }
    } while(Tuple_iterator_next(&top_tiling, iter));
}

int main(int argc, char** argv)
{
    //========================================
    // MPI environment initialization
    //========================================
    int rc = MPI_Init(&argc, &argv);
    if(rc != MPI_SUCCESS) {
       printf("MPI initialization failed!\n");
       MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &_my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_num_processes);
    _is_distributed = 1;
#ifdef DEBUG
    printf("This process is rank %d\n", HTA_get_rank());
    if(_my_rank == 0)
        printf("Total number of processes is %d\n", HTA_get_num_processes());
#endif

    hta_init();
    hta_main(argc, argv);

    MPI_Finalize();
}

void HTA_exit(int status) {
    if(HTA_is_distributed())
        MPI_Finalize();
    exit(status);
}

