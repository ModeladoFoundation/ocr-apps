#ifndef __CSR_H__
#define __CSR_H__
void CSR_read_rcs_file(const char *filename, int* num_rows, int* num_cols, int* nnz, double** val_ptr, int** col_ind, int** row_ptr);
void CSR_read_dense_file(const char *filename, int* num_rows, int* num_cols, int* nnz, double* val_ptr, int* col_ind, int* row_ptr);
void CSR_print_matrix(int num_rows, int num_cols, double* val, int* col_ind, int* row_ptr);
void CSR_print_rcs_format(int n, int nnz, double* val, int* col_ind, int* row_ptr);
void CSR_get_value(int row, int col, void *val, int* col_ind, int* row_ptr, void* data, size_t data_size);
void* CSR_get_ptr_to_value(int row, int col, void *val, int* col_ind, int* row_ptr, size_t data_size);
#endif

