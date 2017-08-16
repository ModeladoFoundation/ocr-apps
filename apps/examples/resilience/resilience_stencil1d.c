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
 *    resEdt[1][0]        resEdt[1][1]      ...  resEdt[1][XDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *    resEdt[2][0]        resEdt[2][1]      ...  resEdt[2][XDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *                 ...
 *                 ...
 *                 ...
 *
 * resEdt[ITERS][0]       resEdt[ITERS][1]  ...  resEdt[ITERS][XDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *        \                 /
 *         \               /
 *          \             /
 *            shutdownEdt
 *
 * NOTE: resEdt[<iter-value>][<x-value>]
 *
 */

//Configurable params
#define N       4
#define XDIM    N   //Number of tiles or blocks in X-dimension
#define ITERS   N   //Number of iterations in application

//Derived params (DO NOT CHANGE)
#define NUM_DIMS 1 //(x)
#define NUM_ITERS (ITERS + 1)
#define ITER_SPACE XDIM
#define ITER_INDEX(x) x

#define USER_KEY(i, x) ((i * ITER_SPACE) + x)

#define GUID_PARAM (sizeof(ocrGuid_t)/sizeof(u64))
#define NUM_PARAMS (1/*iter value*/ + NUM_DIMS/*coordinate value*/ + (GUID_PARAM * 2)/*shutdown guid + cleanup DB*/)
#define NUM_DEPS 3 /*3-point stencil*/

#define NUM_LOCAL_EDTS 2
#define INJECT_FAULT 0

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

    if (iter < NUM_ITERS) {
        PRINTF("[Node %lu]: Hello from resilient EDT (%lu, %lu)\n", ocrGetLocation(), iter, x);
    } else if (iter == NUM_ITERS) {
        PRINTF("[Node %lu]: Hello from epilogue EDT (%lu)\n", ocrGetLocation(), x);
    } else {
        PRINTF("[Node %lu]: Hello from cleanup EDT (%lu)\n", ocrGetLocation(), x);
    }

    //Cleanup past iteration (iter - 2) DB and (iter - 1) Events
    ocrGuid_t *guidParamv = (ocrGuid_t*)&(paramv[1 + NUM_DIMS]);
    ocrDbDestroy(guidParamv[1]);
    if (iter > 2) {
        ocrGuid_t pEvt;
        ocrGuidTableRemove(USER_KEY((iter - 2), x), &pEvt);
        ocrEventDestroy(pEvt);
    }

    //Check for termination condition on this iteration
    if (iter == (NUM_ITERS + 1)) {
        ocrGuid_t shutdownEdt = guidParamv[0];
        ocrAddDependence(NULL_GUID, shutdownEdt, x, DB_MODE_NULL);
        return NULL_GUID;
    }

    //Verify the validity of the input DBs to this iteration (iter)
    if ((*((u64*)depv[0].ptr) != USER_KEY(iter, x)) ||
       ((x > 0) && (*((u64*)depv[1].ptr) != USER_KEY(iter, (x - 1)))) ||
       ((x < (XDIM - 1)) && (*((u64*)depv[2].ptr) != USER_KEY(iter, (x + 1))))) {
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

    ocrGuid_t db = NULL_GUID;

    if (iter < NUM_ITERS) {
        //Create a resilient output DB of this iteration
        void *ptr = NULL;
        ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
        *((u64*)ptr) = USER_KEY((iter + 1), x);

        //Perform local computation for output DB of this iteration
        doLocalComputation(paramc, paramv, depc, depv, db);
    }

    //Satisfy event to transmit DB from this iteration to next iteration (iter + 1) EDT neighbors (x-1, x+1)
    ocrGuid_t oEvt;
    ocrGuidTableGet(USER_KEY((iter + 1), x), &oEvt);
    ocrEventSatisfy(oEvt, db);

    //Schedule EDT for next iteration (iter + 1)
    ocrGuid_t rankEdt_template;
    ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, NUM_PARAMS, NUM_DEPS);

    //Setup params
    u64 params[NUM_PARAMS];
    for (i = 0; i < NUM_PARAMS; i++) params[i] = paramv[i];
    params[0] = iter + 1;
    ocrGuid_t *guidParams = (ocrGuid_t*)&(params[1 + NUM_DIMS]);
    guidParams[1] = depv[0].guid; //DB to destroy

    //Setup dependences
    ocrGuid_t deps[NUM_DEPS];
    //Dependence input from current rank
    deps[0] = db; //We can directly use DB instead of using event

    //Dependence input from left
    if (x > 0) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x - 1)), &deps[1]);
    } else {
        deps[1] = NULL_GUID;
    }

    //Dependence input from right
    if (x < (XDIM - 1)) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x + 1)), &deps[2]);
    } else {
        deps[2] = NULL_GUID;
    }

    //Create EDT
    ocrGuid_t rankEdt, outputEvent;
    ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);

    //Create event for future iteration (iter + 2) and associate with user-defined key
    if ((iter + 2) <= (NUM_ITERS + 1)) {
        ocrGuid_t evt;
        ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG | EVT_PROP_RESILIENT);
        ocrGuidTablePut(USER_KEY((iter + 2), x), evt);
    }
    return NULL_GUID;
}

//Sync EDT used to call shutdown
ocrGuid_t shutdownFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("[Node %lu]: Hello from shutdownEdt\n", ocrGetLocation());
    //Cleanup past iteration events (NUM_ITERS)
    for (i = 0; i < XDIM; i++) {
        if (NUM_ITERS > 1) {
            ocrGuid_t pEvt;
            ocrGuidTableRemove(USER_KEY((NUM_ITERS + 1), i), &pEvt);
            ocrEventDestroy(pEvt);
        }
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
    ocrEdtTemplateCreate(&shutdown_template, shutdownFunc, 0, ITER_SPACE);
    ocrEdtCreate(&shutdownEdt, shutdown_template, 0, NULL, ITER_SPACE, NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    //Create the rank EDT template
    ocrGuid_t rankEdt_template;
    ocrEdtTemplateCreate(&rankEdt_template, resilientFunc, NUM_PARAMS, NUM_DEPS);

    //Create the paramv
    u64 params[NUM_PARAMS];
    params[0] = 1; //start iteration at 1
    ocrGuid_t *guidParams = (ocrGuid_t*)&(params[1 + NUM_DIMS]);
    guidParams[0] = shutdownEdt;
    guidParams[1] = NULL_GUID; //DB to destroy

    //Create the initial set of DBs and Events per rank
    ocrGuid_t dbArray[ITER_SPACE];
    for (i = 0; i < XDIM; i++) {
        //Create a resilient DB for future iteration (iter + 1)
        ocrGuid_t db = NULL_GUID;
        void *ptr = NULL;
        ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
        //PRINTF("[Node %lu]: DB created %lu from EDT (%lu, %lu)\n", ocrGetLocation(), db.guid, 0, i);
        *((u64*)ptr) = USER_KEY(1, i);
        ocrDbRelease(db);
        dbArray[i] = db;

        //Create event for future iteration (iter + 2) and associate with user-defined key
        if (NUM_ITERS > 1) {
            ocrGuid_t evt;
            ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG | EVT_PROP_RESILIENT);
            u64 key = USER_KEY(2, i);
            ocrGuidTablePut(key, evt);
        }
    }

    //Create the initial rank EDTs (start iteration at 1)
    for (i = 0; i < XDIM; i++) {
        params[1] = i; //x-value

        ocrGuid_t deps[NUM_DEPS];
        deps[0] = dbArray[i];
        deps[1] = (i > 0) ? dbArray[i-1] : NULL_GUID;
        deps[2] = (i < (XDIM - 1)) ? dbArray[i+1] : NULL_GUID;

        ocrGuid_t rankEdt, outputEvent;
        ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);
    }

    return NULL_GUID;
}
