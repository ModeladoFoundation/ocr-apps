#include <stdio.h>
#include <stdlib.h>

#define __task __attribute__((ocrtask))

#define N 1

/**
 * Initial code: simple Fibonacci
 */
#include <stdlib.h>
#include "ocr.h"

#define ENABLE_EXTENSION_LEGACY
#include <extensions/ocr-legacy.h>

#if 0
#else
typedef struct fibParams {
		ocrGuid_t event;
		int n;
		int pad;
	} fibParams_t;
#endif

ocrGuid_t fib_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

ocrGuid_t fib_template() {
#if 0
    static ocrGuid_t templateGuid = NULL_GUID;

    if (templateGuid == NULL_GUID)
      ocrEdtTemplateCreate(&templateGuid, &fib_edt, 2, 0);
#else
    static ocrGuid_t templateGuid;
      ocrEdtTemplateCreate(&templateGuid, &fib_edt, sizeof(fibParams_t)/sizeof(u64), 0);
#endif
    return templateGuid;
}

int fib_wait( ocrGuid_t evt ) {
    ocrGuid_t dbGuid;
    void * dbPtr;
    ocrLegacyBlockProgress(evt, &dbGuid, &dbPtr, NULL, LEGACY_PROP_NONE);

    int ret = *(int *)dbPtr;
    ocrEventDestroy(evt);
    return ret;
}

void fib_finish( int retval, ocrGuid_t outEvt ) {
    ocrGuid_t dbGuid;
    int * dbPtr;
    ocrDbCreate(&dbGuid, (void**)(&dbPtr), sizeof(int),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    *dbPtr = retval;
    ocrDbRelease(dbGuid);
    ocrEventSatisfy(outEvt, dbGuid);
}

ocrGuid_t fib_launch(int n) {
    //
    // Create a parameter and dependency array
    //
#if 0
    u64 *ocrparams = (u64 *) alloca(sizeof(u64)*2);
#else
    fibParams_t fibParams;
#endif
    ocrGuid_t *ocrdeps = (ocrGuid_t *) NULL;
    //
    // Create an event to synchronize completion with
    //
    ocrGuid_t returnEvent;
    ocrEventCreate(&returnEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
#if 0
    *(ocrGuid_t *) ocrparams = returnEvent;
#else
    fibParams.event = returnEvent;
#endif
    //
    // Marshall parameters
    //
#if 0
    *(int *)(ocrparams + 1) = n;
#else
    fibParams.n = n;
    fibParams.pad = 0;
#endif


    ocrGuid_t edtGuid;

    //
    // Create the EDT task
    //
#if 0
    ocrEdtCreate( &edtGuid, fib_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_GUID, NULL_GUID);
#else
    ocrEdtCreate( &edtGuid, fib_template(), EDT_PARAM_DEF,
                  (u64 *)&fibParams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);
#endif

    return returnEvent;
}

ocrGuid_t fib_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    /* Get event to satisfy */
#if 0
    ocrGuid_t ocrOutEvt = (ocrGuid_t)ocrparams[0];
    int n = *(int *)(ocrparams + 1);
#else
    fibParams_t *fibParams = (fibParams_t *)ocrparams;
    ocrGuid_t ocrOutEvt = fibParams->event;
    int n = fibParams->n;
#endif
    if (n > 1) {
        ocrGuid_t ocrCallEvt0 = fib_launch(n - 1);
        ocrGuid_t ocrCallEvt1 = fib_launch(n - 2);
        fib_finish(fib_wait(ocrCallEvt0) + fib_wait(ocrCallEvt1), ocrOutEvt);
        return NULL_GUID;
    } else {
        fib_finish(n, ocrOutEvt);
        return NULL_GUID;
    }
}


int ocrapp_main(int argc, char ** argv) {
    int initN = atoi(argv[1]);
    ocrGuid_t ocrCallEvt0 = fib_launch(initN);
    int result = fib_wait(ocrCallEvt0);
    printf("Result: fib(%d) = %d\n", initN, result);
    return 0;
}

int main( int argc, char ** argv ) {
    ocrConfig_t cfg;
    ocrGuid_t legacyCtx;

    cfg.userArgc = argc;
    cfg.userArgv = (char **) argv;
    cfg.iniFile = getenv("OCR_CONFIG");

    ocrLegacyInit(&legacyCtx, &cfg);
    int retval = ocrapp_main( argc, argv );
    ocrShutdown();
    return retval;
}


