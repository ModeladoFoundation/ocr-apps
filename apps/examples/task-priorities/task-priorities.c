#include "ocr.h"

ocrGuid_t f(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    const u64 n = paramv[0];
    PRINTF("Hello from %lu\n", n);
    return NULL_GUID;
}

ocrGuid_t launcherEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t template, edt, depEvent;
    ocrEventCreate(&depEvent, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
    ocrEdtTemplateCreate(&template, f, 1, 1);

    int i;
    for (i=0; i<30; i+=3) {
        u64 n = i%10;
        ocrEdtCreate(&edt, template, 1, &n, 1, &depEvent, EDT_PROP_NONE, NULL_HINT, NULL);
        { // OCR hints
            ocrHint_t _stepHints;
            ocrHintInit(&_stepHints, OCR_HINT_EDT_T);
            u64 _hintVal = n;
            ocrSetHintValue(&_stepHints, OCR_HINT_EDT_PRIORITY, _hintVal);
            ocrSetHint(edt, &_stepHints);
        }
        PRINTF("CREATE %lu\n", n);
    }

    ocrEventSatisfy(depEvent, NULL_GUID);

    return NULL_GUID;
}

ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    PRINTF("Hello from mainEdt()\n");

    ocrGuid_t template, edt, depEvent, outEvent;
    ocrEventCreate(&depEvent, OCR_EVENT_ONCE_T, EVT_PROP_NONE);

    ocrEdtTemplateCreate(&template, launcherEdt, 0, 1);
    ocrEdtCreate(&edt, template, 0, NULL, 1, &depEvent, EDT_PROP_FINISH, NULL_HINT, &outEvent);

    ocrEdtTemplateCreate(&template, shutdownEdt, 0, 1);
    ocrEdtCreate(&edt, template, 0, NULL, 1, &outEvent, EDT_PROP_FINISH, NULL_HINT, NULL);

    ocrEventSatisfy(depEvent, NULL_GUID);

    return NULL_GUID;
}
