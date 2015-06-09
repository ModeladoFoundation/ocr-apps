/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?POTF2,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  Computes the Cholesky factorization of a symmetric positive definite matrix A (unblocked).
 *
 * The factorization has the form
 *    A = U**T * U if UPLO = 'U',   or A = L * L**T if UPLO = 'L'
 * where U is an upper triangular matrix and L is a lower triangular matrix.
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>
#include <math.h>


void NAME(potf2) (
    char *        uplo,     // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
    BLAS_int_t    n,        // The number of rows and columns of the n-by-n matrix.
    BLAS_MATH_t * a,        // Input array, size n*lda.
    BLAS_int_t    lda,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    BLAS_int_t  * pInfo)    // Output status value.  If <0, this indicates an error condition.  If >0, this is a one-based index of a zero coefficient.  If 0, completely normal.
{
    BLAS_int_t j;
    //printf("Entered Standard API %s function.\n", STRINGIFY(NAME(potf2))); fflush(stdout);

    *pInfo = 0;
    // Check argument validity.
    if (uplo[0] != 'U' && uplo[0] != 'u' && uplo[0] != 'L' && uplo[0] != 'l') *pInfo = -1;
    else if (n < 0) *pInfo = -2;
    else if (lda < MAX(n,1)) *pInfo = -4;
    if (*pInfo != 0) {
        xerbla (STRINGIFY(NAME(potf2)), -*pInfo);
        return;
    }
    // Quick return if possible
    if (n == 0) {
        return;
    }

    BLAS_MATH_t   minusOne, one;
    xSETc (minusOne, -1.0, 0.0);
    xSETc (one,       1.0, 0.0);

    if (uplo[0] == 'U' || uplo[0] == 'u') {  // Compute the Cholesky factorization of the upper triangle:  A = U**T*U
        for (j = 0; j < n; j++) {
            // Compute diaganol element and test for it NOT being positive definite.
            BLAS_MATH_COMPONENT_t tmp;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            tmp  = NAME(dot)(j, &a[0*lda+j], lda, &a[0*lda+j], lda);
            tmp  = a[j*lda+j] - tmp;
#else
            tmp  = NAME_COMPONENT(dot)(j, &(a[0*lda+j].real), lda*2, &(a[0*lda+j].real), lda*2);
            tmp += NAME_COMPONENT(dot)(j, &a[0*lda+j].imag, lda*2, &a[0*lda+j].imag, lda*2);
            tmp  = a[j*lda+j].real - tmp;
#endif
            if (tmp < 0.0 || NAME_COMPONENT(isnan)(tmp)) {
                xSETc(a[j*lda+j],tmp,0.0);
                *pInfo = j+1;   // Bug out, indicating one-based index of diagonal element that works out negative or NAN.
                return;
            }
            tmp = sqrt(tmp);
            xSETc(a[j*lda+j],tmp,0.0);

            // Compute elements j+1:n-1 of row j.
            if (j < n-1) {
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                NAME(lacgv)(j,&a[0*lda+j],lda);  // Conjugate the vector X.
#endif
                NAME(gemv)("Transpose", j, n-j-1, minusOne, &a[0*lda+j+1], lda, &a[0*lda+j], lda, one, &a[j*lda+j+1],1);
                tmp = 1.0 / tmp;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                NAME(lacgv)(j,&a[0*lda+j],lda);  // Conjugate the vector X back to the way it was.
                NAME_COMPONENT(scal)(n-1-j, tmp, &a[j*lda+j+1].real, 2);
                NAME_COMPONENT(scal)(n-1-j, tmp, &a[j*lda+j+1].imag, 2);
#else
                NAME(scal)(n-1-j, tmp, &a[j*lda+j+1], 1);
#endif
            }
        }
    } else {   // Compute the Cholesky factorization of the lower triangle:  A = L*L**T
        for (j = 0; j < n; j++) {
            // Compute diaganol element and test for it NOT being positive definite.
            BLAS_MATH_COMPONENT_t tmp;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            tmp  = NAME(dot)(j, &a[j*lda+0], 1, &a[j*lda+0], 1);
            tmp  = a[j*lda+j] - tmp;
#else
            tmp  = NAME_COMPONENT(dot)(j, &a[j*lda+0].real, 2, &a[j*lda+0].real, 2);
            tmp += NAME_COMPONENT(dot)(j, &a[j*lda+0].imag, 2, &a[j*lda+0].imag, 2);
            tmp  = a[j*lda+j].real - tmp;
#endif
            if (tmp < 0 || NAME_COMPONENT(isnan)(tmp)) {
                xSETc(a[j*lda+j],tmp,0.0);
                *pInfo = j+1;   // Bug out, indicating one-based index of diagonal element that works out negative or NAN.
                return;
            }
            tmp = sqrt(tmp);
            xSETc(a[j*lda+j],tmp,0.0);

            // Compute elements j+1:n-1 of row j.
            if (j < n-1) {
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                NAME(lacgv)(j,&a[j*lda+0],1);  // Conjugate the vector X.
#endif
                NAME(gemv)("No Transpose", n-j-1, j, minusOne, &a[(j+1)*lda+0], lda, &a[j*lda+0], 1, one, &a[(j+1)*lda+j],lda);
                tmp = 1.0 / tmp;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                NAME(lacgv)(j,&a[j*lda+0],1);  // Conjugate the vector X back to the way it was.
                NAME_COMPONENT(scal)(n-1-j, tmp, &a[(j+1)*lda+j].real, lda*2);
                NAME_COMPONENT(scal)(n-1-j, tmp, &a[(j+1)*lda+j].imag, lda*2);
#else
                NAME(scal)(n-1-j, tmp, &a[(j+1)*lda+j], lda);
#endif
            }
        }
    }
} // ?potf2

