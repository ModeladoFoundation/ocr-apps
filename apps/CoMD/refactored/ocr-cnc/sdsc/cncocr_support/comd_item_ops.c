/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "comd_internal.h"


/* B */

struct box *cncItemCreateSized_B(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_B(struct box *_item, cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT B @ %ld, %ld, %ld, %ld\n", i, j, k, l);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT B @ %ld, %ld, %ld, %ld\n", i, j, k, l);
#endif
    cncTag_t _tag[] = { i, j, k, l };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.B, _checkSingleAssignment);
}

void cncGet_B(cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP B @ %ld, %ld, %ld, %ld\n", i, j, k, l);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP B @ %ld, %ld, %ld, %ld\n", i, j, k, l);
#endif
    cncTag_t tag[] = { i, j, k, l };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.B);
}


/* SF */

SimFlat *cncItemCreateSized_SF(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_SF(SimFlat *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT SF @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT SF @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.SF, _checkSingleAssignment);
}

void cncGet_SF(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP SF @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP SF @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.SF);
}


/* POT */

LjPotential *cncItemCreateSized_POT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_POT(LjPotential *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT POT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT POT @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.POT, _checkSingleAssignment);
}

void cncGet_POT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP POT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP POT @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.POT);
}


/* EAMPOT */

struct eamPot *cncItemCreateSized_EAMPOT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_EAMPOT(struct eamPot *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT EAMPOT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT EAMPOT @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.EAMPOT, _checkSingleAssignment);
}

void cncGet_EAMPOT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP EAMPOT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP EAMPOT @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.EAMPOT);
}


/* SPECIES */

SpeciesData *cncItemCreateSized_SPECIES(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_SPECIES(SpeciesData *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT SPECIES @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT SPECIES @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.SPECIES, _checkSingleAssignment);
}

void cncGet_SPECIES(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP SPECIES @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP SPECIES @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.SPECIES);
}


/* DD */

Domain *cncItemCreateSized_DD(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_DD(Domain *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT DD @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT DD @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.DD, _checkSingleAssignment);
}

void cncGet_DD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP DD @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP DD @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.DD);
}


/* LC */

LinkCell *cncItemCreateSized_LC(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_LC(LinkCell *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT LC @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT LC @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.LC, _checkSingleAssignment);
}

void cncGet_LC(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP LC @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP LC @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.LC);
}


/* NAtoms */

int *cncItemCreateSized_NAtoms(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_NAtoms(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT NAtoms @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT NAtoms @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.NAtoms, _checkSingleAssignment);
}

void cncGet_NAtoms(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP NAtoms @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP NAtoms @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.NAtoms);
}


/* redc */

struct myReduction *cncItemCreateSized_redc(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_redc(struct myReduction *_item, cncTag_t i, cncTag_t j, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT redc @ %ld, %ld\n", i, j);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT redc @ %ld, %ld\n", i, j);
#endif
    cncTag_t _tag[] = { i, j };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.redc, _checkSingleAssignment);
}

void cncGet_redc(cncTag_t i, cncTag_t j, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP redc @ %ld, %ld\n", i, j);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP redc @ %ld, %ld\n", i, j);
#endif
    cncTag_t tag[] = { i, j };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.redc);
}


/* AtomInfo */

struct atomInfo *cncItemCreateSized_AtomInfo(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_AtomInfo(struct atomInfo *_item, cncTag_t i, cncTag_t j, cncTag_t k, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT AtomInfo @ %ld, %ld, %ld\n", i, j, k);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT AtomInfo @ %ld, %ld, %ld\n", i, j, k);
#endif
    cncTag_t _tag[] = { i, j, k };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.AtomInfo, _checkSingleAssignment);
}

void cncGet_AtomInfo(cncTag_t i, cncTag_t j, cncTag_t k, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP AtomInfo @ %ld, %ld, %ld\n", i, j, k);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP AtomInfo @ %ld, %ld, %ld\n", i, j, k);
#endif
    cncTag_t tag[] = { i, j, k };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.AtomInfo);
}


/* CMD */

struct cmdInfo *cncItemCreateSized_CMD(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_CMD(struct cmdInfo *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT CMD @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT CMD @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.CMD, _checkSingleAssignment);
}

void cncGet_CMD(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP CMD @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP CMD @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.CMD);
}


/* time */

struct timeval *cncItemCreateSized_time(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_time(struct timeval *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT time @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT time @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.time, _checkSingleAssignment);
}

void cncGet_time(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP time @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP time @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.time);
}


/* ATOMS */

Atoms *cncItemCreateSized_ATOMS(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_ATOMS(Atoms *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT ATOMS @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT ATOMS @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.ATOMS, _checkSingleAssignment);
}

void cncGet_ATOMS(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP ATOMS @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP ATOMS @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.ATOMS);
}


/* GID */

int *cncItemCreateSized_GID(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_GID(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT GID @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT GID @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.GID, _checkSingleAssignment);
}

void cncGet_GID(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP GID @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP GID @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.GID);
}


/* ISP */

int *cncItemCreateSized_ISP(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_ISP(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT ISP @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT ISP @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.ISP, _checkSingleAssignment);
}

void cncGet_ISP(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP ISP @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP ISP @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.ISP);
}


/* R */

real3 *cncItemCreateSized_R(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_R(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT R @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT R @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.R, _checkSingleAssignment);
}

void cncGet_R(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP R @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP R @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.R);
}


/* P */

real3 *cncItemCreateSized_P(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_P(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT P @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT P @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.P, _checkSingleAssignment);
}

void cncGet_P(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP P @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP P @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.P);
}


/* F */

real3 *cncItemCreateSized_F(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_F(real3 *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT F @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT F @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.F, _checkSingleAssignment);
}

void cncGet_F(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP F @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP F @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.F);
}


/* U */

real_t *cncItemCreateSized_U(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_U(real_t *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT U @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT U @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.U, _checkSingleAssignment);
}

void cncGet_U(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP U @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP U @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.U);
}


/* IT */

int *cncItemCreateSized_IT(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_IT(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT IT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT IT @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.IT, _checkSingleAssignment);
}

void cncGet_IT(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP IT @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP IT @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.IT);
}


/* TBoxes */

int *cncItemCreateSized_TBoxes(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_TBoxes(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT TBoxes @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT TBoxes @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.TBoxes, _checkSingleAssignment);
}

void cncGet_TBoxes(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP TBoxes @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP TBoxes @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.TBoxes);
}


/* Nbs */

int *cncItemCreateSized_Nbs(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_Nbs(int *_item, cncTag_t i, bool _checkSingleAssignment, comdCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "PUT Nbs @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: PUT Nbs @ %ld\n", i);
#endif
    cncTag_t _tag[] = { i };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.Nbs, _checkSingleAssignment);
}

void cncGet_Nbs(cncTag_t i, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, comdCtx *ctx) {
    #ifdef CNC_DEBUG_LOG
    fprintf(cncDebugLog, "GET-DEP Nbs @ %ld\n", i);
    fflush(cncDebugLog);
#elif CNC_DEBUG_TRACE
    PRINTF("<<CnC Trace>>: GET-DEP Nbs @ %ld\n", i);
#endif
    cncTag_t tag[] = { i };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.Nbs);
}


