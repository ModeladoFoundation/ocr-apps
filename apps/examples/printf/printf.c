#include "ocr.h"
#include "utils/bin-heap.h"

ocrGuid_t f( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    const u64 n = paramv[0];
    PRINTF("Hello from %lu\n", n);
    if (n == 0) ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    //////////////////////////////////////
    ocrPolicyDomain_t *pd;
    getCurrentEnv(&pd, NULL, NULL, NULL);
    binHeap_t *heap = newBinHeap(pd, LOCKED_BIN_HEAP);
    u8 *p = (u8*)1;
    int i;
    for (i=0; i<30; i+=3) {
        const int n = i%10;
        heap->push(heap, p+n, (u32)(u64)p+n, 0);
        PRINTF("%p\n", p+n);
    }
    PRINTF("==\n", p+i);
    for (i=0; i<10; i++) {
        u8 *q = heap->pop(heap, 0);
        PRINTF("%p\n", q);
    }
    PRINTF("Hello from mainEdt()\n");
    /////////////////////////////////////
    ocrGuid_t template, edt, event;
    ocrEventCreate(&event, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
    ocrEdtTemplateCreate(&template, f, 1, 1);
    for (i=0; i<30; i+=3) {
        u64 n = i%10;
        ocrEdtCreate(&edt, template, 1, &n, 1, &event, EDT_PROP_NONE, NULL_GUID, NULL);
        { // OCR hints
            ocrHint_t _stepHints;
            ocrHintInit(&_stepHints, OCR_HINT_EDT_T);
            u64 _hintVal = n;
            ocrSetHintValue(&_stepHints, OCR_HINT_EDT_PRIORITY, _hintVal);
            ocrSetHint(edt, &_stepHints);
        }
        PRINTF("CREATE %lu\n", n);
    }
    ocrEventSatisfy(event, NULL_GUID);
    return NULL_GUID;
}
