/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"

#ifdef CNC_DEBUG_LOG
#if !defined(CNCOCR_x86)
#error "Debug logging mode only supported on x86 targets"
#endif
#include <pthread.h>
extern pthread_mutex_t _cncDebugMutex;
#endif /* CNC_DEBUG_LOG */


/* cncFinalize setup/teardown function */
ocrGuid_t _lulesh_cncStep_cncFinalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double final_origin_energy;
    ocxxr_add_db(&depv[_edtSlot]);
    final_origin_energy = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING cncFinalize @ 0\n");
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING cncFinalize @ 0\n");
    #endif
    lulesh_cncFinalize(final_origin_energy, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE cncFinalize @ 0\n");
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE cncFinalize @ 0\n");
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* cncFinalize task creation */
static void cncPrescribeInternal_cncFinalize(u64 *_tag, luleshCtx *ctx) {
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.cncFinalize,
        /*paramc=*/0, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "final_origin_energy" input dependencies
        cncGet_final_origin_energy(_stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED cncFinalize @ 0\n");
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED cncFinalize @ 0\n");
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_cncFinalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_cncFinalize(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* cncFinalize task creation */
void cncPrescribe_cncFinalize(luleshCtx *ctx) {
    u64 *_args = NULL;
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = 16 % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 0;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_cncFinalize, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_cncFinalize(_args, ctx);
}

/* compute_stress_partial setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_stress_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double pressure;
    ocxxr_add_db(&depv[_edtSlot]);
    pressure = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double viscosity;
    ocxxr_add_db(&depv[_edtSlot]);
    viscosity = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_stress_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_stress_partial @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_stress_partial(iteration, element_id, pressure, viscosity, neighbor_position, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_stress_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_stress_partial @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_stress_partial task creation */
static void cncPrescribeInternal_compute_stress_partial(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_stress_partial,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "pressure" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_pressure(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "viscosity" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_viscosity(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_stress_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_stress_partial @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_stress_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_stress_partial(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_stress_partial task creation */
void cncPrescribe_compute_stress_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_stress_partial, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_stress_partial(_args, ctx);
}

/* compute_hourglass_partial setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_hourglass_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double element_volume;
    ocxxr_add_db(&depv[_edtSlot]);
    element_volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double sound_speed;
    ocxxr_add_db(&depv[_edtSlot]);
    sound_speed = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    vector *neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vector *_item;
        neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_hourglass_partial(iteration, element_id, element_volume, sound_speed, neighbor_position, neighbor_velocity, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    cncLocalFree(neighbor_velocity);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_hourglass_partial task creation */
static void cncPrescribeInternal_compute_hourglass_partial(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 8 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_hourglass_partial,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "element_volume" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "sound_speed" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_sound_speed(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_hourglass_partial @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_hourglass_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_hourglass_partial(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_hourglass_partial task creation */
void cncPrescribe_compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_hourglass_partial, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_hourglass_partial(_args, ctx);
}

/* reduce_force setup/teardown function */
ocrGuid_t _lulesh_cncStep_reduce_force(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    vector *neighbor_stress_partial;
    { // Init ranges for "neighbor_stress_partial"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vector *_item;
        neighbor_stress_partial = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    vector *neighbor_hourglass_partial;
    { // Init ranges for "neighbor_hourglass_partial"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vector *_item;
        neighbor_hourglass_partial = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING reduce_force @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING reduce_force @ %ld, %ld\n", iteration, node_id);
    #endif
    lulesh_reduce_force(iteration, node_id, neighbor_stress_partial, neighbor_hourglass_partial, ctx);
    // Clean up
    cncLocalFree(neighbor_stress_partial);
    cncLocalFree(neighbor_hourglass_partial);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE reduce_force @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE reduce_force @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* reduce_force task creation */
static void cncPrescribeInternal_reduce_force(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 8 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.reduce_force,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "neighbor_stress_partial" input dependencies
        s64 _i0 = node_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_stress_partial(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    { // Set up "neighbor_hourglass_partial" input dependencies
        s64 _i0 = node_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_hourglass_partial(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED reduce_force @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED reduce_force @ %ld, %ld\n", iteration, node_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_reduce_force(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_reduce_force(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* reduce_force task creation */
void cncPrescribe_reduce_force(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)node_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = node_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_reduce_force, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_reduce_force(_args, ctx);
}

/* compute_velocity setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_velocity(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double delta_time;
    ocxxr_add_db(&depv[_edtSlot]);
    delta_time = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector force;
    ocxxr_add_db(&depv[_edtSlot]);
    force = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector previous_velocity;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_velocity = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_velocity @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_velocity @ %ld, %ld\n", iteration, node_id);
    #endif
    lulesh_compute_velocity(iteration, node_id, delta_time, force, previous_velocity, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_velocity @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_velocity @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_velocity task creation */
static void cncPrescribeInternal_compute_velocity(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_velocity,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "delta_time" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "force" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = node_id;
        cncGet_force(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_velocity" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = node_id;
        cncGet_velocity(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_velocity @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_velocity @ %ld, %ld\n", iteration, node_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_velocity(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_velocity(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_velocity task creation */
void cncPrescribe_compute_velocity(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)node_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = node_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_velocity, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_velocity(_args, ctx);
}

/* compute_position setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_position(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double delta_time;
    ocxxr_add_db(&depv[_edtSlot]);
    delta_time = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector velocity;
    ocxxr_add_db(&depv[_edtSlot]);
    velocity = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex previous_position;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_position = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_position @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_position @ %ld, %ld\n", iteration, node_id);
    #endif
    lulesh_compute_position(iteration, node_id, delta_time, velocity, previous_position, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_position @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_position @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_position task creation */
static void cncPrescribeInternal_compute_position(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(node_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_position,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "delta_time" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "velocity" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = node_id;
        cncGet_velocity(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_position" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = node_id;
        cncGet_position(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_position @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_position @ %ld, %ld\n", iteration, node_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_position(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_position(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_position task creation */
void cncPrescribe_compute_position(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)node_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = node_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_position, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_position(_args, ctx);
}

/* compute_volume setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_volume(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_volume @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_volume @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_volume(iteration, element_id, neighbor_position, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_volume @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_volume @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_volume task creation */
static void cncPrescribeInternal_compute_volume(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_volume,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_volume @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_volume @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_volume(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_volume(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_volume task creation */
void cncPrescribe_compute_volume(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_volume, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_volume(_args, ctx);
}

/* compute_volume_derivative setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_volume_derivative(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double delta_time;
    ocxxr_add_db(&depv[_edtSlot]);
    delta_time = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    vector *neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vector *_item;
        neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_volume_derivative(iteration, element_id, delta_time, neighbor_position, neighbor_velocity, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    cncLocalFree(neighbor_velocity);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_volume_derivative task creation */
static void cncPrescribeInternal_compute_volume_derivative(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 8 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_volume_derivative,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "delta_time" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_volume_derivative @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_volume_derivative(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_volume_derivative(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_volume_derivative task creation */
void cncPrescribe_compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_volume_derivative, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_volume_derivative(_args, ctx);
}

/* compute_gradients setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_gradients(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double volume;
    ocxxr_add_db(&depv[_edtSlot]);
    volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    vector *neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vector *_item;
        neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_gradients @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_gradients @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_gradients(iteration, element_id, volume, neighbor_position, neighbor_velocity, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    cncLocalFree(neighbor_velocity);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_gradients @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_gradients @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_gradients task creation */
static void cncPrescribeInternal_compute_gradients(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 8 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_gradients,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "volume" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_gradients @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_gradients @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_gradients(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_gradients(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_gradients task creation */
void cncPrescribe_compute_gradients(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_gradients, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_gradients(_args, ctx);
}

/* compute_viscosity_terms setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_viscosity_terms(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double volume;
    ocxxr_add_db(&depv[_edtSlot]);
    volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double volume_derivative;
    ocxxr_add_db(&depv[_edtSlot]);
    volume_derivative = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector position_gradient;
    ocxxr_add_db(&depv[_edtSlot]);
    position_gradient = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector velocity_gradient;
    ocxxr_add_db(&depv[_edtSlot]);
    velocity_gradient = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vector *neighbor_velocity_gradient;
    { // Init ranges for "neighbor_velocity_gradient"
        u32 _i;
        u32 _itemCount = 6;
        u32 _dims[] = { 6 };
        vector *_item;
        neighbor_velocity_gradient = _cncRangedInputAlloc(1, _dims, sizeof(vector ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vector *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_viscosity_terms(iteration, element_id, volume, volume_derivative, position_gradient, velocity_gradient, neighbor_velocity_gradient, ctx);
    // Clean up
    cncLocalFree(neighbor_velocity_gradient);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_viscosity_terms task creation */
static void cncPrescribeInternal_compute_viscosity_terms(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 1 + 1 + 6 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_viscosity_terms,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "volume" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "volume_derivative" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume_derivative(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "position_gradient" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_position_gradient(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "velocity_gradient" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_velocity_gradient(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_velocity_gradient" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 6; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity_gradient(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_viscosity_terms @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_viscosity_terms(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_viscosity_terms(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_viscosity_terms task creation */
void cncPrescribe_compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_viscosity_terms, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_viscosity_terms(_args, ctx);
}

/* compute_energy setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_energy(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double volume;
    ocxxr_add_db(&depv[_edtSlot]);
    volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double previous_volume;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double previous_energy;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_energy = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double previous_pressure;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_pressure = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double previous_viscosity;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_viscosity = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double qlin;
    ocxxr_add_db(&depv[_edtSlot]);
    qlin = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double qquad;
    ocxxr_add_db(&depv[_edtSlot]);
    qquad = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_energy @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_energy @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_energy(iteration, element_id, volume, previous_volume, previous_energy, previous_pressure, previous_viscosity, qlin, qquad, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_energy @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_energy @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_energy task creation */
static void cncPrescribeInternal_compute_energy(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_energy,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "volume" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_volume" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_energy" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_energy(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_pressure" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_pressure(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_viscosity" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_viscosity(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "qlin" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_linear_viscosity_term(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "qquad" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_quadratic_viscosity_term(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_energy @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_energy @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_energy(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_energy(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_energy task creation */
void cncPrescribe_compute_energy(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_energy, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_energy(_args, ctx);
}

/* compute_characteristic_length setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_characteristic_length(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double volume;
    ocxxr_add_db(&depv[_edtSlot]);
    volume = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    vertex *neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = 8;
        u32 _dims[] = { 8 };
        vertex *_item;
        neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(vertex ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(vertex *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_characteristic_length(iteration, element_id, volume, neighbor_position, ctx);
    // Clean up
    cncLocalFree(neighbor_position);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_characteristic_length task creation */
static void cncPrescribeInternal_compute_characteristic_length(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 8 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_characteristic_length,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "volume" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < 8; _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_characteristic_length @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_characteristic_length(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_characteristic_length(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_characteristic_length task creation */
void cncPrescribe_compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_characteristic_length, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_characteristic_length(_args, ctx);
}

/* compute_time_constraints setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_time_constraints(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double sound_speed;
    ocxxr_add_db(&depv[_edtSlot]);
    sound_speed = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double volume_derivative;
    ocxxr_add_db(&depv[_edtSlot]);
    volume_derivative = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double characteristic_length;
    ocxxr_add_db(&depv[_edtSlot]);
    characteristic_length = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_time_constraints @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_time_constraints @ %ld, %ld\n", iteration, element_id);
    #endif
    lulesh_compute_time_constraints(iteration, element_id, sound_speed, volume_derivative, characteristic_length, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_time_constraints @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_time_constraints @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_time_constraints task creation */
static void cncPrescribeInternal_compute_time_constraints(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)_tag[1]; MAYBE_UNUSED(element_id);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_time_constraints,
        /*paramc=*/2, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "sound_speed" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_sound_speed(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "volume_derivative" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume_derivative(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "characteristic_length" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_characteristic_length(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_time_constraints @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_time_constraints @ %ld, %ld\n", iteration, element_id);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_time_constraints(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_time_constraints(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_time_constraints task creation */
void cncPrescribe_compute_time_constraints(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration, (u64)element_id };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = element_id % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 2;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_time_constraints, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_time_constraints(_args, ctx);
}

/* compute_delta_time setup/teardown function */
ocrGuid_t _lulesh_cncStep_compute_delta_time(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double previous_delta_time;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_delta_time = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double previous_elapsed_time;
    ocxxr_add_db(&depv[_edtSlot]);
    previous_elapsed_time = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    double *courant;
    { // Init ranges for "courant"
        u32 _i;
        u32 _itemCount = ctx->elements;
        u32 _dims[] = { ctx->elements };
        double *_item;
        courant = _cncRangedInputAlloc(1, _dims, sizeof(double ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            //ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    double *hydro;
    { // Init ranges for "hydro"
        u32 _i;
        u32 _itemCount = ctx->elements;
        u32 _dims[] = { ctx->elements };
        double *_item;
        hydro = _cncRangedInputAlloc(1, _dims, sizeof(double ), (void**)&_item);
        for (_i=0; _i<_itemCount; _i++) {
            //ocxxr_add_db(&depv[_edtSlot]);
            _item[_i] = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING compute_delta_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING compute_delta_time @ %ld\n", iteration);
    #endif
    lulesh_compute_delta_time(iteration, previous_delta_time, previous_elapsed_time, courant, hydro, ctx);
    // Clean up
    cncLocalFree(courant);
    cncLocalFree(hydro);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE compute_delta_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE compute_delta_time @ %ld\n", iteration);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* compute_delta_time task creation */
static void cncPrescribeInternal_compute_delta_time(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1 + ctx->elements + ctx->elements + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_delta_time,
        /*paramc=*/1, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "previous_delta_time" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "previous_elapsed_time" input dependencies
        s64 _i0 = iteration;
        cncGet_elapsed_time(_i0, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    { // Set up "courant" input dependencies
        s64 _i0 = iteration;
        s64 _i1;
        for (_i1 = 0; _i1 < ctx->elements; _i1++) {
            cncGet_courant(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    { // Set up "hydro" input dependencies
        s64 _i0 = iteration;
        s64 _i1;
        for (_i1 = 0; _i1 < ctx->elements; _i1++) {
            cncGet_hydro(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED compute_delta_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED compute_delta_time @ %ld\n", iteration);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_compute_delta_time(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_compute_delta_time(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* compute_delta_time task creation */
void cncPrescribe_compute_delta_time(cncTag_t iteration, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = iteration % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 1;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_compute_delta_time, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_compute_delta_time(_args, ctx);
}

/* produce_output setup/teardown function */
ocrGuid_t _lulesh_cncStep_produce_output(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocxxr_start_task();
    luleshCtx *ctx = depv[0].ptr;
    ocxxr_add_db(&depv[0]);

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    double final_energy;
    ocxxr_add_db(&depv[_edtSlot]);
    final_energy = *(double *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    #ifdef CNC_DEBUG_LOG
        pthread_mutex_lock(&_cncDebugMutex);
    #endif
    // Call user-defined step function
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "RUNNING produce_output @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: RUNNING produce_output @ %ld\n", iteration);
    #endif
    lulesh_produce_output(iteration, final_energy, ctx);
    // Clean up
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "DONE produce_output @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: DONE produce_output @ %ld\n", iteration);
    #endif
    #ifdef CNC_DEBUG_LOG
        pthread_mutex_unlock(&_cncDebugMutex);
    #endif
    ocxxr_end_task();
    return NULL_GUID;
}

/* produce_output task creation */
static void cncPrescribeInternal_produce_output(u64 *_tag, luleshCtx *ctx) {
    const cncTag_t iteration = (cncTag_t)_tag[0]; MAYBE_UNUSED(iteration);
    ocrGuid_t _stepGuid;
    u64 _depc = 1 + 1;
    ocrHint_t _hint;
    ocrHint_t *const _hintPtr = _cncCurrentEdtAffinityHint(&_hint);
    ocrEdtCreate(&_stepGuid, ctx->_steps.produce_output,
        /*paramc=*/1, /*paramv=*/_tag,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_hintPtr, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, _CNC_AUX_DATA_MODE);


    { // Set up "final_energy" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = 0;
        cncGet_energy(_i0, _i1, _stepGuid, _edtSlot++, _CNC_ITEM_GET_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PRESCRIBED produce_output @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PRESCRIBED produce_output @ %ld\n", iteration);
    #endif
}

#ifdef CNC_AFFINITIES
static ocrGuid_t _cncRemotePrescribe_produce_output(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    cncPrescribeInternal_produce_output(paramv, ctx);

    return NULL_GUID;
}
#endif /* CNC_AFFINITIES */

/* produce_output task creation */
void cncPrescribe_produce_output(cncTag_t iteration, luleshCtx *ctx) {
    u64 _args[] = { (u64)iteration };
    // affinity
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = iteration % ctx->_affinityCount;
    if (_loc != ctx->_rank) {
        const ocrGuid_t _remoteCtx = ctx->_affinities[_loc];
        const ocrGuid_t _affinity = _cncAffinityFromCtx(_remoteCtx);
        const u32 _argCount = 1;
        const u32 _depCount = 1;
        ocrGuid_t _deps[] = { _remoteCtx };
        // XXX - should just create this template along with the step function template
        ocrGuid_t edtGuid, templGuid;
        ocrEdtTemplateCreate(&templGuid, _cncRemotePrescribe_produce_output, _argCount, _depCount);
        ocrHint_t _hint;
        ocrEdtCreate(&edtGuid, templGuid,
                /*paramc=*/_argCount, /*paramv=*/_args,
                /*depc=*/_depCount, /*depv=*/_deps,
                /*properties=*/EDT_PROP_NONE,
                /*hint=*/_cncEdtAffinityHint(&_hint, _affinity),
                /*outEvent=*/NULL);
        ocrEdtTemplateDestroy(templGuid);
        return;
    }
    #endif /* CNC_AFFINITIES */
    cncPrescribeInternal_produce_output(_args, ctx);
}


