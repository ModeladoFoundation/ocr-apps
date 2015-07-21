/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?SYRK,  where ? is: <s = single precision | d = double precision>
 *
 *  Perform one of the symmetric rank k operations:
 *      C = alpha*A*A**T = beta*C
 *      C = alpha*A**T*A = beta*c
 *  where alpha and beta are scalars, C is an n-by-n symmetric matrix, and A is n-by-k (in the first case) or k-by-n (in the second case).
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


void NAME(syrk)(
    char        * uplo,     // First char:  'U' or 'u' -- only the upper triangle of C is to be referenced;  'L' or 'l' -- only the lower part...
    char        * trans,    // First char:  'N' or 'n' -- C=alpha*A*A**T + beta*C;  'T', 't', 'C', or 'c' -- C=alpha*A**T*A = beta*C.
    BLAS_int_t    n,        // Order of matrix C.  Must be >= 0.
    BLAS_int_t    k,        // If no transpose, number of columns in matrix A; if transpose, number of rows.  Must be >= 0.
    BLAS_MATH_t   alpha,    // Scalar multiplier.
    BLAS_MATH_t * a,        // Triangular input matrix.
    BLAS_int_t    lda,      // Leading dimension of A.
    BLAS_MATH_t   beta,     // Scalar multiplier.
    BLAS_MATH_t * c,        // Triangular input/output matrix.
    BLAS_int_t    ldc)      // Leading dimension of C.
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
    if (n == 0 || ((xCMP_EQ(alpha,zero) || k == 0) && xCMP_EQ(beta,one))) return;

    // Less work when alpha is zero:

    BLAS_int_t i,j,l;

    if (xCMP_EQ(alpha,zero)) {
        if (isUpper) {
            if (xCMP_EQ(beta,zero)) {
                for (j = 0; j < n; j++) {
                    for (i = 0; i <= j; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                }
            } else {
                for (j = 0; j < n; j++) {
                    for (i = 0; i <= j; i++) {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                }
            }
        } else {
            if (xCMP_EQ(beta,zero)) {
                for (j = 0; j < n; j++) {
                    for (i = j; i < n; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                }
            } else {
                for (j = 0; j < n; j++) {
                    for (i = j; i < n; i++) {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                }
            }
        }
        return;
    }

    // Start the full operation.

    if (!doTranspose) {
        if (isUpper) {
            for (j = 0; j < n; j++) {
                if (xCMP_EQ(beta,zero)) {
                    for (i = 0; i <= j; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                } else if (xCMP_NE(beta,one)) {
                    for (i = 0; i <= j; i++) {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                }
                for (l = 0; l < k; l++) {
                    if (xCMP_NE(a[j*lda+l],zero)) {
                        BLAS_MATH_t temp;
                        xMUL(temp,alpha,a[j*lda+l]);
                        for (i = 0; i <= j; i++) {
                            xFMA(c[i*ldc+j],temp,a[i*lda+l]);
                        }
                    }
                }
            }

        } else {
            for (j = 0; j < n; j++) {
                if (xCMP_EQ(beta,zero)) {
                    for (i = j; i < n; i++) {
                        xSET(c[i*ldc+j],zero);
                    }
                } else if (xCMP_NE(beta,one)) {
                    for (i = j; i < n; i++) {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                    }
                }
                for (l = 0; l < k; l++) {
                    if (xCMP_NE(a[j*lda+l],zero)) {
                        BLAS_MATH_t temp;
                        xMUL(temp,alpha,a[j*lda+l]);
                        for (i = j; i < n; i++) {
                            xFMA(c[i*ldc+j],temp,a[i*lda+l]);
                        }
                    }
                }
            }
        }
    } else {
        if (isUpper) {
            for (j = 0; j < n; j++) {
                for (i = 0; i <= j; i++) {
                    BLAS_MATH_t temp;
                    xSET(temp,zero);
                    for (l = 0; l < k; l++) {
                        xFMA(temp,a[l*lda+i],a[l*lda+j]);
                    }
                    if (xCMP_EQ(beta,zero)) {
                        xMUL(c[i*ldc+j],temp,alpha);
                    } else {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                        xFMA(c[i*ldc+j],alpha,temp);
                    }
                }
            }
        } else {
            for (j = 0; j < n; j++) {
                for (i = j; i < n; i++) {
                    BLAS_MATH_t temp;
                    xSET(temp,zero);
                    for (l = 0; l < k; l++) {
                        xFMA(temp,a[l*lda+i],a[l*lda+j]);
                    }
                    if (xCMP_EQ(beta,zero)) {
                        xMUL(c[i*ldc+j],temp,alpha);
                    } else {
                        xMUL(c[i*ldc+j],c[i*ldc+j],beta);
                        xFMA(c[i*ldc+j],alpha,temp);
                    }
                }
            }
        }
    }
} // ?syrk
