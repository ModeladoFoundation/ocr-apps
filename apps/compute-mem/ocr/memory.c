
#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

#include <getopt.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"


#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

#define CONSTANT_LARGE_NUMBER 1200000

#define THRESHOLD 15//00

ocrGuid_t templateMemSpawner;
ocrGuid_t event_mem_guid[THRESHOLD];
ocrHint_t memory_app_type_hint;
int iteration;


ocrGuid_t wrap_mem_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  PRINTF("\nFinishing memory app (instances=%llu)", instances);
    if(instances==0)
      instances++;
    else
      ocrShutdown();
}

ocrGuid_t spawn_and_mem_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    PRINTF("\nMemory spawned EDT");  fflush(stdout);
    u64 j, k;

    long i;

//Spawn more tasks to generate parallelism before doing useful work
 
  
    u64 *func_args = paramv;
    u64 index = func_args[0];

    u64 func_args_new[1];
    func_args_new[0] = index+1;
    ocrGuid_t affinity = NULL_GUID;

    ocrGuid_t memoryEdtGuid;
      
    if(index!=THRESHOLD-1){
        ocrEdtCreate(&memoryEdtGuid, templateMemSpawner, 1, func_args_new, 1, NULL, PROPERTIES, affinity, NULL);//, tag);
     
        ocrSetHint(memoryEdtGuid, &memory_app_type_hint);
        
        ocrGuid_t depv_guid;
        void* db;
        ocrDbCreate(&depv_guid, &db,
            sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
        ocrAddDependence(depv_guid, memoryEdtGuid, 0, DB_DEFAULT_MODE);
    }

    double sum[1];
    double* a = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);
    double* b = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);
    double* c = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);

// init
    for(i=0;i<CONSTANT_LARGE_NUMBER;i++){
        b[i] = i;
    }

// copy
    for(i=0;i<CONSTANT_LARGE_NUMBER;i++){
            a[i] = b[i];
            sum[0]+=a[i]; // prevents compiler from optimizing and removing array copy
    }


//Satisfy wrap-up task
    ocrGuid_t db_guid;
    void * temp;
    temp = &sum;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                            FLAGS, NULL_GUID, NO_ALLOC);
    
    ocrEventSatisfy(event_mem_guid[index], db_guid);

    return NULL_GUID;
}

ocrGuid_t main2Edt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {

    PRINTF("\nMemory main EDT");fflush(stdout);

    u64 app_tag = 1;

    ocrHintInit(&memory_app_type_hint, OCR_HINT_EDT_T);
    ocrSetHintValue(&memory_app_type_hint, OCR_HINT_EDT_TASK_TYPE, app_tag);


    u64 func_args[1];
    func_args[0] = 0;

    ocrGuid_t affinity = NULL_GUID;
    ocrGuid_t templateWrap;
    ocrEdtTemplateCreate(&templateWrap, wrap_mem_task, 0, THRESHOLD);

    ocrGuid_t wrapEdtGuid;
    u64 tag = 2;
    tag = 1;
    ocrEdtCreate/*Tag*/(&wrapEdtGuid, templateWrap, 0, NULL, THRESHOLD, NULL, PROPERTIES, affinity, NULL);//, tag);
    
    u64 i;
    for(i=0;i<THRESHOLD;i++){
        ocrEventCreate(&(event_mem_guid[i]), OCR_EVENT_STICKY_T, TRUE);
        ocrAddDependence(event_mem_guid[i], wrapEdtGuid, i, DB_MODE_ITW);
    }

    ocrSetHint(wrapEdtGuid, &memory_app_type_hint);

    ocrEdtTemplateCreate(&templateMemSpawner, spawn_and_mem_task, 1, 1);
   
    ocrGuid_t memoryEdtGuid; 
    ocrEdtCreate/*Tag*/(&memoryEdtGuid, templateMemSpawner, 1, func_args, 1, NULL, PROPERTIES, affinity, NULL);//, tag);

    ocrSetHint(memoryEdtGuid, &memory_app_type_hint);

    ocrGuid_t depv_guid;
    void* db;
    ocrDbCreate(&depv_guid, &db,
       sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    ocrAddDependence(depv_guid, memoryEdtGuid, 0, DB_DEFAULT_MODE);
}
