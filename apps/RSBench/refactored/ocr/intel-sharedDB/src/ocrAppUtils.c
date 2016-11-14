#ifndef __OCRAPPUTILITIES_HEADER_H__
#define __OCRAPPUTILITIES_HEADER_H__

#include "ocr.h"
#include "extensions/ocr-affinity.h" //needed for affinity

void createEventHelper(ocrGuid_t * evtGuid, u32 nbDeps) {
#ifdef WITH_COUNTED_EVT
    ocrEventParams_t params;
    params.EVENT_COUNTED.nbDeps = nbDeps;
    ocrEventCreateParams(evtGuid, OCR_EVENT_COUNTED_T, false, &params);
#else
    ocrEventCreate(evtGuid, OCR_EVENT_STICKY_T, false);
#endif
}

void getAffinityHintsForDBandEdt( ocrHint_t* PTR_myDbkAffinityHNT, ocrHint_t* PTR_myEdtAffinityHNT )
{
    ocrGuid_t currentAffinity = NULL_GUID;

    ocrHintInit( PTR_myEdtAffinityHNT, OCR_HINT_EDT_T );
    ocrHintInit( PTR_myDbkAffinityHNT, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( PTR_myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( PTR_myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

}

#endif
