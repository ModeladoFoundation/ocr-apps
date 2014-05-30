#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "HTA.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"

#define TILE_WIDTH   (2)
#define TILE_SIZE    (TILE_WIDTH*TILE_WIDTH)
#define MATRIX_WIDTH (16)
#define MATRIX_SIZE  (MATRIX_WIDTH*MATRIX_WIDTH)

void matmul_direct(int width, uint64_t c[], uint64_t a[], uint64_t b[])
{
    int i, j, k;
    for(i = 0; i < width; i++)
        for(j = 0; j < width; j++)
            for(k = 0; k < width; k++)
                c[i*width + j] += a[i*width + k] * b[k*width + j];
}

void mat_print(int width, uint64_t m[])
{
    int i, j;
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < width; j++)
            printf("%lu ", m[i * width + j]);
        printf("\n");
    }
}



#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    uint64_t A[MATRIX_SIZE], B[MATRIX_SIZE], C[MATRIX_SIZE];
    uint64_t G[MATRIX_SIZE];
    uint64_t T[TILE_SIZE];
    int i, j, k;
    int cmp_result;
    Tuple t0 = Tuple_create(2, MATRIX_WIDTH/TILE_WIDTH, MATRIX_WIDTH/TILE_WIDTH);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);
    Tuple flat_size_2 = Tuple_create(2, TILE_WIDTH, TILE_WIDTH);

    // create an empty shell
    Dist dist;
    HTA *a = HTA_create_with_ts(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64, 
            1, &t0);
    HTA *b = HTA_create_with_ts(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64, 
            1, &t0);
    HTA *c = HTA_create_with_ts(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64, 
            1, &t0);
    // tmp is a place to store temporary value
    // There is no tiling for this 1 level HTA
    HTA *tmp = HTA_create(2, 1, &flat_size_2, 0, &dist, HTA_SCALAR_TYPE_UINT64, 0);

    // create a 2D matrix
    srand(time(NULL));

    for(i = 0; i < MATRIX_SIZE; i++)
    {
        A[i] = rand() % 100;
        B[i] = rand() % 100;
        C[i] = 0;
        G[i] = 0;
    }

    for(i = 0; i < TILE_SIZE; i++)
    {
        T[i] = 0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(a, A);
    HTA_init_with_array(b, B);
    HTA_init_with_array(c, C);

    printf("The content of matrix A\n");
    mat_print(MATRIX_WIDTH, A);
    printf("The content of matrix B\n");
    mat_print(MATRIX_WIDTH, B);

    for(i = 0; i < MATRIX_WIDTH/TILE_WIDTH; i++)
        for(j = 0; j < MATRIX_WIDTH/TILE_WIDTH; j++)
            for(k = 0; k < MATRIX_WIDTH/TILE_WIDTH; k++)
            {
                Tuple i_k = Tuple_create(2, i, k);
                Tuple k_j = Tuple_create(2, k, j);
                Tuple i_j = Tuple_create(2, i, j);
                HTA *a_i_k = HTA_pick_one_tile(a, &i_k);
                HTA *b_k_j = HTA_pick_one_tile(b, &k_j);
                HTA *c_i_j = HTA_pick_one_tile(c, &i_j);

                // clear tmp HTA data
                HTA_init_with_array(tmp, T);
                // tile matrix multiplication a[i][k] * b[k][j]
                printf("== block based matrix multiplication : tmp = a[%d][%d] * b[%d][%d]\n", i, k, k, j);
                HTA_map_h3(0, H3_MATMUL, tmp, a_i_k, b_k_j);
                //printf("== block based matrix addition : c[i][j] += tmp\n");
                HTA_map_h3(0, H3_PWADD, c_i_j, c_i_j, tmp); 
            }
    // store the data from HTA to 1D array
    HTA_to_array(c, C);
    printf("The result of matmul on HTA\n");
    mat_print(MATRIX_WIDTH, C);

    matmul_direct(MATRIX_WIDTH, G, A, B);
    printf("The result of matmul_direct\n");
    mat_print(MATRIX_WIDTH, G);

    printf("Comparing matrix content %lu bytes\n", sizeof(C));
    cmp_result = memcmp(C, G, sizeof(C));
    if(cmp_result == 0)
        printf("** result matches! **\n");
    else {
        printf("** result does not match! **\n");
        exit(ERR_UNMATCH);
    }

    HTA_destroy(a);
    HTA_destroy(b);
    HTA_destroy(c);
    HTA_destroy(tmp);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
