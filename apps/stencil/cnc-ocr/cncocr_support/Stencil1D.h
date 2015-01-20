/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_STENCIL1D_H_
#define _CNCOCR_STENCIL1D_H_

#include "cncocr.h"
#include "Stencil1D_defs.h"

/***************************\
 ******** CNC GRAPH ********
\***************************/

typedef struct Stencil1DContext {
    struct {
        ocrGuid_t self;
        ocrGuid_t finalizedEvent;
        ocrGuid_t doneEvent;
        ocrGuid_t awaitTag;
    } _guids;
    struct {
        cncItemCollection_t val;
    } _items;
    struct {
        ocrGuid_t Stencil1D_finalize;
        ocrGuid_t updateStep;
    } _steps;
    int NX;
    int NITER;
} Stencil1DCtx;

Stencil1DCtx *Stencil1D_create();
void Stencil1D_destroy(Stencil1DCtx *context);

void Stencil1D_launch(Stencil1DArgs *args, Stencil1DCtx *ctx);
void Stencil1D_await(Stencil1DCtx *context);

/**********************************\
 ******** ITEM KEY STRUCTS ********
\**********************************/

typedef struct { cncTag_t v, t; } valItemKey;

/*****************************\
 ******** ITEM CREATE ********
\*****************************/

float *cncCreateItemSized_val(size_t size);
static inline float *cncCreateItem_val(size_t count) {
    return cncCreateItemSized_val(sizeof(float) * count);
}

/**************************\
 ******** ITEM PUT ********
\**************************/

// val

void cncPutChecked_val(float *_item, cncTag_t v, cncTag_t t, bool checkSingleAssignment, Stencil1DCtx *ctx);

static inline void cncPut_val(float *_item, cncTag_t v, cncTag_t t, Stencil1DCtx *ctx) {
    cncPutChecked_val(_item, v, t, true, ctx);
}

/************************************\
 ******** STEP PRESCRIPTIONS ********
\************************************/

void cncPrescribe_Stencil1D_finalize(Stencil1DCtx *ctx);
void cncPrescribe_updateStep(cncTag_t v, cncTag_t t, Stencil1DCtx *ctx);

#endif /*_CNCOCR_STENCIL1D_H_*/
