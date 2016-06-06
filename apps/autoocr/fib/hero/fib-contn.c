
#include "ocr.h"

#define ENABLE_EXTENSION_LEGACY
#include <extensions/ocr-legacy.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * First possibility: Use DU chains to launch and wait on futures
 *
 * Assumptions:
 *     - input parameters are all POD values that are copied
 *       in (ie: no pointers that are actually used
 *       for indirection)
 *     - output is only returned with a return statement and the
 *       returned value is also POD
 *     - functions have no side effects and can therefore be
 *       deferred until the use of their result is needed
 *
 * Idea behind transformation:
 *     - Transform calls into the equivalent of an asynchronous launch
 *       (spawn in Cilk parlance, create a future)
 *     - When the spawned computation's value is needed, block on the launch
 *       until the result comes back (sync in Cilk parlance, wait on future)
 * This does not create additional tasks and treats calls into an OCR
 * context and calls from the OCR context the same way
 *
 * launchFib is just to factorize code in this example. The compiler
 * would probably do all this inline but I didn't want to make it too ugly
 */
ocrGuid_t fib_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t launchFib(int arg) {
  /* Get the size of the parameter array that we need for this call */
  u32 paramCount = (sizeof(int) + sizeof(u64) - 1) / sizeof(u64) + 1;
  /* Create an array parameter (heap, stack, whatever) */
  char *startParams, *paramValues;
  startParams = paramValues = malloc(sizeof(u64)*paramCount);

  paramValues += sizeof(u64); // First parameter will be event to
  // satisfy with result. Assumes sizeof(ocrGuid_t) = sizeof(u64)
  /* Copy parameters in (some marshalling code) */
  (*(u32*)paramValues) = arg;
  paramValues += sizeof(u32);

  /* Create event to put result in ("future"). Do not use the
   * output event because that restricts tail recursion (not this
   * example) */
  ocrGuid_t returnEvent, templateGuid, edtGuid;
  ocrEventCreate(&returnEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

  /* Tell the EDT which event it needs to satisfy with its result */
  (*(ocrGuid_t*)startParams) = returnEvent;

  /* This could be factored out by the compiler probably. */
  ocrEdtTemplateCreate(&templateGuid, &fib_edt, paramCount, 0);

  /* Create the EDT task */
  ocrEdtCreate(&edtGuid, templateGuid, EDT_PARAM_DEF, (u64*)startParams,
	       EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

  ocrEdtTemplateDestroy(templateGuid);

  return returnEvent;
}

ocrGuid_t fib_cont(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
  /* Get the event to satisfy on return (by convention
   * always paramv[0])
   */
  ocrGuid_t outEvt = (ocrGuid_t)paramv[0];

  u32 in1 = *((u32*)depv[0].ptr);
  u32 in2 = *((u32*)depv[1].ptr);

  /* Clean up inputs */
  ocrDbDestroy(depv[0].guid);
  ocrDbDestroy(depv[1].guid);
  ocrEventDestroy((ocrGuid_t)(paramv[1]));
  ocrEventDestroy((ocrGuid_t)(paramv[2]));

  /* Return code is unchanged */
  ocrGuid_t dbGuid;
  int* dbPtr;
  ocrDbCreate(&dbGuid, (void**)(&dbPtr), sizeof(int), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  *dbPtr = in1 + in2;
  ocrDbRelease(dbGuid);
  ocrEventSatisfy(outEvt, dbGuid);
  return NULL_GUID;
}


ocrGuid_t fib_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  /* Get event to satisfy */
  ocrGuid_t outEvt = (ocrGuid_t)paramv[0];

  /* Unmarshall parameters */
  int n = (u32)paramv[1];

  if(n > 2) {
    /* Launch both sub-fibs, and create a continuation
     * to fib which will perform the remaining computation
     * (here, just an add).
     */
    ocrGuid_t fib1Evt = launchFib(n-1);
    ocrGuid_t fib2Evt = launchFib(n-2);

    ocrGuid_t templateGuid, edtGuid;
    /* Assumes compiler hoists all that it needs and can safely do */
    ocrGuid_t depv[2] = {fib1Evt, fib2Evt};
    u64 paramv[3] = {outEvt, fib1Evt, fib2Evt};
    ocrEdtTemplateCreate(&templateGuid, &fib_cont, /*paramc*/3, /*paramv*/2);
    ocrEdtCreate(&edtGuid, templateGuid, EDT_PARAM_DEF, paramv,
                 EDT_PARAM_DEF, depv, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    return NULL_GUID;

  } else {
    /* Return statements satisfy the outEvt */
    ocrGuid_t dbGuid;
    int* dbPtr;
    ocrDbCreate(&dbGuid, (void**)(&dbPtr), sizeof(int), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    *dbPtr = n;
    ocrDbRelease(dbGuid);
    ocrEventSatisfy(outEvt, dbGuid);
    return NULL_GUID;
  }
}

int main(int argc, const char* argv[]) {
  int initN = atoi(argv[1]);

  ocrConfig_t cfg;
  ocrGuid_t legacyCtx;


  cfg.userArgc = argc;
  cfg.userArgv = (char **)argv;
  cfg.iniFile = getenv("OCR_CONFIG");

  ocrLegacyInit(&legacyCtx, &cfg);

  /* Launch the EDT and immediately wait on the result since
   * the printf is the user and is right after it */
  ocrGuid_t fibEvt = launchFib(initN);
  ocrGuid_t fibDbGuid;
  void* fibDbPtr;
  ocrLegacyBlockProgress(fibEvt, &fibDbGuid, &fibDbPtr, NULL, LEGACY_PROP_NONE);
  int result = *((u32 *)(fibDbPtr));
  printf("Result: fib(%d) = %d\n", initN, result);
  /* Clean up after use */
  ocrEventDestroy(fibEvt);
  ocrDbDestroy(fibDbGuid);
  ocrLegacyFinalize(legacyCtx, false);

  return 0;
}
