/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "Stencil1D_internal.h"


/* Stencil1D_finalize setup/teardown function */
ocrGuid_t _cncStep_Stencil1D_finalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    Stencil1DCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    float **results;
    ocrEdtDep_t _block_results;
    { // Init ranges for "results"
        u32 _i;
        u32 _itemCount = ctx->NX;
        u32 _dims[] = { ctx->NX };
        // XXX - I'd like to use pdMalloc here instead of creating a datablock
        float **_item = _cncRangedInputAlloc(1, _dims, sizeof(float *), &_block_results);
        results = _block_results.ptr;
        for (_i=0; _i<_itemCount; _i++) {
            _item[_i] = (float *)_cncItemDataPtr(depv[_edtSlot++].ptr);
        }
    }

    // Call user-defined step function

    Stencil1D_finalize(results, ctx);

    // Signal that the finalizer is done
    ocrEventSatisfy(ctx->_guids.finalizedEvent, NULL_GUID);

    // Clean up
    ocrDbDestroy(_block_results.guid);

    return NULL_GUID;
}

/* Stencil1D_finalize task creation */
void cncPrescribe_Stencil1D_finalize(Stencil1DCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 *_args = NULL;
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = ctx->NX + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.Stencil1D_finalize,
        /*paramc=*/0, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "results" input dependencies
        s64 _i0;
        for (_i0 = 0; _i0 < ctx->NX; _i0++) {
            s64 _i1 = ctx->NITER;
            cncGet_val(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
        }
    }

    ASSERT(_depc == _edtSlot);

}

/* updateStep setup/teardown function */
ocrGuid_t _cncStep_updateStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    Stencil1DCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t v = (cncTag_t)_tag[0]; MAYBE_UNUSED(v);
    const cncTag_t t = (cncTag_t)_tag[1]; MAYBE_UNUSED(t);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    float *center;
    center = (float *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    float *left;
    left = (float *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    float *right;
    right = (float *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    // Call user-defined step function

    updateStep(v, t, center, left, right, ctx);

    // Clean up

    return NULL_GUID;
}

/* updateStep task creation */
void cncPrescribe_updateStep(cncTag_t v, cncTag_t t, Stencil1DCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)v, (u64)t };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.updateStep,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "center" input dependencies
        s64 _i0 = v;
        s64 _i1 = t;
        cncGet_val(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "left" input dependencies
        s64 _i0 = v - 1;
        s64 _i1 = t;
        cncGet_val(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "right" input dependencies
        s64 _i0 = v + 1;
        s64 _i1 = t;
        cncGet_val(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}


