#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "test.h"
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (3*3*5*5*4*4)

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    int32_t M[MATRIX_SIZE];
    int32_t G[MATRIX_SIZE];
    int i;
    int cmp_result;

    //Tuple t[2];
    //Tuple_fill_in_sequence(t, 2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Dist dist;
    // create an empty shell
    HTA *h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));
    HTA *h2 = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));

    srand(time(NULL));
    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = rand()%1000;
	G[i] = M[i]+1;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);
    HTA_init_with_array(h2, M);

    // perform increment
    HTA_map_h2(HTA_LEAF_LEVEL(h), H2_INC, h, h);
    HTA_map_h1(HTA_LEAF_LEVEL(h2), H1_INC, h2);

    // dump HTA data
    HTA_to_array(h, M);

    printf("comparing %zd bytes\n", sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0)
        printf("** map_h2 result matches! **\n");
    else
        printf("** map_h2 result does not match! **\n");

    // dump HTA data
    HTA_to_array(h2, M);

    printf("comparing %zd bytes\n", sizeof(M));
    cmp_result = memcmp(M, G, sizeof(M));
    if(cmp_result == 0)
        printf("** map_h1 result matches! **\n");
    else{
        printf("** map_h1 result does not match! **\n");
        exit(ERR_UNMATCH);
    }
    HTA_destroy(h);
    HTA_destroy(h2);

    printf("Objects left (memory leak) %d\n", Alloc_count_objects());
    return 0;
    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
