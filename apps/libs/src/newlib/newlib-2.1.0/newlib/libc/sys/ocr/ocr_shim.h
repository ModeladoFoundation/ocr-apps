#ifndef _OCR_SHIM_
#define _OCR_SHIM_

#include <sys/ocr.h>
/*
 * OCR Shim Methods
 * These methods support translating between the libc notions of open
 * file ids and the associated OCR Guids
 */
short ocrLibAddGuid( struct _reent * r, ocrGuid_t guid );
short ocrLibGuidToFd( struct _reent * r, ocrGuid_t guid );
ocrGuid_t ocrLibRmFd( struct _reent * r, short fd );
ocrGuid_t ocrLibFdToGuid( struct _reent * r, short fd );

#endif // _OCR_SHIM_
