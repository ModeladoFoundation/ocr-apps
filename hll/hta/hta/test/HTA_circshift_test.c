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

#define MATRIX_WIDTH (4*1)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)


int hta_main(int argc, char** argv, int pid)
{
    uint32_t M[MATRIX_SIZE];
    uint32_t N[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    int i, j, err;

    Tuple mesh = HTA_get_vp_mesh(2);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // create an empty shell
    HTA *h1 = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            1, Tuple_create(2, 4, 4));
    HTA *h2 = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            1, Tuple_create(2, 4, 4));
    HTA *result = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            1, Tuple_create(2, 4, 4));

    // create a 2D matrix
    for(i = 0; i < MATRIX_WIDTH; i++)
        for(j = 0; j < MATRIX_WIDTH; j++)
    {
        M[i*MATRIX_WIDTH + j] = (j%2)? -1:1;
        N[i*MATRIX_WIDTH + j] = (j%2)? -1:1;
        R1[i*MATRIX_WIDTH + j] = 0xAA;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h1, M); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(h2, N); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(result, R1);

    //printf("M before circshift\n");
    //for(i = 0; i < MATRIX_WIDTH; i++)
    //{
    //    for (j = 0; j < MATRIX_WIDTH; j++) {
    //        printf(" % d", M[i*MATRIX_WIDTH + j]);
    //    }
    //    printf("\n");
    //}
    Tuple dir = Tuple_create(2, 0, 1);
    HTA_circshift(h1, &dir);
    HTA_flatten(M, NULL, NULL, h1);
    //printf("M after circshift\n");
    //for(i = 0; i < MATRIX_WIDTH; i++)
    //{
    //    for (j = 0; j < MATRIX_WIDTH; j++) {
    //        printf(" % d", M[i*MATRIX_WIDTH + j]);
    //    }
    //    printf("\n");
    //}

    HTA_map_h3(HTA_LEAF_LEVEL(h1), H3_PWADD, result, h1, h2);

    HTA_flatten(R1, NULL, NULL, result);

    int verified = 1;
    for(i = 0; i < MATRIX_WIDTH; i++)
       for (j = 0; j < MATRIX_WIDTH; j++) {
           if(R1[i * MATRIX_WIDTH + j] != 0) {
               verified = 0;
               printf("Incorrect sum at R[%d, %d] = %d\n", i, j, R1[i*MATRIX_WIDTH + j]);
           }
       }

    // verify the result
    if(verified) {
        printf("thread(%d) circshift all results match!\n", pid);
        err = SUCCESS;
    }
    else {
        printf("thread(%d) circshift results mismatch!\n", pid);
        err = ERR_UNMATCH;
    }

    //HTA_destroy(h1); // FIXME
    HTA_destroy(h2);
    HTA_destroy(result);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
