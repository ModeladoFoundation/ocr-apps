/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "comd_internal.h"

#ifdef CNC_DEBUG_LOG
#ifndef CNCOCR_x86
#error "Debug logging mode only supported on x86 targets"
#endif
#include <pthread.h>
pthread_mutex_t _cncDebugMutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* CNC_DEBUG_LOG */

comdCtx *comd_create() {
#ifdef CNC_DEBUG_LOG
    // init debug logger (only once)
    if (!cncDebugLog) {
        cncDebugLog = fopen(CNC_DEBUG_LOG, "w");
    }
#endif /* CNC_DEBUG_LOG */
    // allocate the context datablock
    ocrGuid_t contextGuid;
    comdCtx *ctx;
    SIMPLE_DBCREATE(&contextGuid, (void**)&ctx, sizeof(*ctx));
    // store a copy of its guid inside
    ctx->_guids.self = contextGuid;
    // initialize graph events
    // TODO - these events probably shouldn't be marked as carrying data
    ocrEventCreate(&ctx->_guids.finalizedEvent, OCR_EVENT_STICKY_T, TRUE);
    ocrEventCreate(&ctx->_guids.quiescedEvent, OCR_EVENT_STICKY_T, FALSE);
    ocrEventCreate(&ctx->_guids.doneEvent, OCR_EVENT_STICKY_T, TRUE);
    ocrEventCreate(&ctx->_guids.awaitTag, OCR_EVENT_ONCE_T, TRUE);
    // initialize item collections
    ctx->_items.B = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.SF = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.POT = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.EAMPOT = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.SPECIES = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.DD = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.LC = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.NAtoms = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.redc = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.AtomInfo = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.CMD = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.time = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.ATOMS = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.GID = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.ISP = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.R = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.P = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.F = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.U = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.IT = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.TBoxes = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ctx->_items.Nbs = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    // initialize step collections
    ocrEdtTemplateCreate(&ctx->_steps.cncFinalize,
            _comd_cncStep_cncFinalize, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.cleanupInitStep,
            _comd_cncStep_cleanupInitStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.advanceVelocityStep,
            _comd_cncStep_advanceVelocityStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.updateBoxStep,
            _comd_cncStep_updateBoxStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.updateNeighborsStep,
            _comd_cncStep_updateNeighborsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.generateDataforForceStep,
            _comd_cncStep_generateDataforForceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.generateForceTagsStep,
            _comd_cncStep_generateForceTagsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.forceStep,
            _comd_cncStep_forceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.computeForcefromNeighborsStep,
            _comd_cncStep_computeForcefromNeighborsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.computeForcefromNeighborsStep1,
            _comd_cncStep_computeForcefromNeighborsStep1, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&ctx->_steps.reduceStep,
            _comd_cncStep_reduceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    return ctx;
}

void comd_destroy(comdCtx *ctx) {
    ocrEventDestroy(ctx->_guids.finalizedEvent);
    ocrEventDestroy(ctx->_guids.quiescedEvent);
    ocrEventDestroy(ctx->_guids.doneEvent);
    // destroy item collections
    // XXX - need to do a deep free by traversing the table
    FREE(ctx->_items.B);
    FREE(ctx->_items.SF);
    FREE(ctx->_items.POT);
    FREE(ctx->_items.EAMPOT);
    FREE(ctx->_items.SPECIES);
    FREE(ctx->_items.DD);
    FREE(ctx->_items.LC);
    FREE(ctx->_items.NAtoms);
    FREE(ctx->_items.redc);
    FREE(ctx->_items.AtomInfo);
    FREE(ctx->_items.CMD);
    FREE(ctx->_items.time);
    FREE(ctx->_items.ATOMS);
    FREE(ctx->_items.GID);
    FREE(ctx->_items.ISP);
    FREE(ctx->_items.R);
    FREE(ctx->_items.P);
    FREE(ctx->_items.F);
    FREE(ctx->_items.U);
    FREE(ctx->_items.IT);
    FREE(ctx->_items.TBoxes);
    FREE(ctx->_items.Nbs);// destroy step collections
    ocrEdtTemplateDestroy(ctx->_steps.cncFinalize);
    ocrEdtTemplateDestroy(ctx->_steps.cleanupInitStep);
    ocrEdtTemplateDestroy(ctx->_steps.advanceVelocityStep);
    ocrEdtTemplateDestroy(ctx->_steps.updateBoxStep);
    ocrEdtTemplateDestroy(ctx->_steps.updateNeighborsStep);
    ocrEdtTemplateDestroy(ctx->_steps.generateDataforForceStep);
    ocrEdtTemplateDestroy(ctx->_steps.generateForceTagsStep);
    ocrEdtTemplateDestroy(ctx->_steps.forceStep);
    ocrEdtTemplateDestroy(ctx->_steps.computeForcefromNeighborsStep);
    ocrEdtTemplateDestroy(ctx->_steps.computeForcefromNeighborsStep1);
    ocrEdtTemplateDestroy(ctx->_steps.reduceStep);
    ocrDbDestroy(ctx->_guids.self);
}

static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

/* EDT runs when all compute steps are done AND graph is finalized (graph is DONE) */
static ocrGuid_t _graphFinishedEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t finalizerResult = depv[1].guid;
    return finalizerResult;
}

/* EDT runs when all compute steps are done (graph is quiesced) */
static ocrGuid_t _stepsFinishedEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdArgs *args = depv[0].ptr;
    comdCtx *ctx = depv[1].ptr;
    // XXX - just do finalize from within the finish EDT
    // The graph isn't done until the finalizer runs as well,
    // so we need to make a dummy EDT depending on the
    // finalizer's output event.
    ocrGuid_t emptyEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _emptyEdt, 0, 1);
    ocrEdtCreate(&emptyEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&ctx->_guids.finalizedEvent,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);
    // XXX - destroying this template caused crash on FSim
    //ocrEdtTemplateDestroy(templGuid);
    // Start graph execution
    #ifdef CNC_DEBUG_LOG
pthread_mutex_lock(&_cncDebugMutex);
#endif
    comd_cncInitialize(args, ctx);
    #ifdef CNC_DEBUG_LOG
pthread_mutex_unlock(&_cncDebugMutex);
#endif
    if (args) ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_cncFinalize(tag[0], tag[1], ctx);
    // TODO - I probably need to free this (the tag) sometime
    // XXX - for some reason this causes a segfault?
    //ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
}

void comd_launch(comdArgs *args, comdCtx *ctx) {
    comdArgs *argsCopy;
    ocrGuid_t graphEdtGuid, finalEdtGuid, doneEdtGuid, edtTemplateGuid, outEventGuid, argsDbGuid;
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(ctx->_guids.self);
    // copy the args struct into a data block
    // TODO - I probably need to free this sometime
    if (sizeof(*args) > 0) {
        SIMPLE_DBCREATE(&argsDbGuid, (void**)&argsCopy, sizeof(*args));
        *argsCopy = *args;
        // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
        // ocrDbRelease(argsDbGuid);
    }
    // Don't need to copy empty args structs
    else {
        argsDbGuid = NULL_GUID;
    }
    // create a finish EDT for the CnC graph
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _stepsFinishedEdt, 0, 2);
        ocrEdtCreate(&graphEdtGuid, edtTemplateGuid,
                /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
                /*depc=*/EDT_PARAM_DEF, /*depv=*/NULL,
                /*properties=*/EDT_PROP_FINISH,
                /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's quiescedEvent into the graph's output event
        ocrAddDependence(outEventGuid, ctx->_guids.quiescedEvent, 0, DB_DEFAULT_MODE);
    }
    // set up the finalizer
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _finalizerEdt, 0, 2);
        ocrGuid_t deps[] = { ctx->_guids.self, ctx->_guids.awaitTag };
        ocrEdtCreate(&finalEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_FINISH,
            /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's finalizedEvent into the finalizer's output event
        ocrAddDependence(outEventGuid, ctx->_guids.finalizedEvent, 0, DB_DEFAULT_MODE);
    }
    // set up the EDT that controls the graph's doneEvent
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _graphFinishedEdt, 0, 2);
        ocrGuid_t deps[] = { ctx->_guids.quiescedEvent, ctx->_guids.finalizedEvent };
        ocrEdtCreate(&doneEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_NONE,
            /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        ocrAddDependence(outEventGuid, ctx->_guids.doneEvent, 0, DB_DEFAULT_MODE);
    }
    // start the graph execution
    ocrAddDependence(argsDbGuid, graphEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(ctx->_guids.self, graphEdtGuid, 1, DB_DEFAULT_MODE);
}

void comd_await(cncTag_t i, cncTag_t iter, comdCtx *ctx) {
    // Can't launch the finalizer EDT from within the finish EDT,
    // so we copy the tag information into a DB and do it indirectly.
    cncTag_t *_tagPtr;
    ocrGuid_t _tagGuid;
    int _i = 0;
    SIMPLE_DBCREATE(&_tagGuid, (void**)&_tagPtr, sizeof(cncTag_t) * 2);
    _tagPtr[_i++] = i;
    _tagPtr[_i++] = iter;
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_tagGuid);
    ocrEventSatisfy(ctx->_guids.awaitTag, _tagGuid);
}

/* define NO_CNC_MAIN if you want to use mainEdt as the entry point instead */
#ifndef NO_CNC_MAIN

extern int cncMain(int argc, char *argv[]);

#pragma weak mainEdt
ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    // Unpack argc and argv (passed thru from mainEdt)
    int i, argc = OCR_MAIN_ARGC;
    char **argv = cncMalloc(sizeof(char*)*argc);
    for (i=0; i<argc; i++) argv[i] = OCR_MAIN_ARGV(i);
    // Run user's cncEnvIn function
    cncMain(argc, argv);
    cncFree(argv);
    return NULL_GUID;
}

#endif /* NO_CNC_MAIN */

