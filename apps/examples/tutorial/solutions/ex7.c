/*
 * Fibonacci using labeled GUIDs
 */

#include "ocr.h"

#define ENABLE_EXTENSION_LABELING

#include "extensions/ocr-labeling.h"
#include "ocr-std.h"

// paramv[0]: mapGuid
// depv[0]: fib(X-1)
// depv[1]: fib(X-2)
// depv[2]: X on input fib(X) on output
ocrGuid_t complete(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    ocrGuid_t mapGuid = (ocrGuid_t)(paramv[0]);
    u32 in1, in2;
    u32 out;

    /* When we run, we got our inputs from fib(n-1) and fib(n-2) */
    in1 = *(u32*)depv[0].ptr;
    in2 = *(u32*)depv[1].ptr;
    out = *(u32*)depv[2].ptr;
    PRINTF("Done with %d (%d + %d)\n", out, in1, in2);

    if (out > 3) {
        /* Destruction
           complete(n) and complete(n-1) use complete(n-2). But now,
           complete(n-1) is finished, or else complete(n) would not be
           running. So we don't need DB or Event for n-2 */

        ocrDbDestroy(depv[1].guid);
        ocrGuid_t evt_n_2 = NULL_GUID;
        ocrGuidFromIndex(&evt_n_2, mapGuid, out - 2);
        ocrEventDestroy(evt_n_2);
    }

    /* we return our answer in the 3rd db passed in as an argument */
    *((u32*)(depv[2].ptr)) = in1 + in2;

    // We satisfy the proper event
    ocrGuid_t outEvt = NULL_GUID;
    // At this point, we know that the event is created
    ocrGuidFromIndex(&outEvt, mapGuid, out);
    ocrDbRelease(depv[2].guid);
    ocrEventSatisfy(outEvt, depv[2].guid);

    return NULL_GUID;
}

// paramv[0]: mapGuid
// paramv[1]: fibEdt template
// paramv[2]: complete template
// depv[0]: input value to compute and also DB to write response in
ocrGuid_t fibEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    void* ptr;
    ocrGuid_t mapGuid, fibTemplateGuid, compTemplateGuid;
    ocrGuid_t dbArg;
    ocrGuid_t fibEdt;
    ocrGuid_t evt0, evt1;
    ocrGuid_t comp;

    typedef union {
        u64 param64[3*sizeof(ocrGuid_t)/sizeof(u64)];
        ocrGuid_t paramGuid[3];
    } paramvExtract;

    paramvExtract *extract = (paramvExtract*)paramv;
    mapGuid = extract->paramGuid[0];
    fibTemplateGuid = extract->paramGuid[1];
    compTemplateGuid = extract->paramGuid[2];

    u32 n = *(u32*)(depv[0].ptr);
    PRINTF("Starting fibEdt(%u)\n", n);
    ASSERT(n >= 2);

    /* create the completion EDT and pass it the in/out argument as a dependency */
    /* create the EDT with the done_event as the argument */
    ocrEdtCreate(&comp, compTemplateGuid, sizeof(ocrGuid_t)/sizeof(u64), paramv, 3, NULL, EDT_PROP_NONE,
                 NULL_GUID, NULL);

    PRINTF("In fibEdt(%d) -- checking for required answers\n", n);

    // Check if n-1 exists
    ocrGuidFromIndex(&evt0, mapGuid, n-1);
    if(ocrEventCreate(&evt0, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG) == OCR_EGUIDEXISTS) {
        // Event already exists so I can just link to it
        PRINTF("In fibEdt(%d), reusing answer for %d\n", n, n-1);
    } else {
        // We created the event so we need to launch the computation
        ocrDbCreate(&dbArg, (void**)&ptr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
        PRINTF("In fibEdt(%u) -- created arg DB for %d GUID 0x%llx\n", n, n-1, dbArg);
        *((u32*)ptr) = n-1;
        ocrEdtCreate(&fibEdt, fibTemplateGuid, 3*sizeof(ocrGuid_t)/sizeof(u64), paramv, 1, &dbArg, EDT_PROP_NONE,
                     NULL_GUID, NULL);
    }
    ocrAddDependence(evt0, comp, 0, DB_DEFAULT_MODE);

    // Check if n-2 exists
    ocrGuidFromIndex(&evt1, mapGuid, n-2);
    if(ocrEventCreate(&evt1, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG) == OCR_EGUIDEXISTS) {
        // Event already exists so I can just link to it
        PRINTF("In fibEdt(%d), reusing answer for %d\n", n, n-2);
    } else {
        // We created the event so we need to launch the computation
        ocrDbCreate(&dbArg, (void**)&ptr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
        PRINTF("In fibEdt(%u) -- created arg DB for %d GUID 0x%llx\n", n, n-2, dbArg);
        *((u32*)ptr) = n-2;
        ocrEdtCreate(&fibEdt, fibTemplateGuid, 3*sizeof(ocrGuid_t)/sizeof(u64), paramv, 1, &dbArg, EDT_PROP_NONE,
                     NULL_GUID, NULL);
    }
    ocrAddDependence(evt1, comp, 1, DB_DEFAULT_MODE);

    PRINTF("In fibEdt(%u) -- spawned complete EDT GUID 0x%llx\n", n, (u64)comp);
    ocrAddDependence(depv[0].guid, comp, 2, DB_DEFAULT_MODE);

    PRINTF("Returning from fibEdt(%u)\n", n);
    return NULL_GUID;

}

ocrGuid_t absFinal(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 ans;
    ans = *(u32*)depv[0].ptr;
    VERIFY(ans == (u32)paramv[0], "Totally done: answer is %d\n", ans);
    // Lots of leaks here (events and DBs)
    ocrShutdown();

    return NULL_GUID;
}

u64 fib(u32 n)
{
    if(n<=0) return 0;
    if(n<=2) return 1;
    else return fib(n-1) + fib(n-2);
}

/* just define the main EDT function */
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Starting mainEdt\n");
    u32 input;
    u32 argc = getArgc(depv[0].ptr);
    if((argc != 2)) {
        PRINTF("Usage: fib <num>, defaulting to 10\n");
        input = 10;
    } else {
        input = atoi(getArgv(depv[0].ptr, 1));
    }

    u64 correctAns = fib(input);

    // We create a map of sticky events which will be used
    // to "store" the results of reach fib(X)
    ocrGuid_t mapGuid = NULL_GUID;
    ocrGuidRangeCreate(&mapGuid, input+1, GUID_USER_EVENT_STICKY);

    // Create the base case DBs
    ocrGuid_t ans0, ans1;
    u32* addr;
    ocrDbCreate(&ans0, (void**)&addr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    *addr = 0;
    ocrDbCreate(&ans1, (void**)&addr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    *addr = 1;

    ocrDbRelease(ans0);
    ocrDbRelease(ans1);
    // Create the base events and satisfy. This is the "trust me" mode where
    // the runtime will blindly create the events with the GUID. This is what we
    // want since the events definitely do not exist at this point
    ocrGuid_t evtGuid = NULL_GUID;
    ocrGuidFromIndex(&evtGuid, mapGuid, 0);
    ocrEventCreate(&evtGuid, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);
    ocrEventSatisfy(evtGuid, ans0);

    ocrGuidFromIndex(&evtGuid, mapGuid, 1);
    ocrEventCreate(&evtGuid, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);
    ocrEventSatisfy(evtGuid, ans1);

    ocrGuidFromIndex(&evtGuid, mapGuid, 2);
    ocrEventCreate(&evtGuid, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);
    ocrEventSatisfy(evtGuid, ans1);

    // Now create the EDTs and what not
    ocrGuid_t fibC, totallyDoneEvent, absFinalEdt;
    {
        ocrGuid_t templateGuid;
        ocrEdtTemplateCreate(&templateGuid, absFinal, 1, 1);
        PRINTF("Created template and got GUID 0x%llx\n", templateGuid);
        ocrEdtCreate(&absFinalEdt, templateGuid, 1, &correctAns, 1, NULL, EDT_PROP_NONE,
                     NULL_GUID, NULL);
        PRINTF("Created ABS EDT and got  GUID 0x%llx\n", absFinalEdt);
        ocrEdtTemplateDestroy(templateGuid);
    }

    /* create a db for the results */
    ocrGuid_t fibArg;
    u32* res;

    PRINTF("Before 1st DB create\n");
    ocrDbCreate(&fibArg, (void**)&res, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    PRINTF("Got DB created\n");

    /* DB is in/out */
    *res = input;

    // PROP_CHECK is to deal with 0, 1, 2
    ocrGuidFromIndex(&totallyDoneEvent, mapGuid, input);
    ocrEventCreate(&totallyDoneEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
    ocrAddDependence(totallyDoneEvent, absFinalEdt, 0, DB_DEFAULT_MODE);

    if(input > 2) {
        // Pass the GUID parameters to the mainEdt only if we actually need to do something.
        // For 0, 1 and 2, absFinal is already ready.
        union {
            u64 param64[3*sizeof(ocrGuid_t)/sizeof(u64)];
            ocrGuid_t paramGuid[3];
        } paramv;
        ocrGuid_t depv = fibArg;

        ocrGuid_t templateGuid;
        paramv.paramGuid[0] = mapGuid;

        ocrEdtTemplateCreate(&templateGuid, complete, sizeof(ocrGuid_t)/sizeof(u64), 3);
        paramv.paramGuid[2] = templateGuid;

        ocrEdtTemplateCreate(&templateGuid, fibEdt, 3*sizeof(ocrGuid_t)/sizeof(u64), 1);
        paramv.paramGuid[1] = templateGuid;

        ocrEdtCreate(&fibC, templateGuid, 3*sizeof(ocrGuid_t)/sizeof(u64), &(paramv.param64[0]), 1, &depv, EDT_PROP_NONE,
                     NULL_GUID, NULL);
    }

    return NULL_GUID;
}
