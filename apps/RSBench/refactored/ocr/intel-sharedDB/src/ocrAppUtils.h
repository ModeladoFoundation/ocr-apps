#ifndef __OCRAPPUTILITIES_HEADER_H__
#define __OCRAPPUTILITIES_HEADER_H__

#include "ocr.h"
#include "extensions/ocr-affinity.h" //needed for affinity

#define EDT_ARGS u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]

#ifdef DEBUG_APP
    #define DEBUG_PRINTF(X) PRINTF X
#else
    #define DEBUG_PRINTF(X) do {} while(0)
#endif

typedef ocrGuid_t ocrDBK_t;
typedef ocrGuid_t ocrEVT_t;
typedef ocrGuid_t ocrEDT_t;
typedef ocrGuid_t ocrTML_t;

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

void createEventHelper(ocrGuid_t * evtGuid, u32 nbDeps);
void getAffinityHintsForDBandEdt( ocrHint_t* PTR_myDbkAffinityHNT, ocrHint_t* PTR_myEdtAffinityHNT );

#endif
