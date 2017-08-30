#include <ocr.h>
#include "../src/ocrCpp.h"
#include <iostream>

#ifdef ENABLE_EXTENSION_CHANNEL_EVT
#include <extensions/ocr-labeling.h>

#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

//this macro is used to refer to dependencies
#define DEPV(name,var,field) (name##DEPV->var).field
//this macro computes the slot number (for ocrAddDependence)
#define SLOT(name,var) (offsetof(name##DEPV_t,var)/sizeof(ocrEdtDep_t))


using namespace std;

typedef struct {
    u64 nrank;
    u64 npoints;
    u64 maxt;
} realMainPRM_t;

typedef struct{
    u64 nrank;
    u64 npoints;
    u64 maxt;
    ocrGuid_t startDirs[2];
} shared_t;

typedef struct{
    double buffer;
    ocrGuid_t EVT;
} buffer_t;

typedef struct{
    u32 nrank;
    u32 npoints;
    u32 maxt;
    u32 timestep;
    u32 myrank;
    u64 dataOffset;
    ocrGuid_t leftSendEVT;      //channelEVTs go here
    ocrGuid_t rightSendEVT;
    ocrGuid_t leftRcvEVT;
    ocrGuid_t rightRcvEVT;      //end channelEVTs
    EdtProducer producer;
} private_t;

ocrGuid_t stencilReportEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrEdtDep_t priv = depv[0];
    private_t * privPTR = (private_t *)priv.ptr;

    double * a = (double *)(((u64)privPTR) + privPTR->dataOffset);

    u64 i = 0;

    //ocrPrintf("Rank %ld\n", privPTR->myrank);

    for(i=0;i<privPTR->npoints;i++) ocrPrintf("S%ld i%d valu %f \n", privPTR->myrank, i, a[i]);

    if( privPTR->myrank < privPTR->nrank - 1 )
        ocrEventSatisfy( privPTR->rightSendEVT, NULL_GUID );

    return NULL_GUID;
}


ocrGuid_t stencilEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
     ocrEdtDep_t priv = depv[0];
    private_t * privPTR = (private_t *)priv.ptr;
    buffer_t *lBuff, *rBuff;
    u64 myrank = privPTR->myrank;
    u64 nrank = privPTR->nrank;
    u64 npoints = privPTR->npoints;
    u64 maxt = privPTR->maxt;
    u64 timestep = privPTR->timestep;
    buffer_t * leftInPTR = (buffer_t *)depv[1].ptr;
    buffer_t * rightInPTR = (buffer_t *)depv[2].ptr;

            u64 i;


    if( timestep == 0 ){ //link up the channels
        if( myrank > 0 ){
            lBuff = (buffer_t *)depv[1].ptr;
            #ifdef OWN_SEND
            privPTR->leftRcvEVT = lBuff->EVT;
            #else
            privPTR->leftSendEVT = lBuff->EVT;
            #endif
        } else {
            privPTR->leftRcvEVT = NULL_GUID;
        }
        if( myrank < nrank-1 ){
            rBuff = (buffer_t *)depv[2].ptr;
            #ifdef OWN_SEND
            privPTR->rightRcvEVT = rBuff->EVT;
            #else
            privPTR->rightSendEVT = rBuff->EVT;
            #endif
        } else {
            privPTR->rightRcvEVT = NULL_GUID;
        }
    }

    ocrGuid_t   leftRcvEVT = privPTR->leftRcvEVT,
                rightRcvEVT = privPTR->rightRcvEVT,
                leftSendEVT = privPTR->leftSendEVT,
                rightSendEVT = privPTR->rightSendEVT;

    /*--------------------COMPUTE--------------------*/

    double * a = (double *)(((u64) privPTR) + privPTR->dataOffset);

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

    /*------------------END COMPUTE------------------*/

    /*-------------------OCR API CALLS---------------*/
    if( privPTR->timestep < privPTR->maxt )
    {

        if( leftInPTR != NULL ) leftInPTR->buffer = a[0];
        if( rightInPTR != NULL ) rightInPTR->buffer = a[npoints-1];

        timestep = privPTR->timestep;
        privPTR->timestep++;
        ocrGuid_t stencilEdtGUID;

        EdtLauncher launch( privPTR->producer, privPTR->producer.create() );

        ocrGuid_t   leftRcvEVT = privPTR->leftRcvEVT,
                    rightRcvEVT = privPTR->rightRcvEVT;

        if(timestep != privPTR->maxt){
            if( myrank > 0 )
            {
                ocrDbRelease( depv[1].guid );
                ocrEventSatisfy( leftSendEVT, depv[1].guid );
            }
            if( myrank < nrank-1 ) {
                ocrDbRelease( depv[2].guid );
                ocrEventSatisfy( rightSendEVT, depv[2].guid );
            }
        }

        ocrDbRelease( priv.guid );
        launch.depPush( priv.guid );
        launch.depPush( leftRcvEVT );
        launch.depPush( rightRcvEVT );

    }else{

        EdtProducer prodReport( stencilReportEdt, 0, 2, false );
        EdtLauncher launch( prodReport, prodReport.create() );

        ocrDbRelease( priv.guid );

        launch.depPush( priv.guid );
        launch.depPush( leftRcvEVT );
    }



    return NULL_GUID;

}

ocrGuid_t stencilInitEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*paramv[2]:
     * 0: myrank
     * 1: nrank
     *
     *depv[4]:
     * 0: shared
     * 1: private
     * 2: lBuff
     * 3: rBuff
     */

    //ocrPrintf( "StencilInitEDT %ld\n", paramv[0] );

    u64 _idep = 0;
    u64 myrank = paramv[0];
    u64 nrank = paramv[1];


    ocrEdtDep_t shared = depv[_idep++];
    ocrEdtDep_t priv = depv[_idep++];
    ocrEdtDep_t lBuff = depv[_idep++];
    ocrEdtDep_t rBuff = depv[_idep++];

    shared_t * sharedPTR = (shared_t *)shared.ptr;
    private_t * privPTR = (private_t *)priv.ptr;
    buffer_t * lBuffPTR = (buffer_t *)lBuff.ptr;
    buffer_t * rBuffPTR = (buffer_t *)rBuff.ptr;

    if( ocrGuidIsNull( shared.guid ) ){
        ocrPrintf("%ld null shared ptr\n", paramv[0] );
        return NULL_GUID;
    }

    privPTR->nrank = nrank;
    privPTR->npoints = sharedPTR->npoints;
    privPTR->maxt = sharedPTR->maxt;
    privPTR->timestep = 0;
    privPTR->myrank = myrank;
    privPTR->dataOffset = sizeof( private_t );

    /*---------------------OCR API CALLS--------------------*/

    ocrGuid_t stencilEdtGUID;

    /*------------------CREATE LABELED EVENTs---------------*/
    ocrGuid_t sendLeftEVT = NULL_GUID, sendRightEVT = NULL_GUID;
    ocrGuid_t rcvLeftEVT = NULL_GUID, rcvRightEVT = NULL_GUID;

    if( myrank > 0 ){ //Set the left sending and receiving events.
        ocrGuidFromIndex( &sendLeftEVT, sharedPTR->startDirs[0], myrank );
        ocrGuidFromIndex( &rcvLeftEVT, sharedPTR->startDirs[1], myrank - 1 );
        ocrEventCreate( &sendLeftEVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrEventCreate( &rcvLeftEVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
    }
    if( myrank < nrank - 1 ){ //Set the right sending and receiving events.
        ocrGuidFromIndex( &sendRightEVT, sharedPTR->startDirs[1], myrank );
        ocrGuidFromIndex( &rcvRightEVT, sharedPTR->startDirs[0], myrank + 1 );
        ocrEventCreate( &sendRightEVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrEventCreate( &rcvRightEVT, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
    }
    /*--------------LABELED EVENTS CREATED----------------*/

    /*-------------------CREATE CHANNELS------------------*/
    ocrEventParams_t params;

    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    if( myrank > 0 ){
        #ifdef OWN_SEND
        ocrEventCreateParams( &privPTR->leftSendEVT, OCR_EVENT_CHANNEL_T, false, &params );
        lBuffPTR->EVT = privPTR->leftSendEVT;
        #else
        ocrEventCreateParams( &privPTR->leftRcvEVT, OCR_EVENT_CHANNEL_T, false, &params );
        lBuffPTR->EVT = privPTR->leftRcvEVT;
        #endif
        ocrDbRelease( lBuff.guid );
        ocrEventSatisfy( sendLeftEVT, lBuff.guid );
    }
    if( myrank < nrank - 1 ){
        #ifdef OWN_SEND
        ocrEventCreateParams( &privPTR->rightSendEVT, OCR_EVENT_CHANNEL_T, false, &params );
        rBuffPTR->EVT = privPTR->rightSendEVT;
        #else
        ocrEventCreateParams( &privPTR->rightRcvEVT, OCR_EVENT_CHANNEL_T, false, &params );
        rBuffPTR->EVT = privPTR->rightRcvEVT;
        #endif
        ocrDbRelease( rBuff.guid );
        ocrEventSatisfy( sendRightEVT, rBuff.guid );
    }
    /*------------------CREATED CHANNELS------------------*/

    #ifdef OWN_SEND
    privPTR->leftRcvEVT = NULL_GUID;
    privPTR->rightRcvEVT = NULL_GUID;
    #else
    privPTR->leftSendEVT = NULL_GUID;
    privPTR->rightSendEVT = NULL_GUID;
    #endif


    privPTR->producer = EdtProducer( stencilEdt, 0, 3, false );
    EdtLauncher launch( privPTR->producer, privPTR->producer.create() );

    _idep = 0;

    ocrDbRelease( priv.guid );
    launch.depPush( priv.guid );
    launch.depPush( rcvLeftEVT );
    launch.depPush( rcvRightEVT );

    return NULL_GUID;
}

ocrGuid_t stencilInitDBsEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    ocrGuid_t stencilInitGUID, stencilInitTML;
    ocrGuid_t lBuffDBK = NULL_GUID, rBuffDBK = NULL_GUID, privateDBK;
    double * dummy;
    ocrEdtDep_t shared = depv[0];
    shared_t *sharedPTR = (shared_t *)depv[0].ptr;


    ocrDbCreate( &privateDBK, (void **)&dummy, sizeof( private_t ) + sharedPTR->npoints*sizeof(double),
                    0, NULL_HINT, NO_ALLOC );

    if( paramv[0] > 0 ){
        ocrDbCreate( &lBuffDBK, (void **)&dummy, sizeof( buffer_t ), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( lBuffDBK );
    }

    if( paramv[0] < paramv[1] - 1 ){
        ocrDbCreate( &rBuffDBK, (void **)&dummy, sizeof( buffer_t ), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( rBuffDBK );
    }


    EdtProducer stencilInit( stencilInitEdt, 2, 4, false );
    EdtLauncher launcher( stencilInit, stencilInit.create( paramv ) );

    ocrDbRelease( shared.guid );

    launcher.depPush( shared.guid );
    launcher.depPush( privateDBK );
    launcher.depPush( lBuffDBK );
    launcher.depPush( rBuffDBK );

    return NULL_GUID;
}

ocrGuid_t realMainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u64 i;
    double * dummy;

    ocrPrintf("realMain!\n");

    realMainPRM_t * paramPTR = (realMainPRM_t *) paramv;
    ocrEdtDep_t shared = depv[0];
    shared_t * sharedPTR = (shared_t *)shared.ptr;
    ocrGuid_t sharedGUID = shared.guid;

    u64 nrank = sharedPTR->nrank = paramPTR->nrank;
    u64 npoints = sharedPTR->npoints = paramPTR->npoints;
    u64 maxt = sharedPTR->maxt = paramPTR->maxt;

    /*-----------------------OCR Specific code-------------------*/

    ocrGuid_t stencilInitDBsTML, stencilInitDBsGUID, leftDBK, rightDBK;

    ocrGuidRangeCreate( &(sharedPTR->startDirs[0]), nrank, GUID_USER_EVENT_STICKY );
    ocrGuidRangeCreate( &(sharedPTR->startDirs[1]), nrank, GUID_USER_EVENT_STICKY );

    EdtProducer initDBs( stencilInitDBsEdt, 2, 1, false );

    ocrDbRelease( sharedGUID );

    u64 params[2];
    params[1] = nrank;

    for( i = 0; i < nrank; i++ ){
        params[0] = i;

        EdtLauncher launch( initDBs, initDBs.create( params ) );
        launch.depPush( sharedGUID );
    }
    return NULL_GUID;
}

ocrGuid_t wrapupEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrShutdown( );
    return NULL_GUID;
}


extern "C" ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    u64 i, nrank, npoints, maxt;

    realMainPRM_t realMainPRM;

    void * programArgv = depv[0].ptr;
    u32 argc = ocrGetArgc(programArgv);

    if(argc != 4) {
        ocrPrintf("Using default runtime args\n");
        nrank = 4;
        npoints = 10;
        maxt = 100;
    } else {
        i = 1;
        nrank = (u32) atoi(ocrGetArgv(programArgv, i++));
        npoints = (u32) atoi(ocrGetArgv(programArgv, i++));
        maxt = (u32) atoi(ocrGetArgv(programArgv, i));
    }

    ocrPrintf("1D Stencil code with Channel Events.\n");
    ocrPrintf("number of workers = %ld \n", nrank);
    ocrPrintf("data on each worker = %ld \n", npoints);
    ocrPrintf("number of timesteps = %ld \n", maxt);

    realMainPRM.nrank = nrank;
    realMainPRM.npoints = npoints;
    realMainPRM.maxt = maxt;

/*-----------------------OCR API CALLS-----------------------*/

    ocrGuid_t realMainDBK;

    double * dummy;

    EdtProducer myRealMain( realMainEdt, sizeof(realMainPRM_t)/sizeof(u64), 1, true );
    EdtProducer myWrapup( wrapupEdt, 0, 1, false );

    EdtLauncher realMainLaunch( myRealMain, myRealMain.create( (u64 *)&realMainPRM ) );
    EdtLauncher wrapupLaunch( myWrapup, myWrapup.create( ) );

    wrapupLaunch.depPush( myRealMain.getFinishEVT() );

    ocrDbCreate( &realMainDBK, (void **)&dummy, sizeof( shared_t ), 0, NULL_HINT, NO_ALLOC );
    realMainLaunch.depPush( realMainDBK );

    return NULL_GUID;
}
#else
extern "C" ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrPrintf("Channel event extension not enabled!\n");

    ocrShutdown();
    return NULL_GUID;
}
#endif

