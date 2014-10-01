#define __OCR__
#include "ocr.h"

#define N 1000000
#define T 1

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

ocrGuid_t dataparallelEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 *dbPtr = (u32*)depv[0].ptr;
    u32 idx = (u32)ocrDataParallelGetCurrentIteration();
    dbPtr[idx] += idx;
    compdelay(10000);
    return NULL_GUID;
}

ocrGuid_t awaitingEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i;
#if (T == 1)
    gettimeofday(&b,0);
#endif
    u32 *dbPtr = (u32*)depv[1].ptr;
    for (i = 0; i < N; i++) {
        if (dbPtr[i] != i * 2)
            break;
    }

    if (i == N) {
        PRINTF("Passed Verification\n");
    } else {
        PRINTF("!!! FAILED !!! Verification\n");
    }

#if (T == 1)
    printf("The computation took %f seconds\r\n",
           ((b.tv_sec - a.tv_sec)*1000000+(b.tv_usec - a.tv_usec))*1.0/1000000);
#endif

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
        ptr[i] = i;

#if (T == 1)
    gettimeofday(&a,0);
#endif
    // DATA PARALLEL EDT
    ocrGuid_t dataparallelTemplGuid, dataparallelEdtGuid, dataparallelEventGuid;
    ocrEdtTemplateCreate(&dataparallelTemplGuid, dataparallelEdt, 0 /*paramc*/, 1 /*depc*/);
    ocrParallelFor(&dataparallelEdtGuid, dataparallelTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        N, EDT_PROP_NONE, NULL_GUID, &dataparallelEventGuid);

    // AWAIT
    ocrGuid_t awaitingTemplGuid, awaitingEdtGuid;
    ocrEdtTemplateCreate(&awaitingTemplGuid, awaitingEdt, 0 /*paramc*/, 2 /*depc*/);
    ocrEdtCreate(&awaitingEdtGuid, awaitingTemplGuid, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(dataparallelEventGuid, awaitingEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence(dbGuid, awaitingEdtGuid, 1, DB_DEFAULT_MODE);

    // START
    ocrAddDependence(dbGuid, dataparallelEdtGuid, 0, DB_DEFAULT_MODE);

    return NULL_GUID;
}

