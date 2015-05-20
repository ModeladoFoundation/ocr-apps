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

// h1 += h2 * scalar
void muladd(HTA* h1, HTA* h2, void* scalar) {
    int32_t val = *((int32_t*)scalar);
    int32_t *d1 = (int32_t*) HTA_get_ptr_raw_data(h1);
    int32_t *d2 = (int32_t*) HTA_get_ptr_raw_data(h2);
    for(int i = 0; i < 4*4; i++)
    {
	*d1 += *d2 * val;
        d1++;
        d2++;
    }
}

int hta_main(int argc, char** argv, int pid)
{
    int i, err;
    int32_t M1[MATRIX_SIZE];
    int32_t M2[MATRIX_SIZE];
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
    HTA *h1 = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);
    HTA *h2 = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M1[i] = i%100;
        M2[i] = M1[i];
    }
    // Here, h1 == h2

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h1, M1);
    HTA_init_with_array(h2, M2);

    // keep sum of h1
    HTA_full_reduce(REDUCE_SUM, &result, h1);

    int k = 10;
    // h1 = h1 + h2 * 10
    HTA_map_h2s1(HTA_LEAF_LEVEL(h1), muladd, h1, h2, &k);

    // Here, h1 == 11 * h2
    HTA_full_reduce(REDUCE_SUM, &check, h1);

    printf("Check result\n");
    printf("result = %d\n", result);
    printf("check = %d\n", check);

    if((result * 11 == check)) {
        printf("** VERIFIED! **\n");
        err = SUCCESS;
    }
    else {
        printf("** INCORRECT! **\n");
        err = ERR_UNMATCH;
    }


    HTA_destroy(h1);
    HTA_destroy(h2);

    exit(err);
    return 0;
}
