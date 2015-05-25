#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "test.h"
#define MATRIX_WIDTH (2*2*1)
#define MATRIX_SIZE (2*2*2*2*1*1)

int hta_main(int argc, char** argv, int pid)
{
    int i, err;
    double M[MATRIX_SIZE];
    double result = 1.0;
    double check = 1.0;

    Tuple t0 = Tuple_create(2, 2, 2);
    Tuple t1 = Tuple_create(2, 2, 2);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA *h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = 2.0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);
    HTA_full_reduce(REDUCE_PRODUCT, &result, h);


    for(i = 0; i < MATRIX_SIZE; i++) {
        check *= M[i];
    }

    if(result == check) {
        printf("** result matches! **\n");
        err = SUCCESS;
    }
    else {
        printf("** result does not match! **\n");
        err = ERR_UNMATCH;
    }

    HTA_destroy(h);

    exit(err);
    return 0;
}
