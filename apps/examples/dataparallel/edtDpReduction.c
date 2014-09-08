#define __OCR__
#include "ocr.h"

#define N           10000
#define INIT_VAL    0

ocrGuid_t dataparallelReductionEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 *dbPtr = (u64*)depv[0].ptr;
    u64 idx = (u64)ocrDataParallelGetCurrentIteration();
    dbPtr[idx] += idx;
    //PRINTF("Data Parallel EDT iteration: %u, DB: %u\n", idx, dbPtr[idx]);
    ocrReduce((void *)(&dbPtr[idx]));
    return NULL_GUID;
}

void userReductionFunction(void *partial, void *element) {
    *((u64*)partial) += *((u64*)element);
}

ocrGuid_t awaitingEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
    PRINTF("Data Parallel finish!\n");

    u64 expectedResult = (N * (N - 1)) / 2; //+[0...N)

    u64 *dbPtr1 = (u64*)depv[0].ptr;
    u64 *dbPtr2 = (u64*)depv[1].ptr;
    u64 *computedResult1 = (u64*)depv[2].ptr;
    u64 *computedResult2 = (u64*)depv[3].ptr;

    ASSERT(computedResult1);
    ASSERT(computedResult2);

    for (i = 0; i < N; i++) {
        if ((dbPtr1[i] != i) || (dbPtr2[i] != i))
            break;
    }

    if (i == N && *computedResult1 == expectedResult && *computedResult2 == expectedResult) {
        PRINTF("Passed Verification\n");
    } else {
        PRINTF("!!! FAILED !!! Verification (Expected: %lu Computed1: %lu Computed2: %lu)\n", expectedResult, *computedResult1, *computedResult2);
    }

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;

    // CHECKER DBs
    u64 *ptr1, *ptr2;
    ocrGuid_t dbGuid1, dbGuid2;
    ocrDbCreate(&dbGuid1, (void**)&ptr1, N * sizeof(u64), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&dbGuid2, (void**)&ptr2, N * sizeof(u64), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    for(i = 0; i < N; i++) {
        ptr1[i] = 0; //maybe replace with a rand function
        ptr2[i] = 0; //maybe replace with a rand function
    }

    // DATA PARALLEL REDUCTION EDTS
    ocrGuid_t dataparallelTemplGuid, dataparallelReductionEdtGuid, dataparallelReductionEventGuid, dataparallelUserReductionEdtGuid, dataparallelUserReductionEventGuid;
    ocrEdtTemplateCreate(&dataparallelTemplGuid, dataparallelReductionEdt, 0 /*paramc*/, 1 /*depc*/);
    u64 initVal = INIT_VAL;

    // BUILTIN REDUCTION OP
    ocrParallelReduce(&dataparallelReductionEdtGuid, dataparallelTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, REDUCTION_OP_SUM, REDUCTION_TYPE_U64, sizeof(u64), NULL, (void *)(&initVal), EDT_PROP_NONE, NULL_GUID, &dataparallelReductionEventGuid);

    // USER REDUCTION OP
    ocrParallelReduce(&dataparallelUserReductionEdtGuid, dataparallelTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, REDUCTION_OP_USER, REDUCTION_TYPE_USER, sizeof(u64), userReductionFunction, (void *)(&initVal), EDT_PROP_NONE, NULL_GUID, &dataparallelUserReductionEventGuid);

    // AWAIT
    ocrGuid_t awaitingTemplGuid, awaitingEdtGuid;
    ocrEdtTemplateCreate(&awaitingTemplGuid, awaitingEdt, 0 /*paramc*/, 4 /*depc*/);
    ocrEdtCreate(&awaitingEdtGuid, awaitingTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(dbGuid1, awaitingEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dbGuid2, awaitingEdtGuid, 1, DB_DEFAULT_MODE);
    ocrAddDependence(dataparallelReductionEventGuid, awaitingEdtGuid, 2, DB_DEFAULT_MODE);
    ocrAddDependence(dataparallelUserReductionEventGuid, awaitingEdtGuid, 3, DB_DEFAULT_MODE);

    // START
    PRINTF("Data Parallel Start!\n");
    ocrAddDependence(dbGuid1, dataparallelReductionEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dbGuid2, dataparallelUserReductionEdtGuid, 0, DB_DEFAULT_MODE);

    return NULL_GUID;
}

