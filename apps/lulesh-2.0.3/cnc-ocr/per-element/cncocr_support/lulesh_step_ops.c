/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"

/* lulesh_finalize setup/teardown function */
ocrGuid_t _cncStep_lulesh_finalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;


    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    final_origin_energyItem final_origin_energy;
    final_origin_energy.item = *(double *)depv[_edtSlot].ptr;
    final_origin_energy.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    lulesh_finalize(final_origin_energy, ctx);

    // Signal that the finalizer is done
    ocrEventSatisfy(ctx->_guids.finalizedEvent, NULL_GUID);

    // Clean up

    return NULL_GUID;
}

/* lulesh_finalize task creation */
void cncPrescribe_lulesh_finalize(luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 *_args = NULL;
    u64 _depc = 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.lulesh_finalize,
        /*paramc=*/0, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "final_origin_energy" input dependencies
        cncGet_final_origin_energy(_stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_stress_partial setup/teardown function */
ocrGuid_t _cncStep_compute_stress_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    pressureItem pressure_in;
    pressure_in.item = *(double *)depv[_edtSlot].ptr;
    pressure_in.handle = depv[_edtSlot++].guid;

    viscosityItem viscosity_in;
    viscosity_in.item = *(double *)depv[_edtSlot].ptr;
    viscosity_in.handle = depv[_edtSlot++].guid;

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_stress_partial(iteration, element_id, pressure_in, viscosity_in, neighbor_position, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);

    return NULL_GUID;
}

/* compute_stress_partial task creation */
void cncPrescribe_compute_stress_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + 1 + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_stress_partial,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "pressure_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_pressure(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "viscosity_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_viscosity(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_hourglass_partial setup/teardown function */
ocrGuid_t _cncStep_compute_hourglass_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    volumeItem element_volume_in;
    element_volume_in.item = *(double *)depv[_edtSlot].ptr;
    element_volume_in.handle = depv[_edtSlot++].guid;

    sound_speedItem sound_speed_in;
    sound_speed_in.item = *(double *)depv[_edtSlot].ptr;
    sound_speed_in.handle = depv[_edtSlot++].guid;

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    neighbor_velocityItem *neighbor_velocity;
    ocrEdtDep_t _block_neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_velocityItem));
        neighbor_velocity = (neighbor_velocityItem *) _block_neighbor_velocity.ptr;
        neighbor_velocityItem *_item = neighbor_velocity;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_hourglass_partial(iteration, element_id, element_volume_in, sound_speed_in, neighbor_position, neighbor_velocity, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);
    CNC_DESTROY_ITEM(_block_neighbor_velocity.guid);

    return NULL_GUID;
}

/* compute_hourglass_partial task creation */
void cncPrescribe_compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + 1 + ((8)-(0)) + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_hourglass_partial,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "element_volume_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "sound_speed_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_sound_speed(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration - 1;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* reduce_force setup/teardown function */
ocrGuid_t _cncStep_reduce_force(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    neighbor_stress_partialItem *neighbor_stress_partial;
    ocrEdtDep_t _block_neighbor_stress_partial;
    { // Init ranges for "neighbor_stress_partial"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_stress_partial = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_stress_partialItem));
        neighbor_stress_partial = (neighbor_stress_partialItem *) _block_neighbor_stress_partial.ptr;
        neighbor_stress_partialItem *_item = neighbor_stress_partial;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    neighbor_hourglass_partialItem *neighbor_hourglass_partial;
    ocrEdtDep_t _block_neighbor_hourglass_partial;
    { // Init ranges for "neighbor_hourglass_partial"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_hourglass_partial = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_hourglass_partialItem));
        neighbor_hourglass_partial = (neighbor_hourglass_partialItem *) _block_neighbor_hourglass_partial.ptr;
        neighbor_hourglass_partialItem *_item = neighbor_hourglass_partial;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    reduce_force(iteration, node_id, neighbor_stress_partial, neighbor_hourglass_partial, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_stress_partial.guid);
    CNC_DESTROY_ITEM(_block_neighbor_hourglass_partial.guid);

    return NULL_GUID;
}

/* reduce_force task creation */
void cncPrescribe_reduce_force(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)node_id };
    u64 _depc = ((8)-(0)) + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.reduce_force,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "neighbor_stress_partial" input dependencies
        s64 _i0 = node_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_stress_partial(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    { // Set up "neighbor_hourglass_partial" input dependencies
        s64 _i0 = node_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_hourglass_partial(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_velocity setup/teardown function */
ocrGuid_t _cncStep_compute_velocity(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    delta_timeItem delta_time_in;
    delta_time_in.item = *(double *)depv[_edtSlot].ptr;
    delta_time_in.handle = depv[_edtSlot++].guid;

    forceItem force_in;
    force_in.item = *(vector *)depv[_edtSlot].ptr;
    force_in.handle = depv[_edtSlot++].guid;

    velocityItem previous_velocity_in;
    previous_velocity_in.item = *(vector *)depv[_edtSlot].ptr;
    previous_velocity_in.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    compute_velocity(iteration, node_id, delta_time_in, force_in, previous_velocity_in, ctx);

    // Clean up

    return NULL_GUID;
}

/* compute_velocity task creation */
void cncPrescribe_compute_velocity(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)node_id };
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_velocity,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "delta_time_in" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "force_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = node_id;
        cncGet_force(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_velocity_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = node_id;
        cncGet_velocity(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_position setup/teardown function */
ocrGuid_t _cncStep_compute_position(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t node_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(node_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    delta_timeItem delta_time_in;
    delta_time_in.item = *(double *)depv[_edtSlot].ptr;
    delta_time_in.handle = depv[_edtSlot++].guid;

    velocityItem velocity_in;
    velocity_in.item = *(vector *)depv[_edtSlot].ptr;
    velocity_in.handle = depv[_edtSlot++].guid;

    positionItem previous_position_in;
    previous_position_in.item = *(vertex *)depv[_edtSlot].ptr;
    previous_position_in.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    compute_position(iteration, node_id, delta_time_in, velocity_in, previous_position_in, ctx);

    // Clean up

    return NULL_GUID;
}

/* compute_position task creation */
void cncPrescribe_compute_position(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)node_id };
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_position,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "delta_time_in" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "velocity_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = node_id;
        cncGet_velocity(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_position_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = node_id;
        cncGet_position(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_volume setup/teardown function */
ocrGuid_t _cncStep_compute_volume(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_volume(iteration, element_id, neighbor_position, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);

    return NULL_GUID;
}

/* compute_volume task creation */
void cncPrescribe_compute_volume(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_volume,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_volume_derivative setup/teardown function */
ocrGuid_t _cncStep_compute_volume_derivative(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    delta_timeItem delta_time_in;
    delta_time_in.item = *(double *)depv[_edtSlot].ptr;
    delta_time_in.handle = depv[_edtSlot++].guid;

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    neighbor_velocityItem *neighbor_velocity;
    ocrEdtDep_t _block_neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_velocityItem));
        neighbor_velocity = (neighbor_velocityItem *) _block_neighbor_velocity.ptr;
        neighbor_velocityItem *_item = neighbor_velocity;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_volume_derivative(iteration, element_id, delta_time_in, neighbor_position, neighbor_velocity, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);
    CNC_DESTROY_ITEM(_block_neighbor_velocity.guid);

    return NULL_GUID;
}

/* compute_volume_derivative task creation */
void cncPrescribe_compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + ((8)-(0)) + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_volume_derivative,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "delta_time_in" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_gradients setup/teardown function */
ocrGuid_t _cncStep_compute_gradients(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    volumeItem volume_in;
    volume_in.item = *(double *)depv[_edtSlot].ptr;
    volume_in.handle = depv[_edtSlot++].guid;

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    neighbor_velocityItem *neighbor_velocity;
    ocrEdtDep_t _block_neighbor_velocity;
    { // Init ranges for "neighbor_velocity"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_velocity = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_velocityItem));
        neighbor_velocity = (neighbor_velocityItem *) _block_neighbor_velocity.ptr;
        neighbor_velocityItem *_item = neighbor_velocity;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_gradients(iteration, element_id, volume_in, neighbor_position, neighbor_velocity, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);
    CNC_DESTROY_ITEM(_block_neighbor_velocity.guid);

    return NULL_GUID;
}

/* compute_gradients task creation */
void cncPrescribe_compute_gradients(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + ((8)-(0)) + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_gradients,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "volume_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    { // Set up "neighbor_velocity" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_viscosity_terms setup/teardown function */
ocrGuid_t _cncStep_compute_viscosity_terms(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    volumeItem volume_in;
    volume_in.item = *(double *)depv[_edtSlot].ptr;
    volume_in.handle = depv[_edtSlot++].guid;

    volume_derivativeItem volume_derivative_in;
    volume_derivative_in.item = *(double *)depv[_edtSlot].ptr;
    volume_derivative_in.handle = depv[_edtSlot++].guid;

    position_gradientItem position_gradient_in;
    position_gradient_in.item = *(vector *)depv[_edtSlot].ptr;
    position_gradient_in.handle = depv[_edtSlot++].guid;

    velocity_gradientItem velocity_gradient_in;
    velocity_gradient_in.item = *(vector *)depv[_edtSlot].ptr;
    velocity_gradient_in.handle = depv[_edtSlot++].guid;

    neighbor_velocity_gradientItem *neighbor_velocity_gradient;
    ocrEdtDep_t _block_neighbor_velocity_gradient;
    { // Init ranges for "neighbor_velocity_gradient"
        u32 _i;
        u32 _itemCount = ((6)-(0));
        u32 _dims[] = { ((6)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_velocity_gradient = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_velocity_gradientItem));
        neighbor_velocity_gradient = (neighbor_velocity_gradientItem *) _block_neighbor_velocity_gradient.ptr;
        neighbor_velocity_gradientItem *_item = neighbor_velocity_gradient;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vector *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_viscosity_terms(iteration, element_id, volume_in, volume_derivative_in, position_gradient_in, velocity_gradient_in, neighbor_velocity_gradient, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_velocity_gradient.guid);

    return NULL_GUID;
}

/* compute_viscosity_terms task creation */
void cncPrescribe_compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + 1 + 1 + 1 + ((6)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_viscosity_terms,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "volume_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "volume_derivative_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume_derivative(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "position_gradient_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_position_gradient(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "velocity_gradient_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_velocity_gradient(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_velocity_gradient" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((6)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_velocity_gradient(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_energy setup/teardown function */
ocrGuid_t _cncStep_compute_energy(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    volumeItem volume_in;
    volume_in.item = *(double *)depv[_edtSlot].ptr;
    volume_in.handle = depv[_edtSlot++].guid;

    volumeItem previous_volume_in;
    previous_volume_in.item = *(double *)depv[_edtSlot].ptr;
    previous_volume_in.handle = depv[_edtSlot++].guid;

    energyItem previous_energy_in;
    previous_energy_in.item = *(double *)depv[_edtSlot].ptr;
    previous_energy_in.handle = depv[_edtSlot++].guid;

    pressureItem previous_pressure_in;
    previous_pressure_in.item = *(double *)depv[_edtSlot].ptr;
    previous_pressure_in.handle = depv[_edtSlot++].guid;

    viscosityItem previous_viscosity_in;
    previous_viscosity_in.item = *(double *)depv[_edtSlot].ptr;
    previous_viscosity_in.handle = depv[_edtSlot++].guid;

    linear_viscosity_termItem qlin_in;
    qlin_in.item = *(double *)depv[_edtSlot].ptr;
    qlin_in.handle = depv[_edtSlot++].guid;

    quadratic_viscosity_termItem qquad_in;
    qquad_in.item = *(double *)depv[_edtSlot].ptr;
    qquad_in.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    compute_energy(iteration, element_id, volume_in, previous_volume_in, previous_energy_in, previous_pressure_in, previous_viscosity_in, qlin_in, qquad_in, ctx);

    // Clean up

    return NULL_GUID;
}

/* compute_energy task creation */
void cncPrescribe_compute_energy(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_energy,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "volume_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_volume_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_energy_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_energy(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_pressure_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_pressure(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_viscosity_in" input dependencies
        s64 _i0 = iteration - 1;
        s64 _i1 = element_id;
        cncGet_viscosity(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "qlin_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_linear_viscosity_term(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "qquad_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_quadratic_viscosity_term(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_characteristic_length setup/teardown function */
ocrGuid_t _cncStep_compute_characteristic_length(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    volumeItem volume_in;
    volume_in.item = *(double *)depv[_edtSlot].ptr;
    volume_in.handle = depv[_edtSlot++].guid;

    neighbor_positionItem *neighbor_position;
    ocrEdtDep_t _block_neighbor_position;
    { // Init ranges for "neighbor_position"
        u32 _i;
        u32 _itemCount = ((8)-(0));
        u32 _dims[] = { ((8)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_neighbor_position = _cncRangedInputAlloc(1, _dims, sizeof(neighbor_positionItem));
        neighbor_position = (neighbor_positionItem *) _block_neighbor_position.ptr;
        neighbor_positionItem *_item = neighbor_position;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(vertex *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_characteristic_length(iteration, element_id, volume_in, neighbor_position, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_neighbor_position.guid);

    return NULL_GUID;
}

/* compute_characteristic_length task creation */
void cncPrescribe_compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + ((8)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_characteristic_length,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "volume_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "neighbor_position" input dependencies
        s64 _i0 = element_id;
        s64 _i1;
        for (_i1 = 0; _i1 < ((8)-(0)); _i1++) {
            s64 _i2 = iteration;
            cncGet_neighbor_position(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_time_constraints setup/teardown function */
ocrGuid_t _cncStep_compute_time_constraints(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);
    const cncTag_t element_id = (cncTag_t)paramv[1]; MAYBE_UNUSED(element_id);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    sound_speedItem sound_speed_in;
    sound_speed_in.item = *(double *)depv[_edtSlot].ptr;
    sound_speed_in.handle = depv[_edtSlot++].guid;

    volume_derivativeItem volume_derivative_in;
    volume_derivative_in.item = *(double *)depv[_edtSlot].ptr;
    volume_derivative_in.handle = depv[_edtSlot++].guid;

    characteristic_lengthItem characteristic_length_in;
    characteristic_length_in.item = *(double *)depv[_edtSlot].ptr;
    characteristic_length_in.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    compute_time_constraints(iteration, element_id, sound_speed_in, volume_derivative_in, characteristic_length_in, ctx);

    // Clean up

    return NULL_GUID;
}

/* compute_time_constraints task creation */
void cncPrescribe_compute_time_constraints(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration, (u64)element_id };
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_time_constraints,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "sound_speed_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_sound_speed(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "volume_derivative_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_volume_derivative(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "characteristic_length_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = element_id;
        cncGet_characteristic_length(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* compute_delta_time setup/teardown function */
ocrGuid_t _cncStep_compute_delta_time(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    delta_timeItem previous_delta_time_in;
    previous_delta_time_in.item = *(double *)depv[_edtSlot].ptr;
    previous_delta_time_in.handle = depv[_edtSlot++].guid;

    elapsed_timeItem previous_elapsed_time_in;
    previous_elapsed_time_in.item = *(double *)depv[_edtSlot].ptr;
    previous_elapsed_time_in.handle = depv[_edtSlot++].guid;

    courantItem *courant_in;
    ocrEdtDep_t _block_courant_in;
    { // Init ranges for "courant_in"
        u32 _i;
        u32 _itemCount = ((ELEMENTS)-(0));
        u32 _dims[] = { ((ELEMENTS)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_courant_in = _cncRangedInputAlloc(1, _dims, sizeof(courantItem));
        courant_in = (courantItem *) _block_courant_in.ptr;
        courantItem *_item = courant_in;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(double *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    hydroItem *hydro_in;
    ocrEdtDep_t _block_hydro_in;
    { // Init ranges for "hydro_in"
        u32 _i;
        u32 _itemCount = ((ELEMENTS)-(0));
        u32 _dims[] = { ((ELEMENTS)-(0)) };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        _block_hydro_in = _cncRangedInputAlloc(1, _dims, sizeof(hydroItem));
        hydro_in = (hydroItem *) _block_hydro_in.ptr;
        hydroItem *_item = hydro_in;
        for (_i=0; _i<_itemCount; _i++, _item++) {
            _item->item = *(double *)depv[_edtSlot].ptr;
            _item->handle = depv[_edtSlot++].guid;
        }
    }

    // Call user-defined step function

    compute_delta_time(iteration, previous_delta_time_in, previous_elapsed_time_in, courant_in, hydro_in, ctx);

    // Clean up
    CNC_DESTROY_ITEM(_block_courant_in.guid);
    CNC_DESTROY_ITEM(_block_hydro_in.guid);

    return NULL_GUID;
}

/* compute_delta_time task creation */
void cncPrescribe_compute_delta_time(cncTag_t iteration, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration };
    u64 _depc = 1 + 1 + ((ELEMENTS)-(0)) + ((ELEMENTS)-(0)) + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.compute_delta_time,
        /*paramc=*/1, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "previous_delta_time_in" input dependencies
        s64 _i0 = iteration;
        cncGet_delta_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "previous_elapsed_time_in" input dependencies
        s64 _i0 = iteration;
        cncGet_elapsed_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "courant_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1;
        for (_i1 = 0; _i1 < ((ELEMENTS)-(0)); _i1++) {
            cncGet_courant(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    { // Set up "hydro_in" input dependencies
        s64 _i0 = iteration;
        s64 _i1;
        for (_i1 = 0; _i1 < ((ELEMENTS)-(0)); _i1++) {
            cncGet_hydro(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* produce_output setup/teardown function */
ocrGuid_t _cncStep_produce_output(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    luleshCtx *ctx = depv[0].ptr;

    const cncTag_t iteration = (cncTag_t)paramv[0]; MAYBE_UNUSED(iteration);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    energyItem final_energy;
    final_energy.item = *(double *)depv[_edtSlot].ptr;
    final_energy.handle = depv[_edtSlot++].guid;

    // Call user-defined step function

    produce_output(iteration, final_energy, ctx);

    // Clean up

    return NULL_GUID;
}

/* produce_output task creation */
void cncPrescribe_produce_output(cncTag_t iteration, luleshCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iteration };
    u64 _depc = 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.produce_output,
        /*paramc=*/1, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);

    { // Set up "final_energy" input dependencies
        s64 _i0 = iteration;
        s64 _i1 = 0;
        cncGet_energy(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}


