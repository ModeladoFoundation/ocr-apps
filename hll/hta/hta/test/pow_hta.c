#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "Distribution.h"
#include "timer.h"
#include "test.h"
#include "Comm.h"

#define POW(x, n) \
{ \
    double y = x; \
    x = 1.0; \
    for(int j = 0; j < n; j++) \
        x *= y; \
}

int EXP = 20;

void dummy(HTA* h1, HTA* h2) {
    // the dummy function does not do anything
    return;
}

void power(HTA *h1, HTA* h2) {
    double* data1 = h1->leaf.raw;
    double* data2 = h2->leaf.raw;
    int num_elem = Tuple_product(&h1->flat_size);
    for(int i = 0; i < num_elem; i++)
    {
        double x = data2[i];
        POW(x, EXP);
        data1[i] = x;
    }
}

int hta_main(int argc, char** argv, int pid)
{
    int np = 4;                 // np is the parallelism available
    int num_iters = 10;         // number of iterations wanted
    int matrix_width = 1024;
    int matrix_size;
    int i, j, k;
#ifdef VERIFY
    int err;
#endif
    double *M, *R1, *R2;

    SET_SPMD_PID(pid)

    if(argc > 1)
        np = atoi(argv[1]);
    if(argc > 2)
        num_iters = atoi(argv[2]);
    if(argc > 3)
        EXP = atoi(argv[3]);
    if(argc > 4)
        matrix_width = atoi(argv[4]);
    matrix_size = matrix_width*matrix_width;

    MASTER_EXEC_START
    printf("Parallelism         : %d\n", np);
    printf("Number of iterations: %d\n", num_iters);
    printf("Matrix size         : %d x %d\n", matrix_width, matrix_width);
    MASTER_EXEC_END

    Tuple mesh = HTA_get_vp_mesh(2);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    Tuple flat_size = Tuple_create(2, matrix_width, matrix_width);
    M =  (double*) malloc(sizeof(double) * matrix_size);
    R1 = (double*) malloc(sizeof(double) * matrix_size);
    R2 = (double*) malloc(sizeof(double) * matrix_size);

    // ---------------------------
    // Initialization
    // ---------------------------

    Tuple t0 = Tuple_create(2, np, 1);
    // create an empty shell, 1D decomposition in rows
    HTA *h = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            1, t0);
    HTA *result = HTA_create_with_pid(pid, 2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            1, t0);

    // create a 2D matrix of random numbers
    for(i = 0; i < matrix_size; i++)
    {
        M[i] = i * 1000.0;
        R1[i] = 0.0;
        R2[i] = 0.0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs
    HTA_init_with_array(result, R1);

    // ---------------------------
    // Parallel execution
    // ---------------------------

    // A dummy call
#ifdef DUMMY
    HTA_map_h2(HTA_LEAF_LEVEL(h), dummy, result, h);
#else
    HTA_map_h2(HTA_LEAF_LEVEL(h), power, result, h);
#endif
    // timed iterations
    MASTER_EXEC_START
    hta_timer_start(0);
    MASTER_EXEC_END
    for(k = 0; k < num_iters; k++) {
#ifdef DUMMY
        HTA_map_h2(HTA_LEAF_LEVEL(h), dummy, result, h);
#else
        HTA_map_h2(HTA_LEAF_LEVEL(h), power, result, h);
#endif
    }
    MASTER_EXEC_START
    hta_timer_stop(0);
    MASTER_EXEC_END

    // ---------------------------
    // Result verification
    // ---------------------------
#ifdef VERIFY
    HTA_flatten(R1, NULL, NULL, result);

    // do a direct computation of pointwise multplication on M
    for(k = 0; k < num_iters; k++) {
        for(i = 0; i < matrix_width; i++)
           for (j = 0; j < matrix_width; j++) {
               double x = M[i*matrix_width+j];
               POW(x, EXP);
               R2[i*matrix_width+j] = x;
           }
    }

    // verify the result
    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(double) * matrix_size);
    if(memcmp(R1, R2, sizeof(double) * matrix_size) == 0) {
        printf("thread (%d) all results match!\n", pid);
        err = SUCCESS;
    }
    else{
        printf("thread(%d) incorrect result detected!\n", pid);
        err = ERR_UNMATCH;
    }
#endif
    MASTER_EXEC_START
    // report timing
    double elapsed_time = timer_get_sec(0);
    printf("%.4lf seconds\n", elapsed_time);
    MASTER_EXEC_END

    HTA_destroy(h);
    HTA_destroy(result);

    free(M);
    free(R1);
    free(R2);

#ifdef VERIFY
    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
#endif
    return 0;
}
