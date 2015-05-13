/*
This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed withs file is subject to the license agreement located in the file LICENSE
 *  *  * and cannot be distributed without it. This notice cannot be
 *   *   * removed or modified.


This code does a 1D (3 point) stencil computation on a set of points in pure ocr
This version avoids any use of events.  Like the "once" version it creates 2 sets of EDTs
immediately and then has each EDT create its grandchild.  Rather than creating events it
passes the GUID of the EDT so that the halo datablocks can just be attached directly

N is the number of tasks
M is the number of points in each task
T is the number of time steps

the datapoints are initialized to zero except the boundary values which are set to 1
the particular stencil implemented is anew(i) = .5*a(i) + .25*(a(i+1) + a(i-1))

The values converge (slowly) to all 1s.

Control structure
  N stenciltask EDTs do the computation on M numbers each
  Each stencil task has a "real" datablock for computing values
  Each stencil has a private datablock to hold parameters
  Each stenciltask "clones" itself for each iteration
  Each stenciltask has two data blocks for sending to each neighbor


Structure of code

mainEDT
  creates N datablocks of length 2M
  creates 2*N-2 datablocks for exchanging halo data and control information
  creates N "private" datablocks for local parameters
  launches wrapup that depends on realmain finishing
  launches realmain as a finish EDT

realmain
  creates 2*N stencilEdts
  launches the N stencilEdt with the correct datablocks attached


Each stencil event
  if(iteration == 0) initializes data
  computes its new values using neighbor values as needed
  writes its boundary value(s) to the datablock(s)
  creates it granchild
  copies guid to datablocks
  adds a dependence to its two neighbors
  launches child by adding local datablocks

The values after final iteration are printed out in order by a wrapupEdt.
To make sure wrapup doesn't start early, it depends on realmain
which is declared to be a FINISH event (which means it
doesn't satisfy its output event until ALL of its descendents are finished).


*/

#include <ocr.h>
#include <stdio.h>

#define N 10  //number of blocks
#define M 5   // size of local block
#define T 200  //number of iterations

typedef struct{
    double buffer;
    ocrGuid_t edt; //neighbors child
    } buffer_t;

typedef struct{
    u64 timestep;
    u64 mynode;
    ocrGuid_t template;
    ocrGuid_t mychild;
    } private_t;



ocrGuid_t stencilTask(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
Parameters: none

Dependencies:
0: My datablock
1: Left block (satisfied by my left neighbor)
2: Right block (satisfied by my right neighbor)
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

    if(timestep == 0) { //initialize
        if(M==1) {  //special code for single data point
            if(leftin == NULL || rightin == NULL) a[0] = 1;
              else a[0] = 0;
          } else {
            if(leftin == NULL) a[0] = 1;
              else {
                a[0] = 0;
                leftin->buffer = 0;

            for(i=1;i<M-1;i++) a[i] = 0;

            if(rightin == NULL) a[M-1] = 1;
                else {
                a[M-1] = 0;
                rightin->buffer = 0;
            }
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

        if(leftin != NULL) { //leftin is NULL too
            leftin->buffer = a[0];
            oldEdt = leftin->edt;
            leftin->edt= stencilEdt;
            ocrDbRelease(depv[1].guid);
            ocrAddDependence(depv[1].guid, oldEdt, 2, DB_MODE_ITW);
        }else ocrAddDependence(NULL_GUID, mychild, 1, DB_MODE_ITW);



        if(rightin != NULL) {
            rightin->buffer = a[M-1];
            oldEdt = rightin->edt;
            rightin->edt = stencilEdt;
            ocrDbRelease(depv[2].guid);
            ocrAddDependence(depv[2].guid, oldEdt, 1, DB_MODE_ITW);
        }else ocrAddDependence(NULL_GUID, mychild, 2, DB_MODE_ITW);


        private->timestep = timestep+1;

//create clone
//
        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, mychild, 0 , DB_MODE_ITW);

        ocrDbRelease(depv[3].guid);
        ocrAddDependence(depv[3].guid, mychild, 3 , DB_MODE_ITW);

    }

    return NULL_GUID;
}


ocrGuid_t wrapupTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
waits until realmain is done
prints out the "answer"

params: none

dependencies
0-(N-1) the datablocks in order
N: output event of realmain

*/

    u64 i, j;
    double * data[N];
    for(i=0;i<N;i++) {
        data[i] = depv[i].ptr;
        for(j=0;j<M;j++) PRINTF("%lld %lld %f \n", i, j, data[i][j]);
        }

    ocrShutdown();
    return NULL_GUID;
}


ocrGuid_t realmainTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
creates the first set of sticky events
creates N stencilEdts
passes in the main datablock and the correct set of 2 buffer blocks (or NULL_GUID)
passes in the private block

params: none
dependencies:
0 - (N-1) the N real datablocks
N - (3N-3): the (2*N-2) communication datablocks
3N-2 to 4N-3: N private blocks

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

    ocrAddDependence(NULL_GUID, stencilEdt[0], 1, DB_MODE_ITW);

    u64 toleft =  0;
    u64 toright =  toleft + N - 1;

    for(i=0;i<N-1;i++) {

        buffer[toright]->edt = childEdt[i];
        ocrDbRelease(depv[N+toright].guid);
        ocrAddDependence(depv[N+toright++].guid, stencilEdt[i+1], 1, DB_MODE_ITW);

        buffer[toleft]->edt = childEdt[i+1];
        ocrDbRelease(depv[N+toleft].guid);
        ocrAddDependence(depv[N+toleft++].guid, stencilEdt[i], 2, DB_MODE_ITW);

    }

    ocrAddDependence(NULL_GUID, stencilEdt[N-1], 2, DB_MODE_ITW);
    private[N-1]->mynode = N-1;
    private[N-1]->timestep = 0;

//create N stencil init events, attach the data db

    for(i=0;i<N;i++) {
        ocrDbRelease(depv[i].guid);  //release all dbs
        ocrAddDependence(depv[i].guid, stencilEdt[i], 0, DB_MODE_ITW);
        ocrDbRelease(depv[3*N-2+i].guid);  //release all dbs
        ocrAddDependence(depv[3*N-2+i].guid, stencilEdt[i], 3, DB_MODE_ITW);
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


 PRINTF("1D stencil code: \nnumber of workers = %d \ndata on each worker = %d \nnumber of timesteps = %d \n", N, M, T);


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

    for(i=0;i<N;i++)  ocrAddDependence(dataDb[i], wrapupEdt, i, DB_MODE_ITW);
        ocrAddDependence(realmainOutputEvent, wrapupEdt, N, DB_MODE_ITW);

    for(i=0;i<N;i++) ocrAddDependence(dataDb[i], realmain, i, DB_MODE_ITW);
    for(i=0;i<2*N-2;i++) ocrAddDependence(bufferDb[i], realmain, N+i, DB_MODE_ITW);
    for(i=0;i<N;i++) ocrAddDependence(privateDb[i], realmain, (3*N-2)+i, DB_MODE_ITW);

    return NULL_GUID;
}
