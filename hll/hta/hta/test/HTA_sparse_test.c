#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HTA.h"
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

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    //Tuple t0 = Tuple_create(2, L1_WIDTH, L1_WIDTH);
    //Tuple t1 = Tuple_create(2, L2_WIDTH, L2_WIDTH);
    //Tuple ts = Tuple_concat(t0, t1);

    Tuple flat_size;
    Tuple_init(&flat_size, 2, MATRIX_WIDTH, MATRIX_WIDTH);

    Dist dist;
    // create an empty shell
    HTA *hs = HTA_sparse_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));

    // Create sparse matrix in CSR format
    double *val;
    int *col_ind;
    int *row_ptr;
    int num_rows, num_cols;
    int nz;
    CSR_read_rcs_file("HTA_sparse_test.rcs", &num_rows, &num_cols, &nz, &val, &col_ind, &row_ptr);
    CSR_print_matrix(num_rows, num_cols, val, col_ind, row_ptr);
    CSR_print_rcs_format(num_rows, nz, val, col_ind, row_ptr);

    // initialize sparse HTA with the sparse matrix
    HTA_init_with_sparse_matrix(hs, nz, num_rows, val, col_ind, row_ptr);

    free(val);
    free(row_ptr);
    free(col_ind);

    // initialize dense matrix
    //Tuple_retain_all(flat_size);
    //Tuple_retain_all(ts);
    //HTA hd = HTA_create(2, 3, flat_size, ts, 0, dist, HTA_SCALAR_TYPE_DOUBLE); // Dense HTA operand
    HTA *hd = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));
    //Tuple_retain_all(flat_size);
    //Tuple_retain_all(ts);
    //HTA hr = HTA_create(2, 3, flat_size, ts, 0, dist, HTA_SCALAR_TYPE_DOUBLE); // for results
    HTA *hr = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
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

    HTA_to_array(hr, R);
    printf("Result after pointwise multiplication:\n");
    mat_print(MATRIX_WIDTH, R);    

    HTA_destroy(hs);
    HTA_destroy(hd);
    HTA_destroy(hr);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
