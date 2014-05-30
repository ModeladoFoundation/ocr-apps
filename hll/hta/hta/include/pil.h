#ifndef PIL_H
#define PIL_H

#ifdef PIL2OCR
#include "ocr.h"
typedef ocrGuid_t guid_t;
#else
#define NULL_GUID NULL
typedef void* guid_t;
#endif // PIL2OCR

typedef struct {
	guid_t guid;
	void *ptr;
} gpp_t;

#endif // PIL_H
