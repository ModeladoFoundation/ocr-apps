/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "Stencil1D_internal.h"

Stencil1DCtx *Stencil1D_create() {
// allocate the context datablock
    ocrGuid_t contextGuid;
    Stencil1DCtx *context;
    SIMPLE_DBCREATE(&contextGuid, (void**)&context, sizeof(*context));
    // store a copy of its guid inside
    context->_guids.self = contextGuid;
    // initialize graph events
    ocrEventCreate(&context->_guids.finalizedEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.doneEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.awaitTag, OCR_EVENT_ONCE_T, true);
    // initialize item collections
    context->_items.val = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    // initialize step collections
    ocrEdtTemplateCreate(&context->_steps.Stencil1D_finalize,
            _cncStep_Stencil1D_finalize, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.updateStep,
            _cncStep_updateStep, EDT_PARAM_UNK, EDT_PARAM_UNK);
    return context;
}

void Stencil1D_destroy(Stencil1DCtx *context) {
    ocrEventDestroy(context->_guids.finalizedEvent);
    ocrEventDestroy(context->_guids.doneEvent);
    // destroy item collections
    // XXX - need to do a deep free by traversing the table
    FREE(context->_items.val);// destroy step collections
    ocrEdtTemplateDestroy(context->_steps.Stencil1D_finalize);
    ocrEdtTemplateDestroy(context->_steps.updateStep);
    ocrDbDestroy(context->_guids.self);
}

static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

static ocrGuid_t _graphFinishEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    Stencil1DArgs *args = depv[0].ptr;
    Stencil1DCtx *context = depv[1].ptr;
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
    ocrEdtTemplateDestroy(templGuid);
    // Start graph execution
    Stencil1D_init(args, context);
    if (args) ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    Stencil1DCtx *context = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_Stencil1D_finalize(context);
    // XXX - for some reason this causes a segfault?
    //ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
}

void Stencil1D_launch(Stencil1DArgs *args, Stencil1DCtx *context) {
    Stencil1DArgs *argsCopy;
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

void Stencil1D_await(Stencil1DCtx *ctx) {
    // Can't launch the finalizer EDT from within the finish EDT,
    // so we copy the tag information into a DB and do it indirectly.
    ocrGuid_t _tagGuid = NULL_GUID;
    ocrEventSatisfy(ctx->_guids.awaitTag, _tagGuid);
}
