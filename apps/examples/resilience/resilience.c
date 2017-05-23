/*
 *  This file is subject to the license agreement located in the file LICENSE
 *  and cannot be distributed without it. This notice cannot be
 *  removed or modified.
 */
#include "ocr.h"

/* Example of an iterative 1D Stencil + "fork-join" pattern in OCR
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
#define NUM_ITERS 10
#define NUM_LOCAL_EDTS 5
#define INJECT_FAULT 0

#define NUM_DIMS 1
#define XDIM NUM_RANKS
#define ITER_SPACE XDIM
#define USER_KEY(i, x) ((i * ITER_SPACE) + x)

#define GUID_PARAM (sizeof(ocrGuid_t)/sizeof(u64))
#define NUM_PARAMS (1/*iter value*/ + NUM_DIMS/*stencil dimensions*/ + (GUID_PARAM * 2)/*shutdown guid + cleanup DB*/)
#define NUM_DEPS 3 /*3-point stencil*/

//Dummy function for local computation EDT on every iteration in each rank
ocrGuid_t localFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("[Node %lu]: Hello from local EDT (%lu, %lu, %lu)\n", ocrGetLocation(), paramv[0], paramv[1], paramv[2]);
    return NULL_GUID;
}

//Function for creating local computation in every iteration in each rank
static void doLocalComputation(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[], ocrGuid_t db) {
    u64 i;
    u64 iter = paramv[0];
    u64 x = paramv[1];

    //Create the local computation EDTs
    ocrGuid_t localEdt_template;
    ocrEdtTemplateCreate(&localEdt_template, localFunc, 3, 0);
    u64 paramsLocal[3];
    paramsLocal[0] = iter;
    paramsLocal[1] = x;
    for (i = 0; i < NUM_LOCAL_EDTS; i++) {
        ocrGuid_t localEdt;
        paramsLocal[2] = i;
        ocrEdtCreate(&localEdt, localEdt_template, 3, paramsLocal, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    }
    return;
}

//Top-level function that runs on every iteration in each rank
ocrGuid_t resilientFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    u64 iter = paramv[0];
    u64 x = paramv[1];

    PRINTF("[Node %lu]: Hello from resilient EDT (%lu, %lu)\n", ocrGetLocation(), iter, x);

    //Cleanup past iteration (iter - 2) DB and Events
    ocrGuid_t *guidParamv = (ocrGuid_t*)&(paramv[1 + NUM_DIMS]);
    ocrDbDestroy(guidParamv[1]);
    if (iter > 2) {
        ocrGuid_t pEvt;
        ocrGuidTableRemove(USER_KEY((iter - 1), x), &pEvt);
        ocrEventDestroy(pEvt);
    }

    //Verify the validity of the input DBs from past iteration (iter - 1)
    if ((*((u64*)depv[0].ptr) != ((iter * NUM_RANKS) + x)) ||
       ((x > 0) && (*((u64*)depv[1].ptr) != ((iter * NUM_RANKS) + (x - 1)))) ||
       ((x < (NUM_RANKS - 1)) && (*((u64*)depv[2].ptr) != ((iter * NUM_RANKS) + (x + 1))))) {
        PRINTF("[Node %lu]: Data corruption detected in resilient EDT (%lu, %lu)\n", ocrGetLocation(), iter, x);
        ASSERT(0);
        return NULL_GUID;
    }

#if INJECT_FAULT
    //Fault injection
    if (iter == NUM_ITERS/2 && x == 0) {
        PRINTF("[Node %lu]: Injecting fault from resilient EDT (%lu, %lu)\n", ocrGetLocation(), iter, x);
        ocrNodeFailure();
    }
#endif

    //Create a resilient output DB of this iteration
    ocrGuid_t db = NULL_GUID;
    void *ptr = NULL;
    ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
    *((u64*)ptr) = ((iter + 1) * NUM_RANKS) + x;

    //Perform local computation for output DB of this iteration
    doLocalComputation(paramc, paramv, depc, depv, db);

    //Satisfy event to transmit DB from this iteration to next iteration (iter + 1) EDT neighbors (x-1, x+1)
    if ((iter + 1) <= NUM_ITERS) {
        ocrGuid_t oEvt;
        ocrGuidTableGet(USER_KEY((iter + 1), x), &oEvt);
        ocrEventSatisfy(oEvt, db);
    }

    //Check for termination condition on this iteration
    if (iter == NUM_ITERS) {
        ocrGuid_t shutdownEdt = guidParamv[0];
        ocrGuid_t curOutputEvent;
        ocrGetOutputEvent(&curOutputEvent);
        ocrAddDependence(curOutputEvent, shutdownEdt, x, DB_MODE_CONST);
        ocrAddDependence(depv[0].guid, shutdownEdt, (NUM_RANKS + x), DB_MODE_CONST);
        ocrAddDependence(db, shutdownEdt, ((NUM_RANKS * 2) + x), DB_MODE_CONST);
        return NULL_GUID;
    }

    //Schedule EDT for next iteration (iter + 1)
    ocrGuid_t rankEdt_template;
    ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, NUM_PARAMS, NUM_DEPS);

    //Setup params
    u64 params[NUM_PARAMS];
    for (i = 0; i < NUM_PARAMS; i++) params[i] = paramv[i];
    params[0] = iter + 1;
    ocrGuid_t *guidParams = (ocrGuid_t*)&(params[1 + NUM_DIMS]);
    guidParams[1] = depv[0].guid;

    //Setup dependences
    ocrGuid_t deps[NUM_DEPS];
    //Dependence input from current rank
    deps[0] = db; //We can directly use DB instead of using event

    //Dependence input from left rank
    if (x > 0) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x - 1)), &deps[1]);
    } else {
        deps[1] = NULL_GUID;
    }

    //Dependence input from right rank
    if (x < (NUM_RANKS - 1)) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x + 1)), &deps[2]);
    } else {
        deps[2] = NULL_GUID;
    }

    //Create EDT
    ocrGuid_t rankEdt, outputEvent;
    ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);

    //Create event for future iteration (iter + 2) and associate with user-defined key
    if ((iter + 2) <= NUM_ITERS) {
        ocrGuid_t evt;
        ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        ocrGuidTablePut(USER_KEY((iter + 2), x), evt);
    }
    return NULL_GUID;
}

//Sync EDT used to call shutdown
ocrGuid_t shutdownFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("[Node %lu]: Hello from shutdownEdt\n", ocrGetLocation());
    //Cleanup past iteration events (NUM_ITERS)
    for (i = 0; i < NUM_RANKS; i++) {
        if (NUM_ITERS > 1) {
            ocrGuid_t pEvt;
            ocrGuidTableRemove(USER_KEY(NUM_ITERS, i), &pEvt);
            ocrEventDestroy(pEvt);
        }
        ocrDbDestroy(depv[NUM_RANKS + i].guid);
        ocrDbDestroy(depv[(NUM_RANKS * 2) + i].guid);
    }
    ocrShutdown();
    return NULL_GUID;
}

//EDT to start the program
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("[Node %lu]: Starting mainEdt\n", ocrGetLocation());

    //Create the shutdown EDT
    ocrGuid_t shutdown_template, shutdownEdt;
    ocrEdtTemplateCreate(&shutdown_template, shutdownFunc, 0, (NUM_RANKS * 3));
    ocrEdtCreate(&shutdownEdt, shutdown_template, 0, NULL, (NUM_RANKS * 3), NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    //Create the rank EDT template
    ocrGuid_t rankEdt_template;
    ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, NUM_PARAMS, NUM_DEPS);

    //Create the paramv
    u64 params[NUM_PARAMS];
    params[0] = 1; //start iteration at 1
    ocrGuid_t *guidParams = (ocrGuid_t*)&(params[1 + NUM_DIMS]);
    guidParams[0] = shutdownEdt;
    guidParams[1] = NULL_GUID;

    //Create the initial set of DBs and Events per rank
    ocrGuid_t dbArray[NUM_RANKS];
    for (i = 0; i < NUM_RANKS; i++) {
        //Create a resilient DB for future iteration (iter + 1)
        ocrGuid_t db = NULL_GUID;
        void *ptr = NULL;
        ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
        //PRINTF("[Node %lu]: DB created %lu from EDT (%lu, %lu)\n", ocrGetLocation(), db.guid, 0, i);
        *((u64*)ptr) = NUM_RANKS + i;
        ocrDbRelease(db);
        dbArray[i] = db;

        //Create event for future iteration (iter + 2) and associate with user-defined key
        if (NUM_ITERS > 1) {
            ocrGuid_t evt;
            ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
            u64 k = USER_KEY(2, i);
            ocrGuidTablePut(k, evt);
        }
    }

    //Create the initial rank EDTs (start iteration at 1)
    for (i = 0; i < NUM_RANKS; i++) {
        params[1] = i; //x-value

        ocrGuid_t deps[NUM_DEPS];
        deps[0] = dbArray[i];
        deps[1] = (i > 0) ? dbArray[i-1] : NULL_GUID;
        deps[2] = (i < (NUM_RANKS - 1)) ? dbArray[i+1] : NULL_GUID;

        ocrGuid_t rankEdt, outputEvent;
        ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);
    }

    return NULL_GUID;
}
