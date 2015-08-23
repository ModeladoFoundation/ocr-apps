/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?GETRF,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  This right-looking Level 3 BLAS function computes an LU factorization of a general M-by-N matrix using partial pivoting with row interchanges.
 *
 * The factorization has the form
 *    A = P * L * U
 * where P is a permutation matrix, L is lower triangular with unit diagonal elements (lower trapezoidal if m > n), and U is upper triangular (upper trapezoidal if m < n).
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


typedef struct {                          // These are the scalar variables that are passed to the GETRF thunking EDT as its paramv argument.
    BLAS_int_t    m;                      // The number of rows of the matrix.
    BLAS_int_t    n;                      // The number of columns of the matrix.
    BLAS_int_t    ld_array;               // The leading dimension of the matrix as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ld_datablock;           // The leading dimension of the matrix as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * mat;                    // Address of input matrix.
} NAME(getrf_thunkParams_t);

typedef struct {                          // These are the dependencies that the GETRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GETRF status.
} NAME(getrf_thunkDeps_t);

static ocrGuid_t NAME(getrf_thunkTask) (  // Moves array-based input matrix (passed by standard GETRF API) into OCR datablock, then spawns top-level worker EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the GETRF worker as its paramv argument.
    BLAS_int_t    m;                      // The number of rows of the matrix.
    BLAS_int_t    n;                      // The number of columns of the matrix.
    BLAS_int_t    ld;                     // The leading dimension of the matrix (in elements of BLAS_MATH_t).
    ocrGuid_t     workerTemplate;         // GUID of EdtTemplate for the worker.  Passed down so that worker EDT's can be instantiated.

    BLAS_int_t    j;                      // GETRF main loop index.  Original code loop is "for (j = 0; j < jLimit; j += jInc) ..."
    BLAS_int_t    jLimit;                 // GETRF main loop limit.
    BLAS_int_t    jInc;                   // GETRF main loop increment.
    BLAS_int_t    workerTaskStep;         // Which step of the worker's continuation is next?
} NAME(getrf_workerParams_t);

typedef struct {                          // These are the dependencies that the GETRF worker task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   trigger;                // Trigger fired by completion of predecessor (e.g. previous worker's final operation, which is the GEMM operation).
} NAME(getrf_workerDeps_t);

static ocrGuid_t NAME(getrf_workerTask) ( // This EDT performs one iteration of the blocked GETRF main loop, spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the GETRF wrapup EDT as its paramv argument.
    BLAS_int_t    m;                      // The number of rows of the matrix.
    BLAS_int_t    n;                      // The number of columns of the matrix.
    BLAS_int_t    ld_array;               // The leading dimension of the matrix as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ld_datablock;           // The leading dimension of the matrix as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * mat;                    // Address of input matrix.
    BLAS_int_t  * pivotIdx;               // Address of output array of pivot indices.
    BLAS_int_t  * info;                   // Address of output scalar of info.
} NAME(getrf_wrapupParams_t);

typedef struct {                          // These are the dependencies that the GETRF wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when all GETRF workers are done.
} NAME(getrf_wrapupDeps_t);

static ocrGuid_t NAME(getrf_wrapupTask) ( // Writes matrix, pivot indices, and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?GETRF library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as vector of pivot indices and scalar status info.

void NAME(getrf) (
    const BLAS_int_t    m,        // Specifies the number of rows of the matrix.
    const BLAS_int_t    n,        // Specifies the number of columns of the matrix.
    const BLAS_MATH_t * mat,      // Input array, size m*lda.
    const BLAS_int_t    ld,       // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    const BLAS_int_t  * pivotIdx, // Output array of pivot indices.  Must have enough room for MIN(m,n) elements of size BLAS_int_t.  These are one-based index values.
    const BLAS_int_t  * info)     // Output status value.  If <0, this indicates an error condition.  If >0, this is a one-based index of a zero coefficient.  If 0, completely normal.
{
    printf("Entered Standard API %s function.\n", STRINGIFY(NAME(getrf))); fflush(stdout);

// Create the datablocks:

    ocrGuid_t  dbMat;         // Guid of datablock for matrix
    ocrGuid_t  dbPivotIdx;    // Guid of datablock for pivot index output.
    ocrGuid_t  dbInfo;        // Guid of datablock for info output.

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

    BLAS_int_t pitchOfMat = (n + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatInBytes = (m * pitchOfMat) * sizeof(BLAS_MATH_t);
    ocrDbCreate (&dbMat, (void**) &addr, sizeOfMatInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    BLAS_int_t sizeOfPivotIdxArrayInBytes = MIN(m,n) * sizeof(BLAS_int_t);
    ocrDbCreate (&dbPivotIdx, (void**) &addr, sizeOfPivotIdxArrayInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    ocrDbCreate (&dbInfo, (void**) &addr, sizeof(BLAS_int_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t getrfThunkTemplate;
    NAME(getrf_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&getrfThunkTemplate, NAME(getrf_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(getrf_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the result of GETRF (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t getrfWrapupTemplate;
    NAME(getrf_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&getrfWrapupTemplate, NAME(getrf_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(getrf_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.m                        = m;
    thunkParams.n                        = n;
    thunkParams.ld_array                 = ld;
    thunkParams.ld_datablock             = pitchOfMat;
    thunkParams.mat                      = (BLAS_MATH_t *) mat;

    ocrGuid_t getrfThunkEdt;
    ocrGuid_t getrfThunkOutputEvent;
    ocrEdtCreate(&getrfThunkEdt, getrfThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &getrfThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API GETRF func.
    wrapupParams.m               = m;
    wrapupParams.n               = n;
    wrapupParams.ld_array        = ld;
    wrapupParams.ld_datablock    = pitchOfMat;
    wrapupParams.mat             = (BLAS_MATH_t *) mat;
    wrapupParams.pivotIdx        = (BLAS_int_t  *) pivotIdx;
    wrapupParams.info            = (BLAS_int_t  *) info;

    ocrGuid_t getrfWrapupEdt;
    ocrEdtCreate(&getrfWrapupEdt, getrfWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dbMat,                 getrfWrapupEdt, NAME(getrf_wrapupDeps_t), dbMat,               RO);
    ADD_DEPENDENCE(dbPivotIdx,            getrfWrapupEdt, NAME(getrf_wrapupDeps_t), dbPivotIdx,          RO);
    ADD_DEPENDENCE(dbInfo,                getrfWrapupEdt, NAME(getrf_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(getrfThunkOutputEvent, getrfWrapupEdt, NAME(getrf_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dbMat,                getrfThunkEdt, NAME(getrf_thunkDeps_t), dbMat,      RW);
    ADD_DEPENDENCE(dbPivotIdx,           getrfThunkEdt, NAME(getrf_thunkDeps_t), dbPivotIdx, RW);
    ADD_DEPENDENCE(dbInfo,               getrfThunkEdt, NAME(getrf_thunkDeps_t), dbInfo,     RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GETRF operation.\n", STRINGIFY(NAME(getrf))); fflush(stdout);

} // ?getrf


// Receiving array-based input matrix from the standard API for GETRF, this EDT moves it into OCR datablock, creates datablocks for pivot indices and info, then passes them to a top-level child worker EDT.
static ocrGuid_t NAME(getrf_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(getrf_thunkTask))); fflush(stdout);

    NAME(getrf_thunkParams_t) * thunkParams = (NAME(getrf_thunkParams_t) *) paramv;
    NAME(getrf_thunkDeps_t)   * thunkDeps   = (NAME(getrf_thunkDeps_t)   *) depv;

    u64 i, j;
    const BLAS_MATH_t * pMat_arr = thunkParams->mat;
    BLAS_MATH_t * pMat_db  = thunkDeps->dbMat.ptr;

    for (i = 0; i < thunkParams->m; i++) {
        for (j = 0; j < thunkParams->n; j++) {
            pMat_db[j] = pMat_arr[j];
        }
        pMat_db  += thunkParams->ld_datablock;
        pMat_arr += thunkParams->ld_array;
    }

    ocrGuid_t getrfTemplate;
    NAME(getrf_edtParams_t) NAME(getrf_edtParams);
    ocrEdtTemplateCreate(&getrfTemplate, NAME(getrf_task), sizeof(NAME(getrf_edtParams))/sizeof(u64), sizeof(NAME(getrf_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(getrf_edtParams).m              = thunkParams->m;
    NAME(getrf_edtParams).n              = thunkParams->n;
    NAME(getrf_edtParams).ld             = thunkParams->ld_datablock;

    ocrGuid_t getrfEdt;
    ocrEdtCreate(&getrfEdt, getrfTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(getrf_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dbMat.guid,      getrfEdt, NAME(getrf_edtDeps_t), dbMat,           RO);
    ADD_DEPENDENCE(thunkDeps->dbPivotIdx.guid, getrfEdt, NAME(getrf_edtDeps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid,     getrfEdt, NAME(getrf_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,                  getrfEdt, NAME(getrf_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(getrf_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?getrf_thunkTask


ocrGuid_t NAME(getrf_task) (           // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    printf ("Entered %s\n", STRINGIFY(NAME(getrf_task))); fflush(stdout);

    NAME(getrf_edtParams_t) * myParams = (NAME(getrf_edtParams_t) *) paramv;
    NAME(getrf_edtDeps_t)   * myDeps   = (NAME(getrf_edtDeps_t)   *) depv;

    BLAS_int_t   m              = myParams->m;
    BLAS_int_t   n              = myParams->n;
    BLAS_int_t   ld             = myParams->ld;

    ocrGuid_t     gMat      = myDeps->dbMat.guid;
    ocrGuid_t     gPivotIdx = myDeps->dbPivotIdx.guid;
    ocrGuid_t     gInfo     = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMat      = (BLAS_MATH_t *) (myDeps->dbMat.ptr);
    BLAS_int_t  * pPivotIdx = (BLAS_int_t  *) (myDeps->dbPivotIdx.ptr);
    BLAS_int_t  * pInfo     = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

// Create a template for the worker EDT.
    ocrGuid_t workerTemplate;
    ocrEdtTemplateCreate(&workerTemplate, NAME(getrf_workerTask), sizeof(NAME(getrf_workerParams_t))/sizeof(u64), sizeof(NAME(getrf_workerDeps_t))/sizeof(ocrEdtDep_t));

    *pInfo = 0;
    // Check argument validity.
    if      (m < 0) *pInfo = -1;
    else if (n < 0) *pInfo = -2;
    else if (ld < MAX(m,1)) *pInfo = -4;
    if (*pInfo != 0) {
        xerbla ("GETRF", -*pInfo);
        return NULL_GUID;
    }
    // Quick return if possible
    if (m == 0 || n == 0) {
        return NULL_GUID;
    }

    NAME(getrf_workerParams_t) workerParams;
    workerParams.m              = m;
    workerParams.n              = n;
    workerParams.ld             = ld;
    workerParams.workerTemplate = workerTemplate;
    workerParams.j              = 0;              // Set up for first worker.
    workerParams.jLimit         = MIN(m,n);
    workerParams.jInc           = ilaenv(1, STRINGIFY(NAME(getrf)), " ", m, n, -1, -1);  // Dig up environment-appropriate value for block size. jInc is called nb in ref code.
    workerParams.workerTaskStep = 1;

    if (workerParams.jInc <= 1 || workerParams.jInc >= workerParams.jLimit) {
        // Use unblocked code:   Just invoke DGETF2 ***ONCE***, and that's all!

        NAME(getf2)(m, n, pMat, ld, pPivotIdx, pInfo);
        return NULL_GUID;
    }

    // Use blocked code:  Do so by invoking the worker EDT, set up for the first iteration of the main loop in the reference code.
    ocrGuid_t getrfWorkerEdt;
    ocrEdtCreate(&getrfWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&workerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the worker.
    ADD_DEPENDENCE(gMat,                 getrfWorkerEdt, NAME(getrf_workerDeps_t), dbMat,      RW);
    ADD_DEPENDENCE(gPivotIdx,            getrfWorkerEdt, NAME(getrf_workerDeps_t), dbPivotIdx, RW);
    ADD_DEPENDENCE(gInfo,                getrfWorkerEdt, NAME(getrf_workerDeps_t), dbInfo,     RW);
    ADD_DEPENDENCE(NULL_GUID,            getrfWorkerEdt, NAME(getrf_workerDeps_t), trigger,    RO);
    printf ("        %s exiting\n", STRINGIFY(NAME(getrf_task))); fflush(stdout);
    return NULL_GUID;
} // ?getrf_task


static ocrGuid_t NAME(getrf_workerTask) (    // This EDT performs one iteration of the blocked GETRF main loop, spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    NAME(getrf_workerParams_t) * myParams = (NAME(getrf_workerParams_t) *) paramv;
    NAME(getrf_workerDeps_t)   * myDeps   = (NAME(getrf_workerDeps_t)   *) depv;

    BLAS_int_t    m              = myParams->m;
    BLAS_int_t    n              = myParams->n;
    BLAS_int_t    ld             = myParams->ld;
    BLAS_int_t    workerTemplate = myParams->workerTemplate;
    BLAS_int_t    j              = myParams->j;
    BLAS_int_t    jLimit         = myParams->jLimit;
    BLAS_int_t    jInc           = myParams->jInc;
    BLAS_int_t    workerTaskStep = myParams->workerTaskStep;

    ocrGuid_t     gMat           = myDeps->dbMat.guid;
    ocrGuid_t     gPivotIdx      = myDeps->dbPivotIdx.guid;
    ocrGuid_t     gInfo          = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMat           = (BLAS_MATH_t *) (myDeps->dbMat.ptr);
    BLAS_int_t  * pPivotIdx      = (BLAS_int_t  *) (myDeps->dbPivotIdx.ptr);
    BLAS_int_t  * pInfo          = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    printf ("Entered %s  j = %ld, jLimit = %ld, jInc = %ld\n", STRINGIFY(NAME(getrf_workerTask)), (u64) j, (u64) jLimit, (u64) jInc); fflush(stdout);

    BLAS_MATH_t   minusOne, one;
    xSETc (minusOne, -1.0, 0.0);
    xSETc (one,       1.0, 0.0);

// Create a template for the natural OCR EDT for spawning GEMM.
    ocrGuid_t gemmTemplate;
    ocrEdtTemplateCreate(&gemmTemplate, NAME(gemm_task), sizeof(NAME(gemm_edtParams_t))/sizeof(u64), sizeof(NAME(gemm_edtDeps_t))/sizeof(ocrEdtDep_t));

    ocrGuid_t trsmTemplate;
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    for (/*j = value passed in*/  ; j < jLimit; j += jInc) {   // Iterate in THIS worker until we need to spawn a GEMM, at which time we will also spawn a follow-on worker to continue the iterations.
        BLAS_int_t  blkSz = MIN(MIN(m,n)-j, jInc);
        switch (workerTaskStep) {   // step through the phases of the worker.
        case 1: // Do first step.  That's everything up to spawining TRSM, plus spawning step two of this GETRF worker.
        {
            // Factor diagonal and subdiagonal blocks and test for exact singularity.
            BLAS_int_t iinfo;
            NAME(getf2)(m-j, blkSz, &pMat[j*ld+j], ld, &pPivotIdx[j], &iinfo);
            // Adjust info and the pivot indices.
            if (*pInfo == 0 && iinfo > 0) {
                *pInfo = iinfo + j;    // GETF2 returned a one-based index, but it was only valid from its own perspective.  Adjust by our perspective, i.e. add j, and record as info to return to caller.
            }
            BLAS_int_t i;
            for (i = j; i < MIN(m, j+blkSz); i++) {
                pPivotIdx[i] += j;     // GETF2 returned one-based pivot indices, but they were valid from its own perspective.  Adjust by our perspective, i.e. add j.
            }

            // Apply interchanges to columns 0:j.
            NAME(laswp)(j, pMat, ld, j, j+blkSz-1, pPivotIdx, (BLAS_int_t) 1);

            if (j+blkSz < n) {
                // Apply interchanges to columns j+blkSz:n-1.
                NAME(laswp)(n-j-blkSz, &pMat[(0*ld)+(j+blkSz)], ld, j, j+blkSz-1, pPivotIdx, (BLAS_int_t) 1);

                // Compute block row of U.
                // NAME(trsm)("Left", "Lower", "No transpose", "Unit", blkSz, n-j-blkSz, one, &pMat[j*ld+j], ld, &pMat[j*ld+(j+blkSz)], ld);
                NAME(trsm_edtParams_t) NAME(LLNUtrsm_edtParams);
                NAME(LLNUtrsm_edtParams).m              = blkSz;           // Input:  Number of rows of B.
                NAME(LLNUtrsm_edtParams).n              = n-j-blkSz;       // Input:  Number of columns of B.
                NAME(LLNUtrsm_edtParams).alpha          = one;             // Input:  Scalar by which to multiply each matrix B value before it is modified.
                NAME(LLNUtrsm_edtParams).lda            = ld;              // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(LLNUtrsm_edtParams).ldb            = ld;              // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(LLNUtrsm_edtParams).offseta        = j*ld+j;          // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
                NAME(LLNUtrsm_edtParams).offsetb        = j*ld+(j+blkSz);  // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
                NAME(LLNUtrsm_edtParams).side           = 'L';             // Input:  Left         ("L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B)
                NAME(LLNUtrsm_edtParams).uplo           = 'L';             // Input:  Lower        ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
                NAME(LLNUtrsm_edtParams).transa         = 'N';             // Input:  No transpose ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
                NAME(LLNUtrsm_edtParams).diag           = 'U';             // Input:  Unit         ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

                ocrGuid_t LLNUtrsmEdt;
                ocrGuid_t LLNU_done;
                ocrEdtCreate(&LLNUtrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(LLNUtrsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &LLNU_done);

                // Spawn the continuation of this GETRF worker to the step 2.
                NAME(getrf_workerParams_t) childParams;
                childParams.m              = m;
                childParams.n              = n;
                childParams.ld             = ld;
                childParams.workerTemplate = workerTemplate;
                childParams.j              = j;
                childParams.jLimit         = jLimit;
                childParams.jInc           = jInc;
                childParams.workerTaskStep = 2;

                ocrGuid_t childWorkerEdt;
                ocrEdtCreate(&childWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

                // Add the dependences to the worker and TRSM, in reverse order.
                ADD_DEPENDENCE(gMat,      childWorkerEdt, NAME(getrf_workerDeps_t), dbMat,      RW);
                ADD_DEPENDENCE(gPivotIdx, childWorkerEdt, NAME(getrf_workerDeps_t), dbPivotIdx, RW);
                ADD_DEPENDENCE(gInfo,     childWorkerEdt, NAME(getrf_workerDeps_t), dbInfo,     RW);
                ADD_DEPENDENCE(LLNU_done, childWorkerEdt, NAME(getrf_workerDeps_t), trigger,    RO);

                ADD_DEPENDENCE(gMat,      LLNUtrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
                ADD_DEPENDENCE(gMat,      LLNUtrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
                ADD_DEPENDENCE(NULL_GUID, LLNUtrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

                printf ("        %s exiting (Step 1)\n", STRINGIFY(NAME(getrf_workerTask))); fflush(stdout);
                return NULL_GUID;
            }
        } // case 1.  If control reaches here, it is CORRECT to fall into case 2.  The abscence of usual "break" is NOT a bug!
        case 2:  // Do second step, the continuation of the worker task after the TRSM task (or fall-through, if TRSM was NOT done.
        {
            if (j+blkSz < n) {
                if (j+blkSz < m) {
                    // Update trailing submatrix:  Do so by spawning GEMM.  Also spawn a follow-on worker instance, and make it take over when GEMM finishes.
                    NAME(gemm_edtParams_t) gemmParams;
                    gemmParams.transa          = 'N';        // No transpose matrix A.
                    gemmParams.transb          = 'N';        // No transpose matrix B.
                    gemmParams.m               = m-j-blkSz;
                    gemmParams.n               = n-j-blkSz;
                    gemmParams.k               = blkSz;
                    gemmParams.alpha           = minusOne;
                    gemmParams.beta            = one;
                    gemmParams.offseta         = (j+blkSz)*ld + (j      );
                    gemmParams.offsetb         = (j      )*ld + (j+blkSz);
                    gemmParams.offsetc         = (j+blkSz)*ld + (j+blkSz);
                    gemmParams.lda             = ld;
                    gemmParams.ldb             = ld;
                    gemmParams.ldc             = ld;

                    ocrGuid_t gemmEdt;
                    ocrGuid_t gemmDone;
                    ocrEdtCreate(&gemmEdt, gemmTemplate, EDT_PARAM_DEF, (u64 *) (&gemmParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmDone);

                    if (j + jInc < jLimit) {

                        // Spawn worker here, because there are more iterations of the j-loop to do.

                        NAME(getrf_workerParams_t) childParams;
                        childParams.m              = m;
                        childParams.n              = n;
                        childParams.ld             = ld;
                        childParams.workerTemplate = workerTemplate;
                        childParams.j              = j + jInc;
                        childParams.jLimit         = jLimit;
                        childParams.jInc           = jInc;
                        childParams.workerTaskStep = 1;

                        ocrGuid_t childWorkerEdt;
                        ocrEdtCreate(&childWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

                        // Add the dependences to the worker.
                        ADD_DEPENDENCE(gMat,                 childWorkerEdt, NAME(getrf_workerDeps_t), dbMat,      RW);
                        ADD_DEPENDENCE(gPivotIdx,            childWorkerEdt, NAME(getrf_workerDeps_t), dbPivotIdx, RW);
                        ADD_DEPENDENCE(gInfo,                childWorkerEdt, NAME(getrf_workerDeps_t), dbInfo,     RW);
                        ADD_DEPENDENCE(gemmDone,             childWorkerEdt, NAME(getrf_workerDeps_t), trigger,    RO);
                    }


                    ADD_DEPENDENCE(myDeps->dbMat.guid, gemmEdt, NAME(gemm_edtDeps_t), dba, RO);
                    ADD_DEPENDENCE(myDeps->dbMat.guid, gemmEdt, NAME(gemm_edtDeps_t), dbb, RO);
                    ADD_DEPENDENCE(myDeps->dbMat.guid, gemmEdt, NAME(gemm_edtDeps_t), dbc, RW);
                    ADD_DEPENDENCE(NULL_GUID,          gemmEdt, NAME(gemm_edtDeps_t), optionalTrigger, RO);

                    printf ("        %s exiting (STEP 2)\n", STRINGIFY(NAME(getrf_workerTask))); fflush(stdout);
                    return NULL_GUID;
                }
            }
        } // case
        break;
        default:
        {
            printf ("****  %s  Error: illegal case value %ld\n", STRINGIFY(NAME(getrf_workerTask)), (u64) workerTaskStep);
        }
        } // switch
    } // for
    printf ("        %s exiting (final iteration)\n", STRINGIFY(NAME(getrf_workerTask))); fflush(stdout);
    return NULL_GUID;
} // ?getrf_workerTask


static ocrGuid_t NAME(getrf_wrapupTask) (  // This EDT gains control when all GETRF worker EDTs are done.  It "dethunks" the output matrix C from the OCR datablock to the original array passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("Entered %s\n", STRINGIFY(NAME(getrf_wrapupTask))); fflush(stdout);

    NAME(getrf_wrapupParams_t) * wrapupParams = (NAME(getrf_wrapupParams_t) *) paramv;
    NAME(getrf_wrapupDeps_t)   * wrapupDeps   = (NAME(getrf_wrapupDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pMat_db  = wrapupDeps->dbMat.ptr;
    BLAS_MATH_t * pMat_arr = wrapupParams->mat;
    for (i = 0; i < wrapupParams->m; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
            pMat_arr[j] = pMat_db[j];
        }
        pMat_db  += wrapupParams->ld_datablock;
        pMat_arr += wrapupParams->ld_array;
    }
    BLAS_int_t * pPivotIdx_db  = wrapupDeps->dbPivotIdx.ptr;
    BLAS_int_t * pPivotIdx_arr = wrapupParams->pivotIdx;
    for (i = MIN(wrapupParams->n,wrapupParams->m); i > 0; i--) {
        *pPivotIdx_arr++ = *pPivotIdx_db++;
    }
    BLAS_int_t * pInfo_db = wrapupDeps->dbInfo.ptr;
    BLAS_int_t * pInfo    = wrapupParams->info;
    *pInfo = *pInfo_db;

    ocrDbDestroy (wrapupDeps->dbMat.guid);       // Destroy datablock for the matrix.
    ocrDbDestroy (wrapupDeps->dbPivotIdx.guid);  // Destroy datablock for the pivot indices.
    ocrDbDestroy (wrapupDeps->dbInfo.guid);      // Destroy datablock for the diagnostic info.

#if 0
    pMat_arr = wrapupParams->mat;
    for (i = 0; i < wrapupParams->m; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            printf ("%f ", pMat_arr[j]);
#else
            printf ("(%f,%f) ", pMat_arr[j].real, pMat_arr[j].imag);
#endif
        }
        pMat_arr += wrapupParams->ld_array;
        printf ("\n");
    }
    pMat_arr = wrapupParams->mat;
    for (i = 0; i < wrapupParams->m; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            printf ("0x%lx ", *((u64*) (&(pMat_arr[j]))));
#else
            printf ("(0x%lx,0x%lx) ", *((u64*) (&(pMat_arr[j].real))), *((u64*) (&(pMat_arr[j].imag))));
#endif
        }
        pMat_arr += wrapupParams->ld_array;
        printf ("\n");
    }
#else
    pMat_arr = wrapupParams->mat;
#if defined(BLAS_TYPE__single)
    u32 expected[4][4] = {{0x40a80000, 0xc03ccccd, 0xbf733333, 0xc0733333},
                          {0x3eaf8af8, 0x40790d2a, 0x40180bb4, 0x3ed36200},
                          {0x3e9a166e, 0xbeed1dcd, 0xbfc1c623, 0x3e96f2ba},
                          {0xbe5880bb, 0xbea8e65d, 0x3b9ac5ea, 0x3e0686b8}};

#elif defined(BLAS_TYPE__double)
    u64 expected[4][4] = {{0x4015000000000000L, 0xc00799999999999aL, 0xbfee666666666666L, 0xc00e666666666666L},
                          {0x3fd5f15f15f15f16L, 0x400f21a54d880bb4L, 0x400301767dce434aL, 0x3fda6c405d9f738eL},
                          {0x3fd342cdc67600faL, 0xbfdda3b98727abfdL, 0xbff838c480f5769aL, 0x3fd2de57404287c6L},
                          {0xbfcb101767dce435L, 0xbfd51ccb98d9214fL, 0x3f7358cf5a4cfad6L, 0x3fc0d0d69b95bf3eL}};
#elif defined(BLAS_TYPE__complex)
    u32 expected[4][4][2] = {{{0xc0528f5c, 0xc018f5c3}, {0xbff47ae1, 0x408d70a4}, {0xbe0f5c29, 0xbfaccccd}, {0x3fdc28f6, 0x3faccccd}},
                             {{0x3e735091, 0x3e830d24}, {0x409ca552, 0xbf361bd6}, {0xbeecaff1, 0x3fd92a89}, {0x3f9d0983, 0x3f1e7507}},
                             {{0xbdd0cc41, 0xbf33759f}, {0xbf2b4d64, 0x3ebcdc83}, {0xc0a48670, 0xbf90a2d9}, {0x3f7f8dd5, 0x3ec520b7}},
                             {{0xbf092dc6, 0x3e8a9cc4}, {0xbe50eb14, 0x3f5c30b3}, {0x3c06e3ec, 0x3df7f041}, {0x3e17cc08, 0xbe003ab4}}};
#elif defined(BLAS_TYPE__complex_double)
    u64 expected[4][4][2] = {{{0xc00a51eb851eb852L, 0xc0031eb851eb851fL}, {0xbffe8f5c28f5c28fL, 0x4011ae147ae147aeL}, {0xbfc1eb851eb851ecL, 0xbff599999999999aL}, {0x3ffb851eb851eb85L, 0x3ff599999999999aL}},
                             {{0x3fce6a12266fcaf1L, 0x3fd061a497c70dd3L}, {0x401394aa3b6c38a7L, 0xbfe6c37ab5af5abaL}, {0xbfdd95fe35baa84cL, 0x3ffb255124c226e4L}, {0x3ff3a13072a41af4L, 0x3fe3cea0cdf05d0eL}},
                             {{0xbfba1988152da2aaL, 0xbfe66eb3ef4aee9aL}, {0xbfe569ac8334ca2aL, 0x3fd79b904eb561cfL}, {0xc01490ce05c8731eL, 0xbff2145b24fd6addL}, {0x3feff1babe518fd6L, 0x3fd8a416e8b9c331L}},
                             {{0xbfe125b8b1934db2L, 0x3fd15398665fbdeaL}, {0xbfca1d62bb393332L, 0x3feb861653980f2bL}, {0x3f80dc7cdf16a244L, 0x3fbefe078ae28cdbL}, {0x3fc2f9806159e3d7L, 0xbfc007570958d290L}}};
#else
error: type not defined.
#endif
    u64 ii, jj;
    u64 errCnt = 0;
    BLAS_MATH_t zero;
    xSETc(zero, 0.0, 0.0);
    for (i = 0; i < wrapupParams->m; i+=4) {
        for (j = 0; j < wrapupParams->n; j+=4) {
            for (ii = 0; ii < 4; ii++) {
                for (jj = 0; jj < 4; jj++) {
                    if (i != j) {
                        if (xCMP_NE(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)], zero)) {
                            if (errCnt++ < 100) {
#if defined(BLAS_TYPE__single)
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%x instead of 0.0 or -0.0\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))));
#elif defined(BLAS_TYPE__double)
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%lx instead of 0.0 or -0.0\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))));
#elif defined(BLAS_TYPE__complex)
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag,
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real))),
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag))));
#elif defined(BLAS_TYPE__complex_double)
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag,
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real))),
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag))));
#else
error: type not defined.
#endif
                            }
                        }
                    } else {
#if defined(BLAS_TYPE__single)
                        if (xCMP_NE(*((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))), expected[ii][jj])) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%x instead of %f / 0x%x\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))),
                                    (double) *((BLAS_MATH_t *) &expected[ii][jj]),
                                    (u32) expected[ii][jj]);
                            }
                        }
#elif defined(BLAS_TYPE__double)
                        if (xCMP_NE(*((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))), expected[ii][jj])) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%lx instead of %f / 0x%lx\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]))),
                                    *((BLAS_MATH_t *) &expected[ii][jj]),
                                    (u64) expected[ii][jj]);
                            }
                        }
#elif defined(BLAS_TYPE__complex)
                        if (xCMP_NE(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)], (*(BLAS_MATH_t *) (&expected[ii][jj])))) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {%f, %f} / {0x%x, 0x%x}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    (double) pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag,
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real))),
                                    *((u32 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag))),
                                    (double) *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj][0]),
                                    (double) *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj][1]),
                                    (u32) expected[ii][jj][0],
                                    (u32) expected[ii][jj][1]);
                            }
                        }
#elif defined(BLAS_TYPE__complex_double)
                        if (xCMP_NE(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)], (*(BLAS_MATH_t *) (&expected[ii][jj])))) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} / {0x%lx, 0x%lx} instead of {%f, %f} / {0x%lx, 0x%lx}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag,
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real))),
                                    *((u64 *) (&(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag))),
                                    *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj][0]),
                                    *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj][1]),
                                    (u64) expected[ii][jj][0],
                                    (u64) expected[ii][jj][1]);
                            }
                        }
#else
error: type not defined.
#endif
                    }
                }
            }
        }
    }
    printf ("Error count = %ld\n", (u64) errCnt);
#endif

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#endif
    pPivotIdx_arr = wrapupParams->pivotIdx;
    printf("Pivot indices: %ld %ld %ld %ld ...\n", (u64) pPivotIdx_arr[0], (u64) pPivotIdx_arr[1], (u64) pPivotIdx_arr[2], (u64) pPivotIdx_arr[3]);
    for (i = 4; i < MIN(wrapupParams->n,wrapupParams->m); i++) {
        if (pPivotIdx_arr[i] != pPivotIdx_arr[i-4]+4) {
            printf ("ERROR!!!  pivotIdx[%ld] = %ld ", (u64) i, (u64) pPivotIdx_arr[i]);
            break;
        }
    }
    printf ("Info: %ld\n", (u64) *pInfo);

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(getrf_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the GETRF library function should NOT be pulling the plug!
    return NULL_GUID;
} // ?getrf_wrapupTask

