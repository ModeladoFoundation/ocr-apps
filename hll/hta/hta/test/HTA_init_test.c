#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "Comm.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)

void mat_print(int width, uint32_t m[])
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
    uint32_t M[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    int i, err;

    Tuple t0 = Tuple_create(2, 3, 3);
    Tuple t1 = Tuple_create(2, 5, 5);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA* h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT32, 2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        M[i] = i%8 + 1;
        R1[i] = 9;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs
    HTA_flatten(R1,NULL, NULL, h); // copy from HTA memory to process memory

    // verify the result
    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(R1));
    if(memcmp(M, R1, sizeof(R1)) == 0) {
        printf("thread(%d) all results match!\n", pid);
        err = SUCCESS;
    }
    else {
        printf("thread(%d) results mismatch!\n", pid);
        err = ERR_UNMATCH;
    }


    HTA_destroy(h);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
