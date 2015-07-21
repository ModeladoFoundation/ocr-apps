/*
Author: David Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
Driver code for global sum library.  See README for more information

This is the Shim version which has the global sum of r*r wake up a single ShimEDT that copies
   rtr to rtrold
*/
#include <ocr.h>
#include <stdio.h>
#include <math.h>
#include "ocrGS.h"

typedef struct {
    double x[M];
    double r[M];
    double b[M];
    double p[M];
    double ap[M];
    double a[M];
    } cgBlock_t;
typedef struct{
    ocrGuid_t cgTemplate;
    ocrGuid_t rtrShimTemplate;
    } userSharedBlock_t;
ocrGuid_t cgTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params
0: mynode
1: timestep
2: phase
depv
0: userblock
1: GSsharedBlock
2: my GSblock
3: cgBlock (matrix and vectors)
*/
    u64 mynode = paramv[0];
    u64 timestep = paramv[1];
    u64 phase = paramv[2];
    userSharedBlock_t * uSB = depv[0].ptr;
    GSsharedBlock_t * SB = depv[1].ptr;
    gsBlock_t * mydata = depv[2].ptr;
    cgBlock_t * cgdata = depv[3].ptr;
    ocrGuid_t cgEdt, gsEdt, shimEdt;
    double sum, pap, rtr, alpha, beta;
    u64 i;
    switch (phase) {
        case 0:
/*
 Initial call only
 Initialize matrix and vectors
 Prepare for the first summation of rtr
 Create next version of cgTask with phase = 1
 Launch global sum
 */
            sum = M*mynode+1;
            for(i=0;i<M;i++){
                cgdata->a[i] = sum;
                cgdata->b[i] = sum;
                cgdata->r[i] = sum;
                cgdata->p[i] = sum++;
                cgdata->x[i] = 0.0;
            }
            sum = 0;
            for(i=0;i<M;i++) sum += cgdata->r[i]*cgdata->r[i];
            mydata->data = sum;
            paramv[2] = 1;
//Create Shim
            if(mynode == 0) {
                ocrEdtCreate(&shimEdt, uSB->rtrShimTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
                ocrAddDependence(SB->rootEvent, shimEdt, 0, DB_MODE_RW);
            }
//create next cgTask
            ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[0].guid);
            ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
            ocrAddDependence(SB->shimEvent, cgEdt, 1, DB_MODE_RW);
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
            ocrDbRelease(depv[3].guid);
            ocrAddDependence(depv[3].guid, cgEdt, 3, DB_MODE_RW);
//launch global sum
            ocrEdtCreate(&gsEdt, SB->GSxTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[1].guid);
            ocrAddDependence(depv[1].guid, gsEdt, 0, DB_MODE_RW);
            ocrAddDependence(depv[2].guid, gsEdt, 1, DB_MODE_RW);
            for(i=0;i<ARITY;i++) ocrAddDependence(NULL_GUID, gsEdt, i+2, DB_MODE_RW);
            return NULL_GUID;
        case 1:
// Consume rtr
            if(mynode == 0) PRINTF("time %d rtr %f \n", timestep, SB->sum);
            if(SB->sum/SB->rtr0 < 1e-13 || timestep == T) {
                for(i=0;i<M;i++) PRINTF("CG%d T%d  %d value %f \n", mynode, timestep, i, cgdata->x[i]);
                if(mynode == 0 && M==300 && N==20 && T==100) {
                  if(fabs(cgdata->x[0] - 0.462231) < 1e-5) PRINTF("PASS\n"); else PRINTF("FAIL difference %f is too large\n", cgdata->x[0] - .462231); }
                return NULL_GUID;
            }
            if(timestep == 0) {
                if(mynode == 0) SB->rtr0 = SB->sum;
                beta = 0;
              } else beta = SB->sum/SB->rtrold;
//Update p and compute Ap
            for(i=0;i<M;i++) {
                cgdata->p[i] = cgdata->r[i] + beta*cgdata->p[i];
                cgdata->ap[i] = cgdata->a[i]*cgdata->p[i];
            }
//Compute local inner product
            sum = 0;
            for(i=0;i<M;i++) sum += cgdata->p[i]*cgdata->ap[i];
            mydata->data = sum;
            paramv[2]=2;
//Clone compute
            ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[0].guid);
            ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
            ocrAddDependence(SB->rootEvent, cgEdt, 1, DB_MODE_RW);
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
            ocrDbRelease(depv[3].guid);
            ocrAddDependence(depv[3].guid, cgEdt, 3, DB_MODE_RW);
//launch global sum
            ocrEdtCreate(&gsEdt, SB->GSxTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[1].guid);
            ocrAddDependence(depv[1].guid, gsEdt, 0, DB_MODE_RW);
            ocrAddDependence(depv[2].guid, gsEdt, 1, DB_MODE_RW);
            for(i=0;i<ARITY;i++) ocrAddDependence(NULL_GUID, gsEdt, i+2, DB_MODE_RW);
            return NULL_GUID;
        case 2:
//Consume pAp
            pap = SB->sum;
            alpha = SB->rtr/pap;
            if(mynode == 0) SB->rtrold = SB->rtr;
            for(i=0;i<M;i++) {
                cgdata->x[i] += alpha*cgdata->p[i];
                cgdata->r[i] -= alpha*cgdata->ap[i];
            }
//create shimEdt
            if(mynode == 0) {
                ocrEdtCreate(&shimEdt, uSB->rtrShimTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
                ocrAddDependence(SB->rootEvent, shimEdt, 0, DB_MODE_RW);
                }
//compute local inner product
            sum = 0;
            for(i=0;i<M;i++) sum += cgdata->r[i]*cgdata->r[i];
            mydata->data = sum;
//create and partially launch cgTask
            paramv[2]=1;
            paramv[1]++;
            ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[0].guid);
            ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
            ocrAddDependence(SB->shimEvent, cgEdt, 1, DB_MODE_RW);
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
            ocrDbRelease(depv[3].guid);
            ocrAddDependence(depv[3].guid, cgEdt, 3, DB_MODE_RW);
//create and launch GSxEdt
            ocrEdtCreate(&gsEdt, SB->GSxTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[1].guid);
            ocrAddDependence(depv[1].guid, gsEdt, 0, DB_MODE_RW);
            ocrAddDependence(depv[2].guid, gsEdt, 1, DB_MODE_RW);
            for(i=0;i<ARITY;i++) ocrAddDependence(NULL_GUID, gsEdt, i+2, DB_MODE_RW);
        return NULL_GUID;
    }
}
ocrGuid_t computeInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
param[0] = mynode
depv
0: userSharedBlock
1: GSsharedblock
2: My GSdatablock
Stub connection to cgTask but could create other datablocks as needed (one cgBlock in this case)
computeInit is launched by the initialization phase of the GS library
*/
    u64 mynode = paramv[0];
    userSharedBlock_t * uSB = depv[0].ptr;
    ocrGuid_t cgEdt, cgBlock, dummy;
//create and launch cgTask
    u64 paramvout[3] = {mynode, 0, 0};
    ocrDbCreate(&(cgBlock), (void**) &dummy, sizeof(cgBlock_t), 0, NULL_GUID, NO_ALLOC);
    ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
    ocrAddDependence(depv[1].guid, cgEdt, 1, DB_MODE_RW);
    ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
    ocrAddDependence(cgBlock, cgEdt, 3, DB_MODE_RW);

    return NULL_GUID;
}


ocrGuid_t rtrShimEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
no params
depv[0] = GSsharedBlock
this transfers rtr to rtrold safely
before launching the next compute phase
*/
    GSsharedBlock_t * SB = depv[0].ptr;
    ocrGuid_t once, dest;
    ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
    dest = SB->shimEvent;
    SB->shimEvent = once;
    SB->rtrold = SB->rtr;   //key line
    SB->rtr = SB->sum;
//launch cgTask by satisfying the last dependency
    ocrDbRelease(depv[0].guid);
    ocrEventSatisfy(dest, depv[0].guid);
}

ocrGuid_t realMainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[0]: number of workers
depv[2]
0: userSharedBlock
1: GSsharedBlock
realMain launches the GS initialization
*/
    u64 i, mynode;
    ocrGuid_t once;
    userSharedBlock_t * uSB = depv[0].ptr;
    GSsharedBlock_t * SB = depv[1].ptr;
    SB->numnodes = N;
    ocrEdtTemplateCreate(&(uSB->cgTemplate), cgTask, 3, 4);
    ocrEdtTemplateCreate(&(uSB->rtrShimTemplate), rtrShimEdt, 0, 1);
    SB->userBlock = depv[0].guid;
    ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
    SB->shimEvent = once;
    ocrGuid_t gsBlock, GSi, sticky, dummy;
    ocrEdtTemplateCreate(&(SB->GSiTemplate), GSiEdt, 3, 2);
    ocrDbCreate(&(gsBlock), (void**) &dummy, sizeof(gsBlock_t), 0, NULL_GUID, NO_ALLOC);
//launch GSi
    u64 GSparamvout[3] = {0, NULL_GUID, NULL_GUID};
    ocrEdtCreate(&GSi, SB->GSiTemplate, EDT_PARAM_DEF, GSparamvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, GSi, 0, DB_MODE_RW);
    ocrAddDependence(gsBlock, GSi, 1, DB_MODE_RW);
    return NULL_GUID;
}

ocrGuid_t wrapupEdt(){
    ocrShutdown();
    }
ocrGuid_t mainEdt(){
/*
mainEdt is executed first
Creates the datablocks
creates realmain as a FINISH EDT
creates wrap up to depend on realmain
launches wrapup
launches realmain
*/
    u64 i;
    PRINTF("conjugate gradient driver with Shim coming out of computing rtr\n");
    PRINTF("Number of timesteps is %d \n", T);
    PRINTF("Number of workers is %d \n", N);
    PRINTF("Rows per worker %d \n", M);
    u64 *dummy;
    ocrGuid_t realMain, GSsharedBlock, userSharedBlock, realMainTemplate, output, wrapup, wrapupTemplate;
    ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, 1);
    ocrEdtTemplateCreate(&realMainTemplate, realMainEdt, 0, 2);
    ocrEdtCreate(&realMain, realMainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &output);
    ocrDbCreate(&GSsharedBlock, (void **) &dummy, sizeof(GSsharedBlock_t), 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&userSharedBlock, (void **) &dummy, sizeof(userSharedBlock_t), 0, NULL_GUID, NO_ALLOC);
    ocrEdtCreate(&wrapup, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL_GUID);
    ocrAddDependence(output, wrapup, 0, DB_MODE_RW);
    ocrAddDependence(userSharedBlock, realMain, 0, DB_MODE_RW);
    ocrAddDependence(GSsharedBlock, realMain, 1, DB_MODE_RW);
    return NULL_GUID;
}
