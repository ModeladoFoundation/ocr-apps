/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"


/* stress_partial */

cncHandle_t cncCreateItemSized_stress_partial(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_stress_partial(cncHandle_t handle, cncTag_t iteration, cncTag_t map_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, map_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.stress_partial, checkSingleAssignment);
}

void cncGet_stress_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, map_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.stress_partial);
}


/* hourglass_partial */

cncHandle_t cncCreateItemSized_hourglass_partial(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_hourglass_partial(cncHandle_t handle, cncTag_t iteration, cncTag_t map_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, map_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.hourglass_partial, checkSingleAssignment);
}

void cncGet_hourglass_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, map_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.hourglass_partial);
}


/* force */

cncHandle_t cncCreateItemSized_force(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_force(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.force, checkSingleAssignment);
}

void cncGet_force(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.force);
}


/* position */

cncHandle_t cncCreateItemSized_position(vertex **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_position(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.position, checkSingleAssignment);
}

void cncGet_position(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.position);
}


/* velocity */

cncHandle_t cncCreateItemSized_velocity(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_velocity(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.velocity, checkSingleAssignment);
}

void cncGet_velocity(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, node_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.velocity);
}


/* volume */

cncHandle_t cncCreateItemSized_volume(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_volume(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.volume, checkSingleAssignment);
}

void cncGet_volume(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.volume);
}


/* volume_derivative */

cncHandle_t cncCreateItemSized_volume_derivative(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_volume_derivative(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.volume_derivative, checkSingleAssignment);
}

void cncGet_volume_derivative(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.volume_derivative);
}


/* characteristic_length */

cncHandle_t cncCreateItemSized_characteristic_length(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_characteristic_length(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.characteristic_length, checkSingleAssignment);
}

void cncGet_characteristic_length(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.characteristic_length);
}


/* velocity_gradient */

cncHandle_t cncCreateItemSized_velocity_gradient(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_velocity_gradient(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.velocity_gradient, checkSingleAssignment);
}

void cncGet_velocity_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.velocity_gradient);
}


/* position_gradient */

cncHandle_t cncCreateItemSized_position_gradient(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_position_gradient(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.position_gradient, checkSingleAssignment);
}

void cncGet_position_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.position_gradient);
}


/* quadratic_viscosity_term */

cncHandle_t cncCreateItemSized_quadratic_viscosity_term(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_quadratic_viscosity_term(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.quadratic_viscosity_term, checkSingleAssignment);
}

void cncGet_quadratic_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.quadratic_viscosity_term);
}


/* linear_viscosity_term */

cncHandle_t cncCreateItemSized_linear_viscosity_term(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_linear_viscosity_term(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.linear_viscosity_term, checkSingleAssignment);
}

void cncGet_linear_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.linear_viscosity_term);
}


/* sound_speed */

cncHandle_t cncCreateItemSized_sound_speed(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_sound_speed(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.sound_speed, checkSingleAssignment);
}

void cncGet_sound_speed(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.sound_speed);
}


/* viscosity */

cncHandle_t cncCreateItemSized_viscosity(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_viscosity(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.viscosity, checkSingleAssignment);
}

void cncGet_viscosity(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.viscosity);
}


/* pressure */

cncHandle_t cncCreateItemSized_pressure(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_pressure(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.pressure, checkSingleAssignment);
}

void cncGet_pressure(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.pressure);
}


/* energy */

cncHandle_t cncCreateItemSized_energy(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_energy(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.energy, checkSingleAssignment);
}

void cncGet_energy(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.energy);
}


/* courant */

cncHandle_t cncCreateItemSized_courant(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_courant(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.courant, checkSingleAssignment);
}

void cncGet_courant(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.courant);
}


/* hydro */

cncHandle_t cncCreateItemSized_hydro(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_hydro(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.hydro, checkSingleAssignment);
}

void cncGet_hydro(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration, element_id };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.hydro);
}


/* delta_time */

cncHandle_t cncCreateItemSized_delta_time(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_delta_time(cncHandle_t handle, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.delta_time, checkSingleAssignment);
}

void cncGet_delta_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.delta_time);
}


/* elapsed_time */

cncHandle_t cncCreateItemSized_elapsed_time(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_elapsed_time(cncHandle_t handle, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration };
    _cncPut(handle, (unsigned char*)tag, sizeof(tag), ctx->_items.elapsed_time, checkSingleAssignment);
}

void cncGet_elapsed_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    cncTag_t tag[] = { iteration };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.elapsed_time);
}


/* neighbor_velocity */

cncHandle_t cncCreateItemSized_neighbor_velocity(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_neighbor_velocity(cncHandle_t handle, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {
    velocityItemKey _key = get_elements_node_neighbors_for_velocity(element_id, local_node_id, iteration, ctx);
    cncPutChecked_velocity(handle, _key.iteration, _key.node_id, checkSingleAssignment, ctx);
}

void cncGet_neighbor_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {
    velocityItemKey _key = get_elements_node_neighbors_for_velocity(element_id, local_node_id, iteration, ctx);
    cncGet_velocity(_key.iteration, _key.node_id, destination, slot, mode, ctx);
}


/* neighbor_position */

cncHandle_t cncCreateItemSized_neighbor_position(vertex **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_neighbor_position(cncHandle_t handle, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {
    positionItemKey _key = get_elements_node_neighbors_for_position(element_id, local_node_id, iteration, ctx);
    cncPutChecked_position(handle, _key.iteration, _key.node_id, checkSingleAssignment, ctx);
}

void cncGet_neighbor_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {
    positionItemKey _key = get_elements_node_neighbors_for_position(element_id, local_node_id, iteration, ctx);
    cncGet_position(_key.iteration, _key.node_id, destination, slot, mode, ctx);
}


/* neighbor_stress_partial */

cncHandle_t cncCreateItemSized_neighbor_stress_partial(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_neighbor_stress_partial(cncHandle_t handle, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {
    stress_partialItemKey _key = get_map_id_for_node_element_neighbors_for_stress_partial(node_id, local_element_id, iteration, ctx);
    cncPutChecked_stress_partial(handle, _key.iteration, _key.map_id, checkSingleAssignment, ctx);
}

void cncGet_neighbor_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {
    stress_partialItemKey _key = get_map_id_for_node_element_neighbors_for_stress_partial(node_id, local_element_id, iteration, ctx);
    cncGet_stress_partial(_key.iteration, _key.map_id, destination, slot, mode, ctx);
}


/* neighbor_hourglass_partial */

cncHandle_t cncCreateItemSized_neighbor_hourglass_partial(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_neighbor_hourglass_partial(cncHandle_t handle, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {
    hourglass_partialItemKey _key = get_map_id_for_node_element_neighbors_for_hourglass_partial(node_id, local_element_id, iteration, ctx);
    cncPutChecked_hourglass_partial(handle, _key.iteration, _key.map_id, checkSingleAssignment, ctx);
}

void cncGet_neighbor_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {
    hourglass_partialItemKey _key = get_map_id_for_node_element_neighbors_for_hourglass_partial(node_id, local_element_id, iteration, ctx);
    cncGet_hourglass_partial(_key.iteration, _key.map_id, destination, slot, mode, ctx);
}


/* neighbor_velocity_gradient */

cncHandle_t cncCreateItemSized_neighbor_velocity_gradient(vector **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_neighbor_velocity_gradient(cncHandle_t handle, cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *ctx) {
    velocity_gradientItemKey _key = get_elements_element_neighbors_for_velocity_gradient(element_id, local_element_id, iteration, ctx);
    cncPutChecked_velocity_gradient(handle, _key.iteration, _key.element_id, checkSingleAssignment, ctx);
}

void cncGet_neighbor_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {
    velocity_gradientItemKey _key = get_elements_element_neighbors_for_velocity_gradient(element_id, local_element_id, iteration, ctx);
    cncGet_velocity_gradient(_key.iteration, _key.element_id, destination, slot, mode, ctx);
}


/* final_origin_energy */

cncHandle_t cncCreateItemSized_final_origin_energy(double **item, size_t size) {
    cncHandle_t handle;
    // XXX - do I need to check for busy (and do a retry)?
    CNC_CREATE_ITEM(&handle, (void**)item, size);
    return handle;
}

void cncPutChecked_final_origin_energy(cncHandle_t handle, bool checkSingleAssignment, luleshCtx *ctx) {

    _cncPutSingleton(handle, ctx->_items.final_origin_energy, checkSingleAssignment);
}

void cncGet_final_origin_energy(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx) {

    return _cncGetSingleton(destination, slot, mode, ctx->_items.final_origin_energy);
}


