
/*
Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
driver for reduction "library"

This driver shows how to do global reductions in an OCR code by making ordinary C calls in EDTs to do the OCR
related operations.

There are 2 calls that are done once

ocrGuid_t reductionCreateShared();
void reductionSerialInit(u64 numnodes, u64 length, reductionOperator_t operator, reductionShared_t * shared, ocrGuid_t reductionSharedGuid);

There are 2 calls that are done once for each "rank" (EDTs numbered from 0 to numnodes-1)

ocrGuid_t reductionCreatePrivate();
void reductionParallelInit(u64 mynode, reductionShared_t * shared, reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);

There are two calls done each time a reduction needs to be done

ocrGuid_t reductionGetOnceEvent(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);
void reductionLaunch(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid, ocrGuid_t mydataGuid);

These are documented in more detail in README

The driver copies a modification of the result of the reduction to its private block before the next iteration.

There are 3 parameters defined below:

P is the number of tasks ("numnodes")
T is the number of iterations
N is the length of the vectors

*/

#include "reduction.h"

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "string.h"
#include "stdio.h"

#define P 42  //number of tasks
#define T 100  //number of iterations
#define N 6  //length of vector


ocrGuid_t driverEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params
0: mynode
1: timestep
2: myTemplate (for cloning)

depv
0: private block
1: myblock
2: returned block

initializes the data on T=0

runs T iterations cloning and launching reduction (using F8_ADD  e.g. global sum)

*/
    u64 mynode = paramv[0];
    u64 timestep = paramv[1];
    ocrGuid_t myTemplate = (ocrGuid_t) paramv[2];

    double * a = depv[1].ptr;
    double * b = depv[2].ptr;  //will be NULL for T=0

    u64 i;

    if(timestep == 0) {
//initialize
        for(i=0;i<N;i++) a[i] = mynode*N + i+1;
printf("C%d T%d initialize a %f \n", mynode, timestep, *((double *) depv[1].ptr));
     }else{
//copy
        if(mynode == 0) {
            for(i=0;i<N;i++) PRINTF("C%d T%d i%d %f \n", mynode, timestep, i, b[i]);
        }
        for(i=0;i<N;i++) a[i] = b[i]/(P+2) + i*mynode;
    }

    if(timestep >= T) return NULL_GUID;

    paramv[1]++; //timestep

    ocrGuid_t myEdt;
    ocrGuid_t onceEvent= reductionGetOnceEvent(depv[0].ptr, depv[0].guid);


//clone
    ocrEdtCreate(&myEdt, myTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


    ocrAddDependence(depv[0].guid, myEdt, 0, DB_MODE_RW);
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, myEdt, 1, DB_MODE_RW);
//printf("C%d T%d err %d\n", mynode, timestep, err);

//printf("C%d T%d depend on %lx\n", mynode, timestep, onceEvent);
    ocrAddDependence(onceEvent, myEdt, 2, DB_MODE_RO);

//create and launch reduceEdt
    reductionLaunch(depv[0].ptr, depv[0].guid, depv[1].guid);

//PRINTF("C%d T%d return NULL_GUID \n", mynode, timestep);
    return NULL_GUID;
}

ocrGuid_t initRealEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = mynode;

depv[0] = shared;
depv[1] = private;

initialize the private block from the shared block
launch driverEdt

*/

    u64 mynode = paramv[0];
    reductionShared_t * shared = depv[0].ptr;
    reductionPrivate_t * private = depv[1].ptr;

    reductionParallelInit(mynode, shared, private, depv[1].guid);

    ocrGuid_t driverTemplate, driver, myblock;
    u64 timestep = 0;
    u64 dummy;


    ocrEdtTemplateCreate(&driverTemplate, driverEdt, 3, 3);

    u64 paramvout[3] = {mynode, timestep, driverTemplate};

    ocrEdtCreate(&driver, driverTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

    ocrAddDependence(depv[1].guid, driver, 0, DB_MODE_RW);
//RW needed because the call to reductionGetOnceEvent changes the phase

    ocrDbCreate(&myblock, (void**) &dummy, N*sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(myblock, driver, 1, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, driver, 2, DB_MODE_RW);
//third dependency is the returned result

    return NULL_GUID;
}

ocrGuid_t initEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = mynode

depv[0] = reductionSharedBlock

create private block
launch initReal
*/

    u64 mynode = paramv[0];
    reductionShared_t * shared = depv[0].ptr;

    ocrGuid_t initRealTemplate, private, initReal;

    private = reductionCreatePrivate();

    ocrEdtTemplateCreate(&initRealTemplate, initRealEdt, 1, 2);
    ocrEdtCreate(&initReal, initRealTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[0].guid, initReal, 0, DB_MODE_RO);
    ocrAddDependence(private, initReal, 1, DB_MODE_RW);
    return NULL_GUID;
}


ocrGuid_t realMainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
depv
0: reductionSharedBlock

initialize shared block
launch parallel init with mynode as a parameter

*/
    u64 i;
    ocrGuid_t init, initTemplate;


    reductionSerialInit(P, N, REDUCTION_F8_ADD, depv[0].ptr, depv[0].guid);

    ocrEdtTemplateCreate(&(initTemplate), initEdt, 1, 1);

    for(i=0;i<P;i++) {
        ocrEdtCreate(&init, initTemplate, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, init, 0, DB_MODE_RW);
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
creates realmain as a FINISH EDT
creates wrapup to depend on realmain
creates shared block
launches realmain
*/
    u64 i;
//printf("main\n");
    ocrGuid_t realMain, realMainTemplate, output, wrapup, wrapupTemplate, shared;
    PRINTF("reduction driver \n");
    PRINTF("Number of timesteps is %d \n", T);
    PRINTF("Number of workers is %d \n", P);
    PRINTF("data per worker %d \n", N);
    ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, 1);
    ocrEdtTemplateCreate(&realMainTemplate, realMainEdt, 0, 1);
    ocrEdtCreate(&realMain, realMainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &output);

    ocrEdtCreate(&wrapup, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(output, wrapup, 0, DB_MODE_RW);

    shared = reductionCreateShared();
    ocrAddDependence(shared, realMain, 0, DB_MODE_RW);

    return NULL_GUID;
}
