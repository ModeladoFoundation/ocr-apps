/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


#ifndef __BLAS_H__
#define __BLAS_H__

//#define BLAS_TYPE__single
//#define BLAS_TYPE__double
//#define BLAS_TYPE__complex
//#define BLAS_TYPE__complex_double
//#define BLAS_TYPE__common

#if 1    // Enable if you want to report maximum workload sides experienced by each function;  Disable for production mode silence.
#define REPORT_WORKLOAD_HIGH_WATER_MARKS(counterNameModifier,startingLevel,funcName,sizeOfThisWorkload,hasBeenSplit) {            \
    u64 size = (u64) (sizeOfThisWorkload);                                                                                        \
    static u64 NAME2(highWaterMark,counterNameModifier) = startingLevel - 1;                                                      \
    if (size > NAME2(highWaterMark,counterNameModifier)) {                                                                        \
        if (hasBeenSplit) {                                                                                                       \
            printf ("In %s, workload's high water mark size, already split among many workers, is %ldK\n", funcName, size/1000);  \
        } else {                                                                                                                  \
            printf ("In %s, workload's high water mark size, performed monolithically, is %ldK\n", funcName, size/1000);          \
        }                                                                                                                         \
        NAME2(highWaterMark,counterNameModifier) = size;                                                                          \
    }                                                                                                                             \
}
#else
#define REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME2(i,amax)),false)  // Disabled
#endif

#include "ocr-types.h"
#include <stdio.h>
#include <string.h>


typedef struct {
    u64 unitTest_shallowestLevel;     // Shallowest call depth (or EDT work forking depth) at which to do internal unit testing, i.e.
                                      // comparing "target" implementation results with "reference" implementation results.  Top level EDT is "0".
    u64 unitTest_deepestLevel;        // Deepest call depth (or EDT work forking depth) at which to do unit testing.  Unit testing disabled by
                                      // setting this variable to less than unitTest_shallowestLevel.
    u64 exactMatchExpected;           // True (non-zero) when target and reference versions should produce exact same results.  False (zero) when
                                      // there might be some floating point arithmetic rounding differences between the two.
    u64 forceDeterministicResult;     // This flag exposed to the user, who sets (true, i.e. non-zero) it when blas implementation is required to
                                      // produce identical results (with identical amounts of rounding error) from one run to the next.
} blas_control_t;

//typedef s8  BLAS_char_t;      // Type of BLAS API arguments such as transa and transb into p?gemm.
typedef s64 BLAS_int_t;       // Type of BLAS API arguments such as gemm matrix horizontal and vertical sizes, and 2D array leading dimensions.


#if defined(BLAS_TYPE__single)
typedef float BLAS_MATH_t;
typedef float BLAS_MATH_COMPONENT_t;
typedef double BLAS_UPCONVERTED_MATH_t;
typedef double BLAS_UPCONVERTED_MATH_COMPONENT_t;
#define BLAS_TYPE_PREFIX s
#define safeMinimumForReciprocation 1.0e-38   // TODO:  refine.
#define NAME(suffix)                   s ## suffix
#define NAME_COMPONENT(suffix)         s ## suffix
#define NAME2(prefix,suffix) prefix ## s ## suffix

#elif defined(BLAS_TYPE__double)
typedef double BLAS_MATH_t;
typedef double BLAS_MATH_COMPONENT_t;
typedef long double BLAS_UPCONVERTED_MATH_t;
typedef long double BLAS_UPCONVERTED_MATH_COMPONENT_t;
#define BLAS_TYPE_PREFIX d
#define safeMinimumForReciprocation 1.0e-308  // TODO:  refine.
#define NAME(suffix)                   d ## suffix
#define NAME_COMPONENT(suffix)         d ## suffix
#define NAME2(prefix,suffix) prefix ## d ## suffix

#elif defined(BLAS_TYPE__complex)
typedef struct {float real; float imag;} BLAS_MATH_t;
typedef float BLAS_MATH_COMPONENT_t;
typedef struct {double real; double imag;} BLAS_UPCONVERTED_MATH_t;
typedef double BLAS_UPCONVERTED_MATH_COMPONENT_t;
#define BLAS_TYPE_PREFIX c
#define safeMinimumForReciprocation_component    1.0e-19   // TODO:  refine.
#define safeMinimumForReciprocation_sumOfSquares 1.0e-38   // TODO:  refine.
#define NAME(suffix)                   c ## suffix
#define NAME_COMPONENT(suffix)         s ## suffix
#define NAME2(prefix,suffix) prefix ## c ## suffix

#elif defined(BLAS_TYPE__complex_double)
typedef struct {double real; double imag;} BLAS_MATH_t;
typedef double BLAS_MATH_COMPONENT_t;
typedef struct {long double real; long double imag;} BLAS_UPCONVERTED_MATH_t;
typedef long double BLAS_UPCONVERTED_MATH_COMPONENT_t;
#define BLAS_TYPE_PREFIX z
#define safeMinimumForReciprocation_component    1.0e-153  // TODO:  refine.
#define safeMinimumForReciprocation_sumOfSquares 1.0e-307  // TODO:  refine.
#define NAME(suffix)                   z ## suffix
#define NAME_COMPONENT(suffix)         d ## suffix
#define NAME2(prefix,suffix) prefix ## z ## suffix

#elif !defined(BLAS_TYPE__common)
#ERROR "BLAS_TYPE not defined to a valid selection."
#endif

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define xADD(result,input1,input2) { result  = (input1)+(input2); }
#define xMUL(result,input1,input2) { result  = (input1)*(input2); }
#define xDIV(result,input1,input2) { result  = (input1)/(input2);}
#define xRECIP(result,input) /* Either of the arguments to this macro might be of up-converted type */ { result  = 1.0;  result /= input;}
#define xFMA(result,input1,input2) { result += (input1)*(input2); }
#define xFMS(result,input1,input2) { result -= (input1)*(input2); }
#define xSET(result,input1)        { result  = (input1); }
#define xSETc(result,real,imag)    { result  = real; }
#define xCMP_NE(input1,input2)     ( (input1) != (input2))
#define xCMP_EQ(input1,input2)     ( (input1) == (input2))
#define IS_SAFE_FOR_RECIP_MUL(a)   (((a<0.0)?-a:a)>safeMinimumForReciprocation)
#define xREAL(input)               ((BLAS_MATH_t) ( input ))
#define xIMAG(input)               ((BLAS_MATH_t) 0.0)

#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
#define xADD(res,in1,in2)  {(res).real = (in1).real+(in2).real; (res).imag = (in1).imag+(in2).imag; }
#define xMUL(res,in1,in2)  {BLAS_MATH_t xxxtemp; (xxxtemp).real = (in1).real*(in2).real - (in1).imag*(in2).imag; (xxxtemp).imag = (in1).real*(in2).imag + (in1).imag*(in2).real; res=xxxtemp; }
#define xMULcr(res,in1,in2){(res).real = (in1).real*in2; (res).imag = (in1).imag*in2; }
#define xDIV(result,input1,input2) {                                                                              \
    struct {BLAS_UPCONVERTED_MATH_COMPONENT_t real; BLAS_UPCONVERTED_MATH_COMPONENT_t imag;} in1up, in2up, resup; \
    in1up.real = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input1).real;                                               \
    in1up.imag = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input1).imag;                                               \
    in2up.real = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input2).real;                                               \
    in2up.imag = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input2).imag;                                               \
    BLAS_UPCONVERTED_MATH_COMPONENT_t denom = in2up.real * in2up.real + in2up.imag * in2up.imag;                  \
    resup.real = (in1up.real * in2up.real + in1up.imag * in2up.imag) / denom;                                     \
    resup.imag = (in2up.real * in1up.imag - in1up.real * in2up.imag) / denom;                                     \
    (result).real = (BLAS_MATH_COMPONENT_t) resup.real;  (result).imag = (BLAS_MATH_COMPONENT_t) resup.imag;      \
}
#define xRECIP(result,input2) {  /* Either of the arguments to this macro might be of up-converted type */        \
    struct {BLAS_UPCONVERTED_MATH_COMPONENT_t real; BLAS_UPCONVERTED_MATH_COMPONENT_t imag;} in2up;               \
    in2up.real = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input2).real;                                               \
    in2up.imag = (BLAS_UPCONVERTED_MATH_COMPONENT_t) (input2).imag;                                               \
    BLAS_UPCONVERTED_MATH_COMPONENT_t denom = in2up.real * in2up.real + in2up.imag * in2up.imag;                  \
    (result).real =  in2up.real / denom;                                                                          \
    (result).imag = -in2up.imag / denom;                                                                          \
}
#define xFMAcr(res,in1,in2)      { (res).real += (in1).real*(in2);                              (res).imag +=                         (in1).imag*(in2);      }
#define xFMA(res,in1,in2)        { (res).real += (in1).real*(in2).real - (in1).imag*(in2).imag; (res).imag += (in1).real*(in2).imag + (in1).imag*(in2).real; }
#define xFMA_conjg1(res,in1,in2) { (res).real += (in1).real*(in2).real + (in1).imag*(in2).imag; (res).imag += (in1).real*(in2).imag - (in1).imag*(in2).real; }
#define xFMA_conjg2(res,in1,in2) { (res).real += (in1).real*(in2).real + (in1).imag*(in2).imag; (res).imag += (in1).imag*(in2).real - (in1).real*(in2).imag; }
#define xFMA_conjg12(res,in1,in2){ (res).real += (in1).real*(in2).real - (in1).imag*(in2).imag; (res).imag -= (in1).imag*(in2).real + (in1).real*(in2).imag; }
#define xFMS(res,in1,in2)        { (res).real -= (in1).real*(in2).real - (in1).imag*(in2).imag; (res).imag -= (in1).real*(in2).imag + (in1).imag*(in2).real; }
#define xSET(result,input)       { result  = input; }
#define xSETc(result,r,i)        { (result).real  = r; (result).imag = i; }
#define xCMP_NE(input1,input2)   ((input1).real != (input2).real || (input1).imag != (input2).imag)
#define xCMP_EQ(input1,input2)   ((input1).real == (input2).real && (input1).imag == (input2).imag)
#define IS_SAFE_FOR_RECIP_MUL(a) (                                                            \
    (((((a).real<0.0) ? -(a).real : a.real) > safeMinimumForReciprocation_component) &&       \
     ((((a).imag<0.0) ? -(a).imag : a.imag) > safeMinimumForReciprocation_component)) ?       \
    ((((a).real*(a).real)+((a).imag*(a).imag)) > safeMinimumForReciprocation_sumOfSquares) :  \
    false)
#define xREAL(input)             ((BLAS_MATH_COMPONENT_t) ((input).real))
#define xIMAG(input)             ((BLAS_MATH_COMPONENT_t) ((input).imag))
#define xCONJUGATE(result, input){ (result).real = (input).real; (result).imag = -(input).imag; }
#endif


// Internal API for guidified variant.  Storage for matrices has to be passed as GUIDs rather than pointers in order to implement as OCR EDTs (though for initial unit testing, this is not so).
//#ifdef BLAS_GUIDIFIED
//#define DATABLOCK_TYPE OCRGuid_t
//#else
//#define DATABLOCK_TYPE void *
//#include <stdlib.h>
//#define GETDATABLOCK malloc
//#define FREEDATABLOCK free
//#endif

#define MAKE_DB_MODE_X(mode) DB_MODE_ ## mode
#define ADD_DEPENDENCE(source,dest,depList,depSlotName,mode) ocrAddDependence(source,dest,offsetof(depList,depSlotName)/sizeof(ocrEdtDep_t),MAKE_DB_MODE_X(mode));

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a)

#define MIN(a,b)(((a)<(b)) ? a : b)
#define MAX(a,b)(((a)>(b)) ? a : b)



#if !defined(BLAS_TYPE__common)
typedef u32 SpmmmIdx_t;
typedef struct {
    u64 offendingRowNum:40;               // Row number where an error occurred.
    u64 errorDetail:20;                   // Detail about the error that occurred.
    u64 errorType:4;                      // Type of error that occurred.  (See #defines, below)
} SPMMM_RETURN_CODE_t;
#define SPMMM_ERROR__ROW_TOO_LONG 1                // Detail is how long the row was, i.e. how much ldc should be increased to, to accomodate the full row.  (Not necessarily the first, only, or most
                                                   // offensive row.  Not necessarily signaled deterministically.  Contents of row is curtailed, to fit; which non-zero elements are deleted is not necessarily
                                                   // deterministic, and no particular curtailing criteria is promised.)

void NAME(spmmm) (                     // Function-type API for sparse-matrix-sparse-matrix multiply.
    SpmmmIdx_t    m,                   // Number of rows in matrix A and in matrix C.
    SpmmmIdx_t    n,                   // Number of columns in matrix A, and number of rows in matrix B.
    SpmmmIdx_t    k,                   // Number of columns in matrix B and in matrix C.
    BLAS_MATH_t * aVal,                // Address of 2D array holding non-zero-valued elements of matrix A.
    BLAS_MATH_t * bVal,                // Address of 2D array holding non-zero-valued elements of matrix B.
    BLAS_MATH_t * cVal,                // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t  * aColIdx,             // Address of 2D array holding column indices of matrix A.
    SpmmmIdx_t  * bColIdx,             // Address of 2D array holding column indices of matrix B.
    SpmmmIdx_t  * cColIdx,             // Address of 2D array to receive column indices of matrix C.
    SpmmmIdx_t  * aNumNzColsInRow,     // Address of 1D array holding the count of column indices and values for each row of A.
    SpmmmIdx_t  * bNumNzColsInRow,     // Address of 1D array holding the count of column indices and values for each row of B.
    SpmmmIdx_t  * cNumNzColsInRow,     // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t    lda,                 // Leading dimension of aVal and aIdx arrays, aka the maximum number of non-zero-valued elements any row of A can accomodate.
    SpmmmIdx_t    ldb,                 // Leading dimension of bVal and bIdx arrays, aka the maximum number of non-zero-valued elements any row of B can accomodate.
    SpmmmIdx_t    ldc,                 // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    double        epsilon,             // When a result is between +/- epsilon, it is eliminated from the output, i.e. set to zero.
    s64           aTotalNumNzElements, // Number of nonzeroes in matrix A.  If unknown, provide intelligent guess, or -1 if totally random.
    s64           bTotalNumNzElements, // Number of nonzeroes in matrix B.  If unknown, provide intelligent guess, or -1 if totally random.
    s64         * cTotalNumNzElements, // Output: Number of nonzeroes in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow,  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode);         // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.


void NAME(gemm) (                    // GEMM performs a matrix-matrix operation with matrices stored in normal 2D matrix layout (not "distributed", i.e. tiled and/or paneled).
                                     // The operation is defined as
                                     // C := alpha*op(A)*op(B) + beta*C;
                                     // op(x) is one of op(x) = x, or op(x) = x',
                                     // alpha and beta are scalars,
    // const BLAS_LAYOUT   layout,   // Row or Column major.  Presently assumed to be CblasColMajor.
    const char           * transa,   // 'N' or 'n', op(A) = A;  'T', 't', op(A) = A^^^T; 'C', or 'c', op(A) = A^^^H;.  Presently assumed to be 'N'.
    const char           * transb,   // 'N' or 'n', op(B) = B;  'T', 't', op(B) = B^^^T; 'C', or 'c', op(B) = B^^^H;.  Presently assumed to be 'N'.
    const BLAS_int_t       m,        // Specifies the number of rows of the matrices op(A) and C.
    const BLAS_int_t       n,        // Specifies the number of columns of the matrices op(B) and C.
    const BLAS_int_t       k,        // Specifies the number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    const BLAS_MATH_t      alpha,    // Specifies the scalar alpha.  When alpha is equal to zero, then the local entries of the arrays a and b
                                     // corresponding to the entries of the submatrices sub(A) and sub(B) respectively need not be set on input.
    const BLAS_MATH_t    * a,        // Input array, size m*lda.
    const BLAS_int_t       lda,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix A.
    const BLAS_MATH_t    * b,        // Input array, size k*ldb.
    const BLAS_int_t       ldb,      // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix B.
    const BLAS_MATH_t      beta,     // When beta is equal to zero, then sub(C) need not be set on input.
          BLAS_MATH_t    * c,        // Input/Output array, size m*ldc.
    const BLAS_int_t       ldc);     // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix C.


typedef struct {                     // These are the scalar variables that are passed to the paramv argument of the GEMM EDT, which is the "native OCR" coding of GEMM.
    BLAS_int_t    m;                 // The number of rows of the matrices op(A) and C.
    BLAS_int_t    n;                 // The number of columns of the matrices op(B) and C.
    BLAS_int_t    k;                 // The number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    BLAS_int_t    offseta;           // Offset into datablock dbA (passed in as a dependency) of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    offsetb;           // Offset into datablock dbB (passed in as a dependency) of matrix B (in elements of BLAS_MATH_t).
    BLAS_int_t    offsetc;           // Offset into datablock dbC (passed in as a dependency) of matrix C (in elements of BLAS_MATH_t).
    BLAS_int_t    lda;               // Leading dimension (aka "pitch") of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;               // Leading dimension (aka "pitch") of matrix B (in elements of BLAS_MATH_t).
    BLAS_int_t    ldc;               // Leading dimension (aka "pitch") of matrix C (in elements of BLAS_MATH_t).
    BLAS_MATH_t   alpha;             // When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
    BLAS_MATH_t   beta;              // When beta is equal to zero, then sub(C) need not be set on input.
    // char          layout;            // Row or Column major.  Presently assumed to be CblasColMajor.
    char          transa;            // 'N' or 'n', op(A) = A;  'T', 't', op(A) = A^^^T; 'C', or 'c', op(A) = A^^^H;.  Presently assumed to be 'N'.
    char          transb;            // 'N' or 'n', op(B) = B;  'T', 't', op(B) = B^^^T; 'C', or 'c', op(B) = B^^^H;.  Presently assumed to be 'N'.
} NAME(gemm_edtParams_t);

typedef struct {                     // These are the dependencies that the GEMM EDT task needs satisfied before it can fire.
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   dbc;               // Datablock for matrix C.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(gemm_edtDeps_t);

ocrGuid_t NAME(gemm_task) (          // Spawns top-level worker EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


void NAME(getf2) (                   // Compute the LU factorization of a general m-by-n matrix with partial pivotion with row interechange.  This is the unblocked algorithm.
    BLAS_int_t    m,                 // Input:  Number of rows    in the matrix.
    BLAS_int_t    n,                 // Input:  Number of columns in the matrix.
    BLAS_MATH_t * mat,               // In/Out: Matrix to factor.
    BLAS_int_t    ld,                // Input:  Leading dimension, i.e. pitch from start of one row of the matrix storage construct to start of the next, in units of element type (i.e. number of BLAS_MATH_t's).
    BLAS_int_t  * pivotIdx,          // Output: Pivot indices found.  These are one-based values!
    BLAS_int_t  * info);             // Output: Diagnostic status.  Negative means an error.  If a zero-coefficient is found, we return the index


void NAME(getrf) (                   // GETRF does a right-looking LU factorization of a general M-by-N matrix using partial pivoting with row interchanges.
                                     // The factorization has the form A = P * L * U where P is a permutation matrix, L is lower triangular with unit diagonal
                                     // elements (lower trapezoidal if m > n), and U is upper triangular (upper trapezoidal if m < n).
    const BLAS_int_t    m,           // Specifies the number of rows of the matrix.
    const BLAS_int_t    n,           // Specifies the number of columns of the matrix.
    const BLAS_MATH_t * mat,         // Input array, size m*lda.
    const BLAS_int_t    ld,          // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    const BLAS_int_t  * pivotIdx,    // Output array of pivot indices.  These are one-based indices.
    const BLAS_int_t  * info);       // Output status value.  If <0, indicates error; if >0 indicates an index of a zero-coeff (in one-based form); if == 0, all completely normal.


typedef struct {                     // These are the scalar variables that are passed to the GETRF thunking EDT as its paramv argument.
    BLAS_int_t    m;                 // The number of rows of the matrix.
    BLAS_int_t    n;                 // The number of columns of the matrix.
    BLAS_int_t    ld;                // The leading dimension of the matrix (in elements of BLAS_MATH_t).
} NAME(getrf_edtParams_t);

typedef struct {                     // These are the dependencies that the GETRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;             // Datablock for matrix.
    ocrEdtDep_t   dbPivotIdx;        // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(getrf_edtDeps_t);

ocrGuid_t NAME(getrf_task) (         // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



void NAME(gesv) (                    // Compute solution to a system of linear equations A*X=B for general matrices.
    const BLAS_int_t    n,           // Input:  The number of linear equations, i.e. the order of matrix A.
    const BLAS_int_t    nrhs,        // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,           // Input:  The n*n coefficient matrix.
                                     // Output: The factors L and U from the factorization A+P*L*U.  The unit
                                     //         diagonal elements of L are not stored.
    const BLAS_int_t    lda,         // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_int_t  * pivotIdx,    // Output: The pivot indices that define the permutation matrix P;  row i of
                                     //         the matrix was interchanged with row pivotIdx[i].  Note: contents
                                     //         of the array are one-based, owing to the Fortran history.  They
                                     //         must be adjusted (decremented) to index into C's zero-based matrix.
    const BLAS_MATH_t * b,           // Input:  right hand side n-by-nhrs matrix b.
                                     // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,         // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info);       // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                     //         > 0 first one-based index along diagonal of U that is exactly zero.
                                     //         The factorization has been performed, but the factor U is exactly
                                     //         singular so the solution could not be computed.


typedef struct {                     // These are the scalar variables that are passed to the GETRF thunking EDT as its paramv argument.
    BLAS_int_t    n;                 // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;              // The number of columns of matrix B.
    BLAS_int_t    lda;               // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;               // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(gesv_edtParams_t);

typedef struct {                     // These are the dependencies that the GETRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;        // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(gesv_edtDeps_t);

ocrGuid_t NAME(gesv_task) (          // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


void NAME(getrs) (                   // Compute solution to a system of linear equations A*X=B for general matrices.
    const BLAS_int_t    trans,       // Input:  0 corresponds to "N" in reference code, meaning no transpose.
                                     //         1 corresponds to "T" or "C", meaning transpose or conjugate-transpose, which
                                     //         are treated the same by this function.
    const BLAS_int_t    n,           // Input:  The number of linear equations, i.e. the order of matrix A.
    const BLAS_int_t    nrhs,        // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,           // Input:  The n*n coefficient matrix.
                                     // Output: The factors L and U from the factorization A+P*L*U.  The unit
                                     //         diagonal elements of L are not stored.
    const BLAS_int_t    lda,         // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_int_t  * pivotIdx,    // Output: The pivot indices that define the permutation matrix P;  row i of
                                     //         the matrix was interchanged with row pivotIdx[i].  Note: contents
                                     //         of the array are one-based, owing to the Fortran history.  They
                                     //         must be adjusted (decremented) to index into C's zero-based matrix.
    const BLAS_MATH_t * b,           // Input:  right hand side n-by-nhrs matrix b.
                                     // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,         // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info);       // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                     //         > 0 first one-based index along diagonal of U that is exactly zero.
                                     //         The factorization has been performed, but the factor U is exactly
                                     //         singular so the solution could not be computed.


typedef struct {                     // These are the scalar variables that are passed to the GETRF thunking EDT as its paramv argument.
    BLAS_int_t    trans;             // The value of the TRANS argument, i.e. 1 (true) corresponds to "T" in reference version, to transpose, 0 (false) corresponds to "N" to NOT transpose.  We want "N".
    BLAS_int_t    n;                 // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;              // The number of columns of matrix B.
    BLAS_int_t    lda;               // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;               // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(getrs_edtParams_t);

typedef struct {                     // These are the dependencies that the GETRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;        // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(getrs_edtDeps_t);

ocrGuid_t NAME(getrs_task) (         // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


void NAME(potf2) (
    char        * uplo,              // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
    BLAS_int_t    n,                 // The number of rows and columns of the n-by-n matrix.
    BLAS_MATH_t * a,                 // Input array, size n*lda.
    BLAS_int_t    lda,               // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    BLAS_int_t  * pInfo);            // Output status value.  If <0, this indicates an error condition.  If >0, this is a one-based index of a zero coefficient.  If 0, completely normal.


void NAME(potrf) (                   // POTRF computes the Cholesky factorization of a symmetric positive definite matrix A.
                                     // The factorization has the form A = U**T * U if UPLO = 'U',   or A = L * L**T if UPLO = 'L',
                                     //  where U is an upper triangular matrix and L is a lower triangular matrix.
    char        * uplo,              // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
    BLAS_int_t    n,                 // Specifies the number of columns of the matrix.
    BLAS_MATH_t * mat,               // Input array, size m*lda.
    BLAS_int_t    ld,                // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    BLAS_int_t  * info);             // Output status value.  If <0, indicates error; if >0 indicates an index of a zero-coeff (in one-based form); if == 0, all completely normal.


typedef struct {                     // These are the scalar variables that are passed to the POTRF thunking EDT as its paramv argument.
    char        * uplo;              // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
    BLAS_int_t    n;                 // The number of rows and columns of the matrix.
    BLAS_int_t    ld;                // The leading dimension of the matrix (in elements of BLAS_MATH_t).
} NAME(potrf_edtParams_t);

typedef struct {                     // These are the dependencies that the POTRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;             // Datablock for matrix.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the POTRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(potrf_edtDeps_t);

ocrGuid_t NAME(potrf_task) (         // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


void NAME(posv) (                    // Compute solution to a system of linear equations A*X=B for positive definite matrices.
    const char        * uplo,        // Input:  "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    const BLAS_int_t    n,           // Input:  The number of linear equations, i.e. the order of matrix A.
    const BLAS_int_t    nrhs,        // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,           // Input:  The n*n coefficient matrix.  Either the upper or lower triangle, as guided by uplo, is referenced
                                     // Output: If info==0, the factors L and U from the factorization A=U**T*U, A=U**H*U, A=L*L**T, or A=L*L**H.
    const BLAS_int_t    lda,         // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_MATH_t * b,           // Input:  right hand side n-by-nhrs matrix b.
                                     // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,         // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info);       // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                     //         > 0 One-based index i, the leading minor of order i of A is not
                                     //         positive definite, so the factorization could not be completed and
                                     //         the solution has not been computed.

typedef struct {                     // These are the scalar variables that are passed to the GETRF thunking EDT as its paramv argument.
    const char  * uplo;              // Input:  "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    BLAS_int_t    n;                 // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;              // The number of columns of matrix B.
    BLAS_int_t    lda;               // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;               // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(posv_edtParams_t);

typedef struct {                     // These are the dependencies that the POTRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(posv_edtDeps_t);

ocrGuid_t NAME(posv_task) (          // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


void NAME(potrs) (  // Compute solution to a system of linear equations A*X=B for symmetric/Hermitian positive definite matrix A.
    const char        * uplo,        // Input: "U" or "u" -- upper triangle of A; "L" or "l" -- lower triangle of A.
    const BLAS_int_t    n,           // Input:  The order of matrix A.
    const BLAS_int_t    nrhs,        // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,           // Input:  The results from factorization, by ?potrf.
    const BLAS_int_t    lda,         // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_MATH_t * b,           // Input:  right hand side n-by-nhrs matrix b.
                                     // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,         // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info);       // Output: 0 == success; < 0 negation of which argument had an illegal value.

typedef struct {                     // These are the scalar variables that are passed to the POTRF thunking EDT as its paramv argument.
    const char  * uplo;              // "U" or "u" -- upper triangle of A; "L" or "l" -- lower triangle of A.
    BLAS_int_t    n;                 // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;              // The number of columns of matrix B.
    BLAS_int_t    lda;               // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;               // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(potrs_edtParams_t);

typedef struct {                     // These are the dependencies that the POTRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;            // Datablock for returning info about the POTRF status.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(potrs_edtDeps_t);

ocrGuid_t NAME(potrs_task) (         // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



void NAME(trsm) (
    char        * side,     // "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    char        * uplo,     // "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    char        * transa,   // "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T
    char        * diag,     // "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.
    BLAS_int_t    m,        // Number of rows of B.
    BLAS_int_t    n,        // Number of columns of B.
    BLAS_MATH_t   alpha,
    BLAS_MATH_t * a,        // Matrix of size lda*k, where k = ((side[0]=='L'||side[0]=='l') ? m : n
    BLAS_int_t    lda,      // Leading dimension (aka "pitch") of matrix A, in units of BLAS_MATH_t
    BLAS_MATH_t * b,        // Matrix of size ldb*n
    BLAS_int_t    ldb);     // Leading dimension (aka "pitch") of matrix B, in units of BLAS_MATH_t

typedef struct {
    BLAS_int_t    m;                 // Number of rows of B.
    BLAS_int_t    n;                 // Number of columns of B.
    BLAS_MATH_t   alpha;             // Scalar by which to multiply each matrix B value before it is modified.
    BLAS_int_t    lda;               // The leading dimension of the matrix A.
    BLAS_int_t    ldb;               // The leading dimension of the matrix B.
    BLAS_int_t    offseta;           // Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
    BLAS_int_t    offsetb;           // Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
    char          side;              // "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    char          uplo;              // "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    char          transa;            // "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T
    char          diag;              // "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.
} NAME(trsm_edtParams_t);

typedef struct {
    ocrEdtDep_t   dba;               // Datablock for matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(trsm_edtDeps_t);

ocrGuid_t NAME(trsm_task) (          // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);
#endif


// ****************************************************************************************************************************
// Service functions.  These BLAS routines are low-order enough not to bother spawing as EDTs, but instead are simply called as normal functions.

/**
 * @brief xerbla reports error condition
 **/

void xerbla (char errorSource[], BLAS_int_t errorDetail);

/**
 * @brief ilaenv returns BLAS-specific environment variable values.
 */

BLAS_int_t ilaenv(
    BLAS_int_t ispec,
    char * name,
    char * opts,
    BLAS_int_t n1,
    BLAS_int_t n2,
    BLAS_int_t n3,
    BLAS_int_t n4);


/**
 * @brief sisnan and disnan indicate whether argument is NAN.
 **/

#define sisnan(f) ((((*((u32 *)&f)) & 0x7F800000) == 0x7F800000) && (((*((u32 *)&f)) & 0x7FFFFF) != 0))
#define disnan(d) ((((*((u64 *)&d)) & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) && (((*((u64 *)&d)) & 0xFFFFFFFFFFFFFULL) != 0LL))


#if !defined(BLAS_TYPE__common)
/**
 * @brief Implementation of service function i?amax, where ? is
 * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 * This function returns the index of the first element of a vector with
 * the maximum magnitude.  In the case of complex versions, "maximum" means the largest Euclidean norm, i.e.
 * greatest sum of the squares of the real and imaginary components.
 **/

BLAS_int_t NAME2(i,amax) (BLAS_int_t vectorLen, BLAS_MATH_t * vector, BLAS_int_t stride);


/**
 * @brief Implementation of service function ?ger, where ? is
 * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 * This function calculates A += alpha*x*y, where A is an m-by-n matrix, x is an m-element vector, and y is an n-element vector.
 **/

void NAME(ger)(
    BLAS_int_t    m,     // numRows of matrix A; length of vector x
    BLAS_int_t    n,     // numCols of matrix A; length of vector y
    BLAS_MATH_t   alpha,
    BLAS_MATH_t * x,
    BLAS_int_t    incx,
    BLAS_MATH_t * y,
    BLAS_int_t    incy,
    BLAS_MATH_t * a,
    BLAS_int_t    lda);

/**
 * @brief Implementation of service function ?scal, where ? is
 * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 * This function scales a vector by a constant
 **/

void NAME(scal)(
    BLAS_int_t    n,     // number of elements to scale
    BLAS_MATH_t   alpha, // scaling factor
    BLAS_MATH_t * x,     // vector to scale
    BLAS_int_t    incx); // stride of vector

#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
void NAME_COMPONENT(scal)(
    BLAS_int_t              n,     // number of elements to scale
    BLAS_MATH_COMPONENT_t   alpha, // scaling factor
    BLAS_MATH_COMPONENT_t * x,     // vector to scale
    BLAS_int_t              incx); // stride of vector
#endif

void NAME(scal_upconvertedAlpha)(
    BLAS_int_t                n,     // number of elements to scale
    BLAS_UPCONVERTED_MATH_t   alpha, // scaling factor
    BLAS_MATH_t             * x,     // vector to scale
    BLAS_int_t                incx); // stride of vector

/**
 * @brief Implementation of service function ?swap, where ? is
 * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 * This function swaps two vectors
 **/

void NAME(swap)(
    BLAS_int_t    n,     // number of elements in the two vectors to swap
    BLAS_MATH_t * x,     // first vector to scale
    BLAS_int_t    incx,  // stride of first vector
    BLAS_MATH_t * y,     // second vector to scale
    BLAS_int_t    incy); // stride of second vector

/**
 * @brief ?laswp performs a series of row interchages on a matrix.  One row interchage is done for each of rows k1 through k2.
 **/

void NAME(laswp)(
    BLAS_int_t    n,        // Number of columns in the matrix.
    BLAS_MATH_t * mat,      // Matrix on which to perform the row interchanges.
    BLAS_int_t    ld,       // Leading dimension (aka "pitch") of matrix, in units of BLAS_MATH_t, i.e. element size.
    BLAS_int_t    k1,       // First element of pivotIdx for which a row interchange will be done.  This is a zero-based index into the pivotIdx array and the row of the matrix.
    BLAS_int_t    k2,       // Last  element of pivotIdx for which a row interchange will be done.  This is a zero-based index into the pivotIdx array and the row of the matrix.
    BLAS_int_t  * pivotIdx, // Vector of pivot indices.  The values in this vector are one-based.
    BLAS_int_t    incx);    // Increment between successive values of pivotIdx.  If negative, pivots applied in reverse order.

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
/**
 * @brief sdot or ddot computes dot product of real vectors.
 **/
BLAS_MATH_t NAME(dot)(      // Compute dot product.
    BLAS_int_t    n,        // Number of elements in the two vectors.
    BLAS_MATH_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t    incy);    // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.

/**
 * @brief ?syrk performs one of the symmetric rank k operations: C = alpha*A*A**T = beta*C, or C = alpha*A**T*A = beta*C
 *  where alpha and beta are scalars, C is an n-by-n symmetric matrix, and A is n-by-k (in the first case) or k-by-n (in the second case).
 **/
void NAME(syrk)(
    char        * uplo,     // 'U' or 'u' -- only the upper triangle of C is to be referenced;  'L' or 'l' -- only the lower part...
    char        * trans,    // 'N' or 'n' -- C=alpha*A*A**T + beta*C;  'T', 't', 'C', or 'c' -- C=alpha*A**T*A = beta*C.
    BLAS_int_t    n,        // Order of matrix C.  Must be >= 0.
    BLAS_int_t    k,        // If no transpose, number of columns in matrix A; if transpose, number of rows.  Must be >= 0.
    BLAS_MATH_t   alpha,    // Scalar multiplier.
    BLAS_MATH_t * a,        // Triangular input matrix.
    BLAS_int_t    lda,      // Leading dimension of A.
    BLAS_MATH_t   beta,     // Scalar multiplier.
    BLAS_MATH_t * c,        // Triangular input/output matrix.
    BLAS_int_t    ldc);     // Leading dimension of C.
#else
/**
 * @brief sdot or ddot, function prototypes for when used by complex and complex-double higher-level functions.
 **/
BLAS_MATH_COMPONENT_t NAME_COMPONENT(dot)(  // Compute dot product of real vectors.  (Used to get float/double dot product of real component or imaginary component).
    BLAS_int_t              n,        // Number of elements in the two vectors.
    BLAS_MATH_COMPONENT_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).
    BLAS_int_t              incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_COMPONENT_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t              incy);    // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.

/**
 * @brief cdotu or zdotu computes dot product of complex vectors.
 **/
BLAS_MATH_t NAME(dotu)(     // Compute dot product.
    BLAS_int_t    n,        // Number of elements in the two vectors.
    BLAS_MATH_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t    incy);    // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.

/**
 * @brief cdotc or zdotc computes dot product of complex vectors, conjugating the first vector.
 **/
BLAS_MATH_t NAME(dotc)(
    BLAS_int_t    n,        // Number of elements in the two vectors.
    BLAS_MATH_t * x,        // Address of vector X (i.e. of the first (last, if incx<0) element to participate in dot product calculation).  We will conjugate this operand.
    BLAS_int_t    incx,     // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
    BLAS_MATH_t * y,        // Address of vector Y (i.e. of the first (last, if incy<0) element to participate in dot product calculation).
    BLAS_int_t    incy);    // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.

/**
 * @brief ?herk performs one of the hermetian rank k operations: C = alpha*A*A**H = beta*C, or C = alpha*A**H*A = beta*C
 *  where alpha and beta are scalars, C is an n-by-n hermetian matrix, and A is n-by-k (in the first case) or k-by-n (in the second case).
 **/
void NAME(herk)(
    char                  * uplo,     // 'U' or 'u' -- only the upper triangle of C is to be referenced;  'L' or 'l' -- only the lower part...
    char                  * trans,    // 'N' or 'n' -- C=alpha*A*A**T + beta*C;  'T', 't', 'C', or 'c' -- C=alpha*A**T*A = beta*C.
    BLAS_int_t              n,        // Order of matrix C.  Must be >= 0.
    BLAS_int_t              k,        // If no transpose, number of columns in matrix A; if transpose, number of rows.  Must be >= 0.
    BLAS_MATH_COMPONENT_t   alpha,    // Scalar multiplier.
    BLAS_MATH_t           * a,        // Triangular input matrix.
    BLAS_int_t              lda,      // Leading dimension of A.
    BLAS_MATH_COMPONENT_t   beta,     // Scalar multiplier.
    BLAS_MATH_t           * c,        // Triangular input/output matrix.
    BLAS_int_t              ldc);     // Leading dimension of C.
#endif

/**
 * @brief ?gemv performs one of these matrix-vector operations:   y=alpha*A*x+beta*y or y=alpha*A**T*x+beta*y.
 **/
void NAME(gemv)(
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
    BLAS_int_t    incy);    // Stride (in elements of type BLAS_MATH_t) from one element of Y to the next.  If <0, traverse vector Y in reverse order.


#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
/**
 * @brief ?lacgv conjugates a complex vector
 **/
void NAME(lacgv)(
    BLAS_int_t    n,        // Number of elements in vector X.
    BLAS_MATH_t * x,        // Address of vector X.
    BLAS_int_t    incx);    // Stride (in elements of type BLAS_MATH_t) from one element of X to the next.  If <0, traverse vector X in reverse order.
#endif
#endif

#endif /* __BLAS_H__ */
