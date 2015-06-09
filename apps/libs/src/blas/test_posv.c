/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

#define doUPPER
#ifdef doUPPER
#define UPLO     "upper"
#else
#define UPLO     "lower"
#endif

ocrGuid_t NAME(test_posv)() {
    printf ("Executing %s\n", STRINGIFY(NAME(test_posv))); fflush(stdout);
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define multiple 9
#define square   900   // Must be a multiple of nine
#ifdef doUPPER
    static BLAS_MATH_t  sampleA[multiple][multiple] = {{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                                       {99.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0},
                                                       {99.0,99.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0},
                                                       {99.0,99.0,99.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0},
                                                       {99.0,99.0,99.0,99.0, 5.0, 5.0, 5.0, 5.0, 5.0},
                                                       {99.0,99.0,99.0,99.0,99.0, 6.0, 6.0, 6.0, 6.0},
                                                       {99.0,99.0,99.0,99.0,99.0,99.0, 7.0, 7.0, 7.0},
                                                       {99.0,99.0,99.0,99.0,99.0,99.0,99.0, 8.0, 8.0},
                                                       {99.0,99.0,99.0,99.0,99.0,99.0,99.0,99.0, 9.0}};
#else
    static BLAS_MATH_t  sampleA[multiple][multiple] = {{ 1.0,99.0,99.0,99.0,99.0,99.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0,99.0,99.0,99.0,99.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0,99.0,99.0,99.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0,99.0,99.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0, 5.0,99.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,99.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,99.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,99.0},
                                                       { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}};
#endif
    static BLAS_MATH_t  sampleB[multiple][2] =        {{  9.0, 45.0},
                                                       { 17.0, 89.0},
                                                       { 24.0,131.0},
                                                       { 30.0,170.0},
                                                       { 35.0,205.0},
                                                       { 39.0,235.0},
                                                       { 42.0,259.0},
                                                       { 44.0,276.0},
                                                       { 45.0,285.0}};
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
#define multiple 3
#define square   3     // Must be a multiple of three
#ifdef doUPPER
    static BLAS_MATH_t  sampleA[multiple][multiple] = {{{ 9.0,99.9}, { 3.0, 3.0}, { 3.0,-3.0}},
                                                       {{-1.1, 2.2}, {18.0,99.9}, { 8.0,-6.0}},
                                                       {{ 3.3,-4.4}, {-5.5,-6.6}, {43.0,99.9}}};
    static BLAS_MATH_t  sampleB[multiple][2] =        {{{  33.0, -18.0}, {  15.0,  -3.0}},
                                                       {{  45.0, -45.0}, {   8.0,  -2.0}},
                                                       {{ 152.0,   1.0}, {  43.0, -29.0}}};
#else
    static BLAS_MATH_t  sampleA[multiple][multiple] = {{{25.0,99.9}, {-1.1, 2.2}, { 3.3,-4.4}},
                                                       {{-5.0, 5.0}, {51.0,99.9}, { 5.5, 6.6}},
                                                       {{10.0,-5.0}, { 4.0, 6.0}, {71.0,99.9}}};
    static BLAS_MATH_t  sampleB[multiple][2] =        {{{  60.0, -55.0}, {  70.0,  10.0}},
                                                       {{  34.0,  58.0}, { -51.0, 110.0}},
                                                       {{  13.0,-152.0}, {  75.0,  63.0}}};
#endif
#else
**** ERROR, undefined type
#endif
#define numRows square
#define numCols square
    static BLAS_MATH_t  matA[numRows][numCols];
    static BLAS_MATH_t  matB[numRows][2];
    static BLAS_int_t   info;

    u64 i,j,q;
    BLAS_int_t lda = numCols;
    BLAS_MATH_t zero;

    for (i = 0; i < numRows; i++) {
        for (j = 0; j < numCols; j++) {
            xSETc(matA[i][j],0.0,0.0);
        }
    }

    for (q = 0; q < square; q+=multiple) {
        for (i = 0; i < multiple; i++) {
            for (j = 0; j < multiple; j++) {
                matA[q+i][q+j] = sampleA[i][j];
            }
            matB[q+i][0] = sampleB[i][0];
            matB[q+i][1] = sampleB[i][1];
        }
    }

    printf ("Calling %s standard API  uplo=%s\n", STRINGIFY(NAME(posv)), UPLO); fflush(stdout);
    NAME(posv) (
        UPLO,            // const char        * uplo,        // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
        square,          // const BLAS_int_t    n,           // Specifies the order of the matrix.
        2,               // const BLAS_int_t    nrhs,        // Number of right-hand sides.
        &matA[0][0],     // const BLAS_MATH_t * a,           // Input array, size n*lda.
        lda,             // const BLAS_int_t    lda,         // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
        &matB[0][0],     // const BLAS_MATH_t * b,           // Input array, size nhrs*ldb.
        2,               // const BLAS_int_t    ldb,         // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
        &info);          // const BLAS_int_t  * info);       // Output status value.  If <0, indicates error; if >0 indicates an index of a zero-coeff (in one-based form); if == 0, all completely normal.

    printf ("test_posv done.\n");

    return NULL_GUID;
}
#undef doUpper
#undef UPLO
#undef square
#undef multiple
#undef numRows
#undef numCols
