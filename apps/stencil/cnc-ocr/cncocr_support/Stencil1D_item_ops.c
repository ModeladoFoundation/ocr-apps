/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "Stencil1D_internal.h"


/* val */

float *cncCreateItemSized_val(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_val(float *_item, cncTag_t v, cncTag_t t, bool _checkSingleAssignment, Stencil1DCtx *ctx) {
    ocrGuid_t _handle = _cncItemGuid(_item);

    cncTag_t _tag[] = { v, t };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), ctx->_items.val, _checkSingleAssignment);
}

void cncGet_val(cncTag_t v, cncTag_t t, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, Stencil1DCtx *ctx) {

    cncTag_t tag[] = { v, t };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, ctx->_items.val);
}


