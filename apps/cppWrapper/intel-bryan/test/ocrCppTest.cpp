#include <ocr.h>
#include "../src/ocrCpp.h"
#include <iostream>

using namespace std;

ocrGuid_t realMainEdt( u32 paramc, u64 * paramv, u32 depc, u64 * depv )
{
    ocrPrintf("Hello, from realmain!\n");
    ocrShutdown();

    return NULL_GUID;

}

extern "C" ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, u64 * depv )
{
    double * dummy;

    ocrGuid_t realMainGUID;
    ocrGuid_t dummyDBK;

    EdtProducer myRealMain;
    EdtLauncher realMainLaunch;

    myRealMain = EdtProducer( (ocrEdt_t) realMainEdt, 0, 2, false );

    realMainGUID = myRealMain.create();

    realMainLaunch = EdtLauncher( myRealMain, realMainGUID );


    u64 i;

    for( i = 0; i < 2; i++ ){

        ocrDbCreate( &dummyDBK, (void **)&dummy, sizeof( double ), 0, NULL_HINT, NO_ALLOC );
        realMainLaunch.depPush( dummyDBK );

    }

    //realMainLaunch.launch( );

    return NULL_GUID;

}

