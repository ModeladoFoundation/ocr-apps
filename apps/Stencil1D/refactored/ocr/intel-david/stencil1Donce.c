/*
This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed withs file is subject to the license agreement located in the file LICENSE
 *  *  * and cannot be distributed without it. This notice cannot be
 *   *   * removed or modified.


This code does a 1D (3 point) stencil computation on a set of points in pure ocr
This version uses default (ONCE) events.
It avoids the race condition by having an EDT create its GRANDCHILD and attaching
the events immediately.  It then passes the guid down to its child.

N is the number of tasks
M is the number of points in each task
T is the number of iterations

the datapoints are initialized to zero except the boundary values which are set to 1
the particular stencil implemented is anew(i) = .5*a(i) + .25*(a(i+1) + a(i-1))
The values converge (slowly) to all 1s.


Control structure
  N stenciltask EDTs do the computation on M numbers each
  Each stencil task has a "real" datablock for computing values
  Each stencil has a private datablock to hold parameters
  Each stenciltask has two data blocks for sending to each neighbor
  Each stenciltask creates its grandchild
  Neighboring stencil tasks share once events.

Structure of code

mainEDT
  creates N datablocks of length 2M
  creates 2*N-2 datablocks for exchanging halo data and control information
  N "private" datablocks for local parameters
  launches realmain as a finish EDT
  launches wrapup that depends on realmain finishing

realmain
  creates 2*N stencilEdts (first two generations
  creates 2N-2 once events that will be used to pass the first boundary blocks
  adds them as dependencies to the second generation
  puts the guids of the second generation into the private block
  launches N stencilEdt with the correct datablocks attached

Each stencil event
  if(iteration == 0) initializes data
  computes its new values using neighbor values as needed
  writes its boundary value(s) to the buffer datablock(s)
  creates the (once) events and puts them in the out datablocks
  creates its grandchild and attaches the events
  enables its nephews by satisfying the received events with
    the datablocks
  launches its clone by attaching 2 datablocks (the events are already attached)

The values after final iteration are printed out in order by a wrapupEdt.
To make sure wrapup doesn't start early, it depends on realmain
which is declared to be a finish event (which means it
doesn't satisfy its output event until ALL of its descendents are finished).


*/

#include <ocr.h>
#include <stdio.h>

#define N 10  //number of blocks
#define M 5   // size of local block
#define T 200  //number of iterations

typedef struct{
    double buffer;
    ocrGuid_t link; //once event for control
    } buffer_t;

typedef struct{
    u64 timestep;
    u64 mynode;
    ocrGuid_t mychild;
    } private_t;



ocrGuid_t stencilTask(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
Parameters:
0: stencilTemplate

Dependencies:
0: My datablock
1: Leftin block (satisfied by my left neighbor)
2: Leftout block
3: Rightout block
4: Rightin block (satisfied by my right neighbor)
5: My private block
*/


    u64 i, phase, phasenext;

    ocrGuid_t stencilEdt;
    ocrGuid_t once;


    ocrGuid_t stencilTemplate = paramv[0];

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


        if(timestep<T-1)ocrEdtCreate(&stencilEdt, stencilTemplate, EDT_PARAM_DEF, (u64 *) &stencilTemplate,
                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        once = NULL_GUID;

        if(leftin != NULL) { //leftin is NULL too
            leftinEvent = leftin->link;
            leftin->buffer = a[0];
            ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
            leftin->link = once;
            ocrDbRelease(depv[1].guid);
            ocrEventSatisfy(leftinEvent, depv[1].guid);
        }
        if(timestep<T-1) ocrAddDependence(once, stencilEdt, 1, DB_MODE_ITW);

        once = NULL_GUID;
        if(rightin != NULL) {
            rightinEvent = rightin->link;
            rightin->buffer = a[M-1];
            ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
            rightin->link = once;
            ocrDbRelease(depv[2].guid);
            ocrEventSatisfy(rightinEvent, depv[2].guid);
        }
        if(timestep<T-1) ocrAddDependence(once, stencilEdt, 2, DB_MODE_ITW);

        private->timestep = timestep+1;


//create clone
//
    ocrGuid_t mychild = private->mychild;
    if(timestep<T-1) private->mychild = stencilEdt;

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
N - (5N-5): the (2*N-2) communication datablocks
  2*N-2 of them will be passed as dependencies to the stencilEdts
  2*N-2 of them will be passed in as parameters for the next iteration

*/


    u64 i, mynode;
    ocrGuid_t stencilTemplate, stencilEdt[N], tempEdt;
    ocrEdtTemplateCreate(&stencilTemplate, stencilTask, 1, 4);
    u64 *dummy;
    u64 j;
    ocrGuid_t once;

    double * a[N];
    buffer_t * buffer[2*N-2];
    private_t * private[N];



    for(i=0;i<N;i++) a[i] = depv[i].ptr;
    for(i=0;i<2*N-2;i++) buffer[i] = depv[N+i].ptr;
    for(i=0;i<N;i++) private[i] = depv[3*N-2+i].ptr;




    for(i=0;i<N;i++) {
        private[i]->mynode = i;
        private[i]->timestep = 0;
        ocrEdtCreate(&stencilEdt[i], stencilTemplate, EDT_PARAM_DEF, (u64 *) &stencilTemplate, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrEdtCreate(&tempEdt, stencilTemplate, EDT_PARAM_DEF, (u64 *) &stencilTemplate, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        private[i]->mychild = tempEdt;
        }

    ocrAddDependence(NULL_GUID, private[0]->mychild, 1, DB_MODE_ITW);
    ocrAddDependence(NULL_GUID, stencilEdt[0], 1, DB_MODE_ITW);

    u64 toleft =  0;
    u64 toright =  toleft + N - 1;

    for(i=0;i<N-1;i++) {

        ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
        buffer[toright]->link = once;
        ocrAddDependence(once, private[i]->mychild, 2, DB_MODE_ITW);
        ocrDbRelease(depv[N+toright].guid);
        ocrAddDependence(depv[N+toright++].guid, stencilEdt[i+1], 1, DB_MODE_ITW);


        ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
        buffer[toleft]->link = once;
        ocrAddDependence(once, private[i+1]->mychild, 1, DB_MODE_ITW);
        ocrDbRelease(depv[N+toleft].guid);
        ocrAddDependence(depv[N+toleft++].guid, stencilEdt[i], 2, DB_MODE_ITW);

    }

    ocrAddDependence(NULL_GUID, private[N-1]->mychild, 2, DB_MODE_ITW);
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
creates realmain
launches realmian
*/

    u64 i;


    PRINTF("1D stencil code: \nnumber of workers = %d \ndata on each worker = %d \nnumber of timesteps = %d \n", N, M, T);

    u64 *dummy;
    ocrGuid_t realmain, realmainOutputEvent, realmainTemplate, dataDb[N], privateDb[N], bufferDb[2*N-2];

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
