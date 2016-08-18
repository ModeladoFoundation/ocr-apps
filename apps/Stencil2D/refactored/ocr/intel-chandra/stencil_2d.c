/*
    Author: Chandra S. Martha
    Copywrite Intel Corporation 2015

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.
*/

/* 1-D/2-D stencil is implemented in OCR making use of rank data structures to
 * easily create, track and manage the settings, data, and events needed for OCR implementation.
 *
 * Halo-exchange is done by passing the halo datablocks via event dependency channels
 * E.g., A sticky event is used to establish dependency between the sender and receiver tasks belonging
 * to different ranks. The sender satisfies the event with a halo datablock and the receiver then can access
 * the contents of the halo datablock.
 */

#include "ocr.h"
#include "ocr-std.h"
#include "extensions/ocr-affinity.h"

#define ENABLE_EXTENSION_LABELING
#include "extensions/ocr-labeling.h" //currently needed for labeled guids

#ifndef TG_ARCH
#include "time.h"
#endif
#include "timers.h"
#include <stdlib.h>
#include <math.h>

#include "stencil.h"
#include "reduction.h"

//mainEdt
// Macro _OCR_TASK_FNC_( FNC_name ) expands to ocrGuid_t FNC_name( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
_OCR_TASK_FNC_( FNC_globalInit );
    _OCR_TASK_FNC_( FNC_init_globalParamH );
    _OCR_TASK_FNC_( FNC_init_globalH );
        _OCR_TASK_FNC_( FNC_init_globalH_part1 );
        _OCR_TASK_FNC_( FNC_rankInitSpawner );
        //          /                                                   /                                                   /
            _OCR_TASK_FNC_( FNC_rankInit );               // _OCR_TASK_FNC_( FNC_rankInit );               // _OCR_TASK_FNC_( FNC_rankInit );
                _OCR_TASK_FNC_( FNC_init_rankH );         //     _OCR_TASK_FNC_( FNC_init_rankH );         //     _OCR_TASK_FNC_( FNC_init_rankH );
                    _OCR_TASK_FNC_( FNC_init_rankDataH ); //         _OCR_TASK_FNC_( FNC_init_rankDataH ); //         _OCR_TASK_FNC_( FNC_init_rankDataH );

_OCR_TASK_FNC_( FNC_globalCompute );
    _OCR_TASK_FNC_( FNC_rankComputeSpawner );
    //              /                                                   /                                                   /
        _OCR_TASK_FNC_( FNC_rankMultiTimestepper);        //  _OCR_TASK_FNC_( FNC_rankMultiTimestepper );  //  _OCR_TASK_FNC_( FNC_rankMultiTimestepper );
            _OCR_TASK_FNC_( FNC_timestep );               //      _OCR_TASK_FNC_( FNC_timestep );          //      _OCR_TASK_FNC_( FNC_timestep );
                _OCR_TASK_FNC_( FNC_Lsend );              //          _OCR_TASK_FNC_( FNC_Lsend );         //          _OCR_TASK_FNC_( FNC_Lsend );
                _OCR_TASK_FNC_( FNC_Rsend );              //          _OCR_TASK_FNC_( FNC_Rsend );         //          _OCR_TASK_FNC_( FNC_Rsend );
                _OCR_TASK_FNC_( FNC_Lrecv );              //          _OCR_TASK_FNC_( FNC_Lrecv );         //          _OCR_TASK_FNC_( FNC_Lrecv );
                _OCR_TASK_FNC_( FNC_Rrecv );              //          _OCR_TASK_FNC_( FNC_Rrecv );         //          _OCR_TASK_FNC_( FNC_Rrecv );
                #if PROBLEM_TYPE==2                       //          #if PROBLEM_TYPE==2                  //          #if PROBLEM_TYPE==2
                _OCR_TASK_FNC_( FNC_Bsend );              //          _OCR_TASK_FNC_( FNC_Bsend );         //          _OCR_TASK_FNC_( FNC_Bsend );
                _OCR_TASK_FNC_( FNC_Tsend );              //          _OCR_TASK_FNC_( FNC_Tsend );         //          _OCR_TASK_FNC_( FNC_Tsend );
                _OCR_TASK_FNC_( FNC_Brecv );              //          _OCR_TASK_FNC_( FNC_Brecv );         //          _OCR_TASK_FNC_( FNC_Brecv );
                _OCR_TASK_FNC_( FNC_Trecv );              //          _OCR_TASK_FNC_( FNC_Trecv );         //          _OCR_TASK_FNC_( FNC_Trecv );
                #endif                                    //          #endif                               //          #endif
            _OCR_TASK_FNC_( FNC_update );                 //      _OCR_TASK_FNC_( FNC_update );            //      _OCR_TASK_FNC_( FNC_update );
    _OCR_TASK_FNC_( FNC_reduction );
    _OCR_TASK_FNC_( FNC_summary );

_OCR_TASK_FNC_( FNC_globalFinalize );
//END mainEdt

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    ocrGuid_t DBK_globalH;
    globalH_t* PTR_globalH;

    ocrDbCreate( &DBK_globalH, (void **) &PTR_globalH, sizeof(globalH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t TS_globalInit_OET, TS_globalCompute_OET, TS_globalFinalize_OET;

    ocrEventCreate( &TS_globalInit_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalCompute_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalFinalize_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalInit, TS_globalCompute, TS_globalFinalize;

    TS_globalInit.FNC = FNC_globalInit;
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_cmdLineArgs, TS_globalInit.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_globalInit.EDT, _idep++, DB_MODE_RW );

    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    return NULL_GUID;
}

// ( )
// { DBK_cmdLineArgs <RO>, DBK_globalH <RW> }
ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    void* PTR_cmdLineArgs = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;

    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &(PTR_globalH->DBK_globalParamH), (void **) &PTR_globalParamH, sizeof(globalParamH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    //paramInit
    ocrGuid_t TS_init_globalParamH_OET;
    ocrEventCreate( &TS_init_globalParamH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_globalParamH;

    TS_init_globalParamH.FNC = FNC_init_globalParamH;
    ocrEdtTemplateCreate( &TS_init_globalParamH.TML, TS_init_globalParamH.FNC, 0, 2 );

    ocrEdtCreate( &TS_init_globalParamH.EDT, TS_init_globalParamH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_init_globalParamH.OET );

    ocrAddDependence( TS_init_globalParamH.OET, TS_init_globalParamH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_cmdLineArgs, TS_init_globalParamH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_init_globalParamH.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_init_globalH; _paramc = 0; _depc = 3;

    TS_init_globalH.FNC = FNC_init_globalH;
    ocrEdtTemplateCreate( &TS_init_globalH.TML, TS_init_globalH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_globalH.EDT, TS_init_globalH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_init_globalH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_init_globalH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_globalParamH_OET, TS_init_globalH.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

// ( )
// { DBK_cmdLineArgs <RO>, DBK_globalParamH <RW> }
ocrGuid_t FNC_init_globalParamH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    void* PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t* PTR_globalParamH = depv[1].ptr;

    u32 argc = getArgc(PTR_cmdLineArgs);
    DEBUG_PRINTF(("Got ARGC: %"PRIu32"\n", argc));
#ifdef DEBUG_APP
    u32 i;
    for(i=0; i<argc; ++i) {
        PRINTF("ARG %"PRIu32": %s\n", i, getArgv(PTR_cmdLineArgs, i));
    }
#endif
    s64 npoints, nranks, ntimesteps;

    //Default values
    npoints = NPOINTS;
    nranks = NRANKS;
    ntimesteps = NTIMESTEPS;

    //Overwrite defaults
    if (argc == 4)
    {
        u32 i = 1;
        npoints = (s64) atoi(getArgv(PTR_cmdLineArgs, i++));
        nranks = (s64) atoi(getArgv(PTR_cmdLineArgs, i++));
        ntimesteps = (s64) atoi(getArgv(PTR_cmdLineArgs, i++));

        npoints = (npoints != -1) ? npoints : NPOINTS;
        nranks = (nranks != -1) ? nranks : NRANKS;
        ntimesteps = (ntimesteps != -1) ? ntimesteps : NTIMESTEPS;
    }

    PTR_globalParamH->NP = npoints; //squre global tile is assumed; side
    PTR_globalParamH->NR = nranks;
    PTR_globalParamH->NT = ntimesteps;

    #if PROBLEM_TYPE==2
    PTR_globalParamH->NP_X = (s64) PTR_globalParamH->NP; //Simplified for now
    PTR_globalParamH->NP_Y = (s64) PTR_globalParamH->NP; //Simplified for now

    s64 Num_procs = PTR_globalParamH->NR;
    s64 Num_procsx, Num_procsy;

    splitDimension(Num_procs, &Num_procsx, &Num_procsy);

    PTR_globalParamH->NR_X = (s64) Num_procsx;
    PTR_globalParamH->NR_Y = (s64) Num_procsy;
    #elif PROBLEM_TYPE==1
    PTR_globalParamH->NP_X = (s64) PTR_globalParamH->NP;
    PTR_globalParamH->NR_X = (s64) PTR_globalParamH->NR;
    #endif

    PRINTF("\n");
    PRINTF("OCR stencil execution on 2D grid\n");
    #if PROBLEM_TYPE==2
    PRINTF("Grid size                   = %lldx%lld\n", PTR_globalParamH->NP_X, PTR_globalParamH->NP_Y);
    #elif PROBLEM_TYPE==1
    PRINTF("Grid size                   = %lld\n", PTR_globalParamH->NP_X);
    #endif
    PRINTF("Number of tiles             = %lld\n", PTR_globalParamH->NR);
    #if PROBLEM_TYPE==2
    PRINTF("Tiles in x & y-directions   = %lldx%lld\n", PTR_globalParamH->NR_X, PTR_globalParamH->NR_Y);
    #elif PROBLEM_TYPE==1
    PRINTF("Tiles in x                  = %lld\n", PTR_globalParamH->NR_X);
    #endif
    PRINTF("Radius of stencil           = %d\n", HALO_RADIUS);
    PRINTF("Type of stencil             = star\n");
    PRINTF("Data type                   = double precision\n");
    PRINTF("Number of iterations        = %lld\n", PTR_globalParamH->NT);
    PRINTF("\n");

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_globalH <RW>, EVT_init_globalParamH <NULL> }
ocrGuid_t FNC_init_globalH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;

    MyOcrTaskStruct_t TS_init_globalH_part1; _paramc = 0; _depc = 2;
    ocrGuid_t TS_init_globalH_part1_OET;
    ocrEventCreate( &TS_init_globalH_part1_OET, OCR_EVENT_STICKY_T, false );

    TS_init_globalH_part1.FNC = FNC_init_globalH_part1;
    ocrEdtTemplateCreate( &TS_init_globalH_part1.TML, TS_init_globalH_part1.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_globalH_part1.EDT, TS_init_globalH_part1.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_init_globalH_part1.OET );

    ocrAddDependence( TS_init_globalH_part1.OET, TS_init_globalH_part1_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_init_globalH_part1.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_init_globalH_part1.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_rankInitSpawner; _paramc = 0; _depc = 3;

    TS_rankInitSpawner.FNC = FNC_rankInitSpawner;
    ocrEdtTemplateCreate( &TS_rankInitSpawner.TML, TS_rankInitSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_rankInitSpawner.EDT, TS_rankInitSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL ); //Fires up individual EDTs to allocate subdomain DBs, EDTs

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_globalH_part1_OET, TS_rankInitSpawner.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_globalH <RW> }
ocrGuid_t FNC_init_globalH_part1(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;

    u64 NR = PTR_globalParamH->NR;

    PTR_globalH->itimestep = 0; //Initial timestep is 0

    ocrEventCreate( &PTR_globalH->EVT_OUT_norm_reduction, OCR_EVENT_STICKY_T, true );
    ocrEventCreate( &PTR_globalH->EVT_OUT_timer_reduction, OCR_EVENT_STICKY_T, true );

    ocrGuidRangeCreate(&(PTR_globalH->EVT_RANGE_reduction), NR, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(PTR_globalH->EVT_RANGE_timer_reduction), NR, GUID_USER_EVENT_STICKY);

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_globalH <RW>, EVT_init_globalH_part1 <NULL> }
ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;

    u64 NR = PTR_globalParamH->NR;
    u64 NR_X = PTR_globalParamH->NR_X;
    #if PROBLEM_TYPE==2
    u64 NR_Y = PTR_globalParamH->NR_Y;
    #endif

    //spawn N intializer EDTs
    ocrGuid_t* PTR_rankHs;
    ocrDbCreate( &PTR_globalH->DBK_rankHs, (void **) &PTR_rankHs, sizeof(ocrGuid_t)*NR,
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    MyOcrTaskStruct_t TS_rankInit; _paramc = 1; _depc = 3;

    u64 init_paramv[1] = {-1}; //each spawned EDT gets its rank or id

    TS_rankInit.FNC = FNC_rankInit;
    ocrEdtTemplateCreate( &TS_rankInit.TML, TS_rankInit.FNC, _paramc, _depc );

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    ASSERT( affinityCount >= 1 );
    PRINTF("Using affinity API\n");
    s64 PD_X, PD_Y;
    splitDimension(affinityCount, &PD_X, &PD_Y);
#else
    PRINTF("NOT Using affinity API\n");
#endif

    s32 i;
    for( i = 0; i < NR; i++ )
    {
#ifdef ENABLE_EXTENSION_AFFINITY
        int id_x = i%NR_X;
        s64 pd_x; getPartitionID(id_x, 0, NR_X-1, PD_X, &pd_x);

        int pd = pd_x;

        #if PROBLEM_TYPE==2
        int id_y = i/NR_X;
        s64 pd_y; getPartitionID(id_y, 0, NR_Y-1, PD_Y, &pd_y);

        pd = globalRankFromCoords(pd_x, pd_y, PD_X, PD_Y);
        #endif
        DEBUG_PRINTF(("rank %d, policy domain %d\n", i, pd));
        ocrAffinityGetAt( AFFINITY_PD, pd, &(currentAffinity) );
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

        rankH_t *PTR_rankH;
        ocrDbCreate( &(PTR_rankHs[i]), (void **) &PTR_rankH, sizeof(rankH_t),
                     DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate( &TS_rankInit.EDT, TS_rankInit.TML,
                      EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalParamH, TS_rankInit.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_globalH, TS_rankInit.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_rankHs[i], TS_rankInit.EDT, _idep++, DB_MODE_RW );
    }

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_EVT_IN_reduction <COST>, DBK_rankH <RW> }
ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DBK_globalH = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_rankH = (ocrGuid_t) depv[2].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;
    rankH_t *PTR_rankH = (rankH_t*) depv[2].ptr;

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

    globalParamH_t *PTR_globalParamH_rankCopy;
    ocrDbCreate( &(PTR_rankH->DBK_globalParamH_rankCopy), (void **) &PTR_globalParamH_rankCopy, sizeof(globalParamH_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    rankParamH_t *PTR_rankParamH;
    ocrDbCreate( &(PTR_rankH->DBK_rankParamH), (void **) &PTR_rankParamH, sizeof(rankParamH_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    rankDataH_t *PTR_rankDataH;
    ocrDbCreate( &(PTR_rankH->DBK_rankDataH), (void **) &PTR_rankDataH, sizeof(rankDataH_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    timer* PTR_timers;
    ocrDbCreate( &(PTR_rankH->DBK_timers), (void **) &PTR_timers, sizeof(timer)*number_of_timers,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    rankTemplateH_t *PTR_rankTemplateH;
    ocrDbCreate( &(PTR_rankH->DBK_rankTemplateH), (void **) &PTR_rankTemplateH, sizeof(rankTemplateH_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    reductionPrivate_t* PTR_reductionH;
    ocrDbCreate( &(PTR_rankH->DBK_reductionH), (void **) &PTR_reductionH, sizeof(reductionPrivate_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    reductionPrivate_t* PTR_timer_reductionH;
    ocrDbCreate( &(PTR_rankH->DBK_timer_reductionH), (void **) &PTR_timer_reductionH, sizeof(reductionPrivate_t),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    s32 i;
    for( i = 0; i < 2; i++ )
    {
        rankEventH_t* PTR_rankEventH;
        ocrDbCreate(    &(PTR_rankH->DBK_rankEventHs[i]), (void **) &PTR_rankEventH, sizeof(rankEventH_t),
                        DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    }

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 1; _depc = 11;

    TS_init_rankH.FNC = FNC_init_rankH;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_init_rankH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_init_rankH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_globalParamH_rankCopy, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankParamH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankDataH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_timers, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankEventHs[0], TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankEventHs[1], TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankTemplateH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_reductionH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_timer_reductionH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

// ( id )
// { DBK_globalParamH <COST>, DBK_EVT_IN_reduction <CONST>
//   DBK_rankParamH <RW>, DBK_rankDataH <RW>, DBK_rankEventHs_0 <RW>, DBK_rankEventHs_1 <RW>,
//   EVT_init_globalParamH <NULL>}
ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    s32 _paramc, _depc, _idep;

    s64 id = (s64) paramv[0];

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;
    ocrGuid_t DBK_globalParamH_rankCopy = depv[2].guid;
    ocrGuid_t DBK_rankParamH = depv[3].guid;
    ocrGuid_t DBK_timers = depv[4].guid;

    globalParamH_t *PTR_globalParamH = (globalParamH_t*) depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;
    globalParamH_t *PTR_globalParamH_rankCopy = (globalParamH_t*) depv[2].ptr;
    rankParamH_t *PTR_rankParamH = (rankParamH_t*) depv[3].ptr;
    rankDataH_t *PTR_rankDataH = (rankDataH_t*) depv[4].ptr;
    timer* PTR_timers = depv[5].ptr;

    s64 i;
    for( i = 0; i < number_of_timers; i++ )
    {
        PTR_timers[i].start = 0;
        PTR_timers[i].total = 0;
        PTR_timers[i].count = 0;
        PTR_timers[i].elapsed = 0;
    }

    ocrDbRelease(DBK_timers);

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

    //Make a copy of the DBK_globalParamH for each rank
    PTR_globalParamH_rankCopy->NP = PTR_globalParamH->NP;
    PTR_globalParamH_rankCopy->NR = PTR_globalParamH->NR;
    PTR_globalParamH_rankCopy->NP_X = PTR_globalParamH->NP_X;
    PTR_globalParamH_rankCopy->NR_X = PTR_globalParamH->NR_X;
    #if PROBLEM_TYPE==2
    PTR_globalParamH_rankCopy->NP_Y = PTR_globalParamH->NP_Y;
    PTR_globalParamH_rankCopy->NR_Y = PTR_globalParamH->NR_Y;
    #endif
    PTR_globalParamH_rankCopy->NT = PTR_globalParamH->NT;

    PTR_rankParamH->id = id;
    PTR_rankParamH->id_x = id%PTR_globalParamH->NR_X;

    partition_bounds( PTR_rankParamH->id_x, 0, PTR_globalParamH->NP_X-1, PTR_globalParamH->NR_X,
                      &(PTR_rankParamH->ib), &(PTR_rankParamH->ie) );
    PTR_rankParamH->np_x = PTR_rankParamH->ie - PTR_rankParamH->ib + 1;
    s64 np_x = PTR_rankParamH->np_x;

    #if PROBLEM_TYPE==2
    PTR_rankParamH->id_y = id/PTR_globalParamH->NR_X;
    partition_bounds( PTR_rankParamH->id_y, 0, PTR_globalParamH->NP_Y-1, PTR_globalParamH->NR_Y,
                      &(PTR_rankParamH->jb), &(PTR_rankParamH->je) );
    PTR_rankParamH->np_y = PTR_rankParamH->je - PTR_rankParamH->jb + 1;
    s64 np_y = PTR_rankParamH->np_y;
    #endif

    PTR_rankParamH->itimestep = 0; //Initial timestep is 0

    double *xIn, *xOut, *weight, *refNorm;
    #if PROBLEM_TYPE==2
    ocrDbCreate( &(PTR_rankDataH->DBK_xIn), (void **) &xIn, sizeof(double)*(np_x+2*HALO_RADIUS)*(np_y+2*HALO_RADIUS),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_xOut), (void **) &xOut, sizeof(double)*(np_x)*(np_y),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_weight), (void **) &weight, sizeof(double)*(2*HALO_RADIUS+1)*(2*HALO_RADIUS+1),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    #elif PROBLEM_TYPE==1
    ocrDbCreate( &(PTR_rankDataH->DBK_xIn), (void **) &xIn, sizeof(double)*(np_x+2*HALO_RADIUS),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_xOut), (void **) &xOut, sizeof(double)*(np_x),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_weight), (void **) &weight, sizeof(double)*(2*HALO_RADIUS+1),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    #endif
    ocrDbCreate( &(PTR_rankDataH->DBK_refNorm), (void **) &refNorm, sizeof(double)*(1),
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    double *lsend, *rsend;
    #if PROBLEM_TYPE==2
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[0]), (void **) &lsend, sizeof(double)*HALO_RADIUS*np_y,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[1]), (void **) &lsend, sizeof(double)*HALO_RADIUS*np_y,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[0]), (void **) &rsend,  sizeof(double)*HALO_RADIUS*np_y,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[1]), (void **) &rsend,  sizeof(double)*HALO_RADIUS*np_y,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );

    double *tsend, *bsend;
    ocrDbCreate( &(PTR_rankDataH->DBK_TsendBufs[0]), (void **) &tsend,  sizeof(double)*HALO_RADIUS*np_x,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_TsendBufs[1]), (void **) &tsend,  sizeof(double)*HALO_RADIUS*np_x,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_BsendBufs[0]), (void **) &bsend, sizeof(double)*HALO_RADIUS*np_x,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_BsendBufs[1]), (void **) &bsend, sizeof(double)*HALO_RADIUS*np_x,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    #elif PROBLEM_TYPE==1
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[0]), (void **) &lsend, sizeof(double)*HALO_RADIUS,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[1]), (void **) &lsend, sizeof(double)*HALO_RADIUS,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[0]), (void **) &rsend,  sizeof(double)*HALO_RADIUS,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[1]), (void **) &rsend,  sizeof(double)*HALO_RADIUS,
                 DB_PROP_NONE, PICK_1_1(&HNT_db,currentAffinity), NO_ALLOC );
    #endif

    ocrDbRelease( DBK_globalParamH_rankCopy ); //This datablock is modified in this EDT and a subsequent
    //EDT depends on it. So, ocrDbRelease() is called to makesure the latter EDT sees the changes.
    MyOcrTaskStruct_t TS_init_rankDataH; _paramc = 0; _depc = 6;

    TS_init_rankDataH.FNC = FNC_init_rankDataH;
    ocrEdtTemplateCreate( &TS_init_rankDataH.TML, TS_init_rankDataH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankDataH.EDT, TS_init_rankDataH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_init_rankDataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_rankParamH, TS_init_rankDataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_xIn, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_xOut, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_weight, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_refNorm, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );

    for( i = 0; i < 2; i++ )
    {
        rankEventH_t *PTR_rankEventH = (rankEventH_t*) depv[6+i].ptr;

        ocrEventCreate( &PTR_rankEventH->EVT_Lsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Rsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Lrecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_rankEventH->EVT_Rrecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_rankEventH->EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );

        #if PROBLEM_TYPE==2
        ocrEventCreate( &PTR_rankEventH->EVT_Bsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Tsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Brecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_rankEventH->EVT_Trecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_rankEventH->EVT_Brecv_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_rankEventH->EVT_Trecv_fin, OCR_EVENT_STICKY_T, false );
        #endif
    }

    rankTemplateH_t *PTR_rankTemplateH = depv[8].ptr;

    _paramc = 0; _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Lsend), FNC_Lsend, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Rsend), FNC_Rsend, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Lrecv), FNC_Lrecv, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Rrecv), FNC_Rrecv, _paramc,_depc );

    #if PROBLEM_TYPE==2
    _paramc = 0; _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Bsend), FNC_Bsend, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Tsend), FNC_Tsend, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Brecv), FNC_Brecv, _paramc, _depc );
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Trecv), FNC_Trecv, _paramc,_depc );
    #endif

    #if PROBLEM_TYPE==2
    _paramc = 0; _depc = 18;
    #elif PROBLEM_TYPE==1
    _paramc = 0; _depc = 14;
    #endif

    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_update), FNC_update, _paramc, _depc );

    reductionPrivate_t* PTR_reductionH = depv[9].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[10].ptr;

    PTR_reductionH->nrank = PTR_globalParamH_rankCopy->NR;
    PTR_reductionH->myrank = id;
    PTR_reductionH->ndata = 1;
    PTR_reductionH->reductionOperator = REDUCTION_F8_ADD;
    PTR_reductionH->rangeGUID = PTR_globalH->EVT_RANGE_reduction;
    PTR_reductionH->reductionTML = NULL_GUID;
    PTR_reductionH->new = 1;  //first time
    PTR_reductionH->all = 0;  //go up and down (ALL_REDUCE)
    //ocrEventCreate(&(PTR_reductionH->returnEVT), OCR_EVENT_ONCE_T, true);
    if(id==0) PTR_reductionH->returnEVT = PTR_globalH->EVT_OUT_norm_reduction;

    PTR_timer_reductionH->nrank = PTR_globalParamH_rankCopy->NR;
    PTR_timer_reductionH->myrank = id;
    PTR_timer_reductionH->ndata = 1;
    PTR_timer_reductionH->reductionOperator = REDUCTION_F8_MAX;
    PTR_timer_reductionH->rangeGUID = PTR_globalH->EVT_RANGE_timer_reduction;
    PTR_timer_reductionH->reductionTML = NULL_GUID;
    PTR_timer_reductionH->new = 1;  //first time
    PTR_timer_reductionH->all = 0;  //go up and down (ALL_REDUCE)
    //ocrEventCreate(&(PTR_timer_reductionH->returnEVT), OCR_EVENT_ONCE_T, true);
    if(id==0) PTR_timer_reductionH->returnEVT = PTR_globalH->EVT_OUT_timer_reduction;

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<RO>, DBK_rankParamH<RO>,
//   DBK_xIn<RW>, DBK_xOut<RW>, DBK_weight<RW>, DBK_refNorm<RW> }
ocrGuid_t FNC_init_rankDataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));

    globalParamH_t *PTR_globalParamH = (globalParamH_t*) depv[0].ptr;
    rankParamH_t *PTR_rankParamH = (rankParamH_t*) depv[1].ptr;
    double *xIn = (double*) depv[2].ptr;
    double *xOut = (double*) depv[3].ptr;
    double *weight = (double*) depv[4].ptr;
    double *refNorm = (double*) depv[5].ptr;

    s64 np_x = PTR_rankParamH->np_x;
    s64 id_x = PTR_rankParamH->id_x;
    s64 ib = PTR_rankParamH->ib;
    s64 ie = PTR_rankParamH->ie;

    s64 NR_X = PTR_globalParamH->NR_X;

    #if PROBLEM_TYPE==2
    s64 np_y = PTR_rankParamH->np_y;
    s64 id_y = PTR_rankParamH->id_y;
    s64 jb = PTR_rankParamH->jb;
    s64 je = PTR_rankParamH->je;

    s64 NR_Y = PTR_globalParamH->NR_Y;
    #endif

    s64 i, j;
    int ii, jj;

    #if PROBLEM_TYPE==2
    for( j = jb; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            IN(i,j) = (double) ( i ) + ( j );
            OUT(i,j) = 0.;
        }

    /* fill the stencil weights to reflect a discrete divergence operator         */
    for (jj=-HALO_RADIUS; jj<=HALO_RADIUS; jj++) for (ii=-HALO_RADIUS; ii<=HALO_RADIUS; ii++)
        WEIGHT(ii,jj) = (double) 0.0;

    //stencil_size = 4*HALO_RADIUS+1;
    for (ii=1; ii<=HALO_RADIUS; ii++) {
        WEIGHT(0, ii) = WEIGHT( ii,0) =  (double) (1.0/(2.0*ii*HALO_RADIUS));
        WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(double) (1.0/(2.0*ii*HALO_RADIUS));
    }
    #elif PROBLEM_TYPE==1
    for( i = ib; i <= ie; i++ )
    {
        IN(i) = (double) ( i );
        OUT(i) = 0.;
    }

    /* fill the stencil weights to reflect a discrete divergence operator         */
    for (ii=-HALO_RADIUS; ii<=HALO_RADIUS; ii++)
        WEIGHT(ii) = (double) 0.0;

    //stencil_size = 2*HALO_RADIUS+1;
    for (ii=1; ii<=HALO_RADIUS; ii++)
    {
        WEIGHT(ii) = (double) (1.0/(2.0*ii*HALO_RADIUS));
        WEIGHT(-ii) = -WEIGHT(ii);
    }
    #endif

    refNorm[0] = 0.0;

    return NULL_GUID;
}

// ( )
// { DBK_globalH<RW>, EVT_globalInit<NULL> }
ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t *PTR_globalH = depv[0].ptr;

    ocrGuid_t TS_rankComputeSpawner_OET;
    ocrEventCreate( &TS_rankComputeSpawner_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_rankComputeSpawner; _paramc = 0; _depc = 3;

    TS_rankComputeSpawner.FNC = FNC_rankComputeSpawner;
    ocrEdtTemplateCreate( &TS_rankComputeSpawner.TML, TS_rankComputeSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_rankComputeSpawner.EDT, TS_rankComputeSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_rankComputeSpawner.OET );

    ocrAddDependence( TS_rankComputeSpawner.OET, TS_rankComputeSpawner_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_rankComputeSpawner.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_rankComputeSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankComputeSpawner.EDT, _idep++, DB_MODE_CONST );

    MyOcrTaskStruct_t TS_summary; _paramc = 0; _depc = 4;

    TS_summary.FNC = FNC_summary;
    ocrEdtTemplateCreate( &TS_summary.TML, TS_summary.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_summary.EDT, TS_summary.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_summary.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->EVT_OUT_timer_reduction, TS_summary.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_globalH->EVT_OUT_norm_reduction, TS_summary.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_rankComputeSpawner_OET, TS_summary.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

//( )
//{ DBK_globalParamH<CONST>, DBK_globalH<RW>, DBK_rankHs<CONST>, EVT_globalCompute_setup }
ocrGuid_t FNC_rankComputeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 NR = PTR_globalParamH->NR;
    u64 NR_X = PTR_globalParamH->NR_X;
    #if PROBLEM_TYPE==2
    u64 NR_Y = PTR_globalParamH->NR_Y;
    #endif
    u64 NT = PTR_globalParamH->NT;

    u64 itimestep = (u64) PTR_globalH->itimestep;

    u64 I, id_x, id_y;

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

    MyOcrTaskStruct_t TS_rankMultiTimestepper;
    #if PROBLEM_TYPE==2
    _paramc = 3; _depc = 6;
    #elif PROBLEM_TYPE==1
    _paramc = 3; _depc = 4;
    #endif

    u64 compute_paramv[3];

    TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
    ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

    DEBUG_PRINTF(("Timestep# %d NR = %d\n", itimestep, NR));
    for( I = 0; I < NR; I++ )
    {
        compute_paramv[0] = (u64) I;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) NT;

        id_x = I%NR_X;

#ifdef ENABLE_EXTENSION_AFFINITY
        u64 count = 1;
        ocrAffinityQuery(PTR_rankHs[I], &count, &currentAffinity);
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalParamH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_rankHs[I], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        #if PROBLEM_TYPE==2
        id_y = I/NR_X;
        ocrAddDependence( (id_x!=0) ? PTR_rankHs[globalRankFromCoords(id_x-1,id_y, NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( (id_x!=NR_X-1) ? PTR_rankHs[globalRankFromCoords(id_x+1,id_y, NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( (id_y!=0) ? PTR_rankHs[globalRankFromCoords(id_x,id_y-1,NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( (id_y!=NR_Y-1) ? PTR_rankHs[globalRankFromCoords(id_x,id_y+1,NR_X,NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        #elif PROBLEM_TYPE==1
        ocrAddDependence( (id_x!=0) ? PTR_rankHs[id_x-1] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( (id_x!=NR_X-1) ? PTR_rankHs[id_x+1] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        #endif
    }

    PTR_globalH->itimestep += NT;

    return NULL_GUID;
}

//( id, itimestep, ntimesteps )
//{ DBK_globalParamH<CONST>, DBK_rankH<CONST>, DBK_rankH_l<CONST>, DBK_rankH_r<CONST>,
//  DBK_rankH_b<CONST>, DBK_rankH_t<CONST>, DBK_globalH<CONST>, EVT_control_dep<NULL> }
ocrGuid_t FNC_rankMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_globalParamH = depv[_idep++].guid;
    ocrGuid_t DBK_rankH =  depv[_idep++].guid;
    ocrGuid_t DBK_rankH_l = depv[_idep++].guid;
    ocrGuid_t DBK_rankH_r = depv[_idep++].guid;
    #if PROBLEM_TYPE==2
    ocrGuid_t DBK_rankH_b = depv[_idep++].guid;
    ocrGuid_t DBK_rankH_t = depv[_idep++].guid;
    #endif

    _idep = 0;
    globalParamH_t *PTR_globalParamH = depv[_idep++].ptr;
    rankH_t* PTR_rankH = (rankH_t*) depv[_idep++].ptr;
    rankH_t* PTR_rankH_l = (rankH_t*) depv[_idep++].ptr;
    rankH_t* PTR_rankH_r = (rankH_t*) depv[_idep++].ptr;
    #if PROBLEM_TYPE==2
    rankH_t* PTR_rankH_b = (rankH_t*) depv[_idep++].ptr;
    rankH_t* PTR_rankH_t = (rankH_t*) depv[_idep++].ptr;
    #endif

    s64 id = (s64) paramv[0];
    s64 itimestep = (s64) paramv[1];
    s64 ntimesteps = (s64) paramv[2];

    s64 phase = itimestep%2;

    s64 NR = (s64) PTR_globalParamH->NR;
    u64 NR_X = PTR_globalParamH->NR_X;
    s64 id_x = id%NR_X;
    #if PROBLEM_TYPE==2
    u64 NR_Y = PTR_globalParamH->NR_Y;
    s64 id_y = id/NR_X;
    #endif

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

    MyOcrTaskStruct_t TS_timestep;
    #if PROBLEM_TYPE==2
    _paramc = 0; _depc = 12;
    #elif PROBLEM_TYPE==1
    _paramc = 0; _depc = 10;
    #endif

    TS_timestep.FNC = FNC_timestep;
    ocrEdtTemplateCreate( &TS_timestep.TML, TS_timestep.FNC, _paramc, _depc );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

    ocrEdtCreate( &TS_timestep.EDT, TS_timestep.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(&HNT_edt,currentAffinity), &TS_timestep.OET);

    ocrGuid_t TS_timestep_OET;
    ocrEventCreate( &TS_timestep_OET, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_timestep.OET, TS_timestep_OET,   0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( PTR_rankH->DBK_globalParamH_rankCopy, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_rankH->DBK_timers, TS_timestep.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_reductionH, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_rankH->DBK_timer_reductionH, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_rankH->DBK_rankParamH, TS_timestep.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankTemplateH, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_rankH->DBK_rankDataH, TS_timestep.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankEventHs[phase], TS_timestep.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=0) ? PTR_rankH_l->DBK_rankEventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( (id_x!=NR_X-1) ? PTR_rankH_r->DBK_rankEventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    #if PROBLEM_TYPE==2
    ocrAddDependence( (id_y!=0) ? PTR_rankH_b->DBK_rankEventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( (id_y!=NR_Y-1) ? PTR_rankH_t->DBK_rankEventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    #endif

    itimestep += 1;

    if( itimestep <= ntimesteps )
    {
        MyOcrTaskStruct_t TS_rankMultiTimestepper;
        #if PROBLEM_TYPE==2
        _paramc = 3; _depc = 7;
        #elif PROBLEM_TYPE==1
        _paramc = 3; _depc = 5;
        #endif

        u64 compute_paramv[3];
        compute_paramv[0] = (u64) id;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) ntimesteps;

        TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
        ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), NULL );

        _idep = 0;
        ocrAddDependence( PTR_rankH->DBK_globalParamH_rankCopy, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_rankH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_rankH_l, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_rankH_r, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        #if PROBLEM_TYPE==2
        ocrAddDependence( DBK_rankH_b, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_rankH_t, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        #endif
        ocrAddDependence( TS_timestep_OET, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

//( )
//{ DBK_globalParamH<COST>, DBK_rankParamH<RW>, DBK_rankDataH<RW>, DBK_rankEventH<RW>,
//  DBK_events_l<CONST>, DBK_events_r<CONST>, DBK_events_b<CONST>, DBK_events_t<CONST>,
//  DBK_globalH<CONST> }
ocrGuid_t FNC_timestep(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_globalParamH_rankCopy = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_rankParamH = depv[_idep++].guid;
    ocrGuid_t DBK_rankTemplateH = depv[_idep++].guid;
    ocrGuid_t DBK_rankDataH = depv[_idep++].guid;
    ocrGuid_t DBK_rankEventH = depv[_idep++].guid;

    _idep = 0;
    globalParamH_t* PTR_globalParamH_rankCopy = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;
    rankParamH_t* PTR_rankParamH = depv[_idep++].ptr;
    rankTemplateH_t* PTR_rankTemplateH = depv[_idep++].ptr;
    rankDataH_t* PTR_data = (rankDataH_t*) depv[_idep++].ptr;
    rankEventH_t* PTR_rankEvents = (rankEventH_t*) depv[_idep++].ptr;
    rankEventH_t* PTR_rankEvents_l = (rankEventH_t*) depv[_idep++].ptr;
    rankEventH_t* PTR_rankEvents_r = (rankEventH_t*) depv[_idep++].ptr;
    #if PROBLEM_TYPE==2
    rankEventH_t* PTR_rankEvents_b = (rankEventH_t*) depv[_idep++].ptr;
    rankEventH_t* PTR_rankEvents_t = (rankEventH_t*) depv[_idep++].ptr;
    #endif

    s64 NR = (s64) PTR_globalParamH_rankCopy->NR;
    s64 NR_X = (s64) PTR_globalParamH_rankCopy->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;

    #if PROBLEM_TYPE==2
    s64 NR_Y = (s64) PTR_globalParamH_rankCopy->NR_Y;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    #endif

    s64 phase = itimestep%2;

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

    if(itimestep==1) //Do not time iteration 0
    {
        profile_start( total_timer, PTR_timers );
        DEBUG_PRINTF(("Starting profile\n"));
        ocrDbRelease( DBK_timers );
    }

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

    MyOcrTaskStruct_t TS_Lsend; _paramc = 0; _depc = 4;

    TS_Lsend.TML = PTR_rankTemplateH->TML_FNC_Lsend;
    ocrEdtCreate( &TS_Lsend.EDT, TS_Lsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Lsend.OET);

    if( id_x!=0 ) ocrAddDependence( TS_Lsend.OET, PTR_rankEvents_l->EVT_Rrecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Lsend.OET, PTR_rankEvents->EVT_Lsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Lsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Lsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_LsendBufs[phase], TS_Lsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Rsend; _paramc = 0; _depc = 4;

    TS_Rsend.TML = PTR_rankTemplateH->TML_FNC_Rsend;
    ocrEdtCreate( &TS_Rsend.EDT, TS_Rsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Rsend.OET);

    if( id_x != NR_X - 1 ) ocrAddDependence( TS_Rsend.OET, PTR_rankEvents_r->EVT_Lrecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Rsend.OET, PTR_rankEvents->EVT_Rsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Rsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Rsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_RsendBufs[phase], TS_Rsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Lrecv; _paramc = 0; _depc = 4;

    TS_Lrecv.TML = PTR_rankTemplateH->TML_FNC_Lrecv;
    ocrEdtCreate( &TS_Lrecv.EDT, TS_Lrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Lrecv.OET);

    ocrAddDependence( TS_Lrecv.OET, PTR_rankEvents->EVT_Lrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Lrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Lrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=0)?PTR_rankEvents->EVT_Lrecv_start:NULL_GUID, TS_Lrecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Rrecv; _paramc = 0; _depc = 4;

    TS_Rrecv.TML = PTR_rankTemplateH->TML_FNC_Rrecv;
    ocrEdtCreate( &TS_Rrecv.EDT, TS_Rrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Rrecv.OET);

    ocrAddDependence( TS_Rrecv.OET, PTR_rankEvents->EVT_Rrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Rrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Rrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=NR_X-1)?PTR_rankEvents->EVT_Rrecv_start:NULL_GUID, TS_Rrecv.EDT, _idep++, DB_MODE_RO );


    #if PROBLEM_TYPE==2
    //Y direction communication
    MyOcrTaskStruct_t TS_Bsend; _paramc = 0; _depc = 4;

    TS_Bsend.TML = PTR_rankTemplateH->TML_FNC_Bsend;
    ocrEdtCreate( &TS_Bsend.EDT, TS_Bsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Bsend.OET);

    if( id_y!=0 ) ocrAddDependence( TS_Bsend.OET, PTR_rankEvents_b->EVT_Trecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Bsend.OET, PTR_rankEvents->EVT_Bsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Bsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Bsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Bsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_BsendBufs[phase], TS_Bsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Tsend; _paramc = 0; _depc = 4;

    TS_Tsend.TML = PTR_rankTemplateH->TML_FNC_Tsend;
    ocrEdtCreate( &TS_Tsend.EDT, TS_Tsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Tsend.OET);

    if( id_y != NR_Y - 1 ) ocrAddDependence( TS_Tsend.OET, PTR_rankEvents_t->EVT_Brecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Tsend.OET, PTR_rankEvents->EVT_Tsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Tsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Tsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Tsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_TsendBufs[phase], TS_Tsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Brecv; _paramc = 0; _depc = 4;

    TS_Brecv.TML = PTR_rankTemplateH->TML_FNC_Brecv;
    ocrEdtCreate( &TS_Brecv.EDT, TS_Brecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Brecv.OET);

    ocrAddDependence( TS_Brecv.OET, PTR_rankEvents->EVT_Brecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Brecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Brecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Brecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_y!=0)?PTR_rankEvents->EVT_Brecv_start:NULL_GUID, TS_Brecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Trecv; _paramc = 0; _depc = 4;

    TS_Trecv.TML = PTR_rankTemplateH->TML_FNC_Trecv;
    ocrEdtCreate( &TS_Trecv.EDT, TS_Trecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_Trecv.OET);

    ocrAddDependence( TS_Trecv.OET, PTR_rankEvents->EVT_Trecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_Trecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Trecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Trecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_y!=NR_Y-1)?PTR_rankEvents->EVT_Trecv_start:NULL_GUID, TS_Trecv.EDT, _idep++, DB_MODE_RO );

    //end
    #endif

    ocrGuid_t TS_update_OET;
    ocrEventCreate( &TS_update_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update;

    TS_update.TML = PTR_rankTemplateH->TML_FNC_update;
    ocrEdtCreate( &TS_update.EDT, TS_update.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(&HNT_edt,currentAffinity), &TS_update.OET );

    ocrAddDependence( TS_update.OET, TS_update_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_rankCopy, TS_update.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_timers, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_reductionH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_rankParamH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_data->DBK_weight, TS_update.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_data->DBK_xOut, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_data->DBK_refNorm, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_rankEventH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankEvents->EVT_Lsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Rsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Lrecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Rrecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    #if PROBLEM_TYPE==2
    ocrAddDependence( PTR_rankEvents->EVT_Bsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Tsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Brecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_rankEvents->EVT_Trecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    #endif

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_Lsend = depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* lsend = (double*) depv[3].ptr;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    #if PROBLEM_TYPE==2
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;
    #endif

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    int kk = 0;
    #if PROBLEM_TYPE==2
    for( j = jb; j <= je; j++ )
        for( i = ib; i < ib+HALO_RADIUS; i++ )
        {
            lsend[kk++] = IN(i,j);
        }
    #elif PROBLEM_TYPE==1
    for( i = ib; i < ib+HALO_RADIUS; i++ )
    {
        lsend[kk++] = IN(i);
    }
    #endif
#endif

    if( id_x != 0 )
    {
        //DEBUG_PRINTF(("ID=%d FNC_Lsend lsend guid is %lu\n", id, DBK_Lsend));
        return DBK_Lsend;
    }
    else
    {
        //DEBUG_PRINTF(("ID=%d FNC_Lsend lsend guid is NULL\n", id));
        return NULL_GUID;
    }

}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Rsend = depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* rsend = (double*) depv[3].ptr;

    s64 NR_X = PTR_globalParamH_rankCopy->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    #if PROBLEM_TYPE==2
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;
    #endif

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    int kk = 0;
    #if PROBLEM_TYPE==2
    for( j = jb; j <= je; j++ )
        for( i = ie-HALO_RADIUS+1; i <= ie; i++ )
        {
            rsend[kk++] = IN(i,j);
        }
    #elif PROBLEM_TYPE==1
    for( i = ie-HALO_RADIUS+1; i <= ie; i++ )
    {
        rsend[kk++] = IN(i);
    }
    #endif
#endif

    if( id_x != NR_X - 1 )
    {
        //DEBUG_PRINTF(("ID=%d FNC_Rsend rsend guid is %lu\n", id, DBK_Rsend));
        return DBK_Rsend;
    }
    else
    {
        //DEBUG_PRINTF(("ID=%d FNC_Lsend lsend guid is NULL\n", id));
        return NULL_GUID;
    }
}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Rsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* rsent = (double*) depv[3].ptr;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    #if PROBLEM_TYPE==2
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;
    #endif

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Rsend) || id_x != 0 )
    {
        int kk = 0;
        #if PROBLEM_TYPE==2
        for( j = jb; j <= je; j++ )
            for( i = ib-HALO_RADIUS; i < ib; i++ )
            {
                IN(i,j) = rsent[kk++];
            }
        #elif PROBLEM_TYPE==1
        for( i = ib-HALO_RADIUS; i < ib; i++ )
        {
            IN(i) = rsent[kk++];
        }
        #endif

        //DEBUG_PRINTF(("ID=%d FNC_Lrecv lrecv guid is %lu\n", id, DBK_Rsend));
    }
#endif

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Lsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* lsent = (double*) depv[3].ptr;

    s64 NR_X = PTR_globalParamH_rankCopy->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    #if PROBLEM_TYPE==2
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;
    #endif

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Lsend) || id_x != NR_X - 1)
    {
        int kk = 0;
        #if PROBLEM_TYPE==2
        for( j = jb; j <= je; j++ )
            for( i = ie + 1; i < ie + 1 + HALO_RADIUS; i++ )
            {
                IN(i,j) = lsent[kk++];
            }
        #elif PROBLEM_TYPE==1
        for( i = ie + 1; i < ie + 1 + HALO_RADIUS; i++ )
        {
            IN(i) = lsent[kk++];
        }
        #endif

        //DEBUG_PRINTF(("ID=%d FNC_Rrecv rrecv guid is %lu\n", id, DBK_Lsend));
    }
#endif

    return NULL_GUID;
}

#if PROBLEM_TYPE==2
//y-direction
// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Bsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* bsend = (double*) depv[3].ptr;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = jb; j < jb+HALO_RADIUS; j++ )
        for( i = ib; i <= ie; i++ )
        {
            bsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != 0 )
    {
        //DEBUG_PRINTF(("ID=%d FNC_Bsend bsend guid is %lu\n", id, DBK_Bsend));
        return DBK_Bsend;
    }
    else
    {
        //DEBUG_PRINTF(("ID=%d FNC_Bsend bsend guid is NULL\n", id));
        return NULL_GUID;
    }

}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Tsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* tsend = (double*) depv[3].ptr;

    s64 NR_Y = PTR_globalParamH_rankCopy->NR_Y;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = je - HALO_RADIUS + 1; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            tsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != NR_Y - 1 )
    {
        //DEBUG_PRINTF(("ID=%d FNC_Tsend tsend guid is %lu\n", id, DBK_Tsend));
        return DBK_Tsend;
    }
    else
    {
        //DEBUG_PRINTF(("ID=%d FNC_Bsend bsend guid is NULL\n", id));
        return NULL_GUID;
    }
}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Brecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* tsent = (double*) depv[3].ptr;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Tsend) || id_y != 0 )
    {
        int kk = 0;
        for( j = jb-HALO_RADIUS; j < jb; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = tsent[kk++];
            }

        //DEBUG_PRINTF(("ID=%d FNC_Brecv lrecv guid is %lu\n", id, DBK_Tsend));
    }
#endif

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH_rankCopy<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Trecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH_rankCopy = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* bsent = (double*) depv[3].ptr;

    s64 NR_Y = PTR_globalParamH_rankCopy->NR_Y;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Bsend) || id_y != NR_Y - 1)
    {
        int kk = 0;
        for( j = je + 1; j <= je + HALO_RADIUS; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = bsent[kk++];
            }

        //DEBUG_PRINTF(("ID=%d FNC_Trecv rrecv guid is %lu\n", id, DBK_Bsend));
    }
#endif

    return NULL_GUID;
}
#endif

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<RW>,
//   DBK_weight<CONST>, DBK_xIn<RW>, DBK_xOut<RW>, DBK_refNorm<RW>, DBK_rankEventH<RW>,
//   EVT_Lsend_fin, EVT_Rsend_fin, EVT_Bsend_fin, EVT_Tsend_fin,
//   EVT_Lrecv_fin, EVT_Rrecv_fin, EVT_Brecv_fin, EVT_Trecv_fin }
ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_timers = depv[1].guid;
    ocrGuid_t DBK_reductionH = depv[2].guid;
    ocrGuid_t DBK_timer_reductionH = depv[3].guid;
    ocrGuid_t DBK_refNorm = depv[8].guid;

    _idep = 0;
    globalParamH_t* PTR_globalParamH = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;
    rankParamH_t* PTR_rankParamH = depv[_idep++].ptr;
    double *restrict weight = (double*) depv[_idep++].ptr;
    double *restrict xIn = (double*) depv[_idep++].ptr;
    double *restrict xOut = (double*) depv[_idep++].ptr;
    double* refNorm = (double*) depv[_idep++].ptr;
    rankEventH_t* PTR_rankEvents = (rankEventH_t*) depv[_idep++].ptr;

    s64 NR = PTR_globalParamH->NR;
    s64 NP_X = PTR_globalParamH->NP_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    #if PROBLEM_TYPE==2
    s64 NP_Y = PTR_globalParamH->NP_Y;
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;
    #endif

    DEBUG_PRINTF(("ID:%d %s timestep %d\n", id, __func__, itimestep));

    /* Apply the stencil operator */
#if FULL_APP==1
    //HALO_RADIUS is hard-coded for better compiler optimzation here
    int i, j, ii, jj;
    #if PROBLEM_TYPE==2
    for (j=MAX(jb,HALO_RADIUS); j<=MIN(NP_Y-HALO_RADIUS-1,je); j++)
    {
        for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
        {
            for (jj=-HALO_RADIUS; jj<=HALO_RADIUS; jj++) {
                OUT(i,j) += WEIGHT(0,jj)*IN(i,j+jj);
            }
            for (ii=-HALO_RADIUS; ii<0; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
            }
            for (ii=1; ii<=HALO_RADIUS; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
            }
        }
    }

    /* add constant to solution to force refresh of neighbor data, if any */
    for (j=jb; j<=je; j++) for (i=ib; i<=ie; i++) IN(i,j)+= 1.0;
    #elif PROBLEM_TYPE==1
    for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
    {
        for (ii=-HALO_RADIUS; ii<=HALO_RADIUS; ii++) {
            OUT(i) += WEIGHT(ii)*IN(i+ii);
            //DEBUG_PRINTF(("i %d WEIGHT %f IN %f\n", i, WEIGHT(ii), IN(i+ii) ));
        }
    }

    /* add constant to solution to force refresh of neighbor data, if any */
    for (i=ib; i<=ie; i++) IN(i)+= 1.0;

    #endif
#endif

    ocrEventDestroy( PTR_rankEvents->EVT_Lsend_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Rsend_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Lrecv_start );
    ocrEventDestroy( PTR_rankEvents->EVT_Rrecv_start );
    ocrEventDestroy( PTR_rankEvents->EVT_Lrecv_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Rrecv_fin );

    ocrEventCreate( &PTR_rankEvents->EVT_Lsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_rankEvents->EVT_Rsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_rankEvents->EVT_Lrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_rankEvents->EVT_Rrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_rankEvents->EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &PTR_rankEvents->EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );

    #if PROBLEM_TYPE==2
    ocrEventDestroy( PTR_rankEvents->EVT_Bsend_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Tsend_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Brecv_start );
    ocrEventDestroy( PTR_rankEvents->EVT_Trecv_start );
    ocrEventDestroy( PTR_rankEvents->EVT_Brecv_fin );
    ocrEventDestroy( PTR_rankEvents->EVT_Trecv_fin );

    ocrEventCreate( &PTR_rankEvents->EVT_Bsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_rankEvents->EVT_Tsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_rankEvents->EVT_Brecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_rankEvents->EVT_Trecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_rankEvents->EVT_Brecv_fin, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &PTR_rankEvents->EVT_Trecv_fin, OCR_EVENT_STICKY_T, false );
    #endif

    PTR_rankParamH->itimestep = itimestep + 1;

    if( itimestep == PTR_globalParamH->NT )
    {
#if FULL_APP==1

        #if PROBLEM_TYPE==2
        for (j=MAX(jb,HALO_RADIUS); j<=MIN(NP_Y-HALO_RADIUS-1,je); j++)
            for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
                refNorm[0] += (double) ABS(OUT(i,j));
        #elif PROBLEM_TYPE==1
        for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
            refNorm[0] += (double) ABS(OUT(i));
        #endif
#endif
        ocrDbRelease(DBK_refNorm);

        profile_stop( total_timer, PTR_timers );
        double stencil_time = get_elapsed_time( total_timer, PTR_timers );
        //DEBUG_PRINTF(("time %f\n", stencil_time));
        ocrDbRelease(DBK_timers);

        reductionLaunch(PTR_reductionH, DBK_reductionH, refNorm);

        reductionLaunch(PTR_timer_reductionH, DBK_timer_reductionH, &stencil_time);
    }

    return NULL_GUID;
}

//( NR )
//{ EVT_IN_reduction <RO>, EVT_IN_reduction <RO>, ....., EVT_IN_reduction <RO> } //NR input events
ocrGuid_t FNC_reduction(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));

    double* PTR_norm;
    ocrGuid_t DBK_norm;
    u8 ret = ocrDbCreate( &DBK_norm, (void**) &PTR_norm, sizeof(double)*1,
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    PTR_norm[0] = 0.0;

    s64 NR = paramv[0];

#if FULL_APP==1
    s64 I;
    double refNorm = 0.0;

    for( I = 0; I < NR; I++ )
    {
        //DEBUG_PRINTF(("NR %d \n", I));
        double* norm = depv[I].ptr;
        refNorm += norm[0];

    }
    PTR_norm[0] = refNorm;
#endif

    return DBK_norm;
}

//( )
//{ DBK_globalParamH<CONST>, DBK_timers<RW>, EVT_OUT_norm_reduction <RW>, EVT_control_dep<NULL> }
ocrGuid_t FNC_summary(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));
    //
    ocrGuid_t DBK_norm = depv[2].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    double* stencil_time = depv[1].ptr;
    double* PTR_norm = depv[2].ptr;

    //profile_stop( total_timer, PTR_timers );

    s64 NT = PTR_globalParamH->NT;
    s64 NR = PTR_globalParamH->NR;
    s64 NP_X = PTR_globalParamH->NP_X;

    #if PROBLEM_TYPE==2
    s64 NP_Y = PTR_globalParamH->NP_Y;
    double f_active_points = (double) ( (NP_X - 2*HALO_RADIUS) * (NP_Y - 2*HALO_RADIUS) );
    double reference_norm = (double) (NT+1)*2;
    int stencil_size = 4*HALO_RADIUS + 1;
    #elif PROBLEM_TYPE==1
    double reference_norm = (double) (NT+1)*1;
    double f_active_points = (double) ( (NP_X - 2*HALO_RADIUS) );
    int stencil_size = 2*HALO_RADIUS + 1;
    #endif

#if FULL_APP==1
    PTR_norm[0] /= f_active_points;

    if( ABS( PTR_norm[0] - reference_norm ) > EPSILON )
    {
        PRINTF( "ERROR: L1 norm = %f, Reference L1 norm = %f\n", PTR_norm[0], reference_norm);
    }
    else
    {
        DEBUG_PRINTF(( "SUCCESS: L1 norm = %f, Reference L1 norm = %f\n", PTR_norm[0], reference_norm));
        PRINTF( "Solution validates\n" );
    }
#endif

    //double stencil_time = get_elapsed_time( total_timer, PTR_timers );
    double avgtime = stencil_time[0]/(double)NT;

    double flops = (double) (2*stencil_size+1) * f_active_points;
    PRINTF("Rate (MFlops/s): %f  Avg time (s): %f\n",
           1.0E-06 * flops/avgtime, avgtime);

    ocrShutdown();

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    DEBUG_PRINTF(("%s\n", __func__));

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t* PTR_globalH = depv[0].ptr;

    ocrDbDestroy( DBK_globalH );

    ocrShutdown();

    return NULL_GUID;
}
