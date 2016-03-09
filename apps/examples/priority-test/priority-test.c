#include "ocr.h"

ocrGuid_t f(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    const u64 n = paramv[0];
    PRINTF("Hello from %lu\n", n);
    if (n == 0) ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;

    PRINTF("Hello from mainEdt()\n");

    ocrGuid_t template, edt, event;
    ocrEventCreate(&event, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
    ocrEdtTemplateCreate(&template, f, 1, 1);

    for (i=0; i<30; i+=3) {
        u64 n = i%10;
        ocrEdtCreate(&edt, template, 1, &n, 1, &event, EDT_PROP_NONE, NULL_HINT, NULL);
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
