#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)


#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    uint32_t M[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    uint32_t R2[MATRIX_SIZE];
    int i, j;

    Dist dist;
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // create an empty shell
    HTA *h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));
    HTA *result = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, 3, 3), Tuple_create(2, 5, 5));
    
    // create a 2D matrix
    srand(time(NULL)); /* FIXME: using random to initialize distributed model will fail */
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        M[i] = rand() % 1000;
        R1[i] = 0;
        R2[i] = 0; 
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(result, R1);

    HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, result, h, h);
    //printf("the content of h after pointwise multiplication\n");
    //HTA_dump(h);

    HTA_to_array(result, R1); // copy from HTA memory to process memory

    // do a direct computation of pointwise multplication on M
    for(i = 0; i < MATRIX_WIDTH; i++)
       for (j = 0; j < MATRIX_WIDTH; j++)
           R2[i*MATRIX_WIDTH+j] = M[i*MATRIX_WIDTH+j] * M[i*MATRIX_WIDTH+j];

    // verify the result
    printf("comparing %zd bytes\n", sizeof(R1));
    if(memcmp(R1, R2, sizeof(R1)) == 0)
        printf("all results match!\n");
    else{
        printf("incorrect result detected!\n");
        exit(ERR_UNMATCH);
    }

    HTA_destroy(h);
    HTA_destroy(result);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
