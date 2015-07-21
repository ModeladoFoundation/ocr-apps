/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

#ifdef BLAS_TYPE__single
#define square  1320      // Must be a multiple of four
#else
#define square  900       // Must be a multiple of four
#endif
#define numRows square
#define numCols square
ocrGuid_t NAME(test_gesv)() {
    printf ("Executing %s\n",STRINGIFY(NAME(test_getrf))); fflush(stdout);
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    static BLAS_MATH_t  sampleA[4][4] = {{  1.80,  2.88,  2.05, -0.89},
                                         {  5.25, -2.95, -0.95, -3.80},
                                         {  1.58, -2.69, -2.90, -1.04},
                                         { -1.11, -0.66, -0.59,  0.80}};
    static BLAS_MATH_t  sampleB[4][2] = {{  9.52, 18.47},
                                         { 24.35,  2.25},
                                         {  0.77,-13.28},
                                         { -6.22, -6.21}};
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    static BLAS_MATH_t  sampleA[4][4] = {{{ -1.34,  2.55},{  0.28,  3.17},{ -6.39, -2.20},{  0.72, -0.92}},
                                         {{ -0.17, -1.41},{  3.31, -0.15},{ -0.15,  1.34},{  1.29,  1.38}},
                                         {{ -3.29, -2.39},{ -1.91,  4.42},{ -0.14, -1.35},{  1.72,  1.35}},
                                         {{  2.41,  0.39},{ -0.56,  1.47},{ -0.83, -0.69},{ -1.96,  0.67}}};
    static BLAS_MATH_t  sampleB[4][2] = {{{ 26.26, 51.78},{ 31.32, -6.70}},
                                         {{  6.43, -8.68},{ 15.86, -1.42}},
                                         {{ -5.75, 25.31},{ -2.15, 30.19}},
                                         {{  1.16,  2.57},{ -2.56,  7.55}}};
#else
**** ERROR, undefined type
#endif
    static BLAS_MATH_t  matA[numRows][numCols];
    static BLAS_MATH_t  matB[numRows][3];
    static BLAS_int_t   pivotIdx[MAX(numRows, numCols)];
    static BLAS_int_t   info;

    u64 i,j,q;
    BLAS_int_t lda = numCols;
    BLAS_MATH_t zero;

    for (i = 0; i < numRows; i++) {
        for (j = 0; j < numCols; j++) {
            xSETc(matA[i][j],0.0,0.0);
        }
    }

    for (q = 0; q < square; q+=4) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                matA[q+i][q+j] = sampleA[i][j];
            }
            matB[q+i][0] = sampleB[i][0];
            matB[q+i][1] = sampleB[i][1];
        }
    }

    printf ("Calling %s standard API\n", STRINGIFY(NAME(gesv))); fflush(stdout);
    NAME(gesv) (
        numRows,    // const BLAS_int_t    n,        // Input:  The number of linear equations, i.e. the order of matrix A.
        2,          // const BLAS_int_t    nrhs,     // Input:  Number of right hand sides, aka the number of columns in matrix B.
        &matA[0][0],// const BLAS_MATH_t * a,        // Input:  The n*n coefficient matrix.
                                                     // Output: The factors L and U from the factorization A+P*L*U.  The unit
                                                     //         diagonal elements of L are not stored.
        lda,        // const BLAS_int_t    lda,      // Input:  The leading dimension (aka "pitch") of matrix a.
        pivotIdx,   // const BLAS_int_t  * pivotIdx, // Output: The pivot indices that define the permutation matrix P;  row i of
                                                     //         the matrix was interchanged with row pivotIdx[i].  Note: contents
                                                     //         of the array are one-based, owing to the Fortran history.  They
                                                     //         must be adjusted (decremented) to index into C's zero-based matrix.
        &matB[0][0],// const BLAS_MATH_t * b,        // Input:  right hand side n-by-nhrs matrix b.
                                                     // Output: if info=0, the n-by-nrhs solution of matrix X.
        3,          // const BLAS_int_t    ldb,      // Input:  The leading dimension (aka "pitch") of matrix b.
        &info);     // const BLAS_int_t  * info);    // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                                     //         > 0 first one-based index along diagonal of U that is exactly zero.
                                                     //         The factorization has been performed, but the factor U is exactly
                                                     //         singular so the solution could not be computed.
    printf ("test_gesv done.\n");

    return NULL_GUID;
}

#undef square

