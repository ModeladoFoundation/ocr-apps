/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
//#define BATCHING   -- Use this ONLY when running test_gemm, and only when doing so in batching mode, i.e. when top line of test_gemm.c is "#if 1".

#define CODEVERSION 4
#define IS_UNIT_TESTING
#define TRAVERSE_TOPOLOGY_DEPTH_FIRST   // uncomment this in order to force child workers to serialize, left-before-right, upper-before-lower, so that debugging messages come out in an easier-to-read order.
                                        // ****** WARNING!!!  When workloads are split left-and-right or upper-and-lower, the posturing of subblocks still happens in non-deterministic order.
// The following defines are used just as a device to make it easier to see which CODEVERSION case various statements relate to.  It saves having to look up which switch case the annotated code is in.
#define C1 /**/
#define C2 /**/
#define C3 /**/
#define C4 /**/

#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>

/**
 * @brief Implementation of a BLAS GEnereal Matrix Multiply function of precision ? -- p?gemm,
 * where ? is * <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 **/

//**************************************************************************************
//
//   Function:     ?gemm                 Externally-accessible Standard BLAS API.
//                                       1) Create datablocks for A, B, and C.
//                                       2) Spawn gemmThunkTask to populate datablocks for A, B, and C.  It in turn spawns top-level gemm worker, written with a native OCR API.
//                                       Makes use of strawman routines for a proposed OCR API to do subblock "posturing".  See ocr-posture.h.
//

#define numRowsOpA_numRowsC   m
#define numColsOpB_numColsC   n
#define numColsOpA_numRowsOpB k
#define numColsOpA     numColsOpA_numRowsOpB
#define numRowsOpA     numRowsOpA_numRowsC
#define numColsOpB     numColsOpB_numColsC
#define numRowsOpB     numColsOpA_numRowsOpB
#define numColsC       numColsOpB_numColsC
#define numRowsC       numRowsOpA_numRowsC
#define numColsC_left  numColsOpB_left
#define numRowsC_upper numRowsOpA_upper

typedef struct {                            // These are the scalar variables that are passed to the GEMM thunking EDT as its paramv argument.
    BLAS_int_t    numRowsOpA_numRowsC;      // The number of rows of the matrices op(A) and C.
    BLAS_int_t    numColsOpB_numColsC;      // The number of columns of the matrices op(B) and C.
    BLAS_int_t    numColsOpA_numRowsOpB;    // The number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    BLAS_int_t    lda_datablock;            // Leading dimension of matrix A (as represented in the thunked datablock) (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;            // Leading dimension of matrix B (as represented in the thunked datablock) (in elements of BLAS_MATH_t).
    BLAS_int_t    ldc_datablock;            // Leading dimension of matrix C (as represented in the thunked datablock) (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_inputArray;           // Leading dimension of matrix A (as passed in from the user's call) (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_inputArray;           // Leading dimension of matrix B (as passed in from the user's call) (in elements of BLAS_MATH_t).
    BLAS_int_t    ldc_outputArray;          // Leading dimension of matrix C (as passed in from the user's call) (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                        // Address of input matrix A.
    BLAS_MATH_t * b;                        // Address of input matrix B.
    BLAS_MATH_t * c;                        // Address of input/output matrix C.
    BLAS_MATH_t   alpha;                    // When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
    BLAS_MATH_t   beta;                     // When beta is equal to zero, then sub(C) need not be set on input.
    // char          layout;                // Row or Column major.  Presently assumed to be CblasColMajor.
    char          transa;                   // 'N' or 'n', op(A) = A;  'T', 't', op(A) = A**T; 'C', or 'c', op(A) = A**H;.
    char          transb;                   // 'N' or 'n', op(B) = B;  'T', 't', op(B) = B**T; 'C', or 'c', op(B) = B**H;.
} NAME(gemm_thunkParams_t);

typedef struct {                            // These are the dependencies that the GEMM thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                      // Datablock for matrix A.
    ocrEdtDep_t   dbb;                      // Datablock for matrix B.
    ocrEdtDep_t   dbc;                      // Datablock for matrix C.
} NAME(gemm_thunkDeps_t);

static ocrGuid_t NAME(gemm_thunkTask) (     // Moves array-based input matrices A, B, and C (passed by standard GEMM API) into OCR datablocks, then spawns top-level worker EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                            // These are the scalar variables that are passed to the GEMM thunking and child EDTs as their paramv argument.
    BLAS_int_t    numRowsOpA_numRowsC;      // The number of rows of the matrices op(A) and C.
    BLAS_int_t    numColsOpB_numColsC;      // The number of columns of the matrices op(B) and C.
    BLAS_int_t    numColsOpA_numRowsOpB;    // The number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    BLAS_int_t    lda;                      // Leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;                      // Leading dimension of matrix B (in elements of BLAS_MATH_t).
    BLAS_int_t    ldc;                      // Leading dimension of matrix C (in elements of BLAS_MATH_t).
    BLAS_int_t    offseta;                  // Distance (in elements BLAS_MATH_T) from A[0][0] to upper left element of A to process.
    BLAS_int_t    offsetb;                  // Distance (in elements BLAS_MATH_T) from B[0][0] to upper left element of B to process.
    BLAS_int_t    offsetc;                  // Distance (in elements BLAS_MATH_T) from C[0][0] to upper left element of C to process.
    BLAS_MATH_t   alpha;                    // When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
    BLAS_MATH_t   beta;                     // When beta is equal to zero, then sub(C) need not be set on input.
    ocrGuid_t     backingStoreA;            // Datablock for backingStore of matrix A.
    ocrGuid_t     backingStoreB;            // Datablock for backingStore of matrix B.
    ocrGuid_t     backingStoreC;            // Datablock for backingStore of matrix C.
    ocrGuid_t     workerTemplate;           // GUID of EdtTemplate for the worker.  Passed down so that worker EDT's can be instantiated.
    ocrGuid_t     collectorTemplate;        // GUID of EdtTemplate for the worker collector.  Passed down so that worker EDT's can be instantiated.
    ocrGuid_t     posture3dInwardTemplate;  // GUID of EdtTemplate for posturing 3d subblocks, i.e. POTENTIALLY migrating inputs closer to the computation core.
    ocrGuid_t     posture3dOutwardTemplate; // GUID of EdtTemplate for posturing 3d subblocks, back out to the backing-store (if they were migrated inward earlier).
    // char          layout;                // Row or Column major.  Presently assumed to be CblasColMajor.
    bool          transposeA;               // True if need to transpose matrix A; false otherwise.
    bool          transposeB;               // True if need to transpose matrix B; false otherwise.
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    bool          conjugateA;               // If transposing matrix A, true if need to conjugate it too; false otherwise.
    bool          conjugateB;               // If transposing matrix A, true if need to conjugate it too; false otherwise.
#endif
} NAME(gemm_workerParams_t);

typedef struct {                            // These are the dependencies that the GEMM thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dbPosturedA;              // Acts as trigger for dependency on postured submatrix of A.  Value is either guid of migrated subblock, or guid of backingStore if
                                            // migration not done and dependent EDT has to use original backing-store containing the subblock.
    ocrEdtDep_t   dbPosturedB;              // Acts as trigger for dependency on postured submatrix of B.  Value is either guid of migrated subblock, or guid of backingStore if
                                            // migration not done and dependent EDT has to use original backing-store containing the subblock.
    ocrEdtDep_t   dbPosturedC;              // Acts as trigger for dependency on postured submatrix of C.  Value is either guid of migrated subblock, or guid of backingStore if
                                            // migration not done and dependent EDT has to use original backing-store containing the subblock.
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
    ocrEdtDep_t   serializationEvent;       // When enabled, this event assures left-before-right, upper-before-lower execution order, so debug messages better ordered.
#endif
} NAME(gemm_workerDeps_t);

static ocrGuid_t NAME(gemm_workerTask) (    // This EDT either performs the GEMM (GEneral Matrix Multiply) workload on a set of sub-matrices, or spawns two children and splits the workload between them.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                            // These are the scalar variables that are passed to the GEMM collector EDT as its paramv argument.
    BLAS_int_t    dummy;                    // Presently no params are needed.
} NAME(gemm_collectorParams_t);

typedef struct {                            // These are the dependencies that the GEMM worker collector task needs satisfied before it can fire.
    ocrEdtDep_t   child1AorBDepostured;     // Event that the < left postured submatrix of B | upper of A> has been depostured back to the backing store (i.e. deleted).
    ocrEdtDep_t   child2AorBDepostured;     // Event that the <right postured submatrix of B | lower of A> has been depostured back to the backing store (i.e. deleted).
    ocrEdtDep_t   child1CDepostured;        // Event that the < left postured submatrix of C | upper of C> has been depostured back to the backing store (i.e. deleted).
    ocrEdtDep_t   child2CDepostured;        // Event that the <right postured submatrix of C | lower of C> has been depostured back to the backing store (i.e. deleted).
} NAME(gemm_collectorDeps_t);

typedef struct {                            // These are the dependencies that the GEMM worker collector task needs satisfied before it can fire.
    ocrEdtDep_t   leftDone;                 // Event that the left worker is done.
    ocrEdtDep_t   rightDone;                // Event that the right worker is done.
    ocrEdtDep_t   stub1ForC3AndBeyond;
    ocrEdtDep_t   stub2ForC3AndBeyond;
} NAME(gemm_collectorDepsC2_t);             // These dependencies evolved into something else for case 3 and beyond.  This is a hack to contort case 2 and earlier into the form of case 3's depenencies.

static ocrGuid_t collectorTask (            // This EDT collects the results of the two workers spawned by a single worker that split its workload into two children.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                            // These are the scalar variables that are passed to the wrapup EDT as its paramv argument.  Wrapup is used when the standard API is thunked into the native OCR API.
    BLAS_int_t    numRowsOpA_numRowsC;      // The number of rows of the matrices op(A) and C.
    BLAS_int_t    numColsOpB_numColsC;      // The number of columns of the matrices op(B) and C.
    BLAS_int_t    numColsOpA_numRowsOpB;    // The number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    BLAS_int_t    lda_datablock;            // Leading dimension of matrix A (as represented in the thunked datablock).
    BLAS_int_t    ldb_datablock;            // Leading dimension of matrix B (as represented in the thunked datablock).
    BLAS_int_t    ldc_datablock;            // Leading dimension of matrix C (as represented in the thunked datablock).
    BLAS_int_t    ldc_outputArray;          // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix C (as passed in from the user's call).
    BLAS_MATH_t * c;                        // Destination array pointer.
    BLAS_MATH_t   alpha;                    // Specifies the scalar alpha.  When alpha is equal to zero, then the local entries of the arrays a and b need not be set on input.
    BLAS_MATH_t   beta;                     // When beta is equal to zero, then sub(C) need not be set on input.
    // char          layout;                // Row or Column major.  Presently assumed to be CblasColMajor.
    bool          transposeA;               // True if need to transpose matrix A; false otherwise.
    bool          transposeB;               // True if need to transpose matrix B; false otherwise.
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    bool          conjugateA;               // If transposing matrix A, true if need to conjugate it too; false otherwise.
    bool          conjugateB;               // If transposing matrix A, true if need to conjugate it too; false otherwise.
#endif
} NAME(gemm_wrapupParams_t);

typedef struct {                            // These are the dependencies that the GEMM wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                      // Datablock for matrix A.
    ocrEdtDep_t   dbb;                      // Datablock for matrix B.
    ocrEdtDep_t   dbc;                      // Datablock for matrix C.
    ocrEdtDep_t   event_ThunkToWrapup;      // Output event.
} NAME(gemm_wrapupDeps_t);

static ocrGuid_t NAME(gemm_wrapupTask) (    // This EDT gains control when all GEMM worker EDTs are done.  It "dethunks" the output matrix C from the OCR datablock to the original array passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv);


// Externally visible API for ?GEMM library function, simplified for getting started as follows:
//    1) Layout  assumed to be CblasColMajor.
// This version differs from the standard API in the following important ways:
//    1) The results of the matrix multiply are NOT available to the caller of this function upon return.  Rather,
//       they are available to a downstream EDT created by the caller, dependent upon p?GEMM_wrapup.  In order to
//       provide a truly standard API, we need to craft a way for this function to spawn the first EDT of some
//       topology, and then wait for an output signal from the ending EDT of the topology.  (TODO!)
// The following innefficiencies are known issues:
//    1) It is presently necessary for the top level "thunk" EDT to copy the input matrices (A, B, and C)
//       from array storage to datablock storage, and this copy operation is rather wasteful.  It is made necessary
//       by the fact that the standard-API inputs are array storage, not guidified datablocks. (TODO)
//    2) Likewise, it is necessary for the bottom level "dethunk" EDT to copy the output matrix (C) out of the
//       output datablock back to the output array storage for the matrix.  This is likewise wasteful.  (TODO)
//    3) Ultimately, a means to signify a datablock that is mapped to very specific sequences of XE general
//       registers will be necessary.  This will allow OCR to do the DMA of sub-matrix data to those general
//       registers, and then the XE code to access register-resident data, without having to do loads from and
//       stores back to L1 or any other "real" memory level. (TODO)
//
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input matrices A and B, and inpupt/output matrix C are copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.

void NAME(gemm) ( // GEMM performs a matrix-matrix operation with matrices stored in normal 2D matrix layout (not "distributed", i.e. tiled and/or paneled).
                  // The operation is defined as
                  // C := alpha*op(A)*op(B) + beta*C;
                  // op(x) is one of op(x) = x, or op(x) = x',
                  // alpha and beta are scalars,
    // const char        * layout,                // Row or Column major.  Presently assumed to be CblasColMajor.
    const char        * transa,                // 'N' or 'n', op(A) = A;  'T', 't', op(A) = A**T; 'C', or 'c', op(A) = A**H;.
    const char        * transb,                // 'N' or 'n', op(B) = B;  'T', 't', op(B) = B**T; 'C', or 'c', op(B) = B**H;.
    const BLAS_int_t    numRowsOpA_numRowsC,   // The number of rows of the matrices op(A) and C.
    const BLAS_int_t    numColsOpB_numColsC,   // The number of columns of the matrices op(B) and C.
    const BLAS_int_t    numColsOpA_numRowsOpB, // The number of columns of the matrix op(A) and the number of rows of the matrix op(B).
    const BLAS_MATH_t   alpha,                 // Specifies the scalar alpha.  When alpha is equal to zero, then the local entries of the arrays a and b
                                               // corresponding to the entries of the submatrices sub(A) and sub(B) respectively need not be set on input.
    const BLAS_MATH_t * a,                     // Input array, size m*lda.
    const BLAS_int_t    lda,                   // Leading dimension of matrix A.
    const BLAS_MATH_t * b,                     // Input array, size k*ldb
    const BLAS_int_t    ldb,                   // Leading dimension of matrix B.
    const BLAS_MATH_t   beta,                  // When beta is equal to zero, then sub(C) need not be set on input.
          BLAS_MATH_t * c,                     // Input/Output array, size m*ldc.
    const BLAS_int_t    ldc)                   // Leading dimension of matrix C.
{

//    printf("Entered Standard API %s function.\n", STRINGIFY(NAME(gemm))); fflush(stdout);

// Create the datablocks:

    ocrGuid_t  dba;    // Guid of datablock for matrix A
    ocrGuid_t  dbb;    // Guid of datablock for matrix B
    ocrGuid_t  dbc;    // Guid of datablock for matrix C

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    bool conjugateA = (transa[0] == 'C' || transa[0] == 'c');
    bool conjugateB = (transb[0] == 'C' || transb[0] == 'c');
#endif
    bool transposeA = (transa[0] != 'N' && transa[0] != 'n');
    bool transposeB = (transb[0] != 'N' && transb[0] != 'n');

#if 0
    //  TODO: Probably no need to round to full cache lines, but if needed, this works only for double and complex.
    BLAS_int_t lda_datablock     = ((!transposeA ? numColsOpA : numRowsOpA) + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatAInBytes = ((!transposeA ? numRowsOpA : numColsOpA) * lda_datablock) * sizeof(BLAS_MATH_t);
    BLAS_int_t ldb_datablock     = ((!transposeB ? numColsOpB : numRowsOpB));
    BLAS_int_t sizeOfMatBInBytes = ((!transposeB ? numRowsOpB : numColsOpB) * ldb_datablock) * sizeof(BLAS_MATH_t);
    BLAS_int_t ldc_datablock     = (numColsC  + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatCInBytes = (numRowsC * ldc_datablock) * sizeof(BLAS_MATH_t);
#else
    BLAS_int_t lda_datablock     = ((!transposeA ? numColsOpA : numRowsOpA));
    BLAS_int_t sizeOfMatAInBytes = numRowsOpA * numColsOpA * sizeof(BLAS_MATH_t);
    BLAS_int_t ldb_datablock     = ((!transposeB ? numColsOpB : numRowsOpB));
    BLAS_int_t sizeOfMatBInBytes = numRowsOpB * numColsOpB * sizeof(BLAS_MATH_t);
    BLAS_int_t ldc_datablock     = numColsC;
    BLAS_int_t sizeOfMatCInBytes = numColsC * numRowsC * sizeof(BLAS_MATH_t);
#endif
    ocrDbCreate (&dba, (void**) &addr, sizeOfMatAInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&dbb, (void**) &addr, sizeOfMatBInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&dbc, (void**) &addr, sizeOfMatCInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t gemmThunkTemplate;
    NAME(gemm_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&gemmThunkTemplate, NAME(gemm_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(gemm_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the matrix multiply result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t gemmWrapupTemplate;
    NAME(gemm_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&gemmWrapupTemplate, NAME(gemm_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(gemm_wrapupDeps_t))/sizeof(ocrEdtDep_t));


// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.numRowsOpA_numRowsC      = numRowsOpA_numRowsC;
    thunkParams.numColsOpB_numColsC      = numColsOpB_numColsC;
    thunkParams.numColsOpA_numRowsOpB    = numColsOpA_numRowsOpB;
    thunkParams.lda_datablock            = lda_datablock;
    thunkParams.ldb_datablock            = ldb_datablock;
    thunkParams.ldc_datablock            = ldc_datablock;
    thunkParams.lda_inputArray           = lda;
    thunkParams.ldb_inputArray           = ldb;
    thunkParams.ldc_outputArray          = ldc;
    thunkParams.a                        = (BLAS_MATH_t *) a;
    thunkParams.b                        = (BLAS_MATH_t *) b;
    thunkParams.c                        = (BLAS_MATH_t *) c;
    thunkParams.alpha                    = alpha;
    thunkParams.beta                     = beta;
    thunkParams.transa                   = transa[0];
    thunkParams.transb                   = transb[0];

    ocrGuid_t gemmThunkEdt;
    ocrGuid_t gemmThunkOutputEvent;
    ocrEdtCreate(&gemmThunkEdt, gemmThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix C datablock to the array that the user originally passed into the standard API GEMM func.
    wrapupParams.numRowsOpA_numRowsC     = numRowsOpA_numRowsC;
    wrapupParams.numColsOpB_numColsC     = numColsOpB_numColsC;
    wrapupParams.numColsOpA_numRowsOpB   = numColsOpA_numRowsOpB;
    wrapupParams.lda_datablock           = lda_datablock;
    wrapupParams.ldb_datablock           = ldb_datablock;
    wrapupParams.ldc_datablock           = ldc_datablock;
    wrapupParams.ldc_outputArray         = ldc;
    wrapupParams.c                       = c;
    wrapupParams.alpha                   = alpha;
    wrapupParams.beta                    = beta;
    wrapupParams.transposeA              = transposeA;
    wrapupParams.transposeB              = transposeB;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    wrapupParams.conjugateA              = conjugateA;
    wrapupParams.conjugateB              = conjugateB;
#endif

    ocrGuid_t gemmWrapupEdt;
    ocrEdtCreate(&gemmWrapupEdt, gemmWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                  gemmWrapupEdt, NAME(gemm_wrapupDeps_t), dba, RO);
    ADD_DEPENDENCE(dbb,                  gemmWrapupEdt, NAME(gemm_wrapupDeps_t), dbb, RO);
    ADD_DEPENDENCE(dbc,                  gemmWrapupEdt, NAME(gemm_wrapupDeps_t), dbc, RW);
    ADD_DEPENDENCE(gemmThunkOutputEvent, gemmWrapupEdt, NAME(gemm_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                  gemmThunkEdt, NAME(gemm_thunkDeps_t), dba, RW);
    ADD_DEPENDENCE(dbb,                  gemmThunkEdt, NAME(gemm_thunkDeps_t), dbb, RW);
    ADD_DEPENDENCE(dbc,                  gemmThunkEdt, NAME(gemm_thunkDeps_t), dbc, RW);
//    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GEMM operation.\n", STRINGIFY(NAME(gemm))); fflush(stdout);
} // ?gemm


// Receiving array-based input matrices A, B, and C from the standard API for GEMM, this EDT moves them into OCR datablocks, then passes them to a top-level child worker EDT.
static ocrGuid_t NAME(gemm_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
//    printf ("Entered %s\n", STRINGIFY(NAME(gemm_thunkTask))); fflush(stdout);

    NAME(gemm_thunkParams_t) * thunkParams = (NAME(gemm_thunkParams_t) *) paramv;
    NAME(gemm_thunkDeps_t)   * thunkDeps   = (NAME(gemm_thunkDeps_t)   *) depv;

    u64 i, j;
    const BLAS_MATH_t * pA_arr = thunkParams->a;
    const BLAS_MATH_t * pB_arr = thunkParams->b;
    BLAS_MATH_t * pC_arr = thunkParams->c;
    BLAS_MATH_t * pA_db  = thunkDeps->dba.ptr;
    BLAS_MATH_t * pB_db  = thunkDeps->dbb.ptr;
    BLAS_MATH_t * pC_db  = thunkDeps->dbc.ptr;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    bool conjugateA = (thunkParams->transa == 'C' || thunkParams->transa == 'c');
    bool conjugateB = (thunkParams->transb == 'C' || thunkParams->transb == 'c');
#endif
    bool transposeA = (thunkParams->transa != 'N' && thunkParams->transa != 'n');
    bool transposeB = (thunkParams->transb != 'N' && thunkParams->transb != 'n');

    for (i = 0; i < (!transposeA ? thunkParams->numRowsOpA : thunkParams->numColsOpA); i++) {
        for (j = 0; j < (!transposeA ? thunkParams->numColsOpA : thunkParams->numRowsOpA); j++) {
            pA_db[j] = pA_arr[j];
        }
        pA_db  += thunkParams->lda_datablock;
        pA_arr += thunkParams->lda_inputArray;
    }

    for (i = 0; i < (!transposeB ? thunkParams->numRowsOpB : thunkParams->numColsOpB); i++) {
        for (j = 0; j < (!transposeB ? thunkParams->numColsOpB : thunkParams->numRowsOpB); j++) {
//            printf ("i=%ld j=%ld %f\n", (u64) i, (u64) j, pB_arr[j]);
            pB_db[j] = pB_arr[j];
        }
        pB_db  += thunkParams->ldb_datablock;
        pB_arr += thunkParams->ldb_inputArray;
    }

    for (i = 0; i < thunkParams->numRowsC; i++) {
        for (j = 0; j < thunkParams->numColsC; j++) {
            pC_db[j] = pC_arr[j];
        }
        pC_db  += thunkParams->ldc_datablock;
        pC_arr += thunkParams->ldc_outputArray;
    }

// Create a template for the natural OCR EDT for spawning GEMM.
    ocrGuid_t gemmTemplate;
    ocrEdtTemplateCreate(&gemmTemplate, NAME(gemm_task), sizeof(NAME(gemm_edtParams_t))/sizeof(u64), sizeof(NAME(gemm_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(gemm_edtParams_t) gemmEdtParams;
    gemmEdtParams.numRowsOpA_numRowsC   = thunkParams->numRowsOpA_numRowsC;
    gemmEdtParams.numColsOpB_numColsC   = thunkParams->numColsOpB_numColsC;
    gemmEdtParams.numColsOpA_numRowsOpB = thunkParams->numColsOpA_numRowsOpB;
    gemmEdtParams.alpha                 = thunkParams->alpha;
    gemmEdtParams.beta                  = thunkParams->beta;
    gemmEdtParams.lda                   = thunkParams->lda_datablock;
    gemmEdtParams.ldb                   = thunkParams->ldb_datablock;
    gemmEdtParams.ldc                   = thunkParams->ldc_datablock;
    gemmEdtParams.offseta               = 0;
    gemmEdtParams.offsetb               = 0;
    gemmEdtParams.offsetc               = 0;
    gemmEdtParams.transa                = thunkParams->transa;
    gemmEdtParams.transb                = thunkParams->transb;

// Spawn the native OCR API into the GEMM EDT and supply its dependencies.
    ocrGuid_t gemmEdt;
    ocrGuid_t gemmDone;
    ocrEdtCreate(&gemmEdt, gemmTemplate, EDT_PARAM_DEF, (u64 *) &gemmEdtParams, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmDone);

    ADD_DEPENDENCE(thunkDeps->dba.guid, gemmEdt, NAME(gemm_edtDeps_t), dba, RO);  // At top level, we don't really posture the backing-store, we just force its usage.
    ADD_DEPENDENCE(thunkDeps->dbb.guid, gemmEdt, NAME(gemm_edtDeps_t), dbb, RO);  // At top level, we don't really posture the backing-store, we just force its usage.
    ADD_DEPENDENCE(thunkDeps->dbc.guid, gemmEdt, NAME(gemm_edtDeps_t), dbc, RW);
    ADD_DEPENDENCE(NULL_GUID,           gemmEdt, NAME(gemm_edtDeps_t), optionalTrigger, RO);

//    printf ("        %s exiting\n", STRINGIFY(NAME(gemm_thunkTask))); fflush(stdout);

    return gemmDone;
} // ?gemm_ThunkTask


ocrGuid_t NAME(gemm_task) (           // Spawns top-level worker EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

//    printf ("Entered %s, top-level native OCR GEMM task.\n", STRINGIFY(NAME(gemm_task))); fflush(stdout);

    NAME(gemm_edtParams_t) * myParams = (NAME(gemm_edtParams_t) *) paramv;
    NAME(gemm_edtDeps_t)   * myDeps   = (NAME(gemm_edtDeps_t)   *) depv;

// Create a template for the worker EDT.
    ocrGuid_t workerTemplate;
    ocrEdtTemplateCreate(&workerTemplate, NAME(gemm_workerTask), sizeof(NAME(gemm_workerParams_t))/sizeof(u64), sizeof(NAME(gemm_workerDeps_t))/sizeof(ocrEdtDep_t));

// Create a template for the worker collector EDT.
    ocrGuid_t collectorTemplate;
    ocrEdtTemplateCreate(&collectorTemplate, collectorTask, sizeof(NAME(gemm_collectorParams_t))/sizeof(u64), sizeof(NAME(gemm_collectorDeps_t))/sizeof(ocrEdtDep_t));

// Create templates for the inbound and outbound subblock posturing EDTs.
    ocrGuid_t posture3dInwardTemplate;  // GUID of EdtTemplate for posturing 3d subblocks, i.e. POTENTIALLY migrating inputs closer to the computation core.
    ocrGuid_t posture3dOutwardTemplate; // GUID of EdtTemplate for posturing 3d subblocks, back out to the backing-store (if they were migrated inward earlier).
    ocrEdtTemplateCreate(&posture3dInwardTemplate,  ocrPosture3dSubblockInwardTask,  sizeof(ocrPosture3dSubblockParams_t)/sizeof(u64), sizeof(ocrPosture3dSubblockInwardDeps_t) /sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&posture3dOutwardTemplate, ocrPosture3dSubblockOutwardTask, sizeof(ocrPosture3dSubblockParams_t)/sizeof(u64), sizeof(ocrPosture3dSubblockOutwardDeps_t)/sizeof(ocrEdtDep_t));

    NAME(gemm_workerParams_t) topLevelWorkerParams;
    topLevelWorkerParams.numRowsOpA_numRowsC      = myParams->numRowsOpA_numRowsC;
    topLevelWorkerParams.numColsOpB_numColsC      = myParams->numColsOpB_numColsC;
    topLevelWorkerParams.numColsOpA_numRowsOpB    = myParams->numColsOpA_numRowsOpB;
    topLevelWorkerParams.alpha                    = myParams->alpha;
    topLevelWorkerParams.beta                     = myParams->beta;
    topLevelWorkerParams.lda                      = myParams->lda;
    topLevelWorkerParams.ldb                      = myParams->ldb;
    topLevelWorkerParams.ldc                      = myParams->ldc;
    topLevelWorkerParams.offseta                  = myParams->offseta;
    topLevelWorkerParams.offsetb                  = myParams->offsetb;
    topLevelWorkerParams.offsetc                  = myParams->offsetc;
    topLevelWorkerParams.backingStoreA            = myDeps->dba.guid;
    topLevelWorkerParams.backingStoreB            = myDeps->dbb.guid;
    topLevelWorkerParams.backingStoreC            = myDeps->dbc.guid;
    topLevelWorkerParams.workerTemplate           = workerTemplate;
    topLevelWorkerParams.collectorTemplate        = collectorTemplate;
    topLevelWorkerParams.posture3dInwardTemplate  = posture3dInwardTemplate;
    topLevelWorkerParams.posture3dOutwardTemplate = posture3dOutwardTemplate;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    topLevelWorkerParams.conjugateA               = (myParams->transa == 'C' || myParams->transa == 'c');
    topLevelWorkerParams.conjugateB               = (myParams->transb == 'C' || myParams->transb == 'c');
#endif
    topLevelWorkerParams.transposeA               = (myParams->transa != 'N' && myParams->transa != 'n');
    topLevelWorkerParams.transposeB               = (myParams->transb != 'N' && myParams->transb != 'n');

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(gemm_task--full-workload)),myParams->numRowsOpA_numRowsC*myParams->numColsOpB_numColsC*myParams->numColsOpA_numRowsOpB,true)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(gemm_task--full-workload)),myParams->numRowsOpA_numRowsC*myParams->numColsOpB_numColsC*myParams->numColsOpA_numRowsOpB*4,true)
#endif

    ocrGuid_t workerEdt;
    ocrGuid_t gemmDone;
    ocrEdtCreate(&workerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&topLevelWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmDone);
    ADD_DEPENDENCE(myDeps->dba.guid, workerEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);  // At top level, we don't really posture the backing-store, we just force its usage.
    ADD_DEPENDENCE(myDeps->dbb.guid, workerEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);  // At top level, we don't really posture the backing-store, we just force its usage.
    ADD_DEPENDENCE(myDeps->dbc.guid, workerEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW); // At top level, we don't really posture the backing-store, we just force its usage.
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
    ADD_DEPENDENCE(NULL_GUID,        workerEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif

//    printf ("        %s exiting, top-level native OCR GEMM task.\n", STRINGIFY(NAME(gemm_task))); fflush(stdout);
    return gemmDone;

} // ?gemm_task)

// This EDT either performs the GEMM (GEneral Matrix Multiply) workload on a set of sub-matrices, or spawns two children and splits the workload between them.
static ocrGuid_t NAME(gemm_workerTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    BLAS_MATH_t zero;
    xSETc (zero, 0.0, 0.0);

//    printf ("Entered %s.\n", STRINGIFY(NAME(gemm_workerTask))); fflush(stdout);

    ocrGuid_t myOutputEvent = NULL_GUID;

    NAME(gemm_workerParams_t) * myParams = (NAME(gemm_workerParams_t) *) paramv;
    NAME(gemm_workerDeps_t)   * myDeps   = (NAME(gemm_workerDeps_t)   *) depv;

    ocrGuid_t gdbA = myDeps->dbPosturedA.guid;
    void *    pdbA = myDeps->dbPosturedA.ptr;
    ocrGuid_t gdbB = myDeps->dbPosturedB.guid;
    void *    pdbB = myDeps->dbPosturedB.ptr;
    ocrGuid_t gdbC = myDeps->dbPosturedC.guid;
    void *    pdbC = myDeps->dbPosturedC.ptr;
    char ma, mb, mc;

    if (myParams->backingStoreA != gdbA) {  // Submatrix A was postured by migrating it closer to the computational agent(s) that will utilize it.  Adjust parameters accordingly.
        ma = 'm';
        //printf ("        %s:  Submatrix A was migrated.  Adjust worker arguments accordingly.\n", STRINGIFY(NAME(gemm_workerTask)));
        myParams->lda = (!myParams->transposeA ? myParams->numRowsOpA : myParams->numColsOpA);
        //printf ("        %s:  Submatrix A was migrated.  Adjust worker arguments accordingly.  lda=%ld\n", STRINGIFY(NAME(gemm_workerTask)), (u64) myParams->lda);
        myParams->offseta = 0;
        myParams->backingStoreA = gdbA;     // This migrated subblock becomes the "backing store" for any child workers this worker might spawn.
        myParams->transposeA = false;       // If we needed to transpose A, it has now been done "for free" during the migration operation.
    } else {
        ma = 'b';
        //printf ("        %s:  Submatrix A was NOT migrated.  Use backingStore.\n", STRINGIFY(NAME(gemm_workerTask)));
    }
    if (myParams->backingStoreB != gdbB) {  // Submatrix B was postured by migrating it closer to the computational agent(s) that will utilize it.  Adjust parameters accordingly.
        mb = 'm';
        //printf ("        %s:  Submatrix B was migrated.  Adjust worker arguments accordingly.\n", STRINGIFY(NAME(gemm_workerTask)));
        myParams->ldb = (!myParams->transposeB ? myParams->numRowsOpB : myParams->numColsOpB);
        myParams->offsetb = 0;
        myParams->backingStoreB = gdbB;     // This migrated subblock becomes the "backing store" for any child workers this worker might spawn.
        myParams->transposeB = false;       // If we needed to transpose B, it has now been done "for free" during the migration operation.
    } else {
        mb = 'b';
        //printf ("        %s:  Submatrix B was NOT migrated.  Use backingStore.\n", STRINGIFY(NAME(gemm_workerTask)));
    }
    if (myParams->backingStoreC != gdbC) {  // Submatrix C was postured by migrating it closer to the computational agent(s) that will utilize it.  Adjust parameters accordingly.
        mc = 'm';
        //printf ("        %s:  Submatrix C was migrated.  Adjust worker arguments accordingly.\n", STRINGIFY(NAME(gemm_workerTask)));
        myParams->ldc = myParams->numColsC;
        myParams->offsetc = 0;
        myParams->backingStoreC = gdbC;     // This migrated subblock becomes the "backing store" for any child workers this worker might spawn.
    } else {
        mc = 'b';
        //printf ("        %s:  Submatrix C was NOT migrated.  Use backingStore.\n", STRINGIFY(NAME(gemm_workerTask)));
    }

    BLAS_int_t  numRowsOpA_numRowsC      = myParams->numRowsOpA_numRowsC;
    BLAS_int_t  numColsOpB_numColsC      = myParams->numColsOpB_numColsC;
    BLAS_int_t  numColsOpA_numRowsOpB    = myParams->numColsOpA_numRowsOpB;
    BLAS_MATH_t alpha                    = myParams->alpha;
    BLAS_MATH_t beta                     = myParams->beta;
    BLAS_int_t  lda                      = myParams->lda;
    BLAS_int_t  ldb                      = myParams->ldb;
    BLAS_int_t  ldc                      = myParams->ldc;
    BLAS_int_t  offseta                  = myParams->offseta;
    BLAS_int_t  offsetb                  = myParams->offsetb;
    BLAS_int_t  offsetc                  = myParams->offsetc;
    ocrGuid_t   workerTemplate           = myParams->workerTemplate;
    ocrGuid_t   collectorTemplate        = myParams->collectorTemplate;
    ocrGuid_t   posture3dInwardTemplate  = myParams->posture3dInwardTemplate;
    ocrGuid_t   posture3dOutwardTemplate = myParams->posture3dOutwardTemplate;
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    bool        conjugateA               = myParams->conjugateA;
    bool        conjugateB               = myParams->conjugateB;
#endif
    bool        transposeA               = myParams->transposeA;
    bool        transposeB               = myParams->transposeB;

    int codeVersionToUse = CODEVERSION;
    if (codeVersionToUse < 4 && (transposeA || transposeB)) {
        static bool haveWarnedAboutCodeVersion = false;
        if (! haveWarnedAboutCodeVersion) {
            printf ("******* Transposing matrix A and/or B is NOT implemented in %s for CODEVERSION<4.  You specified CODEVERSION=%ld.  Switching to CODEVERSION=4.\n",
                STRINGIFY(NAME(gemm_task)), (u64) CODEVERSION);
            haveWarnedAboutCodeVersion = true;
        }
        codeVersionToUse = 4;
    }

    //printf ("        %s doing  c=0x%lx  offsetc/ldc:%ld/%ld=%ld  offseta:%ld/%ld=%ld  offsetb:%ld/%ld=%ld  numRowsOpA_numRowsC=%ld numColsOpB_numColsC=%ld numColsOpA_numRowsOpB=%ld migrations: a:%c b:%c c:%c\n",
    //    STRINGIFY(NAME(gemm_task)),
    //    (u64) pdbC,
    //    (u64) offsetc, (u64) ldc, (u64) ((offsetc/ldc)),
    //    (u64) offseta, (u64) lda, (u64) ((offseta/lda)),
    //    (u64) offsetb, (u64) ldb, (u64) ((offsetb/ldb)),
    //    (u64) numRowsOpA_numRowsC, (u64) numColsOpB_numColsC, (u64) numColsOpA_numRowsOpB,
    //    ma, mb, mc); fflush(stdout);

#define numberOfXEsPerBlock 8 // TG-focused.  The distinction is (anticipated to be) irrelevant for other platforms.  Relevance: suggests how many "strips" per "panel" a panel-sized workload would split into.
#define cStripHeight 24
#define cPanelHeight (cStripHeight*numberOfXEsPerBlock)
#define cStripWidth  192
//TODO:  When we decide to break individual C submatrices among multiple workers (and sum the results), be sure to remember to recalculate the estConsumerCount, etc., for A and B subblock posturing.

    // We split workloads that are larger than a parametrically set bolus, but we also split workloads for which the input matrix (B here; A at the next switch statement) needs to be transposed
    // because in the course of posturing the workload, we force it to be migrated, and we perform the transpose for free along the way.  Ultimately, when the workload bolus is actually processed
    // by a leaf worker, all transposes are done.  Unless we get all the way to the else-clause, split the matrices down to two pieces and dispatch those pieces to two child EDTs.  If we get all
    // the way to the else-clause, just do the matrix multiply on the inputs, writing the result to C.  Current strategy of work-splitting is such that each worker is operating on distinct
    // submatrices of C, so no need to accumulate their contributions to C atomically.
//printf ("numColsOpB = %ld, transposeB = %ld, cStripWidth = %ld  |  numRowsOpA = %ld, transposeA = %ld, cStripHeight = %ld\n", (u64) numColsOpB, (u64) transposeB, (u64) cStripWidth, (u64) numRowsOpA, (u64) transposeA, (u64) cStripHeight); fflush(stdout);
    if (codeVersionToUse != 0 && (transposeB ? (numRowsOpB > 1) : (numColsOpB > cStripWidth))) { // Split into left and right portions of C.  Left is rounded up to a multiple of full strip-widths; right is remnant.
//printf ("Splitting B\n");
        switch (codeVersionToUse) {
C1      case 1:   // First cut at production code  (All descendents all the way down to the leaf sink into a common wrapup EDT (set up by the EDT that spawned the top level instance of this EDT).)
C1      {
C1          BLAS_int_t numColsOpB_left = (((numColsOpB >> 1) + (cStripWidth-1)) / cStripWidth) * cStripWidth;
C1          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C1          childWorkerParams.numColsOpB_numColsC = numColsOpB_left;                       // n reduced as appropriate for the left section of C.
C1
C1          ocrGuid_t lChildEdt;
C1          ocrGuid_t lChildOutputEvent;
C1          ocrEdtCreate(&lChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &lChildOutputEvent);
C1
C1          childWorkerParams.numColsOpB_numColsC = numColsOpB_numColsC - numColsOpB_left; // n reduced as appropriate for the right section of C.
C1          childWorkerParams.offsetb += numColsOpB_left;                                  // Offset is adjusted to reach the right section of B.
C1          childWorkerParams.offsetc += numColsOpB_left;                                  // Offset is adjusted to reach the right section of C.
C1
C1          ocrGuid_t rChildEdt;
C1          ocrEdtCreate(&rChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID,
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST  // Only make the caller await completion of this child if we are serializing the traversal of the GEMM solution topology (which we do for easier debugging).
C1              &myOutputEvent
#else
C1              NULL
#endif
C1              );

#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C1          ADD_DEPENDENCE(NULL_GUID,         lChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
C1          ADD_DEPENDENCE(lChildOutputEvent, rChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // Right child waits for left.
#endif
C1          ADD_DEPENDENCE(gdbA, lChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C1          ADD_DEPENDENCE(gdbB, lChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C1          ADD_DEPENDENCE(gdbC, lChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C1          ADD_DEPENDENCE(gdbA, rChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C1          ADD_DEPENDENCE(gdbB, rChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C1          ADD_DEPENDENCE(gdbC, rChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C1      }
C1      break;

C2      case 2:   // This worker spawns left and right worker EDTs, and also spawns a collector, dependent upon those children.
C2      {
C2          BLAS_int_t numColsOpB_left = (((numColsOpB >> 1) + (cStripWidth-1)) / cStripWidth) * cStripWidth;
C2          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C2          childWorkerParams.numColsOpB_numColsC = numColsOpB_left;                       // n reduced as appropriate for the left section of C.
C2
C2          ocrGuid_t lChildEdt;
C2          ocrGuid_t lChildOutputEvent;
C2          ocrEdtCreate(&lChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &lChildOutputEvent);
C2
C2          childWorkerParams.numColsOpB_numColsC = numColsOpB_numColsC - numColsOpB_left; // n reduced as appropriate for the right section of C.
C2          childWorkerParams.offsetb += numColsOpB_left;                                  // Offset is adjusted to reach the right section of B.
C2          childWorkerParams.offsetc += numColsOpB_left;                                  // Offset is adjusted to reach the right section of C.
C2
C2          ocrGuid_t rChildEdt;
C2          ocrGuid_t rChildOutputEvent;
C2          ocrEdtCreate(&rChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &rChildOutputEvent);
C2
C2          NAME(gemm_collectorParams_t) collectorParams;
C2          ocrGuid_t collectorEdt;
C2          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C2
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C2
C2          ADD_DEPENDENCE(lChildOutputEvent, collectorEdt, NAME(gemm_collectorDepsC2_t),  leftDone, RO);
C2          ADD_DEPENDENCE(rChildOutputEvent, collectorEdt, NAME(gemm_collectorDepsC2_t), rightDone, RO);
C2          ADD_DEPENDENCE(NULL_GUID, collectorEdt, NAME(gemm_collectorDepsC2_t),stub1ForC3AndBeyond, RO);
C2          ADD_DEPENDENCE(NULL_GUID, collectorEdt, NAME(gemm_collectorDepsC2_t),stub2ForC3AndBeyond, RO);
C2
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C2          ADD_DEPENDENCE(lChildOutputEvent, rChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // Right child waits for left.
#endif
C2          ADD_DEPENDENCE(gdbA,      rChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C2          ADD_DEPENDENCE(gdbB,      rChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C2          ADD_DEPENDENCE(gdbC,      rChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C2          ADD_DEPENDENCE(NULL_GUID, lChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C2          ADD_DEPENDENCE(gdbA,      lChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C2          ADD_DEPENDENCE(gdbB,      lChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C2          ADD_DEPENDENCE(gdbC,      lChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C2      }
C2      break;

C3      case 3:   // Like case 2, this worker spawns left and right worker EDTs and a collector.  To this version, we add calls to a datablock "posturing" EDT, and a "deposturing" EDT, which will become
C3                // OCR primitives that MIGHT or MIGHT NOT copy the needed operand sub-matrices (B in this case) into a closer-to-the-core datablock, a decision which is NOT made until the consuming EDT
C3                // is scheduled.  Order is me->leftAndRightPosturing->leftAndRightChildren->leftAndRightDeposturing->collector.
C3      {
C3          BLAS_int_t numColsOpB_left = (((numColsOpB >> 1) + (cStripWidth-1)) / cStripWidth) * cStripWidth;
C3          ocrPosture3dSubblockParams_t posture3dParams;
C3          posture3dParams.offset             = offsetb*sizeof(BLAS_MATH_t);
C3          posture3dParams.subblockDepth      = 1;
C3          posture3dParams.subblockHeight     = numRowsOpB;
C3          posture3dParams.subblockWidth      = numColsOpB_left;
C3          posture3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C3          posture3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C3          posture3dParams.rowPitch           = ldb*sizeof(BLAS_MATH_t);
C3          posture3dParams.elementPitch       = sizeof(BLAS_MATH_t);
C3          posture3dParams.estConsumerCount   = (numColsOpB_left / cStripWidth) * ((numRowsOpB + (cStripHeight-1)) / cStripHeight);
C3          posture3dParams.estAccessPressure  = cStripWidth;
C3          posture3dParams.intendToRead       = true;
C3          posture3dParams.intendToWriteAny   = false;
C3          posture3dParams.reserveSpaceOnly   = false;
C3          posture3dParams.forceMigration     = false;
C3
C3          ocrGuid_t postureBForLChildEdt;
C3          ocrGuid_t depostureBForLChildEdt;
C3          ocrGuid_t posturedBForLChild;
C3          ocrGuid_t deposturedBForLChild;
C3          ocrEdtCreate(&postureBForLChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedBForLChild);
C3          ocrEdtCreate(&depostureBForLChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedBForLChild);
C3
C3          posture3dParams.offset                   += numColsOpB_left*sizeof(BLAS_MATH_t); // Byte offset relative to B's backing-store DB, to the right subblock of B to posture.
C3          posture3dParams.subblockWidth             = numColsOpB - numColsOpB_left;        // The number of elements per row for the right split on the columns of B.
C3          posture3dParams.estConsumerCount          = ((numColsOpB - numColsOpB_left+(cStripWidth-1)) / cStripWidth) * ((numRowsOpB + (cStripHeight-1)) / cStripHeight);
C3
C3          ocrGuid_t postureBForRChildEdt;
C3          ocrGuid_t posturedBForRChild;
C3          ocrGuid_t depostureBForRChildEdt;
C3          ocrGuid_t deposturedBForRChild;
C3          ocrEdtCreate(&postureBForRChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedBForRChild);
C3          ocrEdtCreate(&depostureBForRChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedBForRChild);
C3
C3          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C3          childWorkerParams.numColsOpB_numColsC = numColsOpB_left;                       // n reduced as appropriate for the left section of C.
C3
C3          ocrGuid_t lChildEdt;
C3          ocrGuid_t lChildOutputEvent;
C3          ocrEdtCreate(&lChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &lChildOutputEvent);
C3
C3          childWorkerParams.numColsOpB_numColsC = numColsOpB_numColsC - numColsOpB_left; // n reduced as appropriate for the right section of C.
C3          childWorkerParams.offsetb += numColsOpB_left;                                  // Offset is adjusted to reach the right section of B.
C3          childWorkerParams.offsetc += numColsOpB_left;                                  // Offset is adjusted to reach the right section of C.
C3
C3          ocrGuid_t rChildEdt;
C3          ocrGuid_t rChildOutputEvent;
C3          ocrEdtCreate(&rChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &rChildOutputEvent);
C3
C3          NAME(gemm_collectorParams_t) collectorParams;
C3          ocrGuid_t collectorEdt;
C3          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C3
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C3
C3          ADD_DEPENDENCE(deposturedBForLChild, collectorEdt, NAME(gemm_collectorDeps_t), child1AorBDepostured, RO);
C3          ADD_DEPENDENCE(deposturedBForRChild, collectorEdt, NAME(gemm_collectorDeps_t), child2AorBDepostured, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            collectorEdt, NAME(gemm_collectorDeps_t), child1CDepostured, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            collectorEdt, NAME(gemm_collectorDeps_t), child2CDepostured, RO);
C3
C3          ADD_DEPENDENCE(rChildOutputEvent,    depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C3          ADD_DEPENDENCE(posturedBForRChild,   depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C3          ADD_DEPENDENCE(gdbB,                 depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C3          ADD_DEPENDENCE(NULL_GUID,            depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C3
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(NULL_GUID,            rChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // TODO:  This case (and all later) serialze fine on data.  Perhaps delete this dependency?
#endif
C3          ADD_DEPENDENCE(gdbA,                 rChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C3          ADD_DEPENDENCE(posturedBForRChild,   rChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C3          ADD_DEPENDENCE(gdbC,                 rChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C3
C3          ADD_DEPENDENCE(gdbB,                 postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(deposturedBForLChild, postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize right branch after left.
#else
C3          ADD_DEPENDENCE(NULL_GUID,            postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C3
C3          ADD_DEPENDENCE(lChildOutputEvent,    depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C3          ADD_DEPENDENCE(posturedBForLChild,   depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C3          ADD_DEPENDENCE(gdbB,                 depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C3          ADD_DEPENDENCE(NULL_GUID,            depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C3
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(NULL_GUID,            lChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C3          ADD_DEPENDENCE(gdbA,                 lChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C3          ADD_DEPENDENCE(posturedBForLChild,   lChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C3          ADD_DEPENDENCE(gdbC,                 lChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C3
C3          ADD_DEPENDENCE(gdbB,                 postureBForLChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            postureBForLChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Left branch fires right away.
C3      }
C3      break;

        case 4:   // Like case 3, this version postures the sub-matrices of C in addition to posturing of B.
        {
C4          BLAS_int_t numColsOpB_left = (numColsOpB <= cStripWidth) ? (numColsOpB >> 1) : ((((numColsOpB >> 1) + (cStripWidth-1)) / cStripWidth) * cStripWidth);
C4          ocrPosture3dSubblockParams_t postureB3dParams, postureC3dParams;
C4          postureB3dParams.offset             = offsetb*sizeof(BLAS_MATH_t);
C4          postureB3dParams.subblockDepth      = 1;
C4          postureB3dParams.subblockHeight     = numRowsOpB;
C4          postureB3dParams.subblockWidth      = numColsOpB_left;
C4          postureB3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C4          postureB3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C4          postureB3dParams.estConsumerCount   = (numColsOpB_left / cStripWidth) * ((numRowsOpB + (cStripHeight-1)) / cStripHeight);
C4          postureB3dParams.estAccessPressure  = cStripWidth;
C4          postureB3dParams.intendToRead       = true;
C4          postureB3dParams.intendToWriteAny   = false;
C4          postureB3dParams.reserveSpaceOnly   = false;
C4          postureB3dParams.forceMigration     = (transposeB && numColsOpB_left <= cStripWidth); // If split will be the last, and still need B transposed, force posturing service to migrate
C4          if (! transposeB) {
C4              postureB3dParams.rowPitch       = ldb*sizeof(BLAS_MATH_t);
C4              postureB3dParams.elementPitch   = sizeof(BLAS_MATH_t);
C4          } else {
C4              postureB3dParams.rowPitch       = sizeof(BLAS_MATH_t);
C4              postureB3dParams.elementPitch   = ldb*sizeof(BLAS_MATH_t);
C4          }
C4          postureC3dParams.offset             = offsetc*sizeof(BLAS_MATH_t);
C4          postureC3dParams.subblockDepth      = 1;
C4          postureC3dParams.subblockHeight     = numRowsC;
C4          postureC3dParams.subblockWidth      = numColsC_left;
C4          postureC3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C4          postureC3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C4          postureC3dParams.rowPitch           = ldc*sizeof(BLAS_MATH_t);
C4          postureC3dParams.elementPitch       = sizeof(BLAS_MATH_t);
C4          postureC3dParams.estConsumerCount   = (numColsC_left / cStripWidth) * ((numRowsC + (cStripHeight-1)) / cStripHeight);
C4          postureC3dParams.estAccessPressure  = xCMP_EQ(beta, zero) ? 1 : 2; // If beta == 0.0, write only; else read the write.
C4          postureC3dParams.intendToRead       = xCMP_NE(beta, zero);         // If beta != 0.0  (or (0.0, 0.0) for complex), then the C matrix will be read before written.
C4          postureC3dParams.intendToWriteAny   = true;
C4          postureC3dParams.reserveSpaceOnly   = xCMP_EQ(beta, zero);         // If beta == 0.0  (or (0.0, 0.0) for comlex), then no need to copy original value of C in as we posture.
C4          postureC3dParams.forceMigration     = false;
C4
C4          ocrGuid_t postureBForLChildEdt;
C4          ocrGuid_t depostureBForLChildEdt;
C4          ocrGuid_t posturedBForLChild;
C4          ocrGuid_t deposturedBForLChild;
C4          ocrGuid_t postureCForLChildEdt;
C4          ocrGuid_t depostureCForLChildEdt;
C4          ocrGuid_t posturedCForLChild;
C4          ocrGuid_t deposturedCForLChild;
C4
C4          ocrEdtCreate(&postureBForLChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureB3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedBForLChild);
C4          ocrEdtCreate(&depostureBForLChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureB3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedBForLChild);
C4          ocrEdtCreate(&postureCForLChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedCForLChild);
C4          ocrEdtCreate(&depostureCForLChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedCForLChild);
C4
C4          postureB3dParams.offset            += numColsOpB_left*(transposeB?ldb:1)*sizeof(BLAS_MATH_t);        // Byte offset relative to B's backing-store DB, to the right subblock of B to posture.
C4          postureB3dParams.subblockWidth      = numColsOpB-numColsOpB_left;                                    // The number of elements per row for the right split on the columns of B.
C4          postureB3dParams.estConsumerCount   = (((numColsOpB-numColsOpB_left)+(cStripWidth-1)) / cStripWidth) * ((numRowsOpB + (cStripHeight-1)) / cStripHeight);
C4          postureB3dParams.forceMigration     = (transposeB && ((numColsOpB-numColsOpB_left) <= cStripWidth)); // If last split, and still need B transposed, force posturing service to migrate
C4          postureC3dParams.offset            += numColsC_left*sizeof(BLAS_MATH_t);                             // Byte offset relative to C's backing-store DB, to the right subblock of C to posture.
C4          postureC3dParams.subblockWidth      = numColsC-numColsC_left;                                        // The number of elements per row for the right split on the columns of C.
C4          postureC3dParams.estConsumerCount   = (((numColsC-numColsC_left)+(cStripWidth-1)) / cStripWidth) * ((numRowsC + (cStripHeight-1)) / cStripHeight);
C4
C4          ocrGuid_t postureBForRChildEdt;
C4          ocrGuid_t depostureBForRChildEdt;
C4          ocrGuid_t posturedBForRChild;
C4          ocrGuid_t deposturedBForRChild;
C4          ocrGuid_t postureCForRChildEdt;
C4          ocrGuid_t depostureCForRChildEdt;
C4          ocrGuid_t posturedCForRChild;
C4          ocrGuid_t deposturedCForRChild;
C4
C4          ocrEdtCreate(&postureBForRChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureB3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedBForRChild);
C4          ocrEdtCreate(&depostureBForRChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureB3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedBForRChild);
C4          ocrEdtCreate(&postureCForRChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedCForRChild);
C4          ocrEdtCreate(&depostureCForRChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedCForRChild);
C4
C4          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C4          childWorkerParams.numColsOpB_numColsC = numColsOpB_left;           // n reduced as appropriate for the left section of C.
C4
C4          ocrGuid_t lChildEdt;
C4          ocrGuid_t lChildOutputEvent;
C4          ocrEdtCreate(&lChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &lChildOutputEvent);
C4
C4          childWorkerParams.numColsOpB = numColsOpB - numColsOpB_left;       // n reduced as appropriate for the right section of C.
C4          childWorkerParams.offsetb += numColsOpB_left*(!transposeB?1:ldb);  // Offset is adjusted to reach the right section of B.
C4          childWorkerParams.offsetc += numColsOpB_left;                      // Offset is adjusted to reach the right section of C.
C4
C4          ocrGuid_t rChildEdt;
C4          ocrGuid_t rChildOutputEvent;
C4          ocrEdtCreate(&rChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &rChildOutputEvent);
C4
C4          NAME(gemm_collectorParams_t) collectorParams;
C4          ocrGuid_t collectorEdt;
C4          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C4
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C4
C4          ADD_DEPENDENCE(deposturedCForLChild, collectorEdt, NAME(gemm_collectorDeps_t), child1CDepostured, RO);
C4          ADD_DEPENDENCE(deposturedCForRChild, collectorEdt, NAME(gemm_collectorDeps_t), child2CDepostured, RO);
C4          ADD_DEPENDENCE(deposturedBForLChild, collectorEdt, NAME(gemm_collectorDeps_t), child1AorBDepostured, RO);
C4          ADD_DEPENDENCE(deposturedBForRChild, collectorEdt, NAME(gemm_collectorDeps_t), child2AorBDepostured, RO);
C4
C4          ADD_DEPENDENCE(rChildOutputEvent,    depostureCForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedCForRChild,   depostureCForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbC,                 depostureCForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureCForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
C4          ADD_DEPENDENCE(rChildOutputEvent,    depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedBForRChild,   depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbB,                 depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureBForRChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(NULL_GUID,            rChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // TODO:  This case (and all later) serialze fine on data.  Perhaps delete this dependency?
#endif
C4          ADD_DEPENDENCE(gdbA,                 rChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C4          ADD_DEPENDENCE(posturedBForRChild,   rChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C4          ADD_DEPENDENCE(posturedCForRChild,   rChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C4
C4          ADD_DEPENDENCE(gdbC,                 postureCForRChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(deposturedCForLChild, postureCForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize right branch after left.
#else
C4          ADD_DEPENDENCE(NULL_GUID,            postureCForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C4
C4          ADD_DEPENDENCE(gdbB,                 postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(deposturedBForLChild, postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize right branch after left.
#else
C4          ADD_DEPENDENCE(NULL_GUID,            postureBForRChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C4
C4          ADD_DEPENDENCE(lChildOutputEvent,    depostureCForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedCForLChild,   depostureCForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbC,                 depostureCForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureCForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
C4          ADD_DEPENDENCE(lChildOutputEvent,    depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedBForLChild,   depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbB,                 depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureBForLChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(NULL_GUID,            lChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C4          ADD_DEPENDENCE(gdbA,                 lChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C4          ADD_DEPENDENCE(posturedBForLChild,   lChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C4          ADD_DEPENDENCE(posturedCForLChild,   lChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C4
C4          ADD_DEPENDENCE(gdbC,                 postureCForLChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C4          ADD_DEPENDENCE(NULL_GUID,            postureCForLChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Left branch fires right away.
C4
C4          ADD_DEPENDENCE(gdbB,                 postureBForLChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C4          ADD_DEPENDENCE(NULL_GUID,            postureBForLChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Left branch fires right away.
C4      }
        break;
        case 5:   // If splitting workloads along columns of B, this case doesn't do anything different here than for case 4.
        case 6:   // If splitting workloads along columns of B, this case doesn't do anything different here than for case 4.
        default:
        {
            printf("***** Error:  codeVersionToUse == %ld, which is not implemented for GEMM\n", (u64) codeVersionToUse); fflush(stdout);
        } // default
        } // switch

    } else if (codeVersionToUse != 0 && (transposeA ? (numRowsOpA > 1) : (numRowsOpA > cStripHeight))) { // Split into top and bottom portions of C.  Next, decide whether we are splitting into panels, or strips.
//printf ("Splitting A\n");
        switch (codeVersionToUse) {
C1      case 1:   // First cut at production code  (All descendents all the way down to the leaf sink into a common wrapup EDT (set up by the EDT that spawned the top level instance of this EDT).)
C1      {
C1          BLAS_int_t numRowsOpA_upper;
C1          if (numRowsOpA > cPanelHeight) {  // Upper portion is a multiple of full panel-heights;  bottom portion is remnant.
C1              numRowsOpA_upper = (((numRowsOpA >> 1) + (cPanelHeight-1)) / cPanelHeight) * cPanelHeight;
C1          } else {                          // Upper portion is a multiple of full strip-heights;  bottom portion is remnant.
C1              numRowsOpA_upper = (((numRowsOpA >> 1) + (cStripHeight-1)) / cStripHeight) * cStripHeight;
C1          }
C1
C1          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C1          childWorkerParams.numRowsOpA = numRowsOpA_upper;              // m reduced as appropriate for the upper section of C.
C1
C1          ocrGuid_t uChildEdt;
C1          ocrGuid_t uChildOutputEvent;
C1          ocrEdtCreate(&uChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &uChildOutputEvent);
C1
C1          childWorkerParams.numRowsOpA = numRowsOpA - numRowsOpA_upper; // m reduced as appropriate for the upper section of C.
C1          childWorkerParams.offseta += numRowsOpA_upper * lda;          // Offset is adjusted to reach the bottom section of A.
C1          childWorkerParams.offsetc += numRowsOpA_upper * ldc;          // Offset is adjusted to reach the bottom section of C.
C1
C1          ocrGuid_t dChildEdt;
C1          ocrEdtCreate(&dChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID,
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST  // Only make the caller await completion of this child if we are serializing the traversal of the GEMM solution topology (which we do for easier debugging).
C1              &myOutputEvent
#else
C1              NULL
#endif
C1              );
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C1          ADD_DEPENDENCE(NULL_GUID,         uChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
C1          ADD_DEPENDENCE(uChildOutputEvent, dChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // Lower child waits for upper.
#endif
C1          ADD_DEPENDENCE(gdbA, uChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C1          ADD_DEPENDENCE(gdbB, uChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C1          ADD_DEPENDENCE(gdbC, uChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C1          ADD_DEPENDENCE(gdbA, dChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C1          ADD_DEPENDENCE(gdbB, dChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C1          ADD_DEPENDENCE(gdbC, dChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C1      }
C1      break;

C2      case 2:   // This worker spawns upper and lower worker EDTs, and also spawns a collector, dependent upon those children.
C2      {
C2          BLAS_int_t numRowsOpA_upper;
C2          if (numRowsOpA > cPanelHeight) {  // Upper portion is a multiple of full panel-heights;  bottom portion is remnant.
C2              numRowsOpA_upper = (((numRowsOpA >> 1) + (cPanelHeight-1)) / cPanelHeight) * cPanelHeight;
C2          } else {                          // Upper portion is a multiple of full strip-heights;  bottom portion is remnant.
C2              numRowsOpA_upper = (((numRowsOpA >> 1) + (cStripHeight-1)) / cStripHeight) * cStripHeight;
C2          }
C2
C2          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C2          childWorkerParams.numRowsOpA = numRowsOpA_upper;              // m reduced as appropriate for the upper section of C.
C2
C2          ocrGuid_t uChildEdt;
C2          ocrGuid_t uChildOutputEvent;
C2          ocrEdtCreate(&uChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &uChildOutputEvent);
C2
C2          childWorkerParams.numRowsOpA = numRowsOpA - numRowsOpA_upper; // m reduced as appropriate for the upper section of C.
C2          childWorkerParams.offseta += numRowsOpA_upper * lda;          // Offset is adjusted to reach the bottom section of A.
C2          childWorkerParams.offsetc += numRowsOpA_upper * ldc;          // Offset is adjusted to reach the bottom section of C.
C2
C2          ocrGuid_t dChildEdt;
C2          ocrGuid_t dChildOutputEvent;
C2          ocrEdtCreate(&dChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &dChildOutputEvent);
C2
C2          NAME(gemm_collectorParams_t) collectorParams;
C2          ocrGuid_t collectorEdt;
C2          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C2
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C2
C2          ADD_DEPENDENCE(uChildOutputEvent, collectorEdt, NAME(gemm_collectorDepsC2_t),  leftDone, RO);
C2          ADD_DEPENDENCE(dChildOutputEvent, collectorEdt, NAME(gemm_collectorDepsC2_t), rightDone, RO);
C2          ADD_DEPENDENCE(NULL_GUID, collectorEdt, NAME(gemm_collectorDepsC2_t),stub1ForC3AndBeyond, RO);
C2          ADD_DEPENDENCE(NULL_GUID, collectorEdt, NAME(gemm_collectorDepsC2_t),stub2ForC3AndBeyond, RO);
C2
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C2          ADD_DEPENDENCE(uChildOutputEvent, dChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // Upper child waits for lower.
#endif
C2          ADD_DEPENDENCE(gdbA,      dChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C2          ADD_DEPENDENCE(gdbB,      dChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C2          ADD_DEPENDENCE(gdbC,      dChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C2
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C2          ADD_DEPENDENCE(NULL_GUID, uChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C2          ADD_DEPENDENCE(gdbA,      uChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C2          ADD_DEPENDENCE(gdbB,      uChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C2          ADD_DEPENDENCE(gdbC,      uChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C2
C2      }
C2      break;

C3      case 3:   // Like case 2, this worker spawns upper and lower worker EDTs and a collector.  To this version, we add calls to a datablock "posturing" EDT, and a "deposturing" EDT, which will become
C3                // OCR primitives that MIGHT or MIGHT NOT copy the needed operand sub-matrices (B in this case) into a closer-to-the-core datablock, a decision which is NOT made until the consuming EDT
C3                // is scheduled.  Order is me->upperAndLowerPosturing->upperAndLowerChildren->upperAndLowerDeposturing->collector.
C3      {
C3          BLAS_int_t numRowsOpA_upper;
C3          if (numRowsOpA > cPanelHeight) {  // Upper portion is a multiple of full panel-heights;  bottom portion is remnant.
C3              numRowsOpA_upper = (((numRowsOpA >> 1) + (cPanelHeight-1)) / cPanelHeight) * cPanelHeight;
C3          } else {                          // Upper portion is a multiple of full strip-heights;  bottom portion is remnant.
C3              numRowsOpA_upper = (((numRowsOpA >> 1) + (cStripHeight-1)) / cStripHeight) * cStripHeight;
C3          }
C3          ocrPosture3dSubblockParams_t posture3dParams;
C3          posture3dParams.offset             = offseta*sizeof(BLAS_MATH_t);
C3          posture3dParams.subblockDepth      = 1;
C3          posture3dParams.subblockHeight     = numRowsOpA_upper;
C3          posture3dParams.subblockWidth      = numColsOpA;
C3          posture3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C3          posture3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C3          posture3dParams.rowPitch           = lda*sizeof(BLAS_MATH_t);
C3          posture3dParams.elementPitch       = sizeof(BLAS_MATH_t);
C3          posture3dParams.estConsumerCount   = (numRowsOpA_upper / cStripHeight) * ((numColsOpA + (cStripWidth-1)) / cStripWidth);
C3          posture3dParams.estAccessPressure  = cStripWidth;
C3          posture3dParams.intendToRead       = true;
C3          posture3dParams.intendToWriteAny   = false;
C3          posture3dParams.reserveSpaceOnly   = false;
C3          posture3dParams.forceMigration     = false;
C3
C3          ocrGuid_t postureAForUChildEdt;
C3          ocrGuid_t depostureAForUChildEdt;
C3          ocrGuid_t posturedAForUChild;
C3          ocrGuid_t deposturedAForUChild;
C3          ocrEdtCreate(&postureAForUChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedAForUChild);
C3          ocrEdtCreate(&depostureAForUChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedAForUChild);
C3
C3          posture3dParams.offset            += numRowsOpA_upper*lda*sizeof(BLAS_MATH_t);       // Byte offset relative to A's backing-store DB, to the lower subblock of A to posture.
C3          posture3dParams.subblockHeight     = numRowsOpA-numRowsOpA_upper;                    // The number or rows for the lower split of A.
C3          posture3dParams.estConsumerCount   = (((numRowsOpA-numRowsOpA_upper)+(cStripHeight-1)) / cStripHeight) * ((numColsOpA + (cStripWidth-1)) / cStripWidth);
C3
C3          ocrGuid_t postureAForDChildEdt;
C3          ocrGuid_t posturedAForDChild;
C3          ocrGuid_t depostureAForDChildEdt;
C3          ocrGuid_t deposturedAForDChild;
C3          ocrEdtCreate(&postureAForDChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedAForDChild);
C3          ocrEdtCreate(&depostureAForDChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedAForDChild);
C3
C3          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C3          childWorkerParams.numRowsOpA = numRowsOpA_upper;              // m reduced as appropriate for the upper section of C.
C3
C3          ocrGuid_t uChildEdt;
C3          ocrGuid_t uChildOutputEvent;
C3          ocrEdtCreate(&uChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &uChildOutputEvent);
C3
C3          childWorkerParams.numRowsOpA = numRowsOpA - numRowsOpA_upper; // m reduced as appropriate for the upper section of C.
C3          childWorkerParams.offseta += numRowsOpA_upper * lda;          // Offset is adjusted to reach the bottom section of A.
C3          childWorkerParams.offsetc += numRowsOpA_upper * ldc;          // Offset is adjusted to reach the bottom section of C.
C3
C3          ocrGuid_t dChildEdt;
C3          ocrGuid_t dChildOutputEvent;
C3          ocrEdtCreate(&dChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &dChildOutputEvent);
C3
C3          NAME(gemm_collectorParams_t) collectorParams;
C3          ocrGuid_t collectorEdt;
C3          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C3
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C3
C3
C3          ADD_DEPENDENCE(deposturedAForUChild, collectorEdt, NAME(gemm_collectorDeps_t), child1AorBDepostured, RO);
C3          ADD_DEPENDENCE(deposturedAForDChild, collectorEdt, NAME(gemm_collectorDeps_t), child2AorBDepostured, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            collectorEdt, NAME(gemm_collectorDeps_t), child1CDepostured, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            collectorEdt, NAME(gemm_collectorDeps_t), child2CDepostured, RO);
C3
C3          ADD_DEPENDENCE(dChildOutputEvent,    depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C3          ADD_DEPENDENCE(posturedAForDChild,   depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C3          ADD_DEPENDENCE(gdbA,                 depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C3          ADD_DEPENDENCE(NULL_GUID,            depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C3
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(NULL_GUID,            dChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // TODO:  This case (and all later) serialze fine on data.  Perhaps delete this dependency?
#endif
C3          ADD_DEPENDENCE(posturedAForDChild,   dChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C3          ADD_DEPENDENCE(gdbB,                 dChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C3          ADD_DEPENDENCE(gdbC,                 dChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C3
C3          ADD_DEPENDENCE(gdbA,                 postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(deposturedAForUChild, postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize lower branch after upper.
#else
C3          ADD_DEPENDENCE(NULL_GUID,            postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C3
C3          ADD_DEPENDENCE(uChildOutputEvent,    depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C3          ADD_DEPENDENCE(posturedAForUChild,   depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C3          ADD_DEPENDENCE(gdbA,                 depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C3          ADD_DEPENDENCE(NULL_GUID,            depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C3
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C3          ADD_DEPENDENCE(NULL_GUID,            uChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C3          ADD_DEPENDENCE(posturedAForUChild,   uChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C3          ADD_DEPENDENCE(gdbB,                 uChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C3          ADD_DEPENDENCE(gdbC,                 uChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C3
C3          ADD_DEPENDENCE(gdbA,                 postureAForUChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C3          ADD_DEPENDENCE(NULL_GUID,            postureAForUChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Upper branch fires right away.
C3      }
C3      break;

C4      case 4:   // Like case 3, this version postures the sub-matrices of C in addition to posturing of A.
C4      {
C4          BLAS_int_t numRowsOpA_upper;
C4          if (numRowsOpA > cPanelHeight) {  // Upper portion is a multiple of full panel-heights;  bottom portion is remnant.
C4              numRowsOpA_upper = (((numRowsOpA >> 1) + (cPanelHeight-1)) / cPanelHeight) * cPanelHeight;
C4          } else {                          // Upper portion is a multiple of full strip-heights;  bottom portion is remnant.
C4              numRowsOpA_upper = (numRowsOpA <= cStripHeight) ? (numRowsOpA >> 1) : ((((numRowsOpA >> 1) + (cStripHeight-1)) / cStripHeight) * cStripHeight);
C4          }
C4          ocrPosture3dSubblockParams_t postureA3dParams, postureC3dParams;
C4          postureA3dParams.offset             = offseta*sizeof(BLAS_MATH_t);
C4          postureA3dParams.subblockDepth      = 1;
C4          postureA3dParams.subblockHeight     = numRowsOpA_upper;
C4          postureA3dParams.subblockWidth      = numColsOpA;
C4          postureA3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C4          postureA3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C4          postureA3dParams.estConsumerCount   = (numRowsOpA_upper / cStripHeight) * ((numColsOpA + (cStripWidth-1)) / cStripWidth);
C4          postureA3dParams.estAccessPressure  = cStripWidth;
C4          postureA3dParams.intendToRead       = true;
C4          postureA3dParams.intendToWriteAny   = false;
C4          postureA3dParams.reserveSpaceOnly   = false;
C4          postureA3dParams.forceMigration     = (transposeA && numRowsOpA_upper <= cStripHeight); // If split will be the last, and still need A transposed, force posturing service to migrate
C4          if (! transposeA) {
C4              postureA3dParams.rowPitch       = lda*sizeof(BLAS_MATH_t);
C4              postureA3dParams.elementPitch   = sizeof(BLAS_MATH_t);
C4          } else {
C4              postureA3dParams.rowPitch       = sizeof(BLAS_MATH_t);
C4              postureA3dParams.elementPitch   = lda*sizeof(BLAS_MATH_t);
C4          }
C4          postureC3dParams.offset             = offsetc*sizeof(BLAS_MATH_t);
C4          postureC3dParams.subblockDepth      = 1;
C4          postureC3dParams.subblockHeight     = numRowsC_upper;
C4          postureC3dParams.subblockWidth      = numColsC;
C4          postureC3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
C4          postureC3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
C4          postureC3dParams.rowPitch           = ldc*sizeof(BLAS_MATH_t);
C4          postureC3dParams.elementPitch       = sizeof(BLAS_MATH_t);
C4          postureC3dParams.estConsumerCount   = ((numRowsC_upper + (cStripHeight-1)) * (numColsC / cStripWidth) / cStripHeight);
C4          postureC3dParams.estAccessPressure  = xCMP_EQ(beta, zero) ? 1 : 2; // If beta == 0.0, write only; else read the write.
C4          postureC3dParams.intendToRead       = xCMP_NE(beta, zero);         // If beta != 0.0  (or (0.0, 0.0) for complex), then the C matrix will be read before written.
C4          postureC3dParams.intendToWriteAny   = true;
C4          postureC3dParams.reserveSpaceOnly   = xCMP_EQ(beta, zero);         // If beta == 0.0  (or (0.0, 0.0) for comlex), then no need to copy original value of C in as we posture.
C4          postureC3dParams.forceMigration     = false;
C4
C4          ocrGuid_t postureAForUChildEdt;
C4          ocrGuid_t depostureAForUChildEdt;
C4          ocrGuid_t posturedAForUChild;
C4          ocrGuid_t deposturedAForUChild;
C4          ocrGuid_t postureCForUChildEdt;
C4          ocrGuid_t depostureCForUChildEdt;
C4          ocrGuid_t posturedCForUChild;
C4          ocrGuid_t deposturedCForUChild;
C4
C4          ocrEdtCreate(&postureAForUChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureA3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedAForUChild);
C4          ocrEdtCreate(&depostureAForUChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureA3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedAForUChild);
C4          ocrEdtCreate(&postureCForUChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedCForUChild);
C4          ocrEdtCreate(&depostureCForUChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedCForUChild);
C4
C4          postureA3dParams.offset            += numRowsOpA_upper*(transposeA?1:lda)*sizeof(BLAS_MATH_t);       // Byte offset relative to A's backing-store DB, to the lower subblock of A to posture.
C4          postureA3dParams.subblockHeight     = numRowsOpA-numRowsOpA_upper;                                   // The number or rows for the lower split of A.
C4          postureA3dParams.estConsumerCount   = (((numRowsOpA-numRowsOpA_upper)+(cStripHeight-1)) / cStripHeight) * ((numColsOpA + (cStripWidth-1)) / cStripWidth);
C4          postureA3dParams.forceMigration     = (transposeA && (numRowsOpA-numRowsOpA_upper) <= cStripHeight); // If last split, and still need A transposed, force posturing service to migrate
C4          postureC3dParams.offset            += numRowsC_upper*ldc*sizeof(BLAS_MATH_t);                        // Byte offset relative to C's backing-store DB, to the lower subblock of C to posture.
C4          postureC3dParams.subblockHeight     = numRowsC-numRowsC_upper;                                       // The number of rows for the lower split of C.
C4          postureC3dParams.estConsumerCount   = (((numRowsC-numRowsC_upper)+(cStripHeight-1)) / cStripHeight) * ((numColsC + (cStripWidth-1)) / cStripWidth);
C4
C4          ocrGuid_t postureAForDChildEdt;
C4          ocrGuid_t posturedAForDChild;
C4          ocrGuid_t depostureAForDChildEdt;
C4          ocrGuid_t deposturedAForDChild;
C4          ocrGuid_t postureCForDChildEdt;
C4          ocrGuid_t depostureCForDChildEdt;
C4          ocrGuid_t posturedCForDChild;
C4          ocrGuid_t deposturedCForDChild;
C4
C4          ocrEdtCreate(&postureAForDChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureA3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedAForDChild);
C4          ocrEdtCreate(&depostureAForDChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureA3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedAForDChild);
C4          ocrEdtCreate(&postureCForDChildEdt,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedCForDChild);
C4          ocrEdtCreate(&depostureCForDChildEdt, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&postureC3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedCForDChild);
C4
C4          NAME(gemm_workerParams_t) childWorkerParams = *myParams;
C4          childWorkerParams.numRowsOpA = numRowsOpA_upper;                   // m reduced as appropriate for the upper section of C.
C4
C4          ocrGuid_t uChildEdt;
C4          ocrGuid_t uChildOutputEvent;
C4          ocrEdtCreate(&uChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &uChildOutputEvent);
C4
C4          childWorkerParams.numRowsOpA = numRowsOpA - numRowsOpA_upper;      // m reduced as appropriate for the upper section of C.
C4          childWorkerParams.offseta += numRowsOpA_upper*(!transposeA?lda:1); // Offset is adjusted to reach the bottom section of A.
C4          childWorkerParams.offsetc += numRowsOpA_upper * ldc;               // Offset is adjusted to reach the bottom section of C.
C4
C4          ocrGuid_t dChildEdt;
C4          ocrGuid_t dChildOutputEvent;
C4          ocrEdtCreate(&dChildEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childWorkerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &dChildOutputEvent);
C4
C4          NAME(gemm_collectorParams_t) collectorParams;
C4          ocrGuid_t collectorEdt;
C4          ocrEdtCreate(&collectorEdt, collectorTemplate, EDT_PARAM_DEF, (u64 *) (&collectorParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &myOutputEvent);
C4
// Fill in the dependences, bottom-up order (so that earlier EDTs can't race ahead and complete before their outputEvents have been properly registered as dependencies for later EDTs.
C4
C4          ADD_DEPENDENCE(deposturedCForUChild, collectorEdt, NAME(gemm_collectorDeps_t), child1CDepostured, RO);
C4          ADD_DEPENDENCE(deposturedCForDChild, collectorEdt, NAME(gemm_collectorDeps_t), child2CDepostured, RO);
C4          ADD_DEPENDENCE(deposturedAForUChild, collectorEdt, NAME(gemm_collectorDeps_t), child1AorBDepostured, RO);
C4          ADD_DEPENDENCE(deposturedAForDChild, collectorEdt, NAME(gemm_collectorDeps_t), child2AorBDepostured, RO);
C4
C4          ADD_DEPENDENCE(dChildOutputEvent,    depostureCForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedCForDChild,   depostureCForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbC,                 depostureCForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureCForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
C4          ADD_DEPENDENCE(dChildOutputEvent,    depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedAForDChild,   depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbA,                 depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureAForDChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(NULL_GUID,            dChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO); // TODO:  This case (and all later) serialze fine on data.  Perhaps delete this dependency?
#endif
C4          ADD_DEPENDENCE(posturedAForDChild,   dChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C4          ADD_DEPENDENCE(gdbB,                 dChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C4          ADD_DEPENDENCE(posturedCForDChild,   dChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C4
C4          ADD_DEPENDENCE(gdbC,                 postureCForDChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(deposturedCForUChild, postureCForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize lower branch after upper.
#else
C4          ADD_DEPENDENCE(NULL_GUID,            postureCForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C4
C4          ADD_DEPENDENCE(gdbA,                 postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(deposturedAForUChild, postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Serialize lower branch after upper.
#else
C4          ADD_DEPENDENCE(NULL_GUID,            postureAForDChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Run both branches in parallel.
#endif
C4
C4          ADD_DEPENDENCE(uChildOutputEvent,    depostureCForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedCForUChild,   depostureCForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbC,                 depostureCForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureCForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
C4          ADD_DEPENDENCE(uChildOutputEvent,    depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb, RO);
C4          ADD_DEPENDENCE(posturedAForUChild,   depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, postured, RO);
C4          ADD_DEPENDENCE(gdbA,                 depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, backingStore, RW);
C4          ADD_DEPENDENCE(NULL_GUID,            depostureAForUChildEdt, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);
C4
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
C4          ADD_DEPENDENCE(NULL_GUID,            uChildEdt, NAME(gemm_workerDeps_t), serializationEvent, RO);
#endif
C4          ADD_DEPENDENCE(posturedAForUChild,   uChildEdt, NAME(gemm_workerDeps_t), dbPosturedA, RO);
C4          ADD_DEPENDENCE(gdbB,                 uChildEdt, NAME(gemm_workerDeps_t), dbPosturedB, RO);
C4          ADD_DEPENDENCE(posturedCForUChild,   uChildEdt, NAME(gemm_workerDeps_t), dbPosturedC, RW);
C4
C4          ADD_DEPENDENCE(gdbC,                 postureCForUChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C4          ADD_DEPENDENCE(NULL_GUID,            postureCForUChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Upper branch fires right away.
C4
C4          ADD_DEPENDENCE(gdbA,                 postureAForUChildEdt, ocrPosture3dSubblockInwardDeps_t, backingStore, RO);
C4          ADD_DEPENDENCE(NULL_GUID,            postureAForUChildEdt, ocrPosture3dSubblockInwardDeps_t, optionalTriggerEvent, RO);  // Upper branch fires right away.
C4      }
C4      break;
        case 5:   // If splitting workloads along rows of A, this case doesn't do anything different here than for case 4.
        case 6:   // If splitting workloads along rows of A, this case doesn't do anything different here than for case 4.
        default:
        {
            printf("***** Error:  codeVersionToUse == %ld, which is not implemented for GEMM\n", (u64) codeVersionToUse); fflush(stdout);
        } // default
        } // switch

#define cMaxContributors numColsOpA_numRowsOpB // TODO:FIXME: Limit cMaxContributors is set such that we will NOT yet attempt to split workloads such that there are multiple contributors to each sub-matrix of C.
    } else if (codeVersionToUse >= 5 && numColsOpA_numRowsOpB > cMaxContributors) { // Bifurcate into different contributors to C.
        printf ("***** Bifurcation of different contributors to C is NOT yet implemented.\n");
        ocrShutdown();
    } else { // Actually do the math for these sub-matrices.
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(gemm_task--bolus-workload)),numRowsOpA_numRowsC*numColsOpB_numColsC*numColsOpA_numRowsOpB,true)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(gemm_task--bolus-workload)),numRowsOpA_numRowsC*numColsOpB_numColsC*numColsOpA_numRowsOpB*4,true)
#endif
        if (codeVersionToUse < 6) {
            BLAS_MATH_t * a = ((BLAS_MATH_t *) pdbA) + offseta;  // Get address of upper left element of matrix A sub-block to process.
            BLAS_MATH_t * b = ((BLAS_MATH_t *) pdbB) + offsetb;  // Get address of upper left element of matrix B sub-block to process.
            BLAS_MATH_t * c = ((BLAS_MATH_t *) pdbC) + offsetc;  // Get address of upper left element of matrix C sub-block to process.
            BLAS_MATH_t zero, one;
            xSETc(zero, 0.0, 0.0);
            xSETc(one,  1.0, 0.0);
            u64 idxRowOpA_idxRowC,idxColOpB_idxColC,idxColOpA_idxRowOpB;
#define idxColOpA    idxColOpA_idxRowOpB
#define idxRowOpA    idxRowOpA_idxRowC
#define idxColOpB    idxColOpB_idxColC
#define idxRowOpB    idxColOpA_idxRowOpB
#define idxColC      idxColOpB_idxColC
#define idxRowC      idxRowOpA_idxRowC
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
            if (conjugateA) {
                if (conjugateB) {
                    if (xCMP_EQ(beta, zero)) { // Important optimization case:  when beta=0.0, no need for C to be pre-defined; it is just output.  Avoid reading (e.g. to keep valgrind happy).
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg12(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], acc, alpha);                             // C = A*B*alpha
                            }
                        }
                    } else if (xCMP_EQ(beta, one)) {
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg12(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    } else {
                        // TODO:  Could also optimize case where alpha = 1.0 or alpha = -1.0.  Not done yet.
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg12(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], c[idxRowC*ldc+idxColC], beta);           // C *= beta
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    }
                } else {
                    if (xCMP_EQ(beta, zero)) { // Important optimization case:  when beta=0.0, no need for C to be pre-defined; it is just output.  Avoid reading (e.g. to keep valgrind happy).
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg1(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], acc, alpha);                             // C = A*B*alpha
                            }
                        }
                    } else if (xCMP_EQ(beta, one)) {
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg1(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    } else {
                        // TODO:  Could also optimize case where alpha = 1.0 or alpha = -1.0.  Not done yet.
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg1(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], c[idxRowC*ldc+idxColC], beta);           // C *= beta
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    }
                }
            } else {
                if (conjugateB) {
                    if (xCMP_EQ(beta, zero)) { // Important optimization case:  when beta=0.0, no need for C to be pre-defined; it is just output.  Avoid reading (e.g. to keep valgrind happy).
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg2(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], acc, alpha);                             // C = A*B*alpha
                            }
                        }
                    } else if (xCMP_EQ(beta, one)) {
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg2(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    } else {
                        // TODO:  Could also optimize case where alpha = 1.0 or alpha = -1.0.  Not done yet.
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA_conjg2(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], c[idxRowC*ldc+idxColC], beta);           // C *= beta
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    }
                } else {
#endif
                    if (xCMP_EQ(beta, zero)) { // Important optimization case:  when beta=0.0, no need for C to be pre-defined; it is just output.  Avoid reading (e.g. to keep valgrind happy).
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], acc, alpha);                             // C = A*B*alpha
                            }
                        }
                    } else if (xCMP_EQ(beta, one)) {
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    } else {
                        // TODO:  Could also optimize case where alpha = 1.0 or alpha = -1.0.  Not done yet.
                        for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
                            for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC ++) {
                                BLAS_MATH_t acc;
                                xSETc (acc, 0.0, 0.0);                                                 // Zero out the accumulator.
                                for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                                    xFMA(acc, a[idxRowOpA*lda+idxColOpA], b[idxRowOpB*ldb+idxColOpB]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
                                }
                                xMUL (c[idxRowC*ldc+idxColC], c[idxRowC*ldc+idxColC], beta);           // C *= beta
                                xFMA (c[idxRowC*ldc+idxColC], acc, alpha);                             // C += A*B*alpha
                            }
                        }
                    }
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                }
            }
#endif
        } else {
            printf("***** Error:  codeVersionToUse == %ld, which is not implemented for GEMM\n", (u64) codeVersionToUse); fflush(stdout);
        }
    }

//    printf ("        %s exiting.\n", STRINGIFY(NAME(gemm_workerTask))); fflush(stdout);
#ifdef TRAVERSE_TOPOLOGY_DEPTH_FIRST
    return myOutputEvent;
#else
    return NULL_GUID;
#endif
} // gemmWorkerTask


static ocrGuid_t collectorTask (   // This EDT triggers when two worker EDTs (spawned by a higher-level worker EDT that is splitting its work between two children) are done.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

//    printf ("Entered collectorTask.\n"); fflush(stdout);
//    printf ("        collectorTask exiting.\n"); fflush(stdout);
    return NULL_GUID;
} // collectorTask


static ocrGuid_t NAME(gemm_wrapupTask) (   // This EDT gains control when all GEMM worker EDTs are done.  It "dethunks" the output matrix C from the OCR datablock to the original array passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

//    printf ("Entered %s\n", STRINGIFY(NAME(gemm_wrapupTask))); fflush(stdout);

    NAME(gemm_wrapupParams_t) * wrapupParams = (NAME(gemm_wrapupParams_t) *) paramv;
    NAME(gemm_wrapupDeps_t)   * wrapupDeps   = (NAME(gemm_wrapupDeps_t)   *) depv;

    u64 i, j;

#ifdef IS_UNIT_TESTING
    BLAS_MATH_t * a    = ((BLAS_MATH_t *) wrapupDeps->dba.ptr);  // Get address of upper left element of matrix A sub-block to process.
    BLAS_MATH_t * b    = ((BLAS_MATH_t *) wrapupDeps->dbb.ptr);  // Get address of upper left element of matrix B sub-block to process.
    BLAS_MATH_t * c    = ((BLAS_MATH_t *) wrapupDeps->dbc.ptr);  // Get address of upper left element of matrix C sub-block to process.
    BLAS_MATH_t * cRef = wrapupParams->c;
    BLAS_int_t       numRowsOpA_numRowsC   = wrapupParams->numRowsOpA_numRowsC;
    BLAS_int_t       numColsOpB_numColsC   = wrapupParams->numColsOpB_numColsC;
    BLAS_int_t       numColsOpA_numRowsOpB = wrapupParams->numColsOpA_numRowsOpB;
    bool             transposeA = wrapupParams->transposeA;
    bool             transposeB = wrapupParams->transposeB;
    BLAS_int_t       StorRowToRow_a = wrapupParams->lda_datablock;
    BLAS_int_t       StorColToCol_a = 1;
    if (transposeA) {
        StorRowToRow_a = 1;
        StorColToCol_a = wrapupParams->lda_datablock;
    }
    BLAS_int_t       StorRowToRow_b = wrapupParams->ldb_datablock;
    BLAS_int_t       StorColToCol_b = 1;
    if (transposeB) {
        StorRowToRow_b = 1;
        StorColToCol_b = wrapupParams->ldb_datablock;
    }
    BLAS_int_t       idxRowOpA_idxRowC, idxColOpB_idxColC, idxColOpA_idxRowOpB;
    u64 mismatches = 0;
//    printf ("        %s:  Checking GEMM results.  %ld rows, %ld columns\n", STRINGIFY(NAME(gemm_wrapupTask)), (u64) numRowsC, (u64) numColsC); fflush(stdout);
    BLAS_MATH_t zero;
    xSETc(zero, 0.0, 0.0);
    for (idxRowOpA_idxRowC = 0; idxRowOpA_idxRowC < numRowsOpA_numRowsC; idxRowOpA_idxRowC++) {
        for (idxColOpB_idxColC = 0; idxColOpB_idxColC < numColsOpB_numColsC; idxColOpB_idxColC++) {
            BLAS_MATH_t acc;
            xSETc (acc, 0.0, 0.0);                                            // Zero out the accumulator.
            for (idxColOpA_idxRowOpB = 0; idxColOpA_idxRowOpB < numColsOpA_numRowsOpB; idxColOpA_idxRowOpB++) {
                xFMA(acc, a[idxRowOpA*StorRowToRow_a+idxColOpA*StorColToCol_a], b[idxRowOpB*StorRowToRow_b+idxColOpB*StorColToCol_b]); // acc += <element of row-vector of A> * <corresponding element of col-vector of B>
            }
#ifdef BATCHING
            if (!transposeA && !transposeB)
#endif
            {
                if (xCMP_EQ(wrapupParams->beta, zero)) {
                    xMUL (cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC], acc, wrapupParams->alpha);       // C' = C*beta + A*B*alpha
                } else {
                    xMUL (cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC], cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC], wrapupParams->beta);
                    xFMA (cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC], acc, wrapupParams->alpha);       // C' = C*beta + A*B*alpha
                }
            }
            if (xCMP_NE(cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC], c[idxRowC*wrapupParams->ldc_datablock+idxColC])) {
                if (++mismatches <= 10) {
                    printf ("        %s:  MISMATCH: (row=%ld, col=%ld), (%lf, %lf) vs. ref(%lf, %lf)\n", STRINGIFY(NAME(gemm_wrapupTask)),
                        (u64) idxRowC,
                        (u64) idxColC,
                        xREAL(c[idxRowC*wrapupParams->ldc_datablock+idxColC]),
                        xIMAG(c[idxRowC*wrapupParams->ldc_datablock+idxColC]),
                        xREAL(cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC]),
                        xIMAG(cRef[idxRowC*wrapupParams->ldc_outputArray+idxColC])); fflush(stdout);
                }
            }
        }
    }
    printf ("        %s:  Checking: Finished! %ld mismatches\n", STRINGIFY(NAME(gemm_wrapupTask)), (u64) mismatches); fflush(stdout);
#else
    BLAS_MATH_t * pC_db  = wrapupDeps->dbc.ptr;
    BLAS_MATH_t * pC_arr = wrapupParams->c;
    for (i = 0; i < numRowsC; i++) {
        for (j = 0; i < numColsC; i++) {
            pC_arr[j] = pC_db[j];
        }
        pC_db  += wrapupParams->ldc_datablock;
        pC_arr += wrapupParams->ldc_outputArray;
    }
#endif

    ocrDbDestroy (wrapupDeps->dba.guid);  // Destroy datablock for matrix A.
    ocrDbDestroy (wrapupDeps->dbb.guid);  // Destroy datablock for matrix B.
    ocrDbDestroy (wrapupDeps->dbc.guid);  // Destroy datablock for matrix C.

    //printf ("        %s:  Normal done status.  Calling ocrShutdown (which should be moved elsewhere)\n", STRINGIFY(NAME(gemm_wrapupTask))); fflush(stdout);
    if (mismatches != 0) {
        ocrShutdown();  // TODO:  This should move out of here.  Surely the GEMM library function should NOT be pulling the plug!
    } else {
#ifdef BATCHING
        test_gemm();
#else
        ocrShutdown();  // TODO:  This should move out of here.  Surely the GEMM library function should NOT be pulling the plug!
#endif
    }
    return NULL_GUID;
} // gemmWrapupTask

#undef CODEVERSION
