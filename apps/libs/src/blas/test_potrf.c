/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

//#define doUPPER
#ifdef doUPPER
#define UPLO     "upper"
#else
#define UPLO     "lower"
#endif
ocrGuid_t NAME(test_potrf)() {
    printf ("Executing %s\n", STRINGIFY(NAME(test_potrf))); fflush(stdout);
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define multiple 9
#define square   900   // Must be a multiple of nine
    static BLAS_MATH_t  sample[multiple][multiple] = {{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                                      { 1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0},
                                                      { 1.0, 2.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 5.0, 5.0, 5.0, 5.0, 5.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 6.0, 6.0, 6.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 7.0, 7.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 8.0},
                                                      { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}};
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
#define multiple 3
#define square   600 // Must be a multiple of three
#ifdef doUPPER
    static BLAS_MATH_t  sample[multiple][multiple] = {{{ 9.0,99.9}, { 3.0, 3.0}, { 3.0,-3.0}},
                                                      {{-1.1, 2.2}, {18.0,99.9}, { 8.0,-6.0}},
                                                      {{ 3.3,-4.4}, {-5.5,-6.6}, {43.0,99.9}}};
    //static BLAS_MATH_t  sample[multiple][multiple] = {{{25.0,99.9}, {-5.0, 5.0}, {10.0,-5.0}},
    //                                                  {{-1.1, 2.2}, {51.0,99.9}, { 4.0, 6.0}},
    //                                                  {{ 3.3,-4.4}, { 5.5, 6.6}, {71.0,99.9}}};
#else
    static BLAS_MATH_t  sample[multiple][multiple] = {{{25.0,99.9}, {-1.1, 2.2}, { 3.3,-4.4}},
                                                      {{-5.0, 5.0}, {51.0,99.9}, { 5.5, 6.6}},
                                                      {{10.0,-5.0}, { 4.0, 6.0}, {71.0,99.9}}};
#endif
#else
**** ERROR, undefined type
#endif
#define numRows square
#define numCols square
    static BLAS_MATH_t  mat[numRows][numCols];
    static BLAS_int_t   pivotIdx[MAX(numRows, numCols)];
    static BLAS_int_t   info;

    u64 i,j,q;
    BLAS_int_t ld = numCols;
    BLAS_MATH_t zero;

    for (i = 0; i < numRows; i++) {
        for (j = 0; j < numCols; j++) {
            xSETc(mat[i][j],0.0,0.0);
            //xSETc(mat[i][j], ((BLAS_MATH_COMPONENT_t) (rand() % 2)), ((BLAS_MATH_COMPONENT_t) (rand() % 2)));   // Populate A with zeroes and ones.
        }
    }

    for (q = 0; q < square; q+=multiple) {
        for (i = 0; i < multiple; i++) {
            for (j = 0; j < multiple; j++) {
                mat[q+i][q+j] = sample[i][j];
            }
        }
    }

    printf ("Calling %s standard API\n", STRINGIFY(NAME(potrf))); fflush(stdout);
    NAME(potrf) (
        UPLO,            // const char        * uplo,        // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
        square,          // const BLAS_int_t    n,           // Specifies the number of columns of the matrix.
        &mat[0][0],      // const BLAS_MATH_t * mat,         // Input array, size m*lda.
        ld,              // const BLAS_int_t    ld,          // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
        &info);          // const BLAS_int_t  * info);       // Output status value.  If <0, indicates error; if >0 indicates an index of a zero-coeff (in one-based form); if == 0, all completely normal.

    printf ("test_potrf done.\n");

    return NULL_GUID;
}


#undef doUPPER
#undef UPLO
#undef square
#undef multiple
#undef numRows
#undef numCols
