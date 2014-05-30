#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "CSR.h"

void CSR_read_rcs_file(const char *filename, int* num_rows, int* num_cols, int* nnz, double** val_ptr, int** col_ind, int** row_ptr)
{
    assert(filename && num_rows && num_cols && nnz && val_ptr && col_ind && row_ptr);
    FILE* fp = fopen(filename, "r");
    assert(fp);

    fscanf(fp, "%d %d", num_rows, nnz);

    *col_ind = (int*)malloc(sizeof(int) * (*nnz));
    *val_ptr = (double*)malloc(sizeof(double) * (*nnz));
    *row_ptr = (int*)malloc(sizeof(int) * ((*num_rows) + 1));

    for(int i = 0; i <= *num_rows; i++) {
        fscanf(fp, "%d", (*row_ptr) + i);
    }
    for(int i = 0; i < *nnz; i++) {
        fscanf(fp, "%d", (*col_ind) + i);
    }
    for(int i = 0; i < *nnz; i++) {
        fscanf(fp, "%lf", (*val_ptr) + i);
    }
    *num_cols = *num_rows;

}

void CSR_read_dense_file(const char *filename, int* num_rows, int* num_cols, int* nnz, double* val_ptr, int* col_ind, int* row_ptr)
{
    assert(filename && nnz && val_ptr && col_ind && row_ptr);
    FILE* fp = fopen(filename, "r");
    assert(fp);

    int row, col;
    fscanf(fp, "%d %d\n", &row, &col);
    *nnz = 0;
    for(int i = 0; i < row; i++) {
        row_ptr[i] = *nnz;
        for(int j = 0; j < col; j++)
        {
            double val;
            fscanf(fp, "%lf", &val);
            if(val != 0.0)
            {
                col_ind[*nnz] = j;
                val_ptr[*nnz] = val;
                (*nnz)++;
            }
        }
    }
    row_ptr[row] = *nnz;
    *num_rows = row;
    *num_cols = col;
}

void CSR_print_matrix(int num_rows, int num_cols, double* val, int* col_ind, int* row_ptr)
{
    for(int i = 0; i < num_rows; i++)
    {
        for(int j = 0; j < num_cols; j++)
        {
            double d = 0.0;
            CSR_get_value(i, j, val, col_ind, row_ptr, &d, sizeof(double));
            printf("%.2lf ", d); 
        }
        printf("\n");
    }
}

void CSR_print_rcs_format(int n, int nnz, double* val, int* col_ind, int* row_ptr)
{
    printf("%d %d\n", n, nnz);
    for(int i = 0; i <= n; i++)
        printf("%d ", row_ptr[i]);
    printf("\n");
    for(int i = 0; i < nnz; i++)
        printf("%d ", col_ind[i]);
    printf("\n");
    for(int i = 0; i < nnz; i++)
        printf("%e ", val[i]);
    printf("\n");
}

void CSR_get_value(int row, int col, void *val, int* col_ind, int* row_ptr, void* data, size_t data_size)
{
    void *loc = CSR_get_ptr_to_value(row, col, val, col_ind, row_ptr, data_size);
    if(loc)
    {
        memcpy(data, loc, data_size);
    }
    else
    {
        // do nothing, assuming that *data is initialized to zero
    }
}

void* CSR_get_ptr_to_value(int row, int col, void *val, int* col_ind, int* row_ptr, size_t data_size)
{
    int start = row_ptr[row];
    int end = row_ptr[row+1];
    for(int i = start; i < end; i++)
    {
        if(col_ind[i] == col) // found
        {
            return (val + i*data_size);
        }
    }
    return NULL;
}
