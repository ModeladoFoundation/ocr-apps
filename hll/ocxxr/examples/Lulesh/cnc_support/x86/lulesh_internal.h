/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_LULESH_INTERNAL_H_
#define _CNCOCR_LULESH_INTERNAL_H_

#include "lulesh.h"
#include "cncocr_internal.h"

/********************************\
 ******** ITEM FUNCTIONS ********
\********************************/


void cncGet_stress_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_hourglass_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_force(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_position(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_velocity(cncTag_t iteration, cncTag_t node_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_volume(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_volume_derivative(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_characteristic_length(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_velocity_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_position_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_quadratic_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_linear_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_sound_speed(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_viscosity(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_pressure(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_energy(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_courant(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_hydro(cncTag_t iteration, cncTag_t element_id, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_delta_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_elapsed_time(cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_neighbor_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_neighbor_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_neighbor_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_neighbor_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_neighbor_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);

void cncGet_final_origin_energy(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, luleshCtx *ctx);


#ifdef CNC_AFFINITIES
static inline cncLocation_t _cncItemDistFn_stress_partial(cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx) { return map_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_hourglass_partial(cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx) { return map_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_force(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) { return node_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_position(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) { return node_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_velocity(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) { return node_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_volume(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_volume_derivative(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_characteristic_length(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_velocity_gradient(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_position_gradient(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_quadratic_viscosity_term(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_linear_viscosity_term(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_sound_speed(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_viscosity(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_pressure(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_energy(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_courant(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_hydro(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) { return element_id % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_delta_time(cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_elapsed_time(cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_neighbor_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_neighbor_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_neighbor_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_neighbor_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_neighbor_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) { return iteration % ctx->_affinityCount; }
static inline cncLocation_t _cncItemDistFn_final_origin_energy(luleshCtx *ctx) { return 25 % ctx->_affinityCount; }
#endif /* CNC_AFFINITIES */



/********************************\
 ******** STEP FUNCTIONS ********
\********************************/

void lulesh_cncInitialize(luleshArgs *args, luleshCtx *ctx);

void lulesh_cncFinalize(double final_origin_energy, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_cncFinalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_stress_partial(cncTag_t iteration, cncTag_t element_id, double pressure, double viscosity, vertex *neighbor_position, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_stress_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, double element_volume, double sound_speed, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_hourglass_partial(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_reduce_force(cncTag_t iteration, cncTag_t node_id, vector *neighbor_stress_partial, vector *neighbor_hourglass_partial, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_reduce_force(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_velocity(cncTag_t iteration, cncTag_t node_id, double delta_time, vector force, vector previous_velocity, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_velocity(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_position(cncTag_t iteration, cncTag_t node_id, double delta_time, vector velocity, vertex previous_position, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_position(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_volume(cncTag_t iteration, cncTag_t element_id, vertex *neighbor_position, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_volume(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, double delta_time, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_volume_derivative(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_gradients(cncTag_t iteration, cncTag_t element_id, double volume, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_gradients(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, double volume, double volume_derivative, vector position_gradient, vector velocity_gradient, vector *neighbor_velocity_gradient, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_viscosity_terms(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_energy(cncTag_t iteration, cncTag_t element_id, double volume, double previous_volume, double previous_energy, double previous_pressure, double previous_viscosity, double qlin, double qquad, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_energy(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, double volume, vertex *neighbor_position, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_characteristic_length(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_time_constraints(cncTag_t iteration, cncTag_t element_id, double sound_speed, double volume_derivative, double characteristic_length, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_time_constraints(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_compute_delta_time(cncTag_t iteration, double previous_delta_time, double previous_elapsed_time, double *courant, double *hydro, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_compute_delta_time(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void lulesh_produce_output(cncTag_t iteration, double final_energy, luleshCtx *ctx);
ocrGuid_t _lulesh_cncStep_produce_output(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

#endif /*_CNCOCR_LULESH_INTERNAL_H_*/
