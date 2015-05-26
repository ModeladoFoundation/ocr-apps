#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HTA.h"
#include "Comm.h"
#include "Tuple.h"
#include "Distribution.h"
#include "RefCount.h"
#include "CSR.h"
#include "HTA_operations.h"
#include "test.h"

#define L1_WIDTH (3)
#define L2_WIDTH (2)
#define L3_WIDTH (4)
#define MATRIX_WIDTH (16)

double *val;
int *col_ind;
int *row_ptr;
int num_rows, num_cols;
int nz;

void mat_print(int width, double m[])
{
    int i, j;
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < width; j++)
            printf("%8.2lf ", m[i * width + j]);
        printf("\n");
    }
}

int hta_main(int argc, char** argv, int pid)
{
    Tuple flat_size;
    Tuple_init(&flat_size, 2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh = HTA_get_vp_mesh(2);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA *hs = HTA_sparse_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));

    // Create sparse matrix in CSR format
    if(pid == 0 || pid == -1) {
        CSR_read_rcs_file("HTA_sparse_test.rcs", &num_rows, &num_cols, &nz, &val, &col_ind, &row_ptr);
        ASSERT(val);
        ASSERT(row_ptr);
        ASSERT(col_ind);
        CSR_print_matrix(num_rows, num_cols, val, col_ind, row_ptr);
        CSR_print_rcs_format(num_rows, nz, val, col_ind, row_ptr);
    }
    HTA_barrier(pid);
    // initialize sparse HTA with the sparse matrix
    HTA_init_with_sparse_matrix(hs, nz, num_rows, val, col_ind, row_ptr);
    HTA_make_shared_all_leaves(hs); // this has to be explicit since it's not done by the init function
    // make_shared_all_leaves is a barrier function, it's ok to free
    if(pid == 0 || pid == -1) {
        free(val);
        free(row_ptr);
        free(col_ind);
    }

    // initialize dense matrix
    HTA *hd = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));
    HTA *hr = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE,
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));
    double A[MATRIX_WIDTH*MATRIX_WIDTH];
    double R[MATRIX_WIDTH*MATRIX_WIDTH];
    for(int i = 0; i < MATRIX_WIDTH; i++)
    {
        for(int j = 0; j < MATRIX_WIDTH; j++)
        {
            A[i * MATRIX_WIDTH + j] = 2.0;
            R[i * MATRIX_WIDTH + j] = 0.0;
        }
    }

    HTA_init_with_array(hd, A);
    HTA_init_with_array(hr, R);
    // perform computation
    HTA_map_h3(HTA_LEAF_LEVEL(hd), H3_SDPWMUL, hr, hs, hd);
    HTA_flatten(R, NULL, NULL, hr);
    if(pid == 0 || pid == -1) {
        printf("Result after pointwise multiplication:\n");
        mat_print(MATRIX_WIDTH, R);
    }

    HTA_destroy(hs);
    HTA_destroy(hd);
    HTA_destroy(hr);

    return 0;
}
