#include <stdio.h>
#include "CSR.h"
#define MATRIX_WIDTH (16)
#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    double val[MATRIX_WIDTH*MATRIX_WIDTH];
    int col_ind[MATRIX_WIDTH*MATRIX_WIDTH];
    int row_ptr[MATRIX_WIDTH+1];
    int num_rows, num_cols;
    int nz;
    CSR_read_dense_file("HTA_sparse_test.csr", &num_rows, &num_cols, &nz, val, col_ind, row_ptr);
    CSR_print_rcs_format(num_rows, nz, val, col_ind, row_ptr);
    return 0;
}

