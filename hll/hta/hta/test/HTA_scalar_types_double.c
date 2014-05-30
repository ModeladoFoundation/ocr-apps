#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "HTA.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (3*3*5*5*4*4)

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    double M[MATRIX_SIZE];
    double G[MATRIX_SIZE];
    int i;
    int cmp_result;

    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // create an empty shell
    Dist dist;
    //HTA h = HTA_create(2, 3, flat_size, ts, 0, dist, HTA_SCALAR_TYPE_DOUBLE);
    HTA *h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));

    // create a 2D matrix
    srand(time(NULL));
    for(i = 0; i < MATRIX_SIZE; i++) {
        double val = rand()/(double)RAND_MAX;
        M[i] = val;
	G[i] = val*val;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);

    // print the content of HTA
    HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, h, h, h);
    HTA_to_array(h, M);

    printf("comparing %zd bytes\n", sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0)
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
