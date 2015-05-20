#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "HTA.h"
#include "Comm.h"
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

int hta_main(int argc, char** argv, int pid)
{
    uint64_t A[MATRIX_SIZE], B[MATRIX_SIZE], C[MATRIX_SIZE];
    uint64_t G[MATRIX_SIZE];
    int i, err;
    int cmp_result;
    Tuple t0 = Tuple_create(2, MATRIX_WIDTH/TILE_WIDTH, MATRIX_WIDTH/TILE_WIDTH);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // create an empty shell
    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    HTA *a = HTA_create_impl(pid, NULL, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            1, &t0);
    HTA *b = HTA_create_impl(pid, NULL, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            1, &t0);
    HTA *c = HTA_create_impl(pid, NULL, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            1, &t0);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        A[i] = i % 100;
        B[i] = i % 100;
        C[i] = 0;
        G[i] = 0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(a, A);
    HTA_init_with_array(b, B);
    HTA_init_with_array(c, C);

    //printf("The content of matrix A\n");
    //mat_print(MATRIX_WIDTH, A);
    //printf("The content of matrix B\n");
    //mat_print(MATRIX_WIDTH, B);

    HTA_matmul(a, b, c);
    // store the data from HTA to 1D array
    HTA_flatten(C, NULL, NULL, c);
    //printf("The result of matmul on HTA\n");
    //mat_print(MATRIX_WIDTH, C);

    matmul_direct(MATRIX_WIDTH, G, A, B);
    //printf("The result of matmul_direct\n");
    //mat_print(MATRIX_WIDTH, G);

    printf("thread (%d) Comparing matrix content %lu bytes\n", pid, sizeof(C));
    cmp_result = memcmp(C, G, sizeof(C));
    if(cmp_result == 0) {
        printf("** thread(%d) result matches! **\n", pid);
        err = SUCCESS;
    }
    else {
        printf("** thread(%d) result does not match! **\n", pid);
        err = ERR_UNMATCH;
    }

    HTA_destroy(a);
    HTA_destroy(b);
    HTA_destroy(c);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
