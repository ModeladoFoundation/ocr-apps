#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "test.h"
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)


int hta_main(int argc, char** argv, int pid)
{
    int32_t M[MATRIX_SIZE];
    int i, err;
    int32_t result = 0;
    int32_t check = 0;

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

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = i%100;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);

    // sum
    HTA_full_reduce(REDUCE_SUM, &result, h);

    int neg = -1;
    HTA_map_h1s1(HTA_LEAF_LEVEL(h), H1S1_MUL, h, &neg);

    int negresult = 0;
    HTA_full_reduce(REDUCE_SUM, &negresult, h);

    for(i = 0; i < MATRIX_SIZE; i++) {
        check += M[i];
    }

    printf("Check result\n");
    printf("result = %d\n", result);
    printf("check = %d\n", check);
    printf("negresult = %d\n", negresult);

    if((result == check) && (-result == negresult)) {
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
