/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include "ocr.h"
#include "ocr-types.h"
#include "extensions/ocr-legacy.h"
#include <stdio.h>
#include "blas.h"

void NAME(test_spmmm)() {

#define m   4000
#define n   8000
#define k   6000
#define lda 10
#define ldb 15
#define ldc 3
    static SpmmmIdx_t   aColIdx[m][lda];
    static BLAS_MATH_t  aVal[m][lda];
    static SpmmmIdx_t   aNumNzColsInRow[m] = {0};
    static SpmmmIdx_t   bColIdx[n][ldb];
    static BLAS_MATH_t  bVal[n][ldb];
    static SpmmmIdx_t   bNumNzColsInRow[n] = {0};
    static SpmmmIdx_t   cColIdx[m][ldc];
    static BLAS_MATH_t  cVal[m][ldc];
    static SpmmmIdx_t   cNumNzColsInRow[m] = {0};

#define populateA(row,col,val) {aColIdx[row][aNumNzColsInRow[row]] = col; xSETc(aVal[row][aNumNzColsInRow[row]],val,0.0); aNumNzColsInRow[row]++;}
#define populateB(row,col,val) {bColIdx[row][bNumNzColsInRow[row]] = col; xSETc(bVal[row][bNumNzColsInRow[row]],val,0.0); bNumNzColsInRow[row]++;}

//    populateA( 10, 20, 10020.0);
//    populateA( 10,200, 10200.0);
//    populateA(500,200,500200.0);
//    populateA(678,123,678123.0);
//

// CAUTION!  BE SURE TO POPULATE MATRICES IN CANONICAL ORDER!!!

    populateA(   0,1000,2001.0);  // C[0][1000]
    populateA(   0,2000,4001.0);  // Different AxB, same element of C, i.e. C[0][1000].
    populateA(   0,5000,6001.0);  // Same AxB, same element of C, i.e. C[0][1000].
    populateA(   0,6131,1001.0);
    populateA(   1, 123,7001.0);
    populateA(  32,2000,5001.0);  // Different AxB, same C but different row, i.e. C[32][1032].
    populateA(2000,1000,3001.0);  // Same AxB, different row of C, i.e. C[2000][1000]

    populateB( 123, 456, 601.0);
    populateB(1000,1000, 101.0);
    populateB(2000,1000, 201.0);
    populateB(2000,1016, 301.0);
    populateB(2000,1032, 401.0);
    populateB(5000,1000, 501.0);
    populateB(6131,2456, 701.0);

    //populateA(   1,   2, 101.0);

    //populateA(   0,2000,701.0);  // Different AxB, same C but different element in same row, i.e. C[0][1016].

    //populateA(3000,6000,401.0);
    //populateB(1000,2000,601.0);
    //populateB(3000,2000,701.0);
    //populateB(6000,4000,901.0);

    u64 result;
    SpmmmIdx_t cMaxNumNzColsInRow;
    s64        aTotalNumNzElements = 4;
    s64        bTotalNumNzElements = 5;
    s64        cTotalNumNzElements;

    NAME(spmmm) (            // Function-type API for sparse-matrix-sparse-matrix multiply.
        m,                   // MatrixIndex_t      m,                   // Number of rows in matrix A and in matrix C.
        n,                   // MatrixIndex_t      n,                   // Number of columns in matrix A, and number of rows in matrix B.
        k,                   // MatrixIndex_t      k,                   // Number of columns in matrix B and in matrix C.
        &aVal[0][0],         // MatrixDataType_t * aVal,                // Address of 2D array holding non-zero-valued elements of matrix A.
        &bVal[0][0],         // MatrixDataType_t * bVal,                // Address of 2D array holding non-zero-valued elements of matrix B.
        &cVal[0][0],         // MatrixDataType_t * cVal,                // Address of 2D array receiving non-zero-valued elements of matrix C.
        &aColIdx[0][0],      // MatrixIndex_t    * aColIdx,             // Address of 2D array holding column indices of matrix A.
        &bColIdx[0][0],      // MatrixIndex_t    * bColIdx,             // Address of 2D array holding column indices of matrix B.
        &cColIdx[0][0],      // MatrixIndex_t    * cColIdx,             // Address of 2D array receiving column indices of matrix C.
        &aNumNzColsInRow[0], // MatrixIndex_t    * aNumNzColsInRow,     // Address of 1D array holding the count of column indices and values for each row of A.
        &bNumNzColsInRow[0], // MatrixIndex_t    * bNumNzColsInRow,     // Address of 1D array holding the count of column indices and values for each row of B.
        &cNumNzColsInRow[0], // MatrixIndex_t    * cNumNzColsInRow,     // Address of 1D array receiving the count of column indices and values for each row of C.
        lda,                 // MatrixIndex_t      lda,                 // Leading dimension of aVal and aIdx arrays, aka the maximum number of non-zero-valued elements any row of A can accomodate.
        ldb,                 // MatrixIndex_t      ldb,                 // Leading dimension of bVal and bIdx arrays, aka the maximum number of non-zero-valued elements any row of B can accomodate.
        ldc,                 // MatrixIndex_t      ldc,                 // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
        0.0,//701702.0*701702.0, //0.0,                 // double             epsilon,             // When a result is between +/- epsilon, it is eliminated from the output, i.e. set to zero.
        aTotalNumNzElements, // MatrixIndex_t      aTotalNumNzElements, // Number of nonzeroes in matrix A.  If unknown, provide intelligent guess, or -1.
        bTotalNumNzElements, // MatrixIndex_t      bTotalNumNzElements, // Number of nonzeroes in matrix B.  If unknown, provide intelligent guess, or -1.
        &cTotalNumNzElements,// MatrixIndex_t    * cTotalNumNzElements, // Output: Number of nonzeroes in matrix C.  If C is source (A of B) for subsequent ?spmmm, providing this value will improve performance.
        &cMaxNumNzColsInRow, // MatrixIndex_t    * cMaxNumNzColsInRow,  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
        &result);            // u64              * resultCode,          // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.

    if (result == 0) {
        printf ("Upon return from spmmm, cTotalNumNzElements = %ld, cMaxNumNzColsInRow = %ld, resultCode = 0 (success!)\n", (u64) cTotalNumNzElements, (u64) cMaxNumNzColsInRow);
    } else {
        SPMMM_RETURN_CODE_t * errorCode = (SPMMM_RETURN_CODE_t *) (&result);
        printf ("Upon return from spmmm, cTotalNumNzElements = %ld, cMaxNumNzColsInRow = %ld, resultCode = errorCode %ld on row %ld, detail %ld\n",
            (u64) cTotalNumNzElements, (u64) cMaxNumNzColsInRow, (u64) errorCode->errorType, (u64) errorCode->offendingRowNum, (u64) errorCode->errorDetail);
    }

    u64 rowNum;
    for (rowNum = 0; rowNum < m; rowNum++) {
        if (cNumNzColsInRow[rowNum] == 0) continue;
        printf ("Row %ld\n", (u64) rowNum);
        u64 colIdxIdx;
        for (colIdxIdx = 0; colIdxIdx < cNumNzColsInRow[rowNum]; colIdxIdx++) {
            printf ("   Col:  %ld = (%f, %f)\n", cColIdx[rowNum][colIdxIdx], xREAL(cVal[rowNum][colIdxIdx]), xIMAG(cVal[rowNum][colIdxIdx]));
        }
    }

}
#undef m
#undef n
#undef k
#undef lda
#undef ldb
#undef ldc
