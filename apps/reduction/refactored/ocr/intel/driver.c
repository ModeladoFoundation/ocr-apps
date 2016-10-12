/*
Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
driver for the reduction "library"
it uses the macros.h file from the macros "library"

the source code and include file of the reduction library are in xstack/apps/libs/src/reduction

To use the libraries requires two lines in the makefile.*:

REQ_LIBS := reduction macros
LD_FLAGS := lreduction

This driver shows one way to do global reductions in an OCR code

The reduction is done by the participants calling a C function
reductionLaunch(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateGUID, ocrGuid_t mydataGUID)

However before calling reductionLaunch the user must

1. create a reductionPrivate datablock for each participating rank
2. create a clone to receive the result of the reduction
3. initialize some of the values in the reductionPrivate datablock
    nrank: number of participating ranks
    myrank: my number
    ndata: number of elements to be reduced
    new: set to TRUE for the first call
    all: set to TRUE if everyone is getting the result back.  if false only rank 0s event is satisfied
    reductionOperator: choose which reduction to do (choices in reduction1.h)
    rangeGUID: a set of at least nrank-1 labeled STICKY event GUIDs
    returnEVT: a channel event, different for each rank used to return the result
4. make the clone depend on the returnEVT



There are 3 parameters with default values defined below:

P is the number of tasks ("numranks")
N is the length of the vectors
T is the number of iterations

These values can be entered as RUNTIME_ARGS

As written this currently tests only the ALLREDUCE version of the reduction library

*/

// Externally defined SHOW_RESULTS
#define SHOW_RESULTS

#include <string.h>
#include <stdio.h>

#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"

#include "reduction.h"
#include "macros.h"
#include "timer.h"

#define DEFAULTnrank 25         //number of tasks
#define DEFAULTndata 2          //length of vector
#define DEFAULTmaxtimestep 300  //number of iterations

typedef struct {
    u64 nrank;
    u64 ndata;
    u64 maxtimestep;
    ocrGuid_t wrapupEDT;
    ocrGuid_t reductionRangeGUID;
} shared_t;

typedef struct {
    u64 nrank;
    u64 myrank;
    u64 ndata;
    u64 timestep;
    u64 maxtimestep;
    ocrHint_t myAffinityHNT;
    ocrGuid_t driverTML;
    ocrGuid_t wrapupEDT; //needed only by rank 0
} private_t;

typedef struct {
    timestamp_t start;
    u64 nbInstances;
} wrapupPRM_t;

typedef struct {
    ocrEdtDep_t control;
    ocrEdtDep_t initTimer;
} wrapupDEPV_t;

ocrGuid_t wrapupEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
    DEPVDEF(wrapup);
    PRMDEF(wrapup);
    timestamp_t stop;
    get_time(&stop);
    double elapsed = elapsed_sec(&PRM(wrapup,start), &stop);
    double initElapsed = *((double *) DEPV(wrapup,initTimer,ptr));
    PRINTF("startup time=%f (s) elapsed time=%f (s)\n", initElapsed, elapsed);
    print_throughput("Reduction", PRM(wrapup,nbInstances), elapsed);
    ocrShutdown();
}


typedef struct {
} driverPRM_t;

typedef struct {
    ocrEdtDep_t private;
    ocrEdtDep_t reductionPrivate;
    ocrEdtDep_t myData;
    ocrEdtDep_t returnData;
} driverDEPV_t;

ocrGuid_t driverEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/* initializes the data on t=0

runs T iterations cloning and launching reduction (using F8_ADD  e.g. global sum)

*/

    DEPVDEF(driver);
    private_t * privatePTR = DEPV(driver,private,ptr);
    reductionPrivate_t * reductionPrivatePTR = DEPV(driver,reductionPrivate,ptr);
    double * a = DEPV(driver,myData,ptr);
    double * b = DEPV(driver,returnData,ptr); //will be NULL for T=0

    u64 nrank = privatePTR->nrank;
    u64 myrank = privatePTR->myrank;
    u64 ndata = privatePTR->ndata;
    u64 timestep = privatePTR->timestep;
    u64 maxtimestep = privatePTR->maxtimestep;

    u64 i;

    if(timestep == 0) {
//initialize
        for(i=0;i<ndata;i++) {
            a[i] = i*(myrank+1);
        }
     }else{
//copy

#ifdef SHOW_RESULTS
        if(myrank == 0 && timestep <= maxtimestep) {
            for(i=0;i<ndata;i++)
                PRINTF("C%d T%d i%d %f \n", myrank, timestep, i, b[i]);
        }
#endif
        for(i=0;i<ndata;i++) {
            a[i] = i*myrank+timestep;
        }
    }

    if(timestep >= maxtimestep) {
        if(myrank == 0) ocrAddDependence(NULL_GUID, privatePTR->wrapupEDT, SLOT(wrapup, control), DB_MODE_RO);

        return NULL_GUID;
    }

    privatePTR->timestep++;

    ocrGuid_t driverEDT;

//create clone
    ocrEdtCreate(&driverEDT, privatePTR->driverTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &(privatePTR->myAffinityHNT), NULL);

    ocrDbRelease(DEPV(driver,private,guid));
    ocrAddDependence(DEPV(driver,private,guid),driverEDT, SLOT(driver,private), DB_MODE_RW);
    ocrAddDependence(DEPV(driver,reductionPrivate,guid),driverEDT, SLOT(driver,reductionPrivate), DB_MODE_RW);
    if((reductionPrivatePTR->type == REDUCE && myrank !=0) || (reductionPrivatePTR->type == BROADCAST && myrank == 0))
       ocrAddDependence(DEPV(driver,myData,guid), driverEDT, SLOT(driver,returnData), DB_MODE_RO);
    else
       ocrAddDependence(reductionPrivatePTR->returnEVT, driverEDT, SLOT(driver,returnData), DB_MODE_RO);


    reductionLaunch(DEPV(driver,reductionPrivate,ptr), DEPV(driver,reductionPrivate,guid), a);


//finish clone
    ocrDbRelease(DEPV(driver,myData,guid));
if((reductionPrivatePTR->type == ALLREDUCE) || (myrank != 0 && reductionPrivatePTR->type == BROADCAST) || (myrank == 0 && reductionPrivatePTR->type == REDUCE))    ocrAddDependence(DEPV(driver,myData,guid),driverEDT, SLOT(driver,myData), DB_MODE_RW);

    return NULL_GUID;
}

typedef struct {
    u64 myrank;
} initRealPRM_t;

typedef struct {
    ocrEdtDep_t shared;
    ocrEdtDep_t private;
    ocrEdtDep_t reductionPrivate;
} initRealDEPV_t;

ocrGuid_t initRealEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = myrank;

depv[0] = shared;
depv[1] = private;
depv[2] reductionPrivate

initialize the private block from the shared block
launch driverEDT

*/

    PRMDEF(initReal);
    DEPVDEF(initReal);

    u64 myrank = PRM(initReal,myrank);

    shared_t * sharedPTR = DEPV(initReal,shared,ptr);
    private_t * privatePTR = DEPV(initReal,private,ptr);
    reductionPrivate_t * reductionPrivatePTR = DEPV(initReal,reductionPrivate,ptr);

//printf("IR%d ndata %d \n", myrank, sharedPTR->ndata);
//fflush(stdout);
    u64 dummy;

    reductionPrivatePTR->nrank = sharedPTR->nrank;
    reductionPrivatePTR->myrank = myrank;
    reductionPrivatePTR->reductionOperator = REDUCTION_F8_ADD;
    reductionPrivatePTR->rangeGUID = sharedPTR->reductionRangeGUID;
    reductionPrivatePTR->new = 1;
    reductionPrivatePTR->type = ALLREDUCE;
    reductionPrivatePTR->ndata = sharedPTR->ndata;
    if(reductionPrivatePTR->type == BROADCAST) reductionPrivatePTR->ndata = 8*sharedPTR->ndata;
//printf("IR%d ndata %d \n", myrank, reductionPrivatePTR->ndata);
//fflush(stdout);
    //ocrDbCreate(&(reductionPrivatePTR->downDBK), (void**) &dummy, reductionPrivatePTR->ndata*sizeof(double), 0, NULL_HINT, NO_ALLOC);

//printf("D %d downDBK "GUIDF" \n", myrank, reductionPrivatePTR->downDBK);

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    ocrEventCreateParams(&(reductionPrivatePTR->returnEVT), OCR_EVENT_CHANNEL_T, false, &params);


    ocrGuid_t driverTML, driverEDT, myDBK;




    ocrEdtTemplateCreate(&(privatePTR->driverTML), driverEdt, PRMNUM(driver), DEPVNUM(driver));

    privatePTR->nrank = sharedPTR->nrank;
    privatePTR->myrank = myrank;
    privatePTR->ndata = sharedPTR->ndata;
    privatePTR->timestep = 0;
    privatePTR->maxtimestep = sharedPTR->maxtimestep;
    privatePTR->wrapupEDT = sharedPTR->wrapupEDT;

    ocrHint_t myHNT;
    ocrHintInit(&myHNT,OCR_HINT_EDT_T);
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
    privatePTR->myAffinityHNT = myHNT;




//PRINTF("M%ld PBGUID %lx \n", myrank, DEPV(initReal,reductionPrivate, guid));
//

    ocrEdtCreate(&driverEDT, privatePTR->driverTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &(privatePTR->myAffinityHNT), NULL);
    ocrDbRelease(DEPV(initReal,private,guid));
    ocrAddDependence(DEPV(initReal,private,guid), driverEDT, SLOT(driver,private), DB_MODE_RW);
    ocrDbRelease(DEPV(initReal,reductionPrivate,guid));
    ocrAddDependence(DEPV(initReal,reductionPrivate,guid), driverEDT, SLOT(driver,reductionPrivate), DB_MODE_RW);
    ocrDbCreate(&myDBK, (void**) &dummy, privatePTR->ndata*sizeof(double), 0, NULL_HINT, NO_ALLOC);
    ocrAddDependence(myDBK, driverEDT, SLOT(driver,myData), DB_MODE_RW);
    ocrAddDependence(NULL_GUID, driverEDT, SLOT(driver,returnData), DB_MODE_RW);

    return NULL_GUID;
}

typedef struct {
    u64 myrank;
} initPRM_t;

typedef struct {
    ocrEdtDep_t shared;
} initDEPV_t;


ocrGuid_t initEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = myrank

depv[0] = shared

create private block
create reductionPrivate block
launch initReal
*/

    PRMDEF(init);
    DEPVDEF(init);

//printf("I%d ndata %d maxtimestep %d \n", PRM(init,myrank), PRM(init,ndata), PRM(init,maxtimestep));


    u64 myrank = PRM(init,myrank);
    u64 dummy;

    ocrGuid_t initRealTML, reductionPrivateDBK, privateDBK, initRealEDT;
    ocrDbCreate(&reductionPrivateDBK, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC);
    ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t), 0, NULL_HINT, NO_ALLOC);

    ocrEdtTemplateCreate(&initRealTML, initRealEdt, PRMNUM(initReal), DEPVNUM(initReal));

    ocrHint_t myHNT;
    ocrHintInit(&myHNT,OCR_HINT_EDT_T);
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));


    ocrEdtCreate(&initRealEDT, initRealTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
    ocrAddDependence(DEPV(init,shared,guid), initRealEDT, SLOT(initReal,shared), DB_MODE_RO);
    ocrAddDependence(reductionPrivateDBK, initRealEDT, SLOT(initReal,reductionPrivate), DB_MODE_RW);
    ocrAddDependence(privateDBK, initRealEDT, SLOT(initReal,private), DB_MODE_RW);
    return NULL_GUID;
}

typedef struct {
    ocrEdtDep_t shared;
} realMainDEPV_t;

typedef struct {
    u64 nrank;
    u64 ndata;
    u64 maxtimestep;
    } realMainPRM_t;


ocrGuid_t realMainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

    PRMDEF(realMain);
    DEPVDEF(realMain);

/*
initialize shared block
launch parallel init with myrank as a parameter
*/

    u64 nrank = PRM(realMain,nrank);
    u64 ndata = PRM(realMain,ndata);
    u64 maxtimestep = PRM(realMain,maxtimestep);

    shared_t * sharedPTR = DEPV(realMain,shared,ptr);
    ocrGuid_t sharedDBK = DEPV(realMain,shared,guid);

    sharedPTR->nrank = nrank;
    sharedPTR->ndata = ndata;
    sharedPTR->maxtimestep = maxtimestep;

    ocrGuidRangeCreate(&(sharedPTR->reductionRangeGUID), nrank, GUID_USER_EVENT_STICKY);
    timestamp_t initStart;
    get_time(&initStart);
    wrapupPRM_t wrapupPRM;
    wrapupPRM.start = initStart;
    wrapupPRM.nbInstances = (ndata * nrank * maxtimestep);

    ocrGuid_t affinity;
    ocrAffinityGetCurrent(&affinity);
    ocrHint_t myHNT;
    ocrHintInit(&myHNT,OCR_HINT_EDT_T);
    ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affinity));

    ocrGuid_t wrapupTML;
    ocrEdtTemplateCreate(&wrapupTML, wrapupEdt, PRMNUM(wrapup), DEPVNUM(wrapup));
    ocrGuid_t wrapupEdt;
    ocrEdtCreate(&wrapupEdt, wrapupTML, EDT_PARAM_DEF, (u64 *) &wrapupPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
    sharedPTR->wrapupEDT = wrapupEdt;
    ocrDbRelease(sharedDBK);
//PRINTF("returnEVT %lx \n", sharedPTR->returnEVT);
    u64 i;
    ocrGuid_t initEDT, initTML;

    initPRM_t initPRM;

    ocrEdtTemplateCreate(&(initTML), initEdt, PRMNUM(init), DEPVNUM(init));

    u64 count, myPD, block;
    ocrAffinityCount(AFFINITY_PD, &count);
    block = (nrank + count - 1)/count;

    ocrDbRelease(DEPV(realMain,shared,guid));
    for(i=0;i<nrank;i++) {
        myPD = i/block;
//printf("i %d count %d myPD %d slot %d \n", i, count, myPD, SLOT(init,shared));
        ocrAffinityGetAt(AFFINITY_PD, myPD, &(affinity));
        ocrHintInit(&myHNT,OCR_HINT_EDT_T);
        ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affinity));

        initPRM.myrank = i;
        ocrEdtCreate(&initEDT, initTML, EDT_PARAM_DEF, (u64 *) &initPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
        ocrAddDependence(DEPV(realMain,shared,guid), initEDT, SLOT(init,shared), DB_MODE_RO);
    }
    timestamp_t initStop;
    get_time(&initStop);

    double initElapsed = elapsed_sec(&initStart, &initStop);
    ocrGuid_t initElapsedDb;
    double * initElapsedPtr;
    ocrDbCreate(&initElapsedDb, (void **) &initElapsedPtr, sizeof(double), 0, NULL_HINT, NO_ALLOC);
    *initElapsedPtr = initElapsed;
    ocrDbRelease(initElapsedDb);
    ocrAddDependence(initElapsedDb, wrapupEdt, SLOT(wrapup, initTimer), DB_MODE_RO);
    return NULL_GUID;
}

void bomb(char * s){
    PRINTF("error: %s \n", s);
    ocrShutdown();
}

/*
 * mainEdt is executed first
 * creates shared block
 * launches realmain
 */
ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
     realMainPRM_t paramvout;
     realMainPRM_t * realMainPRM = &paramvout;

     u64 i;
     u32 _paramc, _depc, _idep;

     void * programArgv = depv[0].ptr;
     u32 argc = getArgc(programArgv);

     PRM(realMain,nrank) = DEFAULTnrank;
     PRM(realMain,ndata) = DEFAULTndata;
     PRM(realMain,maxtimestep) = DEFAULTmaxtimestep;

     if(argc != 1 && argc !=4)
            bomb("usage: [nbWorkers nbElements nbTimesteps]");

     if(argc == 4) {
         u32 k = 1;
         PRM(realMain,nrank) = (u64) atoi(getArgv(programArgv, k++));
         PRM(realMain,ndata) = (u64) atoi(getArgv(programArgv, k++));
         PRM(realMain,maxtimestep) = (u64) atoi(getArgv(programArgv, k++));
         if(PRM(realMain,nrank) == 0) bomb("number of workers must be positive");
         if(PRM(realMain,ndata) == 0) bomb("number of entries must be positive");
         if(PRM(realMain,maxtimestep) == 0) bomb("number of timesteps must be positive");
     }

    ocrGuid_t realMainEDT, realMainTML, wrapupEDT, wrapupTML, sharedDBK, reductionSharedDBK;

    PRINTF("reduction driver \n");
    PRINTF("Number of workers is %d \n", PRM(realMain,nrank));
    PRINTF("data per worker %d \n", PRM(realMain,ndata));
    PRINTF("Number of timesteps is %d \n", PRM(realMain,maxtimestep));

    ocrEdtTemplateCreate(&realMainTML, realMainEdt, PRMNUM(realMain), DEPVNUM(realMain));
    ocrEdtCreate(&realMainEDT, realMainTML, EDT_PARAM_DEF, (u64 *) realMainPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    u64 dummy;
    ocrDbCreate(&sharedDBK, (void**) &dummy, sizeof(shared_t), 0, NULL_HINT, NO_ALLOC);
    ocrDbRelease(sharedDBK);
    ocrAddDependence(sharedDBK, realMainEDT, SLOT(realMain,shared), DB_MODE_RW);

    return NULL_GUID;
}
