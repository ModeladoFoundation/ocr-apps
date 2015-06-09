/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * @brief Implementation of BLAS ?getf2, compute the LU factorization of a general m-by-n matrix with partial pivotion with row interechange.
 * where ? is * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 * This is the unblocked algorithm.
 **/

#include "blas.h"
#include "ocr-types.h"
#include <stdio.h>

void NAME(getf2) (
    BLAS_int_t    m,            // Input:  Number of rows    in the matrix.
    BLAS_int_t    n,            // Input:  Number of columns in the matrix.
    BLAS_MATH_t * mat,          // In/Out: Matrix to factor.
    BLAS_int_t    ld,           // Input:  Leading dimension, i.e. pitch from start of one row of the matrix storage construct to start of the next, in units of element type (i.e. number of BLAS_MATH_t's).
    BLAS_int_t  * pivotIdx,     // Output: Pivot indices found.  These are one-based values!
    BLAS_int_t  * info)         // Output: Diagnostic status.  Negative means an error.  If a zero-coefficient is found, we return the index
                                //         of that row (i.e. the first such occurrence thereof) in one-based form.  If no such case, zero is returned.
{

    printf ("Entered %s.\n", STRINGIFY(NAME(getf2))); fflush(stdout);

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(getf2)),MIN(m,n) * m / 2,false)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(getf2)),MIN(m,n) * m / 2 * 4,false)
#endif

    // Validate the input parameters:

    *info = 0;
    if      (m < 0)                *info = -1; // Checking number of MATRIX rows (aka number of storage columns).
    else if (n < 0)                *info = -2; // Checking number of MATRIX columns (aka number of storage rows).
    else if (ld < (m < 1 ? 1 : m)) *info = -4; // Checking leadingDimension (aka "pitch") against number of MATRX rows (i.e. against storage columns, since C is row-major).
    if (*info != 0) {
        xerbla(STRINGIFY(NAME(getf2)), -*info);
        return;
    }

    if (m == 0 || n == 0) return; // Quick return if possible:

    static s32 increaseAccuracyOfIntermediates = -1;
    if (increaseAccuracyOfIntermediates < 0) increaseAccuracyOfIntermediates = ilaenv(1001, STRINGIFY(NAME(getf2)), " ", 0, 0, 0, 0);

    BLAS_int_t  j, jPivot, i;
    BLAS_MATH_t zero, negOne;
    xSETc(zero,    0.0, 0.0);
    xSETc(negOne, -1.0, 0.0);

    for (j = 0; j < MIN(m,n); j++) {
        // Find pivot and test for singularity.
        jPivot = j + NAME2(i,amax)(m-j, &mat[j*ld+j], ld);   // This (and the result of IAMAX) is a one-based result, because LAPACK has its roots in Fortran.
        pivotIdx[j] = jPivot;                                // Record the one-based index in the pivotIdx array.
        jPivot--;                                            // Now make it zero-based, for the remainder of its usage here in this function.
        if (xCMP_NE(mat[jPivot*ld+j], zero)) {
            if (jPivot != j) {
                NAME(swap)(n, &mat[j*ld+0], (BLAS_int_t) 1, &mat[jPivot*ld+0], (BLAS_int_t) 1);   // Apply the interchange (swap) of all columns of rows j and jPivot.
            }
            // Compute elements j:m-1 of j-th column.
            if (j < m-1) {
                if (1 || IS_SAFE_FOR_RECIP_MUL(mat[j*ld+j])) {
                    if (increaseAccuracyOfIntermediates != 0) {
                        BLAS_UPCONVERTED_MATH_t recip;
                        xRECIP (recip, mat[j*ld+j]);
                        NAME(scal_upconvertedAlpha)(m-j-1, recip, &mat[((j+1)*ld)+j], ld);
                    } else {
                        BLAS_MATH_t recip;
                        xRECIP (recip, mat[j*ld+j]);
                        NAME(scal)(m-j-1, recip, &mat[((j+1)*ld)+j], ld);
                    }
                } else {
                    for (i = j+1; i < m; i++) {
                        xDIV(mat[i*ld+j], mat[i*ld+j], mat[j*ld+j]);
                    }
                }
            }
        } else if (*info == 0) {
            *info = j + 1;  // Make this a "one-based" index because LAPACK has its roots in Fortran, which uses one-based indices for its [old-style] arrays.
        }
        if (j < MIN(m,n)-1) {
            // Update trailing submatrix.
            NAME(ger)(m-j-1, n-j-1, negOne, &mat[((j+1)*ld)+j], ld, &mat[j*ld+(j+1)], (BLAS_int_t) 1, &mat[((j+1)*ld)+(j+1)], ld);
        }
    }
} // ?getf2
