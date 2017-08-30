/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#define _GNU_SOURCE
#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define ENABLE_EXTENSION_LABELING
#include "extensions/ocr-labeling.h"

#include <math.h>
#include <float.h>

#if 0
#define FANOUT  100
#define DEPTH 200
#define TOTALDB (FANOUT*DEPTH)
#endif

u64 FANOUT = 100, DEPTH = 200, TOTALDB = 20000, SIZE = 1024;

#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#include <sys/time.h>

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;

        gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

double start_time = 0.0;
ocrGuid_t tmp_create, tmp_generate, tmp_destroy;
ocrGuid_t evtMap[100000];

ocrGuid_t destroy(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 depth = paramv[0];
    u64 inst = paramv[1];
    u64 index = paramv[2];

    ocrDbDestroy(depv[0].guid);
    ocrEventDestroy(evtMap[index]);
    if((depth == DEPTH+1) && (inst == FANOUT-1)) {
        PRINTF("Total time %f\n", mysecond()-start_time);
        ocrShutdown();
    }
    // else for(i = 0; i<depth*2; i++) PRINTF("  ");  PRINTF("C: Depth %ld, inst %ld "GUIDF"\n", depth, inst, GUIDA(*(ocrGuid_t*)&paramv[1]));

    return NULL_GUID;
}

ocrGuid_t create(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t db;
    void *ptr;
    ocrDbCreate(&db, &ptr, SIZE, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    return db;
}

ocrGuid_t generate(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 i, j;
    u64 depth = paramv[0];
    u64 delta = FANOUT - depth * FANOUT/DEPTH;
    ocrGuid_t scratchEdt;
    u64 myparamv[3];

    if(depth > DEPTH) return NULL_GUID;
    paramv[0] = depth+1;

//PRINTF("Depth %ld, delta %ld\n", depth, delta);
    for(i = 0; i < delta; i++) {
        ocrGuid_t outputEvt;
        u32 index = 0;
        for(j = 0; j < depth; j++) index += (FANOUT-j*FANOUT/DEPTH); index += i;
        ocrEdtCreate(&scratchEdt, tmp_create, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, &outputEvt);
        ocrAddDependence(outputEvt, evtMap[index], 0, DB_MODE_RW);
        ocrAddDependence(NULL_GUID, scratchEdt, 0, DB_MODE_NULL);
//PRINTF("Create %d "GUIDF"\n", index, evtMap[index]);
    }

    myparamv[0] = paramv[0];
    for(i = delta; i<FANOUT; i++) {
        u32 index = 0;
        myparamv[1] = i;
        for(j = 0; j < depth; j++) index += j*FANOUT/DEPTH; index += (i-delta);
        myparamv[2] = index;
        ocrEdtCreate(&scratchEdt, tmp_destroy, EDT_PARAM_DEF, myparamv, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, NULL);
//PRINTF("Destroy %d "GUIDF"\n", index, evtMap[index]);
        ocrAddDependence(evtMap[index], scratchEdt, 0, DB_MODE_RO);
    }

    ocrEdtCreate(&scratchEdt, tmp_generate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, NULL);
    ocrAddDependence(NULL_GUID, scratchEdt, 0, DB_MODE_NULL);

    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 i;
    ocrGuid_t genEdt;
    u64 param[] = {0};

    i = getArgc(depv[0].ptr);
    FANOUT = atoi(getArgv(depv[0].ptr, 1));
    DEPTH = atoi(getArgv(depv[0].ptr, 2));
    SIZE = atoi(getArgv(depv[0].ptr, 3));
    TOTALDB = FANOUT*DEPTH;

    // Spawn the threads
    ocrEdtTemplateCreate(&tmp_generate, generate, 1, 1);
    ocrEdtTemplateCreate(&tmp_create, create, 1, 1);
    ocrEdtTemplateCreate(&tmp_destroy, destroy, 3, 1);
    for(i = 0; i<TOTALDB; i++) ocrEventCreate(&evtMap[i], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    start_time = mysecond();
    ocrEdtCreate(&genEdt, tmp_generate, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, NULL);
    ocrAddDependence(NULL_GUID, genEdt, 0, DB_MODE_NULL);

    return NULL_GUID;
}
