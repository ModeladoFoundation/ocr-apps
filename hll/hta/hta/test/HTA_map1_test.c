#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Debug.h"
#include "Comm.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "test.h"
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (3*3*5*5*4*4)

void mat_print(int width, int32_t m[])
{
    int i, j;
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < width; j++)
            printf("%u ", m[i * width + j]);
        printf("\n");
    }
}
int hta_main(int argc, char** argv, int pid)
{
    int32_t M[MATRIX_SIZE];
    int32_t G[MATRIX_SIZE];
    int i, err;
    int cmp_result;

    //SET_SPMD_PID(pid)

    Tuple t0 = Tuple_create(2, 3, 3);
    Tuple t1 = Tuple_create(2, 5, 5);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA *h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);
    HTA *h2 = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = i%6+1;
	G[i] = M[i]+1;
    }

    //MASTER_EXEC_START
    //    printf("Original M\n");
    //    mat_print(MATRIX_WIDTH, M);
    //    printf("Original G\n");
    //    mat_print(MATRIX_WIDTH, G);
    //MASTER_EXEC_END

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);
    HTA_init_with_array(h2, M);

    // perform increment
    HTA_map_h2(HTA_LEAF_LEVEL(h), H2_INC, h, h); // h = h + 1;
    HTA_map_h1(HTA_LEAF_LEVEL(h2), H1_INC, h2);  // h2++;

    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = 0;
    }
    // dump HTA data
    HTA_flatten(M, NULL, NULL, h);

    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0) {
        printf("thread(%d) map_h2 all results match!\n", pid);
        err = SUCCESS;
    }
    else {
        printf("thread(%d) map_h2 results mismatch!\n", pid);
        err = ERR_UNMATCH;
    }

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = 0;
    }

    // dump HTA data
    HTA_flatten(M, NULL, NULL, h2);

    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0) {
        printf("thread(%d) map_h1 all results match!\n", pid);
        err = SUCCESS;
    }
    else {
        printf("thread(%d) map_h1 results mismatch!\n", pid);
        err = ERR_UNMATCH;
    }

    HTA_destroy(h);
    HTA_destroy(h2);

    all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
