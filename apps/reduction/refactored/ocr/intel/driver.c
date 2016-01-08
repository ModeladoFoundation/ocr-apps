
/*
Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
driver for reduction "library"

the source code and include file of the library are in xstack/apps/libs/src/reduction1

To use the library requires two lines in the makefile.*:

REQ_LIBS := reduction
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
    reductionOperator: choose which reduction to do (choices in reduction1.h)
    rangeGUID: a set of at least nrank-1 labeled STICKY event GUIDs
    returnEVT: a ONCE event (presumably labeled) that rank 0 will use
       to return the reduced data to the clones
4. make the clone depend on the returnEVT


This code uses macros currently defined in reduction1.h but they may get moved.


There are 3 parameters defined below:

P is the number of tasks ("numranks")
T is the number of iterations
N is the length of the vectors

*/

#include "reduction1.h"
//#include "macros.h"

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "string.h"
#include "stdio.h"

#define DEFAULTnrank 25//number of tasks
#define DEFAULTndata 2  //length of vector
#define DEFAULTmaxtimestep 300  //number of iterations

typedef struct {
    u64 myrank;
    u64 ndata;
    u64 timestep;
    u64 maxtimestep;
    u64 toggle;
    ocrGuid_t returnEVT[2];
    ocrGuid_t driverTML;
} driverPRM_t;

typedef struct {
    ocrEdtDep_t reductionPrivate;
    ocrEdtDep_t myData;
    ocrEdtDep_t returnData;
} driverDEPV_t;

ocrGuid_t driverEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/* initializes the data on t=0

runs T iterations cloning and launching reduction (using F8_ADD  e.g. global sum)

*/

    driverPRM_t * driverPRM = (driverPRM_t *) paramv;

    u64 myrank = PRM(driver,myrank);
    u64 ndata = PRM(driver,ndata);
    u64 timestep = PRM(driver,timestep);
    u64 maxtimestep = PRM(driver,maxtimestep);
    ocrGuid_t driverTML = PRM(driver,driverTML);
//printf("D%d ndata %d timestep %d maxtimestep %d \n", myrank, ndata, timestep, maxtimestep);

    driverDEPV_t * driverDEPV = (driverDEPV_t *) depv;
    reductionPrivate_t * reductionPrivatePTR = DEPV(driver,reductionPrivate,ptr);
    double * a = DEPV(driver,myData,ptr);
    double * b = DEPV(driver,returnData,ptr); //will be NULL for T=0

    u64 i;

    if(timestep == 0) {
//initialize
        for(i=0;i<ndata;i++) {
            a[i] = i*myrank;
        }
     }else{
//copy
        if(myrank == 0) {
            for(i=0;i<ndata;i++) PRINTF("C%d T%d i%d %f \n", myrank, timestep, i, b[i]);
        }
        for(i=0;i<ndata;i++) {
            a[i] = i*myrank+timestep;
        }
    }

    if(timestep >= maxtimestep) return NULL_GUID;

    PRM(driver,timestep)++;

    ocrGuid_t driverGUID;


//clone
    reductionPrivatePTR->returnEVT = PRM(driver,returnEVT[PRM(driver,toggle)]);
    PRM(driver,toggle) ^= 1;
    ocrEdtCreate(&driverGUID, driverTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


    ADDDEP(driver,reductionPrivate,RW);
    ocrDbRelease(DEPV(driver,myData,guid));
    ADDDEP(driver,myData,RW);

    u32 errno = ocrEventCreate(&(reductionPrivatePTR->returnEVT), OCR_EVENT_ONCE_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);


    ocrAddDependence(reductionPrivatePTR->returnEVT, driverGUID, SLOT(driver,returnData), DB_MODE_RO);

//create and launch reduceEdt

    reductionLaunch(DEPV(driver,reductionPrivate,ptr), DEPV(driver,reductionPrivate,guid), DEPV(driver,myData,guid));

//PRINTF("C%d T%d return NULL_GUID \n", myrank, timestep);
    return NULL_GUID;
}

typedef struct {
    u64 myrank;
    u64 ndata;
    u64 maxtimestep;
    ocrGuid_t onceRangeGUID;
} initRealPRM_t;

typedef struct {
    ocrEdtDep_t reductionShared;
    ocrEdtDep_t reductionPrivate;
} initRealDEPV_t;

ocrGuid_t initRealEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = myrank;

depv[0] = shared;
depv[1] = private;

initialize the private block from the shared block
launch driverEDT

*/


    initRealPRM_t * initRealPRM = (initRealPRM_t *) paramv;
    u64 myrank = PRM(initReal,myrank);
    u64 ndata = PRM(initReal,ndata);
    u64 maxtimestep = PRM(initReal,maxtimestep);
//printf("IR%d ndata %d \n", myrank, ndata);
//fflush(stdout);
    ocrGuid_t onceRangeGUID = PRM(initReal,onceRangeGUID);

    initRealDEPV_t * initRealDEPV = (initRealDEPV_t *) depv;
    reductionPrivate_t * reductionSharedPTR = DEPV(initReal,reductionShared,ptr);
    reductionPrivate_t * reductionPrivatePTR = DEPV(initReal,reductionPrivate,ptr);

    reductionPrivatePTR->nrank = reductionSharedPTR->nrank;
    reductionPrivatePTR->myrank = myrank;
    reductionPrivatePTR->ndata = reductionSharedPTR->ndata;
    reductionPrivatePTR->reductionOperator = REDUCTION_F8_ADD;
    reductionPrivatePTR->rangeGUID = reductionSharedPTR->rangeGUID;
    reductionPrivatePTR->reductionTML = NULL_GUID;
    reductionPrivatePTR->new = 1;
//extra can be used to pass extra data from rank 0 to all of the ranks
    reductionPrivatePTR->extra = 0;

    ocrGuid_t driverTML, driverGUID, myDBK;


    u64 dummy;


    ocrEdtTemplateCreate(&driverTML, driverEDT, PRMNUM(driver), 3);

    driverPRM_t paramvout;
    driverPRM_t * driverPRM = (driverPRM_t *) &paramvout;

    PRM(driver,myrank) = myrank;
    PRM(driver,ndata) = ndata;
    PRM(driver,timestep) = 0;
    PRM(driver,maxtimestep) = maxtimestep;
    PRM(driver,toggle) = 0;
    ocrGuidFromIndex(&(PRM(driver,returnEVT[0])),onceRangeGUID,0);
    ocrGuidFromIndex(&(PRM(driver,returnEVT[1])),onceRangeGUID,1);
    paramvout.driverTML = driverTML;

//PRINTF("M%ld PBGUID %lx \n", myrank, DEPV(initReal,reductionPrivate, guid));

    ocrEdtCreate(&driverGUID, driverTML, EDT_PARAM_DEF, (u64 *) &paramvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrDbRelease(DEPV(initReal,reductionPrivate,guid));
    ocrAddDependence(DEPV(initReal,reductionPrivate,guid), driverGUID, SLOT(driver,reductionPrivate), DB_MODE_RW);
    ocrDbCreate(&myDBK, (void**) &dummy, ndata*sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(myDBK, driverGUID, 1, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, driverGUID, 2, DB_MODE_RW);
//third dependency is the returned result

    return NULL_GUID;
}

typedef struct {
    u64 myrank;
    u64 ndata;
    u64 maxtimestep;
    ocrGuid_t onceRangeGUID;
} initPRM_t;

ocrGuid_t initEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0] = myrank

depv[0] = reductionSharedBlock

create private block
launch initReal
*/

initPRM_t * initPRM = (initPRM_t *) paramv;

//printf("I%d ndata %d maxtimestep %d \n", PRM(init,myrank), PRM(init,ndata), PRM(init,maxtimestep));


    u64 dummy;

    ocrGuid_t initRealTML, reductionPrivateDBK, initRealGUID;

    ocrDbCreate(&reductionPrivateDBK, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_GUID, NO_ALLOC);

    ocrEdtTemplateCreate(&initRealTML, initRealEDT, PRMNUM(initReal), DEPNUM(initReal));
    ocrEdtCreate(&initRealGUID, initRealTML, EDT_PARAM_DEF, (u64 *) paramv, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[0].guid, initRealGUID, 0, DB_MODE_RO);
    ocrAddDependence(reductionPrivateDBK, initRealGUID, 1, DB_MODE_RW);
    return NULL_GUID;
}

typedef struct {
    ocrEdtDep_t reductionShared;
} realMainDEPV_t;

typedef struct {
    u64 nrank;
    u64 ndata;
    u64 maxtimestep;
    } realMainPRM_t;


ocrGuid_t realMainEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
    realMainDEPV_t * realMainDEPV = (realMainDEPV_t *) depv;

    reductionPrivate_t * sharedPTR = DEPV(realMain,reductionShared,ptr);
/*
initialize shared block
launch parallel init with myrank as a parameter
*/
    realMainPRM_t * realMainPRM = (realMainPRM_t *) paramv;
    u64 nrank = PRM(realMain,nrank);
    u64 ndata = PRM(realMain,ndata);

    sharedPTR->nrank = nrank;
    sharedPTR->ndata = ndata;
    ocrGuid_t onceRangeGUID;
    if(sharedPTR->nrank > 1) ocrGuidRangeCreate(&(sharedPTR->rangeGUID), nrank-1, GUID_USER_EVENT_STICKY);
      else sharedPTR->rangeGUID = NULL_GUID;
    ocrGuidRangeCreate(&onceRangeGUID, 2, GUID_USER_EVENT_ONCE);

//PRINTF("returnEVT %lx \n", sharedPTR->returnEVT);
    u64 i;
    ocrGuid_t init, initTemplate;

    initPRM_t paramvout;
    initPRM_t * initPRM = &paramvout;
    PRM(init,onceRangeGUID) = onceRangeGUID;
    PRM(init,ndata) = PRM(realMain,ndata);
    PRM(init,maxtimestep) = PRM(realMain,maxtimestep);

    ocrEdtTemplateCreate(&(initTemplate), initEDT, sizeof(initPRM_t)/sizeof(u64), 1);

    for(i=0;i<nrank;i++) {
        PRM(init,myrank)= i;
        ocrEdtCreate(&init, initTemplate, EDT_PARAM_DEF, (u64 *) &paramvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, init, 0, DB_MODE_RW);
    }
    return NULL_GUID;
}

ocrGuid_t wrapupEDT(){
    PRINTF("shutting down\n");
    ocrShutdown();
}

void bomb(char * s){
    PRINTF("%s \n", s);
    ocrShutdown();
}

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
mainEdt is executed first
creates realmain as a FINISH EDT
creates wrapup to depend on realmain
creates shared block
launches realmain
*/
     realMainPRM_t paramvout;
     realMainPRM_t * realMainPRM = &paramvout;

     u64 i;
//printf("main\n");
     u32 _paramc, _depc, _idep;

     void * programArgv = depv[0].ptr;
     u32 argc = getArgc(programArgv);

     PRM(realMain,nrank) = DEFAULTnrank;
     PRM(realMain,ndata) = DEFAULTndata;
     PRM(realMain,maxtimestep) = DEFAULTmaxtimestep;

     printf("argc %d \n", argc);
     if(argc != 1 && argc !=4) bomb("number of run time parameters must be 0 or number of workers, number of data elements, number of timesteps");

     if(argc == 4) {
         u32 k = 1;
         PRM(realMain,nrank) = (u64) atoi(getArgv(programArgv, k++));
         PRM(realMain,ndata) = (u64) atoi(getArgv(programArgv, k++));
         PRM(realMain,maxtimestep) = (u64) atoi(getArgv(programArgv, k++));
         if(PRM(realMain,nrank) == 0) bomb("number of workers must be positive");
         if(PRM(realMain,ndata) == 0) bomb("number of entries must be positive");
         if(PRM(realMain,maxtimestep) == 0) bomb("number of timesteps must be positive");
         }

    ocrGuid_t realMainGUID, realMainTML, outputEVT, wrapup, wrapupTML, reductionSharedDBK;
    PRINTF("reduction driver \n");
    PRINTF("Number of workers is %d \n", PRM(realMain,nrank));
    PRINTF("data per worker %d \n", PRM(realMain,ndata));
    PRINTF("Number of timesteps is %d \n", PRM(realMain,maxtimestep));
    ocrEdtTemplateCreate(&wrapupTML, wrapupEDT, 0, 1);
    ocrEdtTemplateCreate(&realMainTML, realMainEDT, PRMNUM(realMain), 1);
    ocrEdtCreate(&realMainGUID, realMainTML, EDT_PARAM_DEF, (u64 *) realMainPRM, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &outputEVT);

    ocrEdtCreate(&wrapup, wrapupTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(outputEVT, wrapup, 0, DB_MODE_RW);

    u64 dummy;

    ocrDbCreate(&reductionSharedDBK, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(reductionSharedDBK, realMainGUID, 0, DB_MODE_RW);

    return NULL_GUID;
}
