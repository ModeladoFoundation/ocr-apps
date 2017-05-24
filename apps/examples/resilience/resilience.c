/*
 *  This file is subject to the license agreement located in the file LICENSE
 *  and cannot be distributed without it. This notice cannot be
 *  removed or modified.
 */
#include "ocr.h"

/* Example of an iterative 2D Stencil + "fork-join" pattern in OCR
 *
 * Implements the following dependence graph:
 *
 *             mainEdt
 *          /           \
 *         /             \
 *        /               \
 *    resEdt[1][0][0]     resEdt[1][0][1]     ... resEdt[1][XDIM-1][YDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *    resEdt[2][0][0]     resEdt[2][0][1]     ... resEdt[2][XDIM-1][YDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *                 ...
 *                 ...
 *                 ...
 *
 * resEdt[ITERS][0][0]    resEdt[ITERS][0][1] ... resEdt[ITERS][XDIM-1][YDIM-1]
 *       / | \              / | \
 * locEdt ... locEdt  locEdt ... locEdt
 *       \ | /              \ | /
 *        \                 /
 *         \               /
 *          \             /
 *            shutdownEdt
 *
 * NOTE: resEdt[<iter-value>][<x-value>][<y-value>]
 *
 */

//Configurable params
#define N       4
#define XDIM    N   //Number of tiles or blocks in X-dimension
#define YDIM    N   //Number of tiles or blocks on Y-dimension
#define ITERS   N   //Number of iterations in application

//Derived params (DO NOT CHANGE)
#define NUM_DIMS 2 //(x, y)
#define NUM_ITERS (ITERS + 1)
#define ITER_SPACE (XDIM * YDIM)
#define ITER_INDEX(x, y) ((x * YDIM) + y)

#define USER_KEY(i, x, y) ((i * ITER_SPACE) + ITER_INDEX(x, y))

#define GUID_PARAM (sizeof(ocrGuid_t)/sizeof(u64))
#define NUM_PARAMS (1/*iter value*/ + NUM_DIMS/*coordinate value*/ + (GUID_PARAM * 2)/*shutdown guid + cleanup DB*/)
#define NUM_DEPS 5 /*5-point stencil*/

#define NUM_LOCAL_EDTS 2
#define INJECT_FAULT 0

//Dummy function for local computation EDT on every iteration in each rank
ocrGuid_t localFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("[Node %lu]: Hello from local EDT (%lu, %lu, %lu, %lu)\n", ocrGetLocation(), paramv[0], paramv[1], paramv[2], paramv[3]);
    return NULL_GUID;
}

//Function for creating local computation in every iteration in each rank
static void doLocalComputation(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[], ocrGuid_t db) {
    u64 i;
    u64 iter = paramv[0];
    u64 x = paramv[1];
    u64 y = paramv[2];

    //Create the local computation EDTs
    ocrGuid_t localEdt_template;
    ocrEdtTemplateCreate(&localEdt_template, localFunc, 4, 0);
    u64 paramsLocal[4];
    paramsLocal[0] = iter;
    paramsLocal[1] = x;
    paramsLocal[2] = y;
    for (i = 0; i < NUM_LOCAL_EDTS; i++) {
        ocrGuid_t localEdt;
        paramsLocal[3] = i;
        ocrEdtCreate(&localEdt, localEdt_template, 4, paramsLocal, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    }
    return;
}

//Top-level function that runs on every iteration in each rank
ocrGuid_t resilientFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    u64 iter = paramv[0];
    u64 x = paramv[1];
    u64 y = paramv[2];

    if (iter < NUM_ITERS) {
        PRINTF("[Node %lu]: Hello from resilient EDT (%lu, %lu, %lu)\n", ocrGetLocation(), iter, x, y);
    } else if (iter == NUM_ITERS) {
        PRINTF("[Node %lu]: Hello from epilogue EDT (%lu, %lu)\n", ocrGetLocation(), x, y);
    } else {
        PRINTF("[Node %lu]: Hello from cleanup EDT (%lu, %lu)\n", ocrGetLocation(), x, y);
    }

    //Cleanup past iteration (iter - 2) DB and (iter - 1) Events
    ocrGuid_t *guidParamv = (ocrGuid_t*)&(paramv[1 + NUM_DIMS]);
    ocrDbDestroy(guidParamv[1]);
    if (iter > 2) {
        ocrGuid_t pEvt;
        ocrGuidTableRemove(USER_KEY((iter - 1), x, y), &pEvt);
        ocrEventDestroy(pEvt);
    }

    //Check for termination condition on this iteration
    if (iter == (NUM_ITERS + 1)) {
        ocrGuid_t shutdownEdt = guidParamv[0];
        ocrGuid_t curOutputEvent;
        ocrGetOutputEvent(&curOutputEvent);
        ocrAddDependence(curOutputEvent, shutdownEdt, ITER_INDEX(x, y), DB_MODE_CONST);
        return NULL_GUID;
    }

    //Verify the validity of the input DBs for this iteration (iter)
    if ((*((u64*)depv[0].ptr) != USER_KEY(iter, x, y)) ||
       ((x > 0) && (*((u64*)depv[1].ptr) != USER_KEY(iter, (x-1), y))) ||
       ((y > 0) && (*((u64*)depv[2].ptr) != USER_KEY(iter, x, (y-1)))) ||
       ((x < (XDIM-1)) && (*((u64*)depv[3].ptr) != USER_KEY(iter, (x+1), y))) ||
       ((y < (YDIM-1)) && (*((u64*)depv[4].ptr) != USER_KEY(iter, x, (y+1))))) {
        PRINTF("[Node %lu]: Data corruption detected in resilient EDT (%lu, %lu, %lu)\n", ocrGetLocation(), iter, x, y);
        ASSERT(0);
        return NULL_GUID;
    }

#if INJECT_FAULT
    //Fault injection
    if (iter == NUM_ITERS/2 && x == 0 && y == 0) {
        PRINTF("[Node %lu]: Injecting fault from resilient EDT (%lu, %lu, %lu)\n", ocrGetLocation(), iter, x, y);
        ocrNodeFailure();
    }
#endif

    ocrGuid_t db = NULL_GUID;

    if (iter < NUM_ITERS) {
        //Create a resilient output DB of this iteration
        void *ptr = NULL;
        ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
        *((u64*)ptr) = USER_KEY((iter+1), x, y);

        //Perform local computation for output DB of this iteration
        doLocalComputation(paramc, paramv, depc, depv, db);
    }

    //Satisfy event to transmit DB (x,y) from this iteration (iter) to
    //next iteration (iter+1) EDT neighbors [(x-1, y), (x, y-1), (x+1, y), (x, y+1)]
    ocrGuid_t oEvt;
    ocrGuidTableGet(USER_KEY((iter + 1), x, y), &oEvt);
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

    //Setup dependences inputs for 5-point stencil
    ocrGuid_t deps[NUM_DEPS];

    //Dependence input from current (x,y)
    deps[0] = db; //We can directly use output DB instead of using event

    //Dependence input from top (x-1, y)
    if (x > 0) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x - 1), y), &deps[1]);
    } else {
        deps[1] = NULL_GUID;
    }

    //Dependence input from left (x, y-1)
    if (y > 0) {
        ocrGuidTableGet(USER_KEY((iter + 1), x, (y -1)), &deps[2]);
    } else {
        deps[2] = NULL_GUID;
    }

    //Dependence input from down (x+1, y)
    if (x < (XDIM - 1)) {
        ocrGuidTableGet(USER_KEY((iter + 1), (x + 1), y), &deps[3]);
    } else {
        deps[3] = NULL_GUID;
    }

    //Dependence input from right (x, y+1)
    if (y < (YDIM - 1)) {
        ocrGuidTableGet(USER_KEY((iter + 1), x, (y + 1)), &deps[4]);
    } else {
        deps[4] = NULL_GUID;
    }

    //Create EDT
    ocrGuid_t rankEdt, outputEvent;
    ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);

    //Create event for future iteration (iter + 2) and associate with user-defined key
    if ((iter + 2) <= (NUM_ITERS + 1)) {
        ocrGuid_t evt;
        ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        ocrGuidTablePut(USER_KEY((iter + 2), x, y), evt);
    }
    return NULL_GUID;
}

//Sync EDT used to call shutdown
ocrGuid_t shutdownFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i, j;
    PRINTF("[Node %lu]: Hello from shutdownEdt\n", ocrGetLocation());
    //Cleanup past iteration events (NUM_ITERS)
    for (i = 0; i < XDIM; i++) {
        for (j = 0; j < YDIM; j++) {
            if (NUM_ITERS > 1) {
                ocrGuid_t pEvt;
                ocrGuidTableRemove(USER_KEY((NUM_ITERS + 1), i, j), &pEvt);
                ocrEventDestroy(pEvt);
            }
        }
    }
    ocrShutdown();
    return NULL_GUID;
}

//EDT to start the program
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i, j;
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
    guidParams[1] = NULL_GUID; //past iteration DB to destroy

    //Create the initial set of DBs and Events per rank
    ocrGuid_t dbArray[ITER_SPACE];
    for (i = 0; i < XDIM; i++) {
        for (j = 0; j < YDIM; j++) {
            //Create a resilient DB for future iteration (iter + 1)
            ocrGuid_t db = NULL_GUID;
            void *ptr = NULL;
            ocrDbCreate(&db, (void**)&ptr, sizeof(u64), DB_PROP_RESILIENT, NULL_HINT, NO_ALLOC);
            *((u64*)ptr) = USER_KEY(1, i, j);
            ocrDbRelease(db);
            dbArray[ITER_INDEX(i, j)] = db;

            //Create event for future iteration (iter + 2) and associate with user-defined key
            if (NUM_ITERS > 1) {
                ocrGuid_t evt;
                ocrEventCreate(&evt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
                u64 key = USER_KEY(2, i, j);
                ocrGuidTablePut(key, evt);
            }
        }
    }

    //Create the initial rank EDTs (start iteration at 1)
    for (i = 0; i < XDIM; i++) {
        for (j = 0; j < YDIM; j++) {
            params[1] = i; //x-value
            params[2] = j; //y-value

            //5-point stencil
            ocrGuid_t deps[NUM_DEPS];
            deps[0] = dbArray[ITER_INDEX(i, j)];                                    //(i  , j  )
            deps[1] = (i > 0) ? dbArray[ITER_INDEX((i-1), j)] : NULL_GUID;          //(i-1, j  )
            deps[2] = (j > 0) ? dbArray[ITER_INDEX(i, (j-1))] : NULL_GUID;          //(i  , j-1)
            deps[3] = (i < (XDIM-1)) ? dbArray[ITER_INDEX((i+1), j)] : NULL_GUID;   //(i+1, j  )
            deps[4] = (j < (YDIM-1)) ? dbArray[ITER_INDEX(i, (j+1))] : NULL_GUID;   //(i  , j+1)

            ocrGuid_t rankEdt, outputEvent;
            ocrEdtCreate(&rankEdt, rankEdt_template, NUM_PARAMS, params, NUM_DEPS, deps, EDT_PROP_RESILIENT, NULL_HINT, &outputEvent);
        }
    }

    return NULL_GUID;
}
