/*
Author: David Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
See README for more documentation

There are 6 versions of a 1D stencil computation.
This code uses STICKY events with serial initialization

This code does a 1D (3 point) stencil computation on a set of points in pure ocr

nrank is the number of tasks
npoints is the number of points in each task
maxt is the number of timesteps


the datapoints are initialized to zero except the global boundary values which are set to 1
the particular stencil implemented is anew(i) = .5*(a(i) + 0.25*(a(i+1) + a(i-1))
which implements an elliptic solver using the 2nd order three point finite difference.
The values converge (slowly) to all 1s.

if the stencil is wider than 3 points, then the number of shared boundary values
would have to be increased.
*/
#include <ocr.h>
#include <stdio.h>

//there will be one private datablock per rank
//the datablock contains the private structure padded by npoints of doubles to hold the values
typedef struct{
    u32 nrank;
    u32 npoints;
    u32 maxt;
    u32 timestep;
    u32 myrank;
    u32 dataOffset;
    ocrGuid_t leftSendEVT;
    ocrGuid_t rightSendEVT;
    ocrGuid_t oldLeftRecvEVT;
    ocrGuid_t oldRightRecvEVT;
    ocrGuid_t stencilTML;
    } private_t;

//this structure is used to pass boundary values between ranks
typedef struct{
    double buffer;
    ocrGuid_t EVT;
    } buffer_t;


typedef struct {
   ocrEdtDep_t private;
   ocrEdtDep_t leftin;
   ocrEdtDep_t rightin;
} stencilDepv_t;


#define DEPV(var,field) (stencilDEPV->var).field
#define SLOT(var) (offsetof(stencilDepv_t,var)/sizeof(ocrEdtDep_t))


ocrGuid_t stencilEDT(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
paramv[0]:

depv[3]:
0: Private block
1: Leftin block (provided by my left neighbor)
2: Rightin block (provided by my right neighbor)
*/
    u64 i, phase, phasenext;
    ocrGuid_t stencilGUID;
    ocrGuid_t sticky;

    stencilDepv_t * stencilDEPV = (stencilDepv_t *) depv;


//unpack depv pointers
    private_t * pbPTR = DEPV(private,ptr);
    buffer_t * leftinPTR = DEPV(leftin,ptr);
    buffer_t * rightinPTR = DEPV(rightin,ptr);

//unpack private parameters
    u64 myrank = pbPTR->myrank;
    u64 timestep = pbPTR->timestep;
    u64 maxt = pbPTR->maxt;
    u64 npoints = pbPTR->npoints;
    u64 nrank = pbPTR->nrank;

    ocrGuid_t leftinEVT = NULL_GUID;
    ocrGuid_t rightinEVT = NULL_GUID;

    if(pbPTR->oldLeftRecvEVT != NULL_GUID) ocrEventDestroy(pbPTR->oldLeftRecvEVT);
    if(pbPTR->oldRightRecvEVT!= NULL_GUID) ocrEventDestroy(pbPTR->oldRightRecvEVT);

    double * a = (double *) (((u64) pbPTR) + pbPTR->dataOffset);


//if first timestep initialize
    if(timestep == 0) {
        if(npoints==1) {  //special code for single data point
            if(leftinPTR == NULL || rightinPTR == NULL) a[0] = 1;
              else a[0] = 0;
            if(leftinPTR != NULL) {
                if(myrank == 1) leftinPTR->buffer = 1.0;
                  else leftinPTR->buffer= 0.0;
            }
            if(rightinPTR != NULL) {
                if(myrank == nrank-2) rightinPTR->buffer = 1.0;
                  else rightinPTR->buffer= 0.0;
            }
          } else {
            if(leftinPTR == NULL) a[0] = 1;
              else {
                a[0] = 0.0;
                leftinPTR->buffer = 0.0;
            }

            for(i=1;i<npoints-1;i++) a[i] = 0.0;

            if(rightinPTR == NULL) a[npoints-1] = 1;
              else {
                a[npoints-1] = 0;
                rightinPTR->buffer = 0;
            }
        }
    }

    double asave;

//save last value for serializing send right.
    if(timestep == maxt && myrank != nrank-1) asave = a[npoints-1];


//compute
    double aleft, acenter;

        if(npoints==1) {  //special code for single data point
            if(leftinPTR == NULL || rightinPTR == NULL) acenter = a[0];
              else acenter = 0.5*a[0] + 0.25*(leftinPTR->buffer + rightinPTR->buffer);
            a[0] = acenter;
          } else {
            if(leftinPTR != NULL) aleft =  .5*a[0] + .25*(a[1] + leftinPTR->buffer);
                else aleft = a[0];
            for(i=1;i<npoints-1;i++) {
                acenter =  0.5*a[i] + 0.25*(a[i+1] + a[i-1]);
                a[i-1] = aleft;
                aleft = acenter;
            }
            if(rightinPTR != NULL) acenter = 0.5* a[npoints-1] + .25*(rightinPTR->buffer + a[npoints-2]);
                else acenter = a[npoints-1];
            a[npoints-2] = aleft;
            a[npoints-1] = acenter;
        }

//send left
    if(timestep < maxt) {
        if(leftinPTR != NULL) {
            leftinEVT = leftinPTR->EVT;
            leftinPTR->buffer = a[0];
//no new event for timestep==maxt-1
            if(timestep<maxt-1) ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
            leftinPTR->EVT = sticky;
            ocrDbRelease(DEPV(leftin,guid));
            ocrEventSatisfy(pbPTR->leftSendEVT, DEPV(leftin,guid));
            pbPTR->leftSendEVT = sticky;
        }

//send right
        if(rightinPTR != NULL) {
            rightinEVT = rightinPTR->EVT;
            rightinPTR->buffer = a[npoints-1];
//defer sending right when timestep == maxt-1
            if(timestep<maxt-1) {
                ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
                rightinPTR->EVT = sticky;
                ocrDbRelease(DEPV(rightin,guid));
                ocrEventSatisfy(pbPTR->rightSendEVT , DEPV(rightin,guid));
                pbPTR->rightSendEVT  = sticky;
            }
        }
    }
    else {
//last time step, serialized using send right.
        if(myrank < nrank-1) {
            for(i=0;i<npoints;i++) PRINTF("S%ld i%d valu %f \n", myrank, i, a[i]);
                rightinPTR->buffer = asave;
                ocrDbRelease(DEPV(rightin,guid));
                ocrEventSatisfy(pbPTR->rightSendEVT,DEPV(rightin,guid));
            } else {
                for(i=0;i<npoints;i++) PRINTF("S%ld i%d valu %f \n", myrank, i, a[i]);
                ocrShutdown();
            }
        return NULL_GUID;
        }


    pbPTR->timestep++;
    pbPTR->oldLeftRecvEVT = leftinEVT;
    pbPTR->oldRightRecvEVT = rightinEVT;


//create clone

    ocrEdtCreate(&stencilGUID, pbPTR->stencilTML, EDT_PARAM_DEF, NULL,
                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

    ocrDbRelease(DEPV(private,guid));
    ocrAddDependence(DEPV(private,guid), stencilGUID, SLOT(private), DB_MODE_RW);
    ocrAddDependence(leftinEVT, stencilGUID, SLOT(leftin), DB_MODE_RW);
    ocrAddDependence(rightinEVT, stencilGUID, SLOT(rightin), DB_MODE_RW);

    return NULL_GUID;
}


typedef struct {
   u64 nrank;
   u64 npoints;
   u64 maxt;
} realMainParam_t;

ocrGuid_t realMainEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
creates the first set of sticky events
disperses them amond the buffer blocks
creates nranks stencilEdts
passes in the main datablock and the correct set of 2 buffer blocks (or NULL_GUID)

paramv[3]:
0: nrank
1: npoints
2: maxt

dependencies:
0 - (nrank-1) the nrank private datablocks
N - (3*nrank-3): the (2*nrank-2) buffer datablocks

*/

    realMainParam_t * paramPTR = (realMainParam_t *) paramv;

    u64 nrank = paramPTR->nrank;
    u64 npoints = paramPTR->npoints;
    u64 maxt = paramPTR->maxt;

    u64 i, myrank;

    ocrGuid_t stencilTML, stencilGUID, oldStencilGUID;
    ocrEdtTemplateCreate(&stencilTML, stencilEDT, 0, 3);


    u64 *dummy;
    u64 j;
    ocrGuid_t rightSendEVT;
    ocrGuid_t leftSendEVT;
    buffer_t * bufferPTR;

    private_t * pbPTR = depv[0].ptr;
    pbPTR->nrank = nrank;
    pbPTR->npoints = npoints;
    pbPTR->maxt = maxt;
    pbPTR->myrank = 0;
    pbPTR->timestep = 0;
    pbPTR->dataOffset = sizeof(private_t);
    pbPTR->stencilTML = stencilTML;
    pbPTR->oldLeftRecvEVT = NULL_GUID;
    pbPTR->leftSendEVT = NULL_GUID;

    ocrEdtCreate(&stencilGUID, stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(NULL_GUID, stencilGUID, 1, DB_MODE_RW);

//index counts through the buffer blocks
//i counts through the ranks
//each sticky event gets put in two places
//   the private block of the sender (in send{left,right}EVT)
//   in the appropriate buffer block of the receiver
//
    u32 index = nrank;

    for(i=0;i<nrank-1;i++){
        ocrEventCreate(&rightSendEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        bufferPTR = depv[index++].ptr;
        bufferPTR->EVT = rightSendEVT;
        pbPTR->rightSendEVT = rightSendEVT;
        pbPTR->oldRightRecvEVT = NULL_GUID;
//launch worker i
        ocrDbRelease(depv[i].guid);
        ocrAddDependence(depv[i].guid, stencilGUID, 0, DB_MODE_RW);

//create worker i+1
        oldStencilGUID = stencilGUID;
        ocrEdtCreate(&stencilGUID, stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        pbPTR = depv[i+1].ptr;
        pbPTR->nrank = nrank;
        pbPTR->npoints = npoints;
        pbPTR->maxt = maxt;
        pbPTR->myrank = i+1;
        pbPTR->timestep = 0;
        pbPTR->dataOffset = sizeof(private_t);
        pbPTR->stencilTML = stencilTML;
        pbPTR->oldLeftRecvEVT = NULL_GUID;

        ocrEventCreate(&leftSendEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        pbPTR->leftSendEVT = leftSendEVT;
        pbPTR->oldLeftRecvEVT = NULL_GUID;
        bufferPTR = depv[index++].ptr;
        bufferPTR->EVT = leftSendEVT;

        ocrDbRelease(depv[index-1].guid);
        ocrAddDependence(depv[index-1].guid, oldStencilGUID, 2, DB_MODE_RW);
        ocrDbRelease(depv[index-2].guid);
        ocrAddDependence(depv[index-2].guid, stencilGUID, 1, DB_MODE_RW);
    }

    pbPTR->oldRightRecvEVT = NULL_GUID;
    ocrDbRelease(depv[nrank-1].guid);
    ocrAddDependence(depv[nrank-1].guid, stencilGUID, 0, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, stencilGUID, 2, DB_MODE_RW);

    return NULL_GUID;
}



ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

/*
mainEdt is executed first
Creates the datablocks
creates and launches realMain
*/

    u64 i, nrank, npoints, maxt;


    void * programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);
    if(argc != 4) {
        PRINTF("using default runtime args\n");
        nrank = 4;
        npoints = 10;
        maxt = 100;
    } else {
        i = 1;
        nrank = (u32) atoi(getArgv(programArgv, i++));
        npoints = (u32) atoi(getArgv(programArgv, i++));
        maxt = (u32) atoi(getArgv(programArgv, i++));
    }


    PRINTF("1D stencil code STICKY style: \n");
    PRINTF("number of workers = %ld \n", nrank);
    PRINTF("data on each worker = %ld \n", npoints);
    PRINTF("number of timesteps = %ld \n", maxt);
    if(nrank == 0 || npoints == 0 || maxt == 0) {
        PRINTF("nrank, npoints, maxt, must all be positive\n");
        ocrShutdown();
        return NULL_GUID;
    }

    u64 *dummy;
    ocrGuid_t realMainGUID, realMainTML;


    ocrEdtTemplateCreate(&realMainTML, realMainEDT, 3, 3*nrank-2);
    u64 paramvout[3] = {nrank, npoints, maxt};
    ocrEdtCreate(&realMainGUID, realMainTML, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL,
          EDT_PROP_NONE, NULL_GUID, NULL);

    ocrGuid_t privateDBK, bufferDBK;

    for(i=0;i<nrank;i++)  {
        ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t) + npoints*sizeof(double), 0, NULL_GUID, NO_ALLOC);
        ocrAddDependence(privateDBK, realMainGUID, i, DB_MODE_RW);
    }
    for(i=0;i<2*nrank-2;i++)  {
        ocrDbCreate(&bufferDBK, (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
        ocrAddDependence(bufferDBK, realMainGUID, nrank+i, DB_MODE_RW);
    }

    return NULL_GUID;
}
