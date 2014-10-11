/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"

luleshCtx *lulesh_create() {
    // allocate the context datablock
    ocrGuid_t contextGuid;
    luleshCtx *context;
    CNC_CREATE_ITEM(&contextGuid, (void**)&context, sizeof(*context));
    // store a copy of its guid inside
    context->_guids.self = contextGuid;
    // initialize graph events
    ocrEventCreate(&context->_guids.finalizedEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.doneEvent, OCR_EVENT_STICKY_T, true);
    ocrEventCreate(&context->_guids.awaitTag, OCR_EVENT_ONCE_T, true);
    // initialize item collections
    context->_items.stress_partial = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.hourglass_partial = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.force = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.position = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.velocity = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.volume = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.volume_derivative = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.characteristic_length = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.velocity_gradient = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.position_gradient = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.quadratic_viscosity_term = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.linear_viscosity_term = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.sound_speed = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.viscosity = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.pressure = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.energy = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.courant = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.hydro = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.delta_time = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    context->_items.elapsed_time = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
    ocrEventCreate(&context->_items.final_origin_energy, OCR_EVENT_IDEM_T, true);
    // initialize step collections
    ocrEdtTemplateCreate(&context->_steps.lulesh_finalize,
            _cncStep_lulesh_finalize, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_stress_partial,
            _cncStep_compute_stress_partial, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_hourglass_partial,
            _cncStep_compute_hourglass_partial, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.reduce_force,
            _cncStep_reduce_force, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_velocity,
            _cncStep_compute_velocity, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_position,
            _cncStep_compute_position, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_volume,
            _cncStep_compute_volume, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_volume_derivative,
            _cncStep_compute_volume_derivative, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_gradients,
            _cncStep_compute_gradients, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_viscosity_terms,
            _cncStep_compute_viscosity_terms, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_energy,
            _cncStep_compute_energy, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_characteristic_length,
            _cncStep_compute_characteristic_length, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_time_constraints,
            _cncStep_compute_time_constraints, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.compute_delta_time,
            _cncStep_compute_delta_time, EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&context->_steps.produce_output,
            _cncStep_produce_output, EDT_PARAM_UNK, EDT_PARAM_UNK);
    return context;
}

void lulesh_destroy(luleshCtx *context) {
    ocrEventDestroy(context->_guids.finalizedEvent);
    ocrEventDestroy(context->_guids.doneEvent);
    // destroy item collections
    // XXX - need to do a deep free by traversing the table
    FREE(context->_items.stress_partial);
    FREE(context->_items.hourglass_partial);
    FREE(context->_items.force);
    FREE(context->_items.position);
    FREE(context->_items.velocity);
    FREE(context->_items.volume);
    FREE(context->_items.volume_derivative);
    FREE(context->_items.characteristic_length);
    FREE(context->_items.velocity_gradient);
    FREE(context->_items.position_gradient);
    FREE(context->_items.quadratic_viscosity_term);
    FREE(context->_items.linear_viscosity_term);
    FREE(context->_items.sound_speed);
    FREE(context->_items.viscosity);
    FREE(context->_items.pressure);
    FREE(context->_items.energy);
    FREE(context->_items.courant);
    FREE(context->_items.hydro);
    FREE(context->_items.delta_time);
    FREE(context->_items.elapsed_time);
    ocrEventDestroy(context->_items.final_origin_energy);// destroy step collections
    ocrEdtTemplateDestroy(context->_steps.lulesh_finalize);
    ocrEdtTemplateDestroy(context->_steps.compute_stress_partial);
    ocrEdtTemplateDestroy(context->_steps.compute_hourglass_partial);
    ocrEdtTemplateDestroy(context->_steps.reduce_force);
    ocrEdtTemplateDestroy(context->_steps.compute_velocity);
    ocrEdtTemplateDestroy(context->_steps.compute_position);
    ocrEdtTemplateDestroy(context->_steps.compute_volume);
    ocrEdtTemplateDestroy(context->_steps.compute_volume_derivative);
    ocrEdtTemplateDestroy(context->_steps.compute_gradients);
    ocrEdtTemplateDestroy(context->_steps.compute_viscosity_terms);
    ocrEdtTemplateDestroy(context->_steps.compute_energy);
    ocrEdtTemplateDestroy(context->_steps.compute_characteristic_length);
    ocrEdtTemplateDestroy(context->_steps.compute_time_constraints);
    ocrEdtTemplateDestroy(context->_steps.compute_delta_time);
    ocrEdtTemplateDestroy(context->_steps.produce_output);
    ocrDbDestroy(context->_guids.self);
}

static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

static ocrGuid_t _graphFinishEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshArgs *args = depv[0].ptr;
    luleshCtx *context = depv[1].ptr;
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
    lulesh_init(args, context);
    ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *context = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_lulesh_finalize(context);
    // XXX - for some reason this causes a segfault?
    //CNC_DESTROY_ITEM(depv[1].guid);
    return NULL_GUID;
}

void lulesh_launch(luleshArgs *args, luleshCtx *context) {
    luleshArgs *argsCopy;
    ocrGuid_t graphEdtGuid, finalEdtGuid, edtTemplateGuid, outEventGuid, argsDbGuid;
    // copy the args struct into a data block
    // TODO - I probably need to free this sometime
    CNC_CREATE_ITEM(&argsDbGuid, (void**)&argsCopy, sizeof(*args));
    *argsCopy = *args;
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

void lulesh_await(luleshCtx *ctx) {
    // Can't launch the finalizer EDT from within the finish EDT,
    // so we copy the tag information into a DB and do it indirectly.
    ocrGuid_t _tagGuid = NULL_GUID;
    ocrEventSatisfy(ctx->_guids.awaitTag, _tagGuid);
}
