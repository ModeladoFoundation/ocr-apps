#define __OCR__
#include "ocr.h"

#define N           10000
#define T           1
#define INIT_VAL    0

#if (T == 1)
#include <stdio.h>
#include <sys/time.h>
struct timeval a,b;
#endif

static void compdelay(int delaylength) {
   int  i;
   float a=0.;
   for (i=0; i<delaylength; i++) a+=i;
   if (a < 0) PRINTF("%f \n",a);
}

ocrGuid_t dataparallelReductionEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 *dbPtr = (u64*)depv[0].ptr;
    u64 idx = (u64)ocrDataParallelGetCurrentIteration();
    dbPtr[idx] += idx;
    ocrReduce((void *)(&idx));
    compdelay(1000000);
    return NULL_GUID;
}

ocrGuid_t awaitingEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;
#if (T == 1)
    gettimeofday(&b,0);
#endif

    u64 expectedResult = (N * (N - 1)) / 2; //+[0...N)

    u64 *dbPtr = (u64*)depv[0].ptr;
    u64 *computedResult = (u64*)depv[1].ptr;
    ASSERT(computedResult);

    for (i = 0; i < N; i++) {
        if (dbPtr[i] != i * 2)
            break;
    }

    if (i == N && *computedResult == expectedResult) {
        PRINTF("Passed Verification\n");
    } else {
        PRINTF("!!! FAILED !!! Verification (Expected: %lu Computed: %lu)\n", expectedResult, *computedResult);
    }

#if (T == 1)
    printf("The computation took %f seconds\r\n",
           ((b.tv_sec - a.tv_sec)*1000000+(b.tv_usec - a.tv_usec))*1.0/1000000);
#endif

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 i;

    // CHECKER DBs
    u64 *ptr1;
    ocrGuid_t dbGuid1;
    ocrDbCreate(&dbGuid1, (void**)&ptr1, N * sizeof(u64), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    for(i = 0; i < N; i++) {
        ptr1[i] = i; //maybe replace with a rand function
    }

#if (T == 1)
    gettimeofday(&a,0);
#endif
    // DATA PARALLEL REDUCTION EDTS
    ocrGuid_t dataparallelTemplGuid, dataparallelReductionEdtGuid, dataparallelReductionEventGuid, dataparallelUserReductionEdtGuid, dataparallelUserReductionEventGuid;
    ocrEdtTemplateCreate(&dataparallelTemplGuid, dataparallelReductionEdt, 0 /*paramc*/, 1 /*depc*/);
    u64 initVal = INIT_VAL;

    // BUILTIN REDUCTION OP
    ocrParallelReduce(&dataparallelReductionEdtGuid, dataparallelTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, REDUCTION_OP_SUM, REDUCTION_TYPE_U64, sizeof(u64), NULL, (void *)(&initVal), EDT_PROP_NONE, NULL_GUID, &dataparallelReductionEventGuid);

    // AWAIT
    ocrGuid_t awaitingTemplGuid, awaitingEdtGuid;
    ocrEdtTemplateCreate(&awaitingTemplGuid, awaitingEdt, 0 /*paramc*/, 2 /*depc*/);
    ocrEdtCreate(&awaitingEdtGuid, awaitingTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(dbGuid1, awaitingEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dataparallelReductionEventGuid, awaitingEdtGuid, 1, DB_DEFAULT_MODE);

    // START
    ocrAddDependence(dbGuid1, dataparallelReductionEdtGuid, 0, DB_DEFAULT_MODE);

    return NULL_GUID;
}

