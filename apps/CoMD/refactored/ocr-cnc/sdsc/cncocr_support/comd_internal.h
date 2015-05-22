/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_COMD_INTERNAL_H_
#define _CNCOCR_COMD_INTERNAL_H_

#include "comd.h"
#include "cncocr_internal.h"

/******************************\
 ******** ITEM GETTERS ********
\******************************/


void cncGet_B(cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_SF(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_POT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_EAMPOT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_SPECIES(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_DD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_LC(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_NAtoms(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_redc(cncTag_t i, cncTag_t j, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_AtomInfo(cncTag_t i, cncTag_t j, cncTag_t k, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_CMD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_time(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_ATOMS(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_GID(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_ISP(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_R(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_P(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_F(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_U(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_IT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_TBoxes(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);

void cncGet_Nbs(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx);


/********************************\
 ******** STEP FUNCTIONS ********
\********************************/

void comd_cncInitialize(comdArgs *args, comdCtx *ctx);

void comd_cncFinalize(cncTag_t i, cncTag_t iter, struct box *B, struct myReduction *r, struct timeval *start, comdCtx *ctx);
ocrGuid_t _comd_cncStep_cncFinalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_cleanupInitStep(cncTag_t i, int *GID, int *ISP, real3 *R, real3 *P, real3 *F, real_t *U, comdCtx *ctx);
ocrGuid_t _comd_cncStep_cleanupInitStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_advanceVelocityStep(cncTag_t i, cncTag_t iter, struct box *B0, comdCtx *ctx);
ocrGuid_t _comd_cncStep_advanceVelocityStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_updateBoxStep(cncTag_t i, cncTag_t k, cncTag_t iter, struct box *B, int TBoxes, comdCtx *ctx);
ocrGuid_t _comd_cncStep_updateBoxStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_updateNeighborsStep(cncTag_t i, cncTag_t j, cncTag_t iter, struct atomInfo *AtomInfo0, struct box *B, int TBoxes, comdCtx *ctx);
ocrGuid_t _comd_cncStep_updateNeighborsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_generateDataforForceStep(cncTag_t i, cncTag_t iter, struct box *B, int totalBoxes, comdCtx *ctx);
ocrGuid_t _comd_cncStep_generateDataforForceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_generateForceTagsStep(cncTag_t iter, int TBoxes, comdCtx *ctx);
ocrGuid_t _comd_cncStep_generateForceTagsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_forceStep(cncTag_t i, cncTag_t iter, struct box *B, comdCtx *ctx);
ocrGuid_t _comd_cncStep_forceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_computeForcefromNeighborsStep(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, struct box *B0, struct box *B1, struct box *B2, struct box *B3, struct box *B4, struct box *B5, struct box *B6, struct box *B7, struct box *B8, struct box *B9, struct box *B10, struct box *B11, struct box *B12, struct box *B13, struct box *B14, struct box *B15, struct box *B16, struct box *B17, struct box *B18, struct box *B19, struct box *B20, struct box *B21, struct box *B22, struct box *B23, struct box *B24, struct box *B25, struct box *B26, struct eamPot *EAMPOT, comdCtx *ctx);
ocrGuid_t _comd_cncStep_computeForcefromNeighborsStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_computeForcefromNeighborsStep1(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, struct box *B0, struct box *B1, struct box *B2, struct box *B3, struct box *B4, struct box *B5, struct box *B6, struct box *B7, struct box *B8, struct box *B9, struct box *B10, struct box *B11, struct box *B12, struct box *B13, struct box *B14, struct box *B15, struct box *B16, struct box *B17, struct box *B18, struct box *B19, struct box *B20, struct box *B21, struct box *B22, struct box *B23, struct box *B24, struct box *B25, struct box *B26, struct eamPot *EAMPOT, comdCtx *ctx);
ocrGuid_t _comd_cncStep_computeForcefromNeighborsStep1(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void comd_reduceStep(cncTag_t i, cncTag_t iter, struct box *B0, struct myReduction *redc, int IT, int TBoxes, comdCtx *ctx);
ocrGuid_t _comd_cncStep_reduceStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

#endif /*_CNCOCR_COMD_INTERNAL_H_*/
