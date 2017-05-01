/*
 *  This file is subject to the license agreement located in the file LICENSE
 *  and cannot be distributed without it. This notice cannot be
 *  removed or modified.
 */
#include "ocr.h"

/* Example of an iterative 2D "fork-join" pattern in OCR
 *
 * Implements the following dependence graph:
 *
 *             mainEdt
 *          /           \
 *         /             \
 *        /               \
 *    resEdt[0][0]        resEdt[0][1]      ...    resEdt[0][ranks-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *    resEdt[1][0]        resEdt[1][1]      ...    resEdt[1][ranks-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *                 ...
 *                 ...
 *                 ...
 *
 * resEdt[iters-1][0] resEdt[iters-1][1]    ...    resEdt[iters-1][ranks-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *        \                 /
 *         \               /
 *          \             /
 *            shutdownEdt
 *
 */

#define NUM_RANKS 4
#define NUM_ITERS 20
#define NUM_LOCAL_EDTS 10
#define INJECT_FAULT 0

ocrGuid_t localFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("[Rank: %lu] Hello from local EDT (%d, %d, %d)\n", ocrGetLocation(), paramv[0], paramv[1], paramv[2]);
    return NULL_GUID;
}

ocrGuid_t resilientFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("[Rank: %lu] Hello from resilient EDT (%d, %d)\n", ocrGetLocation(), paramv[0], paramv[1]);

#if INJECT_FAULT
    //Fault injection
    if (paramv[0] == NUM_ITERS/2 && paramv[1] == 0) {
        PRINTF("[Rank: %lu] Injecting fault from resilient EDT (%d, %d)\n", ocrGetLocation(), paramv[0], paramv[1]);
        ocrNodeFailure();
    }
#endif

    //Create the local EDTs
    ocrGuid_t localEdt_template;
    ocrEdtTemplateCreate(&localEdt_template, localFunc, 3, 0);
    u64 paramsLocal[3];
    paramsLocal[0] = paramv[0];
    paramsLocal[1] = paramv[1];
    for (i = 0; i < NUM_LOCAL_EDTS; i++) {
        ocrGuid_t localEdt;
        paramsLocal[2] = i;
        ocrEdtCreate(&localEdt, localEdt_template, 3, paramsLocal, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    }

    u64 nextIter = paramv[0] + 1;
    if (nextIter == NUM_ITERS) {
        //Termination condition
        ocrGuid_t shutdownEdt = *((ocrGuid_t*)(&(paramv[2])));
        ocrGuid_t curOutputEvent;
        ocrGetOutputEvent(&curOutputEvent);
        ocrAddDependence(curOutputEvent, shutdownEdt, paramv[1], DB_MODE_CONST);
    } else {
        //Schedule EDT for next iteration
        ocrGuid_t rankEdt_template;
        u64 numParams = 2 + sizeof(ocrGuid_t)/sizeof(u64);
        ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, numParams, 1);

        u64 paramsIter[numParams];
        for (i = 0; i < numParams; i++) paramsIter[i] = paramv[i];
        paramsIter[0] += 1;
        ocrGuid_t rankEdt, outputResEvent;
        ocrEdtCreate(&rankEdt, rankEdt_template, numParams, paramsIter, 1, NULL, EDT_PROP_RESILIENT, NULL_HINT, &outputResEvent);

        ocrGuid_t curOutputEvent;
        ocrGetOutputEvent(&curOutputEvent);
        ocrAddDependence(curOutputEvent, rankEdt, 0, DB_MODE_CONST);
    }
    return NULL_GUID;
}

ocrGuid_t shutdownFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("[Rank: %lu] Hello from shutdownEdt\n", ocrGetLocation());
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("[Rank: %lu] Starting mainEdt\n", ocrGetLocation());

    //Create the shutdown EDT
    ocrGuid_t shutdown_template, shutdownEdt;
    ocrEdtTemplateCreate(&shutdown_template, shutdownFunc, 0, NUM_RANKS);
    ocrEdtCreate(&shutdownEdt, shutdown_template, 0, NULL, NUM_RANKS, NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    //Create the rank EDT template
    ocrGuid_t rankEdt_template;
    u64 numParams = 2 + sizeof(ocrGuid_t)/sizeof(u64);
    ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, numParams, 1);

    //Create the paramv
    u64 params[numParams];
    params[0] = 0; //row
    params[1] = 0; //col
    *((ocrGuid_t*)(&(params[2]))) = shutdownEdt;

    //Create the initial depv
    ocrGuid_t depNull = NULL_GUID;

    //Create the initial rank EDTs
    for (i = 0; i < NUM_RANKS; i++) {
        ocrGuid_t rankEdt, outputResEvent;
        params[1] = i; //col
        ocrEdtCreate(&rankEdt, rankEdt_template, numParams, params, 1, &depNull, EDT_PROP_RESILIENT, NULL_HINT, &outputResEvent);
    }

    return NULL_GUID;
}
