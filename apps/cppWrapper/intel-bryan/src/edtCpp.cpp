#ifndef OCR_CPP
#define OCR_CPP
#include<ocrCpp.h>
#endif
#include<iostream>
#include<cstring>


EdtProducer::EdtProducer( ) {

    finishEVT = NULL_GUID;
    TML = NULL_GUID;
    depSize = 0;
    paramSize = 0;

}

EdtProducer::EdtProducer( ocrEdt_t funcPtr, u32 paramc, u32 depc, bool isFinish ) {

    paramSize = paramc;
    depSize = depc;
    finish = isFinish;

    ocrEdtTemplateCreate( &TML, funcPtr, paramSize, depSize );

}

EdtProducer::EdtProducer( ocrGuid_t tml, u32 paramc, u32 depc, bool isFinish ) {

    paramSize = paramc,
    depSize = depc;
    finish = isFinish;
    TML = tml;
}

ocrGuid_t EdtProducer::create( )
{


    ocrGuid_t edt = NULL_GUID;

    u64 * paramPTR = NULL;

    if( paramSize != 0 ) paramPTR = params;

    if( IS_GUID_NULL( TML ) ){
        std::cout << "this edt class has not been initialized (TML == NULL_GUID)!" << std::endl;
    }else{
        if( finish ) ocrEdtCreate( &edt, TML, EDT_PARAM_DEF, paramPTR, EDT_PARAM_DEF, NULL,
                                        EDT_PROP_FINISH, NULL_HINT, &finishEVT );

        else ocrEdtCreate( &edt, TML, EDT_PARAM_DEF, paramPTR, EDT_PARAM_DEF, NULL,
                                        EDT_PROP_NONE, NULL_HINT, NULL );
    }

    return edt;
}

void EdtProducer::setFuncPtr( ocrEdt_t funcPtr ) {

    if( !IS_GUID_NULL( TML ) ) std::cout << "Warning: TML was already created." << std::endl;

    ocrEdtTemplateCreate( &TML, funcPtr, paramSize, depSize ); //just create the TML over again.
}

void EdtProducer::setParams( u64 * inParams ) {

    memcpy( params, inParams, sizeof( u64 ) * paramSize );

}

EdtLauncher::EdtLauncher( EdtProducer prod, ocrGuid_t edt = NULL_GUID ) {

    depSize = prod.getDepSize( );
    edtGUID = edt;

}

u32 EdtLauncher::depPush( ocrGuid_t dep ) {

    u32 result = 0;

    deps.push_back( dep );

    if( deps.size() == depSize ) launch( );

    return result;

}


void EdtLauncher::launch( ) {

    u64 i;

    for( i = 0; i < depSize; i++ )
    {
        ocrAddDependence( deps[i], edtGUID, i, DB_MODE_RW );
    }

    reset();

}

void EdtLauncher::reset( ) {

    edtGUID = NULL_GUID;
    deps.clear();

}
