/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "comd_internal.h"

comdCtx *comd_create() {
// allocate the context datablock
    ocrGuid_t contextGuid;
    comdCtx *context;
    SIMPLE_DBCREATE(&contextGuid, (void**)&context, sizeof(*context));
    // store a copy of its guid inside
    context->_guids.self = contextGuid;
    // initialize graph events
    ocrEventCreate(&context->_guids.finalizedEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.doneEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.awaitTag, OCR_EVENT_ONCE_T, true);
    // initialize item collections
    s32 i;
    ocrGuid_t *itemTable;
    SIMPLE_DBCREATE(&context->_items.B, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.SF, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.POT, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.EAMPOT, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.SPECIES, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.DD, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.LC, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.NAtoms, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.redc, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.AtomInfo, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.CMD, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.time, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.ATOMS, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.GID, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.ISP, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.R, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.P, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.F, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.U, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.IT, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.TBoxes, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    SIMPLE_DBCREATE(&context->_items.Nbs, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
    for (i=0; i<CNC_TABLE_SIZE; i++) {
        ocrGuid_t *_ptr;
        // Add one level of indirection to help with contention
        SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
        *_ptr = NULL_GUID;
    }
    // initialize step collections
    ocrEdtTemplateCreate(&context->_steps.comd_finalize,
            _cncStep_comd_finalize, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.cleanupInitStep,
            _cncStep_cleanupInitStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.advanceVelocityStep,
            _cncStep_advanceVelocityStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.updateBoxStep,
            _cncStep_updateBoxStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.updateNeighborsStep,
            _cncStep_updateNeighborsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.generateDataforForceStep,
            _cncStep_generateDataforForceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.generateForceTagsStep,
            _cncStep_generateForceTagsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.forceStep,
            _cncStep_forceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.computeForcefromNeighborsStep,
            _cncStep_computeForcefromNeighborsStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.computeForcefromNeighborsStep1,
            _cncStep_computeForcefromNeighborsStep1, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.reduceStep,
            _cncStep_reduceStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    return context;
}

void comd_destroy(comdCtx *context) {
    ocrEventDestroy(context->_guids.finalizedEvent);
    ocrEventDestroy(context->_guids.doneEvent);
    // destroy item collections
    // XXX - need to do a deep free by traversing the table
    // destroy step collections
    ocrEdtTemplateDestroy(context->_steps.comd_finalize);
    ocrEdtTemplateDestroy(context->_steps.cleanupInitStep);
    ocrEdtTemplateDestroy(context->_steps.advanceVelocityStep);
    ocrEdtTemplateDestroy(context->_steps.updateBoxStep);
    ocrEdtTemplateDestroy(context->_steps.updateNeighborsStep);
    ocrEdtTemplateDestroy(context->_steps.generateDataforForceStep);
    ocrEdtTemplateDestroy(context->_steps.generateForceTagsStep);
    ocrEdtTemplateDestroy(context->_steps.forceStep);
    ocrEdtTemplateDestroy(context->_steps.computeForcefromNeighborsStep);
    ocrEdtTemplateDestroy(context->_steps.computeForcefromNeighborsStep1);
    ocrEdtTemplateDestroy(context->_steps.reduceStep);
    ocrDbDestroy(context->_guids.self);
}

static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

static ocrGuid_t _graphFinishEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdArgs *args = depv[0].ptr;
    comdCtx *context = depv[1].ptr;
    // XXX - just do finalize from within the finish EDT
    // The graph isn't done until the finalizer runs as well,
    // so we need to make a dummy EDT depending on the
    // finalizer's output event.
    ocrGuid_t emptyEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _emptyEdt, 0, 1);
    ocrEdtCreate(&emptyEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&context->_guids.finalizedEvent,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);
    // XXX - destroying this template caused crash on FSim
    //ocrEdtTemplateDestroy(templGuid);
    // Start graph execution

    comd_init(args, context);

    if (args) ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *context = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_comd_finalize(tag[0], tag[1], context);
    // XXX - for some reason this causes a segfault?
    //ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
}

void comd_launch(comdArgs *args, comdCtx *context) {
    comdArgs *argsCopy;
    ocrGuid_t graphEdtGuid, finalEdtGuid, edtTemplateGuid, outEventGuid, argsDbGuid;
    // copy the args struct into a data block
    // TODO - I probably need to free this sometime
    if (sizeof(*args) > 0) {
        SIMPLE_DBCREATE(&argsDbGuid, (void**)&argsCopy, sizeof(*args));
        *argsCopy = *args;
    }
    // Don't need to copy empty args structs
    else {
        argsDbGuid = NULL_GUID;
    }
    // create a finish EDT for the CnC graph
    ocrEdtTemplateCreate(&edtTemplateGuid, _graphFinishEdt, 0, 2);
    ocrEdtCreate(&graphEdtGuid, edtTemplateGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/NULL,
        /*properties=*/EDT_PROP_FINISH,
        /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
    ocrEdtTemplateDestroy(edtTemplateGuid);
    // hook doneEvent into the graph's output event
    ocrAddDependence(outEventGuid, context->_guids.doneEvent, 0, DB_DEFAULT_MODE);
    // set up the finalizer
    ocrEdtTemplateCreate(&edtTemplateGuid, _finalizerEdt, 0, 2);
    ocrGuid_t deps[] = { context->_guids.self, context->_guids.awaitTag };
    ocrEdtCreate(&finalEdtGuid, edtTemplateGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);
    ocrEdtTemplateDestroy(edtTemplateGuid);
    // start the graph execution
    ocrAddDependence(argsDbGuid, graphEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(context->_guids.self, graphEdtGuid, 1, DB_DEFAULT_MODE);
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

