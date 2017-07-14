/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include <ocr.h>
#include <sys/time.h>

#if !defined(NTHREADS)
#define NTHREADS 16
#endif
#if !defined(DBSIZE)
#define DBSIZE   64*1024*1024
#endif
#if !defined(ITER)
#define ITER     200
#endif

struct timeval tv1, tv2;

ocrGuid_t done(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    gettimeofday(&tv2, NULL);
    PRINTF("Time: %d ms\n", ((tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec))/1000);
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t work(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i, j;
    u8 *ptr = depv[0].ptr;

    for(i = 0; i<ITER; i++) {
        for(j = 0; j<DBSIZE; j++) {
            ptr[j]++ ;
        }
    }
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t twork, tdone, workedt, doneedt, event;
    ocrGuid_t dbs[NTHREADS];
    void *ptr;
    u32 i;

    gettimeofday(&tv1, NULL);

    ocrHint_t myHint;
    ocrHintInit(&myHint, OCR_HINT_DB_T);
    ocrSetHintValue(&myHint, OCR_HINT_DB_HIGHBW, 1);

    for(i = 0; i < NTHREADS; i++) ocrDbCreate(&dbs[i], &ptr, sizeof(char)*DBSIZE, DB_PROP_NONE, &myHint, NO_ALLOC);

    ocrEdtTemplateCreate(&twork  , work  , 0, 1);
    ocrEdtTemplateCreate(&tdone , done , 0, NTHREADS);
    ocrEdtCreate(&doneedt, tdone , 0, NULL, NTHREADS, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    for(i = 0; i < NTHREADS; i++) {
        ocrEdtCreate(&workedt, twork, 0, NULL, 1, &dbs[i], EDT_PROP_NONE, NULL_HINT, &event);
        ocrAddDependence(event, doneedt, i, DB_MODE_NULL);
    }

    return NULL_GUID;
}
