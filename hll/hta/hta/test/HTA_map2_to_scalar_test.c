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

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    int32_t M1[MATRIX_SIZE];
    int32_t M2[MATRIX_SIZE];
    int32_t result = 0;
    int32_t check = 0;

    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Dist dist;
    // create an empty shell
    HTA *h1 = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));
    HTA *h2 = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));

    srand(time(NULL));
    // create a 2D matrix
    for(int i = 0; i < MATRIX_SIZE; i++) {
        M1[i] = rand()%100;
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

    if((result * 11 == check))
        printf("** VERIFIED! **\n");
    else {
        printf("** INCORRECT! **\n");
        exit(ERR_UNMATCH);
    }
    
    
    HTA_destroy(h1);
    HTA_destroy(h2);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
