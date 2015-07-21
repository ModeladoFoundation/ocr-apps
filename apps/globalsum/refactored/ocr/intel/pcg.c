/*
Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
driver for global sum code
see README for more documentation
*/
//this version of the driver program updates rtzold in the alternate phase (during the pAp computation)
#include <ocr.h>
#include <stdio.h>
#include <math.h>
#include "ocrGS.h"
typedef struct {
    double x[M];
    double r[M];
    double b[M];
    double p[M];
    double z[M];
    double ap[M];
    double a[M];
    double pc[M];
    } cgBlock_t;
typedef struct{
    ocrGuid_t cgTemplate;
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
    ocrGuid_t cgEdt, gsEdt;
    double sum, pap, rtz, alpha, beta;
    u64 i;
    switch (phase) {
        case 0:
//Initial call only
//Initialize matrix and vectors
        sum = M*mynode+1;
        for(i=0;i<M;i++){
            cgdata->a[i] = sum;
            cgdata->b[i] = sum;
            cgdata->r[i] = sum;
            cgdata->p[i] = sum++;
            cgdata->x[i] = 0.0;
            cgdata->pc[i] = sum;  //already incremented
        }
//Prepare for the first summation of rtz
        sum = 0;
        for(i=0;i<M;i++) sum += cgdata->r[i]*cgdata->r[i];
        mydata->data = sum;  //local sum
//Create clone
        paramv[2] = 1;
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
    case 1:
//consume rtr
        if(mynode==0) PRINTF("time %d rtr %f \n", timestep, SB->sum);
        if(timestep==0) SB->rtr0 = SB->sum;
           else if(SB->sum/SB->rtr0 < 1e-13 || timestep == T) {
             for(i=0;i<M;i++) PRINTF("CG%d T%d  %d value %f \n", mynode, timestep, i, cgdata->x[i]);
             if(mynode == 0 && M==300 && N==20 && T==100) {
                if(fabs(cgdata->x[0] - .999794) < 1e-5) PRINTF("PASS\n"); else PRINTF("FAIL difference %f is too large\n", cgdata->x[0] - .999794); }
             return NULL_GUID;
        }
//preconditioning
        for(i=0;i<M;i++) cgdata->z[i] = cgdata->r[i]/cgdata->pc[i];
//compute local rtz
        sum = 0;
        for(i=0;i<M;i++) sum += cgdata->z[i]*cgdata->r[i];
        mydata->data = sum;
//Create clone
        paramv[2] = 2;
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
//consume rtz
            if(mynode == 0) SB->rtz = SB->sum;
//compute beta
            if(timestep == 0) beta = 0;
                else beta = SB->sum/SB->rtzold;
//update p
            for(i=0;i<M;i++) cgdata->p[i] = cgdata->z[i] + beta*cgdata->p[i];
//compute Ap
            for(i=0;i<M;i++) cgdata->ap[i] = cgdata->a[i]*cgdata->p[i];
//compute pAp
            sum = 0;
            for(i=0;i<M;i++) sum += cgdata->p[i]*cgdata->ap[i];
            mydata->data = sum;
//Create clone
            paramv[2]=3;
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
        case 3:
//consume pAp
            pap = SB->sum;
            alpha = SB->rtz/pap;
            if(mynode == 0) SB->rtzold = SB->rtz;  //Here so don't need shim
//update x and r
            for(i=0;i<M;i++) {
                cgdata->x[i] += alpha*cgdata->p[i];
                cgdata->r[i] -= alpha*cgdata->ap[i];
            }
//Prepare for the summation of rtr
        sum = 0;
        for(i=0;i<M;i++) sum += cgdata->r[i]*cgdata->r[i];
        mydata->data = sum;  //local sum
//create clone
            paramv[2]=1;
            paramv[1]++;
            ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            ocrDbRelease(depv[0].guid);
            ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
            ocrAddDependence(SB->rootEvent, cgEdt, 1, DB_MODE_RW);
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
            ocrDbRelease(depv[3].guid);
            ocrAddDependence(depv[3].guid, cgEdt, 3, DB_MODE_RW);
//launch global sum for pAp
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
//create and launch cgTask
    ocrGuid_t cgEdt, cgBlock, dummy;
    u64 paramvout[3] = {mynode, 0, 0};
    ocrDbCreate(&(cgBlock), (void**) &dummy, sizeof(cgBlock_t), 0, NULL_GUID, NO_ALLOC);
    ocrEdtCreate(&cgEdt, uSB->cgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[0].guid, cgEdt, 0, DB_MODE_RW);
    ocrAddDependence(depv[1].guid, cgEdt, 1, DB_MODE_RW);
    ocrAddDependence(depv[2].guid, cgEdt, 2, DB_MODE_RW);
    ocrAddDependence(cgBlock, cgEdt, 3, DB_MODE_RW);
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
    u64 i, mynode;
    ocrGuid_t once;
    userSharedBlock_t * uSB = depv[0].ptr;
    GSsharedBlock_t * SB = depv[1].ptr;
    SB->numnodes = N;  //setting the number of "ranks"   REQUIRED
    ocrEdtTemplateCreate(&(uSB->cgTemplate), cgTask, 3, 4);
    SB->userBlock = depv[0].guid;   //passed through GSiEDT to computeInitEdt
    ocrGuid_t gsBlock, GSi, sticky, dummy;
    ocrEdtTemplateCreate(&(SB->GSiTemplate), GSiEdt, 3, 2);  //needed both to launch GSiEDT and inside for cloning REQUIRED
    ocrDbCreate(&(gsBlock), (void**) &dummy, sizeof(gsBlock_t), 0, NULL_GUID, NO_ALLOC); //needed for GSiEDT   REQUIRED
//create and luanch GSiEdt
    u64 GSparamvout[3] = {0, NULL_GUID, NULL_GUID};  //REQUIRED
    ocrEdtCreate(&GSi, SB->GSiTemplate, EDT_PARAM_DEF, GSparamvout, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);  //REQUIRED
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, GSi, 0, DB_MODE_RW);  //REQUIRED
    ocrAddDependence(gsBlock, GSi, 1, DB_MODE_RW);  //REQUIRED
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
launches realmain (wrapup waits until realmain is done)
*/
    u64 i;
    u64 *dummy;
    ocrGuid_t realMain, GSsharedBlock, userSharedBlock, realMainTemplate, output, wrapup, wrapupTemplate;
    PRINTF("Preconditioned driver\n");
    PRINTF("Number of timesteps is %d \n", T);
    PRINTF("Number of workers is %d \n", N);
    PRINTF("Rows per worker %d \n", M);
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
