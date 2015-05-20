#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*4+13)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)

int hta_main(int argc, char** argv, int pid)
{
    uint32_t M[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    uint32_t R2[MATRIX_SIZE];
    int i, j, err;

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
    HTA *result = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        M[i] = i % 1000;
        R1[i] = 0;
        R2[i] = 0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(result, R1);

    HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, result, h, h);
    //printf("the content of h after pointwise multiplication\n");
    //HTA_dump(h);

    HTA_flatten(R1, NULL, NULL, result);

    // do a direct computation of pointwise multplication on M
    for(i = 0; i < MATRIX_WIDTH; i++)
       for (j = 0; j < MATRIX_WIDTH; j++)
       {
           R2[i*MATRIX_WIDTH+j] = M[i*MATRIX_WIDTH+j] * M[i*MATRIX_WIDTH+j];
       }

    // verify the result
    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(R1));
    if(memcmp(R1, R2, sizeof(R1)) == 0) {
        printf("thread(%d) map_h3 all results match!\n", pid);
        err = SUCCESS;
    }
    else{
        printf("thread(%d) map_h3 results mismatch!\n", pid);
        err = ERR_UNMATCH;
    }
    HTA_destroy(h);
    HTA_destroy(result);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
