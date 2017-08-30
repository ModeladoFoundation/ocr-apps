/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNC_LULESH_CONTEXT_H_
#define _CNC_LULESH_CONTEXT_H_

#include "cncocr.h"
#include "lulesh_defs.h"

typedef struct luleshContext {
    struct constraints constraints;
    struct constants constants;
    struct cutoffs cutoffs;
    struct domain domain;
    struct mesh mesh;
    int elements;
    int nodes;
    struct {
        ocrGuid_t self;
        ocrGuid_t finalizedEvent;
        ocrGuid_t quiescedEvent;
        ocrGuid_t doneEvent;
        ocrGuid_t awaitTag;
        ocrGuid_t contextReady;
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
        cncItemSingleton_t final_origin_energy;
    } _items;
    struct {
        ocrGuid_t cncFinalize;
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
#ifdef CNC_AFFINITIES
    cncLocation_t _rank;
    u64 _affinityCount;
    ocrGuid_t _affinities[];
#endif /* CNC_AFFINITIES */
} luleshCtx;

#endif /*_CNC_LULESH_CONTEXT_H_*/
