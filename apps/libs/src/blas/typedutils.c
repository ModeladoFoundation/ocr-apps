/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include "blas.h"
#include <stdio.h>
#include <string.h>


BLAS_int_t NAME2(i,amax)(BLAS_int_t vectorLen, BLAS_MATH_t * vector, BLAS_int_t stride)
{
    if (vectorLen == 0) return 0;
    u64 index;
    u64 winner = 0;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME2(i,amax)),vectorLen,false)
    BLAS_MATH_t greatest = (*vector<0) ? -(*vector) : (*vector);
    for (index = 1; index < vectorLen; index++) {
        vector += stride;
        BLAS_MATH_t candidate = (*vector<0) ? -(*vector) : (*vector);
        if (candidate > greatest) {
            greatest = candidate;
            winner = index;
        }
    }
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME2(i,amax)),vectorLen*5,false)
    BLAS_UPCONVERTED_MATH_COMPONENT_t realPart = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (vector[0].real);
    BLAS_UPCONVERTED_MATH_COMPONENT_t imagPart = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (vector[0].imag);
    BLAS_UPCONVERTED_MATH_COMPONENT_t greatest = (realPart * realPart) + (imagPart * imagPart);
    BLAS_UPCONVERTED_MATH_COMPONENT_t candidate;
    for (index = 1; index < vectorLen; index++) {
        realPart = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (vector[index].real);
        imagPart = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (vector[index].imag);
        candidate = (realPart * realPart) + (imagPart * imagPart);
        if (candidate > greatest) {
            greatest = candidate;
            winner = index;
        }
    }
#endif
    return winner + 1;    // Return a one-based result for the winning index, because LAPACK has its roots in Fortran, a one-based array-indexing language
} // i?amax


void NAME(ger)(
    BLAS_int_t    m,     // numRows of matrix A; length of vector x
    BLAS_int_t    n,     // numCols of matrix A; length of vector y
    BLAS_MATH_t   alpha,
    BLAS_MATH_t * x,
    BLAS_int_t    incx,
    BLAS_MATH_t * y,
    BLAS_int_t    incy,
    BLAS_MATH_t * a,
    BLAS_int_t    lda)
{
    BLAS_int_t   info = 0;
    if (m < 0) info = 1;
    else if (n < 0) info = 2;
    else if (incx == 0) info = 5;
    else if (incy == 0) info = 7;
    else if (lda < MAX(1,n)) info = 9;
    if (info != 0) {
        xerbla (STRINGIFY(NAME(ger)), info);
        return;
    }
    if (m == 0 || n == 0) return;

    BLAS_MATH_t  one;
    BLAS_MATH_t  negOne;
    BLAS_MATH_t  zero;
    xSETc(one,     1.0, 0.0);
    xSETc(negOne, -1.0, 0.0);
    xSETc(zero,    0.0, 0.0);
    if (xCMP_EQ(alpha, zero)) return;    // Alpha is zero.  No change to matrix A.
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(ger)),(m*n),false)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(ger)),(m*n*4),false)
#endif
    if (xCMP_EQ(alpha, one)) {           // No alpha scaling necessary.  Just calculate A += x*y;
        u64 i, j;
        for (i = 0; i < m; i++) {
            for (j = 0; j < n; j++) {
                xFMA(a[i*lda+j],x[i*incx],y[j*incy]);
            }
        }
    } else if (xCMP_EQ(alpha, negOne)) { // Alpha is minus one.  Just calculate A -= x*y;
        u64 i, j;
        for (i = 0; i < m; i++) {
            for (j = 0; j < n; j++) {
                xFMS(a[i*lda+j],x[i*incx],y[j*incy]);
            }
        }
    } else {                             // General alpha-scaling is necessary.
        u64 i, j;
        for (i = 0; i < m; i++) {
            BLAS_MATH_t temp;
            xMUL(temp,alpha,x[i*incx]);
            for (j = 0; j < n; j++) {
                xFMA(a[i*lda+j],temp,y[j*incy]);
            }
        }
    }
} // ?ger


void NAME(scal)(
    BLAS_int_t    n,     // number of elements to scale
    BLAS_MATH_t   alpha, // scaling factor
    BLAS_MATH_t * x,     // vector to scale
    BLAS_int_t    incx)  // stride of vector
{
    BLAS_int_t i;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    if (alpha.imag == 0.0) {   // Scaling factor is just a real, meaning scaling math is simpler.  Optimize accordingly.
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(scal)),n,false)
        for (i = 0; i < n; i++) {
            xMULcr(*x, *x, alpha.real);
            x+= incx;
        }
        return;
    }
#endif
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(scal)),n,false)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(scal)),n*4,false)
#endif
    for (i = 0; i < n; i++) {
        xMUL(*x, *x, alpha);
        x+= incx;
    }
} // ?scal

void NAME(scal_upconvertedAlpha)(
    BLAS_int_t                n,     // number of elements to scale
    BLAS_UPCONVERTED_MATH_t   alpha, // scaling factor
    BLAS_MATH_t             * x,     // vector to scale
    BLAS_int_t                incx)  // stride of vector
{
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(scal_upconvertedAlpha)),n,false)
    BLAS_int_t i;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    if (alpha.imag == 0.0) {   // Scaling factor is just a real, meaning scaling math is simpler.  Optimize accordingly.
        for (i = 0; i < n; i++) {
            xMULcr(*x, *x, alpha.real);
            x+= incx;
        }
        return;
    }
#endif
    for (i = 0; i < n; i++) {
        xMUL(*x, *x, alpha);
        x+= incx;
    }
} // ?scal_upconvertedAlpha


void NAME(swap)(
    BLAS_int_t    n,     // number of elements in the two vectors to swap
    BLAS_MATH_t * x,     // first vector to scale
    BLAS_int_t    incx,  // stride of first vector
    BLAS_MATH_t * y,     // second vector to scale
    BLAS_int_t    incy)  // stride of second vector
{
    BLAS_int_t i;
    BLAS_MATH_t temp;
    for (i = 0; i < n; i++) {
        xSET(temp,*x);
        xSET(*x,*y);
        xSET(*y,temp);
        x+=incx;
        y+=incy;
    }
    return;
} // ?swap


void NAME(laswp)(
    BLAS_int_t    n,        // Number of columns in the matrix.
    BLAS_MATH_t * mat,      // Matrix on which to perform the row interchanges.
    BLAS_int_t    ld,       // Leading dimension (aka "pitch") of matrix, in units of BLAS_MATH_t, i.e. element size.
    BLAS_int_t    k1,       // First element of pivotIdx for which a row interchange will be done.  This is a zero-based index into the pivotIdx array and the row of the matrix.
    BLAS_int_t    k2,       // Last  element of pivotIdx for which a row interchange will be done.  This is a zero-based index into the pivotIdx array and the row of the matrix.
    BLAS_int_t  * pivotIdx, // Vector of pivot indices.  The values in this vector are one-based.
    BLAS_int_t    incx)     // Increment between successive values of pivotIdx.  If negative, pivots applied in reverse order.
{
    BLAS_int_t    ix, i, j, ip;
    BLAS_MATH_t   temp;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(laswp)),n*(incx>0?(k2-k1+1):(k1-k2+1))*4,false)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(laswp)),n*(incx>0?(k2-k1+1):(k1-k2+1))*8,false)
#endif
    if (incx > 0) {
        ix = k1;
        for (i = k1; i <= k2; i++) {
            ip = pivotIdx[ix] - 1;   // Make index zero-based
            if (ip != i) {
                for (j = 0; j < n; j++) {
                    xSET(temp, mat[i*ld+j]);
                    xSET(mat[i*ld+j],mat[ip*ld+j]);
                    xSET(mat[ip*ld+j],temp);
                }
            }
            ix += incx;
        }
    } else if (incx < 0) {
        printf ("******** In %s, negative value for incx argument is not yet implemented\n", STRINGIFY(NAME(laswp)));
    } else {
        return;  // No pivots if incx == 0
    }
} // ?laswp


#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
BLAS_MATH_t NAME(dot)       // Compute dot product.
#else
BLAS_MATH_t NAME(dotu)      // Compute dot product.
#endif
(
    BLAS_int_t    n,        // Number of elements in the two vectors.
    BLAS_MATH_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t    incy)     // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.
{
    BLAS_MATH_t dot0, dot1, dot2, dot3;
    xSETc(dot0, 0.0, 0.0);
    xSETc(dot1, 0.0, 0.0);
    xSETc(dot2, 0.0, 0.0);
    xSETc(dot3, 0.0, 0.0);

    if (n <= 0) return dot0;

    if (incx == 1 && incy == 1) {
        while (n > 3) {
            xFMA(dot0, x[0], y[0]);
            xFMA(dot1, x[1], y[1]);
            xFMA(dot2, x[2], y[2]);
            xFMA(dot3, x[3], y[3]);
            x += 4;
            y += 4;
            n -= 4;
        }
    } else {
        if (incx < 0) x -= incx * (n-1);      // Traverse X in reverse order if increment is negative.
        if (incy < 0) y -= incy * (n-1);      // Traverse Y in reverse order if increment is negative.
        while (n > 3) {
            xFMA(dot0, x[0     ], y[0     ]);
            xFMA(dot1, x[incx  ], y[incy  ]);
            xFMA(dot2, x[incx*2], y[incy*2]);
            xFMA(dot3, x[incx*3], y[incy*3]);
            x += incx*4;
            y += incy*4;
            n -= 4;
        }
    }
    if (n > 0) {
        xFMA(dot0, x[0     ], y[0     ]);
        if (n > 1) {
            xFMA(dot1, x[incx  ], y[incy  ]);
            if (n > 2) {
                xFMA(dot2, x[incx*2], y[incy*2]);
            }
        }
    }
    xADD(dot0,dot0,dot1);
    xADD(dot2,dot2,dot3);
    xADD(dot0,dot0,dot2);
    return dot0;
} // sdot, ddot, cdotu, or zdotu


#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
BLAS_MATH_t NAME(dotc)(     // Compute dot product, conjugating the first vector.
    BLAS_int_t    n,        // Number of elements in the two vectors.
    BLAS_MATH_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).  We will conjugate this operand.
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t    incy)     // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.
{
    BLAS_MATH_t dot0, dot1, dot2, dot3;
    xSETc(dot0, 0.0, 0.0);
    xSETc(dot1, 0.0, 0.0);
    xSETc(dot2, 0.0, 0.0);
    xSETc(dot3, 0.0, 0.0);

    if (n <= 0) return dot0;

    if (incx == 1 && incy == 1) {
        while (n > 3) {
            xFMA_conjg1(dot0, x[0], y[0]);
            xFMA_conjg1(dot1, x[1], y[1]);
            xFMA_conjg1(dot2, x[2], y[2]);
            xFMA_conjg1(dot3, x[3], y[3]);
            x += 4;
            y += 4;
            n -= 4;
        }
    } else {
        if (incx < 0) x -= incx * (n-1);      // Traverse X in reverse order if increment is negative.
        if (incy < 0) y -= incy * (n-1);      // Traverse Y in reverse order if increment is negative.
        while (n > 3) {
            xFMA_conjg1(dot0, x[0     ], y[0     ]);
            xFMA_conjg1(dot1, x[incx  ], y[incy  ]);
            xFMA_conjg1(dot2, x[incx*2], y[incy*2]);
            xFMA_conjg1(dot3, x[incx*3], y[incy*3]);
            x += incx*4;
            y += incy*4;
            n -= 4;
        }
    }
    if (n > 0) {
        xFMA_conjg1(dot0, x[0     ], y[0     ]);
        if (n > 1) {
            xFMA_conjg1(dot1, x[incx  ], y[incy  ]);
            if (n > 2) {
                xFMA_conjg1(dot2, x[incx*2], y[incy*2]);
            }
        }
    }
    xADD(dot0,dot0,dot1);
    xADD(dot2,dot2,dot3);
    xADD(dot0,dot0,dot2);
    return dot0;
} // cdotc or zdotc
#endif


void NAME(gemv)(      // perform one of these matrix-vector operations:   y=alpha*A*x+beta*y or y=alpha*A**T*x+beta*y.
    char        * trans,    // If first character = 'N' or 'n' compute y=alpha*A*x+beta*y;  if 'T', 't', 'C', or 'c' compute y=alpha*A**T*x+beta*y
    BLAS_int_t    m,        // Number of rows in matrix A.
    BLAS_int_t    n,        // Number of columns in matrix A.
    BLAS_MATH_t   alpha,    // Scalar multiplier.
    BLAS_MATH_t * a,        // Address of matrix A.
    BLAS_int_t    lda,      // Leading dimension of matrix A.
    BLAS_MATH_t * x,        // Address of vector X.
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t   beta,     // Scalar multiplier.
    BLAS_MATH_t * y,        // Address of vector Y.
    BLAS_int_t    incy)     // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.
{

    bool doTranspose;
    BLAS_int_t lenx, leny, i, j;
    if (trans[0] == 'N' || trans[0] == 'n') {
        doTranspose = false;
        lenx = n;
        leny = m;
    } else if (trans[0] == 'T' || trans[0] == 't' || trans[0] == 'C' || trans[0] == 'c') {
        doTranspose = true;
        lenx = m;
        leny = n;
    } else {
        xerbla (STRINGIFY(NAME(gemv)), 1);
        return;
    }
    if (m < 0) {
        xerbla (STRINGIFY(NAME(gemv)), 2);
        return;
    }
    if (n < 0) {
        xerbla (STRINGIFY(NAME(gemv)), 3);
        return;
    }
    if (lda < MAX(1,m)) {
        xerbla (STRINGIFY(NAME(gemv)), 6);
        return;
    }
    if (incx == 0) {
        xerbla (STRINGIFY(NAME(gemv)), 8);
        return;
    }
    if (incy == 0) {
        xerbla (STRINGIFY(NAME(gemv)), 11);
        return;
    }
    // Quick return if possible:
    BLAS_MATH_t zero, one;
    xSETc(zero,0.0,0.0);
    xSETc(one, 1.0,0.0);
    if ((m == 0) || (n == 0) || (xCMP_EQ(alpha,zero) && xCMP_EQ(beta,one))) return;

    if (incx < 0) x -= incx * (n-1);      // Traverse X in reverse order if increment is negative.
    if (incy < 0) y -= incy * (n-1);      // Traverse Y in reverse order if increment is negative.

    // Start the operations.  Elements of A are accessed sequentially and in a single pass.  First, calculate y=beta*y.
    if (xCMP_NE(beta,one)) {
        if (incy == 1) {
            if (xCMP_EQ(beta,zero)) {
                for (i = 0; i < leny; i++) {
                    xSET(y[i],zero);
                }
            } else {
                for (i = 0; i < leny; i++) {
                    xMUL(y[i],y[i],beta);
                }
            }
        } else {
            if (xCMP_EQ(beta,zero)) {
                BLAS_MATH_t *py = y;
                for (i = 0; i < leny; i++) {
                    xSET(*py,zero);
                    py += incy;
                }
            } else {
                BLAS_MATH_t *py = y;
                for (i = 0; i < leny; i++) {
                    xMUL(*py,*py,beta);
                    py += incy;
                }
            }
        }
    }
    if (xCMP_EQ(alpha,zero)) return;

    if (!doTranspose) {    // Calculate y=alpha*A*x+y
        if (incy == 1) {
            BLAS_MATH_t *px = x;
            for (j = 0; j < n; j++) {
                if (xCMP_NE(*px,zero)) {
                    BLAS_MATH_t temp;
                    xMUL(temp, alpha, *px);
                    for (i = 0; i < m; i++) {
                        xFMA(y[i],temp,a[i*lda+j]);
                    }
                }
                px += incx;
            }
        } else {
            BLAS_MATH_t *px = x;
            for (j = 0; j < n; j++) {
                if (xCMP_NE(*px,zero)) {
                    BLAS_MATH_t temp;
                    xMUL(temp, alpha, *px);
                    BLAS_MATH_t *py = y;
                    for (i = 0; i < m; i++) {
                        xFMA(*py,temp,a[i*lda+j]);
                        py += incy;
                    }
                }
                px += incx;
            }
        }
    } else {               // Calculate y=alpha*A**T*x+y.
        if (incx == 1) {
            BLAS_MATH_t *py = y;
            for (j = 0; j < n; j++) {
                BLAS_MATH_t temp;
                xSET(temp,zero);
                for (i = 0; i < m; i++) {
                    xFMA(temp,a[i*lda+j],x[i]);
                }
                xFMA(*py,alpha,temp);
                py += incy;
            }
        } else {
            BLAS_MATH_t *py = y;
            for (j = 0; j < n; j++) {
                BLAS_MATH_t temp;
                xSET(temp,zero);
                BLAS_MATH_t *px = x;
                for (i = 0; i < m; i++) {
                    xFMA(temp,a[i*lda+j],*px);
                    px += incx;
                }
                xFMA(*py,alpha,temp);
                py += incy;
            }
        }
    }
} // ?gemv



#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
void NAME(lacgv)(           // congucate a complex vector.
    BLAS_int_t    n,        // Number of elements in vector X.
    BLAS_MATH_t * x,        // Address of vector X.
    BLAS_int_t    incx)     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
{
    if (incx < 0) x += incx * (n-1);
    while (n-- > 0) {
        x->imag = -x->imag;
        x += incx;
    }
} // clacgv or zlacgv
#endif
