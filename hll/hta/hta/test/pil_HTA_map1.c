#include <stdio.h>
#include "Debug.h"
#include "HTA.h"

#define MATRIX_SIZE (3*3*5*5*4*4)

void increment(HTA dest, HTA src)
{
    int i;
    int cur_level = dest->level;
    int num_scalars = HTA_tile_size(cur_level, dest->tiling);
    DBG(0, "increment(): At level %d, add %d scalars\n", cur_level, num_scalars);
    // Get pointer to the start of leaf tile
    int32_t* dest_ptr = (int32_t*) Alloc_get_block_ptr(dest->mmap->map_blocks[dest->linear_idx]);
    int32_t* src_ptr = (int32_t*) Alloc_get_block_ptr(src->mmap->map_blocks[src->linear_idx]);
    for(i = 0; i < num_scalars; i++)
    {
        dest_ptr[i] = src_ptr[i] + 1;
    }
}

int hta_main(int argc, char** argv)
{
    int32_t M[MATRIX_SIZE];
    int i;

    TupleSeq ts;
    Tuple t0 = Tuple_create(2, 3, 3, END_ARGS);
    Tuple t1 = Tuple_create(2, 5, 5, END_ARGS);
    Tuple t2 = Tuple_create(2, 4, 4, END_ARGS);
    ts = TupleSeq_create(3, t0, t1, t2, END_ARGS);

    // create an empty shell
    HTA h = HTA_create(2, 3, ts, NULL, NULL, HTA_SCALAR_TYPE_INT32);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++)
        M[i] = i;

    // initialize the HTA using 2D matrix
    HTA new_h = HTA_assign_array_int32_t(h, M);

    // print the content of HTA
    printf("the content of new_h\n");
    HTA_dump(new_h);

    HTA_map1(LEAF_LEVEL(ts), increment, h, new_h);
    printf("the content of h after increment\n");
    HTA_dump(h);

    HTA_destroy(new_h);
    TupleSeq_destroy(ts);

    return 0;
}
