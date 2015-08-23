
#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

#define CONSTANT_LARGE_NUMBER 6000//0//00

#define THRESHOLD 150//0


//ocrHint_t app_type_hint_2;

ocrGuid_t wrap_task_2 ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
/*    if(instances==0)
        instances++;
    else    
*/        ocrShutdown();
}

ocrGuid_t spawn_and_mem_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    
    u64 j, k;
    long i;

//Spawn more tasks to generate parallelism before doing useful work
  
    u64 *func_args = paramv;
    u64 index = func_args[0];

    u64 func_args_new[2];
    func_args_new[0] = index+1;

    ocrGuid_t wrap_guid = (ocrGuid_t)func_args[1]; //since 0th value is the index itself

    func_args_new[1] = func_args[1];
	
    ocrGuid_t event_guid;
    ocrEventCreate(&event_guid, OCR_EVENT_STICKY_T, TRUE);
    ocrAddDependence(event_guid, wrap_guid, index, DB_MODE_ITW);

    ocrGuid_t affinity = NULL_GUID;

    ocrGuid_t memoryEdtGuid;

    if(index%5==0){

        PRINTF("\nFinished %dth timestep (analytics)", index/*/150*/);
    }

    if(index!=THRESHOLD-1){
	ocrGuid_t templateMemSpawner;
	ocrEdtTemplateCreate(&templateMemSpawner, spawn_and_mem_task, 2, 1);
        ocrEdtCreate(&memoryEdtGuid, templateMemSpawner, 2, func_args_new, 1, NULL, PROPERTIES, affinity, NULL);
    
//        ocrSetHint(memoryEdtGuid, &app_type_hint_2);

        ocrGuid_t depv_guid;
        void* db;
        ocrDbCreate(&depv_guid, &db,
           sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
        ocrAddDependence(depv_guid, memoryEdtGuid, 0, DB_DEFAULT_MODE);

    }


    double sum[1];
    double* a;// = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);
    double* b;// = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);
    double* c;// = (double*)malloc(sizeof(double)*CONSTANT_LARGE_NUMBER);

    ocrGuid_t a_guid, b_guid, c_guid;	
    
    ocrDbCreate(&a_guid, (void**)&a, sizeof(double)*CONSTANT_LARGE_NUMBER,
			FLAGS, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&b_guid, (void**)&b, sizeof(double)*CONSTANT_LARGE_NUMBER,
                        FLAGS, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&c_guid, (void**)&c, sizeof(double)*CONSTANT_LARGE_NUMBER,
                        FLAGS, NULL_GUID, NO_ALLOC);
// init
    for(i=0;i<CONSTANT_LARGE_NUMBER;i++){
        b[i] = i;
    }

// copy
    for(i=0;i<CONSTANT_LARGE_NUMBER;i++){
            a[i] = b[i];
            sum[0]+=a[i]; // prevents compiler from optimizing and removing array copy
    }


    ocrDbDestroy(a_guid);
    ocrDbDestroy(b_guid);
    ocrDbDestroy(c_guid);

//Satisfy wrap-up task
    ocrGuid_t db_guid;
    void * temp;
    temp = &sum;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                            FLAGS, NULL_GUID, NO_ALLOC);
    
    ocrEventSatisfy(event_guid, db_guid);

    return NULL_GUID;
}

//Just setting priority might not be very helpful
//We can just do appTurn, since it's easier
//Or progress rate?
//What is the information that gets passed?
//App sends information on lack of progress rate?
//Runtime responds by re-adjusting?
//The app itself can't calculate the lack of progress.
//The app can provide information on every iteration or timestep finishing
//The runtime can store this information, use previous timestep/expected timestep value to estimate if resources need to be increased, in order to allow for faster cleaning
//
//
ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("\nStarted Analytics");
    u64 app_tag = 2;

/* 
    ocrHintInit(&app_type_hint_2, OCR_HINT_EDT_T);
    ocrSetHintValue(&app_type_hint_2, OCR_HINT_EDT_APP_TYPE, 1);
*/
    u64 func_args[2];
    func_args[0] = 0;

    ocrGuid_t affinity = NULL_GUID;
    ocrGuid_t templateWrap;
    ocrEdtTemplateCreate(&templateWrap, wrap_task_2, 0, THRESHOLD);

    ocrGuid_t wrapEdtGuid;
    ocrEdtCreate(&wrapEdtGuid, templateWrap, 0, NULL, THRESHOLD, NULL, PROPERTIES, affinity, NULL);
 

    func_args[1] = wrapEdtGuid;

    //ocrGuid_t event_guid[THRESHOLD];
    /*
    u64 i;
    for(i=0;i<THRESHOLD;i++){
        ocrEventCreate(&(event_guid[i]), OCR_EVENT_STICKY_T, TRUE);
        ocrAddDependence(event_guid[i], wrapEdtGuid, i, DB_MODE_ITW);
	func_args[i+1] = event_guid[i];
    }
*/
    ocrGuid_t templateMemSpawner;
    ocrEdtTemplateCreate(&templateMemSpawner, spawn_and_mem_task, 2, 1);
   
    ocrGuid_t memoryEdtGuid; 
    ocrEdtCreate(&memoryEdtGuid, templateMemSpawner, 2, func_args, 1, NULL, PROPERTIES, affinity, NULL);

//    ocrSetHint(memoryEdtGuid, &app_type_hint_2);

    ocrGuid_t depv_guid;
    void* db;
    ocrDbCreate(&depv_guid, &db,
       sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    ocrAddDependence(depv_guid, memoryEdtGuid, 0, DB_DEFAULT_MODE);
}
