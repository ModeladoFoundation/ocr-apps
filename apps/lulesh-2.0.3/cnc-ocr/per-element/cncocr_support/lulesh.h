/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_LULESH_H_
#define _CNCOCR_LULESH_H_

#include "cncocr.h"
#include "lulesh_defs.h"

/***************************\
 ******** CNC GRAPH ********
\***************************/

typedef struct luleshContext {
    struct {
        ocrGuid_t self;
        ocrGuid_t finalizedEvent;
        ocrGuid_t doneEvent;
        ocrGuid_t awaitTag;
    } _guids;
    struct {
        cncItemCollection_t stress_partial;
        cncItemCollection_t hourglass_partial;
        cncItemCollection_t force;
        cncItemCollection_t position;
        cncItemCollection_t velocity;
        cncItemCollection_t volume;
        cncItemCollection_t volume_derivative;
        cncItemCollection_t characteristic_length;
        cncItemCollection_t velocity_gradient;
        cncItemCollection_t position_gradient;
        cncItemCollection_t quadratic_viscosity_term;
        cncItemCollection_t linear_viscosity_term;
        cncItemCollection_t sound_speed;
        cncItemCollection_t viscosity;
        cncItemCollection_t pressure;
        cncItemCollection_t energy;
        cncItemCollection_t courant;
        cncItemCollection_t hydro;
        cncItemCollection_t delta_time;
        cncItemCollection_t elapsed_time;
        cncHandle_t final_origin_energy;
    } _items;
    struct {
        ocrGuid_t lulesh_finalize;
        ocrGuid_t compute_stress_partial;
        ocrGuid_t compute_hourglass_partial;
        ocrGuid_t reduce_force;
        ocrGuid_t compute_velocity;
        ocrGuid_t compute_position;
        ocrGuid_t compute_volume;
        ocrGuid_t compute_volume_derivative;
        ocrGuid_t compute_gradients;
        ocrGuid_t compute_viscosity_terms;
        ocrGuid_t compute_energy;
        ocrGuid_t compute_characteristic_length;
        ocrGuid_t compute_time_constraints;
        ocrGuid_t compute_delta_time;
        ocrGuid_t produce_output;
    } _steps;
    struct constraints constraints;
    struct constants constants;
    struct cutoffs cutoffs;
    struct domain domain;
    struct mesh mesh;
} luleshCtx;

luleshCtx *lulesh_create();
void lulesh_destroy(luleshCtx *context);

void lulesh_launch(luleshArgs *args, luleshCtx *ctx);
void lulesh_await(luleshCtx *context);

/****************************\
 ******** ITEM TYPES ********
\****************************/

typedef struct { vector item; cncHandle_t handle; } stress_partialItem;
typedef struct { vector item; cncHandle_t handle; } hourglass_partialItem;
typedef struct { vector item; cncHandle_t handle; } forceItem;
typedef struct { vertex item; cncHandle_t handle; } positionItem;
typedef struct { vector item; cncHandle_t handle; } velocityItem;
typedef struct { double item; cncHandle_t handle; } volumeItem;
typedef struct { double item; cncHandle_t handle; } volume_derivativeItem;
typedef struct { double item; cncHandle_t handle; } characteristic_lengthItem;
typedef struct { vector item; cncHandle_t handle; } velocity_gradientItem;
typedef struct { vector item; cncHandle_t handle; } position_gradientItem;
typedef struct { double item; cncHandle_t handle; } quadratic_viscosity_termItem;
typedef struct { double item; cncHandle_t handle; } linear_viscosity_termItem;
typedef struct { double item; cncHandle_t handle; } sound_speedItem;
typedef struct { double item; cncHandle_t handle; } viscosityItem;
typedef struct { double item; cncHandle_t handle; } pressureItem;
typedef struct { double item; cncHandle_t handle; } energyItem;
typedef struct { double item; cncHandle_t handle; } courantItem;
typedef struct { double item; cncHandle_t handle; } hydroItem;
typedef struct { double item; cncHandle_t handle; } delta_timeItem;
typedef struct { double item; cncHandle_t handle; } elapsed_timeItem;
typedef struct { vector item; cncHandle_t handle; } neighbor_velocityItem;
typedef struct { vertex item; cncHandle_t handle; } neighbor_positionItem;
typedef struct { vector item; cncHandle_t handle; } neighbor_stress_partialItem;
typedef struct { vector item; cncHandle_t handle; } neighbor_hourglass_partialItem;
typedef struct { vector item; cncHandle_t handle; } neighbor_velocity_gradientItem;
typedef struct { double item; cncHandle_t handle; } final_origin_energyItem;

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

/*****************************\
 ******** ITEM CREATE ********
\*****************************/

cncHandle_t cncCreateItemSized_stress_partial(vector **item, size_t size);
#define cncCreateItem_stress_partial(ptrptr) \
   cncCreateItemSized_stress_partial(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_hourglass_partial(vector **item, size_t size);
#define cncCreateItem_hourglass_partial(ptrptr) \
   cncCreateItemSized_hourglass_partial(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_force(vector **item, size_t size);
#define cncCreateItem_force(ptrptr) \
   cncCreateItemSized_force(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_position(vertex **item, size_t size);
#define cncCreateItem_position(ptrptr) \
   cncCreateItemSized_position(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_velocity(vector **item, size_t size);
#define cncCreateItem_velocity(ptrptr) \
   cncCreateItemSized_velocity(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_volume(double **item, size_t size);
#define cncCreateItem_volume(ptrptr) \
   cncCreateItemSized_volume(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_volume_derivative(double **item, size_t size);
#define cncCreateItem_volume_derivative(ptrptr) \
   cncCreateItemSized_volume_derivative(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_characteristic_length(double **item, size_t size);
#define cncCreateItem_characteristic_length(ptrptr) \
   cncCreateItemSized_characteristic_length(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_velocity_gradient(vector **item, size_t size);
#define cncCreateItem_velocity_gradient(ptrptr) \
   cncCreateItemSized_velocity_gradient(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_position_gradient(vector **item, size_t size);
#define cncCreateItem_position_gradient(ptrptr) \
   cncCreateItemSized_position_gradient(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_quadratic_viscosity_term(double **item, size_t size);
#define cncCreateItem_quadratic_viscosity_term(ptrptr) \
   cncCreateItemSized_quadratic_viscosity_term(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_linear_viscosity_term(double **item, size_t size);
#define cncCreateItem_linear_viscosity_term(ptrptr) \
   cncCreateItemSized_linear_viscosity_term(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_sound_speed(double **item, size_t size);
#define cncCreateItem_sound_speed(ptrptr) \
   cncCreateItemSized_sound_speed(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_viscosity(double **item, size_t size);
#define cncCreateItem_viscosity(ptrptr) \
   cncCreateItemSized_viscosity(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_pressure(double **item, size_t size);
#define cncCreateItem_pressure(ptrptr) \
   cncCreateItemSized_pressure(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_energy(double **item, size_t size);
#define cncCreateItem_energy(ptrptr) \
   cncCreateItemSized_energy(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_courant(double **item, size_t size);
#define cncCreateItem_courant(ptrptr) \
   cncCreateItemSized_courant(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_hydro(double **item, size_t size);
#define cncCreateItem_hydro(ptrptr) \
   cncCreateItemSized_hydro(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_delta_time(double **item, size_t size);
#define cncCreateItem_delta_time(ptrptr) \
   cncCreateItemSized_delta_time(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_elapsed_time(double **item, size_t size);
#define cncCreateItem_elapsed_time(ptrptr) \
   cncCreateItemSized_elapsed_time(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_neighbor_velocity(vector **item, size_t size);
#define cncCreateItem_neighbor_velocity(ptrptr) \
   cncCreateItemSized_neighbor_velocity(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_neighbor_position(vertex **item, size_t size);
#define cncCreateItem_neighbor_position(ptrptr) \
   cncCreateItemSized_neighbor_position(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_neighbor_stress_partial(vector **item, size_t size);
#define cncCreateItem_neighbor_stress_partial(ptrptr) \
   cncCreateItemSized_neighbor_stress_partial(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_neighbor_hourglass_partial(vector **item, size_t size);
#define cncCreateItem_neighbor_hourglass_partial(ptrptr) \
   cncCreateItemSized_neighbor_hourglass_partial(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_neighbor_velocity_gradient(vector **item, size_t size);
#define cncCreateItem_neighbor_velocity_gradient(ptrptr) \
   cncCreateItemSized_neighbor_velocity_gradient(ptrptr, sizeof(**(ptrptr)))

cncHandle_t cncCreateItemSized_final_origin_energy(double **item, size_t size);
#define cncCreateItem_final_origin_energy(ptrptr) \
   cncCreateItemSized_final_origin_energy(ptrptr, sizeof(**(ptrptr)))

/**************************\
 ******** ITEM PUT ********
\**************************/

void cncPutChecked_stress_partial(cncHandle_t handle, cncTag_t iteration, cncTag_t map_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_stress_partial(handle, iteration, map_id, context) \
 cncPutChecked_stress_partial(handle, iteration, map_id, true, context)

void cncPutChecked_hourglass_partial(cncHandle_t handle, cncTag_t iteration, cncTag_t map_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_hourglass_partial(handle, iteration, map_id, context) \
 cncPutChecked_hourglass_partial(handle, iteration, map_id, true, context)

void cncPutChecked_force(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_force(handle, iteration, node_id, context) \
 cncPutChecked_force(handle, iteration, node_id, true, context)

void cncPutChecked_position(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_position(handle, iteration, node_id, context) \
 cncPutChecked_position(handle, iteration, node_id, true, context)

void cncPutChecked_velocity(cncHandle_t handle, cncTag_t iteration, cncTag_t node_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_velocity(handle, iteration, node_id, context) \
 cncPutChecked_velocity(handle, iteration, node_id, true, context)

void cncPutChecked_volume(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_volume(handle, iteration, element_id, context) \
 cncPutChecked_volume(handle, iteration, element_id, true, context)

void cncPutChecked_volume_derivative(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_volume_derivative(handle, iteration, element_id, context) \
 cncPutChecked_volume_derivative(handle, iteration, element_id, true, context)

void cncPutChecked_characteristic_length(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_characteristic_length(handle, iteration, element_id, context) \
 cncPutChecked_characteristic_length(handle, iteration, element_id, true, context)

void cncPutChecked_velocity_gradient(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_velocity_gradient(handle, iteration, element_id, context) \
 cncPutChecked_velocity_gradient(handle, iteration, element_id, true, context)

void cncPutChecked_position_gradient(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_position_gradient(handle, iteration, element_id, context) \
 cncPutChecked_position_gradient(handle, iteration, element_id, true, context)

void cncPutChecked_quadratic_viscosity_term(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_quadratic_viscosity_term(handle, iteration, element_id, context) \
 cncPutChecked_quadratic_viscosity_term(handle, iteration, element_id, true, context)

void cncPutChecked_linear_viscosity_term(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_linear_viscosity_term(handle, iteration, element_id, context) \
 cncPutChecked_linear_viscosity_term(handle, iteration, element_id, true, context)

void cncPutChecked_sound_speed(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_sound_speed(handle, iteration, element_id, context) \
 cncPutChecked_sound_speed(handle, iteration, element_id, true, context)

void cncPutChecked_viscosity(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_viscosity(handle, iteration, element_id, context) \
 cncPutChecked_viscosity(handle, iteration, element_id, true, context)

void cncPutChecked_pressure(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_pressure(handle, iteration, element_id, context) \
 cncPutChecked_pressure(handle, iteration, element_id, true, context)

void cncPutChecked_energy(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_energy(handle, iteration, element_id, context) \
 cncPutChecked_energy(handle, iteration, element_id, true, context)

void cncPutChecked_courant(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_courant(handle, iteration, element_id, context) \
 cncPutChecked_courant(handle, iteration, element_id, true, context)

void cncPutChecked_hydro(cncHandle_t handle, cncTag_t iteration, cncTag_t element_id, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_hydro(handle, iteration, element_id, context) \
 cncPutChecked_hydro(handle, iteration, element_id, true, context)

void cncPutChecked_delta_time(cncHandle_t handle, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_delta_time(handle, iteration, context) \
 cncPutChecked_delta_time(handle, iteration, true, context)

void cncPutChecked_elapsed_time(cncHandle_t handle, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_elapsed_time(handle, iteration, context) \
 cncPutChecked_elapsed_time(handle, iteration, true, context)

void cncPutChecked_neighbor_velocity(cncHandle_t handle, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_neighbor_velocity(handle, element_id, local_node_id, iteration, context) \
 cncPutChecked_neighbor_velocity(handle, element_id, local_node_id, iteration, true, context)

void cncPutChecked_neighbor_position(cncHandle_t handle, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_neighbor_position(handle, element_id, local_node_id, iteration, context) \
 cncPutChecked_neighbor_position(handle, element_id, local_node_id, iteration, true, context)

void cncPutChecked_neighbor_stress_partial(cncHandle_t handle, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_neighbor_stress_partial(handle, node_id, local_element_id, iteration, context) \
 cncPutChecked_neighbor_stress_partial(handle, node_id, local_element_id, iteration, true, context)

void cncPutChecked_neighbor_hourglass_partial(cncHandle_t handle, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_neighbor_hourglass_partial(handle, node_id, local_element_id, iteration, context) \
 cncPutChecked_neighbor_hourglass_partial(handle, node_id, local_element_id, iteration, true, context)

void cncPutChecked_neighbor_velocity_gradient(cncHandle_t handle, cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_neighbor_velocity_gradient(handle, element_id, local_element_id, iteration, context) \
 cncPutChecked_neighbor_velocity_gradient(handle, element_id, local_element_id, iteration, true, context)

void cncPutChecked_final_origin_energy(cncHandle_t handle, bool checkSingleAssignment, luleshCtx *context);
#define cncPut_final_origin_energy(handle, context) \
 cncPutChecked_final_origin_energy(handle, true, context)

/************************************\
 ******** STEP PRESCRIPTIONS ********
\************************************/

void cncPrescribe_lulesh_finalize(luleshCtx *ctx);
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

#endif /*_CNCOCR_LULESH_H_*/
