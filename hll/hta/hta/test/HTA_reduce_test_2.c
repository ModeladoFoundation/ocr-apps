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

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    double M[MATRIX_SIZE];
    int i;
    double result = 1.0;
    double check = 1.0;

    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Dist dist;
    // create an empty shell
    HTA *h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
            2, Tuple_create(2, 2, 2), Tuple_create(2, 2, 2));

    srand(time(NULL));
    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = 2.0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);
    HTA_full_reduce(REDUCE_PRODUCT, &result, h);

    // dump HTA data
    // HTA_to_array_int32_t(h, M);

    for(i = 0; i < MATRIX_SIZE; i++) {
        check *= M[i];
    }

    printf("Check result\n");
    printf("result = %lf\n", result);
    printf("check = %lf\n", check);
    if(result == check)
        printf("** result matches! **\n");
    else {
        printf("** result does not match! **\n");
        exit(ERR_UNMATCH);
    }

    HTA_destroy(h);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
