/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "lulesh_internal.h"
#include <string.h>


/* stress_partial */

void cncPut_stress_partial(vector *_item, cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_stress_partial(iteration, map_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT stress_partial @ %ld, %ld\n", iteration, map_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT stress_partial @ %ld, %ld\n", iteration, map_id);
    #endif
    cncTag_t _tag[] = { iteration, map_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, stress_partial, _loc));
}

void cncGet_stress_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP stress_partial @ %ld, %ld\n", iteration, map_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP stress_partial @ %ld, %ld\n", iteration, map_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_stress_partial(iteration, map_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, map_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, stress_partial, _loc));
}


/* hourglass_partial */

void cncPut_hourglass_partial(vector *_item, cncTag_t iteration, cncTag_t map_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_hourglass_partial(iteration, map_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT hourglass_partial @ %ld, %ld\n", iteration, map_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT hourglass_partial @ %ld, %ld\n", iteration, map_id);
    #endif
    cncTag_t _tag[] = { iteration, map_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, hourglass_partial, _loc));
}

void cncGet_hourglass_partial(cncTag_t iteration, cncTag_t map_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP hourglass_partial @ %ld, %ld\n", iteration, map_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP hourglass_partial @ %ld, %ld\n", iteration, map_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_hourglass_partial(iteration, map_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, map_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, hourglass_partial, _loc));
}


/* force */

void cncPut_force(vector *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_force(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT force @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT force @ %ld, %ld\n", iteration, node_id);
    #endif
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, force, _loc));
}

void cncGet_force(cncTag_t iteration, cncTag_t node_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP force @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP force @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_force(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, force, _loc));
}


/* position */

void cncPut_position(vertex *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_position(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT position @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT position @ %ld, %ld\n", iteration, node_id);
    #endif
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, position, _loc));
}

void cncGet_position(cncTag_t iteration, cncTag_t node_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP position @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP position @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_position(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, position, _loc));
}


/* velocity */

void cncPut_velocity(vector *_item, cncTag_t iteration, cncTag_t node_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_velocity(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT velocity @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT velocity @ %ld, %ld\n", iteration, node_id);
    #endif
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, velocity, _loc));
}

void cncGet_velocity(cncTag_t iteration, cncTag_t node_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP velocity @ %ld, %ld\n", iteration, node_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP velocity @ %ld, %ld\n", iteration, node_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_velocity(iteration, node_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, node_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, velocity, _loc));
}


/* volume */

void cncPut_volume(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_volume(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT volume @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT volume @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, volume, _loc));
}

void cncGet_volume(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP volume @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP volume @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_volume(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, volume, _loc));
}


/* volume_derivative */

void cncPut_volume_derivative(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_volume_derivative(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT volume_derivative @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT volume_derivative @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, volume_derivative, _loc));
}

void cncGet_volume_derivative(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP volume_derivative @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP volume_derivative @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_volume_derivative(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, volume_derivative, _loc));
}


/* characteristic_length */

void cncPut_characteristic_length(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_characteristic_length(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT characteristic_length @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT characteristic_length @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, characteristic_length, _loc));
}

void cncGet_characteristic_length(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP characteristic_length @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP characteristic_length @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_characteristic_length(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, characteristic_length, _loc));
}


/* velocity_gradient */

void cncPut_velocity_gradient(vector *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_velocity_gradient(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT velocity_gradient @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT velocity_gradient @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, velocity_gradient, _loc));
}

void cncGet_velocity_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP velocity_gradient @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP velocity_gradient @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_velocity_gradient(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, velocity_gradient, _loc));
}


/* position_gradient */

void cncPut_position_gradient(vector *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_position_gradient(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT position_gradient @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT position_gradient @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, position_gradient, _loc));
}

void cncGet_position_gradient(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP position_gradient @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP position_gradient @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_position_gradient(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, position_gradient, _loc));
}


/* quadratic_viscosity_term */

void cncPut_quadratic_viscosity_term(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_quadratic_viscosity_term(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT quadratic_viscosity_term @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT quadratic_viscosity_term @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, quadratic_viscosity_term, _loc));
}

void cncGet_quadratic_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP quadratic_viscosity_term @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP quadratic_viscosity_term @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_quadratic_viscosity_term(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, quadratic_viscosity_term, _loc));
}


/* linear_viscosity_term */

void cncPut_linear_viscosity_term(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_linear_viscosity_term(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT linear_viscosity_term @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT linear_viscosity_term @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, linear_viscosity_term, _loc));
}

void cncGet_linear_viscosity_term(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP linear_viscosity_term @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP linear_viscosity_term @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_linear_viscosity_term(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, linear_viscosity_term, _loc));
}


/* sound_speed */

void cncPut_sound_speed(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_sound_speed(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT sound_speed @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT sound_speed @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, sound_speed, _loc));
}

void cncGet_sound_speed(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP sound_speed @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP sound_speed @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_sound_speed(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, sound_speed, _loc));
}


/* viscosity */

void cncPut_viscosity(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_viscosity(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT viscosity @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT viscosity @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, viscosity, _loc));
}

void cncGet_viscosity(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP viscosity @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP viscosity @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_viscosity(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, viscosity, _loc));
}


/* pressure */

void cncPut_pressure(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_pressure(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT pressure @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT pressure @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, pressure, _loc));
}

void cncGet_pressure(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP pressure @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP pressure @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_pressure(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, pressure, _loc));
}


/* energy */

void cncPut_energy(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_energy(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT energy @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT energy @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, energy, _loc));
}

void cncGet_energy(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP energy @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP energy @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_energy(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, energy, _loc));
}


/* courant */

void cncPut_courant(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_courant(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT courant @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT courant @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, courant, _loc));
}

void cncGet_courant(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP courant @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP courant @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_courant(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, courant, _loc));
}


/* hydro */

void cncPut_hydro(double *_item, cncTag_t iteration, cncTag_t element_id, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_hydro(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT hydro @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT hydro @ %ld, %ld\n", iteration, element_id);
    #endif
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, hydro, _loc));
}

void cncGet_hydro(cncTag_t iteration, cncTag_t element_id, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP hydro @ %ld, %ld\n", iteration, element_id);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP hydro @ %ld, %ld\n", iteration, element_id);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_hydro(iteration, element_id, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration, element_id };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, hydro, _loc));
}


/* delta_time */

void cncPut_delta_time(double *_item, cncTag_t iteration, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_delta_time(iteration, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT delta_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT delta_time @ %ld\n", iteration);
    #endif
    cncTag_t _tag[] = { iteration };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, delta_time, _loc));
}

void cncGet_delta_time(cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP delta_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP delta_time @ %ld\n", iteration);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_delta_time(iteration, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, delta_time, _loc));
}


/* elapsed_time */

void cncPut_elapsed_time(double *_item, cncTag_t iteration, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_elapsed_time(iteration, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT elapsed_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT elapsed_time @ %ld\n", iteration);
    #endif
    cncTag_t _tag[] = { iteration };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE(ctx, elapsed_time, _loc));
}

void cncGet_elapsed_time(cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP elapsed_time @ %ld\n", iteration);
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP elapsed_time @ %ld\n", iteration);
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_elapsed_time(iteration, ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    cncTag_t _tag[] = { iteration };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, elapsed_time, _loc));
}


/* neighbor_velocity */

void cncPut_neighbor_velocity(vector *_item, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) {
    velocityItemKey _key = get_elements_node_neighbors_for_velocity(element_id, local_node_id, iteration, ctx);
    cncPut_velocity(_item, _key.iteration, _key.node_id, ctx);
}

void cncGet_neighbor_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    velocityItemKey _key = get_elements_node_neighbors_for_velocity(element_id, local_node_id, iteration, ctx);
    cncGet_velocity(_key.iteration, _key.node_id, _destination, _slot, _mode, ctx);
}


/* neighbor_position */

void cncPut_neighbor_position(vertex *_item, cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) {
    positionItemKey _key = get_elements_node_neighbors_for_position(element_id, local_node_id, iteration, ctx);
    cncPut_position(_item, _key.iteration, _key.node_id, ctx);
}

void cncGet_neighbor_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    positionItemKey _key = get_elements_node_neighbors_for_position(element_id, local_node_id, iteration, ctx);
    cncGet_position(_key.iteration, _key.node_id, _destination, _slot, _mode, ctx);
}


/* neighbor_stress_partial */

void cncPut_neighbor_stress_partial(vector *_item, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {
    stress_partialItemKey _key = get_map_id_for_node_element_neighbors_for_stress_partial(node_id, local_element_id, iteration, ctx);
    cncPut_stress_partial(_item, _key.iteration, _key.map_id, ctx);
}

void cncGet_neighbor_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    stress_partialItemKey _key = get_map_id_for_node_element_neighbors_for_stress_partial(node_id, local_element_id, iteration, ctx);
    cncGet_stress_partial(_key.iteration, _key.map_id, _destination, _slot, _mode, ctx);
}


/* neighbor_hourglass_partial */

void cncPut_neighbor_hourglass_partial(vector *_item, cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {
    hourglass_partialItemKey _key = get_map_id_for_node_element_neighbors_for_hourglass_partial(node_id, local_element_id, iteration, ctx);
    cncPut_hourglass_partial(_item, _key.iteration, _key.map_id, ctx);
}

void cncGet_neighbor_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    hourglass_partialItemKey _key = get_map_id_for_node_element_neighbors_for_hourglass_partial(node_id, local_element_id, iteration, ctx);
    cncGet_hourglass_partial(_key.iteration, _key.map_id, _destination, _slot, _mode, ctx);
}


/* neighbor_velocity_gradient */

void cncPut_neighbor_velocity_gradient(vector *_item, cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {
    velocity_gradientItemKey _key = get_elements_element_neighbors_for_velocity_gradient(element_id, local_element_id, iteration, ctx);
    cncPut_velocity_gradient(_item, _key.iteration, _key.element_id, ctx);
}

void cncGet_neighbor_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    velocity_gradientItemKey _key = get_elements_element_neighbors_for_velocity_gradient(element_id, local_element_id, iteration, ctx);
    cncGet_velocity_gradient(_key.iteration, _key.element_id, _destination, _slot, _mode, ctx);
}


/* final_origin_energy */

void cncPut_final_origin_energy(double *_item, luleshCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = _cncItemDistFn_final_origin_energy(ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "PUT final_origin_energy @ 0\n");
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: PUT final_origin_energy @ 0\n");
    #endif
    // FIXME - affinities not correctly set for singletons
    // _cncPutSingleton(_handle, _CNC_ITEM_COLL_HANDLE(ctx, final_origin_energy, _loc));
    _cncPutSingleton(_handle, ctx->_items.final_origin_energy);
}

void cncGet_final_origin_energy(ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, luleshCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
        fprintf(cncDebugLog, "GET-DEP final_origin_energy @ 0\n");
        fflush(cncDebugLog);
    #elif CNC_DEBUG_TRACE
        printf("<<CnC Trace>>: GET-DEP final_origin_energy @ 0\n");
    #endif
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = _cncItemDistFn_final_origin_energy(ctx);
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif /* CNC_AFFINITIES */
    // FIXME - affinities not correctly set for singletons
    // return _cncGetSingleton(_destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE(ctx, final_origin_energy, _loc));
    return _cncGetSingleton(_destination, _slot, _mode, ctx->_items.final_origin_energy);
}


