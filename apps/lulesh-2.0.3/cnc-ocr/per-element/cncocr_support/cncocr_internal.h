/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_INTERNAL_H_
#define _CNCOCR_INTERNAL_H_

#include "cncocr.h"

#if CNCOCR_x86
extern void *memcpy(void *dest, const void *src, size_t n);
#    ifndef HAL_X86_64
#    define hal_memCopy(dest, src, len, bg) memcpy(dest, src, len)
#    define hal_fence() __sync_synchronize()
#    endif
#elif CNCOCR_TG
// some HAL definitions (apps/lib/tg/include)
#    include "misc.h"
#else
#    warning UNKNOWN PLATFORM (possibly unsupported)
#endif

#define CNC_TABLE_SIZE 512 /* TODO - Table size should be set by the user when initializing the item collection */

bool _cncPut(ocrGuid_t item, unsigned char *tag, int tagLength, cncItemCollection_t collection, bool isSingleAssignment);
bool _cncPutSingleton(ocrGuid_t item, ocrGuid_t collection, bool isSingleAssignment);

void _cncGet(unsigned char *tag, int tagLength, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, cncItemCollection_t collection);
void _cncGetSingleton(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, ocrGuid_t collection);

ocrEdtDep_t _cncRangedInputAlloc(u32 n, u32 dims[], size_t itemSize);

#endif /*_CNCOCR_INTERNAL_H_*/
