/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?trsm,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  Solve one of the matrix equations op(A)*X=alpha*B  or   X*op(A)=alpha*B, where alpha is a scalar, X and B are m by n matrices, A is
 *  a unit or non-unit, upper or lower, triangular matrix, and op(A) is one of op(A)=A or op(A)=A**T.  The matrix X is overwritten on B.
 *
 **/

#define CODEVERSION 1   // 0 -- reference version (workload NOT split at all).  1 -- workload split, down to point of whole row(s) per agent.
#define TRAVERSE_TOPOLOGY_DEPTH_FIRST   // uncomment this in order to force child workers to serialize, left-before-right, upper-before-lower, so that debugging messages come out in an easier-to-read order.
                                        // ****** WARNING!!!  When workloads are split left-and-right or upper-and-lower, the posturing of subblocks still happens in non-deterministic order.

#include "blas.h"
#include "ocr.h"
#include "ocr-posture.h"
#include <stdio.h>

typedef struct {                          // These are the scalar variables that are passed to the TRSM thunking task as its paramv argument.
    BLAS_int_t    m;                      // Input:  Number of rows of B.
    BLAS_int_t    n;                      // Input:  Number of columns of B.
    BLAS_MATH_t   alpha;                  // Input:  Scalar by which to multiply each matrix B value before it is modified.
    BLAS_int_t    lda_array;              // Input:  The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // Input:  The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Input:  Address of input matrix A.
    BLAS_MATH_t * b;                      // In/Out: Address of input/output matrix B.
    char          side;                   // Input:  "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    char          uplo;                   // Input:  "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    char          transa;                 // Input:  "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**H
    char          diag;                   // Input:  "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.
} NAME(trsm_thunkParams_t);

typedef struct {                          // These are the dependencies that the TRSM thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
} NAME(trsm_thunkDeps_t);

static ocrGuid_t NAME(trsm_thunkTask) (    // Moves array-based input matrices (passed by standard TRSM API) into OCR datablocks, then spawns TRSM native OCR EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    BLAS_int_t    m;                 // Number of rows of B.
    BLAS_int_t    n;                 // Number of columns of B.
    BLAS_MATH_t   alpha;             // Scalar by which to multiply each matrix B value before it is modified.
    BLAS_int_t    lda;               // The leading dimension of the matrix A.
    BLAS_int_t    ldb;               // The leading dimension of the matrix B.
    BLAS_int_t    offseta;           // Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
    BLAS_int_t    offsetb;           // Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
    BLAS_int_t    workerFirstIter;   // Zero-based index of first column (or rows if side="R") of B to process.  For top level invocation, this is most likely zero.
    BLAS_int_t    workerNumIters;    // Number of columns (or rows if side="R") to process.  For top level invocation, this is most likely n (or m if side="R").
    char          side;              // "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    char          uplo;              // "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    char          transa;            // "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**H
    char          diag;              // "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.
    ocrGuid_t     trsmTemplate;      // Edt template for spawning child workers, when workload needs to be split.
    ocrGuid_t     backingStoreA;     // Datablock for backingStore of matrix A.
    BLAS_int_t    idealWorkloadSize; // Approximate number of flops for the "ideal" leaf workload.
} NAME(trsm_workerParams_t);

typedef struct {
    ocrEdtDep_t   dbPosturedA;       // Datablock for postured matrix A.
    ocrEdtDep_t   dbb;               // Datablock for matrix B.
    ocrEdtDep_t   optionalTrigger;   // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(trsm_workerDeps_t);

ocrGuid_t NAME(trsm_workerTask) (    // This worker either performs the operation or splits it among up to eight children tasks
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the TRSM wrapup task as its paramv argument.
    BLAS_int_t    m;                      // Input:  Number of rows of B.
    BLAS_int_t    n;                      // Input:  Number of columns of B.
    BLAS_MATH_t * b;                      // In/Out: Address of input/output matrix B.
    BLAS_int_t    ldb_array;              // Input:  The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
} NAME(trsm_wrapupParams_t);

typedef struct {                          // These are the dependencies that the TRSM wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when GETRS native OCR EDT is done.
} NAME(trsm_wrapupDeps_t);

static ocrGuid_t NAME(trsm_wrapupTask) (  // Writes matrix, pivot indices, and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?trsm library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as vector of pivot indices and scalar status info.

void NAME(trsm) (
    char        * side,     // Input:  "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    char        * uplo,     // Input:  "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    char        * transa,   // Input:  "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**H
    char        * diag,     // Input:  "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.
    BLAS_int_t    m,        // Input:  Number of rows of B.
    BLAS_int_t    n,        // Input:  Number of columns of B.
    BLAS_MATH_t   alpha,    // Input:  Scalar by which to multiply each matrix B value before it is modified.
    BLAS_MATH_t * a,        // Input:  Matrix of size lda*k, where k = ((side[0]=='L'||side[0]=='l') ? m : n
    BLAS_int_t    lda,      // Input:  Leading dimension (aka "pitch") of matrix A, in units of BLAS_MATH_t
    BLAS_MATH_t * b,        // In/Out: Matrix of size ldb*n
    BLAS_int_t    ldb)      // Input:  Leading dimension (aka "pitch") of matrix B, in units of BLAS_MATH_t
{
    printf("***** NOT TESTED!!!! ***** Entered Standard API %s function.\n", STRINGIFY(NAME(trsm))); fflush(stdout); // TODO

    BLAS_int_t    nrowa;
    if (side[0] == 'L' || side[0] == 'l') {
        nrowa = m;
    } else if (side[0] == 'R' || side[0] == 'r') {
        nrowa = n;
    } else {
        xerbla (STRINGIFY(NAME(trsm)), 1);
        return;
    }

    if (uplo[0] != 'U' && uplo[0] != 'u' && uplo[0] != 'L' && uplo[0] != 'l') {
        xerbla (STRINGIFY(NAME(trsm)), 2);
        return;
    }

    if (transa[0] != 'N' && transa[0] != 'n' && transa[0] != 'T' && transa[0] != 't' && transa[0] != 'C' && transa[0] != 'c') {
        xerbla (STRINGIFY(NAME(trsm)), 3);
        return;
    }

    if (diag[0] != 'U' && diag[0] != 'u' && diag[0] != 'N' && diag[0] != 'n') {
        xerbla (STRINGIFY(NAME(trsm)), 4);
        return;
    }

    if (m < 0) {
        xerbla (STRINGIFY(NAME(trsm)), 5);
        return;
    }

    if (n < 0) {
        xerbla (STRINGIFY(NAME(trsm)), 6);
        return;
    }

    if (lda < MAX(1, nrowa)) {
        xerbla (STRINGIFY(NAME(trsm)), 9);
        return;
    }

    if (ldb < MAX(1, n)) {
        xerbla (STRINGIFY(NAME(trsm)), 11);
        return;
    }

    // Quick return if possible.
    if (m == 0 || n == 0) return;


// Create the datablocks:

    ocrGuid_t  dba;           // Guid of datablock for matrix A.
    ocrGuid_t  dbb;           // Guid of datablock for matrix B.

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

    BLAS_int_t pitchOfMatA = (m + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t pitchOfMatB = (n + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatAInBytes = (m * pitchOfMatA) * sizeof(BLAS_MATH_t);
    BLAS_int_t sizeOfMatBInBytes = (m * pitchOfMatB) * sizeof(BLAS_MATH_t);
    ocrDbCreate (&dba, (void**) &addr, sizeOfMatAInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&dbb, (void**) &addr, sizeOfMatBInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t trsmThunkTemplate;
    NAME(trsm_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&trsmThunkTemplate, NAME(trsm_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(trsm_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the TRSM result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t trsmWrapupTemplate;
    NAME(trsm_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&trsmWrapupTemplate, NAME(trsm_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(trsm_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.m              = m;                  // Input:  Number of rows of B.
    thunkParams.n              = n;                  // Input:  Number of columns of B.
    thunkParams.alpha          = alpha;              // Input:  Scalar by which to multiply each matrix B value before it is modified.
    thunkParams.lda_array      = lda;                // Input:  The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    thunkParams.ldb_array      = ldb;                // Input:  The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    thunkParams.lda_datablock  = pitchOfMatA;        // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    thunkParams.ldb_datablock  = pitchOfMatB;        // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    thunkParams.a              = (BLAS_MATH_t *) a;  // Input:  Address of input matrix A.
    thunkParams.b              = (BLAS_MATH_t *) b;  // In/Out: Address of input/output matrix B.
    thunkParams.side           = side[0];            // Input:  "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    thunkParams.uplo           = uplo[0];            // Input:  "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    thunkParams.transa         = transa[0];          // Input:  "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**H
    thunkParams.diag           = diag[0];            // Input:  "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.

    ocrGuid_t trsmThunkEdt;
    ocrGuid_t trsmThunkOutputEvent;
    ocrEdtCreate(&trsmThunkEdt, trsmThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &trsmThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API TRSM func.
    wrapupParams.m             = m;                  // Input:  Number of rows of B.
    wrapupParams.n             = n;                  // Input:  Number of columns of B.
    wrapupParams.b             = (BLAS_MATH_t *) b;  // In/Out: Address of input/output matrix B.
    wrapupParams.ldb_array     = ldb;                // Input:  The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    wrapupParams.ldb_datablock = pitchOfMatB;        // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).

    ocrGuid_t trsmWrapupEdt;
    ocrEdtCreate(&trsmWrapupEdt, trsmWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                  trsmWrapupEdt, NAME(trsm_wrapupDeps_t), dba,                 RO);
    ADD_DEPENDENCE(dbb,                  trsmWrapupEdt, NAME(trsm_wrapupDeps_t), dbb,                 RO);
    ADD_DEPENDENCE(trsmThunkOutputEvent, trsmWrapupEdt, NAME(trsm_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                  trsmThunkEdt, NAME(trsm_thunkDeps_t), dba,        RW);
    ADD_DEPENDENCE(dbb,                  trsmThunkEdt, NAME(trsm_thunkDeps_t), dbb,        RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GETRS operation.\n", STRINGIFY(NAME(trsm))); fflush(stdout);

} // ?trsm


// Receiving array-based input matrices from the standard API for TRSM, this EDT moves them into OCR datablocks, creates datablocks for pivot indices and info, then passes them to TRSM native OCR EDT.
static ocrGuid_t NAME(trsm_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(trsm_thunkTask))); fflush(stdout); // TODO

    NAME(trsm_thunkParams_t) * thunkParams = (NAME(trsm_thunkParams_t) *) paramv;
    NAME(trsm_thunkDeps_t)   * thunkDeps   = (NAME(trsm_thunkDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pMatA_arr = thunkParams->a;
    BLAS_MATH_t * pMatA_db  = thunkDeps->dba.ptr;

    for (i = 0; i < thunkParams->m; i++) {
        for (j = 0; j < thunkParams->m; j++) {
            pMatA_db[j] = pMatA_arr[j];
        }
        pMatA_db  += thunkParams->lda_datablock;
        pMatA_arr += thunkParams->lda_array;
    }

    BLAS_MATH_t * pMatB_arr = thunkParams->b;
    BLAS_MATH_t * pMatB_db  = thunkDeps->dbb.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->m; j++) {
            pMatB_db[j] = pMatB_arr[j];
        }
        pMatB_db  += thunkParams->ldb_datablock;
        pMatB_arr += thunkParams->ldb_array;
    }

    ocrGuid_t trsmTemplate;
    NAME(trsm_edtParams_t) NAME(trsm_edtParams);
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(trsm_edtParams).m              = thunkParams->m;               // Input:  Number of rows of B.
    NAME(trsm_edtParams).n              = thunkParams->n;               // Input:  Number of columns of B.
    NAME(trsm_edtParams).alpha          = thunkParams->alpha;           // Input:  Scalar by which to multiply each matrix B value before it is modified.
    NAME(trsm_edtParams).lda            = thunkParams->lda_datablock;   // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    NAME(trsm_edtParams).ldb            = thunkParams->ldb_datablock;   // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    NAME(trsm_edtParams).offseta        = 0;                            // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
    NAME(trsm_edtParams).offsetb        = 0;                            // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
    NAME(trsm_edtParams).side           = thunkParams->side;            // Input:  "L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B
    NAME(trsm_edtParams).uplo           = thunkParams->uplo;            // Input:  "U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.
    NAME(trsm_edtParams).transa         = thunkParams->transa;          // Input:  "N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**H
    NAME(trsm_edtParams).diag           = thunkParams->diag;            // Input:  "U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.

    ocrGuid_t trsmEdt;
    ocrEdtCreate(&trsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(trsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dba.guid,        trsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(thunkDeps->dbb.guid,        trsmEdt, NAME(trsm_edtDeps_t), dbb,             RW);
    ADD_DEPENDENCE(NULL_GUID,                  trsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(trsm_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?trsm_thunkTask


ocrGuid_t NAME(trsm_task) (            // Spawnable externally.  Spawn the top level worker.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    static BLAS_int_t idealWorkloadSize = -1;
    if (idealWorkloadSize < 0) idealWorkloadSize = ilaenv(1, STRINGIFY(NAME(trsm)), " ", -1, -1, -1, -1);

    NAME(trsm_edtParams_t) * myParams = (NAME(trsm_edtParams_t) *) paramv;
    NAME(trsm_edtDeps_t)   * myDeps   = (NAME(trsm_edtDeps_t)   *) depv;
    NAME(trsm_workerParams_t)  childParams;

    //printf ("Entered %s,  m = %ld, n = %ld\n", STRINGIFY(NAME(trsm_task)), ((u64) (myParams->m)), ((u64) (myParams->n))); fflush(stdout);

// Create templates for the inbound and outbound subblock posturing EDTs.
    ocrGuid_t posture3dInwardTemplate;  // GUID of EdtTemplate for posturing 3d subblocks, i.e. POTENTIALLY migrating inputs closer to the computation core.
    ocrGuid_t posture3dOutwardTemplate; // GUID of EdtTemplate for posturing 3d subblocks, back out to the backing-store (if they were migrated inward earlier).
    ocrEdtTemplateCreate(&posture3dInwardTemplate,  ocrPosture3dSubblockInwardTask,  sizeof(ocrPosture3dSubblockParams_t)/sizeof(u64), sizeof(ocrPosture3dSubblockInwardDeps_t) /sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&posture3dOutwardTemplate, ocrPosture3dSubblockOutwardTask, sizeof(ocrPosture3dSubblockParams_t)/sizeof(u64), sizeof(ocrPosture3dSubblockOutwardDeps_t)/sizeof(ocrEdtDep_t));

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    BLAS_int_t approxFlopsPerCol = MAX(1,(myParams->m * (myParams->m - 1)) >> 1);
#else
    BLAS_int_t approxFlopsPerCol = MAX(1,(myParams->m * (myParams->m - 1)) << 1);
#endif
    BLAS_int_t idealNumColsPerChild = MAX(1, (idealWorkloadSize + (idealWorkloadSize>>3)) / approxFlopsPerCol);
    BLAS_int_t numLeafWorkers = (myParams->m + idealNumColsPerChild - 1) / idealNumColsPerChild;

    ocrPosture3dSubblockParams_t posture3dParams;
    posture3dParams.offset             = myParams->offseta*sizeof(BLAS_MATH_t);
    posture3dParams.subblockDepth      = 1;
    posture3dParams.subblockHeight     = myParams->m;
    posture3dParams.subblockWidth      = myParams->m*sizeof(BLAS_MATH_t);
    posture3dParams.numBytesPerElement = sizeof(BLAS_MATH_t);
    posture3dParams.planePitch         = 9999; // Distance from one plane to the next (in bytes) is irrelevant; just posturing one plane.
    posture3dParams.rowPitch           = myParams->lda;
    posture3dParams.elementPitch       = sizeof(BLAS_MATH_t);
    posture3dParams.estConsumerCount   = numLeafWorkers;
    posture3dParams.estAccessPressure  = idealNumColsPerChild;
    posture3dParams.intendToRead       = true;
    posture3dParams.intendToWriteAny   = false;
    posture3dParams.reserveSpaceOnly   = false;
    posture3dParams.forceMigration     = false;

    ocrGuid_t postureA;
    ocrGuid_t depostureA;
    ocrGuid_t posturedA;
    ocrGuid_t deposturedA;
    ocrEdtCreate(&postureA,   posture3dInwardTemplate,  EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &posturedA);
    ocrEdtCreate(&depostureA, posture3dOutwardTemplate, EDT_PARAM_DEF, (u64 *) (&posture3dParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &deposturedA);

    ocrGuid_t trsmTemplate;      // Edt template for spawning child workers, when workload needs to be split.
    NAME(trsm_edtParams_t) NAME(trsm_edtParams);
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_workerTask), sizeof(NAME(trsm_workerParams_t))/sizeof(u64), sizeof(NAME(trsm_workerDeps_t))/sizeof(ocrEdtDep_t));

    childParams.m                 = myParams->m;
    childParams.n                 = myParams->n;
    childParams.alpha             = myParams->alpha;
    childParams.lda               = myParams->lda;
    childParams.ldb               = myParams->ldb;
    childParams.offseta           = myParams->offseta;
    childParams.offsetb           = myParams->offsetb;
    childParams.workerFirstIter   = 0;
    childParams.workerNumIters    = (myParams->side == 'L' || myParams->side == 'l') ? childParams.n : childParams.m;
    childParams.side              = myParams->side;
    childParams.uplo              = myParams->uplo;
    childParams.transa            = myParams->transa;
    childParams.diag              = myParams->diag;
    childParams.trsmTemplate      = trsmTemplate;
    childParams.backingStoreA     = myDeps->dba.guid;
    childParams.idealWorkloadSize = idealWorkloadSize;

    ocrGuid_t childEdt;
    ocrGuid_t workerTreeOutputEvent;
    ocrEdtCreate(&childEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &workerTreeOutputEvent);

// Plumb dependencies for posture, worker, and deposture, in reverse order.

    ADD_DEPENDENCE(workerTreeOutputEvent, depostureA, ocrPosture3dSubblockOutwardDeps_t, doneWithPosturedDb,   RO);
    ADD_DEPENDENCE(posturedA,             depostureA, ocrPosture3dSubblockOutwardDeps_t, postured,             RO);
    ADD_DEPENDENCE(myDeps->dba.guid,      depostureA, ocrPosture3dSubblockOutwardDeps_t, backingStore,         RW);
    ADD_DEPENDENCE(NULL_GUID,             depostureA, ocrPosture3dSubblockOutwardDeps_t, optionalTriggerEvent, RO);

    ADD_DEPENDENCE(posturedA,             childEdt,   NAME(trsm_workerDeps_t),           dbPosturedA,          RO);
    ADD_DEPENDENCE(myDeps->dbb.guid,      childEdt,   NAME(trsm_workerDeps_t),           dbb,                  RW);
    ADD_DEPENDENCE(NULL_GUID,             childEdt,   NAME(trsm_workerDeps_t),           optionalTrigger,      RO);

    ADD_DEPENDENCE(myDeps->dba.guid,      postureA,   ocrPosture3dSubblockInwardDeps_t,  backingStore,         RO);
    ADD_DEPENDENCE(NULL_GUID,             postureA,   ocrPosture3dSubblockInwardDeps_t,  optionalTriggerEvent, RO);

    //printf ("        %s exiting\n", STRINGIFY(NAME(trsm_task))); fflush(stdout);
} // ?trsm_task


ocrGuid_t NAME(trsm_workerTask) (            // Either perform the operation on the input workload or split it among up to eight children.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    //printf ("Entered %s\n", STRINGIFY(NAME(trsm_workerTask))); fflush(stdout);

    NAME(trsm_workerParams_t) * myParams = (NAME(trsm_workerParams_t) *) paramv;
    NAME(trsm_workerDeps_t)   * myDeps   = (NAME(trsm_workerDeps_t)   *) depv;

    if (myParams->backingStoreA != myDeps->dbPosturedA.guid) {  // Submatrix A was postured by migrating it closer to the computational agent(s) that will utilize it.  Adjust parameters accordingly.
        //printf ("        %s:  Submatrix A was migrated.  Adjust worker arguments accordingly.\n", STRINGIFY(NAME(trsm_workerTask)));
        myParams->lda = myParams->m;
        myParams->offseta = 0;
        myParams->backingStoreA = myDeps->dbPosturedA.guid;     // This migrated subblock becomes the "backing store" for any child workers this worker might spawn.
    } else {
        //printf ("        %s:  Submatrix A was NOT migrated.  Use backingStore.\n", STRINGIFY(NAME(trsm_workerTask)));
    }

    BLAS_int_t    m                 = myParams->m;
    BLAS_int_t    n                 = myParams->n;
    BLAS_MATH_t   alpha             = myParams->alpha;
    BLAS_int_t    lda               = myParams->lda;
    BLAS_int_t    ldb               = myParams->ldb;
    BLAS_int_t    offseta           = myParams->offseta;
    BLAS_int_t    offsetb           = myParams->offsetb;
    BLAS_int_t    workerFirstIter   = myParams->workerFirstIter;
    BLAS_int_t    workerNumIters    = myParams->workerNumIters;
    char          side              = myParams->side;
    char          uplo              = myParams->uplo;
    char          transa            = myParams->transa;
    char          diag              = myParams->diag;
    ocrGuid_t     trsmTemplate      = myParams->trsmTemplate;
    ocrGuid_t     backingStoreA     = myParams->backingStoreA;
    BLAS_int_t    idealWorkloadSize = myParams->idealWorkloadSize;

    ocrGuid_t     gMatA             = myDeps->dbPosturedA.guid;
    ocrGuid_t     gMatB             = myDeps->dbb.guid;
    BLAS_MATH_t * pMatA             = ((BLAS_MATH_t *) (myDeps->dbPosturedA.ptr)) + offseta;
    BLAS_MATH_t * pMatB             = ((BLAS_MATH_t *) (myDeps->dbb.ptr)) + offsetb;

    BLAS_int_t    nrowa;
    bool isLeftSide = true;
    if (side == 'L' || side == 'l') {
        nrowa = m;
    } else if (side == 'R' || side == 'r') {
        nrowa = n;
        isLeftSide = false;
    } else {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 1);
        return;
    }

    if (uplo != 'U' && uplo != 'u' && uplo != 'L' && uplo != 'l') {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 2);
        return;
    }

    if (transa != 'N' && transa != 'n' && transa != 'T' && transa != 't' && transa != 'C' && transa != 'c') {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 3);
        return;
    }

    if (diag != 'U' && diag != 'u' && diag != 'N' && diag != 'n') {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 4);
        return;
    }

    if (m < 0) {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 5);
        return;
    }

    if (n < 0) {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 6);
        return;
    }

    if (lda < MAX(1, nrowa)) {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 9);
        return;
    }

    if (ldb < MAX(1, n)) {
        xerbla (STRINGIFY(NAME(trsm_workerTask)), 11);
        return;
    }

    // Quick return if possible.
    if (m == 0 || n == 0) return;

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(trsm--full-workload)),(n*m*(m-1)/2),false)
#else
    REPORT_WORKLOAD_HIGH_WATER_MARKS(A,10000,STRINGIFY(NAME(trsm--full-workload)),(n*m*(m-1)/2*4),false)
#endif

    switch (CODEVERSION) {
    case 0: break;           // No splitting of workload done for reference version.
    case 1:
    {
        u64 approxFlopsPerIter;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        if (isLeftSide) {
            approxFlopsPerIter = MAX(1, (((u64) m) * (((u64) m) - 1)) >> 1);
        } else {
            approxFlopsPerIter = MAX(1, (((u64) n) * (((u64) n) - 1)) >> 1);
        }
#else
        if (isLeftSide) {
            approxFlopsPerIter = MAX(1, (((u64) m) * (((u64) m) - 1)) << 1);
        } else {
            approxFlopsPerIter = MAX(1, (((u64) n) * (((u64) n) - 1)) << 1);
        }
#endif
        u64 idealNumItersPerChild = MAX(1, (idealWorkloadSize + (idealWorkloadSize>>3)) / approxFlopsPerIter);
        if (workerNumIters > idealNumItersPerChild) {  // If the number of rows/columns THIS worker is charged to process exceeds ideal number of rows/columns, we will split workload.
            while (workerNumIters > (idealNumItersPerChild << 3)) {   // We will split THIS worker into no more than eight children (which might then be split further.)
                idealNumItersPerChild <<= 3;
            }
            NAME(trsm_workerParams_t) childParams = *((NAME(trsm_workerParams_t) *) paramv);
            ocrGuid_t serializationEvent[8];
            ocrGuid_t trsmEdt[8];
            int i;
            for (i=0 ; workerNumIters > 0; i++, workerNumIters -= idealNumItersPerChild) {
                if (idealNumItersPerChild > workerNumIters) idealNumItersPerChild = workerNumIters;   // Last child we spawn might be just a remnant, the leftovers of what we already spun off.
                childParams.workerFirstIter = workerFirstIter;
                childParams.workerNumIters  = idealNumItersPerChild;
                workerFirstIter += idealNumItersPerChild;
                ocrEdtCreate(&trsmEdt[i], trsmTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &serializationEvent[i]);
            }

            for (i--; i > 0; i--) {
                // Add the dependences, in reverse order.
                ADD_DEPENDENCE(gMatA,                   trsmEdt[i], NAME(trsm_workerDeps_t), dbPosturedA,     RO);
                ADD_DEPENDENCE(gMatB,                   trsmEdt[i], NAME(trsm_workerDeps_t), dbb,             RW);
#if defined(TRAVERSE_TOPOLOGY_DEPTH_FIRST)
                ADD_DEPENDENCE(serializationEvent[i-1], trsmEdt[i], NAME(trsm_workerDeps_t), optionalTrigger, RO);
#else
                ADD_DEPENDENCE(NULL_GUID,               trsmEdt[i], NAME(trsm_workerDeps_t), optionalTrigger, RO);
#endif
            }
            ADD_DEPENDENCE(gMatA,     trsmEdt[0], NAME(trsm_workerDeps_t), dbPosturedA,     RO);
            ADD_DEPENDENCE(gMatB,     trsmEdt[0], NAME(trsm_workerDeps_t), dbb,             RW);
            ADD_DEPENDENCE(NULL_GUID, trsmEdt[0], NAME(trsm_workerDeps_t), optionalTrigger, RO);
            return NULL_GUID;  // All children spawned.  This worker is done.
        }
    }
    break;
    default:
    {
        printf ("******** Unimplemented case %ld, in %s, line %ld.  Dropping back to reference version\n", (u64) CODEVERSION, __FILE__, (u64) __LINE__);
        break;
    }
    }

    BLAS_MATH_t zero, one;
    xSETc (zero, 0.0, 0.0);
    xSETc (one,  1.0, 0.0);
    if (xCMP_EQ(alpha, zero)) {
        BLAS_int_t i, j;
        for (i = 0; i < m; i++) {
            for (j = 0; j < n; j++) {
                xSET(pMatB[i*ldb+j],zero);
            }
        }
        return;
    }

#define side_L    0 // left
#define side_R    1 // right
#define uplo_L    0 // lower
#define uplo_U    2 // upper
#define diag_U    0 // unit
#define diag_N    4 // non-unit
#define transa_N  0 // no transpose
#define transa_T  8 // transpose
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define transa_C  8 // conjugate transpose is the same as transpose for non-complex data types
#else
#define transa_C 16 // conjugate transpose
#endif
    if (isLeftSide) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        REPORT_WORKLOAD_HIGH_WATER_MARKS(A,100,STRINGIFY(NAME(trsm--bolus-workload)),(workerNumIters*m*(m-1)/2),false)
#else
        REPORT_WORKLOAD_HIGH_WATER_MARKS(A,100,STRINGIFY(NAME(trsm--bolus-workload)),(workerNumIters*m*(m-1)/2*4),false)
#endif
    } else {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        REPORT_WORKLOAD_HIGH_WATER_MARKS(A,100,STRINGIFY(NAME(trsm--bolus-workload)),(workerNumIters*n*(n-1)/2),false)
#else
        REPORT_WORKLOAD_HIGH_WATER_MARKS(A,100,STRINGIFY(NAME(trsm--bolus-workload)),(workerNumIters*n*(n-1)/2*4),false)
#endif
    }

    int caseSelector =
        (isLeftSide ? side_L : side_R) +
        ((uplo   == 'L' || uplo   == 'l') ? uplo_L : uplo_U) +
        ((transa == 'N' || transa == 'n') ? transa_N : ((transa == 'T' || transa == 'T') ? transa_T : transa_C)) +
        ((diag   == 'U' || diag   == 'u') ? diag_U : diag_N);
    switch (caseSelector) {
    case side_L + uplo_L + transa_N + diag_U:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            if (xCMP_NE(alpha, one)) {
                for (i = 0; i < m; i++) {
                    xMUL(pMatB[i*ldb+j], alpha, pMatB[i*ldb+j]);
                }
            }
            for (k = 0; k < m; k++) {
                if (xCMP_NE(pMatB[k*ldb+j],zero)) {
                    for (i = k+1; i < m; i++) {
                        xFMS(pMatB[i*ldb+j],pMatB[k*ldb+j],pMatA[i*lda+k]);
                    }
                }
            }
        }
        break;
    }
    case side_L + uplo_L + transa_N + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            if (xCMP_NE(alpha, one)) {
                for (i = 0; i < m; i++) {
                    xMUL(pMatB[i*ldb+j], alpha, pMatB[i*ldb+j]);
                }
            }
            for (k = 0; k < m; k++) {
                if (xCMP_NE(pMatB[k*ldb+j],zero)) {
                    xDIV(pMatB[k*ldb+j],pMatB[k*ldb+j],pMatA[k*lda+k]);
                    for (i = k+1; i < m; i++) {
                        xFMS(pMatB[i*ldb+j],pMatB[k*ldb+j],pMatA[i*lda+k]);
                    }
                }
            }
        }
        break;
    }
    case side_L + uplo_L + transa_T + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            for (i = m-1; i >= 0; i--) {
                BLAS_MATH_t temp;
                xMUL(temp, alpha, pMatB[i*ldb+j]);
                for (k = i+1; k < m; k++) {
                    xFMS(temp,pMatA[k*lda+i],pMatB[k*ldb+j]);
                }
                xDIV(pMatB[i*ldb+j],temp,pMatA[i*lda+i]);
            }
        }
        break;
    }
    case side_L + uplo_U + transa_N + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            if (xCMP_NE(alpha, one)) {
                for (i = 0; i < m; i++) {
                    xMUL(pMatB[i*ldb+j], alpha, pMatB[i*ldb+j]);
                }
            }
            for (k = m-1; k >= 0; k--) {
                if (xCMP_NE(pMatB[k*ldb+j],zero)) {
                    xDIV(pMatB[k*ldb+j], pMatB[k*ldb+j], pMatA[k*lda+k]);
                    for (i = 0; i < k; i++) {
                        xFMS(pMatB[i*ldb+j],pMatB[k*ldb+j],pMatA[i*lda+k]);
                    }
                }
            }
        }
        break;
    }
    case side_L + uplo_U + transa_T + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            for (i = 0; i < m; i++) {
                BLAS_MATH_t temp;
                xMUL(temp,alpha,pMatB[i*ldb+j]);
                for (k = 0; k < i; k++) {
                    xFMS(temp,pMatA[k*lda+i],pMatB[k*ldb+j]);
                }
                xDIV(pMatB[i*ldb+j],temp,pMatA[i*lda+i]);
            }
        }
        break;
    }
    case side_R + uplo_U + transa_T + diag_N:
    {
        BLAS_int_t i,j,k;
        for (k = n-1; k >= 0; k--) {
            BLAS_MATH_t temp;
            xDIV(temp,one,pMatA[k*lda+k]);
            for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                xMUL(pMatB[i*ldb+k],temp,pMatB[i*ldb+k]);
            }
            for (j = 0; j < k; j++) {
                if (xCMP_NE(pMatA[j*lda+k],zero)) {
                    xSET(temp,pMatA[j*lda+k]);
                    for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                        xFMS(pMatB[i*ldb+j],temp,pMatB[i*ldb+k]);
                    }
                }
            }
            if (xCMP_NE(alpha,one)) {
                for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                    xMUL(pMatB[i*ldb+k],alpha,pMatB[i*ldb+k]);
                }
            }
        }
        break;
    }
    case side_R + uplo_L + transa_T + diag_N:
    {
        BLAS_int_t i,j,k;
        for (k = 0; k < n; k++) {
            BLAS_MATH_t temp;
            xDIV(temp,one,pMatA[k*lda+k]);
            for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                xMUL(pMatB[i*ldb+k],temp,pMatB[i*ldb+k]);
            }
            for (j = k+1; j < n; j++) {
                if (xCMP_NE(pMatA[j*lda+k],zero)) {
                    xSET(temp,pMatA[j*lda+k]);
                    for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                        xFMS(pMatB[i*ldb+j],temp,pMatB[i*ldb+k]);
                    }
                }
            }
            if (xCMP_NE(alpha,one)) {
                for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                    xMUL(pMatB[i*ldb+k],alpha,pMatB[i*ldb+k]);
                }
            }
        }
        break;
    }
    case side_R + uplo_L + transa_N + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = n-1; j >= 0; j--) {
            if (xCMP_NE(alpha, one)) {
                for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                    xMUL(pMatB[i*ldb+j],alpha,pMatB[i*ldb+j]);
                }
            }
            for (k = j+1; k < n; k++) {
                if (xCMP_NE(pMatA[k*lda+j],zero)) {
                    for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                        xFMS(pMatB[i*ldb+j],pMatA[k*lda+j],pMatB[i*ldb+k]);
                    }
                }
            }
            BLAS_MATH_t temp;
            xDIV(temp,one,pMatA[j*lda+j]);
            for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                xMUL(pMatB[i*ldb+j],pMatB[i*ldb+j],temp);
            }
        }
        break;
    }
#if defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
    case side_L + uplo_L + transa_C + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            for (i = m-1; i >= 0; i--) {
                BLAS_MATH_t temp1, temp2;
                xMUL(temp1,alpha,pMatB[i*ldb+j]);
                for (k = i+1; k < m; k++) {
                    xCONJUGATE(temp2,pMatA[k*lda+i]);
                    xFMS(temp1,temp2,pMatB[k*ldb+j]);
                }
                xCONJUGATE(temp2,pMatA[i*lda+i]);
                xDIV(pMatB[i*ldb+j],temp1,temp2);
            }
        }
        break;
    }
    case side_L + uplo_U + transa_C + diag_N:
    {
        BLAS_int_t i,j,k;
        for (j = workerFirstIter; j < workerFirstIter+workerNumIters; j++) {
            for (i = 0; i < m; i++) {
                BLAS_MATH_t temp1, temp2;
                xMUL(temp1,alpha,pMatB[i*ldb+j]);
                for (k = 0; k < i; k++) {
                    xCONJUGATE(temp2,pMatA[k*lda+i]);
                    xFMS(temp1,temp2,pMatB[k*ldb+j]);
                }
                xCONJUGATE(temp2,pMatA[i*lda+i]);
                xDIV(pMatB[i*ldb+j],temp1,temp2);
            }
        }
        break;
    }
    case side_R + uplo_L + transa_C + diag_N:
    {
        BLAS_int_t i,j,k;
        for (k = 0; k < n; k++) {
            BLAS_MATH_t temp1, temp2;
            xCONJUGATE(temp2,pMatA[k*lda+k]);
            xDIV(temp1,one,temp2);
            for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                xMUL(pMatB[i*ldb+k],temp1,pMatB[i*ldb+k]);
            }
            for (j = k+1; j < n; j++) {
                if (xCMP_NE(pMatA[j*lda+k],zero)) {
                    xCONJUGATE(temp1,pMatA[j*lda+k]);
                    for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                        xFMS(pMatB[i*ldb+j],temp1,pMatB[i*ldb+k]);
                    }
                }
            }
            if (xCMP_NE(alpha,one)) {
                for (i = workerFirstIter; i < workerFirstIter+workerNumIters; i++) {
                    xMUL(pMatB[i*ldb+k],alpha,pMatB[i*ldb+k]);
                }
            }
        }
        break;
    }
#endif
    default:
    {
        xerbla (STRINGIFY(NAME(trsm)), 1001);     // Case not yet implemented.
        break;
    }
    }
    //printf("        Exiting %s.\n", STRINGIFY(NAME(trsm_workerTask))); fflush(stdout);
    return NULL_GUID;
} // ?trsm_workerTask

static ocrGuid_t NAME(trsm_wrapupTask) (  // This EDT gains control when ?trsm_task and all that is spawns are done.  It "dethunks" the output original storage passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    //printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(trsm_wrapupTask))); fflush(stdout); // TODO

    NAME(trsm_wrapupParams_t) * wrapupParams = (NAME(trsm_wrapupParams_t) *) paramv;
    NAME(trsm_wrapupDeps_t)   * wrapupDeps   = (NAME(trsm_wrapupDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pMatB_arr = wrapupParams->b;
    BLAS_MATH_t * pMatB_db  = wrapupDeps->dbb.ptr;
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->m; j++) {
            pMatB_arr[j] = pMatB_db[j];
        }
        pMatB_db  += wrapupParams->ldb_datablock;
        pMatB_arr += wrapupParams->ldb_array;
    }

    ocrDbDestroy (wrapupDeps->dba.guid);         // Destroy datablock for the matrix A.
    ocrDbDestroy (wrapupDeps->dbb.guid);         // Destroy datablock for the matrix B.

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(trsm_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the GETRF library function should NOT be pulling the plug!
    return NULL_GUID;
} // trsmWrapupTask

#undef CODEVERSION
