#define __OCR__
#include "ocr.h"

#define N           1000000

ocrGuid_t dataparallelEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 *dbPtr = (u32*)depv[0].ptr;
    u32 idx = (u32)ocrDataParallelGetCurrentIteration();
    dbPtr[idx] += idx;
    //PRINTF("Data Parallel EDT iteration: %lu\n", ocrDataParallelGetCurrentIteration());
    return NULL_GUID;
}

ocrGuid_t awaitingEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i;
    PRINTF("Data Parallel finish!\n");
    u32 *dbPtr = (u32*)depv[1].ptr;
    for (i = 0; i < N; i++) {
        if (dbPtr[i] != i)
            break;
    }

    if (i == N) {
        PRINTF("Passed Verification\n");
    } else {
        PRINTF("!!! FAILED !!! Verification\n");
    }

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i;

    // CHECKER DB
    u32* ptr;
    ocrGuid_t dbGuid;
    ocrDbCreate(&dbGuid, (void**)&ptr, N * sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    for(i = 0; i < N; i++)
        ptr[i] = 0;

    // DATA PARALLEL EDT
    ocrGuid_t dataparallelTemplGuid, dataparallelEdtGuid, dataparallelEventGuid;
    ocrEdtTemplateCreate(&dataparallelTemplGuid, dataparallelEdt, 0 /*paramc*/, 1 /*depc*/);
    ocrDataParallelEdtCreate(&dataparallelEdtGuid, dataparallelTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, EDT_PROP_NONE, NULL_GUID, &dataparallelEventGuid);

    // AWAIT
    ocrGuid_t awaitingTemplGuid, awaitingEdtGuid;
    ocrEdtTemplateCreate(&awaitingTemplGuid, awaitingEdt, 0 /*paramc*/, 2 /*depc*/);
    ocrEdtCreate(&awaitingEdtGuid, awaitingTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(dataparallelEventGuid, awaitingEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dbGuid, awaitingEdtGuid, 1, DB_DEFAULT_MODE);

    // START
    PRINTF("Data Parallel Start!\n");
    ocrAddDependence(dbGuid, dataparallelEdtGuid, 0, DB_DEFAULT_MODE);

    return NULL_GUID;
}

