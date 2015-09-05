/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?POSV,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  This Level 3 BLAS function computes the solution to a system of linear equations A*X=B for positive definite matrices.
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


typedef struct {                          // These are the scalar variables that are passed to the POSV thunking EDT as its paramv argument.
    const char  * uplo;                   // "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    BLAS_int_t    n;                      // The number of linear equations, i.e. the order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
} NAME(posv_thunkParams_t);

typedef struct {                          // These are the dependencies that the POSV thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POSV status.
} NAME(posv_thunkDeps_t);

static ocrGuid_t NAME(posv_thunkTask) (   // Moves array-based input matrices (passed by standard POSV API) into OCR datablocks, then spawns POSV native OCR EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the POSV_STEP2 EDT as its paramv argument.
    const char  * uplo;                   // "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number of columns of matrix B.
    BLAS_int_t    lda;                    // The leading dimension of matrix A (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb;                    // The leading dimension of matrix B (in elements of BLAS_MATH_t).
} NAME(posv_Step2Params_t);

typedef struct {                          // These are the dependencies that the POSV_STEP2 task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POSV status.
    ocrEdtDep_t   optionalTrigger;        // Optional additional trigger event.  Satisfy with NULL_GUID when not needed.
} NAME(posv_Step2Deps_t);

ocrGuid_t NAME(posv_Step2) (              // Spawned by posf_task, this evaluates the "info" value out of potrf (the factorization code), and if it is zero, spawns potrs (the equation-solving code).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the POSV thunking EDT as its paramv argument.
    const char  * uplo;                   // "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
    BLAS_int_t  * info;                   // Address of output scalar of info.
} NAME(posv_wrapupParams_t);

typedef struct {                          // These are the dependencies that the POSV wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POSV status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when POSV native OCR EDT is done.
} NAME(posv_wrapupDeps_t);

static ocrGuid_t NAME(posv_wrapupTask) (  // Writes matrix and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?POSV library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as scalar status info.

void NAME(posv) (                 // Compute solution to a system of linear equations A*X=B for positive definite matrices.
    const char        * uplo,     // Input:  "U" or "u" -- upper triangle of A is stored; "L" or "l" -- lower triangle.
    const BLAS_int_t    n,        // Input:  The number of linear equations, i.e. the order of matrix A.
    const BLAS_int_t    nrhs,     // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,        // Input:  The n*n coefficient matrix.  Either the upper or lower triangle, as guided by uplo, is referenced
                                  // Output: If info==0, the factors L and U from the factorization A=U**T*U, A=U**H*U, A=L*L**T, or A=L*L**H.
    const BLAS_int_t    lda,      // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_MATH_t * b,        // Input:  right hand side n-by-nhrs matrix b.
                                  // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,      // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info)     // Output: 0 == success; < 0 negation of which argument had an illegal value;
                                  //         > 0 One-based index i, the leading minor of order i of A is not
                                  //         positive definite, so the factorization could not be completed and
                                  //         the solution has not been computed.
{
    printf("Entered Standard API %s function.\n", STRINGIFY(NAME(posv))); fflush(stdout);

// Create the datablocks:

    ocrGuid_t  dba;           // Guid of datablock for matrix A.
    ocrGuid_t  dbb;           // Guid of datablock for matrix B.
    ocrGuid_t  dbInfo;        // Guid of datablock for info output.

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

    BLAS_int_t pitchOfMatA = (n    + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t pitchOfMatB = (nrhs + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatAInBytes = (n * pitchOfMatA) * sizeof(BLAS_MATH_t);
    BLAS_int_t sizeOfMatBInBytes = (n * pitchOfMatB) * sizeof(BLAS_MATH_t);
    ocrDbCreate (&dba, (void**) &addr, sizeOfMatAInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&dbb, (void**) &addr, sizeOfMatBInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    ocrDbCreate (&dbInfo, (void**) &addr, sizeof(BLAS_int_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t posvThunkTemplate;
    NAME(posv_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&posvThunkTemplate, NAME(posv_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(posv_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the POSV result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t posvWrapupTemplate;
    NAME(posv_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&posvWrapupTemplate, NAME(posv_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(posv_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.

    thunkParams.uplo                     = uplo;
    thunkParams.n                        = n;
    thunkParams.nrhs                     = nrhs;
    thunkParams.lda_array                = lda;
    thunkParams.ldb_array                = ldb;
    thunkParams.lda_datablock            = pitchOfMatA;
    thunkParams.ldb_datablock            = pitchOfMatB;
    thunkParams.a                        = (BLAS_MATH_t *) a;
    thunkParams.b                        = (BLAS_MATH_t *) b;

    ocrGuid_t posvThunkEdt;
    ocrGuid_t posvThunkOutputEvent;
    ocrEdtCreate(&posvThunkEdt, posvThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &posvThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API GESV func.
    wrapupParams.uplo            = uplo;
    wrapupParams.n               = n;
    wrapupParams.nrhs            = nrhs;
    wrapupParams.lda_array       = lda;
    wrapupParams.ldb_array       = ldb;
    wrapupParams.lda_datablock   = pitchOfMatA;
    wrapupParams.ldb_datablock   = pitchOfMatB;
    wrapupParams.a               = (BLAS_MATH_t *) a;
    wrapupParams.b               = (BLAS_MATH_t *) b;
    wrapupParams.info            = (BLAS_int_t  *) info;

    ocrGuid_t posvWrapupEdt;
    ocrEdtCreate(&posvWrapupEdt, posvWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                  posvWrapupEdt, NAME(posv_wrapupDeps_t), dba,                 RO);
    ADD_DEPENDENCE(dbb,                  posvWrapupEdt, NAME(posv_wrapupDeps_t), dbb,                 RO);
    ADD_DEPENDENCE(dbInfo,               posvWrapupEdt, NAME(posv_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(posvThunkOutputEvent, posvWrapupEdt, NAME(posv_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                  posvThunkEdt,  NAME(posv_thunkDeps_t),  dba,    RW);
    ADD_DEPENDENCE(dbb,                  posvThunkEdt,  NAME(posv_thunkDeps_t),  dbb,    RW);
    ADD_DEPENDENCE(dbInfo,               posvThunkEdt,  NAME(posv_thunkDeps_t),  dbInfo, RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GESV operation.\n", STRINGIFY(NAME(posv))); fflush(stdout);

} // ?posv


// Receiving array-based input matrices from the standard API for POSV, this EDT moves them into OCR datablocks, then passes them native OCR EDT.
static ocrGuid_t NAME(posv_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(posv_thunkTask))); fflush(stdout);

    NAME(posv_thunkParams_t) * thunkParams = (NAME(posv_thunkParams_t) *) paramv;
    NAME(posv_thunkDeps_t)   * thunkDeps   = (NAME(posv_thunkDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pMatA_arr = thunkParams->a;
    BLAS_MATH_t * pMatA_db  = thunkDeps->dba.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->n; j++) {
            pMatA_db[j] = pMatA_arr[j];
        }
        pMatA_db  += thunkParams->lda_datablock;
        pMatA_arr += thunkParams->lda_array;
    }

    BLAS_MATH_t * pMatB_arr = thunkParams->b;
    BLAS_MATH_t * pMatB_db  = thunkDeps->dbb.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->nrhs; j++) {
            pMatB_db[j] = pMatB_arr[j];
        }
        pMatB_db  += thunkParams->ldb_datablock;
        pMatB_arr += thunkParams->ldb_array;
    }

    ocrGuid_t posvTemplate;
    NAME(posv_edtParams_t) NAME(posv_edtParams);
    ocrEdtTemplateCreate(&posvTemplate, NAME(posv_task), sizeof(NAME(posv_edtParams))/sizeof(u64), sizeof(NAME(posv_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(posv_edtParams).uplo           = thunkParams->uplo;
    NAME(posv_edtParams).n              = thunkParams->n;
    NAME(posv_edtParams).nrhs           = thunkParams->nrhs;
    NAME(posv_edtParams).lda            = thunkParams->lda_datablock;
    NAME(posv_edtParams).ldb            = thunkParams->ldb_datablock;

    ocrGuid_t posvEdt;
    ocrEdtCreate(&posvEdt, posvTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(posv_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dba.guid,    posvEdt, NAME(posv_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(thunkDeps->dbb.guid,    posvEdt, NAME(posv_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid, posvEdt, NAME(posv_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,              posvEdt, NAME(posv_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(posv_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?posv_thunkTask


ocrGuid_t NAME(posv_task) (           // Spawnable externally, or spawned by the thunk layer, this spawns ?POTRF to factor the matrix, then ?POTRS to solve the equations.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    printf ("Entered %s\n", STRINGIFY(NAME(posv_task))); fflush(stdout);

    NAME(posv_edtParams_t) * myParams = (NAME(posv_edtParams_t) *) paramv;
    NAME(posv_edtDeps_t)   * myDeps   = (NAME(posv_edtDeps_t)   *) depv;

    char  * uplo  = (char *) myParams->uplo;
    BLAS_int_t    n     = myParams->n;
    BLAS_int_t    nrhs  = myParams->nrhs;
    BLAS_int_t    lda   = myParams->lda;
    BLAS_int_t    ldb   = myParams->ldb;
printf("uplo=%s n=%ld, nrhs=%ld, lda=%ld, ldb=%ld\n", uplo, (u64) n, (u64) nrhs, (u64) lda, (u64) ldb);

    ocrGuid_t     gMatA = myDeps->dba.guid;
    ocrGuid_t     gMatB = myDeps->dbb.guid;
    ocrGuid_t     gInfo = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMatA = (BLAS_MATH_t *) (myDeps->dba.ptr);
    BLAS_MATH_t * pMatB = (BLAS_MATH_t *) (myDeps->dbb.ptr);
    BLAS_int_t  * pInfo = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    // Check argument validity.
    if (uplo[0] != 'U' && uplo[0] != 'u' && uplo[0] != 'L' && uplo[0] != 'l') {
        *pInfo = -1;
        xerbla (STRINGIFY(NAME(posv)), -*pInfo);
        return NULL_GUID;
    }
    if (n    < 0) {
        *pInfo = -2;
        xerbla (STRINGIFY(NAME(posv)), -*pInfo);
        return NULL_GUID;
    }
    if (nrhs < 0) {
        *pInfo = -3;
        xerbla (STRINGIFY(NAME(posv)), -*pInfo);
        return NULL_GUID;
    }
    if (lda < MAX(n,1)) {
        *pInfo = -5;
        xerbla (STRINGIFY(NAME(posv)), -*pInfo);
        return NULL_GUID;
    }
    if (ldb < MAX(nrhs,1)) {
        *pInfo = -7;
        xerbla (STRINGIFY(NAME(posv)), -*pInfo);
        return NULL_GUID;
    }
    *pInfo = 0;

    ocrGuid_t potrfTemplate;
    ocrEdtTemplateCreate(&potrfTemplate, NAME(potrf_task), sizeof(NAME(potrf_edtParams_t))/sizeof(u64), sizeof(NAME(potrf_edtDeps_t))/sizeof(ocrEdtDep_t));
    ocrGuid_t posv_Step2Template;
    ocrEdtTemplateCreate(&posv_Step2Template, NAME(posv_Step2), sizeof(NAME(posv_Step2Params_t))/sizeof(u64), sizeof(NAME(posv_Step2Deps_t))/sizeof(ocrEdtDep_t));

    // Spawn POSV.
    NAME(potrf_edtParams_t) potrfParams;
    potrfParams.uplo  = uplo;  // Whether using the upper or lower triangle of A.
    potrfParams.n     = n;     // The order of matrix A.
    potrfParams.ld    = lda;   // The leading dimension of the matrix.

    ocrGuid_t potrfEdt;
    ocrGuid_t potrfOutputEvent;
    ocrEdtCreate(&potrfEdt, potrfTemplate, EDT_PARAM_DEF, (u64 *) (&potrfParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &potrfOutputEvent);

    // Spawn Step 2 of posv, which looks at info out of potrf, and if zero, spawns potrs.
    NAME(posv_Step2Params_t) posv_Step2Params;
    posv_Step2Params.uplo  = uplo;  // Whether using the upper or lower triangle of A.
    posv_Step2Params.n     = n;     // The order of n-by-n matrix A and the number of rows in matrix B.
    posv_Step2Params.nrhs  = nrhs;  // The number of columns in matrix B.
    posv_Step2Params.lda   = lda;   // The leading dimension of matrix A.
    posv_Step2Params.ldb   = ldb;   // The leading dimension of matrix B.

    ocrGuid_t posv_Step2;
    ocrEdtCreate(&posv_Step2, posv_Step2Template, EDT_PARAM_DEF, (u64 *) (&posv_Step2Params), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the spawned EDTs in reverse order.

    ADD_DEPENDENCE(gMatA,            posv_Step2, NAME(posv_Step2Deps_t), dba,             RO);
    ADD_DEPENDENCE(gMatB,            posv_Step2, NAME(posv_Step2Deps_t), dbb,             RO);
    ADD_DEPENDENCE(gInfo,            posv_Step2, NAME(posv_Step2Deps_t), dbInfo,          RW);
    ADD_DEPENDENCE(potrfOutputEvent, posv_Step2, NAME(posv_Step2Deps_t), optionalTrigger, RO);

    ADD_DEPENDENCE(gMatA,            potrfEdt, NAME(potrf_edtDeps_t), dbMat,           RO);
    ADD_DEPENDENCE(gInfo,            potrfEdt, NAME(potrf_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,        potrfEdt, NAME(potrf_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting.\n", STRINGIFY(NAME(posv_task))); fflush(stdout);
    return NULL_GUID;
} // ?posv_task


ocrGuid_t NAME(posv_Step2) (              // Check the results of the factorization step, and if it was successful, create the solving step.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(posv_Step2))); fflush(stdout);

    NAME(posv_Step2Params_t) * myParams = (NAME(posv_Step2Params_t) *) paramv;
    NAME(posv_Step2Deps_t)   * myDeps   = (NAME(posv_Step2Deps_t)   *) depv;

    if (*((BLAS_int_t *)(myDeps->dbInfo.ptr)) != 0) {
        printf ("        %s discovered that INFO out of %s is %ld, not zero, so %s is NOT being invoked!\n",
            STRINGIFY(NAME(posv_Step2)), (u64) (*((BLAS_int_t *)(myDeps->dbInfo.ptr))), STRINGIFY(NAME(potrf_task)), STRINGIFY(NAME(potrs_task)));
    } else {
        printf ("        %s is spawning because INFO out of %s is zero\n", STRINGIFY(NAME(posv_Step2)), STRINGIFY(NAME(potrs_task)), STRINGIFY(NAME(potrf_task)));
    }
    ocrGuid_t potrsTemplate;
    ocrEdtTemplateCreate(&potrsTemplate, NAME(potrs_task), sizeof(NAME(potrs_edtParams_t))/sizeof(u64), sizeof(NAME(potrs_edtDeps_t))/sizeof(ocrEdtDep_t));
    // Spawn GETRS.
    NAME(potrs_edtParams_t) potrsParams;
    potrsParams.uplo = myParams->uplo;
    potrsParams.n    = myParams->n;
    potrsParams.nrhs = myParams->nrhs;
    potrsParams.lda  = myParams->lda;
    potrsParams.ldb  = myParams->ldb;

    ocrGuid_t potrsEdt;
    ocrEdtCreate(&potrsEdt, potrsTemplate, EDT_PARAM_DEF, (u64 *) (&potrsParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

    ADD_DEPENDENCE(myDeps->dba.guid,    potrsEdt, NAME(potrs_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(myDeps->dbb.guid,    potrsEdt, NAME(potrs_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(myDeps->dbInfo.guid, potrsEdt, NAME(potrs_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,           potrsEdt, NAME(potrs_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(posv_Step2))); fflush(stdout);
    return NULL_GUID;
} // ?posv_Step2

static ocrGuid_t NAME(posv_wrapupTask) (  // This EDT gains control when ?posv_task and all that is spawns are done.  It "dethunks" the output original storage passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("Entered %s\n", STRINGIFY(NAME(posv_wrapupTask))); fflush(stdout);

    NAME(posv_wrapupParams_t) * wrapupParams = (NAME(posv_wrapupParams_t) *) paramv;
    NAME(posv_wrapupDeps_t)   * wrapupDeps   = (NAME(posv_wrapupDeps_t)   *) depv;

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

    BLAS_int_t * pInfo_db = wrapupDeps->dbInfo.ptr;
    BLAS_int_t * pInfo    = wrapupParams->info;
    *pInfo = *pInfo_db;

    ocrDbDestroy (wrapupDeps->dba.guid);         // Destroy datablock for the matrix A.
    ocrDbDestroy (wrapupDeps->dbb.guid);         // Destroy datablock for the matrix B.
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

#if 0
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
#endif
#else
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
    BLAS_MATH_t templateA_upper[9][9] = {{  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0, 99.0,  1.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0, 99.0, 99.0,  1.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0, 99.0, 99.0, 99.0,  1.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0,  1.0,  1.0},
                                         { 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0,  1.0}};
    BLAS_MATH_t templateA_lower[9][9] = {{  1.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0, 99.0, 99.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0,  1.0, 99.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, 99.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, 99.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, 99.0},
                                         {  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0}};
    BLAS_MATH_t templateB_upper[9][2] = {{  1.0,  1.0},
                                         {  1.0,  2.0},
                                         {  1.0,  3.0},
                                         {  1.0,  4.0},
                                         {  1.0,  5.0},
                                         {  1.0,  6.0},
                                         {  1.0,  7.0},
                                         {  1.0,  8.0},
                                         {  1.0,  9.0}};
    BLAS_MATH_t templateB_lower[9][2] = {{  1.0,  1.0},
                                         {  1.0,  2.0},
                                         {  1.0,  3.0},
                                         {  1.0,  4.0},
                                         {  1.0,  5.0},
                                         {  1.0,  6.0},
                                         {  1.0,  7.0},
                                         {  1.0,  8.0},
                                         {  1.0,  9.0}};
#define templateLen 9
#else
    BLAS_MATH_t templateA_upper[3][3] = {{{ 3.000000, 0.000000},{ 1.000000, 1.000000},{ 1.000000,-1.000000}},
                                         {{-1.100000, 2.200000},{ 4.000000, 0.000000},{ 2.000000,-1.000000}},
                                         {{ 3.300000,-4.400000},{-5.500000,-6.600000},{ 6.000000, 0.000000}}};
    BLAS_MATH_t templateB_upper[3][2] = {{{ 2.000000,-1.000000},{ 2.000000, 0.000000}},
                                         {{ 1.000000,-1.000000},{ 0.000000, 1.000000}},
                                         {{ 3.000000, 0.000000},{ 1.000000,-1.000000}}};
    BLAS_MATH_t templateA_lower[3][3] = {{{ 5.000000, 0.000000},{-1.100000, 2.200000},{ 3.300000,-4.400000}},
                                         {{-1.000000, 1.000000},{ 7.000000, 0.000000},{ 5.500000, 6.600000}},
                                         {{ 2.000000,-1.000000},{ 1.000000, 1.000000},{ 8.000000, 0.000000}}};
    BLAS_MATH_t templateB_lower[3][2] = {{{ 2.000000,-1.000000},{ 2.000000, 0.000000}},
                                         {{ 1.000000, 1.000000},{-1.000000, 2.000000}},
                                         {{ 0.000000,-2.000000},{ 1.000000, 1.000000}}};
#define templateLen 3
#endif
    bool isUpper = (wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u');
    u64 ii, jj;
    u64 errCnt = 0;
    BLAS_MATH_t zero;
    xSETc(zero, 0.0, 0.0);
    for (i = 0; i < wrapupParams->n; i+=templateLen) {
        for (j = 0; j < wrapupParams->n; j+=templateLen) {
            for (ii = 0; ii < templateLen; ii++) {
                for (jj = 0; jj < templateLen; jj++) {
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
                        if (xCMP_NE(pa[(i+ii)*wrapupParams->lda_array+(j+jj)], isUpper ? templateA_upper[ii][jj] : templateA_lower[ii][jj])) {
                            if (errCnt++ < 100) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                                printf ("a[%ld][%ld] = %f instead of %f\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)],
                                    isUpper ? templateA_upper[ii][jj] : templateA_lower[ii][jj]);
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                                printf ("a[%ld][%ld] = {%f, %f} instead of {%f, %f}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].real,
                                    pa[(i+ii)*wrapupParams->lda_array+(j+jj)].imag,
                                    isUpper ? templateA_upper[ii][jj].real : templateA_lower[ii][jj].real,
                                    isUpper ? templateA_upper[ii][jj].imag : templateA_lower[ii][jj].imag);
#else
error: type not defined.
#endif
                            }
                        }
                        if (j+jj < 2 && xCMP_NE(pb[(i+ii)*wrapupParams->ldb_array+(j+jj)], isUpper ? templateB_upper[ii][jj] : templateB_lower[ii][jj])) {
                            if (errCnt++ < 100) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                                printf ("b[%ld][%ld] = %f instead of %f\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pb[(i+ii)*wrapupParams->ldb_array+(j+jj)],
                                    isUpper ? templateB_upper[ii][jj] : templateB_lower[ii][jj]);
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
                                printf ("b[%ld][%ld] = {%f, %f} instead of {%f, %f}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pb[(i+ii)*wrapupParams->ldb_array+(j+jj)].real,
                                    pb[(i+ii)*wrapupParams->ldb_array+(j+jj)].imag,
                                    isUpper ? templateB_upper[ii][jj].real : templateB_lower[ii][jj].real,
                                    isUpper ? templateB_upper[ii][jj].imag : templateB_lower[ii][jj].imag);
#else
error: type not defined.
#endif
                            }
                        }
                    }
                }
            }
        }
    }
    printf ("Error count for Matrix A and B combined = %ld\n", (u64) errCnt);
#endif

    printf ("Info: %ld\n", (u64) *pInfo);

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(posv_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the POSV library function should NOT be pulling the plug!
    return NULL_GUID;
} // posvWrapupTask
