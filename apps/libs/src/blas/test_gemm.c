#if 1
/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

BLAS_int_t TRANSPOSE_A;    // 0 for no transpose, 1 for transpose.
BLAS_int_t TRANSPOSE_B;    // 0 for no transpose, 1 for transpose.
BLAS_int_t MtrxNumRows_a;
BLAS_int_t MtrxNumCols_a;
BLAS_int_t MtrxNumCols_b;

#define StorNumPadCols_a 7
#define StorNumPadCols_b 5
#define StorNumPadCols_c 19
#define MtrxNumRows_c    MtrxNumRows_a
#define MtrxNumRows_b    MtrxNumCols_a
#define MtrxNumCols_c    MtrxNumCols_b
#define StorNumRows_c    MtrxNumRows_c
#define StorNumCols_c    MtrxNumCols_c

ocrGuid_t NAME(test_gemm)() {

    //static BLAS_int_t ColsARowsBCases[4]  = {  1,  10, 15, 20};
    //static BLAS_int_t ColsBColsCCases[16] = {  1,   2,   3,   4,  95,  96,  97, 191, 192, 193, 383, 384, 385, 575, 576, 577};
    static BLAS_int_t ColsARowsBCases[2]  = {  1,  10};
    static BLAS_int_t ColsBColsCCases[8] = {  1,   2,  191, 193, 383, 385, 575, 577};
    static BLAS_int_t RowsARowsCCases[31] = {  1,   2,   3,   4,  23,  24,  25,  47,  48,  49,  71,  72,  73,  95,  96,  97,
                                  119, 120, 121, 143, 144, 145, 191, 192, 193, 383, 384, 385, 575, 576, 577};
    static BLAS_MATH_t a[(577)*(200+StorNumPadCols_a)];
    static BLAS_MATH_t b[(200+StorNumPadCols_a)*(577)];
    static BLAS_MATH_t c[(577)*(577+StorNumPadCols_c)];
    static BLAS_MATH_t scr[(577)*(577+StorNumPadCols_c)];

    static int iter = -1;
    if (++iter == 2*2*2*8*31) ocrShutdown();

    TRANSPOSE_A = iter & 1;
    TRANSPOSE_B = (iter >> 1) & 1;
    MtrxNumCols_a = ColsARowsBCases[(iter>>2)&1];
    MtrxNumCols_b = ColsBColsCCases[(iter>>3)&7];
    MtrxNumRows_a = RowsARowsCCases[(iter>>6)];

    static BLAS_int_t StorNumRows_a;
    static BLAS_int_t StorNumCols_a;
    static BLAS_int_t MtrxRowToRow_a;
    static BLAS_int_t MtrxColToCol_a;
    if (TRANSPOSE_A == 0) {
        StorNumRows_a  = MtrxNumRows_a;
        StorNumCols_a  = MtrxNumCols_a;
        MtrxRowToRow_a = StorNumCols_a+StorNumPadCols_a;
        MtrxColToCol_a = 1;
    } else {
        StorNumRows_a  = MtrxNumCols_a;
        StorNumCols_a  = MtrxNumRows_a;
        MtrxColToCol_a = StorNumCols_a+StorNumPadCols_a;
        MtrxRowToRow_a = 1;
    }
    static BLAS_int_t StorRowToRow_a;
    static BLAS_int_t StorColToCol_a;
    StorRowToRow_a = StorNumCols_a+StorNumPadCols_a;
    StorColToCol_a = 1;

    static BLAS_int_t StorNumRows_b;
    static BLAS_int_t StorNumCols_b;
    static BLAS_int_t MtrxRowToRow_b;
    static BLAS_int_t MtrxColToCol_b;
    if (TRANSPOSE_B == 0) {
        StorNumRows_b  = MtrxNumRows_b;
        StorNumCols_b  = MtrxNumCols_b;
        MtrxRowToRow_b = StorNumCols_b+StorNumPadCols_b;
        MtrxColToCol_b = 1;
    } else {
        StorNumRows_b  = MtrxNumCols_b;
        StorNumCols_b  = MtrxNumRows_b;
        MtrxColToCol_b = StorNumCols_b+StorNumPadCols_b;
        MtrxRowToRow_b = 1;
    }
    static BLAS_int_t StorRowToRow_b;
    static BLAS_int_t StorColToCol_b;
    StorRowToRow_b = StorNumCols_b+StorNumPadCols_b;
    StorColToCol_b = 1;

    static BLAS_int_t StorRowToRow_c;
    static BLAS_int_t StorColToCol_c;
    static BLAS_int_t MtrxRowToRow_c;
    static BLAS_int_t MtrxColToCol_c;
    StorRowToRow_c = StorNumCols_c+StorNumPadCols_c;
    StorColToCol_c = 1;
    MtrxRowToRow_c = StorNumCols_c+StorNumPadCols_c;
    MtrxColToCol_c = 1;

    static BLAS_MATH_t alpha;
    static BLAS_MATH_t beta;

    static u64 i,j;
    static u64 ctr;
    ctr = 1;

    printf ("Executing %s iter=%ld, C[%ld][%ld]=%s[%ld][%ld]*%s[%ld][%ld]  lda=%ld  ldb=%ld  ldc=%ld\n",
        STRINGIFY(NAME(test_gemm)),
        (u64) iter, (u64) StorNumRows_c, (u64) StorNumCols_c,
        TRANSPOSE_A?"transA":"A", (u64) StorNumRows_a, (u64) StorNumCols_a,
        TRANSPOSE_B?"transB":"B", (u64) StorNumRows_b, (u64) StorNumCols_b,
        (u64) StorRowToRow_a, (u64) StorRowToRow_b, (u64) StorRowToRow_c); fflush(stdout);
    for (i = 0;i < MtrxNumRows_a; i++) {
        for (j = 0;j < MtrxNumCols_a; j++) {
            xSETc(a[i*MtrxRowToRow_a+j*MtrxColToCol_a], ((BLAS_MATH_COMPONENT_t) (rand() % 2)), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
            xSETc(a[i*MtrxRowToRow_a+j*MtrxColToCol_a], ((BLAS_MATH_COMPONENT_t) ctr++), ((BLAS_MATH_COMPONENT_t) ctr++));   // Populate A with zeroes and ones.
        }
    }
    for (i = 0;i < MtrxNumRows_b; i++) {
        for (j = 0;j < MtrxNumCols_b; j++) {
            xSETc(b[i*MtrxRowToRow_b+j*MtrxColToCol_b], ((BLAS_MATH_COMPONENT_t) (rand() % 2)), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
            xSETc(b[i*MtrxRowToRow_b+j*MtrxColToCol_b], ((BLAS_MATH_COMPONENT_t) ctr++), ((BLAS_MATH_COMPONENT_t) ctr++));   // Populate A with zeroes and ones.
        }
    }
    if (!TRANSPOSE_A && !TRANSPOSE_B)
    {
        for (i = 0;i < MtrxNumRows_c; i++) {
            for (j = 0;j < MtrxNumCols_c; j++) {
                xSETc(c[i*MtrxRowToRow_c+j*MtrxColToCol_c], ((BLAS_MATH_COMPONENT_t) ((rand() % 2) + 3)), ((BLAS_MATH_COMPONENT_t) ((rand() % 2) + 3)));   // Populate C with threes and fours.
            }
        }
    }
    xSETc(alpha, 2.2, 3.3);
    xSETc(beta,  4.4, 5.5);
    //xSETc(beta,  1.0, 0.0);
    xSETc(beta,  0.0, 0.0);
    xSETc(alpha, 1.0, 1.0);

    //printf ("Calling %s standard API\n", STRINGIFY(NAME(gemm))); fflush(stdout);
    NAME(gemm) (                 // Performs a matrix-matrix operation with matrices stored in normal 2D matrix layout (not "distributed", i.e. tiled and/or paneled).
        TRANSPOSE_A ? "T" : "N", // "N" if No-transpose; "T" if Transpose.
        TRANSPOSE_B ? "T" : "N", // "N" if No-transpose; "T" if Transpose.
        MtrxNumRows_a,           // const BLAS_int_t    m,        // Specifies the number of rows of the matrices op(A) and C.
        MtrxNumCols_b,           // const BLAS_int_t    n,        // Specifies the number of columns of the matrices op(B) and C.
        MtrxNumCols_a,           // const BLAS_int_t    k,        // Specifies the number of columns of the matrix op(A) and the number of rows of the matrix op(B).
        alpha,                   // const BLAS_MATH_t   alpha,    // Specifies the scalar alpha.  When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
        &a[0],                   // const BLAS_MATH_t * a,        // Input array, size m*lda.
        StorRowToRow_a,          // const BLAS_int_t    lda,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix A.
        &b[0],                   // const BLAS_MATH_t * b,        // Input array, size k*ldb.
        StorRowToRow_b,          // const BLAS_int_t    ldb,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix B.
        beta,                    // const BLAS_MATH_t   beta,     // When beta is equal to zero, then sub(C) need not be set on input.
        &c[0],                   //       BLAS_MATH_t * c,        // Input/Output array, size m*ldc.
        StorRowToRow_c);         // const BLAS_int_t    ldc);     // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix C.

    //printf ("test_gemm done.\n");

    return NULL_GUID;
}


#else
/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#define TRANSPOSE_A 0    // 0 for no transpose, 1 for transpose.
#define TRANSPOSE_B 1    // 0 for no transpose, 1 for transpose.

#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

#define StorNumPadCols_a 7
#define StorNumPadCols_b 5
#define StorNumPadCols_c 19
#define MtrxNumRows_a    1
#define MtrxNumRows_c    MtrxNumRows_a
#define MtrxNumCols_a    10
#define MtrxNumRows_b    MtrxNumCols_a
#define MtrxNumCols_b    768
#define MtrxNumCols_c    MtrxNumCols_b

#if (TRANSPOSE_A == 0)
#define StorNumRows_a    MtrxNumRows_a
#define StorNumCols_a    MtrxNumCols_a
#define MtrxRowToRow_a   (StorNumCols_a+StorNumPadCols_a)
#define MtrxColToCol_a   1
#elif (TRANSPOSE_A == 1)
#define StorNumRows_a    MtrxNumCols_a
#define StorNumCols_a    MtrxNumRows_a
#define MtrxColToCol_a   (StorNumCols_a+StorNumPadCols_a)
#define MtrxRowToRow_a   1
#else
***** ERROR:  TRANSPOSE_A not defined to "0" or "1"
#endif
#define StorRowToRow_a   (StorNumCols_a+StorNumPadCols_a)
#define StorColToCol_a   1

#if (TRANSPOSE_B == 0)
#define StorNumRows_b    MtrxNumRows_b
#define StorNumCols_b    MtrxNumCols_b
#define MtrxRowToRow_b   (StorNumCols_b+StorNumPadCols_b)
#define MtrxColToCol_b   1
#elif (TRANSPOSE_B == 1)
#define StorNumRows_b    MtrxNumCols_b
#define StorNumCols_b    MtrxNumRows_b
#define MtrxColToCol_b   (StorNumCols_b+StorNumPadCols_b)
#define MtrxRowToRow_b   1
#else
***** ERROR:  TRANSPOSE_B not defined to "0" or "1"
#endif
#define StorRowToRow_b   (StorNumCols_b+StorNumPadCols_b)
#define StorColToCol_b   1

#define StorNumRows_c    MtrxNumRows_c
#define StorNumCols_c    MtrxNumCols_c
#define StorRowToRow_c   (StorNumCols_c+StorNumPadCols_c)
#define StorColToCol_c   1
#define MtrxRowToRow_c   (StorNumCols_c+StorNumPadCols_c)
#define MtrxColToCol_c   1

ocrGuid_t test_gemm() {
    printf ("Executing test_gemm, C[%ld][%ld]=%s[%ld][%ld]*%s[%ld][%ld]  lda=%ld  ldb=%ld  ldc=%ld\n",
        (u64) StorNumRows_c, (u64) StorNumCols_c,
        TRANSPOSE_A?"transA":"A", (u64) StorNumRows_a, (u64) StorNumCols_a,
        TRANSPOSE_B?"transB":"B", (u64) StorNumRows_b, (u64) StorNumCols_b,
        (u64) StorRowToRow_a, (u64) StorRowToRow_b, (u64) StorRowToRow_c); fflush(stdout);
    static BLAS_MATH_t a[StorNumRows_a*(StorNumCols_a+StorNumPadCols_a)];
    static BLAS_MATH_t b[StorNumRows_b*(StorNumCols_b+StorNumPadCols_b)];
    static BLAS_MATH_t c[StorNumRows_c*(StorNumCols_c+StorNumPadCols_c)];
    BLAS_MATH_t alpha;
    BLAS_MATH_t beta;

    u64 i,j;
static u64 ctr=1;

    printf ("Executing test_gemm A\n"); fflush(stdout);
    for (i = 0;i < MtrxNumRows_a; i++) {
        for (j = 0;j < MtrxNumCols_a; j++) {
            xSETc(a[i*MtrxRowToRow_a+j*MtrxColToCol_a], ((BLAS_MATH_COMPONENT_t) (rand() % 2)), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
            xSETc(a[i*MtrxRowToRow_a+j*MtrxColToCol_a], ((BLAS_MATH_COMPONENT_t) ctr++), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
        }
    }
    printf ("Executing test_gemm B\n"); fflush(stdout);
printf ("TEST_GEMM:  b: numRows=%ld, numCols=%ld, ldb=%ld  mtrxRowToRow=%ld, mtrxColToCol = %ld\n",  (u64) MtrxNumRows_b, (u64) MtrxNumCols_b, (u64) StorRowToRow_b, (u64) MtrxRowToRow_b, (u64) MtrxColToCol_b);
    for (i = 0;i < MtrxNumRows_b; i++) {
        for (j = 0;j < MtrxNumCols_b; j++) {
            xSETc(b[i*MtrxRowToRow_b+j*MtrxColToCol_b], ((BLAS_MATH_COMPONENT_t) (rand() % 2)), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
            xSETc(b[i*MtrxRowToRow_b+j*MtrxColToCol_b], ((BLAS_MATH_COMPONENT_t) ctr++), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
//printf ("Writing b[%ld]=%ld R2R=%ld C2C=%ld\n", (u64) (i*MtrxRowToRow_b+j*MtrxColToCol_b), (u64) (ctr-1), (u64) MtrxRowToRow_b, (u64) MtrxColToCol_b);
        }
    }
    printf ("Executing test_gemm C\n"); fflush(stdout);
    for (i = 0;i < MtrxNumRows_c; i++) {
        for (j = 0;j < MtrxNumCols_c; j++) {
            xSETc(c[i*MtrxRowToRow_c+j*MtrxColToCol_c], ((BLAS_MATH_COMPONENT_t) ((rand() % 2) + 3)), ((BLAS_MATH_COMPONENT_t) ((rand() % 2) + 3)));   // Populate C with threes and fours.
        }
    }
    xSETc(alpha, 2.2, 3.3);
    xSETc(beta,  4.4, 5.5);
    //xSETc(beta,  1.0, 0.0);
    xSETc(beta,  0.0, 0.0);
    xSETc(alpha, 1.0, 1.0);

    printf ("Calling %s standard API\n", STRINGIFY(NAME(gemm))); fflush(stdout);
    NAME(gemm) (                 // Performs a matrix-matrix operation with matrices stored in normal 2D matrix layout (not "distributed", i.e. tiled and/or paneled).
        TRANSPOSE_A ? "T" : "N", // "N" if No-transpose; "T" if Transpose.
        TRANSPOSE_B ? "T" : "N", // "N" if No-transpose; "T" if Transpose.
        MtrxNumRows_a,           // const BLAS_int_t    m,        // Specifies the number of rows of the matrices op(A) and C.
        MtrxNumCols_b,           // const BLAS_int_t    n,        // Specifies the number of columns of the matrices op(B) and C.
        MtrxNumCols_a,           // const BLAS_int_t    k,        // Specifies the number of columns of the matrix op(A) and the number of rows of the matrix op(B).
        alpha,                   // const BLAS_MATH_t   alpha,    // Specifies the scalar alpha.  When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
        &a[0],                   // const BLAS_MATH_t * a,        // Input array, size m*lda.
        StorRowToRow_a,          // const BLAS_int_t    lda,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix A.
        &b[0],                   // const BLAS_MATH_t * b,        // Input array, size k*ldb.
        StorRowToRow_b,          // const BLAS_int_t    ldb,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix B.
        beta,                    // const BLAS_MATH_t   beta,     // When beta is equal to zero, then sub(C) need not be set on input.
        &c[0],                   //       BLAS_MATH_t * c,        // Input/Output array, size m*ldc.
        StorRowToRow_c);         // const BLAS_int_t    ldc);     // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix C.

    printf ("test_gemm done.\n");

    return NULL_GUID;
}


#endif
