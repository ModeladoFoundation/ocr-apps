/*
Author: David Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*

See README and stencil1D.h for more documentation

There are 3 versions of a 1D stencil computation.
This code uses NO events passing guids of EDTs instead

This code does a 1D (3 point) stencil computation on a set of points in pure ocr

There are three parameters defined in stencil1D.h:

N is the number of tasks
M is the number of points in each task
I is the number of iterations

the datapoints are initialized to zero except the boundary values which are set to 1
the particular stencil implemented is anew(i) = .5*(a(i+1) + a(i-1))
which implements an elliptic solver using the 2nd order three point finite difference.
The values converge (slowly) to all 1s.

if the stencil is wider than 3 points, then the number of shared boundary values
would have to be increased.

*/

#include <ocr.h>
#include <stdio.h>   //needed only to support debug printing
#include "stencil1D.h"

typedef struct{
    u64 timestep;
    u64 mynode;
    ocrGuid_t template;
    ocrGuid_t mychild;
    } private_t;

ocrGuid_t stencilTask(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
Paramv[0]: none

depv[4]:
0: My datablock
1: Left buffer block (provided by my left neighbor)
2: Right buffer block (provided by my right neighbor)
3: My private block
*/
    ocrGuid_t stencilEdt, oldEdt;
    u64 i;

    double * a = depv[0].ptr;
    double * atemp = a + M;
    buffer_t * leftin = depv[1].ptr;
    buffer_t * rightin = depv[2].ptr;
    private_t * private = depv[3].ptr;
    u64 mynode = private->mynode;
    u64 timestep = private->timestep;
    ocrGuid_t leftinEvent = NULL_GUID;
    ocrGuid_t rightinEvent = NULL_GUID;
//initialize
    if(timestep == 0) {
        if(M==1) {  //special code for single data point
            if(leftin == NULL || rightin == NULL) a[0] = 1;
              else a[0] = 0;
          }else{
            if(leftin == NULL) a[0] = 1;
              else {
                a[0] = 0;
                leftin->buffer = 0;
            }

            for(i=1;i<M-1;i++) a[i] = 0;

            if(rightin == NULL) a[M-1] = 1;
                else {
                a[M-1] = 0;
                rightin->buffer = 0;
            }
        }
    }


    if(timestep < T) {

//compute
        if(M==1) {  //special code for single data point
            if(leftin == NULL || rightin == NULL) atemp[0] = a[0];
              else atemp[0] = 0.5*a[0] + 0.25*(leftin->buffer + rightin->buffer);
          } else {
            if(leftin != NULL) atemp[0] =  .5*a[0] + .25*(a[1] + leftin->buffer);
                else atemp[0] = a[0];

            for(i=1;i<M-1;i++) atemp[i] =  0.5*a[i] + 0.25*(a[i+1] + a[i-1]);
                if(rightin != NULL) atemp[M-1] = 0.5* a[M-1] + .25*( (rightin->buffer) + a[M-2]);
                    else atemp[M - 1] = a[M-1];
        }
//could optimized by changing a pointer rather than actually copying
        for(i=0;i<M;i++) a[i] = atemp[i];

        if(timestep<T-1)ocrEdtCreate(&stencilEdt, private->template, EDT_PARAM_DEF, NULL,
                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
            else stencilEdt = NULL_GUID;

    ocrGuid_t mychild = private->mychild;
    private->mychild = stencilEdt;
    if(timestep<T-1) private->mychild = stencilEdt;

        if(leftin != NULL) {
            leftin->buffer = a[0];
            oldEdt = leftin->control;
            leftin->control= stencilEdt;
            ocrDbRelease(depv[1].guid);
            ocrAddDependence(depv[1].guid, oldEdt, 2, DB_MODE_RW);
        }else ocrAddDependence(NULL_GUID, mychild, 1, DB_MODE_RW);



        if(rightin != NULL) {
            rightin->buffer = a[M-1];
            oldEdt = rightin->control;
            rightin->control = stencilEdt;
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, oldEdt, 1, DB_MODE_RW);
        }else ocrAddDependence(NULL_GUID, mychild, 2, DB_MODE_RW);


        private->timestep = timestep+1;

//create clone
//
        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, mychild, 0 , DB_MODE_RW);
        ocrDbRelease(depv[3].guid);
        ocrAddDependence(depv[3].guid, mychild, 3 , DB_MODE_RW);
    }
    return NULL_GUID;
}
ocrGuid_t wrapupTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv[0]:
depv[N+1]:
0-(N-1) the datablocks in order
N: output event of realmain

waits until realmain is done
prints out the "answer"
*/
    u64 i, j;
    double * data[N];
    for(i=0;i<N;i++) {
        data[i] = depv[i].ptr;
        for(j=0;j<M;j++) PRINTF("%lld %lld %f \n", i, j, data[i][j]);
        }
    if(M==50&&N==10&&T==10000) //default values for Jenkins
        if(data[4][49] - .000836 < 1e-5) PRINTF("PASS\n");
           else PRINTF("fail by %f\n", data[4][3]-.000836);
    ocrShutdown();
    return NULL_GUID;
}
ocrGuid_t realmainTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params: none
depv[4N-2]:
0 - (N-1) the N real datablocks
N - (3N-3): the (2*N-2) buffer datablocks
3N-2 to 4N-3: N private blocks

creates 2*N stencilEdts
passes in the main datablock and the correct set of 2 buffer blocks (or NULL_GUID)
passes in the private block
*/
    u64 i, mynode;
    ocrGuid_t stencilTemplate, stencilEdt[N], childEdt[N], tempEdt;
    ocrEdtTemplateCreate(&stencilTemplate, stencilTask, 0, 4);
    u64 *dummy;
    u64 j;

    double * a[N];
    buffer_t * buffer[2*N-2];
    private_t * private[N];

    for(i=0;i<N;i++) a[i] = depv[i].ptr;
    for(i=0;i<2*N-2;i++) buffer[i] = depv[N+i].ptr;
    for(i=0;i<N;i++) private[i] = depv[3*N-2+i].ptr;

    for(i=0;i<N;i++) {
        private[i]->mynode = i;
        private[i]->timestep = 0;
        private[i]->template = stencilTemplate;
        ocrEdtCreate(&stencilEdt[i], stencilTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrEdtCreate(&childEdt[i], stencilTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        private[i]->mychild = childEdt[i];
        }

//march through buffer blocks
    ocrAddDependence(NULL_GUID, stencilEdt[0], 1, DB_MODE_RW);
    u64 toleft =  0;
    u64 toright =  toleft + N - 1;
    for(i=0;i<N-1;i++) {
        buffer[toright]->control = childEdt[i];
        ocrDbRelease(depv[N+toright].guid);
        ocrAddDependence(depv[N+toright++].guid, stencilEdt[i+1], 1, DB_MODE_RW);

        buffer[toleft]->control = childEdt[i+1];
        ocrDbRelease(depv[N+toleft].guid);
        ocrAddDependence(depv[N+toleft++].guid, stencilEdt[i], 2, DB_MODE_RW);
    }
    ocrAddDependence(NULL_GUID, stencilEdt[N-1], 2, DB_MODE_RW);
    private[N-1]->mynode = N-1;
    private[N-1]->timestep = 0;

//launch N stencil Edts, attach the data db and private db

    for(i=0;i<N;i++) {
        ocrDbRelease(depv[i].guid);  //release all dbs
        ocrAddDependence(depv[i].guid, stencilEdt[i], 0, DB_MODE_RW);
        ocrDbRelease(depv[3*N-2+i].guid);  //release all dbs
        ocrAddDependence(depv[3*N-2+i].guid, stencilEdt[i], 3, DB_MODE_RW);
    }
    return NULL_GUID;
}
ocrGuid_t mainEdt(){

/*
mainEdt is executed first
Creates the datablocks
creates wrapup
creates realmain
launches wrapup
launches realmian
*/

    u64 i;
    u64 *dummy;
    ocrGuid_t realmain, realmainOutputEvent, realmainTemplate, dataDb[N], privateDb[N], bufferDb[2*N-2];
 PRINTF("1D stencil code GUID style: \nnumber of workers = %d \ndata on each worker = %d \nnumber of timesteps = %d \n", N, M, T);
    for(i=0;i<2*N-2;i++) {
       ocrDbCreate(&(bufferDb[i]), (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
   }


    for(i=0;i<N;i++) {
       ocrDbCreate(&(privateDb[i]), (void**) &dummy, sizeof(private_t), 0, NULL_GUID, NO_ALLOC);
       ocrDbCreate(&(dataDb[i]), (void**) &dummy, 2*M*sizeof(double), 0, NULL_GUID, NO_ALLOC);
   }

    ocrGuid_t wrapupTemplate;
    ocrGuid_t wrapupEdt;

    ocrEdtTemplateCreate(&wrapupTemplate, wrapupTask, 0, N+1);
    ocrEdtCreate(&wrapupEdt, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

    ocrEdtTemplateCreate(&realmainTemplate, realmainTask, 0, 4*N-2);
    ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
          EDT_PROP_FINISH, NULL_GUID, &realmainOutputEvent);

    for(i=0;i<N;i++)  ocrAddDependence(dataDb[i], wrapupEdt, i, DB_MODE_RW);
        ocrAddDependence(realmainOutputEvent, wrapupEdt, N, DB_MODE_RW);

    for(i=0;i<N;i++) ocrAddDependence(dataDb[i], realmain, i, DB_MODE_RW);
    for(i=0;i<2*N-2;i++) ocrAddDependence(bufferDb[i], realmain, N+i, DB_MODE_RW);
    for(i=0;i<N;i++) ocrAddDependence(privateDb[i], realmain, (3*N-2)+i, DB_MODE_RW);

    return NULL_GUID;
}
