/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?getrs,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  Solves a system of linear equations
 *    A * X = B  or  A**T * X = B
 *  with a general N-by-N matrix A using the LU factorization computed by ?getrf.
 *
 **/

#include "blas.h"
#include "ocr.h"
#include <stdio.h>

typedef struct {                          // These are the scalar variables that are passed to the GETRF thunking task as its paramv argument.
    BLAS_int_t    n;                      // The order of n-by-n matrix A, and the number of rows in matrix B.
    BLAS_int_t    nrhs;                   // The number right hand sides, i.e. the number of columns in matrix B.
    BLAS_int_t    lda_array;              // The leading dimension of the matrix A as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_array;              // The leading dimension of the matrix B as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    lda_datablock;          // The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_int_t    ldb_datablock;          // The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * a;                      // Address of input matrix A.
    BLAS_MATH_t * b;                      // Address of input matrix B.
} NAME(getrs_thunkParams_t);

typedef struct {                          // These are the dependencies that the GETRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GETRF status.
} NAME(getrs_thunkDeps_t);

static ocrGuid_t NAME(getrs_thunkTask) (   // Moves array-based input matrices (passed by standard GETRS API) into OCR datablocks, then spawns GETRS native OCR EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the GETRF wrapup task as its paramv argument.
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
} NAME(getrs_wrapupParams_t);

typedef struct {                          // These are the dependencies that the GETRF wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dba;                    // Datablock for matrix A.
    ocrEdtDep_t   dbb;                    // Datablock for matrix B.
    ocrEdtDep_t   dbPivotIdx;             // Datablock for pivot indices output.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the GETRF status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when GETRS native OCR EDT is done.
} NAME(getrs_wrapupDeps_t);

static ocrGuid_t NAME(getrs_wrapupTask) ( // Writes matrix, pivot indices, and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?GETRS library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as vector of pivot indices and scalar status info.

void NAME(getrs) (                // Compute solution to a system of linear equations A*X=B for general matrices.
    const BLAS_int_t    trans,    // Input:  0 corresponds to "N" in reference code, meaning no transpose.
                                  //         1 corresponds to "T" or "C", meaning transpose or conjugate-transpose, which
                                  //         are treated the same by this function.
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
    printf("***** NOT TESTED!!!! ***** Entered Standard API %s function.\n", STRINGIFY(NAME(getrs))); fflush(stdout); // TODO

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
    ocrGuid_t getrsThunkTemplate;
    NAME(getrs_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&getrsThunkTemplate, NAME(getrs_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(getrs_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the GETRS result (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t getrsWrapupTemplate;
    NAME(getrs_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&getrsWrapupTemplate, NAME(getrs_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(getrs_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.n                        = n;
    thunkParams.nrhs                     = nrhs;
    thunkParams.lda_array                = lda;
    thunkParams.ldb_array                = ldb;
    thunkParams.lda_datablock            = pitchOfMatA;
    thunkParams.ldb_datablock            = pitchOfMatB;
    thunkParams.a                        = (BLAS_MATH_t *) a;
    thunkParams.b                        = (BLAS_MATH_t *) b;

    ocrGuid_t getrsThunkEdt;
    ocrGuid_t getrsThunkOutputEvent;
    ocrEdtCreate(&getrsThunkEdt, getrsThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &getrsThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API GETRF func.
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

    ocrGuid_t getrsWrapupEdt;
    ocrEdtCreate(&getrsWrapupEdt, getrsWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dba,                   getrsWrapupEdt, NAME(getrs_wrapupDeps_t), dba,                 RO);
    ADD_DEPENDENCE(dbb,                   getrsWrapupEdt, NAME(getrs_wrapupDeps_t), dbb,                 RO);
    ADD_DEPENDENCE(dbPivotIdx,            getrsWrapupEdt, NAME(getrs_wrapupDeps_t), dbPivotIdx,          RO);
    ADD_DEPENDENCE(dbInfo,                getrsWrapupEdt, NAME(getrs_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(getrsThunkOutputEvent, getrsWrapupEdt, NAME(getrs_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dba,                   getrsThunkEdt, NAME(getrs_thunkDeps_t), dba,        RW);
    ADD_DEPENDENCE(dbb,                   getrsThunkEdt, NAME(getrs_thunkDeps_t), dbb,        RW);
    ADD_DEPENDENCE(dbPivotIdx,            getrsThunkEdt, NAME(getrs_thunkDeps_t), dbPivotIdx, RW);
    ADD_DEPENDENCE(dbInfo,                getrsThunkEdt, NAME(getrs_thunkDeps_t), dbInfo,     RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the GETRS operation.\n", STRINGIFY(NAME(getrs))); fflush(stdout);

} // ?getrs


// Receiving array-based input matrices from the standard API for GETRS, this EDT moves them into OCR datablocks, creates datablocks for pivot indices and info, then passes them to GETRS native OCR EDT.
static ocrGuid_t NAME(getrs_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(getrs_thunkTask))); fflush(stdout); // TODO

    NAME(getrs_thunkParams_t) * thunkParams = (NAME(getrs_thunkParams_t) *) paramv;
    NAME(getrs_thunkDeps_t)   * thunkDeps   = (NAME(getrs_thunkDeps_t)   *) depv;

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

    ocrGuid_t getrsTemplate;
    NAME(getrs_edtParams_t) NAME(getrs_edtParams);
    ocrEdtTemplateCreate(&getrsTemplate, NAME(getrs_task), sizeof(NAME(getrs_edtParams))/sizeof(u64), sizeof(NAME(getrs_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(getrs_edtParams).n              = thunkParams->n;
    NAME(getrs_edtParams).nrhs           = thunkParams->nrhs;
    NAME(getrs_edtParams).lda            = thunkParams->lda_datablock;
    NAME(getrs_edtParams).ldb            = thunkParams->ldb_datablock;

    ocrGuid_t getrsEdt;
    ocrEdtCreate(&getrsEdt, getrsTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(getrs_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dba.guid,        getrsEdt, NAME(getrs_edtDeps_t), dba,             RO);
    ADD_DEPENDENCE(thunkDeps->dbb.guid,        getrsEdt, NAME(getrs_edtDeps_t), dbb,             RO);
    ADD_DEPENDENCE(thunkDeps->dbPivotIdx.guid, getrsEdt, NAME(getrs_edtDeps_t), dbPivotIdx,      RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid,     getrsEdt, NAME(getrs_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,                  getrsEdt, NAME(getrs_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(getrs_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?getrs_thunkTask


ocrGuid_t NAME(getrs_task) (           // Spawnable externally, this solves the equations.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    printf ("Entered %s\n", STRINGIFY(NAME(getrs_task))); fflush(stdout);

    NAME(getrs_edtParams_t) * myParams = (NAME(getrs_edtParams_t) *) paramv;
    NAME(getrs_edtDeps_t)   * myDeps   = (NAME(getrs_edtDeps_t)   *) depv;

    BLAS_int_t   trans          = myParams->trans;
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
    // Reference version checks TRANS argument for a value "N", "T", "C", or similar.  This implementation
    // translates that to 0 for "No Transpose", non-zero for "Transpose".  No need to check it here.
    if      (n    < 0) *pInfo = -2;
    else if (nrhs < 0) *pInfo = -3;
    else if (lda < MAX(n,1)) *pInfo = -5;
    else if (ldb < MAX(nrhs,1)) *pInfo = -8;
    if (*pInfo != 0) {
        xerbla (STRINGIFY(NAME(getrs)), -*pInfo);
        return NULL_GUID;
    }

    if (n == 0 || nrhs == 0) return NULL_GUID;   // Quick return if possible.

    ocrGuid_t trsmTemplate;
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    BLAS_MATH_t one;
    xSETc(one, 1.0, 0.0);

    if (trans == 0) {    // Solve A * X = B.
        // Apply row interchages to right hand side.
        NAME(laswp) (nrhs, pMatB, ldb, 0, n-1, pPivotIdx, 1);  // Apply row interchages to right hand side.

        // Solve L*X=B, overwriting B with X.
        // NAME(trsm)  ("Left", "Lower", "No transpose", "Unit",     n, nrhs, one, pMatA, lda, pMatB, ldb);
        NAME(trsm_edtParams_t) NAME(LLNUtrsm_edtParams);
        NAME(LLNUtrsm_edtParams).m              = n;               // Input:  Number of rows of B.
        NAME(LLNUtrsm_edtParams).n              = nrhs;            // Input:  Number of columns of B.
        NAME(LLNUtrsm_edtParams).alpha          = one;             // Input:  Scalar by which to multiply each matrix B value before it is modified.
        NAME(LLNUtrsm_edtParams).lda            = lda;             // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
        NAME(LLNUtrsm_edtParams).ldb            = ldb;             // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
        NAME(LLNUtrsm_edtParams).offseta        = 0;               // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
        NAME(LLNUtrsm_edtParams).offsetb        = 0;               // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
        NAME(LLNUtrsm_edtParams).side           = 'L';             // Input:  Left         ("L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B)
        NAME(LLNUtrsm_edtParams).uplo           = 'L';             // Input:  Lower        ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
        NAME(LLNUtrsm_edtParams).transa         = 'N';             // Input:  No transpose ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
        NAME(LLNUtrsm_edtParams).diag           = 'U';             // Input:  Unit         ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

        ocrGuid_t LLNUtrsmEdt;
        ocrGuid_t LLNU_done;
        ocrEdtCreate(&LLNUtrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(LLNUtrsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &LLNU_done);

        // Solve U*X=B, overwriting B with X.
        // NAME(trsm)  ("Left", "Upper", "No transpose", "Non-unit", n, nrhs, one, pMatA, lda, pMatB, ldb);
        NAME(trsm_edtParams_t) NAME(LUNNtrsm_edtParams) = NAME(LLNUtrsm_edtParams);
        NAME(LUNNtrsm_edtParams).uplo         = 'U';             // Input:  Upper        ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
        NAME(LUNNtrsm_edtParams).diag         = 'N';             // Input:  Non-Unit     ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

        ocrGuid_t LUNNtrsmEdt;
        ocrEdtCreate(&LUNNtrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(LUNNtrsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences, in reverse order.
        ADD_DEPENDENCE(gMatA,     LUNNtrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
        ADD_DEPENDENCE(gMatB,     LUNNtrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
        ADD_DEPENDENCE(LLNU_done, LUNNtrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

        ADD_DEPENDENCE(gMatA,     LLNUtrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
        ADD_DEPENDENCE(gMatB,     LLNUtrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
        ADD_DEPENDENCE(NULL_GUID, LLNUtrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

    } else {             // Solve A**T * X = B
        xerbla (STRINGIFY(NAME(getrs)), 1001);     // Case not yet implemented.
    }
    printf ("        %s exiting\n", STRINGIFY(NAME(getrs_task))); fflush(stdout);
    return NULL_GUID;
} // ?getrs_task


static ocrGuid_t NAME(getrs_wrapupTask) (  // This EDT gains control when ?getrs_task and all that is spawns are done.  It "dethunks" the output original storage passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("***** NOT TESTED!!!! ***** Entered %s\n", STRINGIFY(NAME(getrs_wrapupTask))); fflush(stdout); // TODO

    NAME(getrs_wrapupParams_t) * wrapupParams = (NAME(getrs_wrapupParams_t) *) paramv;
    NAME(getrs_wrapupDeps_t)   * wrapupDeps   = (NAME(getrs_wrapupDeps_t)   *) depv;

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

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(getrs_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the GETRF library function should NOT be pulling the plug!
    return NULL_GUID;
} // getrsWrapupTask
