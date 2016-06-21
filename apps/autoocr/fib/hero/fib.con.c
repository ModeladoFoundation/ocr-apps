#include <stdio.h>
#include <stdlib.h>
/*
 *  continuation version of fib.ocr.c
 * based on ret_val.con.c
 * 1. removed main()
 * 2. changed ocrapp_main to mainEdt()
 *    returns the value of fib(n)
 * 3. changed finish() into a task with two preslots
 * 4. changed fib_edt() to use a switch and state structure
 *    a. case 0  : normal fib()
 *    b. case 1  :  continuation from finish()
 * 5. added done() which is triggered by the first event returned
 *    from fib_launch(). It prints the fib number and exits
 */

#define __task __attribute__((ocrtask))

#define N 1

/**
 * Initial code: simple Fibonacci
 */
#include <stdlib.h>
#include "ocr.h"

ocrGuid_t fib_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

ocrGuid_t finish_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

ocrGuid_t done_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

/*
 * the termination event triggers here
 */
ocrGuid_t done_template() {
    static ocrGuid_t doneGuid;
    ocrEdtTemplateCreate(&doneGuid, &done_edt, 1, 1);
    return doneGuid;
}

ocrGuid_t done_launch(int n, ocrGuid_t returnEvent) {
    ocrGuid_t done_edt;
    ocrGuid_t ocrdeps[1];
    u64 ocrparams[1];
    ocrdeps[0] = returnEvent;
    *(u64 *)(ocrparams) = n;

    ocrEdtCreate( &done_edt, done_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return NULL_GUID;
}
/*
 * triggers when fib_edt(n-1) and fib_edt(n-2) finish
 * sums fib(n-1)+ fib(n-2) and does a continuation call
 * to fib_edt ith fib(n).
 */
ocrGuid_t finish_template() {
    static ocrGuid_t templateGuid;

    ocrEdtTemplateCreate(&templateGuid, &finish_edt, 1, 2);
    return templateGuid;
}

ocrGuid_t finish_launch(ocrGuid_t evt1, ocrGuid_t evt2, ocrGuid_t returnEvent) {
    //
    // Create a parameter and dependency array
    //
    u64 ocrparams[1];
    ocrGuid_t ocrdeps[2];
    //
    // Marshall parameters
    //
    *(ocrGuid_t *)(ocrparams) = returnEvent;
    ocrdeps[0] = evt1;
    ocrdeps[1] = evt2;

    ocrGuid_t finish_edt;

    //
    // Create the EDT task
    //
    ocrEdtCreate( &finish_edt, finish_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return returnEvent;
}
ocrGuid_t fib_template() {
    static ocrGuid_t templateGuid;

    ocrEdtTemplateCreate(&templateGuid, &fib_edt, 2, 1);
    return templateGuid;
}

ocrGuid_t fib_launch(int n, ocrGuid_t resultDb) {
    //
    // Create a parameter and dependency array
    //
    u64 ocrparams[2];
    ocrGuid_t ocrdeps[1];
    //
    // Create an event to synchronize completion with
    //
    ocrGuid_t returnEvent;
    ocrEventCreate(&returnEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
    *(ocrGuid_t *)(ocrparams) = returnEvent;
    ocrdeps[0] = resultDb;
    //
    // Marshall parameters
    //
    *(u64 *)(ocrparams + 1) = n;

    ocrGuid_t edtGuid;

    //
    // Create the EDT task
    //
    ocrEdtCreate( &edtGuid, fib_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return returnEvent;
}

ocrGuid_t fib_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    /* Get event to satisfy */
    ocrGuid_t ocrOutEvt = *(ocrGuid_t *)ocrparams;
    ocrGuid_t ocrDb = ocrdepv[0].guid;
    int n = *(int *)(ocrparams + 1);

    if (n > N) {
        ocrGuid_t ocrCallEvt0 = fib_launch(n - 1, ocrDb);
        ocrGuid_t ocrCallEvt1 = fib_launch(n - 2, ocrDb);
        finish_launch(ocrCallEvt0, ocrCallEvt1, ocrOutEvt);
        return NULL_GUID;
    } else {
        ocrGuid_t dbGuid;
        int * dbPtr;
        ocrDbCreate(&dbGuid, (void**)(&dbPtr), sizeof(int),
            DB_PROP_NONE, NULL_HINT, NO_ALLOC);
        *dbPtr = n;
        ocrDbRelease(dbGuid);
        ocrEventSatisfy(ocrOutEvt, dbGuid);
        return dbGuid;
    }
}

/*
 * has two pre-slots which contain
 * fib(n-1) and fib(n-2) values. sum them and
 * return them via continuation to fib_edt()
 */
ocrGuid_t finish_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    /* Get event to satisfy */
    //ocrGuid_t ocrOutEvt = *(ocrGuid_t *)ocrparams;
    int fibn_1 = *(int *)(ocrdepv[0].ptr);
    int fibn_2 = *(int *)(ocrdepv[1].ptr);
    int retval = fibn_1 + fibn_2;
    ocrGuid_t outEvt = *(ocrGuid_t*)(ocrparams);

    int * dbPtr;
    ocrGuid_t dbGuid;
    ocrDbCreate(&dbGuid, (void**)(&dbPtr), sizeof(int),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    *dbPtr = retval;
    ocrDbRelease(dbGuid);
    ocrEventSatisfy(outEvt, dbGuid);
    return dbGuid;
}

ocrGuid_t mainEdt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {

/*
 * OCR argument handling
 * arg0 is the program name
 */
    int numArgs = getArgc(ocrdepv[0].ptr);
    if (numArgs != 2 ) {
        ocrShutdown();
        return NULL_GUID;
    }
    char *arg1 = getArgv(ocrdepv[0].ptr, 1);
    int initN=atoi(arg1);
    // placeholder, not used for this call
    ocrGuid_t dbGuid = {0};

    ocrGuid_t finalEvent = fib_launch(initN, dbGuid);
    done_launch(initN, finalEvent);
    return NULL_GUID;
}

ocrGuid_t done_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    int result = *(int *)(ocrdepv[0].ptr);
    int initN = *(int *)(ocrparams);

    printf("Result: fib(%d) = %d\n", initN, result);
    ocrShutdown();
    return NULL_GUID;
}
