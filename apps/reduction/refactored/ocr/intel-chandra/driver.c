/*
    Author: Chandra S Martha
    Copyright Intel Corporation 2015

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.
*/

#include <stdio.h>
#include <stdlib.h>

#include "ocr.h"
#include "ocr-std.h"

#include "reduction_v1.h"

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

typedef struct
{
    u32 id;
    ocrGuid_t EVT_reduction;
} rankPartialSumCompute_t;

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_rankPartialSumCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //ocrPrintf("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    void* PTR_cmdLineArgs = depv[0].ptr;

    u32 argc = ocrGetArgc( PTR_cmdLineArgs );

    u64 N = atoi( ocrGetArgv( PTR_cmdLineArgs, 1 ) );

    ocrGuid_t DBK_reductionEventsH;
    ocrGuid_t* PTR_reductionEventsH;

    ocrDbCreate( &DBK_reductionEventsH, (void **) &PTR_reductionEventsH, (N+1)*sizeof(ocrGuid_t),
                 DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    ocrGuid_t TS_globalInit_OET, TS_globalCompute_OET, TS_globalFinalize_OET;

    ocrEventCreate( &TS_globalInit_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalCompute_OET, OCR_EVENT_STICKY_T, true );
    ocrEventCreate( &TS_globalFinalize_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalInit, TS_globalCompute, TS_globalFinalize;

    TS_globalInit.FNC = FNC_globalInit;
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, 1, 2 );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, &N, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_HINT, &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_cmdLineArgs, TS_globalInit.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_reductionEventsH, TS_globalInit.EDT, _idep++, DB_MODE_RW );

    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, 1, 2 );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, &N, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_HINT, &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_reductionEventsH, TS_globalCompute.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    TS_globalFinalize.FNC = FNC_globalFinalize;
    ocrEdtTemplateCreate( &TS_globalFinalize.TML, TS_globalFinalize.FNC, 1, 2 );

    ocrEdtCreate( &TS_globalFinalize.EDT, TS_globalFinalize.TML,
                  EDT_PARAM_DEF, &N, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_HINT, NULL);

    _idep = 0;
    ocrAddDependence( DBK_reductionEventsH, TS_globalFinalize.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalCompute_OET, TS_globalFinalize.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

// ( )
// { DBK_cmdLineArgs <RO>, DBK_reductionEventsH <RW> }
ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //ocrPrintf("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    u64 N = paramv[0];
    ocrPrintf("\nDoing scalar reduction across %d EDTs\n", N);

    ocrGuid_t DBK_reductionEventsH = depv[1].guid;

    ocrGuid_t* PTR_reductionEventsH = depv[1].ptr;

    for( u32 i = 0; i < N+1; i++ )
        ocrEventCreate( &PTR_reductionEventsH[i], OCR_EVENT_STICKY_T, true );

    ocrLibRed_setup_tree_serial( N, PTR_reductionEventsH, PTR_reductionEventsH[N], FNC_reduction_double );

    return NULL_GUID;
}

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //ocrPrintf("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_reductionEventsH = depv[0].guid;

    ocrGuid_t *PTR_reductionEventsH = depv[0].ptr;

    u64 N = paramv[0];

    u64 compute_paramv[2];

    rankPartialSumCompute_t PRM_rankPartialSumCompute_t;
    MyOcrTaskStruct_t TS_rankPartialSumCompute;
    _paramc = sizeof(PRM_rankPartialSumCompute_t)/sizeof(u64); _depc = 1;
    TS_rankPartialSumCompute.FNC = FNC_rankPartialSumCompute;
    ocrEdtTemplateCreate( &TS_rankPartialSumCompute.TML, TS_rankPartialSumCompute.FNC, _paramc, _depc );

    //ocrPrintf("Timestep# %d NR = %d\n", itimestep, NR);
    for( u64 I = 0; I < N; I++ )
    {
        PRM_rankPartialSumCompute_t.id = I;
        PRM_rankPartialSumCompute_t.EVT_reduction = PTR_reductionEventsH[I];

        double* PTR_reduction_in;
        ocrGuid_t DBK_reduction_in;
        ocrDbCreate( &DBK_reduction_in, (void **) &PTR_reduction_in, (1)*sizeof(double),
                     DB_PROP_NONE, NULL_HINT, NO_ALLOC );

        ocrEdtCreate( &TS_rankPartialSumCompute.EDT, TS_rankPartialSumCompute.TML,
                      EDT_PARAM_DEF, (u64*)&PRM_rankPartialSumCompute_t, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_HINT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_reduction_in, TS_rankPartialSumCompute.EDT, _idep++, DB_MODE_RW );
    }

    return PTR_reductionEventsH[N];
}

ocrGuid_t FNC_rankPartialSumCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //ocrPrintf("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    _idep = 0;

    rankPartialSumCompute_t* PTR_PRM_rankPartialSumCompute_t = (rankPartialSumCompute_t*) paramv;

    u64 I = PTR_PRM_rankPartialSumCompute_t->id;
    ocrGuid_t EVT_reduction_in = PTR_PRM_rankPartialSumCompute_t->EVT_reduction;

    ocrGuid_t DBK_reduction_in = depv[0].guid;
    double* PTR_reduction_in = depv[0].ptr;

    PTR_reduction_in[0] = (double) I;

    ocrDbRelease( DBK_reduction_in );

    ocrEventSatisfy( EVT_reduction_in, DBK_reduction_in );

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //ocrPrintf("%s\n", __func__);

    ocrGuid_t DBK_reductionEventsH = depv[0].guid;
    ocrGuid_t DBK_reduction_out = depv[1].guid;

    ocrGuid_t* PTR_reductionEventsH = depv[0].ptr;
    double* PTR_reduction_out = depv[1].ptr;

    u64 N = paramv[0];

    if( PTR_reduction_out[0] == N*(N-1)*0.5 )
        ocrPrintf("\nVERIFICATION passed!\n");
    else
        ocrPrintf("\nVERIFICATION Failed!\n");

    ocrPrintf("RESULT = %f Expected = %f\n", PTR_reduction_out[0], N*(N-1)*0.5 );

    ocrDbDestroy( DBK_reductionEventsH );

    ocrShutdown();

    return NULL_GUID;
}
