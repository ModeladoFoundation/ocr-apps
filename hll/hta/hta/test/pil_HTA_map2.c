#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "Tuple.h"
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)

void pointwise_mul(HTA dest, HTA src1, HTA src2)
{
    int i;
    int cur_level = dest->level;
    int num_scalars = HTA_tile_size(cur_level, dest->tiling);
    // DBG(0, "pointwise_mul(): At level %d, add %d scalars\n", cur_level, num_scalars);
    // Get pointer to the start of leaf tile
    int* dest_ptr = (int*) Alloc_get_block_ptr(dest->mmap->map_blocks[dest->linear_idx]);
    int* src1_ptr = (int*) Alloc_get_block_ptr(src1->mmap->map_blocks[src1->linear_idx]);
    int* src2_ptr = (int*) Alloc_get_block_ptr(src2->mmap->map_blocks[src2->linear_idx]);
    for(i = 0; i < num_scalars; i++)
    {
        dest_ptr[i] = src1_ptr[i] * src2_ptr[i];
    }
}

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    uint32_t M[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    uint32_t R2[MATRIX_SIZE];
    int i, j;

    TupleSeq ts;
    Tuple t0 = Tuple_create(2, 3, 3, END_ARGS);
    Tuple t1 = Tuple_create(2, 5, 5, END_ARGS);
    Tuple t2 = Tuple_create(2, 4, 4, END_ARGS);
    ts = TupleSeq_create(3, t0, t1, t2, END_ARGS);

    // create an empty shell
    HTA h = HTA_create(2, 3, ts, NULL, NULL, HTA_SCALAR_TYPE_UINT32);
    HTA result = HTA_create(2, 3, ts, NULL, NULL, HTA_SCALAR_TYPE_UINT32);

    // create a 2D matrix
    srand(time(NULL));
    for(i = 0; i < MATRIX_SIZE; i++)
    {
    //    M[i] = i;
        M[i] = rand() % 1000;
        R1[i] = 0;
        R2[i] = 0;
    }
    // initialize the HTA using 2D matrix
    HTA_assign_array_uint32_t(h, M); // copy from process memory to memory allocated for HTAs
    HTA_assign_array_uint32_t(result, R1);

    // print the content of HTA
    //printf("the content of new_h\n");
    //HTA_dump(new_h);

    HTA_map2(LEAF_LEVEL(ts), pointwise_mul, result, h, h);
    //printf("the content of h after pointwise multiplication\n");
    //HTA_dump(h);

    HTA_to_array_uint32_t(result, R1); // copy from HTA memory to process memory

    // do a direct computation of pointwise multplication on M
    for(i = 0; i < MATRIX_WIDTH; i++)
       for (j = 0; j < MATRIX_WIDTH; j++)
       {
           R2[i*MATRIX_WIDTH+j] = M[i*MATRIX_WIDTH+j] * M[i*MATRIX_WIDTH+j];
       } 

    // verify the result
    if(memcmp(R1, R2, sizeof(R1)) == 0)
        printf("all results match!\n");
    else
        printf("incorrect result detected!\n");

    HTA_destroy(h);
    HTA_destroy(result);
    TupleSeq_destroy(ts);

    return 0;
}
