/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_STENCIL1D_INTERNAL_H_
#define _CNCOCR_STENCIL1D_INTERNAL_H_

#include "Stencil1D.h"
#include "cncocr_internal.h"

/******************************\
 ******** ITEM GETTERS ********
\******************************/


void cncGet_val(cncTag_t v, cncTag_t t, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, Stencil1DCtx *context);


/********************************\
 ******** STEP FUNCTIONS ********
\********************************/

void Stencil1D_init(Stencil1DArgs *args, Stencil1DCtx *ctx);

void Stencil1D_finalize(float **results, Stencil1DCtx *ctx);
ocrGuid_t _cncStep_Stencil1D_finalize(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

void updateStep(cncTag_t v, cncTag_t t, float *center, float *left, float *right, Stencil1DCtx *ctx);
ocrGuid_t _cncStep_updateStep(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]);

#endif /*_CNCOCR_STENCIL1D_INTERNAL_H_*/
