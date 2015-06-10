
/*
Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
driver for reduction code

This driver does the following:

Creates a reductEdt template with 8 parameters and an unknown number of dependencies
Creates P+1 datablocks of length N (currently doubles but could be changed)
Has reductionFunction which will be called by reduceEdt
Reserves P-1 STICKY GUIDs
Reserves 2 ONCE GUIDs

Creates and launches P driverEdts.

Each Driver EDT:

Initializes its datablock
Creates a ONCE event from a ONCE GUID (alternating between the two)
Creates a clone
Attaches the the ONCE event as a dependency
Attaches its private block

Creates a reduceEdt with 8 parameters and 1 (or 2 for node zero) dependencies
Launches the reduce EDT by attaching its datablock (and node 0 attaches the special datablock)

The driver copies the result of the reduction to its private block before the next iteration.

There are 3 parameters defined below:

P is the number of tasks ("numnodes")
T is the number of iterations
N is the length of the vectors

*/

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "string.h"
#include "stdio.h"

#define P 11  //number of tasks
#define T 4  //number of iterations
#define N 5  //length of vector

#define size 8  //double


//prototype for reduceEdt
ocrGuid_t reduceEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

reductionFunction(u64 n, void * a, void * b) {
    u64 i;

//printf("in function with %lx %lx \n", a, b);

    if(a==NULL || b == NULL) {
        PRINTF("ERROR in reduction, function called with NULL pointer\n");
        return;
    }


    for(i=0;i<n;i++) ((double *)a)[i] += ((double *) b)[i] ;
    return;
}


ocrGuid_t driverTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params
0: mynode
1: timestep
2: myTemplate
3: reduceTemplate
4: stickyRangeGuid
5: onceEvent

depv
0: myblock
1: special block
*/
    u64 mynode = paramv[0];
    u64 timestep = paramv[1];
    ocrGuid_t myTemplate = (ocrGuid_t) paramv[2];
    ocrGuid_t reduceTemplate = (ocrGuid_t) paramv[3];
    ocrGuid_t stickyRangeGuid = (ocrGuid_t) paramv[4];
    ocrGuid_t onceRangeGuid = (ocrGuid_t) paramv[5];

//printf("C%d T%d start with myT %lx rT %lx stickyRangeGuid %lx onceRangeGuid %lx depv0 %lx depv1 %lx\n", mynode, timestep, myTemplate, reduceTemplate, stickyRangeGuid, onceRangeGuid, depv[0].guid, depv[1].guid);


    double * a = depv[0].ptr;
    double * b = depv[1].ptr;

    u64 i;
    ocrGuid_t reduceEdt, myEdt, onceEvent, eventTemp;

    if(timestep == 0) {
//initialize
        for(i=0;i<N;i++) a[i] = mynode*N + i;
//printf("C%d T%d initialize\n", mynode, timestep);
     }else{
//copy
        for(i=0;i<N;i++) a[i] = b[i];
        if(mynode == 0) {
            for(i=0;i<N;i++) PRINTF("C%d T%d i%d %f \n", mynode, timestep, i, a[i]);
        }
    }

    if(timestep >= T) return NULL_GUID;

    paramv[1]++; //timestep
//clone
    ocrEdtCreate(&myEdt, myTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrDbRelease(depv[0].guid);
    ocrAddDependence(depv[0].guid, myEdt, 0, DB_MODE_RW);

   ocrGuidFromIndex(&onceEvent, onceRangeGuid, timestep%2);

//printf("C%d T%d err %d\n", mynode, timestep, err);
    eventTemp = onceEvent;  //work around for bug that failing EventCreate zeroes out the GUID

    u64 err =  ocrEventCreate(&eventTemp, OCR_EVENT_ONCE_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);

//helps put time between creation and use which mitigates bug 627
    printf("C%d T%d create once event %lx errno %d \n", mynode, timestep, onceEvent, err);
    fflush(stdout);

//printf("C%d T%d depend on %lx\n", mynode, timestep, onceEvent);
    ocrAddDependence(onceEvent, myEdt, 1, DB_MODE_RO);

//create and launch reduceEdt
    u64 paramvReduce[8] = {mynode, P, reduceTemplate, N, sizeof(double), stickyRangeGuid, onceEvent, 0};
    u64 ndep = 1;
    if(mynode == 0) ndep = 2;
//printf("C%d T%d launch M with ndep %d\n", mynode, timestep, ndep);
    ocrEdtCreate(&reduceEdt, reduceTemplate, EDT_PARAM_DEF, paramvReduce, ndep, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[0].guid, reduceEdt, 0, DB_MODE_RW);
//printf("C%d T%d after add depv0 %lx\n", mynode, timestep, depv[0].guid);
    if(mynode == 0) ocrAddDependence(depv[1].guid, reduceEdt, 1, DB_MODE_RW);


//PRINTF("C%d T%d return NULL_GUID \n", mynode, timestep);
    return NULL_GUID;
}
ocrGuid_t realMainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0]: number of workers for GSi to create
depv
0: userSharedBlock
1: GSsharedBlock
realMain launches the GS initialization
*/
    u64 i, mynode, dummy;
    ocrGuid_t blocks[P+1], edt, stickyRangeGuid, onceRangeGuid, onceEvent, myTemplate, reduceTemplate;
//printf("realmain\n");

    for(i=0;i<P+1;i++) {
        ocrDbCreate(&(blocks[i]), (void**) &dummy, N*sizeof(double), 0, NULL_GUID, NO_ALLOC);
    }

    ocrEdtTemplateCreate(&(myTemplate), driverTask, 6, 2);
    ocrEdtTemplateCreate(&(reduceTemplate), reduceEdt, 8, EDT_PARAM_UNK);

//printf("before create range \n");
    ocrGuidRangeCreate(&stickyRangeGuid,(u64) P-1, GUID_USER_EVENT_STICKY);
//printf("after create range \n");

    ocrGuidRangeCreate(&onceRangeGuid,(u64) 2, GUID_USER_EVENT_ONCE);
    ocrGuidFromIndex(&onceEvent, onceRangeGuid, 0);
//printf("RM once event guid %llx \n", onceEvent);


    u64 paramvout[6] = {0, 0, (u64) myTemplate, (u64) reduceTemplate, (u64) stickyRangeGuid, onceRangeGuid};
    for(i=0;i<P;i++) {
//printf("RM P %d i %d \n", P, i);
        paramvout[0] = i;
        ocrEdtCreate(&edt, myTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(blocks[i], edt, 0, DB_MODE_RW);
        ocrAddDependence(blocks[P], edt, 1, DB_MODE_RO);
    }
    return NULL_GUID;
}

ocrGuid_t wrapupEdt(){
    PRINTF("shutting down\n");
    ocrShutdown();
    }

ocrGuid_t mainEdt(){
/*
mainEdt is executed first
Creates the datablocks
creates realmain as a FINISH EDT
creates wrap up to depend on realmain
launches realmain (wrapup waits until realmain is done)
*/
    u64 i;
    u64 *dummy;
//printf("main\n");
    ocrGuid_t realMain, GSsharedBlock, userSharedBlock, realMainTemplate, output, wrapup, wrapupTemplate;
    PRINTF("reduction driver \n");
    PRINTF("Number of timesteps is %d \n", T);
    PRINTF("Number of workers is %d \n", P);
    PRINTF("data per worker %d \n", N);
    ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, 1);
    ocrEdtTemplateCreate(&realMainTemplate, realMainEdt, 0, 0);
    ocrEdtCreate(&realMain, realMainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &output);
    ocrEdtCreate(&wrapup, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL_GUID);
    ocrAddDependence(output, wrapup, 0, DB_MODE_RW);
    return NULL_GUID;
}
