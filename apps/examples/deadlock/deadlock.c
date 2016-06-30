//
// A demonstration of the work-shift strategy causing deadlock
// in a legal OCR program on the x86-mpi platform.
//
// Written by Sara Hamouda and Nick Vrvilo.
//
// Expects to be run over MPI with 2 ranks (will fail otherwise).
//
// When run with CONFIG_NUM_THREADS=2, this code will deadlock.
// When run with any greater number of threads, it should complete successfully.
// (Remember that 2 threads means 1 communication worker and 1 compute worker.)
//

#include "ocr.h"
#define ENABLE_EXTENSION_AFFINITY
#include <extensions/ocr-affinity.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define SZ_32MB ((size_t)(1L << 25))

#define SLEEP() sleep(2)

typedef struct {
    volatile u8 flag;
    ocrGuid_t signal;
    ocrGuid_t affinities[2];
} DbData;

ocrGuid_t aEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    DbData *data = depv[0].ptr;
    ocrGuid_t *affinities = data->affinities;
    char name[2] = { paramv[0], 0 };
    assert(name[0] == 'A');

    PRINTF("%s  started ...\n", name);

    ocrHint_t p1DbAffinityHint;
    ocrHintInit(&p1DbAffinityHint,OCR_HINT_DB_T);
    ocrSetHintValue(&p1DbAffinityHint, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(affinities[1]));

    ocrEventSatisfy(data->signal, NULL_GUID);

    // Create a nice, big, remote datablock.
    // On x86-mpi, this operation will cause the current worker thread
    // to perform a "work-shift" to try to keep busy while communicating
    // with the remote policy domain.
    ocrGuid_t Y_DbGuid;
    void *Y_Ptr;
    ocrDbCreate(&Y_DbGuid, &Y_Ptr, SZ_32MB, DB_PROP_NONE, &p1DbAffinityHint, NO_ALLOC);

    PRINTF("%s (DB created) ...\n", name);

    data->flag = 1;

    SLEEP();
    PRINTF("%s  ended ...\n", name);
    return NULL_GUID;
}

ocrGuid_t bEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    DbData *data = depv[0].ptr;
    char name[2] = { paramv[0], 0 };
    assert(name[0] == 'B');

    PRINTF("%s  started ...\n", name);

    // NOTE: The OCR v1.1.0 spec specifically states in section 1.1.4
    // that it is legal for OCR programs to contain data races.
    // Furthermore, this statement is made specifically in the context
    // of defining Read-Write Mode access to datablocks, implying that
    // such accesses may race. I.e., as per the spec, I believe this
    // busy-loop on the "flag" value is completely legal.
    while (data->flag == 0) {
        PRINTF("%s is spinning...\n", name);
        SLEEP();
    }

    SLEEP();

    PRINTF("%s  ended ...\n", name);
    return NULL_GUID;
}

ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("shutdownEdt  started ...\n");

    ocrShutdown();

    PRINTF("shutdownEdt  ended ...\n");
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    //create templates
    ocrGuid_t aEdtTemplateGuid, bEdtTemplateGuid, shutdownEdtTemplateGuid;
    ocrEdtTemplateCreate(&aEdtTemplateGuid, aEdt, 1 /*paramc*/, 1 /*depc*/);
    ocrEdtTemplateCreate(&bEdtTemplateGuid, bEdt, 1 /*paramc*/, 2 /*depc*/);
    ocrEdtTemplateCreate(&shutdownEdtTemplateGuid, shutdownEdt, 0 /*paramc*/, 2 /*depc*/);

    ocrGuid_t affinities[2];
    u64 affinityCount = 2;
    ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
    assert(affinityCount == 2);

    // EDT Hint
    ocrHint_t p0EdtAffinityHint;
    ocrHintInit(&p0EdtAffinityHint,OCR_HINT_EDT_T);
    ocrSetHintValue(&p0EdtAffinityHint, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affinities[0]));

    // DB hint
    ocrHint_t p0DbAffinityHint;
    ocrHintInit(&p0DbAffinityHint,OCR_HINT_DB_T);
    ocrSetHintValue(&p0DbAffinityHint, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(affinities[0]));

    ocrGuid_t signalGuid;
    ocrEventCreate(&signalGuid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);

    ocrGuid_t xDbGuid;
    DbData *xPtr;
    ocrDbCreate(&xDbGuid, (void**)&xPtr, sizeof(*xPtr), DB_PROP_NONE, &p0DbAffinityHint, NO_ALLOC);
    xPtr->flag = 0;
    xPtr->signal = signalGuid;
    xPtr->affinities[0] = affinities[0];
    xPtr->affinities[1] = affinities[1];
    ocrDbRelease(xDbGuid);

    // create EDTs A@P0 and B@P0
    u64 A = 'A';
    u64 B = 'B';
    ocrGuid_t aEventGuid, bEventGuid;
    ocrGuid_t aEdtGuid, bEdtGuid;
    ocrEdtCreate(&aEdtGuid, aEdtTemplateGuid, 1, &A, EDT_PARAM_DEF, NULL,
                      /*prop=*/EDT_PROP_NONE, &p0EdtAffinityHint, &aEventGuid);

    ocrEdtCreate(&bEdtGuid, bEdtTemplateGuid, 1, &B, EDT_PARAM_DEF, NULL,
                          /*prop=*/EDT_PROP_NONE, &p0EdtAffinityHint, &bEventGuid);

    ocrGuid_t shutdownEdtGuid;
    ocrEdtCreate(&shutdownEdtGuid , shutdownEdtTemplateGuid , EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                /*prop=*/EDT_PROP_NONE, &p0EdtAffinityHint, NULL);

    ocrAddDependence(aEventGuid, shutdownEdtGuid, 0, DB_MODE_RO);
    ocrAddDependence(bEventGuid, shutdownEdtGuid, 1, DB_MODE_RO);

    // A and B depend on X in RW mode (i.e., they may access it concurrently)
    ocrAddDependence(xDbGuid, aEdtGuid, 0, DB_MODE_RW);

    ocrAddDependence(xDbGuid,  bEdtGuid, 0, DB_MODE_RW);
    ocrAddDependence(signalGuid, bEdtGuid, 1, DB_MODE_RO);

    return NULL_GUID;
}
