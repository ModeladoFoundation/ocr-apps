#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "HTA.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"

#define MATRIX_WIDTH (32)
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
    int i, j, err;
    int cmp_result;
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);
    Tuple flat_size2 = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH); // MxM

    // create an empty shell
    Tuple mesh = HTA_get_vp_mesh(1);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    HTA *a = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            1, Tuple_create(2, MATRIX_WIDTH, 1)); // (M, 1) (1, M)
    HTA *b = HTA_create_with_pid(pid, 2, 1, &flat_size2, 0, &dist, HTA_SCALAR_TYPE_UINT64, 0); // only 1 level // FIXME: not well defined in SPMD mode
    HTA *c = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            1, Tuple_create(2, MATRIX_WIDTH, 1)); // (M, 1) (1, M)

    // create a 2D matrix
    srand(time(NULL));
    for(i = 0; i < MATRIX_WIDTH; i++)
	for(j = 0; j < MATRIX_WIDTH; j++)
	{
	    uint64_t tmp;
	    tmp = rand() % 100;
	    A[i*MATRIX_WIDTH + j] = tmp;
	    C[i*MATRIX_WIDTH + j] = 0;
	    B[i*MATRIX_WIDTH + j] = rand() % 100;
	    G[i*MATRIX_WIDTH + j] = 0;
	}

    // initialize the HTA using 2D matrix
    HTA_init_with_array(a, A);
    HTA_init_with_array(b, B);
    HTA_init_with_array(c, C);
    HTA_barrier(pid);

    if(pid == -1 || pid == 0) {
        printf("The content of matrix A\n");
        mat_print(MATRIX_WIDTH, A);
        printf("The content of matrix B\n");
        mat_print(MATRIX_WIDTH, B);
    }
    // forall tile t in a, matmul(c, t, b)
    HTA_tile_to_hta(1, H3_MATMUL, c, a, b);

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
        err = SUCCESS;
        printf("** thread(%d) result matches! **\n", pid);
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
