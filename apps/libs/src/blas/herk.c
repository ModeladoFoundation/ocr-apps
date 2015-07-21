/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?HERK,  where ? is: <c = single precision complex | z = double precision complex>
 *
 *  Perform one of the hermitian rank k operations:
 *      C = alpha*A*A**H = beta*C
 *      C = alpha*A**H*A = beta*c
 *  where alpha and beta are real scalars, C is an n-by-n hermitian matrix, and A is n-by-k (in the first case) or k-by-n (in the second case).
 *
 **/

#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


void NAME(herk)(
    char                  * uplo,     // First char:  'U' or 'u' -- only the upper triangle of C is to be referenced;  'L' or 'l' -- only the lower part...
    char                  * trans,    // First char:  'N' or 'n' -- C=alpha*A*A**H + beta*C;  'T', 't', 'C', or 'c' -- C=alpha*A**H*A = beta*C.
    BLAS_int_t              n,        // Order of matrix C.  Must be >= 0.
    BLAS_int_t              k,        // If no transpose, number of columns in matrix A; if transpose, number of rows.  Must be >= 0.
    BLAS_MATH_COMPONENT_t   alpha,    // Scalar multiplier.
    BLAS_MATH_t           * a,        // Triangular input matrix.
    BLAS_int_t              lda,      // Leading dimension of A.
    BLAS_MATH_COMPONENT_t   beta,     // Scalar multiplier.
    BLAS_MATH_t           * c,        // Triangular input/output matrix.  (Note that on input, the imaginary components of the diagonal are assumed to be zero, and they are set to zero upon output.)
    BLAS_int_t              ldc)      // Leading dimension of C.
{
    bool isUpper;
    if (uplo[0] == 'U' || uplo[0] == 'u') {
        isUpper = true;
    } else if (uplo[0] == 'L' || uplo[0] == 'L') {
        isUpper = false;
    } else {
        xerbla(STRINGIFY(NAME(syrk)),1);
        return;
    }
    bool doTranspose;
    if (trans[0] == 'N' || trans[0] == 'n') {
        doTranspose = false;
    } else if (trans[0] == 'T' || trans[0] == 't' || trans[0] == 'C' || trans[0] == 'c') {
        doTranspose = true;
    } else {
        xerbla(STRINGIFY(NAME(syrk)),2);
        return;
    }
    if (n < 0) {
        xerbla(STRINGIFY(NAME(syrk)),3);
        return;
    }
    if (k < 0) {
        xerbla(STRINGIFY(NAME(syrk)),4);
        return;
    }
    if (lda < MAX(1,(doTranspose?n:k))) {
        xerbla(STRINGIFY(NAME(syrk)),7);
        return;
    }
    if (ldc < MAX(1,n)) {
        xerbla(STRINGIFY(NAME(syrk)),10);
        return;
    }

    BLAS_MATH_t zero, one;
    xSETc(zero, 0.0, 0.0);
    xSETc(one,  1.0, 0.0);
    if (n == 0 || ((alpha == 0.0 || k == 0) && beta == 1.0)) return;

    // Less work when alpha is zero:

    BLAS_int_t i,j,l;

    if (alpha == 0.0) {
        if (isUpper) {
            if (beta == 0.0) {
                for (j = 0; j < n; j++) {
                    for (i = 0; i <= j; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                }
            } else {
                for (j = 0; j < n; j++) {
                    for (i = 0; i < j; i++) {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                    c[j*ldc+j].real *= beta;
                    c[j*ldc+j].imag  = 0.0;
                }
            }
        } else {
            if (beta == 0.0) {
                for (j = 0; j < n; j++) {
                    for (i = j; i < n; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                }
            } else {
                for (j = 0; j < n; j++) {
                    c[j*ldc+j].real *= beta;
                    c[j*ldc+j].imag  = 0.0;
                    for (i = j + 1; i < n; i++) {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                }
            }
        }
        return;
    }

    // Start the full operation.

    if (!doTranspose) {   // Form C = alpha*A*A**H = beta*C.
        if (isUpper) {
            for (j = 0; j < n; j++) {
                if (beta == 0.0) {
                    for (i = 0; i <= j; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                } else if (beta != 1.0) {
                    for (i = 0; i <  j; i++) {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                    c[j*ldc+j].real *= beta;
                    c[j*ldc+j].imag  = 0.0;
                } else {
                    c[j*ldc+j].imag  = 0.0;
                }
                for (l = 0; l < k; l++) {
                    if (xCMP_NE(a[j*lda+l],zero)) {
                        BLAS_MATH_t temp;
                        xCONJUGATE(temp,a[j*lda+l]);
                        xMULcr(temp,temp,alpha);
                        for (i = 0; i < j; i++) {
                            xFMA(c[i*ldc+j],temp,a[i*lda+l]);
                        }
                        c[j*ldc+j].real += temp.real * a[i*lda+l].real - temp.imag * a[i*lda+l].imag;
                        c[j*ldc+j].imag  = 0.0;
                    }
                }
            }
        } else {
            for (j = 0; j < n; j++) {
                if (beta == 0.0) {
                    for (i = j; i < n; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                } else if (beta != 1.0) {
                    c[j*ldc+j].real *= beta;
                    c[j*ldc+j].imag  = 0.0;
                    for (i = j + 1; i < n; i++) {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                } else {
                    c[j*ldc+j].imag  = 0.0;
                }
                for (l = 0; l < k; l++) {
                    if (xCMP_NE(a[j*lda+l],zero)) {
                        BLAS_MATH_t temp;
                        xCONJUGATE(temp,a[j*lda+l]);
                        xMULcr(temp,temp,alpha);
                        c[j*ldc+j].real += temp.real * a[j*lda+l].real - temp.imag * a[j*lda+l].imag;
                        c[j*ldc+j].imag  = 0.0;
                        for (i = j + 1; i < n; i++) {
                            xFMA(c[i*ldc+j],temp,a[i*lda+l]);
                        }
                    }
                }
            }
        }
    } else {   // Form C = alpha*A**H*A + beta*C
        if (isUpper) {
            for (j = 0; j < n; j++) {
                for (i = 0; i < j; i++) {
                    BLAS_MATH_t temp;
                    xSET(temp,zero);
                    for (l = 0; l < k; l++) {
                        BLAS_MATH_t term;
                        xCONJUGATE(term, a[l*lda+i]);
                        xFMA(temp, term, a[l*lda+j]);
                    }
                    if (beta == 0.0) {
                        xMULcr(c[i*ldc+j],temp,alpha);
                    } else {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                        xFMAcr(c[i*ldc+j],temp,alpha);
                    }
                }
                BLAS_MATH_COMPONENT_t rtemp = 0.0;
                for (l = 0; l < k; l++) {
                    rtemp += a[l*lda+j].real * a[l*lda+j].real + a[l*lda+j].imag * a[l*lda+j].imag;
                }
                if (beta == 0.0) {
                    c[j*ldc+j].real = alpha * rtemp;
                    c[j*ldc+j].imag = 0;
                } else {
                    c[j*ldc+j].real = alpha * rtemp + beta * c[j*ldc+j].real;
                    c[j*ldc+j].imag = 0;
                }
            }
        } else {
            for (j = 0; j < n; j++) {
                BLAS_MATH_COMPONENT_t rtemp = 0.0;
                for (l = 0; l < k; l++) {
                    rtemp += a[l*lda+j].real * a[l*lda+j].real + a[l*lda+j].imag * a[l*lda+j].imag;
                }
                if (beta == 0.0) {
                    c[j*ldc+j].real = alpha * rtemp;
                    c[j*ldc+j].imag = 0;
                } else {
                    c[j*ldc+j].real = alpha * rtemp + beta * c[j*ldc+j].real;
                    c[j*ldc+j].imag = 0;
                }
                for (i = j + 1; i < n; i++) {
                    BLAS_MATH_t temp;
                    xSET(temp,zero);
                    for (l = 0; l < k; l++) {
                        BLAS_MATH_t term;
                        xCONJUGATE(term, a[l*lda+i]);
                        xFMA(temp, term, a[l*lda+j]);
                    }
                    if (beta == 0.0) {
                        xMULcr(c[i*ldc+j],temp,alpha);
                    } else {
                        xMULcr(c[i*ldc+j],c[i*ldc+j],beta);
                        xFMAcr(c[i*ldc+j],temp,alpha);
                    }
                }
            }
        }
    }
} // ?herk
