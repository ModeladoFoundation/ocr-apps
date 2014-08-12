#define __OCR__
#include "ocr.h"

#define N 1000000

ocrGuid_t childEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 *dbPtr = (u32*)depv[0].ptr;
    u32 idx = (u32)ocrDataParallelGetCurrentIteration(); 
    dbPtr[idx] += idx;
    //PRINTF("The child is running. %lu\n", ocrDataParallelGetCurrentIteration());
    return NULL_GUID;
}

ocrGuid_t finishScopeEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Start finish!\n");
    ocrGuid_t childTemplGuid, childEdtGuid;
    ocrEdtTemplateCreate(&childTemplGuid, childEdt, 0 /*paramc*/, 1 /*depc*/);
    ocrDataParallelEdtCreate(&childEdtGuid, childTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, EDT_PROP_DATA_PARALLEL, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, childEdtGuid, 0, DB_DEFAULT_MODE);
    return NULL_GUID;
}

ocrGuid_t awaitingEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i;
    PRINTF("End finish!\n");
    u32 *dbPtr = (u32*)depv[1].ptr;
    for (i = 0; i < N; i++) {
        if (dbPtr[i] != i) {
            PRINTF("!!! FAILED !!! Verification\n");
            break;
        }
    }
    if (i == N)
        PRINTF("Passed Verification\n");
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

    // FINISH SCOPE
    ocrGuid_t finishTemplGuid, finishEdtGuid, finishEventGuid;
    ocrEdtTemplateCreate(&finishTemplGuid, finishScopeEdt, 0 /*paramc*/, 1 /*depc*/);
    ocrEdtCreate(&finishEdtGuid, finishTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_FINISH, NULL_GUID, &finishEventGuid);

    // AWAIT
    ocrGuid_t awaitingTemplGuid, awaitingEdtGuid;
    ocrEdtTemplateCreate(&awaitingTemplGuid, awaitingEdt, 0 /*paramc*/, 2 /*depc*/);
    ocrEdtCreate(&awaitingEdtGuid, awaitingTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(finishEventGuid, awaitingEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dbGuid, awaitingEdtGuid, 1, DB_DEFAULT_MODE);

    // START
    ocrAddDependence(dbGuid, finishEdtGuid, 0, DB_DEFAULT_MODE);

    return NULL_GUID;
}

