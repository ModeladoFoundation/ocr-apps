/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNC_LULESH_H_
#define _CNC_LULESH_H_
#include "lulesh_context.h"
#include "cnc_common.h"

/***************************\
 ******** CNC GRAPH ********
\***************************/

void lulesh_ctx_init(ocrGuid_t contextGuid, luleshCtx *ctx);
void lulesh_destroy(luleshCtx *ctx);

void lulesh_launch(luleshArgs *args, luleshCtx *ctx);
void lulesh_await(luleshCtx *ctx);

/**********************************\
 ******** ITEM KEY STRUCTS ********
\**********************************/

typedef struct { cncTag_t iteration, map_id; } stress_partialItemKey;
typedef struct { cncTag_t iteration, map_id; } hourglass_partialItemKey;
typedef struct { cncTag_t iteration, node_id; } forceItemKey;
typedef struct { cncTag_t iteration, node_id; } positionItemKey;
typedef struct { cncTag_t iteration, node_id; } velocityItemKey;
typedef struct { cncTag_t iteration, element_id; } volumeItemKey;
typedef struct { cncTag_t iteration, element_id; } volume_derivativeItemKey;
typedef struct { cncTag_t iteration, element_id; } characteristic_lengthItemKey;
typedef struct { cncTag_t iteration, element_id; } velocity_gradientItemKey;
typedef struct { cncTag_t iteration, element_id; } position_gradientItemKey;
typedef struct { cncTag_t iteration, element_id; } quadratic_viscosity_termItemKey;
typedef struct { cncTag_t iteration, element_id; } linear_viscosity_termItemKey;
typedef struct { cncTag_t iteration, element_id; } sound_speedItemKey;
typedef struct { cncTag_t iteration, element_id; } viscosityItemKey;
typedef struct { cncTag_t iteration, element_id; } pressureItemKey;
typedef struct { cncTag_t iteration, element_id; } energyItemKey;
typedef struct { cncTag_t iteration, element_id; } courantItemKey;
typedef struct { cncTag_t iteration, element_id; } hydroItemKey;
typedef struct { cncTag_t iteration; } delta_timeItemKey;
typedef struct { cncTag_t iteration; } elapsed_timeItemKey;
typedef struct { cncTag_t element_id, local_node_id, iteration; } neighbor_velocityItemKey;
typedef struct { cncTag_t element_id, local_node_id, iteration; } neighbor_positionItemKey;
typedef struct { cncTag_t node_id, local_element_id, iteration; } neighbor_stress_partialItemKey;
typedef struct { cncTag_t node_id, local_element_id, iteration; } neighbor_hourglass_partialItemKey;
typedef struct { cncTag_t element_id, local_element_id, iteration; } neighbor_velocity_gradientItemKey;

/****************************************\
 ******** ITEM MAPPING FUNCTIONS ********
\****************************************/

velocityItemKey get_elements_node_neighbors_for_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx);
positionItemKey get_elements_node_neighbors_for_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx);
stress_partialItemKey get_map_id_for_node_element_neighbors_for_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);
hourglass_partialItemKey get_map_id_for_node_element_neighbors_for_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);
velocity_gradientItemKey get_elements_element_neighbors_for_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);

/**************************\
 ******** ITEM PUT ********
\**************************/

// stress_partial

void cncPut_stress_partial(vector *_item, cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx);

// hourglass_partial

void cncPut_hourglass_partial(vector *_item, cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx);

// force

void cncPut_force(vector *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);

// position

void cncPut_position(vertex *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);

// velocity

void cncPut_velocity(vector *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);

// volume

void cncPut_volume(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// volume_derivative

void cncPut_volume_derivative(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// characteristic_length

void cncPut_characteristic_length(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// velocity_gradient

void cncPut_velocity_gradient(vector *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// position_gradient

void cncPut_position_gradient(vector *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// quadratic_viscosity_term

void cncPut_quadratic_viscosity_term(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// linear_viscosity_term

void cncPut_linear_viscosity_term(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// sound_speed

void cncPut_sound_speed(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// viscosity

void cncPut_viscosity(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// pressure

void cncPut_pressure(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// energy

void cncPut_energy(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// courant

void cncPut_courant(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// hydro

void cncPut_hydro(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);

// delta_time

void cncPut_delta_time(double *_item, cncTag_t iteration, luleshCtx *ctx);

// elapsed_time

void cncPut_elapsed_time(double *_item, cncTag_t iteration, luleshCtx *ctx);

// neighbor_velocity

void cncPut_neighbor_velocity(vector *_item, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx);

// neighbor_position

void cncPut_neighbor_position(vertex *_item, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx);

// neighbor_stress_partial

void cncPut_neighbor_stress_partial(vector *_item, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);

// neighbor_hourglass_partial

void cncPut_neighbor_hourglass_partial(vector *_item, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);

// neighbor_velocity_gradient

void cncPut_neighbor_velocity_gradient(vector *_item, cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx);

// final_origin_energy

void cncPut_final_origin_energy(double *_item, luleshCtx *ctx);

/************************************\
 ******** STEP PRESCRIPTIONS ********
\************************************/

void cncPrescribe_cncFinalize(luleshCtx *ctx);
void cncPrescribe_compute_stress_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_reduce_force(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);
void cncPrescribe_compute_velocity(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);
void cncPrescribe_compute_position(cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx);
void cncPrescribe_compute_volume(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_gradients(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_energy(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_time_constraints(cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx);
void cncPrescribe_compute_delta_time(cncTag_t iteration, luleshCtx *ctx);
void cncPrescribe_produce_output(cncTag_t iteration, luleshCtx *ctx);


#endif /*_CNC_LULESH_H_*/
