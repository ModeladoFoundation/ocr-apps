/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?GESV,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  This Level 3 BLAS function computes the solution to a system of linear equations A*X=B for general matrices.
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


typedef struct {                          // These are the scalar variables that are passed to the GESV thunking EDT as its paramv argument.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
} NAME(gesv_thunkParams_t);

typedef struct {                          // These are the dependencies that the GESV thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GESV status.
} NAME(gesv_thunkDeps_t);

static ocrGuid_t NAME(gesv_thunkTask) (   // Moves array-based input matrices (passed by standard GESV API) into OCR datablocks, then spawns GESV native OCR EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the GESV_STEP2 EDT as its paramv argument.
    BLAS_int_t    trans;                  // The value of the TRANS argument, i.e. 1 (true) corresponds to "T" in reference version, to transpose, 0 (false) corresponds to "N" to NOT transpose.  We want "N".
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number of columns of matrix B.
    BLAS_int_t    lda;                    // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;                    // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(gesv_Step2Params_t);

typedef struct {                          // These are the dependencies that the GESV_STEP2 task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GESV status.
    ocrEdtDep_t   optionalTrigger;        // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(gesv_Step2Deps_t);

ocrGuid_t NAME(gesv_Step2) (              // Spawned by gesf_task, this evaluates the "info" value out of getrf (the factorization code), and if it is zero, spawns getrs (the equation-solving code).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the GESV thunking EDT as its paramv argument.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
    BLAS_int_t  * pivotIdx;               // Address of output array of pivot indices.
    BLAS_int_t  * info;                   // Address of output scalar of info.
} NAME(gesv_wrapupParams_t);

typedef struct {                          // These are the dependencies that the GESV wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GESV status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when GESV native OCR EDT is done.
} NAME(gesv_wrapupDeps_t);

static ocrGuid_t NAME(gesv_wrapupTask) (  // Writes matrix, pivot indices, and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?GESV library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as vector of pivot indices and scalar status info.

void NAME(gesv) (                 // Compute solution to a system of linear equations A*X=B for general matrices.
    const BLAS_int_t    n,        // Input:  The number of linear equations, i.e. the order of matrix A.
    const BLAS_int_t    nrhs,     // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,        // Input:  The n*n coefficient matrix.
                                  // Output: The factors L and U from the factorization A+P*L*U.  The unit
                                  //         diagonal elements of L are not stored.
    const BLAS_int_t    lda,      // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_int_t  * pivotIdx, // Output: The pivot indices that define the permutation matrix P;  row i of
                                  //         the matrix was interchanged with row pivotIdx[i].  Note: contents
                                  //         of the array are one-based, owing to the Fortran history.  They
                                  //         must be adjusted (decremented) to index into C's zero-based matrix.
    const BLAS_MATH_t * b,        // Input:  right hand side n-by-nhrs matrix b.
                                  // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,      // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info)     // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                  //         > 0 first one-based index along diagonal of U that is exactly zero.
                                  //         The factorization has been performed, but the factor U is exactly
                                  //         singular so the solution could not be computed.
{
    printf("Entered Standard API %s function.\n", STRINGIFY(NAME(gesv))); fflush(stdout);

// Create the datablocks:

    ocrGuid_t  dba;           // Guid of datablock for matrix A.
    ocrGuid_t  dbb;           // Guid of datablock for matrix B.
    ocrGuid_t  dbPivotIdx;    // Guid of datablock for pivot index output.
    ocrGuid_t  dbInfo;        // Guid of datablock for info output.

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

    BLAS_int_t pitchOfMatA = (n    + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t pitchOfMatB = (nrhs + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatAInBytes = (n * pitchOfMatA) * sizeof(BLAS_MATH_t);
    BLAS_int_t sizeOfMatBInBytes = (n * pitchOfMatB) * sizeof(BLAS_MATH_t);
    ocrDbCreate (&dba, (void**) &addr, sizeOfMatAInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&dbb, (void**) &addr, sizeOfMatBInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    BLAS_int_t sizeOfPivotIdxArrayInBytes = n * sizeof(BLAS_int_t);
    ocrDbCreate (&dbPivotIdx, (void**) &addr, sizeOfPivotIdxArrayInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    ocrDbCreate (&dbInfo, (void**) &addr, sizeof(BLAS_int_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t gesvThunkTemplate;
    NAME(gesv_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&gesvThunkTemplate, NAME(gesv_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(gesv_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the GESV result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t gesvWrapupTemplate;
    NAME(gesv_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&gesvWrapupTemplate, NAME(gesv_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(gesv_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.n                        = n;
    thunkParams.nrhs                     = nrhs;
    thunkParams.lda_array                = lda;
    thunkParams.ldb_array                = ldb;
    thunkParams.lda_datablock            = pitchOfMatA;
    thunkParams.ldb_datablock            = pitchOfMatB;
    thunkParams.a                        = (BLAS_MATH_t *) a;
    thunkParams.b                        = (BLAS_MATH_t *) b;

    ocrGuid_t gesvThunkEdt;
    ocrGuid_t gesvThunkOutputEvent;
    ocrEdtCreate(&gesvThunkEdt, gesvThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gesvThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API GESV func.
    wrapupParams.n               = n;
    wrapupParams.nrhs            = nrhs;
    wrapupParams.lda_array       = lda;
    wrapupParams.ldb_array       = ldb;
    wrapupParams.lda_datablock   = pitchOfMatA;
    wrapupParams.ldb_datablock   = pitchOfMatB;
    wrapupParams.a               = (BLAS_MATH_t *) a;
    wrapupParams.b               = (BLAS_MATH_t *) b;
    wrapupParams.pivotIdx        = (BLAS_int_t  *) pivotIdx;
    wrapupParams.info            = (BLAS_int_t  *) info;

    ocrGuid_t gesvWrapupEdt;
    ocrEdtCreate(&gesvWrapupEdt, gesvWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                   gesvWrapupEdt, NAME(gesv_wrapupDeps_t), dba,                 RO);
    ADD_DEPENDENCE(dbb,                   gesvWrapupEdt, NAME(gesv_wrapupDeps_t), dbb,                 RO);
    ADD_DEPENDENCE(dbPivotIdx,            gesvWrapupEdt, NAME(gesv_wrapupDeps_t), dbPivotIdx,          RO);
    ADD_DEPENDENCE(dbInfo,                gesvWrapupEdt, NAME(gesv_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(gesvThunkOutputEvent,  gesvWrapupEdt, NAME(gesv_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                  gesvThunkEdt, NAME(gesv_thunkDeps_t), dba,        RW);
    ADD_DEPENDENCE(dbb,                  gesvThunkEdt, NAME(gesv_thunkDeps_t), dbb,        RW);
    ADD_DEPENDENCE(dbPivotIdx,           gesvThunkEdt, NAME(gesv_thunkDeps_t), dbPivotIdx, RW);
    ADD_DEPENDENCE(dbInfo,               gesvThunkEdt, NAME(gesv_thunkDeps_t), dbInfo,     RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GESV operation.\n", STRINGIFY(NAME(gesv))); fflush(stdout);

} // ?gesv


// Receiving array-based input matrices from the standard API for GESV, this EDT moves them into OCR datablocks, creates datablocks for pivot indices and info, then passes them to GESV native OCR EDT.
static ocrGuid_t NAME(gesv_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(gesv_thunkTask))); fflush(stdout);

    NAME(gesv_thunkParams_t) * thunkParams = (NAME(gesv_thunkParams_t) *) paramv;
    NAME(gesv_thunkDeps_t)   * thunkDeps   = (NAME(gesv_thunkDeps_t)   *) depv;

    u64 i, j;
    const BLAS_MATH_t * pMatA_arr = thunkParams->a;
    BLAS_MATH_t * pMatA_db  = thunkDeps->dba.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->n; j++) {
            pMatA_db[j] = pMatA_arr[j];
        }
        pMatA_db  += thunkParams->lda_datablock;
        pMatA_arr += thunkParams->lda_array;
    }

    const BLAS_MATH_t * pMatB_arr = thunkParams->b;
    BLAS_MATH_t * pMatB_db  = thunkDeps->dbb.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->nrhs; j++) {
            pMatB_db[j] = pMatB_arr[j];
        }
        pMatB_db  += thunkParams->ldb_datablock;
        pMatB_arr += thunkParams->ldb_array;
    }

    ocrGuid_t gesvTemplate;
    NAME(gesv_edtParams_t) NAME(gesv_edtParams);
    ocrEdtTemplateCreate(&gesvTemplate, NAME(gesv_task), sizeof(NAME(gesv_edtParams))/sizeof(u64), sizeof(NAME(gesv_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(gesv_edtParams).n              = thunkParams->n;
    NAME(gesv_edtParams).nrhs           = thunkParams->nrhs;
    NAME(gesv_edtParams).lda            = thunkParams->lda_datablock;
    NAME(gesv_edtParams).ldb            = thunkParams->ldb_datablock;

    ocrGuid_t gesvEdt;
    ocrEdtCreate(&gesvEdt, gesvTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(gesv_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dba.guid,        gesvEdt, NAME(gesv_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(thunkDeps->dbb.guid,        gesvEdt, NAME(gesv_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(thunkDeps->dbPivotIdx.guid, gesvEdt, NAME(gesv_edtDeps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid,     gesvEdt, NAME(gesv_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,                  gesvEdt, NAME(gesv_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(gesv_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?gesv_thunkTask


ocrGuid_t NAME(gesv_task) (           // Spawnable externally, or spawned by the thunk layer, this spawns ?GETRF to factor the matrix, then ?GETRS to solve the equations.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    printf ("Entered %s\n", STRINGIFY(NAME(gesv_task))); fflush(stdout);

    NAME(gesv_edtParams_t) * myParams = (NAME(gesv_edtParams_t) *) paramv;
    NAME(gesv_edtDeps_t)   * myDeps   = (NAME(gesv_edtDeps_t)   *) depv;

    BLAS_int_t   n              = myParams->n;
    BLAS_int_t   nrhs           = myParams->nrhs;
    BLAS_int_t   lda            = myParams->lda;
    BLAS_int_t   ldb            = myParams->ldb;

    ocrGuid_t     gMatA     = myDeps->dba.guid;
    ocrGuid_t     gMatB     = myDeps->dbb.guid;
    ocrGuid_t     gPivotIdx = myDeps->dbPivotIdx.guid;
    ocrGuid_t     gInfo     = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMatA     = (BLAS_MATH_t *) (myDeps->dba.ptr);
    BLAS_MATH_t * pMatB     = (BLAS_MATH_t *) (myDeps->dbb.ptr);
    BLAS_int_t  * pPivotIdx = (BLAS_int_t  *) (myDeps->dbPivotIdx.ptr);
    BLAS_int_t  * pInfo     = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    *pInfo = 0;
    // Check argument validity.
    if      (n    < 0) *pInfo = -1;
    else if (nrhs < 0) *pInfo = -2;
    else if (lda < MAX(n,1)) *pInfo = -4;
    else if (ldb < MAX(nrhs,1)) *pInfo = -7;
    if (*pInfo != 0) {
        xerbla (STRINGIFY(NAME(gesv)), -*pInfo);
        return NULL_GUID;
    }

    ocrGuid_t getrfTemplate;
    ocrEdtTemplateCreate(&getrfTemplate, NAME(getrf_task), sizeof(NAME(getrf_edtParams_t))/sizeof(u64), sizeof(NAME(getrf_edtDeps_t))/sizeof(ocrEdtDep_t));
    ocrGuid_t gesv_Step2Template;
    ocrEdtTemplateCreate(&gesv_Step2Template, NAME(gesv_Step2), sizeof(NAME(gesv_Step2Params_t))/sizeof(u64), sizeof(NAME(gesv_Step2Deps_t))/sizeof(ocrEdtDep_t));

    // Spawn GESV.
    NAME(getrf_edtParams_t) getrfParams;
    getrfParams.m     = n;     // The number of rows in the matrix A.
    getrfParams.n     = n;     // The number of columns in the matrix A.
    getrfParams.ld    = lda;   // The leading dimension of the matrix.

    ocrGuid_t getrfEdt;
    ocrGuid_t getrfOutputEvent;
    ocrEdtCreate(&getrfEdt, getrfTemplate, EDT_PARAM_DEF, (u64 *) (&getrfParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &getrfOutputEvent);

    // Spawn Step 2 of gesv, which looks at info out of getrf, and if zero, spawns getrs.
    NAME(gesv_Step2Params_t) gesv_Step2Params;
    gesv_Step2Params.trans = 0;     // The value of the TRANS argument, i.e. 1 (true) corresponds to "T" in reference version, to transpose, 0 (false) corresponds to "N" to NOT transpose.  We want "N".
    gesv_Step2Params.n     = n;     // The order of n-by-n matrix A and the number of rows in matrix B.
    gesv_Step2Params.nrhs  = nrhs;  // The number of columns in matrix B.
    gesv_Step2Params.lda   = lda;   // The leading dimension of matrix A.
    gesv_Step2Params.ldb   = ldb;   // The leading dimension of matrix B.

    ocrGuid_t gesv_Step2;
    ocrEdtCreate(&gesv_Step2, gesv_Step2Template, EDT_PARAM_DEF, (u64 *) (&gesv_Step2Params), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the spawned EDTs in reverse order.

    ADD_DEPENDENCE(gMatA,            gesv_Step2, NAME(gesv_Step2Deps_t), dba,             RO);
    ADD_DEPENDENCE(gMatB,            gesv_Step2, NAME(gesv_Step2Deps_t), dbb,             RO);
    ADD_DEPENDENCE(gPivotIdx,        gesv_Step2, NAME(gesv_Step2Deps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(gInfo,            gesv_Step2, NAME(gesv_Step2Deps_t), dbInfo,          RW);
    ADD_DEPENDENCE(getrfOutputEvent, gesv_Step2, NAME(gesv_Step2Deps_t), optionalTrigger, RO);

    ADD_DEPENDENCE(gMatA,            getrfEdt, NAME(getrf_edtDeps_t), dbMat,           RO);
    ADD_DEPENDENCE(gPivotIdx,        getrfEdt, NAME(getrf_edtDeps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(gInfo,            getrfEdt, NAME(getrf_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,        getrfEdt, NAME(getrf_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting.\n", STRINGIFY(NAME(gesv_task))); fflush(stdout);
    return NULL_GUID;
} // ?gesv_task


ocrGuid_t NAME(gesv_Step2) (              // Check the results of the factorization step, and if it was successful, create the solving step.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(gesv_Step2))); fflush(stdout);

    NAME(gesv_Step2Params_t) * myParams = (NAME(gesv_Step2Params_t) *) paramv;
    NAME(gesv_Step2Deps_t)   * myDeps   = (NAME(gesv_Step2Deps_t)   *) depv;

    if (*((BLAS_int_t *)(myDeps->dbInfo.ptr)) != 0) {
        printf ("        %s discovered that INFO out of %s is %ld, not zero, so %s is NOT being invoked!\n",
            STRINGIFY(NAME(gesv_Step2)), (u64) (*((BLAS_int_t *)(myDeps->dbInfo.ptr))), STRINGIFY(NAME(getrf_task)), STRINGIFY(NAME(getrs_task)));
    } else {
        printf ("        %s is spawning because INFO out of %s is zero\n", STRINGIFY(NAME(gesv_Step2)), STRINGIFY(NAME(getrs_task)), STRINGIFY(NAME(getrf_task)));
    }
    ocrGuid_t getrsTemplate;
    ocrEdtTemplateCreate(&getrsTemplate, NAME(getrs_task), sizeof(NAME(getrs_edtParams_t))/sizeof(u64), sizeof(NAME(getrs_edtDeps_t))/sizeof(ocrEdtDep_t));
    // Spawn GETRS.
    NAME(getrs_edtParams_t) getrsParams;
    getrsParams.trans = myParams->trans;
    getrsParams.n     = myParams->n;
    getrsParams.nrhs  = myParams->nrhs;
    getrsParams.lda   = myParams->lda;
    getrsParams.ldb   = myParams->ldb;

    ocrGuid_t getrsEdt;
    ocrEdtCreate(&getrsEdt, getrsTemplate, EDT_PARAM_DEF, (u64 *) (&getrsParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

    ADD_DEPENDENCE(myDeps->dba.guid,        getrsEdt, NAME(getrs_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(myDeps->dbb.guid,        getrsEdt, NAME(getrs_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(myDeps->dbPivotIdx.guid, getrsEdt, NAME(getrs_edtDeps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(myDeps->dbInfo.guid,     getrsEdt, NAME(getrs_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,               getrsEdt, NAME(getrs_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(gesv_Step2))); fflush(stdout);
    return NULL_GUID;
} // ?gesv_Step2

static ocrGuid_t NAME(gesv_wrapupTask) (  // This EDT gains control when ?gesv_task and all that is spawns are done.  It "dethunks" the output original storage passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("Entered %s\n", STRINGIFY(NAME(gesv_wrapupTask))); fflush(stdout);

    NAME(gesv_wrapupParams_t) * wrapupParams = (NAME(gesv_wrapupParams_t) *) paramv;
    NAME(gesv_wrapupDeps_t)   * wrapupDeps   = (NAME(gesv_wrapupDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pdba  = wrapupDeps->dba.ptr;
    BLAS_MATH_t * pa    = wrapupParams->a;
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
            pa[j] = pdba[j];
        }
        pdba += wrapupParams->lda_datablock;
        pa   += wrapupParams->lda_array;
    }

    BLAS_MATH_t * pdbb  = wrapupDeps->dbb.ptr;
    BLAS_MATH_t * pb    = wrapupParams->b;
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->nrhs; j++) {
            pb[j] = pdbb[j];
        }
        pdbb += wrapupParams->ldb_datablock;
        pb   += wrapupParams->ldb_array;
    }

    BLAS_int_t * pPivotIdx_db  = wrapupDeps->dbPivotIdx.ptr;
    BLAS_int_t * pPivotIdx_arr = wrapupParams->pivotIdx;
    for (i = wrapupParams->n; i > 0; i--) {
        *pPivotIdx_arr++ = *pPivotIdx_db++;
    }

    BLAS_int_t * pInfo_db = wrapupDeps->dbInfo.ptr;
    BLAS_int_t * pInfo    = wrapupParams->info;
    *pInfo = *pInfo_db;

    ocrDbDestroy (wrapupDeps->dba.guid);         // Destroy datablock for the matrix A.
    ocrDbDestroy (wrapupDeps->dbb.guid);         // Destroy datablock for the matrix B.
    ocrDbDestroy (wrapupDeps->dbPivotIdx.guid);  // Destroy datablock for the pivot indices.
    ocrDbDestroy (wrapupDeps->dbInfo.guid);      // Destroy datablock for the diagnostic info.

    pa = wrapupParams->a;
    pb = wrapupParams->b;
#if 0
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            printf ("%f ", pa[j]);
#else
            printf ("(%f,%f) ", pa[j].real, pa[j].imag);
#endif
        }
        printf("    ");
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        printf ("%f ", pb[0]);
        printf ("%f ", pb[1]);
#else
        printf ("(%f,%f) ", pb[0].real, pb[0].imag);
        printf ("(%f,%f) ", pb[1].real, pb[1].imag);
#endif
        pa += wrapupParams->lda_array;
        pb += wrapupParams->ldb_array;
        printf ("\n");
    }

    pa = wrapupParams->a;
    pb = wrapupParams->b;
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
#if defined(BLAS_TYPE__single)
            printf ("0x%x ", *((u32*) (&(pa[j]))));
#elif defined(BLAS_TYPE__double)
            printf ("0x%lx ", *((u64*) (&(pa[j]))));
#elif defined(BLAS_TYPE__complex)
            printf ("(0x%x,0x%x) ", *((u32*) (&(pa[j].real))), *((u32*) (&(pa[j].imag))));
#elif defined(BLAS_TYPE__complex_double)
            printf ("(0x%lx,0x%lx) ", *((u64*) (&(pa[j].real))), *((u64*) (&(pa[j].imag))));
#else
****ERROR
#endif
        }
        printf("    ");
#if defined(BLAS_TYPE__single)
        printf ("0x%x ", *((u32*) (&(pb[0]))));
        printf ("0x%x ", *((u32*) (&(pb[1]))));
#elif defined(BLAS_TYPE__double)
        printf ("0x%lx ", *((u64*) (&(pb[0]))));
        printf ("0x%lx ", *((u64*) (&(pb[1]))));
#elif defined(BLAS_TYPE__complex)
        printf ("(0x%x,0x%x) ", *((u32*) (&(pb[0].real))), *((u32*) (&(pb[0].imag))));
        printf ("(0x%x,0x%x) ", *((u32*) (&(pb[1].real))), *((u32*) (&(pb[1].imag))));
#elif defined(BLAS_TYPE__complex_double)
        printf ("(0x%lx,0x%lx) ", *((u64*) (&(pb[0].real))), *((u64*) (&(pb[0].imag))));
        printf ("(0x%lx,0x%lx) ", *((u64*) (&(pb[1].real))), *((u64*) (&(pb[1].imag))));
#else
****ERROR
#endif

        pa += wrapupParams->lda_array;
        pb += wrapupParams->ldb_array;
        printf ("\n");
    }
#else
#if defined(BLAS_TYPE__single)
    u32 templateA[4][4] = {{0x40a80000, 0xc03ccccd, 0xbf733333, 0xc0733333},
                           {0x3eaf8af8, 0x40790d2a, 0x40180bb4, 0x3ed36200},
                           {0x3e9a166e, 0xbeed1dcd, 0xbfc1c623, 0x3e96f2ba},
                           {0xbe5880bb, 0xbea8e65d, 0x3b9ac5ea, 0x3e0686b8}};
    u32 templateB[4][2] = {{0x3f80001b, 0x403fffeb},
                           {0xbf800008, 0x40000006},
                           {0x40400004, 0x407ffffb},
                           {0xc09ffff6, 0x3f7fff81}};


#elif defined(BLAS_TYPE__double)
    u64 templateA[4][4] = {{0x4015000000000000L, 0xc00799999999999aL, 0xbfee666666666666L, 0xc00e666666666666L},
                           {0x3fd5f15f15f15f16L, 0x400f21a54d880bb4L, 0x400301767dce434aL, 0x3fda6c405d9f738eL},
                           {0x3fd342cdc67600faL, 0xbfdda3b98727abfdL, 0xbff838c480f5769aL, 0x3fd2de57404287c6L},
                           {0xbfcb101767dce435L, 0xbfd51ccb98d9214fL, 0x3f7358cf5a4cfad6L, 0x3fc0d0d69b95bf3eL}};
    u64 templateB[4][2] = {{0x3ff000000000000cL, 0x4008000000000000L},
                           {0xbff000000000000aL, 0x3ffffffffffffffeL},
                           {0x4008000000000006L, 0x4010000000000001L},
                           {0xc013fffffffffffbL, 0x3ff0000000000000L}};

#elif defined(BLAS_TYPE__complex)
    u32 templateA[4][4][2] = {{{0xc0528f5c, 0xc018f5c3}, {0xbff47ae1, 0x408d70a4}, {0xbe0f5c29, 0xbfaccccd}, {0x3fdc28f6, 0x3faccccd}},
                              {{0x3e735091, 0x3e830d24}, {0x409ca552, 0xbf361bd6}, {0xbeecaff1, 0x3fd92a89}, {0x3f9d0983, 0x3f1e7507}},
                              {{0xbdd0cc41, 0xbf33759f}, {0xbf2b4d64, 0x3ebcdc83}, {0xc0a48670, 0xbf90a2d9}, {0x3f7f8dd5, 0x3ec520b7}},
                              {{0xbf092dc6, 0x3e8a9cc4}, {0xbe50eb14, 0x3f5c30b3}, {0x3c06e3ec, 0x3df7f041}, {0x3e17cc08, 0xbe003ab4}}};
    u32 templateB[4][2][2] = {{{0x3f7fffe8, 0x3f7ffff5}, {0xbf800033, 0xbfffffe7}},
                              {{0x3fffffff, 0xc03ffffd}, {0x40a00003,0x3f80000c}},
                              {{0xc0800001, 0xc0a00000}, {0xc0400006,0x407fffff}},
                              {{0xb5f22b15, 0x40bffffe}, {0x3fffffaf,0xc03ffffa}}};

#elif defined(BLAS_TYPE__complex_double)
    u64 templateA[4][4][2] = {{{0xc00a51eb851eb852L, 0xc0031eb851eb851fL}, {0xbffe8f5c28f5c28fL, 0x4011ae147ae147aeL}, {0xbfc1eb851eb851ecL, 0xbff599999999999aL}, {0x3ffb851eb851eb85L, 0x3ff599999999999aL}},
                              {{0x3fce6a12266fcaf1L, 0x3fd061a497c70dd3L}, {0x401394aa3b6c38a7L, 0xbfe6c37ab5af5abaL}, {0xbfdd95fe35baa84cL, 0x3ffb255124c226e4L}, {0x3ff3a13072a41af4L, 0x3fe3cea0cdf05d0eL}},
                              {{0xbfba1988152da2aaL, 0xbfe66eb3ef4aee9aL}, {0xbfe569ac8334ca2aL, 0x3fd79b904eb561cfL}, {0xc01490ce05c8731eL, 0xbff2145b24fd6addL}, {0x3feff1babe518fd6L, 0x3fd8a416e8b9c331L}},
                              {{0xbfe125b8b1934db2L, 0x3fd15398665fbdeaL}, {0xbfca1d62bb393332L, 0x3feb861653980f2bL}, {0x3f80dc7cdf16a244L, 0x3fbefe078ae28cdbL}, {0x3fc2f9806159e3d7L, 0xbfc007570958d290L}}};
    u64 templateB[4][2][2] = {{{0x3ff000000000001aL, 0x3ff000000000000cL}, {0xbff0000000000044L, 0xbfffffffffffffa6L}},
                              {{0x3ffffffffffffffeL, 0xc008000000000005L}, {0x401400000000000aL, 0x3ff000000000000eL}},
                              {{0xc010000000000000L, 0xc013ffffffffffffL}, {0xc00800000000000eL, 0x4010000000000003L}},
                              {{0x3cf9ac7787fe78b3L, 0x4018000000000008L}, {0x3fffffffffffff81L, 0xc007ffffffffffdcL}}};

#else
error: type not defined.
#endif
    u64 ii, jj;
    u64 errCnt = 0;
    BLAS_MATH_t zero;
    xSETc(zero, 0.0, 0.0);
    for (i = 0; i < wrapupParams->n; i+=4) {
        for (j = 0; j < wrapupParams->n; j+=4) {
            for (ii = 0; ii < 4; ii++) {
                for (jj = 0; jj < 4; jj++) {
                    if (i != j) {
                        if (xCMP_NE(pa[(i+ii)*wrapupParams->lda_array+(j+jj)], zero)) {
                            if (errCnt++ < 100) {
#if defined(BLAS_TYPE__single)
                                printf ("a[%ld][%ld] = %f / 0x%x instead of 0.0 or -0.0\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)],
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))));
#elif defined(BLAS_TYPE__double)
                                printf ("a[%ld][%ld] = %f / 0x%lx instead of 0.0 or -0.0\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)],
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))));
#elif defined(BLAS_TYPE__complex)
                                printf ("a[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real,
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag,
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real))),
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag))));
#elif defined(BLAS_TYPE__complex_double)
                                printf ("a[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real,
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag,
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real))),
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag))));
#else
error: type not defined.
#endif
                            }
                        }
                    } else {
#if defined(BLAS_TYPE__single)
                        if (xCMP_NE(*((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))), templateA[ii][jj])) {
                            if (errCnt++ < 100) {
                                printf ("a[%ld][%ld] = %f / 0x%x instead of %f / 0x%x\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pa[(i+ii)*wrapupParams->lda_array+(j+jj)],
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))),
                                    (double) *((BLAS_MATH_t *) &templateA[ii][jj]),
                                    (u32) templateA[ii][jj]);
                            }
                        }
#elif defined(BLAS_TYPE__double)
                        if (xCMP_NE(*((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))), templateA[ii][jj])) {
                            if (errCnt++ < 100) {
                                printf ("a[%ld][%ld] = %f / 0x%lx instead of %f / 0x%lx\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)],
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)]))),
                                    *((BLAS_MATH_t *) &templateA[ii][jj]),
                                    (u64) templateA[ii][jj]);
                            }
                        }
#elif defined(BLAS_TYPE__complex)
                        if (xCMP_NE(pa[(i+ii)*wrapupParams->lda_array+(j+jj)], (*(BLAS_MATH_t *) (&templateA[ii][jj])))) {
                            if (errCnt++ < 100) {
                                printf ("a[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {%f, %f} / {0x%x, 0x%x}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real,
                                    (double) pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag,
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real))),
                                    *((u32 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag))),
                                    (double) *((BLAS_MATH_COMPONENT_t *) &templateA[ii][jj][0]),
                                    (double) *((BLAS_MATH_COMPONENT_t *) &templateA[ii][jj][1]),
                                    (u32) templateA[ii][jj][0],
                                    (u32) templateA[ii][jj][1]);
                            }
                        }
#elif defined(BLAS_TYPE__complex_double)
                        if (xCMP_NE(pa[(i+ii)*wrapupParams->lda_array+(j+jj)], (*(BLAS_MATH_t *) (&templateA[ii][jj])))) {
                            if (errCnt++ < 100) {
                                printf ("a[%ld][%ld] = {%f, %f} / {0x%lx, 0x%lx} instead of {%f, %f} / {0x%lx, 0x%lx}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real,
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag,
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real))),
                                    *((u64 *) (&(pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag))),
                                    *((BLAS_MATH_COMPONENT_t *) &templateA[ii][jj][0]),
                                    *((BLAS_MATH_COMPONENT_t *) &templateA[ii][jj][1]),
                                    (u64) templateA[ii][jj][0],
                                    (u64) templateA[ii][jj][1]);
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
    printf ("Error count for Matrix A = %ld\n", (u64) errCnt);
    errCnt = 0;
    for (i = 0; i < wrapupParams->n; i+=4) {
        for (ii = 0; ii < 4; ii++) {
            for (j = 0; j < wrapupParams->nrhs; j++) {
#if defined(BLAS_TYPE__single)
                if (xCMP_NE(*((u32 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j]))), templateB[ii][j])) {
                    if (errCnt++ < 100) {
                        printf ("b[%ld][%ld] = %f / 0x%x instead of %f / 0x%x\n",
                            (u64) (i+ii), (u64) (j),
                            (double) pb[(i+ii)*wrapupParams->ldb_array+j],
                            *((u32 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j]))),
                            (double) *((BLAS_MATH_t *) &templateA[ii][j]),
                            (u32) templateA[ii][j]);
                    }
                }
#elif defined(BLAS_TYPE__double)
                if (xCMP_NE(*((u64 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j]))), templateB[ii][j])) {
                    if (errCnt++ < 100) {
                        printf ("b[%ld][%ld] = %f / 0x%lx instead of %f / 0x%lx\n",
                            (u64) (i+ii), (u64) (j),
                            pb[(i+ii)*wrapupParams->ldb_array+j],
                            *((u64 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j]))),
                            *((BLAS_MATH_t *) &templateA[ii][j]),
                            (u64) templateA[ii][j]);
                    }
                }
#elif defined(BLAS_TYPE__complex)
                if (xCMP_NE(pb[(i+ii)*wrapupParams->ldb_array+j], (*(BLAS_MATH_t *) (&templateB[ii][j])))) {
                    if (errCnt++ < 100) {
                        printf ("b[%ld][%ld] = {%f, %f} / {0x%x, 0x%x} instead of {%f, %f} / {0x%x, 0x%x}\n",
                            (u64) (i+ii), (u64) (j),
                            (double) pb[(i+ii)*wrapupParams->ldb_array+j].real,
                            (double) pb[(i+ii)*wrapupParams->ldb_array+j].imag,
                            *((u32 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j].real))),
                            *((u32 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j].imag))),
                            (double) *((BLAS_MATH_COMPONENT_t *) &templateA[ii][j][0]),
                            (double) *((BLAS_MATH_COMPONENT_t *) &templateA[ii][j][1]),
                            (u32) templateA[ii][j][0],
                            (u32) templateA[ii][j][1]);
                    }
                }
#elif defined(BLAS_TYPE__complex_double)
                if (xCMP_NE(pb[(i+ii)*wrapupParams->ldb_array+j], (*(BLAS_MATH_t *) (&templateB[ii][j])))) {
                    if (errCnt++ < 100) {
                        printf ("b[%ld][%ld] = {%f, %f} / {0x%lx, 0x%lx} instead of {%f, %f} / {0x%lx, 0x%lx}\n",
                            (u64) (i+ii), (u64) (j),
                            pb[(i+ii)*wrapupParams->ldb_array+j].real,
                            pb[(i+ii)*wrapupParams->ldb_array+j].imag,
                            *((u64 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j].real))),
                            *((u64 *) (&(pb[(i+ii)*wrapupParams->ldb_array+j].imag))),
                            *((BLAS_MATH_COMPONENT_t *) &templateA[ii][j][0]),
                            *((BLAS_MATH_COMPONENT_t *) &templateA[ii][j][1]),
                            (u64) templateA[ii][j][0],
                            (u64) templateA[ii][j][1]);
                    }
                }
#else
error: type not defined.
#endif
            }
        }
    }
    printf ("Error count for Matrix B = %ld\n", (u64) errCnt);
#endif

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#endif
    pPivotIdx_arr = wrapupParams->pivotIdx;
    printf("Pivot indices: %ld %ld %ld %ld ...\n", (u64) pPivotIdx_arr[0], (u64) pPivotIdx_arr[1], (u64) pPivotIdx_arr[2], (u64) pPivotIdx_arr[3]);
    for (i = 4; i < wrapupParams->n; i++) {
        if (pPivotIdx_arr[i] != pPivotIdx_arr[i-4]+4) {
            printf ("ERROR!!!  pivotIdx[%ld] = %ld ", (u64) i, (u64) pPivotIdx_arr[i]);
            break;
        }
    }
    printf ("Info: %ld\n", (u64) *pInfo);

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(gesv_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the GESV library function should NOT be pulling the plug!
    return NULL_GUID;
} // gesvWrapupTask
