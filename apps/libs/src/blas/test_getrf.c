/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include "ocr.h"
#include "ocr-types.h"
#include <stdio.h>
#include "blas.h"

#define square  800    // Must be a multiple of four
#define numRows square
#define numCols square
ocrGuid_t NAME(test_getrf)() {
    printf ("Executing %s\n", STRINGIFY(NAME(test_getrf))); fflush(stdout);
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    static BLAS_MATH_t  sample[4][4] = {{ 1.80, 2.88, 2.05,-0.89},
                                        { 5.25,-2.95,-0.95,-3.80},
                                        { 1.58,-2.69,-2.90,-1.04},
                                        {-1.11,-0.66,-0.59, 0.80}};
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    static BLAS_MATH_t  sample[4][4] = {{{-1.34, 2.55},{ 0.28, 3.17},{-6.39,-2.20},{ 0.72,-0.92}},
                                        {{-0.17,-1.41},{ 3.31,-0.15},{-0.15, 1.34},{ 1.29, 1.38}},
                                        {{-3.29,-2.39},{-1.91, 4.42},{-0.14,-1.35},{ 1.72, 1.35}},
                                        {{ 2.41, 0.39},{-0.56, 1.47},{-0.83,-0.69},{-1.96, 0.67}}};
#else
**** ERROR, undefined type
#endif
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

    for (q = 0; q < square; q+=4) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                mat[q+i][q+j] = sample[i][j];
            }
        }
    }

    printf ("Calling %s standard API\n", STRINGIFY(NAME(getrf))); fflush(stdout);
    NAME(getrf) (
        numRows,         // const BLAS_int_t    m,           // Specifies the number of rows of the matrix.
        numCols,         // const BLAS_int_t    n,           // Specifies the number of columns of the matrix.
        &mat[0][0],      // const BLAS_MATH_t * mat,         // Input array, size m*lda.
        ld,              // const BLAS_int_t    ld,          // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
        pivotIdx,        // const BLAS_int_t  * pivotIdx,    // Output array of pivot indices.  These are one-based indices.
        &info);          // const BLAS_int_t  * info);       // Output status value.  If <0, indicates error; if >0 indicates an index of a zero-coeff (in one-based form); if == 0, all completely normal.


    printf ("test_getrf done.\n");

    return NULL_GUID;
}


#undef square
