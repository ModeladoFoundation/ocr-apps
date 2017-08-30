/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"
#include <string.h>

#ifdef CNC_DEBUG_LOG
#ifndef CNCOCR_x86
#error "Debug logging mode only supported on x86 targets"
#endif
#include <pthread.h>
pthread_mutex_t _cncDebugMutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* CNC_DEBUG_LOG */

void lulesh_ctx_init(ocrGuid_t contextGuid, luleshCtx *ctx) {
#ifdef CNC_DEBUG_LOG
    // init debug logger (only once)
    if (!cncDebugLog) {
        cncDebugLog = fopen(CNC_DEBUG_LOG, "w");
    }
#endif /* CNC_DEBUG_LOG */
    #ifdef CNC_AFFINITIES
    u64 affinityCount;
    ocrAffinityCount(AFFINITY_PD, &affinityCount);
    ctxBytes += sizeof(ocrGuid_t) * affinityCount;
    #endif /* CNC_AFFINITIES */
    // store a copy of its guid inside
    ctx->_guids.self = contextGuid;
    // initialize graph events
    // TODO - these events probably shouldn't be marked as carrying data
    ocrEventCreate(&ctx->_guids.finalizedEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
    ocrEventCreate(&ctx->_guids.quiescedEvent, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
    ocrEventCreate(&ctx->_guids.doneEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
    ocrEventCreate(&ctx->_guids.awaitTag, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
    ocrEventCreate(&ctx->_guids.contextReady, OCR_EVENT_LATCH_T, EVT_PROP_NONE);
    #ifdef CNC_AFFINITIES
    ctx->_affinityCount = affinityCount;
    // XXX - should use this for item collection affinities
    // Currently, I have to query DB guids to get affinities safely
    // across PDs (see issue #640). However, this works well since I'm
    // making a local copy of the context on each node anyway.
    { // initialize affinity data
        const ocrGuid_t currentLoc = _cncCurrentAffinity();
        ocrAffinityGet(AFFINITY_PD, &affinityCount, ctx->_affinities);
        assert(affinityCount == ctx->_affinityCount);
        u32 i;
        void *data;
        for (i=0; i<affinityCount; i++) {
            ocrGuid_t a = ctx->_affinities[i];
            if (ocrGuidIsEq(a, currentLoc)) {
                ctx->_affinities[i] = contextGuid;
            }
            else {
                ocrHint_t hint;
                // IMPORTANT! We do NOT use _CNC_DBCREATE here because we do NOT want DB_PROP_SINGLE_ASSIGNMENT
                // XXX - Should be usiing DB_PROP_NO_ACQUIRE, but it's not working right now... (issue #933)
                // const u16 flags = DB_PROP_NO_ACQUIRE;
                const u16 flags = DB_PROP_NONE;
                ocrDbCreate(&ctx->_affinities[i], &data, ctxBytes, flags,
                        _cncDbAffinityHint(&hint, a), NO_ALLOC);
                // XXX - shouldn't need this release because we should be using NO_ACQUIRE (see comment above)
                ocrDbRelease(ctx->_affinities[i]);
            }
        }
    }
    #endif /* CNC_AFFINITIES */
}

static void _initCtxColls(luleshCtx *ctx) {
    // initialize item collections
    ctx->_items.stress_partial = _cncItemCollectionCreate();
    ctx->_items.hourglass_partial = _cncItemCollectionCreate();
    ctx->_items.force = _cncItemCollectionCreate();
    ctx->_items.position = _cncItemCollectionCreate();
    ctx->_items.velocity = _cncItemCollectionCreate();
    ctx->_items.volume = _cncItemCollectionCreate();
    ctx->_items.volume_derivative = _cncItemCollectionCreate();
    ctx->_items.characteristic_length = _cncItemCollectionCreate();
    ctx->_items.velocity_gradient = _cncItemCollectionCreate();
    ctx->_items.position_gradient = _cncItemCollectionCreate();
    ctx->_items.quadratic_viscosity_term = _cncItemCollectionCreate();
    ctx->_items.linear_viscosity_term = _cncItemCollectionCreate();
    ctx->_items.sound_speed = _cncItemCollectionCreate();
    ctx->_items.viscosity = _cncItemCollectionCreate();
    ctx->_items.pressure = _cncItemCollectionCreate();
    ctx->_items.energy = _cncItemCollectionCreate();
    ctx->_items.courant = _cncItemCollectionCreate();
    ctx->_items.hydro = _cncItemCollectionCreate();
    ctx->_items.delta_time = _cncItemCollectionCreate();
    ctx->_items.elapsed_time = _cncItemCollectionCreate();
    ctx->_items.final_origin_energy = _cncItemCollectionSingletonCreate();
    // initialize step collections
    ocrEdtTemplateCreate(&ctx->_steps.cncFinalize,
            _lulesh_cncStep_cncFinalize, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_stress_partial,
            _lulesh_cncStep_compute_stress_partial, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_hourglass_partial,
            _lulesh_cncStep_compute_hourglass_partial, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.reduce_force,
            _lulesh_cncStep_reduce_force, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_velocity,
            _lulesh_cncStep_compute_velocity, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_position,
            _lulesh_cncStep_compute_position, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_volume,
            _lulesh_cncStep_compute_volume, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_volume_derivative,
            _lulesh_cncStep_compute_volume_derivative, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_gradients,
            _lulesh_cncStep_compute_gradients, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_viscosity_terms,
            _lulesh_cncStep_compute_viscosity_terms, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_energy,
            _lulesh_cncStep_compute_energy, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_characteristic_length,
            _lulesh_cncStep_compute_characteristic_length, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_time_constraints,
            _lulesh_cncStep_compute_time_constraints, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.compute_delta_time,
            _lulesh_cncStep_compute_delta_time, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.produce_output,
            _lulesh_cncStep_produce_output, EDT_PARAM_UNK, EDT_PARAM_UNK);
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _distInitEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    const u64 rank = paramv[0];
    luleshCtx *orig = depv[0].ptr;
    luleshCtx *local = depv[1].ptr;

    // copy user data and guids
    memcpy(local, orig, offsetof(luleshCtx, _items));
    local->_guids.self = depv[1].guid;

    // set up collections
    _initCtxColls(local);

    // copy affinity info
    local->_rank = rank;
    local->_affinityCount = orig->_affinityCount;
    const u64 affinityBytes = sizeof(ocrGuid_t) * local->_affinityCount;
    memcpy(local->_affinities, orig->_affinities, affinityBytes);

    ocrDbRelease(local->_guids.self);

    ocrEventSatisfySlot(orig->_guids.contextReady, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);

    return NULL_GUID;
}

static void _distSetup(luleshCtx *ctx) {
    u64 r;
    const u64 ranks = ctx->_affinityCount;
    const ocrGuid_t currentLoc = _cncCurrentAffinity();
    ocrGuid_t edtGuid, templGuid, loc;
    ocrEdtTemplateCreate(&templGuid, _distInitEdt, 1, 2);
    for (r=0; r<ranks; r++) {
        loc = _cncAffinityFromRank(r, ctx->_affinities);
        if (ocrGuidIsEq(loc, currentLoc)) {
            ctx->_rank = r;
            _initCtxColls(ctx);
        }
        else {
            ocrEventSatisfySlot(ctx->_guids.contextReady, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
            const ocrGuid_t remoteCtx = ctx->_affinities[r];
            ocrHint_t hint;
            ocrEdtCreate(&edtGuid, templGuid,
                    /*paramc=*/EDT_PARAM_DEF, /*paramv=*/&r,
                    /*depc=*/EDT_PARAM_DEF, /*depv=*/NULL,
                    /*properties=*/EDT_PROP_NONE,
                    /*hint=*/_cncEdtAffinityHint(&hint, loc), /*outEvent=*/NULL);
            ocrAddDependence(ctx->_guids.self, edtGuid, 0, DB_MODE_RO);
            ocrAddDependence(remoteCtx, edtGuid, 1, DB_DEFAULT_MODE);
        }
    }
    ocrEdtTemplateDestroy(templGuid);
}
#endif /* CNC_AFFINITIES */

void lulesh_destroy(luleshCtx *ctx) {
    ocrEventDestroy(ctx->_guids.finalizedEvent);
    ocrEventDestroy(ctx->_guids.quiescedEvent);
    ocrEventDestroy(ctx->_guids.doneEvent);
    // destroy item collections
    _cncItemCollectionDestroy(ctx->_items.stress_partial);
    _cncItemCollectionDestroy(ctx->_items.hourglass_partial);
    _cncItemCollectionDestroy(ctx->_items.force);
    _cncItemCollectionDestroy(ctx->_items.position);
    _cncItemCollectionDestroy(ctx->_items.velocity);
    _cncItemCollectionDestroy(ctx->_items.volume);
    _cncItemCollectionDestroy(ctx->_items.volume_derivative);
    _cncItemCollectionDestroy(ctx->_items.characteristic_length);
    _cncItemCollectionDestroy(ctx->_items.velocity_gradient);
    _cncItemCollectionDestroy(ctx->_items.position_gradient);
    _cncItemCollectionDestroy(ctx->_items.quadratic_viscosity_term);
    _cncItemCollectionDestroy(ctx->_items.linear_viscosity_term);
    _cncItemCollectionDestroy(ctx->_items.sound_speed);
    _cncItemCollectionDestroy(ctx->_items.viscosity);
    _cncItemCollectionDestroy(ctx->_items.pressure);
    _cncItemCollectionDestroy(ctx->_items.energy);
    _cncItemCollectionDestroy(ctx->_items.courant);
    _cncItemCollectionDestroy(ctx->_items.hydro);
    _cncItemCollectionDestroy(ctx->_items.delta_time);
    _cncItemCollectionDestroy(ctx->_items.elapsed_time);
    _cncItemCollectionSingletonDestroy(ctx->_items.final_origin_energy);// destroy step collections
    ocrEdtTemplateDestroy(ctx->_steps.cncFinalize);
    ocrEdtTemplateDestroy(ctx->_steps.compute_stress_partial);
    ocrEdtTemplateDestroy(ctx->_steps.compute_hourglass_partial);
    ocrEdtTemplateDestroy(ctx->_steps.reduce_force);
    ocrEdtTemplateDestroy(ctx->_steps.compute_velocity);
    ocrEdtTemplateDestroy(ctx->_steps.compute_position);
    ocrEdtTemplateDestroy(ctx->_steps.compute_volume);
    ocrEdtTemplateDestroy(ctx->_steps.compute_volume_derivative);
    ocrEdtTemplateDestroy(ctx->_steps.compute_gradients);
    ocrEdtTemplateDestroy(ctx->_steps.compute_viscosity_terms);
    ocrEdtTemplateDestroy(ctx->_steps.compute_energy);
    ocrEdtTemplateDestroy(ctx->_steps.compute_characteristic_length);
    ocrEdtTemplateDestroy(ctx->_steps.compute_time_constraints);
    ocrEdtTemplateDestroy(ctx->_steps.compute_delta_time);
    ocrEdtTemplateDestroy(ctx->_steps.produce_output);
    ocrDbDestroy(ctx->_guids.self);
}

// FIXME - shouldn't need this. that's why we have finalized vs done
static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

/* EDT's output event fires when all compute steps are done AND graph is finalized (graph is DONE) */
static ocrGuid_t _graphFinishedEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t finalizerResult = depv[1].guid;
    return finalizerResult;
}

/* EDT's output event fires when all compute steps are done (graph is quiesced) */
static ocrGuid_t _stepsFinishEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshArgs *args = _cncItemDataPtr(depv[1].ptr);
    luleshCtx *ctx = depv[2].ptr;
    // XXX - finalizer should be a finish EDT?
    // The graph isn't done until the finalizer runs as well,
    // so we need to make a dummy EDT depending on the
    // finalizer's output event.
    ocrGuid_t emptyEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _emptyEdt, 0, 1);
    ocrHint_t hint;
    ocrEdtCreate(&emptyEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&ctx->_guids.finalizedEvent,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_cncCurrentEdtAffinityHint(&hint), /*outEvent=*/NULL);
    // XXX - destroying this template caused crash on FSim
    //ocrEdtTemplateDestroy(templGuid);
    // Start graph execution
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    assert(!depv[0].ptr && "UNUSED");
    assert(!depv[1].ptr && "UNUSED");
    ocxxr_start_task();
    ocxxr_add_db(&depv[2]);
    lulesh_cncInitialize(args, ctx);
    ocxxr_end_task();
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_cncFinalize(ctx);
    // FIXME - I probably need to free this (the tag) sometime
    // XXX - for some reason this causes a segfault?
    //ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
}

void lulesh_launch(luleshArgs *args, luleshCtx *ctx) {
    ocrGuid_t graphEdtGuid, finalEdtGuid, doneEdtGuid, edtTemplateGuid, outEventGuid;
    // args struct should already be in a datablock (allocated with cncItemAlloc)
    ocrGuid_t argsDbGuid = _cncItemGuid(args);
    // XXX - should only do this if it wasn't already released? (should check release flag)
    // I probably need to refactor some code out from cncPut
    if (args) ocrDbRelease(argsDbGuid);
    // affinity
    const ocrGuid_t _affinity = _cncCurrentAffinity();
    // create a finish EDT for the CnC graph
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _stepsFinishEdt, 0, 3);
        ocrHint_t _hint;
        ocrEdtCreate(&graphEdtGuid, edtTemplateGuid,
                /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
                /*depc=*/EDT_PARAM_DEF, /*depv=*/NULL,
                /*properties=*/EDT_PROP_FINISH,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's quiescedEvent into the graph's output event
        ocrAddDependence(outEventGuid, ctx->_guids.quiescedEvent, 0, DB_MODE_NULL);
        // don't start until the context is fully initialized
        ocrAddDependence(ctx->_guids.contextReady, graphEdtGuid, 0, DB_MODE_NULL);
    }
    // finish initializing the context
    {
        ocrEventSatisfySlot(ctx->_guids.contextReady, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
        #ifdef CNC_AFFINITIES
        _distSetup(ctx);
        #else
        _initCtxColls(ctx);
        #endif /* CNC_AFFINITIES */
        ocrEventSatisfySlot(ctx->_guids.contextReady, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);
    }
    // set up the finalizer
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _finalizerEdt, 0, 2);
        ocrGuid_t deps[] = { ctx->_guids.self, ctx->_guids.awaitTag };
        ocrHint_t _hint;
        ocrEdtCreate(&finalEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_FINISH,
            /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
            /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's finalizedEvent into the finalizer's output event
        ocrAddDependence(outEventGuid, ctx->_guids.finalizedEvent, 0, DB_MODE_NULL);
    }
    // set up the EDT that controls the graph's doneEvent
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _graphFinishedEdt, 0, 2);
        ocrGuid_t deps[] = { ctx->_guids.quiescedEvent, ctx->_guids.finalizedEvent };
        ocrHint_t _hint;
        ocrEdtCreate(&doneEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_NONE,
            /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
            /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        ocrAddDependence(outEventGuid, ctx->_guids.doneEvent, 0, DB_MODE_NULL);
    }
    // start the graph execution
    ocrAddDependence(argsDbGuid, graphEdtGuid, 1, DB_DEFAULT_MODE);
    ocrGuid_t selfCtxGuid = ctx->_guids.self;
    ocrDbRelease(selfCtxGuid);
    ocrAddDependence(selfCtxGuid, graphEdtGuid, 2, DB_DEFAULT_MODE);
}

void lulesh_await(luleshCtx *ctx) {
    // Can't launch the finalizer EDT from within the finish EDT,
    // so we copy the tag information into a DB and do it indirectly.
    ocrGuid_t _tagGuid = NULL_GUID;
    ocrEventSatisfy(ctx->_guids.awaitTag, _tagGuid);
}

/* define NO_CNC_MAIN if you want to use mainEdt as the entry point instead */
#ifndef NO_CNC_MAIN

extern int cncMain(int argc, char *argv[]);

ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    // Unpack argc and argv (passed thru from mainEdt)
    int i, argc = OCR_MAIN_ARGC;
    char **argv = cncLocalAlloc(sizeof(char*)*argc);
    for (i=0; i<argc; i++) argv[i] = OCR_MAIN_ARGV(i);
    // Run user's cncEnvIn function
    cncMain(argc, argv);
    cncLocalFree(argv);
    return NULL_GUID;
}

#endif /* NO_CNC_MAIN */

