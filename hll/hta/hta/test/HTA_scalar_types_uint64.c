#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "HTA.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (3*3*5*5*4*4)

int hta_main(int argc, char** argv, int pid)
{
    uint64_t M[MATRIX_SIZE];
    uint64_t G[MATRIX_SIZE];
    int i, err;
    int cmp_result;

    Tuple t0 = Tuple_create(2, 3, 3);
    Tuple t1 = Tuple_create(2, 5, 5);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // create an empty shell
    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    HTA *h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT64,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        uint64_t val = i%100;
        M[i] = val;
	G[i] = val*val;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);

    // print the content of HTA
    HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, h, h, h);
    HTA_flatten(M, NULL, NULL, h);

    printf("comparing %zd bytes\n", sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0) {
        printf("** result matches! **\n");
        err = SUCCESS;
    }
    else {
        printf("** result does not match! **\n");
        err = SUCCESS;
    }

    HTA_destroy(h);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
