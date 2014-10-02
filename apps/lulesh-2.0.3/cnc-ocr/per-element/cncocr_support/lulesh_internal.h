/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_LULESH_INTERNAL_H_
#define _CNCOCR_LULESH_INTERNAL_H_

#include "lulesh.h"
#include "cncocr_internal.h"

/******************************\
 ******** ITEM GETTERS ********
\******************************/


void cncGet_stress_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_hourglass_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_force(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_position(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_velocity(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_volume(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_volume_derivative(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_characteristic_length(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_velocity_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_position_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_quadratic_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_linear_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_sound_speed(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_viscosity(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_pressure(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_energy(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_courant(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_hydro(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_delta_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_elapsed_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_neighbor_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_neighbor_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_neighbor_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_neighbor_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_neighbor_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);

void cncGet_final_origin_energy(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *context);


/********************************\
 ******** STEP FUNCTIONS ********
\********************************/

void lulesh_init(luleshArgs *args, luleshCtx *ctx);

void lulesh_finalize(final_origin_energyItem final_origin_energy, luleshCtx *ctx);
ocrGuid_t _cncStep_lulesh_finalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_stress_partial(cncTag_t iteration, cncTag_t element_id, pressureItem pressure_in, viscosityItem viscosity_in, neighbor_positionItem *neighbor_position, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_stress_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, volumeItem element_volume_in, sound_speedItem sound_speed_in, neighbor_positionItem *neighbor_position, neighbor_velocityItem *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_hourglass_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void reduce_force(cncTag_t iteration, cncTag_t node_id, neighbor_stress_partialItem *neighbor_stress_partial, neighbor_hourglass_partialItem *neighbor_hourglass_partial, luleshCtx *ctx);
ocrGuid_t _cncStep_reduce_force(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_velocity(cncTag_t iteration, cncTag_t node_id, delta_timeItem delta_time_in, forceItem force_in, velocityItem previous_velocity_in, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_velocity(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_position(cncTag_t iteration, cncTag_t node_id, delta_timeItem delta_time_in, velocityItem velocity_in, positionItem previous_position_in, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_position(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_volume(cncTag_t iteration, cncTag_t element_id, neighbor_positionItem *neighbor_position, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_volume(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, delta_timeItem delta_time_in, neighbor_positionItem *neighbor_position, neighbor_velocityItem *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_volume_derivative(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_gradients(cncTag_t iteration, cncTag_t element_id, volumeItem volume_in, neighbor_positionItem *neighbor_position, neighbor_velocityItem *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_gradients(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, volumeItem volume_in, volume_derivativeItem volume_derivative_in, position_gradientItem position_gradient_in, velocity_gradientItem velocity_gradient_in, neighbor_velocity_gradientItem *neighbor_velocity_gradient, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_viscosity_terms(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_energy(cncTag_t iteration, cncTag_t element_id, volumeItem volume_in, volumeItem previous_volume_in, energyItem previous_energy_in, pressureItem previous_pressure_in, viscosityItem previous_viscosity_in, linear_viscosity_termItem qlin_in, quadratic_viscosity_termItem qquad_in, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_energy(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, volumeItem volume_in, neighbor_positionItem *neighbor_position, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_characteristic_length(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_time_constraints(cncTag_t iteration, cncTag_t element_id, sound_speedItem sound_speed_in, volume_derivativeItem volume_derivative_in, characteristic_lengthItem characteristic_length_in, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_time_constraints(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void compute_delta_time(cncTag_t iteration, delta_timeItem previous_delta_time_in, elapsed_timeItem previous_elapsed_time_in, courantItem *courant_in, hydroItem *hydro_in, luleshCtx *ctx);
ocrGuid_t _cncStep_compute_delta_time(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void produce_output(cncTag_t iteration, energyItem final_energy, luleshCtx *ctx);
ocrGuid_t _cncStep_produce_output(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

#endif /*_CNCOCR_LULESH_INTERNAL_H_*/
