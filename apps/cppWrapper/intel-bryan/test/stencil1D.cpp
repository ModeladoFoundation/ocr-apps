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
    ocrGuid_t stencilTML;
} private_t;

ocrGuid_t stencilInitDBsEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    return NULL_GUID;
}

ocrGuid_t realMainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u64 i;
    double * dummy;

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

    ocrEdtTemplateCreate( &stencilInitDBsTML, stencilInitDBsEdt, 2, 1 );

    ocrDbRelease( sharedGUID );

    u64 params[2];
    params[1] = nrank;

    for( i = 0; i < nrank; i++ ){
        params[0] = i;
        ocrEdtCreate( &stencilInitDBsGUID, stencilInitDBsTML, EDT_PARAM_DEF, params, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, NULL_HINT, NULL );
        ocrAddDependence( sharedGUID, stencilInitDBsGUID, 0, DB_MODE_RW );
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
    u32 argc = getArgc(programArgv);

    if(argc != 4) {
        PRINTF("Using default runtime args\n");
        nrank = 4;
        npoints = 10;
        maxt = 100;
    } else {
        i = 1;
        nrank = (u32) atoi(getArgv(programArgv, i++));
        npoints = (u32) atoi(getArgv(programArgv, i++));
        maxt = (u32) atoi(getArgv(programArgv, i));
    }

    PRINTF("1D Stencil code with Channel Events.\n");
    PRINTF("number of workers = %ld \n", nrank);
    PRINTF("data on each worker = %ld \n", npoints);
    PRINTF("number of timesteps = %ld \n", maxt);

    realMainPRM.nrank = nrank;
    realMainPRM.npoints = npoints;
    realMainPRM.maxt = maxt;

/*-----------------------OCR API CALLS-----------------------*/

    ocrGuid_t realMainDBK;

    double * dummy;

    EdtProducer myRealMain( realMainEdt, sizeof(realMainPRM_t)/sizeof(u64), 1, true );
    EdtProducer myWrapup( wrapupEdt, 0, 1, false );

    EdtLauncher realMainLaunch( myRealMain, myRealMain.create() );
    EdtLauncher wrapupLaunch( myWrapup, myWrapup.create() );

    wrapupLaunch.depPush( myRealMain.getFinishEVT() );

    ocrDbCreate( &realMainDBK, (void **)&dummy, sizeof( shared_t ), 0, NULL_HINT, NO_ALLOC );
    realMainLaunch.depPush( realMainDBK );

    return NULL_GUID;
}
#else
extern "C" ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    PRINTF("Channel event extension not enabled!\n");

    ocrShutdown();
    return NULL_GUID;
}
#endif

