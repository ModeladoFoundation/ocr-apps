#ifndef NEKBONE_BLAS3_H
#define NEKBONE_BLAS3_H

#include "blas.h"

//===== Assorted index hashing ================================================
unsigned long hash_columnMajor3(int if_onebased_array_use1,
                               unsigned long Ni, unsigned long Nj, unsigned long Nk,
                               unsigned long i, unsigned long j, unsigned long k);

void dehash_columnMajor3(int if_onebased_array_use1,
                        unsigned long Ni, unsigned long Nj, unsigned long Nk,
                        unsigned long h, unsigned long * o_i,
                        unsigned long * o_j, unsigned long * o_k);

//===== 2D matrices ===========================================================
typedef struct nbb_matrix2 {
    BLAS_UINT_TYPE sz_rows; //The number of rows
    BLAS_UINT_TYPE sz_cols; //The number of columns
    BLAS_REAL_TYPE * m;  //Use the accessor functions at() and get() to access an entry
} nbb_matrix2_t;

unsigned long nbb_byte_sizeof2(nbb_matrix2_t in_m); //Returns the number of byte needed to
                                                    // store the entire matrix for given sz_rows & sz_cols.
                                                    //So set sz_rows & sz_cols before use.

BLAS_REAL_TYPE  nbb_get2(nbb_matrix2_t in_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col);
BLAS_REAL_TYPE * nbb_at2(nbb_matrix2_t io_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col); //Returns address to value
                                                                                      //Returns NULL upon failure.

int nbb_transpose2(nbb_matrix2_t * o_a, nbb_matrix2_t in_b);    // A = B'  //Returns zero upon success
int nbb_mxm2(nbb_matrix2_t in_a, nbb_matrix2_t in_b, nbb_matrix2_t * o_c); //Calculate C= A*B  //Returns zero upon success

//===== 3D matrices ===========================================================
typedef struct nbb_matrix3 {
    BLAS_UINT_TYPE sz_rows; //The number of rows
    BLAS_UINT_TYPE sz_cols; //The number of columns
    BLAS_UINT_TYPE sz_depth; //The number of rows-columns 2D matrices
    BLAS_REAL_TYPE * m;  //Use the accessor functions at() and get() to access an entry
} nbb_matrix3_t;

unsigned long nbb_byte_sizeof3(nbb_matrix3_t in_m); //Returns the number of byte needed to
                                                    // store the entire tensor for given sz_rows,sz_cols & sz_depth.
                                                    //So set sz_rows,sz_cols & sz_depth before use.

BLAS_REAL_TYPE  nbb_get3(nbb_matrix3_t in_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col, BLAS_UINT_TYPE in_depth);
BLAS_REAL_TYPE * nbb_at3(nbb_matrix3_t io_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col, BLAS_UINT_TYPE in_depth); //Returns address to value
                                                                                                                     //Returns NULL upon failure.


#endif // NEKBONE_BLAS3_H
