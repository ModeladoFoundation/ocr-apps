/*
This code does a 1D (3 point) stencil computation on a set of points in pure ocr

N is the number of tasks
M is the number of points in each task
T is the number of timesteps

the datapoints are initialized to zero except the boundary values which are set to 1
the particular stencil implemented is anew(i) = a(i) + .5*(a(i+1)-a(i-1))
which implements the equivalent of Gaby Jost's CnC version

if the stencil is wider than 3 points, then the number of shared boundary values
would have to be increased.

This stencil is numerically unstable so that the values blow up over time.

Control structure
  N stenciltask EDTs to do the computation on M numbers each
  Each stencil task has a private datablock for computing values
  Each stenciltask "clones" itself for each timestep
  Each stenciltask has two data blocks for sending to each neighbor
   (the blocks alternate each time step to break a race condition)
  Neighboring stencil tasks share sticky events.
    The receiver uses the sticky event as a dependence for its clone
    The sender staisfies the event with the boundary data
    THe sender creates the sticky event for the next exchange and puts it in the datablok
    The receiver is responsible for destroying the used sticky event


Structure of code

mainEDT
  creates N datablocks of length 2M
  creates 4*N-4 datablocks for exchanging halo data and control information
  launches wrapup that depends on realmain finishing
  launches realmain as a finish EDT

realmain
  creates N stencil Init tasks
  creates 2N-2 sticky events that will be used to pass the first boundary blocks
  launches the N stencil init tasks with the correct datablocks attached
    and the sticky events in the parameter list

Each stencil init task
  initializes their datablock
  writes its boundary values to the out data blocks
  creates the first stencil event
  creates the sticky events and puts them in the out datablocks
  launches the first stencil event

Each stencil event
  destroys the previous sticky events
  computes its time step using neighbor values as needed
  writes its boundary value(s) to the out datablock(s)
  creates the sticky events and puts them in the out datablocks
  enables its nephews by satisfying the previous sticky events with
    the out datablocks
  clones itself for the next iteration
  launches its clone by attaching 3 datablocks and 2 sticky events

The sticky events prevent the sons from starting until their uncle(s)
are finished writing and making the data available.

They have to be sticky events to avoid the race condition that they
may be satisfied before the nephews are created

The boundary data blocks alternate each time step to make sure that my son
doesn't overwrite my data before my nephews have read it

The blocks an Edt is writing to are acquired in ITW mode
The blocks that a stencilEdt is reading are acquired in RO
mode.

The final timestep is printed out in order by a wrapup event.
To make sure wrapup doesn't start early, it depends on realmain
which is declared to be a finish event (which means it
doesn't satisfy its output event until ALL of its family
tree is finish).


*/

#include <ocr.h>
#include <stdio.h>

#define N 5  //number of blocks
#define M 3   // size of local block
#define T 20  //number of time steps

typedef struct{
    double buffer;
    ocrGuid_t link; //sticky event for control
    } buffer_t;


ocrGuid_t stencilTask(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
Parameters:
0: stencilTemplate
1: timestep
2: mynode
3: left send block guid for next iteration
4: right send block guid for next iteration
5: left send event
6: right send event
7: old event to destroy
8: old event to destroy

Dependencies:
0: My datablock
1: Leftin block (satisfied by my left neighbor)
2: Leftout block
3: Rightout block
4: Rightin block (satisfied by my right neighbor)
*/


    u64 i, phase, phasenext;
    u64 nexttimestep;


    ocrGuid_t stencilEdt;
    ocrGuid_t mydest, sticky;


    ocrGuid_t stencilTemplate = paramv[0];
    u64 timestep = paramv[1];
    u64 mynode = paramv[2];

    if(paramv[7] != NULL_GUID) {
      ocrEventDestroy(paramv[7]);
      }
    if(paramv[8] != NULL_GUID) {
      ocrEventDestroy(paramv[8]);
      }

    if(timestep < T) {

    double * a = depv[0].ptr;
    double * atemp = a + M;
    buffer_t * leftin = depv[1].ptr;
    buffer_t * leftout = depv[2].ptr;
    buffer_t * rightout = depv[3].ptr;
    buffer_t * rightin = depv[4].ptr;


//compute

      if(leftin != NULL){

        atemp[0] = a[0] + .5*(a[1] - leftin->buffer);
        }
       else atemp[0] = a[0];

    for(i=1;i<M-1;i++) atemp[i] = a[i] + 0.5*(a[i+1] - a[i-1]);

      if(rightin != NULL) {
        atemp[M-1] = a[M-1] + .5*( (rightin->buffer) - a[M-2]);
        }

       else atemp[M - 1] = a[M-1];



//could optimized by changing a pointer rather than actually copying

    for(i=0;i<M;i++) a[i] = atemp[i];


//control nephews


  ocrGuid_t leftinEvent = NULL_GUID;
  ocrGuid_t rightinEvent = NULL_GUID;

    nexttimestep = timestep+1;
    u64 stencilout_paramv[9] = {stencilTemplate, nexttimestep, mynode,
      depv[2].guid, depv[3].guid, NULL_GUID, NULL_GUID, NULL_GUID, NULL_GUID};


        if(leftout != NULL) { //leftin is NULL too
//set next receive from left
          leftinEvent = leftin->link;
//copy boundary
          leftout->buffer = a[0];

//create new sticky
          ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
          leftout->link = sticky;

//satisfy old sticky (send leftout)
          ocrEventSatisfy(paramv[5], depv[2].guid);

//send stickies down
          stencilout_paramv[5] = sticky;
          }


        if(rightout != NULL) {
//set next receive from right
          rightinEvent = rightin->link;

//copy boundary
          rightout->buffer = a[M-1];

//create new sticky
          ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
          rightout->link = sticky;

//satisfy old sticky (send rightout)
          ocrEventSatisfy(paramv[6], depv[3].guid);

//send stickies down
          stencilout_paramv[6] = sticky;
          }

        stencilout_paramv[7] = leftinEvent;
        stencilout_paramv[8] = rightinEvent;

//create son

        ocrEdtCreate(&stencilEdt, stencilTemplate, EDT_PARAM_DEF, stencilout_paramv,
          EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

//launch son



        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, stencilEdt, 0 , DB_MODE_ITW);

        ocrAddDependence(leftinEvent, stencilEdt, 1 , DB_MODE_RO);


        ocrAddDependence(paramv[3], stencilEdt, 2 , DB_MODE_ITW);

        ocrAddDependence(paramv[4], stencilEdt, 3 , DB_MODE_ITW);

        ocrAddDependence(rightinEvent, stencilEdt, 4 , DB_MODE_RO);

     }

return NULL_GUID;
   }







ocrGuid_t wrapupTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
//waits until realmain is done
//prints out the "answer"

    u64 i, j;
    double * data[N];
    for(i=0;i<N;i++) {
      data[i] = depv[i].ptr;
      for(j=0;j<M;j++) PRINTF("%d %d %f \n", i, j, data[i][j]);
      }

    ocrShutdown();
    return NULL_GUID;
}


ocrGuid_t stencilInitTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
initializes the datablocks (zero or 1 at the end)
initializes the buffers values to 0
Creates the first StencilEDT
Creates the first stickyEvents
puts them in the databuffers
launches the first "real" EDTs

params
0: stencilTemplate
1: mynode
2: leftin event
3: leftout event
4: rightout event
5: rightin event
6: next left send block
7: next right send block

Dependencies
0: my datablock
1: leftout
2: rightout

*/



u64 timestep = 0;

u64 i, j;

ocrGuid_t sticky;

ocrGuid_t stencilEdt;

double * a;

buffer_t * leftout, * rightout;

ocrGuid_t stencilTemplate = paramv[0];
u64 mynode = paramv[1];
ocrGuid_t leftinEvent = paramv[2];
ocrGuid_t leftoutEvent = paramv[3];
ocrGuid_t rightoutEvent = paramv[4];
ocrGuid_t rightinEvent = paramv[5];
ocrGuid_t nextleftout = paramv[6];
ocrGuid_t nextrightout = paramv[7];


a = depv[0].ptr;
leftout = depv[1].ptr;
rightout = depv[2].ptr;


//initialize values


if(leftout == NULL) {
   a[0] = 1;
   }
  else {
    a[0] = 0;
    leftout->buffer = 0;
    }

for(i=1;i<M-1;i++) {
  a[i] = 0;

  }

if(rightout == NULL) {
   a[M-1] = 1;
   }
  else {
    a[M-1] = 0;
    rightout->buffer = 0;
    }


    u64 stencil_paramv[9] = {stencilTemplate, 0, mynode, depv[1].guid,
       depv[2].guid, NULL_GUID, NULL_GUID, NULL_GUID, NULL_GUID};


if(leftout != NULL) {
  leftout->buffer = a[0];
  ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
  leftout->link = sticky;

  ocrEventSatisfy(paramv[3], depv[1].guid);

  stencil_paramv[5] = sticky;
  }


if(rightout != NULL) {
  rightout->buffer = a[M-1];
  ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
  rightout->link = sticky;
  stencil_paramv[6] = sticky;


  ocrEventSatisfy(paramv[4], depv[2].guid);
  }


stencil_paramv[7] = leftinEvent;
stencil_paramv[8] = rightinEvent;

ocrEdtCreate(&stencilEdt, stencilTemplate, EDT_PARAM_DEF, stencil_paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


ocrDbRelease(depv[0].guid);

ocrAddDependence(depv[0].guid, stencilEdt, 0, DB_MODE_ITW);

ocrAddDependence(leftinEvent, stencilEdt, 1, DB_MODE_RO);

ocrAddDependence(paramv[6], stencilEdt, 2, DB_MODE_ITW);

ocrAddDependence(paramv[7], stencilEdt, 3, DB_MODE_ITW);

ocrAddDependence(rightinEvent, stencilEdt, 4, DB_MODE_RO);

  return NULL_GUID;

}


ocrGuid_t realmainTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
creates N stendilInitEdts
passes in the datablock and the correct set of 4 buffer blocks (or NULL_GUID)
*/


u64 i, mynode;


ocrGuid_t stencilTaskTemplate, stencilInitTemplate, stencilInitEdt[N];


ocrEdtTemplateCreate(&stencilInitTemplate, stencilInitTask, 8, 3);

ocrEdtTemplateCreate(&stencilTaskTemplate, stencilTask, 9, 5);


u64 *dummy;

u64 paramInitv[8] = {stencilTaskTemplate,0, NULL_GUID, NULL_GUID, NULL_GUID, NULL_GUID, NULL_GUID, NULL_GUID};


u64 buffer = N;

ocrGuid_t sticky;

//for(i=0;i<(N-2);i++) {


ocrGuid_t leftSendBlock = NULL_GUID;
paramInitv[2] = NULL_GUID;
paramInitv[3] = NULL_GUID;
paramInitv[6] = NULL_GUID;

for(i=0;i<N-1;i++) {

  paramInitv[1] = i;
  ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
  paramInitv[4] = sticky;
  ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
  paramInitv[5] = sticky;
  paramInitv[7] = depv[buffer++].guid;

  ocrEdtCreate(&stencilInitEdt[i], stencilInitTemplate, EDT_PARAM_DEF, paramInitv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
  ocrDbRelease(depv[i].guid);
  ocrAddDependence(depv[i].guid, stencilInitEdt[i], 0, DB_MODE_ITW);
  ocrAddDependence(leftSendBlock, stencilInitEdt[i], 1, DB_MODE_ITW);
  ocrAddDependence(depv[buffer++].guid, stencilInitEdt[i], 2, DB_MODE_ITW);

  leftSendBlock = depv[buffer++].guid;

  paramInitv[2] = paramInitv[4];
  paramInitv[3] = paramInitv[5];
  paramInitv[6] = depv[buffer++].guid;

  }



//create N stencil init events, attach the data db

paramInitv[1] = N-1;
paramInitv[4] = NULL_GUID;
paramInitv[5] = NULL_GUID;
paramInitv[7] = NULL_GUID;

  ocrEdtCreate(&stencilInitEdt[N-1], stencilInitTemplate, EDT_PARAM_DEF, paramInitv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
  ocrDbRelease(depv[N-1].guid);
  ocrAddDependence(depv[N-1].guid, stencilInitEdt[i], 0, DB_MODE_ITW);
  ocrAddDependence(leftSendBlock, stencilInitEdt[N-1], 1, DB_MODE_ITW);
  ocrAddDependence(NULL_GUID, stencilInitEdt[N-1], 2, DB_MODE_ITW);



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
ocrGuid_t realmain, realmainOutputEvent, realmainTemplate, dataDb[N], bufferDb[4*N-4];

for(i=0;i<4*N-4;i++) {
   ocrDbCreate(&(bufferDb[i]), (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
   }


for(i=0;i<N;i++) {
   ocrDbCreate(&(dataDb[i]), (void**) &dummy, 2*M*sizeof(double), 0, NULL_GUID, NO_ALLOC);
   }


ocrGuid_t wrapupTemplate;
ocrGuid_t wrapupEdt;

ocrEdtTemplateCreate(&wrapupTemplate, wrapupTask, 0, N+1);

ocrEdtCreate(&wrapupEdt, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

ocrEdtTemplateCreate(&realmainTemplate, realmainTask, 0, 5*N-4);


ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
  EDT_PROP_FINISH, NULL_GUID, &realmainOutputEvent);


for(i=0;i<N;i++)  ocrAddDependence(dataDb[i], wrapupEdt, i, DB_MODE_ITW);
ocrAddDependence(realmainOutputEvent, wrapupEdt, N, DB_MODE_ITW);


for(i=0;i<N;i++) ocrAddDependence(dataDb[i], realmain, i, DB_MODE_ITW);
for(i=0;i<4*N-4;i++) ocrAddDependence(bufferDb[i], realmain, N+i, DB_MODE_ITW);


return NULL_GUID;

}
