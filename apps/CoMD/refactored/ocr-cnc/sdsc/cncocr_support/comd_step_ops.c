/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "comd_internal.h"


/* comd_finalize setup/teardown function */
ocrGuid_t _cncStep_comd_finalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t iter = (cncTag_t)_tag[1]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B;
    B = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct myReduction *r;
    r = (struct myReduction *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct timeval *start;
    start = (struct timeval *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    comd_finalize(i, iter, B, r, start, ctx);

    // Signal that the finalizer is done
    ocrEventSatisfy(ctx->_guids.finalizedEvent, NULL_GUID);

    // Clean up


    return NULL_GUID;
}

/* comd_finalize task creation */
void cncPrescribe_comd_finalize(cncTag_t i, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.comd_finalize,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B" input dependencies
        s64 _i0 = i;
        s64 _i1 = 5;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "r" input dependencies
        s64 _i0 = i+1;
        s64 _i1 = iter;
        cncGet_redc(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "start" input dependencies
        s64 _i0 = 0;
        cncGet_time(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* cleanupInitStep setup/teardown function */
ocrGuid_t _cncStep_cleanupInitStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    int *GID;
    GID = (int *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int *ISP;
    ISP = (int *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    real3 *R;
    R = (real3 *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    real3 *P;
    P = (real3 *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    real3 *F;
    F = (real3 *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    real_t *U;
    U = (real_t *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    cleanupInitStep(i, GID, ISP, R, P, F, U, ctx);

    // Clean up


    return NULL_GUID;
}

/* cleanupInitStep task creation */
void cncPrescribe_cleanupInitStep(cncTag_t i, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.cleanupInitStep,
        /*paramc=*/1, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "GID" input dependencies
        s64 _i0 = 1;
        cncGet_GID(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "ISP" input dependencies
        s64 _i0 = 1;
        cncGet_ISP(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "R" input dependencies
        s64 _i0 = 1;
        cncGet_R(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "P" input dependencies
        s64 _i0 = 1;
        cncGet_P(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "F" input dependencies
        s64 _i0 = 1;
        cncGet_F(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "U" input dependencies
        s64 _i0 = 1;
        cncGet_U(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* advanceVelocityStep setup/teardown function */
ocrGuid_t _cncStep_advanceVelocityStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t iter = (cncTag_t)_tag[1]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B0;
    B0 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    advanceVelocityStep(i, iter, B0, ctx);

    // Clean up


    return NULL_GUID;
}

/* advanceVelocityStep task creation */
void cncPrescribe_advanceVelocityStep(cncTag_t i, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.advanceVelocityStep,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B0" input dependencies
        s64 _i0 = i;
        s64 _i1 = 0;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* updateBoxStep setup/teardown function */
ocrGuid_t _cncStep_updateBoxStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t k = (cncTag_t)_tag[1]; MAYBE_UNUSED(k);
    const cncTag_t iter = (cncTag_t)_tag[2]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B;
    B = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int TBoxes;
    TBoxes = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    updateBoxStep(i, k, iter, B, TBoxes, ctx);

    // Clean up


    return NULL_GUID;
}

/* updateBoxStep task creation */
void cncPrescribe_updateBoxStep(cncTag_t i, cncTag_t k, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)k, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.updateBoxStep,
        /*paramc=*/3, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B" input dependencies
        s64 _i0 = i;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "TBoxes" input dependencies
        s64 _i0 = 0;
        cncGet_TBoxes(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* updateNeighborsStep setup/teardown function */
ocrGuid_t _cncStep_updateNeighborsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t j = (cncTag_t)_tag[1]; MAYBE_UNUSED(j);
    const cncTag_t iter = (cncTag_t)_tag[2]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct atomInfo *AtomInfo0;
    AtomInfo0 = (struct atomInfo *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B;
    B = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int TBoxes;
    TBoxes = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    updateNeighborsStep(i, j, iter, AtomInfo0, B, TBoxes, ctx);

    // Clean up


    return NULL_GUID;
}

/* updateNeighborsStep task creation */
void cncPrescribe_updateNeighborsStep(cncTag_t i, cncTag_t j, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)j, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.updateNeighborsStep,
        /*paramc=*/3, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "AtomInfo0" input dependencies
        s64 _i0 = i;
        s64 _i1 = j;
        s64 _i2 = iter;
        cncGet_AtomInfo(_i0, _i1, _i2, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B" input dependencies
        s64 _i0 = j;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "TBoxes" input dependencies
        s64 _i0 = 0;
        cncGet_TBoxes(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* generateDataforForceStep setup/teardown function */
ocrGuid_t _cncStep_generateDataforForceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t iter = (cncTag_t)_tag[1]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B;
    B = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int totalBoxes;
    totalBoxes = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    generateDataforForceStep(i, iter, B, totalBoxes, ctx);

    // Clean up


    return NULL_GUID;
}

/* generateDataforForceStep task creation */
void cncPrescribe_generateDataforForceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.generateDataforForceStep,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B" input dependencies
        s64 _i0 = i;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "totalBoxes" input dependencies
        s64 _i0 = 0;
        cncGet_TBoxes(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* generateForceTagsStep setup/teardown function */
ocrGuid_t _cncStep_generateForceTagsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t iter = (cncTag_t)_tag[0]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    int TBoxes;
    TBoxes = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    generateForceTagsStep(iter, TBoxes, ctx);

    // Clean up


    return NULL_GUID;
}

/* generateForceTagsStep task creation */
void cncPrescribe_generateForceTagsStep(cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.generateForceTagsStep,
        /*paramc=*/1, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "TBoxes" input dependencies
        s64 _i0 = 0;
        cncGet_TBoxes(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* forceStep setup/teardown function */
ocrGuid_t _cncStep_forceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t iter = (cncTag_t)_tag[1]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B;
    B = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    forceStep(i, iter, B, ctx);

    // Clean up


    return NULL_GUID;
}

/* forceStep task creation */
void cncPrescribe_forceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.forceStep,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B" input dependencies
        s64 _i0 = i;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* computeForcefromNeighborsStep setup/teardown function */
ocrGuid_t _cncStep_computeForcefromNeighborsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = depv[1].ptr; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t j1 = (cncTag_t)_tag[1]; MAYBE_UNUSED(j1);
    const cncTag_t j2 = (cncTag_t)_tag[2]; MAYBE_UNUSED(j2);
    const cncTag_t j3 = (cncTag_t)_tag[3]; MAYBE_UNUSED(j3);
    const cncTag_t j4 = (cncTag_t)_tag[4]; MAYBE_UNUSED(j4);
    const cncTag_t j5 = (cncTag_t)_tag[5]; MAYBE_UNUSED(j5);
    const cncTag_t j6 = (cncTag_t)_tag[6]; MAYBE_UNUSED(j6);
    const cncTag_t j7 = (cncTag_t)_tag[7]; MAYBE_UNUSED(j7);
    const cncTag_t j8 = (cncTag_t)_tag[8]; MAYBE_UNUSED(j8);
    const cncTag_t j9 = (cncTag_t)_tag[9]; MAYBE_UNUSED(j9);
    const cncTag_t j10 = (cncTag_t)_tag[10]; MAYBE_UNUSED(j10);
    const cncTag_t j11 = (cncTag_t)_tag[11]; MAYBE_UNUSED(j11);
    const cncTag_t j12 = (cncTag_t)_tag[12]; MAYBE_UNUSED(j12);
    const cncTag_t j13 = (cncTag_t)_tag[13]; MAYBE_UNUSED(j13);
    const cncTag_t j14 = (cncTag_t)_tag[14]; MAYBE_UNUSED(j14);
    const cncTag_t j15 = (cncTag_t)_tag[15]; MAYBE_UNUSED(j15);
    const cncTag_t j16 = (cncTag_t)_tag[16]; MAYBE_UNUSED(j16);
    const cncTag_t j17 = (cncTag_t)_tag[17]; MAYBE_UNUSED(j17);
    const cncTag_t j18 = (cncTag_t)_tag[18]; MAYBE_UNUSED(j18);
    const cncTag_t j19 = (cncTag_t)_tag[19]; MAYBE_UNUSED(j19);
    const cncTag_t j20 = (cncTag_t)_tag[20]; MAYBE_UNUSED(j20);
    const cncTag_t j21 = (cncTag_t)_tag[21]; MAYBE_UNUSED(j21);
    const cncTag_t j22 = (cncTag_t)_tag[22]; MAYBE_UNUSED(j22);
    const cncTag_t j23 = (cncTag_t)_tag[23]; MAYBE_UNUSED(j23);
    const cncTag_t j24 = (cncTag_t)_tag[24]; MAYBE_UNUSED(j24);
    const cncTag_t j25 = (cncTag_t)_tag[25]; MAYBE_UNUSED(j25);
    const cncTag_t j26 = (cncTag_t)_tag[26]; MAYBE_UNUSED(j26);
    const cncTag_t iter = (cncTag_t)_tag[27]; MAYBE_UNUSED(iter);
    ocrDbDestroy(depv[1].guid); // free tag component datablock

    s32 _edtSlot = 2; MAYBE_UNUSED(_edtSlot);

    struct box *B0;
    B0 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B1;
    B1 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B2;
    B2 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B3;
    B3 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B4;
    B4 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B5;
    B5 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B6;
    B6 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B7;
    B7 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B8;
    B8 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B9;
    B9 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B10;
    B10 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B11;
    B11 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B12;
    B12 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B13;
    B13 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B14;
    B14 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B15;
    B15 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B16;
    B16 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B17;
    B17 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B18;
    B18 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B19;
    B19 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B20;
    B20 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B21;
    B21 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B22;
    B22 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B23;
    B23 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B24;
    B24 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B25;
    B25 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B26;
    B26 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct eamPot *EAMPOT;
    EAMPOT = (struct eamPot *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    computeForcefromNeighborsStep(i, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15, j16, j17, j18, j19, j20, j21, j22, j23, j24, j25, j26, iter, B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20, B21, B22, B23, B24, B25, B26, EAMPOT, ctx);

    // Clean up


    return NULL_GUID;
}

/* computeForcefromNeighborsStep task creation */
void cncPrescribe_computeForcefromNeighborsStep(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)j1, (u64)j2, (u64)j3, (u64)j4, (u64)j5, (u64)j6, (u64)j7, (u64)j8, (u64)j9, (u64)j10, (u64)j11, (u64)j12, (u64)j13, (u64)j14, (u64)j15, (u64)j16, (u64)j17, (u64)j18, (u64)j19, (u64)j20, (u64)j21, (u64)j22, (u64)j23, (u64)j24, (u64)j25, (u64)j26, (u64)iter };
    ocrGuid_t _tagBlockGuid;
    u64 *_tagBlockPtr;
    SIMPLE_DBCREATE(&_tagBlockGuid, (void**)&_tagBlockPtr, sizeof(_args));
    hal_memCopy(_tagBlockPtr, _args, sizeof(_args), 0);
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2;
    ocrEdtCreate(&_stepGuid, ctx->_steps.computeForcefromNeighborsStep,
        /*paramc=*/0, /*paramv=*/NULL,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);
    ocrAddDependence(_tagBlockGuid, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B0" input dependencies
        s64 _i0 = i;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B1" input dependencies
        s64 _i0 = j1;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B2" input dependencies
        s64 _i0 = j2;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B3" input dependencies
        s64 _i0 = j3;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B4" input dependencies
        s64 _i0 = j4;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B5" input dependencies
        s64 _i0 = j5;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B6" input dependencies
        s64 _i0 = j6;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B7" input dependencies
        s64 _i0 = j7;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B8" input dependencies
        s64 _i0 = j8;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B9" input dependencies
        s64 _i0 = j9;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B10" input dependencies
        s64 _i0 = j10;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B11" input dependencies
        s64 _i0 = j11;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B12" input dependencies
        s64 _i0 = j12;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B13" input dependencies
        s64 _i0 = j13;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B14" input dependencies
        s64 _i0 = j14;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B15" input dependencies
        s64 _i0 = j15;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B16" input dependencies
        s64 _i0 = j16;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B17" input dependencies
        s64 _i0 = j17;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B18" input dependencies
        s64 _i0 = j18;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B19" input dependencies
        s64 _i0 = j19;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B20" input dependencies
        s64 _i0 = j20;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B21" input dependencies
        s64 _i0 = j21;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B22" input dependencies
        s64 _i0 = j22;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B23" input dependencies
        s64 _i0 = j23;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B24" input dependencies
        s64 _i0 = j24;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B25" input dependencies
        s64 _i0 = j25;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B26" input dependencies
        s64 _i0 = j26;
        s64 _i1 = 1;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "EAMPOT" input dependencies
        s64 _i0 = 0;
        cncGet_EAMPOT(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* computeForcefromNeighborsStep1 setup/teardown function */
ocrGuid_t _cncStep_computeForcefromNeighborsStep1(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = depv[1].ptr; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t j1 = (cncTag_t)_tag[1]; MAYBE_UNUSED(j1);
    const cncTag_t j2 = (cncTag_t)_tag[2]; MAYBE_UNUSED(j2);
    const cncTag_t j3 = (cncTag_t)_tag[3]; MAYBE_UNUSED(j3);
    const cncTag_t j4 = (cncTag_t)_tag[4]; MAYBE_UNUSED(j4);
    const cncTag_t j5 = (cncTag_t)_tag[5]; MAYBE_UNUSED(j5);
    const cncTag_t j6 = (cncTag_t)_tag[6]; MAYBE_UNUSED(j6);
    const cncTag_t j7 = (cncTag_t)_tag[7]; MAYBE_UNUSED(j7);
    const cncTag_t j8 = (cncTag_t)_tag[8]; MAYBE_UNUSED(j8);
    const cncTag_t j9 = (cncTag_t)_tag[9]; MAYBE_UNUSED(j9);
    const cncTag_t j10 = (cncTag_t)_tag[10]; MAYBE_UNUSED(j10);
    const cncTag_t j11 = (cncTag_t)_tag[11]; MAYBE_UNUSED(j11);
    const cncTag_t j12 = (cncTag_t)_tag[12]; MAYBE_UNUSED(j12);
    const cncTag_t j13 = (cncTag_t)_tag[13]; MAYBE_UNUSED(j13);
    const cncTag_t j14 = (cncTag_t)_tag[14]; MAYBE_UNUSED(j14);
    const cncTag_t j15 = (cncTag_t)_tag[15]; MAYBE_UNUSED(j15);
    const cncTag_t j16 = (cncTag_t)_tag[16]; MAYBE_UNUSED(j16);
    const cncTag_t j17 = (cncTag_t)_tag[17]; MAYBE_UNUSED(j17);
    const cncTag_t j18 = (cncTag_t)_tag[18]; MAYBE_UNUSED(j18);
    const cncTag_t j19 = (cncTag_t)_tag[19]; MAYBE_UNUSED(j19);
    const cncTag_t j20 = (cncTag_t)_tag[20]; MAYBE_UNUSED(j20);
    const cncTag_t j21 = (cncTag_t)_tag[21]; MAYBE_UNUSED(j21);
    const cncTag_t j22 = (cncTag_t)_tag[22]; MAYBE_UNUSED(j22);
    const cncTag_t j23 = (cncTag_t)_tag[23]; MAYBE_UNUSED(j23);
    const cncTag_t j24 = (cncTag_t)_tag[24]; MAYBE_UNUSED(j24);
    const cncTag_t j25 = (cncTag_t)_tag[25]; MAYBE_UNUSED(j25);
    const cncTag_t j26 = (cncTag_t)_tag[26]; MAYBE_UNUSED(j26);
    const cncTag_t iter = (cncTag_t)_tag[27]; MAYBE_UNUSED(iter);
    ocrDbDestroy(depv[1].guid); // free tag component datablock

    s32 _edtSlot = 2; MAYBE_UNUSED(_edtSlot);

    struct box *B0;
    B0 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B1;
    B1 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B2;
    B2 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B3;
    B3 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B4;
    B4 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B5;
    B5 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B6;
    B6 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B7;
    B7 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B8;
    B8 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B9;
    B9 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B10;
    B10 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B11;
    B11 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B12;
    B12 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B13;
    B13 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B14;
    B14 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B15;
    B15 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B16;
    B16 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B17;
    B17 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B18;
    B18 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B19;
    B19 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B20;
    B20 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B21;
    B21 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B22;
    B22 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B23;
    B23 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B24;
    B24 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B25;
    B25 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct box *B26;
    B26 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct eamPot *EAMPOT;
    EAMPOT = (struct eamPot *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    computeForcefromNeighborsStep1(i, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15, j16, j17, j18, j19, j20, j21, j22, j23, j24, j25, j26, iter, B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20, B21, B22, B23, B24, B25, B26, EAMPOT, ctx);

    // Clean up


    return NULL_GUID;
}

/* computeForcefromNeighborsStep1 task creation */
void cncPrescribe_computeForcefromNeighborsStep1(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)j1, (u64)j2, (u64)j3, (u64)j4, (u64)j5, (u64)j6, (u64)j7, (u64)j8, (u64)j9, (u64)j10, (u64)j11, (u64)j12, (u64)j13, (u64)j14, (u64)j15, (u64)j16, (u64)j17, (u64)j18, (u64)j19, (u64)j20, (u64)j21, (u64)j22, (u64)j23, (u64)j24, (u64)j25, (u64)j26, (u64)iter };
    ocrGuid_t _tagBlockGuid;
    u64 *_tagBlockPtr;
    SIMPLE_DBCREATE(&_tagBlockGuid, (void**)&_tagBlockPtr, sizeof(_args));
    hal_memCopy(_tagBlockPtr, _args, sizeof(_args), 0);
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2;
    ocrEdtCreate(&_stepGuid, ctx->_steps.computeForcefromNeighborsStep1,
        /*paramc=*/0, /*paramv=*/NULL,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);
    ocrAddDependence(_tagBlockGuid, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B0" input dependencies
        s64 _i0 = i;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B1" input dependencies
        s64 _i0 = j1;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B2" input dependencies
        s64 _i0 = j2;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B3" input dependencies
        s64 _i0 = j3;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B4" input dependencies
        s64 _i0 = j4;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B5" input dependencies
        s64 _i0 = j5;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B6" input dependencies
        s64 _i0 = j6;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B7" input dependencies
        s64 _i0 = j7;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B8" input dependencies
        s64 _i0 = j8;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B9" input dependencies
        s64 _i0 = j9;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B10" input dependencies
        s64 _i0 = j10;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B11" input dependencies
        s64 _i0 = j11;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B12" input dependencies
        s64 _i0 = j12;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B13" input dependencies
        s64 _i0 = j13;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B14" input dependencies
        s64 _i0 = j14;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B15" input dependencies
        s64 _i0 = j15;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B16" input dependencies
        s64 _i0 = j16;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B17" input dependencies
        s64 _i0 = j17;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B18" input dependencies
        s64 _i0 = j18;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B19" input dependencies
        s64 _i0 = j19;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B20" input dependencies
        s64 _i0 = j20;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B21" input dependencies
        s64 _i0 = j21;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B22" input dependencies
        s64 _i0 = j22;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B23" input dependencies
        s64 _i0 = j23;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B24" input dependencies
        s64 _i0 = j24;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B25" input dependencies
        s64 _i0 = j25;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "B26" input dependencies
        s64 _i0 = j26;
        s64 _i1 = 3;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "EAMPOT" input dependencies
        s64 _i0 = 0;
        cncGet_EAMPOT(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}

/* reduceStep setup/teardown function */
ocrGuid_t _cncStep_reduceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    comdCtx *ctx = depv[0].ptr;

    u64 *_tag = paramv; MAYBE_UNUSED(_tag);
    const cncTag_t i = (cncTag_t)_tag[0]; MAYBE_UNUSED(i);
    const cncTag_t iter = (cncTag_t)_tag[1]; MAYBE_UNUSED(iter);

    s32 _edtSlot = 1; MAYBE_UNUSED(_edtSlot);

    struct box *B0;
    B0 = (struct box *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    struct myReduction *redc;
    redc = (struct myReduction *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int IT;
    IT = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);

    int TBoxes;
    TBoxes = *(int *)_cncItemDataPtr(depv[_edtSlot++].ptr);


    // Call user-defined step function

    reduceStep(i, iter, B0, redc, IT, TBoxes, ctx);

    // Clean up


    return NULL_GUID;
}

/* reduceStep task creation */
void cncPrescribe_reduceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx) {

    ocrGuid_t _stepGuid;
    u64 _args[] = { (u64)i, (u64)iter };
    // affinity
    // TODO - allow custom distribution
    const ocrGuid_t _affinity = NULL_GUID;
    u64 _depc = 1 + 1 + 1 + 1 + 1;
    ocrEdtCreate(&_stepGuid, ctx->_steps.reduceStep,
        /*paramc=*/2, /*paramv=*/_args,
        /*depc=*/_depc, /*depv=*/NULL,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/_affinity, /*outEvent=*/NULL);

    s32 _edtSlot = 0; MAYBE_UNUSED(_edtSlot);
    ocrAddDependence(ctx->_guids.self, _stepGuid, _edtSlot++, DB_MODE_RO);


    { // Set up "B0" input dependencies
        s64 _i0 = i;
        s64 _i1 = 4;
        s64 _i2 = 0;
        s64 _i3 = iter;
        cncGet_B(_i0, _i1, _i2, _i3, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "redc" input dependencies
        s64 _i0 = 0;
        s64 _i1 = 1;
        cncGet_redc(_i0, _i1, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "IT" input dependencies
        s64 _i0 = 0;
        cncGet_IT(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    { // Set up "TBoxes" input dependencies
        s64 _i0 = 0;
        cncGet_TBoxes(_i0, _stepGuid, _edtSlot++, DB_DEFAULT_MODE, ctx);
    }

    ASSERT(_depc == _edtSlot);

}


