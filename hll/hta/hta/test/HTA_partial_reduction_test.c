#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "HTA.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "Distribution.h"
#include "util.h"
#include "test.h"
#include "timer.h"

int hta_main(int argc, char** argv, int pid)
{
    int i, err = SUCCESS;
    double *M, *R;

    int dim_reduc = (argc == 1) ? 0 : atoi(argv[1]);

    Tuple t0 = Tuple_create(2, 2, 2);
    Tuple t1 = Tuple_create(2, 2, 2);
    Tuple flat_size = Tuple_create(2, 8, 8);

    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA* h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            2, t0, t1);

    // Create a 2D matrix
    int matrix_size = Tuple_product(&flat_size);
    M = (double *) malloc(matrix_size * sizeof(double));
    for(i = 0; i < matrix_size; i++) {
        M[i] = i+1;
    }
    // Initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);

    double initval = 0.0;
    // Partial reduction along dim_reduc

    int num_iters = 1;
    HTA *r;
    hta_timer_start(0);
    for(int k = 0; k < num_iters; k++) {
      r = HTA_partial_reduce(REDUCE_MAX, h, dim_reduc, &initval);
    }
    hta_timer_stop(0);

    assert(r);

    // Print the result
    matrix_size = Tuple_product(&r->flat_size);
    R = (double *) malloc(matrix_size * sizeof(double));
    HTA_flatten(R, NULL, NULL, r);

    for(i = 0; i < matrix_size; i++) printf("(%d) %lf \n", i, R[i]);

    // report timing
    double elapsed_time = timer_get_sec(0);
    printf("Time: %.4lf seconds\n", elapsed_time);

    for(i = 0; i < matrix_size; i++)
    {
        double v = (!dim_reduc) ? (56 + i + 1) : (8 * (i+1));
        if( v != R[i] ) {
            printf("incorrect result detected!\n");
            err = ERR_UNMATCH;
        }
    }

    HTA_destroy(h);
    HTA_destroy(r);

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
