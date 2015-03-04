/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "comd_internal.h"


/* B */

struct box *cncCreateItemSized_B(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_B(struct box *_item, cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i, j, k, l };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.B, _checkSingleAssignment);
}

void cncGet_B(cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i, j, k, l };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.B);
}


/* SF */

SimFlat *cncCreateItemSized_SF(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_SF(SimFlat *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.SF, _checkSingleAssignment);
}

void cncGet_SF(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.SF);
}


/* POT */

LjPotential *cncCreateItemSized_POT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_POT(LjPotential *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.POT, _checkSingleAssignment);
}

void cncGet_POT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.POT);
}


/* EAMPOT */

struct eamPot *cncCreateItemSized_EAMPOT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_EAMPOT(struct eamPot *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.EAMPOT, _checkSingleAssignment);
}

void cncGet_EAMPOT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.EAMPOT);
}


/* SPECIES */

SpeciesData *cncCreateItemSized_SPECIES(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_SPECIES(SpeciesData *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.SPECIES, _checkSingleAssignment);
}

void cncGet_SPECIES(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.SPECIES);
}


/* DD */

Domain *cncCreateItemSized_DD(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_DD(Domain *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.DD, _checkSingleAssignment);
}

void cncGet_DD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.DD);
}


/* LC */

LinkCell *cncCreateItemSized_LC(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_LC(LinkCell *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.LC, _checkSingleAssignment);
}

void cncGet_LC(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.LC);
}


/* NAtoms */

int *cncCreateItemSized_NAtoms(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_NAtoms(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.NAtoms, _checkSingleAssignment);
}

void cncGet_NAtoms(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.NAtoms);
}


/* redc */

struct myReduction *cncCreateItemSized_redc(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_redc(struct myReduction *_item, cncTag_t i, cncTag_t j, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i, j };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.redc, _checkSingleAssignment);
}

void cncGet_redc(cncTag_t i, cncTag_t j, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i, j };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.redc);
}


/* AtomInfo */

struct atomInfo *cncCreateItemSized_AtomInfo(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_AtomInfo(struct atomInfo *_item, cncTag_t i, cncTag_t j, cncTag_t k, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i, j, k };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.AtomInfo, _checkSingleAssignment);
}

void cncGet_AtomInfo(cncTag_t i, cncTag_t j, cncTag_t k, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i, j, k };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.AtomInfo);
}


/* CMD */

struct cmdInfo *cncCreateItemSized_CMD(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_CMD(struct cmdInfo *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.CMD, _checkSingleAssignment);
}

void cncGet_CMD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.CMD);
}


/* time */

struct timeval *cncCreateItemSized_time(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_time(struct timeval *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.time, _checkSingleAssignment);
}

void cncGet_time(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.time);
}


/* ATOMS */

Atoms *cncCreateItemSized_ATOMS(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_ATOMS(Atoms *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.ATOMS, _checkSingleAssignment);
}

void cncGet_ATOMS(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.ATOMS);
}


/* GID */

int *cncCreateItemSized_GID(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_GID(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.GID, _checkSingleAssignment);
}

void cncGet_GID(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.GID);
}


/* ISP */

int *cncCreateItemSized_ISP(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_ISP(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.ISP, _checkSingleAssignment);
}

void cncGet_ISP(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.ISP);
}


/* R */

real3 *cncCreateItemSized_R(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_R(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.R, _checkSingleAssignment);
}

void cncGet_R(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.R);
}


/* P */

real3 *cncCreateItemSized_P(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_P(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.P, _checkSingleAssignment);
}

void cncGet_P(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.P);
}


/* F */

real3 *cncCreateItemSized_F(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_F(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.F, _checkSingleAssignment);
}

void cncGet_F(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.F);
}


/* U */

real_t *cncCreateItemSized_U(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_U(real_t *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.U, _checkSingleAssignment);
}

void cncGet_U(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.U);
}


/* IT */

int *cncCreateItemSized_IT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_IT(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.IT, _checkSingleAssignment);
}

void cncGet_IT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.IT);
}


/* TBoxes */

int *cncCreateItemSized_TBoxes(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_TBoxes(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.TBoxes, _checkSingleAssignment);
}

void cncGet_TBoxes(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.TBoxes);
}


/* Nbs */

int *cncCreateItemSized_Nbs(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_Nbs(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.Nbs, _checkSingleAssignment);
}

void cncGet_Nbs(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.Nbs);
}


