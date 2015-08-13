#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

#include <getopt.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

#define CONSTANT_LARGE_NUMBER 1000

#define THRESHOLD 100//180
#include "common.h"

ocrGuid_t templateComputeSpawner;
ocrGuid_t event_guid[THRESHOLD];


int iteration;

ocrGuid_t wrap_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  PRINTF("\nFinishing compute tasks");
    if(instances==0)
      instances++;
    else
      ocrShutdown();
}

ocrGuid_t spawn_and_compute_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    
    u64 i, j, k;


//Spawn more tasks before starting computation to generate enough parallelism
 
  
    u64 *func_args = paramv;
    u64 index = func_args[0];


    iteration++;

    u64 func_args_new[1];
    func_args_new[0] = index+1;
    ocrGuid_t affinity = NULL_GUID;

    ocrGuid_t computeEdtGuid;

    u64 tag = 1;

    if(index!=THRESHOLD-1)
        ocrEdtCreate/*Tag*/(&computeEdtGuid, templateComputeSpawner, 1, func_args_new, 0, NULL, PROPERTIES, affinity, NULL);//, tag);



//Perform computation on two array for n^3 loop iterations
    double sum[1];
    for(i=0;i<CONSTANT_LARGE_NUMBER;i++){
        for(j=i+1;j<CONSTANT_LARGE_NUMBER;j++){
            for(k=j+1;k<CONSTANT_LARGE_NUMBER;k++){
                double a[1];
                double b[1];
                b[0] = i*j;
                a[0] = b[0] + b[0] +k;
                sum[0]+=a[0];
            }
        }
    } 


//Send satisfy db for wrap-up task
    ocrGuid_t db_guid;
    void * temp;
    temp = &sum;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                            FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid[index], db_guid);

    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {

  PRINTF("\n Compute main Edt");
    u64 func_args[1];
    func_args[0] = 0;

    ocrGuid_t affinity = NULL_GUID;
    ocrGuid_t templateWrap;
    ocrEdtTemplateCreate(&templateWrap, wrap_task, 0, THRESHOLD);

    ocrGuid_t wrapEdtGuid;
    u64 tag = 1;
    ocrEdtCreate/*Tag*/(&wrapEdtGuid, templateWrap, 0, NULL, THRESHOLD, NULL, PROPERTIES, affinity, NULL);//, tag);
    
    u64 i;
    for(i=0;i<THRESHOLD;i++){
        ocrEventCreate(&(event_guid[i]), OCR_EVENT_STICKY_T, TRUE);
        ocrAddDependence(event_guid[i], wrapEdtGuid, i, DB_MODE_ITW);
    }

    ocrEdtTemplateCreate(&templateComputeSpawner, spawn_and_compute_task, 1, 0);
   
    ocrGuid_t computeEdtGuid; 
    ocrEdtCreate/*Tag*/(&computeEdtGuid, templateComputeSpawner, 1, func_args, 0, NULL, PROPERTIES, affinity, NULL);//, tag);
    return NULL_GUID;
}
