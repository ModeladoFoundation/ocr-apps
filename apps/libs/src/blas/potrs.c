/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?potrs,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  Solves a system of linear equations A * X = B with a symmetric (single/double precisons) or Hermitian (complex/complex-double precisions) positive
 *  definite matrix A, using the Cholesky factorization A=U**T*U (single/double), A=U**H*U (complex/complex-double); or A=L*L**T or A=L*L**H, computed by ?potrf.
 *
 **/

#include "blas.h"
#include "ocr.h"
#include <stdio.h>

typedef struct {                          // These are the scalar variables that are passed to the POTRF thunking task as its paramv argument.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
} NAME(potrs_thunkParams_t);

typedef struct {                          // These are the dependencies that the POTRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POTRF status.
} NAME(potrs_thunkDeps_t);

static ocrGuid_t NAME(potrs_thunkTask) (   // Moves array-based input matrices (passed by standard POTRS API) into OCR datablocks, then spawns POTRS native OCR EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the POTRF wrapup task as its paramv argument.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
    BLAS_int_t  * info;                   // Address of output scalar of info.
} NAME(potrs_wrapupParams_t);

typedef struct {                          // These are the dependencies that the POTRF wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POTRF status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when POTRS native OCR EDT is done.
} NAME(potrs_wrapupDeps_t);

static ocrGuid_t NAME(potrs_wrapupTask) ( // Writes matrix, pivot indices, and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?POTRS library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as scalar status info.

void NAME(potrs) (                // Compute solution to a system of linear equations A*X=B for symmetric/Hermitian positive definite matrix A.
    const char        * uplo,     // Input: "U" or "u" -- upper triangle of A; "L" or "l" -- lower triangle of A.
    const BLAS_int_t    n,        // Input:  The order of matrix A.
    const BLAS_int_t    nrhs,     // Input:  Number of right hand sides, aka the number of columns in matrix B.
    const BLAS_MATH_t * a,        // Input:  The results from factorization, by ?potrf.
    const BLAS_int_t    lda,      // Input:  The leading dimension (aka "pitch") of matrix a.
    const BLAS_MATH_t * b,        // Input:  right hand side n-by-nhrs matrix b.
                                  // Output: if info=0, the n-by-nrhs solution of matrix X.
    const BLAS_int_t    ldb,      // Input:  The leading dimension (aka "pitch") of matrix b.
    const BLAS_int_t  * info)     // Output: 0 == success; < 0 negation of which argument had an illegal value.
{
    printf("***** NOT TESTED!!!! ***** Entered Standard API %s function.\n", STRINGIFY(NAME(potrs))); fflush(stdout); // TODO

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
    ocrGuid_t potrsThunkTemplate;
    NAME(potrs_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&potrsThunkTemplate, NAME(potrs_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(potrs_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the POTRS result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t potrsWrapupTemplate;
    NAME(potrs_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&potrsWrapupTemplate, NAME(potrs_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(potrs_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.n                        = n;
    thunkParams.nrhs                     = nrhs;
    thunkParams.lda_array                = lda;
    thunkParams.ldb_array                = ldb;
    thunkParams.lda_datablock            = pitchOfMatA;
    thunkParams.ldb_datablock            = pitchOfMatB;
    thunkParams.a                        = (BLAS_MATH_t *) a;
    thunkParams.b                        = (BLAS_MATH_t *) b;

    ocrGuid_t potrsThunkEdt;
    ocrGuid_t potrsThunkOutputEvent;
    ocrEdtCreate(&potrsThunkEdt, potrsThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &potrsThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API POTRF func.
    wrapupParams.n               = n;
    wrapupParams.nrhs            = nrhs;
    wrapupParams.lda_array       = lda;
    wrapupParams.ldb_array       = ldb;
    wrapupParams.lda_datablock   = pitchOfMatA;
    wrapupParams.ldb_datablock   = pitchOfMatB;
    wrapupParams.a               = (BLAS_MATH_t *) a;
    wrapupParams.b               = (BLAS_MATH_t *) b;
    wrapupParams.info            = (BLAS_int_t  *) info;

    ocrGuid_t potrsWrapupEdt;
    ocrEdtCreate(&potrsWrapupEdt, potrsWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                   potrsWrapupEdt, NAME(potrs_wrapupDeps_t), dba,                 RO);
    ADD_DEPENDENCE(dbb,                   potrsWrapupEdt, NAME(potrs_wrapupDeps_t), dbb,                 RO);
    ADD_DEPENDENCE(dbInfo,                potrsWrapupEdt, NAME(potrs_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(potrsThunkOutputEvent, potrsWrapupEdt, NAME(potrs_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                   potrsThunkEdt, NAME(potrs_thunkDeps_t), dba,        RW);
    ADD_DEPENDENCE(dbb,                   potrsThunkEdt, NAME(potrs_thunkDeps_t), dbb,        RW);
    ADD_DEPENDENCE(dbInfo,                potrsThunkEdt, NAME(potrs_thunkDeps_t), dbInfo,     RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the POTRS operation.\n", STRINGIFY(NAME(potrs))); fflush(stdout);

} // ?potrs


// Receiving array-based input matrices from the standard API for POTRS, this EDT moves them into OCR datablocks, creates datablocks for pivot indices and info, then passes them to POTRS native OCR EDT.
static ocrGuid_t NAME(potrs_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(potrs_thunkTask))); fflush(stdout); // TODO

    NAME(potrs_thunkParams_t) * thunkParams = (NAME(potrs_thunkParams_t) *) paramv;
    NAME(potrs_thunkDeps_t)   * thunkDeps   = (NAME(potrs_thunkDeps_t)   *) depv;

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

    ocrGuid_t potrsTemplate;
    NAME(potrs_edtParams_t) NAME(potrs_edtParams);
    ocrEdtTemplateCreate(&potrsTemplate, NAME(potrs_task), sizeof(NAME(potrs_edtParams))/sizeof(u64), sizeof(NAME(potrs_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(potrs_edtParams).n              = thunkParams->n;
    NAME(potrs_edtParams).nrhs           = thunkParams->nrhs;
    NAME(potrs_edtParams).lda            = thunkParams->lda_datablock;
    NAME(potrs_edtParams).ldb            = thunkParams->ldb_datablock;

    ocrGuid_t potrsEdt;
    ocrEdtCreate(&potrsEdt, potrsTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(potrs_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dba.guid,        potrsEdt, NAME(potrs_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(thunkDeps->dbb.guid,        potrsEdt, NAME(potrs_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid,     potrsEdt, NAME(potrs_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,                  potrsEdt, NAME(potrs_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(potrs_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?potrs_thunkTask


ocrGuid_t NAME(potrs_task) (           // Spawnable externally, this solves the equations.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    printf ("Entered %s\n", STRINGIFY(NAME(potrs_task))); fflush(stdout);

    NAME(potrs_edtParams_t) * myParams = (NAME(potrs_edtParams_t) *) paramv;
    NAME(potrs_edtDeps_t)   * myDeps   = (NAME(potrs_edtDeps_t)   *) depv;

    char         uplo           = myParams->uplo[0];
    BLAS_int_t   n              = myParams->n;
    BLAS_int_t   nrhs           = myParams->nrhs;
    BLAS_int_t   lda            = myParams->lda;
    BLAS_int_t   ldb            = myParams->ldb;

    ocrGuid_t     gMatA     = myDeps->dba.guid;
    ocrGuid_t     gMatB     = myDeps->dbb.guid;
    ocrGuid_t     gInfo     = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMatA     = (BLAS_MATH_t *) (myDeps->dba.ptr);
    BLAS_MATH_t * pMatB     = (BLAS_MATH_t *) (myDeps->dbb.ptr);
    BLAS_int_t  * pInfo     = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    // Check argument validity.
    bool isUpper;
    if (uplo == 'U' || uplo == 'u') {
        isUpper = true;
    } else if (uplo == 'L' || uplo == 'l') {
        isUpper = false;
    } else {
        *pInfo = -1;
        xerbla (STRINGIFY(NAME(potrs)), -*pInfo);
        return NULL_GUID;
    }
    if (n    < 0) {
        *pInfo = -2;
        xerbla (STRINGIFY(NAME(potrs)), -*pInfo);
        return NULL_GUID;
    }
    if (nrhs < 0) {
        *pInfo = -3;
        xerbla (STRINGIFY(NAME(potrs)), -*pInfo);
        return NULL_GUID;
    }
    if (lda < MAX(n,1)) {
        *pInfo = -5;
        xerbla (STRINGIFY(NAME(potrs)), -*pInfo);
        return NULL_GUID;
    }
    if (ldb < MAX(nrhs,1)) {
        *pInfo = -7;
        xerbla (STRINGIFY(NAME(potrs)), -*pInfo);
        return NULL_GUID;
    }
    *pInfo = 0;

    if (n == 0 || nrhs == 0) return NULL_GUID;   // Quick return if possible.

    ocrGuid_t trsmTemplate;
    NAME(trsm_edtParams_t) NAME(trsm_edtParams);
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    BLAS_MATH_t one;
    xSETc(one, 1.0, 0.0);

    NAME(trsm_edtParams).m              = n;               // Input:  Number of rows of B.
    NAME(trsm_edtParams).n              = nrhs;            // Input:  Number of columns of B.
    NAME(trsm_edtParams).alpha          = one;             // Input:  Scalar by which to multiply each matrix B value before it is modified.
    NAME(trsm_edtParams).lda            = lda;             // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    NAME(trsm_edtParams).ldb            = ldb;             // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    NAME(trsm_edtParams).offseta        = 0;               // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
    NAME(trsm_edtParams).offsetb        = 0;               // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
    NAME(trsm_edtParams).side           = 'L';             // Input:  Left           ("L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B)
    NAME(trsm_edtParams).uplo           = uplo;            // Input:  Upper or Lower ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
    if (isUpper) {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        NAME(trsm_edtParams).transa     = 'T';             // Input:  Transpose      ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#else
        NAME(trsm_edtParams).transa     = 'C';             // Input:  Conj transpose ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#endif
    } else {
        NAME(trsm_edtParams).transa     = 'N';             // Input:  No transpose   ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
    }
    NAME(trsm_edtParams).diag           = 'N';             // Input:  Non-Unit       ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

    ocrGuid_t firstTrsmEdt;
    ocrGuid_t firstDone;
    ocrEdtCreate(&firstTrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(trsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &firstDone);

    if (isUpper) {
        NAME(trsm_edtParams).transa     = 'N';             // Input:  No transpose   ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
    } else {
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
        NAME(trsm_edtParams).transa     = 'T';             // Input:  Transpose      ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#else
        NAME(trsm_edtParams).transa     = 'C';             // Input:  Conj transpose ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#endif
    }

    ocrGuid_t secondTrsmEdt;
    ocrGuid_t secondDone;
    ocrEdtCreate(&secondTrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(trsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &secondDone);

// Add the dependences, in reverse order.
    ADD_DEPENDENCE(gMatA,     secondTrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(gMatB,     secondTrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(firstDone, secondTrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

    ADD_DEPENDENCE(gMatA,     firstTrsmEdt,  NAME(trsm_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(gMatB,     firstTrsmEdt,  NAME(trsm_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(NULL_GUID, firstTrsmEdt,  NAME(trsm_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(potrs_task))); fflush(stdout);
    return NULL_GUID;
} // ?potrs_task


static ocrGuid_t NAME(potrs_wrapupTask) (  // This EDT gains control when ?potrs_task and all that is spawns are done.  It "dethunks" the output original storage passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(potrs_wrapupTask))); fflush(stdout); // TODO

    NAME(potrs_wrapupParams_t) * wrapupParams = (NAME(potrs_wrapupParams_t) *) paramv;
    NAME(potrs_wrapupDeps_t)   * wrapupDeps   = (NAME(potrs_wrapupDeps_t)   *) depv;

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

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(potrs_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the POTRF library function should NOT be pulling the plug!
    return NULL_GUID;
} // potrsWrapupTask
