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
#include "timer.h"

void operation(HTA* h1, HTA* h2, HTA* h3) {
    // the dummy function does not do anything
    return;
}

#if defined(PILHTA) || defined(MPI)
int hta_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
    int np = 4;                 // np is the parallelism available
    int num_iters = 100;         // number of iterations wanted
    int matrix_width = 1024;
    int matrix_size;
    int i, k;
    int *M, *R1, *R2;

    if(argc > 1) {
        np = atoi(argv[1]);
    }
    if(argc > 2) {
        num_iters = atoi(argv[2]);
    }
    if(argc > 3) {
        matrix_width = atoi(argv[3]);
    }
    matrix_size = matrix_width*matrix_width;

    printf("HTA performance test: \n");
    printf("Parallelism         : %d\n", np);
    printf("Number of iterations: %d\n", num_iters);
    printf("Matrix size         : %dx%d\n", matrix_width, matrix_width);

    Dist dist;
    Tuple flat_size = Tuple_create(2, matrix_width, matrix_width);
    M =  (int*) malloc(sizeof(int) * matrix_size);
    R1 = (int*) malloc(sizeof(int) * matrix_size);
    R2 = (int*) malloc(sizeof(int) * matrix_size);

    // ---------------------------
    // Initialization
    // ---------------------------

    // create an empty shell, 1D decomposition in rows
    HTA *h = HTA_create(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            1, Tuple_create(2, np, 1));
    HTA *result = HTA_create(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            1, Tuple_create(2, np, 1));

    // create a 2D matrix of random numbers
    srand(time(NULL));
    for(i = 0; i < matrix_size; i++)
    {
        M[i] = rand() % 1000;
        R1[i] = 0;
        R2[i] = 0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(result, R1);

    // ---------------------------
    // Parallel execution
    // ---------------------------

    // A dummy call
    HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, result, h, h);
    // timed iterations
    timer_start(0);
    for(k = 0; k < num_iters; k++) {
        HTA_map_h3(HTA_LEAF_LEVEL(h), H3_PWMUL, result, h, h);
        //HTA_map_h3(HTA_LEAF_LEVEL(h), operation, result, h, h);
    }
    timer_stop(0);

    // ---------------------------
    // Result verification
    // ---------------------------
    //HTA_to_array(result, R1); // copy from HTA memory to process memory

    //// do a direct computation of pointwise multplication on M
    //for(k = 0; k < num_iters; k++) {
    //    for(i = 0; i < matrix_width; i++)
    //       for (j = 0; j < matrix_width; j++)
    //           R2[i*matrix_width+j] = M[i*matrix_width+j] * M[i*matrix_width+j];
    //}

    //// verify the result
    //printf("comparing %zd bytes\n", sizeof(int) * matrix_size);
    //if(memcmp(R1, R2, sizeof(int) * matrix_size) == 0)
    //    printf("all results match!\n");
    //else{
    //    printf("incorrect result detected!\n");
    //    exit(ERR_UNMATCH);
    //}

    // report timing
    double elapsed_time = timer_get_sec(0);
    printf("%.4lf seconds\n", elapsed_time);

    HTA_destroy(h);
    HTA_destroy(result);

    free(M);
    free(R1);
    free(R2);

    exit(SUCCESS);
    return 0;
}
