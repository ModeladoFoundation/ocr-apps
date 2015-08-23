/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

/**
 * ?POTRF,  where ? is: <s = single precision | d = double precision | c = single precision complex | z = double precision complex>
 *
 *  Computes the Cholesky factorization of a symmetric positive definite matrix A.
 *
 * The factorization has the form
 *    A = U**T * U if UPLO = 'U',   or A = L * L**T if UPLO = 'L'
 * where U is an upper triangular matrix and L is a lower triangular matrix.
 *
 **/


#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include <stdio.h>


typedef struct {                          // These are the scalar variables that are passed to the POTRF thunking EDT as its paramv argument.
    char        * uplo;                   // 'U' or 'u' for upper triangle of A;  'L' or 'l' for lower triangle of A.
    BLAS_int_t    n;                      // The number of rows and columns of the n-by-n matrix.
    BLAS_int_t    ld_array;               // The leading dimension of the matrix as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ld_datablock;           // The leading dimension of the matrix as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * mat;                    // Address of input matrix.
} NAME(potrf_thunkParams_t);

typedef struct {                          // These are the dependencies that the POTRF thunking task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POTRF status.
} NAME(potrf_thunkDeps_t);

static ocrGuid_t NAME(potrf_thunkTask) (  // Moves array-based input matrix (passed by standard POTRF API) into OCR datablock, then spawns top-level worker EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the POTRF worker as its paramv argument.
    BLAS_int_t    n;                      // The number of rows and columns of the n-by-n matrix.
    BLAS_int_t    ld;                     // The leading dimension of the matrix (in elements of BLAS_MATH_t).
    ocrGuid_t     workerTemplate;         // GUID of EdtTemplate for the worker.  Passed down so that worker EDT's can be instantiated.
    BLAS_int_t    j;                      // POTRF main loop index.  Original code loop is "for (j = 0; j < jLimit; j += jInc) ..."
    BLAS_int_t    jInc;                   // POTRF main loop increment.
    BLAS_int_t    workerTaskStep;         // Which step of the worker's continuation is next?
} NAME(potrf_workerParams_t);

typedef struct {                          // These are the dependencies that the POTRF worker task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POTRF status.
    ocrEdtDep_t   trigger;                // Trigger fired by completion of predecessor (e.g. previous worker's final operation, which is the GEMM operation).
} NAME(potrf_workerDeps_t);

static ocrGuid_t NAME(potrf_workerTask_upperTriangle) ( // This EDT performs one iteration of the blocked POTRF main loop (for uplo='U'), spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);

static ocrGuid_t NAME(potrf_workerTask_lowerTriangle) ( // This EDT performs one iteration of the blocked POTRF main loop (for uplo='L'), spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {                          // These are the scalar variables that are passed to the POTRF wrapup EDT as its paramv argument.
    char        * uplo;                   // 'U' or 'u' for upper triangle of A;  'L' or 'l' for lower triangle of A.
    BLAS_int_t    n;                      // The number of rows and columns of the n-by-n matrix.
    BLAS_int_t    ld_array;               // The leading dimension of the matrix as passed in (in elements of BLAS_MATH_t).
    BLAS_int_t    ld_datablock;           // The leading dimension of the matrix as represented in the thunked datablock (in elements of BLAS_MATH_t).
    BLAS_MATH_t * mat;                    // Address of input matrix.
    BLAS_int_t  * info;                   // Address of output scalar of info.
} NAME(potrf_wrapupParams_t);

typedef struct {                          // These are the dependencies that the POTRF wrapup task needs satisfied before it can fire.
    ocrEdtDep_t   dbMat;                  // Datablock for matrix.
    ocrEdtDep_t   dbInfo;                 // Datablock for returning info about the POTRF status.
    ocrEdtDep_t   event_ThunkToWrapup;    // Trigger for wrapup, fired when all POTRF workers are done.
} NAME(potrf_wrapupDeps_t);

static ocrGuid_t NAME(potrf_wrapupTask) ( // Writes matrix and status info back out to caller.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// Externally visible API for ?POTRF library function:
//
// "Thunks" the input arguments into suitable form for running as pure OCR.  This implies the following:
//
//    Input/output matrix is copied from the array storage passed to this function, to datablocks for processing in pure OCR fashion.  Upon completion,
//    matrix is copied back out, as well as scalar status info.

void NAME(potrf) (
    char        * uplo,     // "U" or "u" for upper triangle of A;  "L" or "l" for lower triangle of A.
    BLAS_int_t    n,        // The number of rows and columns of the n-by-n matrix.
    BLAS_MATH_t * mat,      // Input array, size n*lda.
    BLAS_int_t    ld,       // Leading dimension (aka "pitch", aka "vertical stride", aka "vertical skip distance") of matrix.
    BLAS_int_t  * info)     // Output status value.  If <0, this indicates an error condition.  If >0, this is a one-based index of a zero coefficient.  If 0, completely normal.
{
    printf("Entered Standard API %s function.\n", STRINGIFY(NAME(potrf))); fflush(stdout);

// Create the datablocks:

    ocrGuid_t  dbMat;         // Guid of datablock for matrix
    ocrGuid_t  dbInfo;        // Guid of datablock for info output.

    u64 * addr;        // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.  The initialization of the datablock's values happens in the thunk func.)

    BLAS_int_t pitchOfMat = (n + ((BLAS_int_t) 7)) & (~((BLAS_int_t) 7));
    BLAS_int_t sizeOfMatInBytes = (n * pitchOfMat) * sizeof(BLAS_MATH_t);
    ocrDbCreate (&dbMat, (void**) &addr, sizeOfMatInBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    ocrDbCreate (&dbInfo, (void**) &addr, sizeof(BLAS_int_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create a template for the thunking EDT.  An instance of it will be created after we create other templates we will need.
    ocrGuid_t potrfThunkTemplate;
    NAME(potrf_thunkParams_t) thunkParams;
    ocrEdtTemplateCreate(&potrfThunkTemplate, NAME(potrf_thunkTask), sizeof(thunkParams)/sizeof(u64), sizeof(NAME(potrf_thunkDeps_t))/sizeof(ocrEdtDep_t));

// Finally, create a template for the final wrapup EDT.  This EDT will examine the result of POTRF (if in testing mode), and (TODO) return control back to the caller of the standard API (somehow TBD!)
    ocrGuid_t potrfWrapupTemplate;
    NAME(potrf_wrapupParams_t) wrapupParams;
    ocrEdtTemplateCreate(&potrfWrapupTemplate, NAME(potrf_wrapupTask), sizeof(wrapupParams)/sizeof(u64), sizeof(NAME(potrf_wrapupDeps_t))/sizeof(ocrEdtDep_t));

// Create an instance of the "thunking" EDT -- which completes the work of THIS EDT by copying the input matrix arrays to datablocks, and then spawns the top-level worker.
    thunkParams.uplo                     = uplo;
    thunkParams.n                        = n;
    thunkParams.ld_array                 = ld;
    thunkParams.ld_datablock             = pitchOfMat;
    thunkParams.mat                      = (BLAS_MATH_t *) mat;

    ocrGuid_t potrfThunkEdt;
    ocrGuid_t potrfThunkOutputEvent;
    ocrEdtCreate(&potrfThunkEdt, potrfThunkTemplate, EDT_PARAM_DEF, (u64 *) (&thunkParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &potrfThunkOutputEvent);

// Create an instance for the wrapup EDT, which also "dethunks" the output data back from the matrix datablock to the array that the user originally passed into the standard API POTRF func.
    wrapupParams.uplo            = uplo;
    wrapupParams.n               = n;
    wrapupParams.ld_array        = ld;
    wrapupParams.ld_datablock    = pitchOfMat;
    wrapupParams.mat             = (BLAS_MATH_t *) mat;
    wrapupParams.info            = (BLAS_int_t  *) info;

    ocrGuid_t potrfWrapupEdt;
    ocrEdtCreate(&potrfWrapupEdt, potrfWrapupTemplate, EDT_PARAM_DEF, (u64 *) (&wrapupParams), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

// Add the dependences to the wrapup EDT.
    ADD_DEPENDENCE(dbMat,                 potrfWrapupEdt, NAME(potrf_wrapupDeps_t), dbMat,               RO);
    ADD_DEPENDENCE(dbInfo,                potrfWrapupEdt, NAME(potrf_wrapupDeps_t), dbInfo,              RO);
    ADD_DEPENDENCE(potrfThunkOutputEvent, potrfWrapupEdt, NAME(potrf_wrapupDeps_t), event_ThunkToWrapup, RO);

// Add the dependences to the thunking EDT.
    ADD_DEPENDENCE(dbMat,                potrfThunkEdt, NAME(potrf_thunkDeps_t), dbMat,      RW);
    ADD_DEPENDENCE(dbInfo,               potrfThunkEdt, NAME(potrf_thunkDeps_t), dbInfo,     RW);
    printf("        Standard API %s function exiting.  TODO: evolve into WAITING for result of spawned OCR topology that does the POTRF operation.\n", STRINGIFY(NAME(potrf))); fflush(stdout);

} // ?potrf


// Receiving array-based input matrix from the standard API for POTRF, this EDT moves it into OCR datablock, creates datablock for info, then passes them to a top-level child worker EDT.
static ocrGuid_t NAME(potrf_thunkTask) (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    printf ("Entered %s\n", STRINGIFY(NAME(potrf_thunkTask))); fflush(stdout);

    NAME(potrf_thunkParams_t) * thunkParams = (NAME(potrf_thunkParams_t) *) paramv;
    NAME(potrf_thunkDeps_t)   * thunkDeps   = (NAME(potrf_thunkDeps_t)   *) depv;

    u64 i, j;
    const BLAS_MATH_t * pMat_arr = thunkParams->mat;
    BLAS_MATH_t * pMat_db  = thunkDeps->dbMat.ptr;

    for (i = 0; i < thunkParams->n; i++) {
        for (j = 0; j < thunkParams->n; j++) {
            pMat_db[j] = pMat_arr[j];
        }
        pMat_db  += thunkParams->ld_datablock;
        pMat_arr += thunkParams->ld_array;
    }

    ocrGuid_t potrfTemplate;
    NAME(potrf_edtParams_t) NAME(potrf_edtParams);
    ocrEdtTemplateCreate(&potrfTemplate, NAME(potrf_task), sizeof(NAME(potrf_edtParams))/sizeof(u64), sizeof(NAME(potrf_edtDeps_t))/sizeof(ocrEdtDep_t));

    NAME(potrf_edtParams).uplo           = thunkParams->uplo;
    NAME(potrf_edtParams).n              = thunkParams->n;
    NAME(potrf_edtParams).ld             = thunkParams->ld_datablock;

    ocrGuid_t potrfEdt;
    ocrEdtCreate(&potrfEdt, potrfTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(potrf_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the top level EDT.
    ADD_DEPENDENCE(thunkDeps->dbMat.guid,      potrfEdt, NAME(potrf_edtDeps_t), dbMat,           RO);
    ADD_DEPENDENCE(thunkDeps->dbInfo.guid,     potrfEdt, NAME(potrf_edtDeps_t), dbInfo,          RW);
    ADD_DEPENDENCE(NULL_GUID,                  potrfEdt, NAME(potrf_edtDeps_t), optionalTrigger, RO);

    printf ("        %s exiting\n", STRINGIFY(NAME(potrf_thunkTask))); fflush(stdout);
    return NULL_GUID;
} // ?potrf_thunkTask

BLAS_MATH_t * pMat_glbl;

ocrGuid_t NAME(potrf_task) (           // Spawnable externally, or spawned by the thunk layer, this spawns the top-level worker, i.e. the worker for the first iteration of the main loop.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    //printf ("Entered %s\n", STRINGIFY(NAME(potrf_task))); fflush(stdout);

    NAME(potrf_edtParams_t) * myParams = (NAME(potrf_edtParams_t) *) paramv;
    NAME(potrf_edtDeps_t)   * myDeps   = (NAME(potrf_edtDeps_t)   *) depv;

    char          uplo      = myParams->uplo[0];
    BLAS_int_t    n         = myParams->n;
    BLAS_int_t    ld        = myParams->ld;

    ocrGuid_t     gMat      = myDeps->dbMat.guid;
    ocrGuid_t     gInfo     = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMat      = (BLAS_MATH_t *) (myDeps->dbMat.ptr);
    BLAS_int_t  * pInfo     = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

// Create a template for the worker EDT.
    ocrGuid_t workerTemplate;
    ocrEdtTemplateCreate(&workerTemplate,
        ((uplo == 'U' || uplo == 'u') ? NAME(potrf_workerTask_upperTriangle) : NAME(potrf_workerTask_lowerTriangle)),
        sizeof(NAME(potrf_workerParams_t))/sizeof(u64), sizeof(NAME(potrf_workerDeps_t))/sizeof(ocrEdtDep_t));

    *pInfo = 0;
    // Check argument validity.
    if (uplo != 'U' && uplo != 'u' && uplo != 'L' && uplo != 'l') *pInfo = -1;
    else if (n < 0) *pInfo = -2;
    else if (ld < MAX(n,1)) *pInfo = -4;
    if (*pInfo != 0) {
        xerbla (STRINGIFY(NAME(potrf)), -*pInfo);
        return NULL_GUID;
    }
    // Quick return if possible
    if (n == 0) {
        return NULL_GUID;
    }

    NAME(potrf_workerParams_t) workerParams;
    workerParams.n              = n;
    workerParams.ld             = ld;
    workerParams.workerTemplate = workerTemplate;
    workerParams.j              = 0;              // Set up for first worker.
    workerParams.jInc           = ilaenv(1, STRINGIFY(NAME(potrf)), " ", n, -1, -1, -1);  // Dig up environment-appropriate value for block size. jInc is called nb in ref code.
    workerParams.workerTaskStep = 1;

    if (workerParams.jInc <= 1 || workerParams.jInc >= workerParams.n) {
        // Use unblocked code:   Just invoke DGETF2 ***ONCE***, and that's all!

        NAME(potf2)(myParams->uplo, n, pMat, ld, pInfo);
        return NULL_GUID;
    }

pMat_glbl = pMat;

    // Use blocked code:  Do so by invoking the worker EDT, set up for the first iteration of the main loop in the reference code.
    ocrGuid_t potrfWorkerEdt;
    ocrEdtCreate(&potrfWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&workerParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Add the dependences to the worker.
    ADD_DEPENDENCE(gMat,                 potrfWorkerEdt, NAME(potrf_workerDeps_t), dbMat,      RW);
    ADD_DEPENDENCE(gInfo,                potrfWorkerEdt, NAME(potrf_workerDeps_t), dbInfo,     RW);
    ADD_DEPENDENCE(NULL_GUID,            potrfWorkerEdt, NAME(potrf_workerDeps_t), trigger,    RO);
    //printf ("        %s exiting\n", STRINGIFY(NAME(potrf_task))); fflush(stdout);
    return NULL_GUID;
} // ?potrf_task


static ocrGuid_t NAME(potrf_workerTask_upperTriangle) ( // This EDT performs one iteration of the blocked POTRF main loop (for uplo='U'), spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    NAME(potrf_workerParams_t) * myParams = (NAME(potrf_workerParams_t) *) paramv;
    NAME(potrf_workerDeps_t)   * myDeps   = (NAME(potrf_workerDeps_t)   *) depv;

    BLAS_int_t    n              = myParams->n;
    BLAS_int_t    ld             = myParams->ld;
    BLAS_int_t    workerTemplate = myParams->workerTemplate;
    BLAS_int_t    j              = myParams->j;
    BLAS_int_t    jInc           = myParams->jInc;
    BLAS_int_t    workerTaskStep = myParams->workerTaskStep;

    ocrGuid_t     gMat           = myDeps->dbMat.guid;
    ocrGuid_t     gInfo          = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMat           = (BLAS_MATH_t *) (myDeps->dbMat.ptr);
    BLAS_int_t  * pInfo          = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    //printf ("Entered %s  j = %ld, jInc = %ld, n = %ld\n", STRINGIFY(NAME(potrf_workerTask_upperTriangle)), (u64) j, (u64) jInc, (u64) n); fflush(stdout);

    BLAS_MATH_t   minusOne, one;
    xSETc (minusOne, -1.0, 0.0);
    xSETc (one,       1.0, 0.0);

// Create a template for the natural OCR EDT for spawning GEMM.
    ocrGuid_t gemmTemplate;
    ocrEdtTemplateCreate(&gemmTemplate, NAME(gemm_task), sizeof(NAME(gemm_edtParams_t))/sizeof(u64), sizeof(NAME(gemm_edtDeps_t))/sizeof(ocrEdtDep_t));

    ocrGuid_t trsmTemplate;
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    for (/*j = value passed in*/  ; j < n; j += jInc) {   // Iterate in THIS worker until we need to spawn a GEMM, at which time we will also spawn a follow-on worker to continue the iterations.
        BLAS_int_t  blkSz = MIN(n-j, jInc);
        switch (workerTaskStep) {   // step through the phases of the worker.
        case 1: // Do first step.  Right now, that's everything.
        {
            // Update and factorize the current diagonal block and test for non-positive-definiteness.
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            NAME(syrk)("Upper", "Transpose", blkSz, j, minusOne, &pMat[0*ld+j], ld, one, &pMat[j*ld+j], ld);
#else
            NAME(herk)("Upper", "Conjugate Transpose", blkSz, j, -1.0, &pMat[0*ld+j], ld, 1.0, &pMat[j*ld+j], ld);
#endif
            NAME(potf2)("Upper", blkSz, &pMat[j*ld+j], ld, pInfo);
            if (*pInfo != 0) {
                pInfo += j;   // Return one-based index of leading minor that is not positive definite.
                return NULL_GUID;
            }
            if (j + blkSz < n) {
                // Compute the current block row.

//                  CALL SGEMM( 'Transpose', 'No transpose', JB, N-J-JB+1,
//     $                        J-1, -ONE, A( 1, J ), LDA, A( 1, J+JB ),
//     $                        LDA, ONE, A( J, J+JB ), LDA )
                NAME(gemm_edtParams_t) gemmParams;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                gemmParams.transa          = 'T';
#else
                gemmParams.transa          = 'C';
#endif
                gemmParams.transb          = 'N';
                gemmParams.m               = blkSz;
                gemmParams.n               = n-j-blkSz;
                gemmParams.k               = j;
                gemmParams.alpha           = minusOne;
                gemmParams.beta            = one;
                gemmParams.offseta         = 0*ld + (j      );
                gemmParams.offsetb         = 0*ld + (j+blkSz);
                gemmParams.offsetc         = j*ld + (j+blkSz);
                gemmParams.lda             = ld;
                gemmParams.ldb             = ld;
                gemmParams.ldc             = ld;

                ocrGuid_t gemmEdt;
                ocrGuid_t gemmDone;
                ocrEdtCreate(&gemmEdt, gemmTemplate, EDT_PARAM_DEF, (u64 *) (&gemmParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmDone);


//                  CALL STRSM( 'Left', 'Upper', 'Transpose', 'Non-unit',
//     $                        JB, N-J-JB+1, ONE, A( J, J ), LDA,
//     $                        A( J, J+JB ), LDA )
                // NAME(trsm)("Left", "Upper", "Transpose", "Non-unit", blkSz, n-j-blkSz, one, &pMat[j*ld+j], ld, &pMat[j*ld+(j+blkSz)], ld);
                NAME(trsm_edtParams_t) NAME(LUTNtrsm_edtParams);
                NAME(LUTNtrsm_edtParams).m              = blkSz;           // Input:  Number of rows of B.
                NAME(LUTNtrsm_edtParams).n              = n-j-blkSz;       // Input:  Number of columns of B.
                NAME(LUTNtrsm_edtParams).alpha          = one;             // Input:  Scalar by which to multiply each matrix B value before it is modified.
                NAME(LUTNtrsm_edtParams).lda            = ld;              // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(LUTNtrsm_edtParams).ldb            = ld;              // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(LUTNtrsm_edtParams).offseta        = j*ld+j;          // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
                NAME(LUTNtrsm_edtParams).offsetb        = j*ld+(j+blkSz);  // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
                NAME(LUTNtrsm_edtParams).side           = 'L';             // Input:  Left         ("L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B)
                NAME(LUTNtrsm_edtParams).uplo           = 'U';             // Input:  Upper        ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                NAME(LUTNtrsm_edtParams).transa         = 'T';             // Input:  Transpose    ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#else
                NAME(LUTNtrsm_edtParams).transa         = 'C';             // Input:  Transpose    ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#endif
                NAME(LUTNtrsm_edtParams).diag           = 'N';             // Input:  Non-unit     ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

                ocrGuid_t LUTNtrsmEdt;
                ocrGuid_t LUTN_done;
                ocrEdtCreate(&LUTNtrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(LUTNtrsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &LUTN_done);

                // Spawn the continuation of this POTRF worker to the next iteration of the loop.
                NAME(potrf_workerParams_t) childParams;
                childParams.n              = n;
                childParams.ld             = ld;
                childParams.workerTemplate = workerTemplate;
                childParams.j              = j+jInc;
                childParams.jInc           = jInc;
                childParams.workerTaskStep = 1;

                ocrGuid_t childWorkerEdt;
                ocrEdtCreate(&childWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

                // Add the dependences to the worker and TRSM and GEMM, in reverse order.
                ADD_DEPENDENCE(gMat,      childWorkerEdt, NAME(potrf_workerDeps_t), dbMat,      RW);
                ADD_DEPENDENCE(gInfo,     childWorkerEdt, NAME(potrf_workerDeps_t), dbInfo,     RW);
                ADD_DEPENDENCE(LUTN_done, childWorkerEdt, NAME(potrf_workerDeps_t), trigger,    RO);

                ADD_DEPENDENCE(gMat,      LUTNtrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
                ADD_DEPENDENCE(gMat,      LUTNtrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
                ADD_DEPENDENCE(gemmDone,  LUTNtrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dba,             RO);
                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dbb,             RO);
                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dbc,             RW);
                ADD_DEPENDENCE(NULL_GUID, gemmEdt, NAME(gemm_edtDeps_t), optionalTrigger, RO);

                //printf ("        %s exiting (Step 1)\n", STRINGIFY(NAME(potrf_workerTask_upperTriangle))); fflush(stdout);
                return NULL_GUID;
            }
        }
        break;
        default:
        {
            printf ("****  %s  Error: illegal case value %ld\n", STRINGIFY(NAME(potrf_workerTask_upperTriangle)), (u64) workerTaskStep);
        }
        } // switch
    } // for
    //printf ("        %s exiting (final iteration)\n", STRINGIFY(NAME(potrf_workerTask_upperTriangle))); fflush(stdout);
    return NULL_GUID;
} // ?potrf_workerTask_upperTriangle


static ocrGuid_t NAME(potrf_workerTask_lowerTriangle) ( // This EDT performs one iteration of the blocked POTRF main loop (for uplo='L'), spawning another instance of itself to carry on with remaining instances.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    NAME(potrf_workerParams_t) * myParams = (NAME(potrf_workerParams_t) *) paramv;
    NAME(potrf_workerDeps_t)   * myDeps   = (NAME(potrf_workerDeps_t)   *) depv;

    BLAS_int_t    n              = myParams->n;
    BLAS_int_t    ld             = myParams->ld;
    BLAS_int_t    workerTemplate = myParams->workerTemplate;
    BLAS_int_t    j              = myParams->j;
    BLAS_int_t    jInc           = myParams->jInc;
    BLAS_int_t    workerTaskStep = myParams->workerTaskStep;

    ocrGuid_t     gMat           = myDeps->dbMat.guid;
    ocrGuid_t     gInfo          = myDeps->dbInfo.guid;
    BLAS_MATH_t * pMat           = (BLAS_MATH_t *) (myDeps->dbMat.ptr);
    BLAS_int_t  * pInfo          = (BLAS_int_t *)  (myDeps->dbInfo.ptr);

    //printf ("Entered %s  j = %ld, jInc = %ld, n = %ld\n", STRINGIFY(NAME(potrf_workerTask_lowerTriangle)), (u64) j, (u64) jInc, (u64) n); fflush(stdout);

    BLAS_MATH_t   minusOne, one;
    xSETc (minusOne, -1.0, 0.0);
    xSETc (one,       1.0, 0.0);

// Create a template for the natural OCR EDT for spawning GEMM.
    ocrGuid_t gemmTemplate;
    ocrEdtTemplateCreate(&gemmTemplate, NAME(gemm_task), sizeof(NAME(gemm_edtParams_t))/sizeof(u64), sizeof(NAME(gemm_edtDeps_t))/sizeof(ocrEdtDep_t));

    ocrGuid_t trsmTemplate;
    ocrEdtTemplateCreate(&trsmTemplate, NAME(trsm_task), sizeof(NAME(trsm_edtParams_t))/sizeof(u64), sizeof(NAME(trsm_edtDeps_t))/sizeof(ocrEdtDep_t));

    for (/*j = value passed in*/  ; j < n; j += jInc) {   // Iterate in THIS worker until we need to spawn a GEMM, at which time we will also spawn a follow-on worker to continue the iterations.
        BLAS_int_t  blkSz = MIN(n-j, jInc);
        switch (workerTaskStep) {   // step through the phases of the worker.
        case 1: // Do first step.  Right now, that's everything.
        {
            // Update and factorize the current diagonal block and test for non-positive-definiteness.
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
            NAME(syrk)("Lower", "No Transpose", blkSz, j, minusOne, &pMat[j*ld+0], ld, one, &pMat[j*ld+j], ld);
#else
            NAME(herk)("Lower", "No Transpose", blkSz, j, -1.0, &pMat[j*ld+0], ld, 1.0, &pMat[j*ld+j], ld);
#endif
            NAME(potf2)("Lower", blkSz, &pMat[j*ld+j], ld, pInfo);
            if (*pInfo != 0) {
                pInfo += j;   // Return one-based index of leading minor that is not positive definite.
                return NULL_GUID;
            }
            if (j + blkSz < n) {
                // Compute the current block row.

//                  CALL SGEMM( 'No Transpose', 'transpose', N-J-JB+1, JB,
//     $                        J-1, -ONE, A( J+JB, 1 ), LDA, A( J, 1 ),
//     $                        LDA, ONE, A( J+JB, J ), LDA )
                NAME(gemm_edtParams_t) gemmParams;
                gemmParams.transa          = 'N';
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                gemmParams.transb          = 'T';
#else
                gemmParams.transb          = 'C';
#endif
                gemmParams.m               = n-j-blkSz;
                gemmParams.n               = blkSz;
                gemmParams.k               = j;
                gemmParams.alpha           = minusOne;
                gemmParams.beta            = one;
                gemmParams.offseta         = (j+blkSz)*ld + 0;
                gemmParams.offsetb         = (j      )*ld + 0;
                gemmParams.offsetc         = (j+blkSz)*ld + j;
                gemmParams.lda             = ld;
                gemmParams.ldb             = ld;
                gemmParams.ldc             = ld;

                ocrGuid_t gemmEdt;
                ocrGuid_t gemmDone;
                ocrEdtCreate(&gemmEdt, gemmTemplate, EDT_PARAM_DEF, (u64 *) (&gemmParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gemmDone);


//                  CALL STRSM( 'Right', 'Lower', 'Transpose', 'Non-unit',
//     $                        N-J-JB+1, JB, ONE, A( J, J ), LDA,
//     $                        A( J+JB, J ), LDA )
                // NAME(trsm)("Right", "Lower", "Transpose", "Non-unit", n-j-blkSz, blkSz, one, &pMat[j*ld+j], ld, &pMat[(j+blkSz)*ld+j], ld);
                NAME(trsm_edtParams_t) NAME(RLTNtrsm_edtParams);
                NAME(RLTNtrsm_edtParams).m              = n-j-blkSz;       // Input:  Number of rows of B.
                NAME(RLTNtrsm_edtParams).n              = blkSz;           // Input:  Number of columns of B.
                NAME(RLTNtrsm_edtParams).alpha          = one;             // Input:  Scalar by which to multiply each matrix B value before it is modified.
                NAME(RLTNtrsm_edtParams).lda            = ld;              // Input:  The leading dimension of the matrix A as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(RLTNtrsm_edtParams).ldb            = ld;              // Input:  The leading dimension of the matrix B as represented in the thunked datablock (in elements of BLAS_MATH_t).
                NAME(RLTNtrsm_edtParams).offseta        = j*ld+j;          // Input:  Offset (in units of BLAS_MATH_t) from start of dba to upper left element to process.
                NAME(RLTNtrsm_edtParams).offsetb        = (j+blkSz)*ld+j;  // Input:  Offset (in units of BLAS_MATH_t) from start of dbb to upper left element to process.
                NAME(RLTNtrsm_edtParams).side           = 'R';             // Input:  Left         ("L" or "l" (or "Left" ...) --> op(A)*X=alpha*B;   "R" or "r" --> X*op(A)=alpha*B)
                NAME(RLTNtrsm_edtParams).uplo           = 'L';             // Input:  Upper        ("U" or "u" --> A is an upper triangular matrix;   "L" or "l" --> A is a lower triangular matrix.)
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
                NAME(RLTNtrsm_edtParams).transa         = 'T';             // Input:  Transpose    ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#else
                NAME(RLTNtrsm_edtParams).transa         = 'C';             // Input:  Transpose    ("N" or "n" --> op(A)=A;  "T" or "t" --> op(A)=A**T;   "C" or "c" --> op(A) = A**T)
#endif
                NAME(RLTNtrsm_edtParams).diag           = 'N';             // Input:  Non-unit     ("U" or "u" --> A is assumed to be unit triangular;  "N" or "n"  --> A is not assumed to be unit triangular.)

                ocrGuid_t RLTNtrsmEdt;
                ocrGuid_t RLTN_done;
                ocrEdtCreate(&RLTNtrsmEdt, trsmTemplate, EDT_PARAM_DEF, (u64 *) (&NAME(RLTNtrsm_edtParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &RLTN_done);

                // Spawn the continuation of this POTRF worker to the next iteration of the loop.
                NAME(potrf_workerParams_t) childParams;
                childParams.n              = n;
                childParams.ld             = ld;
                childParams.workerTemplate = workerTemplate;
                childParams.j              = j+jInc;
                childParams.jInc           = jInc;
                childParams.workerTaskStep = 1;

                ocrGuid_t childWorkerEdt;
                ocrEdtCreate(&childWorkerEdt, workerTemplate, EDT_PARAM_DEF, (u64 *) (&childParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

                // Add the dependences to the worker and TRSM and GEMM, in reverse order.
                ADD_DEPENDENCE(gMat,      childWorkerEdt, NAME(potrf_workerDeps_t), dbMat,      RW);
                ADD_DEPENDENCE(gInfo,     childWorkerEdt, NAME(potrf_workerDeps_t), dbInfo,     RW);
                ADD_DEPENDENCE(RLTN_done, childWorkerEdt, NAME(potrf_workerDeps_t), trigger,    RO);

                ADD_DEPENDENCE(gMat,      RLTNtrsmEdt, NAME(trsm_edtDeps_t), dba,             RO);
                ADD_DEPENDENCE(gMat,      RLTNtrsmEdt, NAME(trsm_edtDeps_t), dbb,             RO);
                ADD_DEPENDENCE(gemmDone,  RLTNtrsmEdt, NAME(trsm_edtDeps_t), optionalTrigger, RO);

                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dba,             RO);
                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dbb,             RO);
                ADD_DEPENDENCE(gMat,      gemmEdt, NAME(gemm_edtDeps_t), dbc,             RW);
                ADD_DEPENDENCE(NULL_GUID, gemmEdt, NAME(gemm_edtDeps_t), optionalTrigger, RO);

                //printf ("        %s exiting (Step 1)\n", STRINGIFY(NAME(potrf_workerTask_lowerTriangle))); fflush(stdout);
                return NULL_GUID;
            }
        }
        break;
        default:
        {
            printf ("****  %s  Error: illegal case value %ld\n", STRINGIFY(NAME(potrf_workerTask_lowerTriangle)), (u64) workerTaskStep);
        }
        } // switch
    } // for
    //printf ("        %s exiting (final iteration)\n", STRINGIFY(NAME(potrf_workerTask_lowerTriangle))); fflush(stdout);
    return NULL_GUID;
} // ?potrf_workerTask_lowerTriangle


static ocrGuid_t NAME(potrf_wrapupTask) (  // This EDT gains control when all POTRF worker EDTs are done.  It "dethunks" the output matrix C from the OCR datablock to the original array passed in by the user.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t * depv) {

    printf ("Entered %s\n", STRINGIFY(NAME(potrf_wrapupTask))); fflush(stdout);

    NAME(potrf_wrapupParams_t) * wrapupParams = (NAME(potrf_wrapupParams_t) *) paramv;
    NAME(potrf_wrapupDeps_t)   * wrapupDeps   = (NAME(potrf_wrapupDeps_t)   *) depv;

    u64 i, j;
    BLAS_MATH_t * pMat_db  = wrapupDeps->dbMat.ptr;
    BLAS_MATH_t * pMat_arr = wrapupParams->mat;
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
            pMat_arr[j] = pMat_db[j];
        }
        pMat_db  += wrapupParams->ld_datablock;
        pMat_arr += wrapupParams->ld_array;
    }

    BLAS_int_t * pInfo_db = wrapupDeps->dbInfo.ptr;
    BLAS_int_t * pInfo    = wrapupParams->info;
    *pInfo = *pInfo_db;

    ocrDbDestroy (wrapupDeps->dbMat.guid);       // Destroy datablock for the matrix.
    ocrDbDestroy (wrapupDeps->dbInfo.guid);      // Destroy datablock for the diagnostic info.

#if 0
    pMat_arr = wrapupParams->mat;
    for (i = 0; i < wrapupParams->n; i++) {
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
    for (i = 0; i < wrapupParams->n; i++) {
        for (j = 0; j < wrapupParams->n; j++) {
#if defined(BLAS_TYPE__single)
            printf ("0x%x ", *((u32*) (&(pMat_arr[j]))));
#elif defined(BLAS_TYPE__double)
            printf ("0x%lx ", *((u64*) (&(pMat_arr[j]))));
#elif defined(BLAS_TYPE__complex)
            printf ("(0x%x,0x%x) ", *((u32*) (&(pMat_arr[j].real))), *((u32*) (&(pMat_arr[j].imag))));
#else
            printf ("(0x%lx,0x%lx) ", *((u64*) (&(pMat_arr[j].real))), *((u64*) (&(pMat_arr[j].imag))));
#endif
        }
        pMat_arr += wrapupParams->ld_array;
        printf ("\n");
    }
#else
    pMat_arr = wrapupParams->mat;
#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define square 9
        BLAS_MATH_t expected[9][9] = {{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 4.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 4.0, 5.0, 1.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 1.0, 1.0},
                                      {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 1.0}};
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
#define square 3
        BLAS_MATH_t expected_upper[3][3] = {{{ 3.0, 0.0}, { 1.0, 1.0}, { 1.0,-1.0}},
                                            {{-1.1, 2.2}, { 4.0, 0.0}, { 2.0,-1.0}},
                                            {{ 3.3,-4.4}, {-5.5,-6.6}, { 6.0, 0.0}}};
        BLAS_MATH_t expected_lower[3][3] = {{{ 5.0, 0.0}, {-1.1, 2.2}, { 3.3,-4.4}},
                                            {{-1.0, 1.0}, { 7.0, 0.0}, { 5.5, 6.6}},
                                            {{ 2.0,-1.0}, { 1.0, 1.0}, { 8.0, 0.0}}};
        BLAS_MATH_t expected[3][3];
        {
            int iii,jjj;
            for (iii=0;iii<square;iii++) {
                for (jjj=0;jjj<square;jjj++) {
                    expected[iii][jjj] = (wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ? expected_upper[iii][jjj] : expected_lower[iii][jjj];
                }
            }
         }
#else
error: type not defined.
#endif
    u64 ii, jj;
    u64 errCnt = 0;
    BLAS_MATH_t zero;
    xSETc(zero, 0.0, 0.0);
    for (i = 0; i < wrapupParams->n; i+=square) {
        for (j = 0; j < wrapupParams->n; j+=square) {
            for (ii = 0; ii < square; ii++) {
                for (jj = 0; jj < square; jj++) {
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
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag);
#elif defined(BLAS_TYPE__complex_double)
                                printf ("pMat_arr[%ld][%ld] = {%f, %f} instead of {0.0 or -0.0, 0.0 or -0.0}\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].real,
                                    pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)].imag);
#else
error: type not defined.
#endif
                            }
                        }
                    } else {
#if defined(BLAS_TYPE__single)
                        if (xCMP_NE(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)], (wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ? expected[ii][jj] : expected[jj][ii])) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%x instead of %f / 0x%x\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u32*) &pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]),
                                    (double) ((wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ?  expected[ii][jj] :  expected[jj][ii]),
                                    *((u32*) ((wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ? &expected[ii][jj] : &expected[jj][ii])));
                            }
                        }
#elif defined(BLAS_TYPE__double)
                        if (xCMP_NE(pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)], (wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ? expected[ii][jj] : expected[jj][ii])) {
                            if (errCnt++ < 100) {
                                printf ("pMat_arr[%ld][%ld] = %f / 0x%lx instead of %f / 0x%lx\n",
                                    (u64) (i+ii), (u64) (j+jj),
                                    (double) pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)],
                                    *((u64*) &pMat_arr[(i+ii)*wrapupParams->ld_array+(j+jj)]),
                                    (double) ((wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ?  expected[ii][jj] :  expected[jj][ii]),
                                    *((u64*) ((wrapupParams->uplo[0] == 'U' || wrapupParams->uplo[0] == 'u') ? &expected[ii][jj] : &expected[jj][ii])));
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
                                    (double) *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj].real),
                                    (double) *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj].imag),
                                    (u32) expected[ii][jj].real,
                                    (u32) expected[ii][jj].imag);
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
                                    *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj].real),
                                    *((BLAS_MATH_COMPONENT_t *) &expected[ii][jj].imag),
                                    (u64) expected[ii][jj].real,
                                    (u64) expected[ii][jj].imag);
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

    printf ("Info: %ld\n", (u64) *pInfo);

    printf ("        %s exiting with normal done status.  Calling ocrShutdown (which should be moved elsewere)\n", STRINGIFY(NAME(potrf_wrapupTask))); fflush(stdout);
    ocrShutdown();  // TODO:  This should move out of here.  Surely the POTRF library function should NOT be pulling the plug!
    return NULL_GUID;
} // ?potrf_wrapupTask

