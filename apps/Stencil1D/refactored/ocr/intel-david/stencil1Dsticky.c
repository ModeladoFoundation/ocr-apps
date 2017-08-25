/*
Author: David Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
See README for more documentation

There are 6 versions of a 1D stencil computation (3 points wide).

This code uses STICKY events with serial initialization

nrank is the number of tasks
npoints is the number of points in each task
maxt is the number of timesteps

the datapoints are initialized to zero except the global boundary values which are set to 1
the particular stencil implemented is anew(i) = .5*(a(i) + 0.25*(a(i+1) + a(i-1))
The values converge (slowly) to all 1s.

if the stencil is wider than 3 points, then the number of shared boundary values
would have to be increased.
*/

#include <ocr.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PARALLEL
#define ENABLE_EXTENSION_LABELING
#include<extensions/ocr-labeling.h>
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
typedef struct{
    u64 nrank;
    u64 npoints;
    u64 maxt;
    ocrGuid_t startDirs[2];
} shared_t;
#endif


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

//this structure is used to unpack the dependencies of stencilEDT
typedef struct {
   ocrEdtDep_t leftIn;
   ocrEdtDep_t private;
   ocrEdtDep_t rightIn;
} stencilDEPV_t;


//this macro is used to refer to dependencies
#define DEPV(name,var,field) (name##DEPV->var).field
//this macro computes the slot number (for ocrAddDependence)
#define SLOT(name,var) (offsetof(name##DEPV_t,var)/sizeof(ocrEdtDep_t))


ocrGuid_t stencilEDT(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
paramv[0]:

depv[3]:
0: Leftin block (provided by my left neighbor)
1: Private block
2: Rightin block (provided by my right neighbor)
*/
    u64 i;
    ocrGuid_t stencilGUID;
    ocrGuid_t sticky;

    stencilDEPV_t * stencilDEPV = (stencilDEPV_t *) depv;


//unpack depv pointers
    private_t * pbPTR = DEPV(stencil,private,ptr);
    buffer_t * leftInPTR = DEPV(stencil,leftIn,ptr);
    buffer_t * rightInPTR = DEPV(stencil,rightIn,ptr);

//unpack private parameters
    u64 myrank = pbPTR->myrank;
    u64 timestep = pbPTR->timestep;
    u64 maxt = pbPTR->maxt;
    u64 npoints = pbPTR->npoints;
    u64 nrank = pbPTR->nrank;


    ocrGuid_t leftInEVT = NULL_GUID;
    ocrGuid_t rightInEVT = NULL_GUID;

//destroy old events (if any)
    if( !ocrGuidIsNull( pbPTR->oldLeftRecvEVT ) ) ocrEventDestroy(pbPTR->oldLeftRecvEVT);
    if( !ocrGuidIsNull( pbPTR->oldRightRecvEVT ) ) ocrEventDestroy(pbPTR->oldRightRecvEVT);

//pointer to the "real" data
    double * a = (double *) (((u64) pbPTR) + pbPTR->dataOffset);



//if first timestep initialize
    if(timestep == 0) {
        if(npoints==1) {  //special code for single data point
            if(leftInPTR == NULL || rightInPTR == NULL) a[0] = 1;
              else a[0] = 0;
            if(leftInPTR != NULL) {
                if(myrank == 1) leftInPTR->buffer = 1.0;
                  else leftInPTR->buffer= 0.0;
            }
            if(rightInPTR != NULL) {
                if(myrank == nrank-2) rightInPTR->buffer = 1.0;
                  else rightInPTR->buffer= 0.0;
            }
          } else {
            if(leftInPTR == NULL) a[0] = 1;
              else {
                a[0] = 0.0;
                leftInPTR->buffer = 0.0;
            }

            for(i=1;i<npoints-1;i++) a[i] = 0.0;

            if(rightInPTR == NULL) a[npoints-1] = 1;
              else {
                a[npoints-1] = 0;
                rightInPTR->buffer = 0;
            }
        }
    }

    double asave;

//save last value for serializing send right.
    if(timestep == maxt && myrank != nrank-1) asave = a[npoints-1];


//compute using temporaries to avoid doubling the required storage

    double aleft, acenter;

        if(npoints==1) {  //special code for single data point
            if(leftInPTR == NULL || rightInPTR == NULL) acenter = a[0];
              else acenter = 0.5*a[0] + 0.25*(leftInPTR->buffer + rightInPTR->buffer);
            a[0] = acenter;
          } else {//more than one datapoint
            if(leftInPTR != NULL) aleft =  .5*a[0] + .25*(a[1] + leftInPTR->buffer);
                else aleft = a[0];
            for(i=1;i<npoints-1;i++) {
                acenter =  0.5*a[i] + 0.25*(a[i+1] + a[i-1]);
                a[i-1] = aleft;
                aleft = acenter;
            }
            if(rightInPTR != NULL) acenter = 0.5* a[npoints-1] + .25*(rightInPTR->buffer + a[npoints-2]);
                else acenter = a[npoints-1];
            a[npoints-2] = aleft;
            a[npoints-1] = acenter;
        }

//send left
    if(timestep < maxt) {
        if(leftInPTR != NULL) {
            leftInEVT = leftInPTR->EVT;
            leftInPTR->buffer = a[0];

//no new event for timestep==maxt-1
            if(timestep<maxt-1) ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
            leftInPTR->EVT = sticky;
            ocrDbRelease(DEPV(stencil,leftIn,guid));
            ocrEventSatisfy(pbPTR->leftSendEVT, DEPV(stencil,leftIn,guid));
            pbPTR->leftSendEVT = sticky;
        }

//send right
        if(rightInPTR != NULL) {
            rightInEVT = rightInPTR->EVT;
            rightInPTR->buffer = a[npoints-1];

//defer sending right when timestep == maxt-1
            if(timestep<maxt-1) {
                ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
                rightInPTR->EVT = sticky;
                ocrDbRelease(DEPV(stencil,rightIn,guid));
                ocrEventSatisfy(pbPTR->rightSendEVT , DEPV(stencil,rightIn,guid));
                pbPTR->rightSendEVT  = sticky;
            }
        }
    }
    else {
//last time step, serialized using send right.
        if(myrank < nrank-1) {
            for(i=0;i<npoints;i++) ocrPrintf("S%ld i%d valu %f \n", myrank, i, a[i]);
                rightInPTR->buffer = asave;
                ocrDbRelease(DEPV(stencil,rightIn,guid));
                ocrEventSatisfy(pbPTR->rightSendEVT,DEPV(stencil,rightIn,guid));
            } else {
                for(i=0;i<npoints;i++) ocrPrintf("S%ld i%d valu %f \n", myrank, i, a[i]);
                ocrShutdown();
            }
        return NULL_GUID;
        }


    pbPTR->timestep++;
    pbPTR->oldLeftRecvEVT = leftInEVT;
    pbPTR->oldRightRecvEVT = rightInEVT;


//create clone

    ocrEdtCreate(&stencilGUID, pbPTR->stencilTML, EDT_PARAM_DEF, NULL,
                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    ocrAddDependence(leftInEVT, stencilGUID, SLOT(stencil,leftIn), DB_MODE_RW);
    ocrDbRelease(DEPV(stencil,private,guid));
    ocrAddDependence(DEPV(stencil,private,guid), stencilGUID, SLOT(stencil,private), DB_MODE_RW);
    ocrAddDependence(rightInEVT, stencilGUID, SLOT(stencil,rightIn), DB_MODE_RW);

    return NULL_GUID;
}


//used to unpack the parameters of realMainEDT
typedef struct {
   u64 nrank;
   u64 npoints;
   u64 maxt;
} realMainPRM_t;

#ifndef PARALLEL
ocrGuid_t realMainEDT(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
creates the first set of sticky events
disperses them among the buffer blocks and private blocks
creates nrank stencilEDTs
passes in the the private block and the correct set of 2 buffer blocks (or NULL_GUID)

paramv[3]:
0: nrank
1: npoints
2: maxt

dependencies:
0 - (nrank-1) the nrank private datablocks
N - (3*nrank-3): the (2*nrank-2) buffer datablocks

*/

    realMainPRM_t * paramPTR = (realMainPRM_t *) paramv;

    u64 nrank = paramPTR->nrank;
    u64 npoints = paramPTR->npoints;
    u64 maxt = paramPTR->maxt;

    u64 i;

    ocrGuid_t stencilTML, stencilGUID, leftStencilGUID;
    ocrEdtTemplateCreate(&stencilTML, stencilEDT, 0, 3);


    ocrGuid_t rightSendEVT;
    ocrGuid_t leftSendEVT;
    buffer_t * leftBufferPTR, * rightBufferPTR;

    u64 lslot = 0;
    u64 pslot = 1;
    u64 rslot = 2;

    ocrEdtCreate(&stencilGUID, stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(depv[lslot].guid, stencilGUID, 0, DB_MODE_RW); //NULL_GUID
    lslot += 3;

    private_t * pbPTR = depv[pslot].ptr;
    pbPTR->nrank = nrank;
    pbPTR->npoints = npoints;
    pbPTR->maxt = maxt;
    pbPTR->myrank = 0;
    pbPTR->timestep = 0;
    pbPTR->dataOffset = sizeof(private_t);
    pbPTR->stencilTML = stencilTML;
    pbPTR->oldLeftRecvEVT = NULL_GUID;
    pbPTR->leftSendEVT = NULL_GUID;

//i counts through the ranks
//lslot, pslot, and rslot count through the datablocks
//each sticky event gets put in two places
//   the private block of the sender (in send{left,right}EVT)
//   in the appropriate buffer block of the receiver
//data values in the buffer blocks are initialized in the first stencilEDT

    for(i=0;i<nrank-1;i++){
        ocrEventCreate(&rightSendEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        rightBufferPTR = depv[rslot].ptr;
        rightBufferPTR->EVT = rightSendEVT;

        pbPTR->rightSendEVT = rightSendEVT;
        pbPTR->oldRightRecvEVT = NULL_GUID;

        ocrDbRelease(depv[pslot].guid);
        ocrAddDependence(depv[pslot].guid, stencilGUID, 1, DB_MODE_RW);
        pslot += 3;

//create worker i+1

        leftStencilGUID = stencilGUID;
        ocrEdtCreate(&stencilGUID, stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrDbRelease(depv[rslot].guid);
        ocrAddDependence(depv[rslot].guid, stencilGUID, 0, DB_MODE_RW);
        rslot +=3;

        ocrEventCreate(&leftSendEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
        leftBufferPTR = depv[lslot].ptr;
        leftBufferPTR->EVT = leftSendEVT;

        ocrDbRelease(depv[lslot].guid);
        ocrAddDependence(depv[lslot].guid, leftStencilGUID, 2, DB_MODE_RW);
        lslot += 3;

        pbPTR = depv[pslot].ptr;
        pbPTR->leftSendEVT = leftSendEVT;
        pbPTR->oldLeftRecvEVT = NULL_GUID;
        pbPTR->nrank = nrank;
        pbPTR->npoints = npoints;
        pbPTR->maxt = maxt;
        pbPTR->myrank = i+1;
        pbPTR->timestep = 0;
        pbPTR->dataOffset = sizeof(private_t);
        pbPTR->stencilTML = stencilTML;
        pbPTR->oldLeftRecvEVT = NULL_GUID;
    }

    pbPTR->rightSendEVT = NULL_GUID;
    pbPTR->oldRightRecvEVT = NULL_GUID;

    ocrDbRelease(depv[pslot].guid);
    ocrAddDependence(depv[pslot].guid, stencilGUID, 1, DB_MODE_RW);

    ocrAddDependence(depv[rslot].guid, stencilGUID, 2, DB_MODE_RW); //NULL_GUID

    return NULL_GUID;
}
#endif


#ifdef PARALLEL
ocrGuid_t stencilInitEDT( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ){

    /* paramv[1]:
     *  0: myrank
     *
     * depv[1]:
     *  0: shared block
     *  1: empty private block
     *  2: empty left block
     *  3: empty right block
     *
     *  - initialize private block. This is the last place we'll use the shared
     *      block.
     *  - initialize the set of labeled GUIDs to start the steady state.
     */

    ocrGuid_t stencilTML, leftSendEVT, rightSendEVT, stencilGUID;

    shared_t * sharedPTR    = depv[0].ptr;

    private_t * privPTR     = depv[1].ptr;
    ocrGuid_t privGUID       = depv[1].guid;

    buffer_t * leftPTR      = depv[2].ptr;
    ocrGuid_t leftGUID      = depv[2].guid;

    buffer_t * rightPTR     = depv[3].ptr;
    ocrGuid_t rightGUID     = depv[3].guid;

    ocrEdtTemplateCreate(&stencilTML, stencilEDT, 0, 3);

    privPTR->nrank = paramv[1];
    privPTR->npoints = sharedPTR->npoints;
    privPTR->maxt = sharedPTR->maxt;            //our need for shared is over until we index our start evts.
    privPTR->timestep = 0;
    privPTR->myrank = paramv[0];
    privPTR->dataOffset = sizeof(private_t);




    //set leftSend EVT.
    ocrGuidFromIndex( &leftSendEVT, sharedPTR->startDirs[0], privPTR->myrank );
    ocrEventCreate( &leftSendEVT,  OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
    privPTR->leftSendEVT = leftSendEVT;

    //set rightSend EVT.
    ocrGuidFromIndex( &rightSendEVT, sharedPTR->startDirs[1], privPTR->myrank );
    ocrEventCreate( &rightSendEVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
    privPTR->rightSendEVT = rightSendEVT;

    privPTR->oldLeftRecvEVT = NULL_GUID;
    privPTR->oldRightRecvEVT = NULL_GUID;
    privPTR->stencilTML = stencilTML;

    ocrEdtCreate( &stencilGUID, stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
        EDT_PROP_NONE, NULL_HINT, NULL );

    //set event to satisfy for leftblock;
    if( privPTR->myrank != 0 ){
        ocrGuid_t leftinEVT;
        ocrGuidFromIndex( &leftPTR->EVT, sharedPTR->startDirs[1], privPTR->myrank - 1 );
        ocrEventCreate( &leftPTR->EVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrDbRelease( leftGUID );
        ocrAddDependence( leftGUID, stencilGUID, 0, DB_MODE_RW );
    }else{
        ocrAddDependence( NULL_GUID, stencilGUID, 0, DB_MODE_RW );
    }


    if( privPTR->myrank < privPTR->nrank - 1 ){
        //ocrPrintf("MYRANK: %d %d\n", privPTR->myrank, privPTR->nrank);
        ocrGuidFromIndex( &rightPTR->EVT, sharedPTR->startDirs[0], privPTR->myrank + 1 );
        ocrEventCreate( &rightPTR->EVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrDbRelease( rightGUID );
        ocrAddDependence( rightGUID, stencilGUID, 2, DB_MODE_RW );
    }else{
        ocrAddDependence( NULL_GUID, stencilGUID, 2, DB_MODE_RW );
    }
    //set event to satisfy for rightblock;

    ocrDbRelease( privGUID );
    ocrAddDependence( privGUID, stencilGUID, 1, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t parallelInitEDT( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ){

    /* paramv[1]:
     *
     *  0: myrank
     *
     * depv[1]:
     * 0: populated shared block
     *
     *  -create datablocks needed by steady state of this rank.
     */

    shared_t * sharedPTR = depv[0].ptr;
    ocrGuid_t sharedGUID = depv[0].guid;

    double* dummy;

    u64 npoints = sharedPTR->npoints;



    ocrGuid_t privateDBK, stencilInitTML, stencilInitGUID, leftDBK, rightDBK;

    ocrEdtTemplateCreate( &stencilInitTML, stencilInitEDT, 2, 4 );

    ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t) + npoints*sizeof(double), 0, NULL_HINT, NO_ALLOC);
    if( paramv[0] != 0 )ocrDbCreate(&leftDBK, (void **) &dummy, sizeof(buffer_t), 0, NULL_HINT, NO_ALLOC );
    if( paramv[0] != sharedPTR->nrank - 1 )ocrDbCreate(&rightDBK, (void **) &dummy, sizeof(buffer_t), 0, NULL_HINT, NO_ALLOC );

    ocrEdtCreate( &stencilInitGUID, stencilInitTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
            EDT_PROP_NONE, NULL_HINT, NULL );


    ocrDbRelease( sharedGUID );
    ocrAddDependence( sharedGUID, stencilInitGUID, 0, DB_MODE_RO );
    ocrAddDependence( privateDBK, stencilInitGUID, 1, DB_MODE_RW );
    if( paramv[0] != 0 ){
        ocrDbRelease( leftDBK );
        ocrAddDependence( leftDBK, stencilInitGUID, 2, DB_MODE_RW );
    }else{
        ocrAddDependence( NULL_GUID, stencilInitGUID, 2, DB_MODE_RW );
    }
    if( paramv[0] != sharedPTR->nrank - 1 ){
        ocrDbRelease( rightDBK );
        ocrAddDependence( rightDBK, stencilInitGUID, 3, DB_MODE_RW );
    }else{
        ocrAddDependence( NULL_GUID, stencilInitGUID, 3, DB_MODE_RW );
    }
    return NULL_GUID;
}

ocrGuid_t realMainEDT( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ){

    /* paramv[1]:
     *  0: myrank
     *
     * depv[1]:
     *  0: empty shared block
     *
     *  - get the shared block information all set up for use in the parallel initialization.
     *  - this includes requesting the guid rangest to start the steady state.
     */

    realMainPRM_t * paramPTR = (realMainPRM_t *) paramv;

    u64 nrank = paramPTR->nrank;
    u64 npoints = paramPTR->npoints;
    u64 maxt = paramPTR->maxt;
    u64 i;
    ocrGuid_t parallelInitGUID, parallelInitTML;

    shared_t * sharedPTR = depv[0].ptr;
    ocrGuid_t sharedGUID = depv[0].guid;



    sharedPTR->nrank = nrank;
    sharedPTR->npoints = npoints;
    sharedPTR->maxt = maxt;

    ocrGuidRangeCreate( &(sharedPTR->startDirs[0]), sharedPTR->nrank, GUID_USER_EVENT_STICKY );
    ocrGuidRangeCreate( &(sharedPTR->startDirs[1]), sharedPTR->nrank, GUID_USER_EVENT_STICKY );

    ocrEdtTemplateCreate( &parallelInitTML, parallelInitEDT, 2, 1 );

    ocrDbRelease( sharedGUID );

    u64 params[2];
    params[1] = nrank;

    for( i = 0; i < nrank; i++ ){
        params[0] = i;
        ocrEdtCreate( &parallelInitGUID, parallelInitTML, EDT_PARAM_DEF, params, EDT_PARAM_DEF, NULL,
                EDT_PROP_NONE, NULL_HINT, NULL );

        ocrAddDependence( sharedGUID, parallelInitGUID, 0, DB_MODE_RO );
    }

    return NULL_GUID;
}
#endif

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

/*
mainEdt is executed first
Creates the datablocks
creates and launches realMain
*/

    u64 i, nrank, npoints, maxt;


    void * programArgv = depv[0].ptr;
    u32 argc = ocrGetArgc(programArgv);
    if(argc != 4) {
        ocrPrintf("using default runtime args\n");
        nrank = 4;
        npoints = 10;
        maxt = 100;
    } else {
        i = 1;
        nrank = (u32) atoi(ocrGetArgv(programArgv, i++));
        npoints = (u32) atoi(ocrGetArgv(programArgv, i++));
        maxt = (u32) atoi(ocrGetArgv(programArgv, i++));
    }

    #ifndef PARALLEL
    ocrPrintf("1D stencil code STICKY style: \n");
    #endif

    #ifdef PARALLEL
    ocrPrintf("1D stencil code STICKY style, parallel init: \n");
    #endif

    ocrPrintf("number of workers = %ld \n", nrank);
    ocrPrintf("data on each worker = %ld \n", npoints);
    ocrPrintf("number of timesteps = %ld \n", maxt);
    if(nrank == 0 || npoints == 0 || maxt == 0) {
        ocrPrintf("nrank, npoints, maxt, must all be positive\n");
        ocrShutdown();
        return NULL_GUID;
    }

    u64 *dummy;
    ocrGuid_t realMainGUID, realMainTML;

    realMainPRM_t realMainPRM;

    #ifndef PARALLEL //serial init
    ocrEdtTemplateCreate(&realMainTML, realMainEDT, sizeof(realMainPRM_t)/sizeof(u64), 3*nrank);
    realMainPRM.nrank = nrank;
    realMainPRM.npoints = npoints;
    realMainPRM.maxt = maxt;
    ocrEdtCreate(&realMainGUID, realMainTML, EDT_PARAM_DEF, (u64 *) &realMainPRM, EDT_PARAM_DEF, NULL,
          EDT_PROP_NONE, NULL_GUID, NULL);

    ocrGuid_t privateDBK, bufferDBK;

    u64 slot = 0;
    ocrAddDependence(NULL_GUID, realMainGUID, slot++, DB_MODE_RW);

    for(i=0;i<nrank-1;i++)  {

        ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t) + npoints*sizeof(double), 0, NULL_GUID, NO_ALLOC);
        ocrAddDependence(privateDBK, realMainGUID, slot++, DB_MODE_RW);

        ocrDbCreate(&bufferDBK, (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
        ocrAddDependence(bufferDBK, realMainGUID, slot++, DB_MODE_RW);
        ocrDbCreate(&bufferDBK, (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
        ocrAddDependence(bufferDBK, realMainGUID, slot++, DB_MODE_RW);
    }
    ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t) + npoints*sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(privateDBK, realMainGUID, slot++, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, realMainGUID, slot, DB_MODE_RW);
    #endif


    #ifdef PARALLEL //parallel init
    ocrEdtTemplateCreate( &realMainTML, realMainEDT, sizeof(realMainPRM_t)/sizeof(u64), 1 );
    realMainPRM.nrank = nrank;
    realMainPRM.npoints = npoints;
    realMainPRM.maxt = maxt;

    ocrGuid_t sharedDBK;

    ocrEdtCreate( &realMainGUID, realMainTML, EDT_PARAM_DEF, (u64 *) &realMainPRM, EDT_PARAM_DEF, NULL,
            EDT_PROP_NONE, NULL_HINT, NULL );

    ocrDbCreate( &sharedDBK, (void **)&dummy, sizeof(shared_t), 0, NULL_HINT, NO_ALLOC );

    ocrAddDependence( sharedDBK, realMainGUID, 0, DB_MODE_RW );
    #endif



    return NULL_GUID;
}
