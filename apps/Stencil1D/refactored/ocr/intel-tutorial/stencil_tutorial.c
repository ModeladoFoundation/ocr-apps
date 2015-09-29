/*
    Author: Chandra S. Martha
    Copywrite Intel Corporation 2015

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.
*/

/* 1-D stencil is implemented in OCR making use of rank data structures to
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
#ifndef TG_ARCH
#include "time.h"
#endif
#include "timers.h"
#include <stdlib.h>
#include <math.h>

#include "stencil.h"

ocrGuid_t FNC_paramInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_init_globalParamH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_init_rankDataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_globalMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankMultiTimestepSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_rankMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_timestep(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    #if PROBLEM_TYPE==2
                    ocrGuid_t FNC_Bsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Tsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Brecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Trecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    #endif
                    //ocrGuid_t FNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]); //Verification is done at the end
                    ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_reduction(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_timer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    void* programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);

    s64 npoints, nranks, ntimesteps, ntimesteps_sync, itimestep0, halo_radius;

    npoints = NPOINTS;
    nranks = NRANKS;
    ntimesteps = NTIMESTEPS;
    ntimesteps_sync = NTIMESTEPS_SYNC;
    itimestep0 = ITIMESTEP0;
    halo_radius = HALO_RADIUS;

    if (argc == 7)
    {
        u32 i = 1;
        npoints = (s64) atoi(getArgv(programArgv, i++));
        nranks = (s64) atoi(getArgv(programArgv, i++));
        ntimesteps = (s64) atoi(getArgv(programArgv, i++));
        ntimesteps_sync = (s64) atoi(getArgv(programArgv, i++));
        itimestep0 = (s64) atoi(getArgv(programArgv, i++));
        halo_radius = (s64) atoi(getArgv(programArgv, i++));

        npoints = (npoints != -1) ? npoints : NPOINTS;
        nranks = (nranks != -1) ? nranks : NRANKS;
        ntimesteps = (ntimesteps != -1) ? ntimesteps : NTIMESTEPS;
        ntimesteps_sync = (ntimesteps_sync != -1) ? ntimesteps_sync : NTIMESTEPS_SYNC;
        itimestep0 = (itimestep0 != -1) ? itimestep0 : ITIMESTEP0;
        halo_radius = (halo_radius != -1) ? halo_radius : HALO_RADIUS;

    }

    ocrGuid_t DBK_globalParamH_0;
    globalParamH_t* PTR_globalParamH_0;

    ocrDbCreate( &DBK_globalParamH_0, (void **) &PTR_globalParamH_0, sizeof(globalParamH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TS_paramInit_OET;
    ocrEventCreate( &TS_paramInit_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_paramInit;

    u64 paramInit_paramv[6] = { (u64) npoints, (u64) nranks, (u64) ntimesteps, (u64) ntimesteps_sync, (u64) itimestep0, (u64) halo_radius };

    TS_paramInit.FNC = FNC_paramInit;
    ocrEdtTemplateCreate( &TS_paramInit.TML, TS_paramInit.FNC, 6, 1 );

    ocrEdtCreate( &TS_paramInit.EDT, TS_paramInit.TML,
                  EDT_PARAM_DEF, paramInit_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_paramInit.OET );

    ocrAddDependence( TS_paramInit.OET, TS_paramInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_0, TS_paramInit.EDT, _idep++, DB_MODE_RW );

    ocrGuid_t DBK_globalH;
    globalH_t* PTR_globalH;

    ocrDbCreate( &DBK_globalH, (void **) &PTR_globalH, sizeof(globalH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TS_globalInit_OET, TS_globalCompute_OET, TS_globalFinalize_OET;

    ocrEventCreate( &TS_globalInit_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalCompute_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalFinalize_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalInit, TS_globalCompute, TS_globalFinalize;

    TS_globalInit.FNC = FNC_globalInit;
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, 0, 3 );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_0, TS_globalInit.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_globalInit.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_paramInit_OET, TS_globalInit.EDT, _idep++, DB_MODE_NULL );

    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    TS_globalFinalize.FNC = FNC_globalFinalize;
    ocrEdtTemplateCreate( &TS_globalFinalize.TML, TS_globalFinalize.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalFinalize.EDT, TS_globalFinalize.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalFinalize.OET );

    ocrAddDependence( TS_globalFinalize.OET, TS_globalFinalize_OET, 0, DB_MODE_NULL);

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalFinalize.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalCompute_OET, TS_globalFinalize.EDT, _idep++, DB_MODE_NULL);

    return NULL_GUID;
}

// ( (u64) npoints, (u64) nranks, (u64) ntimesteps, (u64) ntimesteps_sync, (u64) itimestep0, (u64) halo_radius )
// { DBK_globalParamH_0 <RW> }
ocrGuid_t FNC_paramInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    globalParamH_t* PTR_globalParamH_0 = depv[0].ptr;

    PTR_globalParamH_0->NP = (s64) paramv[0];
    PTR_globalParamH_0->NR = (s64) paramv[1];
    PTR_globalParamH_0->NT = (s64) paramv[2];
    PTR_globalParamH_0->NT_SYNC = (s64) paramv[3];
    PTR_globalParamH_0->IT0 = (s64) paramv[4];
    PTR_globalParamH_0->HR = (s64) paramv[5];

    return NULL_GUID;
}

// ( print_info )
// { DBK_globalParamH_0 <RO>, DBK_globalParamH <RW> }
ocrGuid_t FNC_init_globalParamH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    globalParamH_t* PTR_globalParamH_0 = depv[0].ptr;
    globalParamH_t* PTR_globalParamH = depv[1].ptr;

    PTR_globalParamH->NP = PTR_globalParamH_0->NP; //squre global tile is assumed; side
    PTR_globalParamH->NR = PTR_globalParamH_0->NR;
    PTR_globalParamH->NT = PTR_globalParamH_0->NT;
    PTR_globalParamH->NT_SYNC = PTR_globalParamH_0->NT_SYNC;
    PTR_globalParamH->IT0 = PTR_globalParamH_0->IT0;
    PTR_globalParamH->HR = PTR_globalParamH_0->HR;

    #if PROBLEM_TYPE==2
    PTR_globalParamH->NP_X = (s64) PTR_globalParamH_0->NP; //Simplified for now
    PTR_globalParamH->NP_Y = (s64) PTR_globalParamH_0->NP; //Simplified for now

    int Num_procs = PTR_globalParamH_0->NR;
    int Num_procsx, Num_procsy;
    for (Num_procsx=(int) (sqrt(Num_procs+1)); Num_procsx>0; Num_procsx--)
    {
        if (!(Num_procs%Num_procsx))
        {
            Num_procsy = Num_procs/Num_procsx;
            break;
        }
    }

    PTR_globalParamH->NR_X = (s64) Num_procsx;
    PTR_globalParamH->NR_Y = (s64) Num_procsy;
    //PRINTF("NR_X = %d NR_Y = %d\n", PTR_globalParamH->NR_X, PTR_globalParamH->NR_Y);
    #elif PROBLEM_TYPE==1
    PTR_globalParamH->NP_X = (s64) PTR_globalParamH_0->NP;
    PTR_globalParamH->NR_X = (s64) PTR_globalParamH_0->NR;
    #endif

    if( paramc == 1 && paramv[0] == 1 )
    {
    PRINTF("\n");
    PRINTF("OCR stencil execution on 2D grid\n");
    #if PROBLEM_TYPE==2
    PRINTF("Grid size                   = %dx%d\n", PTR_globalParamH->NP_X, PTR_globalParamH->NP_Y);
    #elif PROBLEM_TYPE==1
    PRINTF("Grid size                   = %d\n", PTR_globalParamH->NP_X);
    #endif
    PRINTF("Number of tiles             = %d\n", PTR_globalParamH->NR);
    #if PROBLEM_TYPE==2
    PRINTF("Tiles in x & y-directions   = %dx%d\n", PTR_globalParamH->NR_X, PTR_globalParamH->NR_Y);
    #elif PROBLEM_TYPE==1
    PRINTF("Tiles in x                  = %d\n", PTR_globalParamH->NR_X);
    #endif
    PRINTF("Radius of stencil           = %d\n", PTR_globalParamH->HR);
    PRINTF("Type of stencil             = star\n");
    PRINTF("Data type                   = double precision\n");
    PRINTF("Number of iterations        = %d\n", PTR_globalParamH->NT);
    PRINTF("\n");
    }

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH_0 <RO>, DBK_globalH <RW>, EVT_paramInit <NULL> }
ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH_0 = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    globalParamH_t* PTR_globalParamH_0 = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;

    u64 NR = PTR_globalParamH_0->NR;
    u64 IT0 = PTR_globalParamH_0->IT0;

    PTR_globalH->itimestep = IT0;

    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &(PTR_globalH->DBK_globalParamH), (void **) &PTR_globalParamH, sizeof(globalParamH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    //paramInit
    ocrGuid_t TS_init_globalParamH_OET;
    ocrEventCreate( &TS_init_globalParamH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_globalParamH;

    TS_init_globalParamH.FNC = FNC_init_globalParamH;
    ocrEdtTemplateCreate( &TS_init_globalParamH.TML, TS_init_globalParamH.FNC, 1, 2 );

    u64 print_info = 1;

    ocrEdtCreate( &TS_init_globalParamH.EDT, TS_init_globalParamH.TML,
                  EDT_PARAM_DEF, &print_info, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_init_globalParamH.OET );

    ocrAddDependence( TS_init_globalParamH.OET, TS_init_globalParamH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_0, TS_init_globalParamH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_init_globalParamH.EDT, _idep++, DB_MODE_RW );
    ocrDbRelease( PTR_globalH->DBK_globalParamH);

    ocrGuid_t* PTR_globalParamHs;
    ocrDbCreate( &PTR_globalH->DBK_globalParamHs, (void **) &PTR_globalParamHs, sizeof(ocrGuid_t)*NR,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t* PTR_rankHs;
    ocrDbCreate( &PTR_globalH->DBK_rankHs, (void **) &PTR_rankHs, sizeof(ocrGuid_t)*NR,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    MyOcrTaskStruct_t TS_rankInitSpawner; _paramc = 0; _depc = 4;

    TS_rankInitSpawner.FNC = FNC_rankInitSpawner;
    ocrEdtTemplateCreate( &TS_rankInitSpawner.TML, TS_rankInitSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_rankInitSpawner.EDT, TS_rankInitSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL ); //Fires up individual EDTs to allocate subdomain DBs, EDTs

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_globalParamHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_globalParamH_OET, TS_rankInitSpawner.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_globalParamHs <RW>, DBK_rankHs <RW>, EVT_init_gSettings <NULL> }
ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalParamHs = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    ocrGuid_t* PTR_globalParamHs = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 NR = PTR_globalParamH->NR;

    //spawn N intializer EDTs
    //PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankInit; _paramc = 1; _depc = 3;

    u64 init_paramv[1] = {-1}; //each spawned EDT gets its rank or id

    TS_rankInit.FNC = FNC_rankInit;
    ocrEdtTemplateCreate( &TS_rankInit.TML, TS_rankInit.FNC, _paramc, _depc );

#ifdef ENABLE_EXTENSION_AFFINITY
    u64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    ocrGuid_t DBK_affinityGuids;
    ocrGuid_t* PTR_affinityGuids;
    ocrDbCreate( &DBK_affinityGuids, (void**) &PTR_affinityGuids, sizeof(ocrGuid_t)*affinityCount,
                 DB_PROP_SINGLE_ASSIGNMENT, NULL_GUID, NO_ALLOC );
    ocrAffinityGet( AFFINITY_PD, &affinityCount, PTR_affinityGuids );
    ASSERT( affinityCount >= 1 );
    PRINTF("Using affinity API\n");
#else
    PRINTF("NOT Using affinity API\n");
#endif

    ocrGuid_t currentAffinity = NULL_GUID;

    s64 i;
    for( i = 0; i < NR; i++ )
    {
#ifdef ENABLE_EXTENSION_AFFINITY
        currentAffinity = PTR_affinityGuids[i%affinityCount];
#endif
        globalParamH_t* PTR_globalParamH;
        ocrDbCreate( &(PTR_globalParamHs[i]), (void **) &PTR_globalParamH, sizeof(globalParamH_t),
                     DB_PROP_NONE, currentAffinity, NO_ALLOC );

        rankH_t *PTR_rankH;
        ocrDbCreate( &(PTR_rankHs[i]), (void **) &PTR_rankH, sizeof(rankH_t),
                     DB_PROP_NONE, currentAffinity, NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate( &TS_rankInit.EDT, TS_rankInit.TML,
                      EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, currentAffinity, NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalParamH, TS_rankInit.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_globalParamHs[i], TS_rankInit.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_rankHs[i], TS_rankInit.EDT, _idep++, DB_MODE_RW );
    }

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH <RO>, DBK_globalParamHs_i <RW>, DBK_rankHs_i <RW> }
ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH_0 = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DBK_globalParamH = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_rankH = (ocrGuid_t) depv[2].guid;

    rankH_t *PTR_rankH = (rankH_t*) depv[2].ptr;

    ocrGuid_t currentAffinity = NULL_GUID;

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
#endif

    //paramInit
    ocrGuid_t TS_init_globalParamH_OET;
    ocrEventCreate( &TS_init_globalParamH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_globalParamH; _paramc = 0; _depc = 2;

    TS_init_globalParamH.FNC = FNC_init_globalParamH;
    ocrEdtTemplateCreate( &TS_init_globalParamH.TML, TS_init_globalParamH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_globalParamH.EDT, TS_init_globalParamH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_init_globalParamH.OET );

    ocrAddDependence( TS_init_globalParamH.OET, TS_init_globalParamH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH_0, TS_init_globalParamH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalParamH, TS_init_globalParamH.EDT, _idep++, DB_MODE_RW );

    rankParamH_t *PTR_rankParamH;
    ocrDbCreate( &(PTR_rankH->DBK_rankParamH), (void **) &PTR_rankParamH, sizeof(rankParamH_t),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    rankDataH_t *PTR_rankDataH;
    ocrDbCreate( &(PTR_rankH->DBK_rankDataH), (void **) &PTR_rankDataH, sizeof(rankDataH_t),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );

    rankTemplateH_t *PTR_rankTemplateH;
    ocrDbCreate( &(PTR_rankH->DBK_rankTemplateH), (void **) &PTR_rankTemplateH, sizeof(rankTemplateH_t),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    s32 i;
    for( i = 0; i < 2; i++ )
    {
        rankEventH_t* PTR_rankEventH;
        ocrDbCreate(    &(PTR_rankH->DBK_rankEventHs[i]), (void **) &PTR_rankEventH, sizeof(rankEventH_t),
                        DB_PROP_NONE, currentAffinity, NO_ALLOC );
    }

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 1; _depc = 7;

    TS_init_rankH.FNC = FNC_init_rankH;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_init_rankH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_rankParamH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankDataH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankEventHs[0], TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankEventHs[1], TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_rankTemplateH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_globalParamH_OET, TS_init_rankH.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

// ( id )
// { DBK_globalParamH <RO>,
//   DBK_rankParamH <RW>, DBK_rankDataH <RW>, DBK_rankEventHs_0 <RW>, DBK_rankEventHs_1 <RW>,
//   EVT_init_globalParamH <NULL>}
ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s64 id = (s64) paramv[0];

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_rankParamH = depv[1].guid;

    globalParamH_t *PTR_globalParamH = (globalParamH_t*) depv[0].ptr;
    rankParamH_t *PTR_rankParamH = (rankParamH_t*) depv[1].ptr;
    rankDataH_t *PTR_rankDataH = (rankDataH_t*) depv[2].ptr;

    ocrGuid_t currentAffinity = NULL_GUID;

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
#endif

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

    PTR_rankParamH->itimestep = PTR_globalParamH->IT0;
    s64 HR = PTR_globalParamH->HR;

    double *xIn, *xOut, *weight, *refNorm;
    #if PROBLEM_TYPE==2
    ocrDbCreate( &(PTR_rankDataH->DBK_xIn), (void **) &xIn, sizeof(double)*(np_x+2*HR)*(np_y+2*HR),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_xOut), (void **) &xOut, sizeof(double)*(np_x)*(np_y),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_weight), (void **) &weight, sizeof(double)*(2*HR+1)*(2*HR+1),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    #elif PROBLEM_TYPE==1
    ocrDbCreate( &(PTR_rankDataH->DBK_xIn), (void **) &xIn, sizeof(double)*(np_x+2*HR),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_xOut), (void **) &xOut, sizeof(double)*(np_x),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_weight), (void **) &weight, sizeof(double)*(2*HR+1),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    #endif
    ocrDbCreate( &(PTR_rankDataH->DBK_refNorm), (void **) &refNorm, sizeof(double)*(1),
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );

    double *lsend, *rsend;
    #if PROBLEM_TYPE==2
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[0]), (void **) &lsend, sizeof(double)*HR*np_y,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[1]), (void **) &lsend, sizeof(double)*HR*np_y,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[0]), (void **) &rsend,  sizeof(double)*HR*np_y,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[1]), (void **) &rsend,  sizeof(double)*HR*np_y,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );

    double *tsend, *bsend;
    ocrDbCreate( &(PTR_rankDataH->DBK_TsendBufs[0]), (void **) &tsend,  sizeof(double)*HR*np_x,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_TsendBufs[1]), (void **) &tsend,  sizeof(double)*HR*np_x,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_BsendBufs[0]), (void **) &bsend, sizeof(double)*HR*np_x,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_BsendBufs[1]), (void **) &bsend, sizeof(double)*HR*np_x,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    #elif PROBLEM_TYPE==1
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[0]), (void **) &lsend, sizeof(double)*HR,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_LsendBufs[1]), (void **) &lsend, sizeof(double)*HR,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[0]), (void **) &rsend,  sizeof(double)*HR,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    ocrDbCreate( &(PTR_rankDataH->DBK_RsendBufs[1]), (void **) &rsend,  sizeof(double)*HR,
                 DB_PROP_NONE, currentAffinity, NO_ALLOC );
    #endif

    MyOcrTaskStruct_t TS_init_rankDataH; _paramc = 0; _depc = 6;

    TS_init_rankDataH.FNC = FNC_init_rankDataH;
    ocrEdtTemplateCreate( &TS_init_rankDataH.TML, TS_init_rankDataH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankDataH.EDT, TS_init_rankDataH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_init_rankDataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_rankParamH, TS_init_rankDataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_xIn, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_xOut, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_weight, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_refNorm, TS_init_rankDataH.EDT, _idep++, DB_MODE_RW );

    s64 i;
    for( i = 0; i < 2; i++ )
    {
        rankEventH_t *PTR_rankEventH = (rankEventH_t*) depv[3+i].ptr;

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

        ocrEventCreate( &PTR_rankEventH->EVT_reduction, OCR_EVENT_STICKY_T, true );
    }

    rankTemplateH_t *PTR_rankTemplateH = depv[5].ptr;

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
    _paramc = 0; _depc = 16;
    #elif PROBLEM_TYPE==1
    _paramc = 0; _depc = 12;
    #endif
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_update), FNC_update, _paramc, _depc );

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<RO>, DBK_rankParamH<RO>,
//   DBK_xIn<RW>, DBK_xOut<RW>, DBK_weight<RW>, DBK_refNorm<RW> }
ocrGuid_t FNC_init_rankDataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);

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

    s64 HR = PTR_globalParamH->HR;

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
    for (jj=-HR; jj<=HR; jj++) for (ii=-HR; ii<=HR; ii++)
        WEIGHT(ii,jj) = (double) 0.0;

    //stencil_size = 4*HR+1;
    for (ii=1; ii<=HR; ii++) {
        WEIGHT(0, ii) = WEIGHT( ii,0) =  (double) (1.0/(2.0*ii*HR));
        WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(double) (1.0/(2.0*ii*HR));
    }
    #elif PROBLEM_TYPE==1
    for( i = ib; i <= ie; i++ )
    {
        IN(i) = (double) ( i );
        OUT(i) = 0.;
    }

    /* fill the stencil weights to reflect a discrete divergence operator         */
    for (ii=-HR; ii<=HR; ii++)
        WEIGHT(ii) = (double) 0.0;

    //stencil_size = 2*HR+1;
    for (ii=1; ii<=HR; ii++)
    {
        WEIGHT(ii) = (double) (1.0/(2.0*ii*HR));
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
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t *PTR_globalH = depv[0].ptr;

    timer* PTR_timers;
    ocrDbCreate( &PTR_globalH->DBK_timers, (void**) &PTR_timers, sizeof(timer)*number_of_timers,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    int i;
    for( i = 0; i < number_of_timers; i++ )
    {
        PTR_timers[i].start = 0;
        PTR_timers[i].total = 0;
        PTR_timers[i].count = 0;
        PTR_timers[i].elapsed = 0;
    }
    profile_start( total_timer, PTR_timers );
    ocrDbRelease(PTR_globalH->DBK_timers);

    ocrGuid_t TS_globalMultiTimestepper_OET;
    ocrEventCreate( &TS_globalMultiTimestepper_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalMultiTimestepper; _paramc = 1; _depc = 2;

    u64 computeSpawner_paramv[1] = { NULL_GUID };

    TS_globalMultiTimestepper.FNC = FNC_globalMultiTimestepper;
    ocrEdtTemplateCreate( &TS_globalMultiTimestepper.TML, TS_globalMultiTimestepper.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalMultiTimestepper.EDT, TS_globalMultiTimestepper.TML,
                  EDT_PARAM_DEF, computeSpawner_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalMultiTimestepper.OET );

    ocrAddDependence( TS_globalMultiTimestepper.OET, TS_globalMultiTimestepper_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_timer; _paramc = 0; _depc = 3;

    TS_timer.FNC = FNC_timer;
    ocrEdtTemplateCreate( &TS_timer.TML, TS_timer.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_timer.EDT, TS_timer.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_timer.OET );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_globalParamH, TS_timer.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->DBK_timers, TS_timer.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalMultiTimestepper_OET, TS_timer.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

//( )
//{ DBK_globalParamH<CONST>, DBK_timers<CONST>, EVT_control_dep<NULL> }
ocrGuid_t FNC_timer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    timer* PTR_timers = depv[1].ptr;

    profile_stop( total_timer, PTR_timers );

    s64 NT = PTR_globalParamH->NT;
    s64 NR = PTR_globalParamH->NR;
    s64 HR = PTR_globalParamH->HR;
    s64 NP_X = PTR_globalParamH->NP_X;

    #if PROBLEM_TYPE==2
    s64 NP_Y = PTR_globalParamH->NP_Y;
    double f_active_points = (double) ( (NP_X - 2*HR) * (NP_Y - 2*HR) );
    int stencil_size = 4*HR + 1;
    #elif PROBLEM_TYPE==1
    double f_active_points = (double) ( (NP_X - 2*HR) );
    int stencil_size = 2*HR + 1;
    #endif

    double stencil_time = get_elapsed_time( total_timer, PTR_timers );
    double avgtime = stencil_time/(double)NT;

    double flops = (double) (2*stencil_size+1) * f_active_points;
    PRINTF("Rate (MFlops/s): %f  Avg time (s): %f\n",
           1.0E-06 * flops/avgtime, avgtime);

    return NULL_GUID;
}

//( EVT_control_dep )
//{ DBK_globalParamH<CONST>, DBK_globalH<RW>, EVT_control_dep<NULL> }
ocrGuid_t FNC_globalMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t TS_rankMultiTimestepSpawner_OET_old = (ocrGuid_t) paramv[0];

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;

    ocrGuid_t DBK_globalParamHs = PTR_globalH->DBK_globalParamHs;
    ocrGuid_t DBK_rankHs = PTR_globalH->DBK_rankHs;

    u64 itimestep = (u64) PTR_globalH->itimestep;
    u64 itimestep0 = (u64) (PTR_globalParamH->IT0);

    u64 NT = PTR_globalParamH->NT;
    u64 NT_SYNC = PTR_globalParamH->NT_SYNC;
    u64 NR = PTR_globalParamH->NR;

    if( TS_rankMultiTimestepSpawner_OET_old != NULL_GUID )
        ocrEventDestroy(TS_rankMultiTimestepSpawner_OET_old);

    //PRINTF("%s Timestep %d\n", __func__, itimestep);
    //
    if(itimestep == itimestep0)
    {
        MyOcrTaskStruct_t TS_reduction; _paramc = 0; _depc = NR+1;

        TS_reduction.FNC = FNC_reduction;
        ocrEdtTemplateCreate( &TS_reduction.TML, TS_reduction.FNC, _paramc, _depc );

        ocrEdtCreate( &PTR_globalH->EDT_reduction, TS_reduction.TML,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( PTR_globalH->DBK_globalParamH, PTR_globalH->EDT_reduction, _idep++, DB_MODE_CONST );
        ocrDbRelease( DBK_globalH );
    }

    if( itimestep <= NT )
    {
        ocrGuid_t TS_rankMultiTimestepSpawner_OET;
        MyOcrTaskStruct_t TS_rankMultiTimestepSpawner; _paramc = 0; _depc = 4;

        ocrEventCreate( &TS_rankMultiTimestepSpawner_OET, OCR_EVENT_STICKY_T, false );

        TS_rankMultiTimestepSpawner.FNC = FNC_rankMultiTimestepSpawner;
        ocrEdtTemplateCreate( &TS_rankMultiTimestepSpawner.TML, TS_rankMultiTimestepSpawner.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiTimestepSpawner.EDT, TS_rankMultiTimestepSpawner.TML,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_GUID, &TS_rankMultiTimestepSpawner.OET );

        ocrAddDependence( TS_rankMultiTimestepSpawner.OET, TS_rankMultiTimestepSpawner_OET, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalParamH, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_globalParamHs, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_rankHs, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_CONST );

        itimestep += NT_SYNC;

        if( itimestep <= NT )
        {
            MyOcrTaskStruct_t TS_globalMultiTimestepper; _paramc = 1; _depc = 3;
            u64 mainComputeManager_paramv[1] = { TS_rankMultiTimestepSpawner_OET };

            TS_globalMultiTimestepper.FNC = FNC_globalMultiTimestepper;
            ocrEdtTemplateCreate( &TS_globalMultiTimestepper.TML, TS_globalMultiTimestepper.FNC, _paramc, _depc );

            ocrEdtCreate( &TS_globalMultiTimestepper.EDT, TS_globalMultiTimestepper.TML,
                          EDT_PARAM_DEF, mainComputeManager_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, NULL );

            _idep = 0;
            ocrAddDependence( DBK_globalParamH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
            ocrAddDependence( DBK_globalH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RW );
            ocrAddDependence( TS_rankMultiTimestepSpawner_OET, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_NULL );
        }
    }

    return NULL_GUID;
}

int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}

//( )
//{ DBK_globalParamH<CONST>, DBK_globalH<RW>, DBK_globalParamHs<CONST>, DBK_rankHs<CONST> }
ocrGuid_t FNC_rankMultiTimestepSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalParamH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;
    ocrGuid_t DBK_globalParamHs = depv[2].guid;
    ocrGuid_t DBK_rankHs = depv[3].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;
    ocrGuid_t* PTR_globalParamHs = depv[2].ptr;
    ocrGuid_t* PTR_rankHs = depv[3].ptr;

    u64 NR = PTR_globalParamH->NR;
    u64 NR_X = PTR_globalParamH->NR_X;
    #if PROBLEM_TYPE==2
    u64 NR_Y = PTR_globalParamH->NR_Y;
    #endif
    u64 NT = PTR_globalParamH->NT;
    u64 NT_SYNC = PTR_globalParamH->NT_SYNC;

    u64 itimestep = (u64) PTR_globalH->itimestep;

    u64 I, id_x, id_y;

    //PRINTF("#Subdomains %d\n", NR);
    //
    ocrGuid_t currentAffinity = NULL_GUID;

    MyOcrTaskStruct_t TS_rankMultiTimestepper;
    #if PROBLEM_TYPE==2
    _paramc = 3; _depc = 7;
    #elif PROBLEM_TYPE==1
    _paramc = 3; _depc = 5;
    #endif

    u64 compute_paramv[3];

    TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
    ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

    //PRINTF("Timestep# %d NR = %d\n", itimestep, NR);
    for( I = 0; I < NR; I++ )
    {
        compute_paramv[0] = (u64) I;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) MIN( itimestep + NT_SYNC - 1, NT );

        id_x = I%NR_X;

#ifdef ENABLE_EXTENSION_AFFINITY
        u64 count = 1;
        ocrAffinityQuery(PTR_rankHs[I], &count, &currentAffinity);
#endif

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, currentAffinity, NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_globalParamHs[I], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
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

    PTR_globalH->itimestep += NT_SYNC;

    return NULL_GUID;
}

//( id, itimestep, ntimestep_l )
//{ DBK_globalParamH<CONST>, DBK_rankH<CONST>, DBK_rankH_l<CONST>, DBK_rankH_r<CONST>,
//  DBK_rankH_b<CONST>, DBK_rankH_t<CONST>, DBK_globalH<CONST>, EVT_control_dep<NULL> }
ocrGuid_t FNC_rankMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_globalH = depv[_idep++].guid;
    ocrGuid_t DBK_globalParamH = depv[_idep++].guid;
    ocrGuid_t DBK_rankH =  depv[_idep++].guid;
    ocrGuid_t DBK_rankH_l = depv[_idep++].guid;
    ocrGuid_t DBK_rankH_r = depv[_idep++].guid;
    #if PROBLEM_TYPE==2
    ocrGuid_t DBK_rankH_b = depv[_idep++].guid;
    ocrGuid_t DBK_rankH_t = depv[_idep++].guid;
    #endif

    _idep = 0;
    globalH_t* PTR_globalH = depv[_idep++].ptr;
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
    s64 ntimestep_l = (s64) paramv[2];

    s64 phase = itimestep%2;

    s64 NR = (s64) PTR_globalParamH->NR;
    u64 NR_X = PTR_globalParamH->NR_X;
    s64 id_x = id%NR_X;
    #if PROBLEM_TYPE==2
    u64 NR_Y = PTR_globalParamH->NR_Y;
    s64 id_y = id/NR_X;
    #endif

    ocrGuid_t currentAffinity = NULL_GUID;

    MyOcrTaskStruct_t TS_timestep;
    #if PROBLEM_TYPE==2
    _paramc = 0; _depc = 10;
    #elif PROBLEM_TYPE==1
    _paramc = 0; _depc = 8;
    #endif

    TS_timestep.FNC = FNC_timestep;
    ocrEdtTemplateCreate( &TS_timestep.TML, TS_timestep.FNC, _paramc, _depc );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
#endif

    ocrEdtCreate( &TS_timestep.EDT, TS_timestep.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, currentAffinity, &TS_timestep.OET);

    ocrGuid_t TS_timestep_OET;
    ocrEventCreate( &TS_timestep_OET, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_timestep.OET, TS_timestep_OET,   0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_timestep.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalParamH, TS_timestep.EDT, _idep++, DB_MODE_CONST );
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

    if( itimestep <= ntimestep_l )
    {
        MyOcrTaskStruct_t TS_rankMultiTimestepper;
        #if PROBLEM_TYPE==2
        _paramc = 3; _depc = 8;
        #elif PROBLEM_TYPE==1
        _paramc = 3; _depc = 6;
        #endif

        u64 compute_paramv[3];
        compute_paramv[0] = (u64) id;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) ntimestep_l;

        TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
        ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, currentAffinity, NULL );

        _idep = 0;
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_globalParamH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_CONST );
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
    ocrGuid_t DBK_globalH = depv[_idep++].guid;
    ocrGuid_t DBK_globalParamH = depv[_idep++].guid;
    ocrGuid_t DBK_rankParamH = depv[_idep++].guid;
    ocrGuid_t DBK_rankTemplateH = depv[_idep++].guid;
    ocrGuid_t DBK_rankDataH = depv[_idep++].guid;
    ocrGuid_t DBK_rankEventH = depv[_idep++].guid;

    _idep = 0;
    globalH_t* PTR_globalH = depv[_idep++].ptr;
    globalParamH_t* PTR_globalParamH = depv[_idep++].ptr;
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

    s64 NR = (s64) PTR_globalParamH->NR;
    s64 NR_X = (s64) PTR_globalParamH->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;

    #if PROBLEM_TYPE==2
    s64 NR_Y = (s64) PTR_globalParamH->NR_Y;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    #endif

    s64 phase = itimestep%2;

    //PRINTF("%s id %d x %d y %d\n", __func__, id, id_x, id_y);
    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    ocrGuid_t currentAffinity = NULL_GUID;

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
#endif

    MyOcrTaskStruct_t TS_Lsend; _paramc = 0; _depc = 4;

    TS_Lsend.TML = PTR_rankTemplateH->TML_FNC_Lsend;
    ocrEdtCreate( &TS_Lsend.EDT, TS_Lsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, &TS_Lsend.OET);

    if( id_x!=0 ) ocrAddDependence( TS_Lsend.OET, PTR_rankEvents_l->EVT_Rrecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Lsend.OET, PTR_rankEvents->EVT_Lsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_Lsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Lsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_LsendBufs[phase], TS_Lsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Rsend; _paramc = 0; _depc = 4;

    TS_Rsend.TML = PTR_rankTemplateH->TML_FNC_Rsend;
    ocrEdtCreate( &TS_Rsend.EDT, TS_Rsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, &TS_Rsend.OET);

    if( id_x != NR_X - 1 ) ocrAddDependence( TS_Rsend.OET, PTR_rankEvents_r->EVT_Lrecv_start, 0, DB_MODE_RO );
    ocrAddDependence( TS_Rsend.OET, PTR_rankEvents->EVT_Rsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_Rsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Rsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_RsendBufs[phase], TS_Rsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Lrecv; _paramc = 0; _depc = 4;

    TS_Lrecv.TML = PTR_rankTemplateH->TML_FNC_Lrecv;
    ocrEdtCreate( &TS_Lrecv.EDT, TS_Lrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, &TS_Lrecv.OET);

    ocrAddDependence( TS_Lrecv.OET, PTR_rankEvents->EVT_Lrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_Lrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Lrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=0)?PTR_rankEvents->EVT_Lrecv_start:NULL_GUID, TS_Lrecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Rrecv; _paramc = 0; _depc = 4;

    TS_Rrecv.TML = PTR_rankTemplateH->TML_FNC_Rrecv;
    ocrEdtCreate( &TS_Rrecv.EDT, TS_Rrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, &TS_Rrecv.OET);

    ocrAddDependence( TS_Rrecv.OET, PTR_rankEvents->EVT_Rrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalParamH, TS_Rrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_rankParamH, TS_Rrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=NR_X-1)?PTR_rankEvents->EVT_Rrecv_start:NULL_GUID, TS_Rrecv.EDT, _idep++, DB_MODE_RO );


    #if PROBLEM_TYPE==2
    //Y direction communication
    MyOcrTaskStruct_t TS_Bsend; _paramc = 0; _depc = 4;

    MyOcrTaskStruct_t TS_Tsend; _paramc = 0; _depc = 4;

    MyOcrTaskStruct_t TS_Brecv; _paramc = 0; _depc = 4;

    MyOcrTaskStruct_t TS_Trecv; _paramc = 0; _depc = 4;

    //end
    #endif

    ocrGuid_t TS_update_OET;
    ocrEventCreate( &TS_update_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update;

    TS_update.TML = PTR_rankTemplateH->TML_FNC_update;
    ocrEdtCreate( &TS_update.EDT, TS_update.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, currentAffinity, &TS_update.OET );

    ocrAddDependence( TS_update.OET, TS_update_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_update.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalParamH, TS_update.EDT, _idep++, DB_MODE_CONST );
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
    ocrAddDependence( );
    ocrAddDependence( );
    ocrAddDependence( );
    ocrAddDependence( );
    #endif
    //ocrAddDependence( TS_verify_OET, TS_update.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_Lsend = depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* lsend = (double*) depv[3].ptr;

    s64 HR = PTR_globalParamH->HR;

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

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    //double* lsend;
    //ocrGuid_t DBK_Lsend;
    //ocrDbCreate( &DBK_Lsend, (void **) &lsend, sizeof(double)*HR*np_y,
    //             DB_PROP_NONE, NULL_GUID, NO_ALLOC );

#if FULL_APP==1
    int i, j;
    int kk = 0;
    #if PROBLEM_TYPE==2
    for( j = jb; j <= je; j++ )
        for( i = ib; i < ib+HR; i++ )
        {
            lsend[kk++] = IN(i,j);
        }
    #elif PROBLEM_TYPE==1
    for( i = ib; i < ib+HR; i++ )
    {
        lsend[kk++] = IN(i);
    }
    #endif
#endif

    if( id_x != 0 )
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is %lu\n", id, DBK_Lsend);
        return DBK_Lsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is NULL\n", id);
        return NULL_GUID;
    }

}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Rsend = depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* rsend = (double*) depv[3].ptr;

    s64 NR_X = PTR_globalParamH->NR_X;
    s64 HR = PTR_globalParamH->HR;

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

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    //double* rsend;
    //ocrGuid_t DBK_Rsend;
    //ocrDbCreate( &DBK_Rsend, (void **) &rsend,  sizeof(double)*HR*np_y,
    //             DB_PROP_NONE, NULL_GUID, NO_ALLOC );

#if FULL_APP==1
    int i, j;
    int kk = 0;
    #if PROBLEM_TYPE==2
    for( j = jb; j <= je; j++ )
        for( i = ie-HR+1; i <= ie; i++ )
        {
            rsend[kk++] = IN(i,j);
        }
    #elif PROBLEM_TYPE==1
    for( i = ie-HR+1; i <= ie; i++ )
    {
        rsend[kk++] = IN(i);
    }
    #endif
#endif

    if( id_x != NR_X - 1 )
    {
        //PRINTF("ID=%d FNC_Rsend rsend guid is %lu\n", id, DBK_Rsend);
        return DBK_Rsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is NULL\n", id);
        return NULL_GUID;
    }
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Rsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* rsent = (double*) depv[3].ptr;

    s64 HR = PTR_globalParamH->HR;

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

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( DBK_Rsend != NULL_GUID || id_x != 0 )
    {
        int kk = 0;
        #if PROBLEM_TYPE==2
        for( j = jb; j <= je; j++ )
            for( i = ib-HR; i < ib; i++ )
            {
                IN(i,j) = rsent[kk++];
            }
        #elif PROBLEM_TYPE==1
        for( i = ib-HR; i < ib; i++ )
        {
            IN(i) = rsent[kk++];
        }
        #endif

        //PRINTF("ID=%d FNC_Lrecv lrecv guid is %lu\n", id, DBK_Rsend);
        //ocrDbDestroy( DBK_Rsend );
    }
#endif

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Lsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* lsent = (double*) depv[3].ptr;

    s64 NR_X = PTR_globalParamH->NR_X;
    s64 HR = PTR_globalParamH->HR;

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

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( DBK_Lsend != NULL_GUID || id_x != NR_X - 1)
    {
        int kk = 0;
        #if PROBLEM_TYPE==2
        for( j = jb; j <= je; j++ )
            for( i = ie + 1; i < ie + 1 + HR; i++ )
            {
                IN(i,j) = lsent[kk++];
            }
        #elif PROBLEM_TYPE==1
        for( i = ie + 1; i < ie + 1 + HR; i++ )
        {
            IN(i) = lsent[kk++];
        }
        #endif

        //PRINTF("ID=%d FNC_Rrecv rrecv guid is %lu\n", id, DBK_Lsend);
        //ocrDbDestroy( DBK_Lsend );
    }
#endif

    return NULL_GUID;
}

#if PROBLEM_TYPE==2
//y-direction
// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Bsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* bsend = (double*) depv[3].ptr;

    s64 HR = PTR_globalParamH->HR;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    //double* bsend;
    //ocrGuid_t DBK_Bsend;
    //ocrDbCreate( &DBK_Bsend, (void **) &bsend, sizeof(double)*HR*np_x,
    //             DB_PROP_NONE, NULL_GUID, NO_ALLOC );

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = jb; j < jb+HR; j++ )
        for( i = ib; i <= ie; i++ )
        {
            bsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != 0 )
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is %lu\n", id, DBK_Bsend);
        return DBK_Bsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is NULL\n", id);
        return NULL_GUID;
    }

}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RO>, DBK_sendBufs<RW> }
ocrGuid_t FNC_Tsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* tsend = (double*) depv[3].ptr;

    s64 NR_Y = PTR_globalParamH->NR_Y;
    s64 HR = PTR_globalParamH->HR;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    //double* tsend;
    //ocrGuid_t DBK_Tsend;
    //ocrDbCreate( &DBK_Tsend, (void **) &tsend,  sizeof(double)*HR*np_x,
    //             DB_PROP_NONE, NULL_GUID, NO_ALLOC );

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = je - HR + 1; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            tsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != NR_Y - 1 )
    {
        //PRINTF("ID=%d FNC_Tsend tsend guid is %lu\n", id, DBK_Tsend);
        return DBK_Tsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is NULL\n", id);
        return NULL_GUID;
    }
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Brecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* tsent = (double*) depv[3].ptr;

    s64 HR = PTR_globalParamH->HR;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( DBK_Tsend != NULL_GUID || id_y != 0 )
    {
        int kk = 0;
        for( j = jb-HR; j < jb; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = tsent[kk++];
            }

        //PRINTF("ID=%d FNC_Brecv lrecv guid is %lu\n", id, DBK_Tsend);
        //ocrDbDestroy( DBK_Tsend );
    }
#endif

    return NULL_GUID;
}

// ( )
// { DBK_globalParamH<CONST>, DBK_rankParamH<CONST>, DBK_xIn<RW>, EVT_triger<RW> }
ocrGuid_t FNC_Trecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[3].guid;

    globalParamH_t* PTR_globalParamH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* bsent = (double*) depv[3].ptr;

    s64 NR_Y = PTR_globalParamH->NR_Y;
    s64 HR = PTR_globalParamH->HR;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( DBK_Bsend != NULL_GUID || id_y != NR_Y - 1)
    {
        int kk = 0;
        for( j = je + 1; j <= je + HR; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = bsent[kk++];
            }

        //PRINTF("ID=%d FNC_Trecv rrecv guid is %lu\n", id, DBK_Bsend);
        //ocrDbDestroy( DBK_Bsend );
    }
#endif

    return NULL_GUID;
}
#endif

//ocrGuid_t FNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
//{
//    globalParamH_t* PTR_globalParamH = depv[0].ptr;
//    rankParamH_t* PTR_rankParamH = depv[1].ptr;
//    double* xIn = (double*) depv[2].ptr;
//
//    s64 NR = PTR_globalParamH->NR;
//    s64 HR = PTR_globalParamH->HR;
//    s64 NP_X = PTR_globalParamH->NP_X;
//    s64 NP_Y = PTR_globalParamH->NP_Y;
//
//    s64 id = (s64) PTR_rankParamH->id;
//    s64 itimestep = (s64) PTR_rankParamH->itimestep;
//    s64 np_x = (s64) PTR_rankParamH->np_x;
//    s64 np_y = (s64) PTR_rankParamH->np_y;
//
//    s64 ib = (s64) PTR_rankParamH->ib;
//    s64 ie = (s64) PTR_rankParamH->ie;
//    s64 jb = (s64) PTR_rankParamH->jb;
//    s64 je = (s64) PTR_rankParamH->je;
//
//    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);
//
//    int i, j;
//    double local_norm = (double) 0.0;
//    for( j=MAX(jb,HR); j<=MIN(NP_Y-HR-1,je); j++ )
//        for (i=MAX(ib,HR); i<=MIN(NP_X-HR-1,ie); i++)
//            //local_norm += (double)ABS(OUT(i,j));
//            ;
//
//    return NULL_GUID;
//}

// ( )
// { DBK_globalH<CONST>, DBK_globalParamH<CONST>, DBK_rankParamH<RW>,
//   DBK_weight<CONST>, DBK_xIn<RW>, DBK_xOut<RW>, DBK_refNorm<RW>, DBK_rankEventH<RW>,
//   EVT_Lsend_fin, EVT_Rsend_fin, EVT_Bsend_fin, EVT_Tsend_fin,
//   EVT_Lrecv_fin, EVT_Rrecv_fin, EVT_Brecv_fin, EVT_Trecv_fin }
ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_refNorm = depv[6].guid;

    globalH_t* PTR_globalH = depv[0].ptr;
    globalParamH_t* PTR_globalParamH = depv[1].ptr;
    rankParamH_t* PTR_rankParamH = depv[2].ptr;
    double *restrict weight = (double*) depv[3].ptr;
    double *restrict xIn = (double*) depv[4].ptr;
    double *restrict xOut = (double*) depv[5].ptr;
    double* refNorm = (double*) depv[6].ptr;
    rankEventH_t* PTR_rankEvents = (rankEventH_t*) depv[7].ptr;

    s64 NR = PTR_globalParamH->NR;
    s64 HR = PTR_globalParamH->HR;
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

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);
    //PRINTF("ID: %d ib %d ie %d jb %d je %d\n", id, ib, ie, jb, je);

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
            //PRINTF("i %d WEIGHT %f IN %f\n", i, WEIGHT(ii), IN(i+ii) );
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
        for (j=MAX(jb,HR); j<=MIN(NP_Y-HR-1,je); j++)
        {
            for (i=MAX(ib,HR); i<=MIN(NP_X-HR-1,ie); i++)
            {
                refNorm[0] += (double) ABS(OUT(i,j));
                //PRINTF("%d %d\n", i, j);
                //if( OUT(i,j) != (itimestep)*(2) )
                //{
                //    PRINTF("ID: %d Verification failed: timestep %d\n", id, itimestep);
                //    //break;
                //}
                //else
                //{
                //    PRINTF("ID: %d Verification passed: timestep %d\n", id, itimestep);
                //}

            }
        }
        #elif PROBLEM_TYPE==1
        for (i=MAX(ib,HR); i<=MIN(NP_X-HR-1,ie); i++)
        {
            refNorm[0] += (double) ABS(OUT(i));
            //PRINTF("%d %f\n", i, OUT(i));
            //if( OUT(i) != (itimestep)*(1) )
            //{
            //    PRINTF("ID: %d Verification failed: timestep %d\n", id, itimestep);
            //    //break;
            //}
            //else
            //{
            //    PRINTF("ID: %d Verification passed: timestep %d\n", id, itimestep);
            //}

        }
        #endif
#endif
        ocrEventSatisfy( PTR_rankEvents->EVT_reduction, DBK_refNorm);
        ocrDbRelease(DBK_refNorm);
        ocrAddDependence( PTR_rankEvents->EVT_reduction, PTR_globalH->EDT_reduction, 1+id, DB_MODE_RO );

    }

    return NULL_GUID;
}

ocrGuid_t FNC_reduction(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    ocrGuid_t DBK_globalParamH = depv[0].guid;

    globalParamH_t *PTR_globalParamH = depv[0].ptr;

    s64 NT = PTR_globalParamH->NT;
    s64 NR = PTR_globalParamH->NR;
    s64 HR = PTR_globalParamH->HR;
    s64 NP_X = PTR_globalParamH->NP_X;

    s64 I;
    double refNorm = 0.0;

    for( I = 0; I < NR; I++ )
    {
        double* norm = depv[1+I].ptr;
        refNorm += norm[0];

    }
    #if PROBLEM_TYPE==2
    s64 NP_Y = PTR_globalParamH->NP_Y;
    double f_active_points = (double) ( (NP_X - 2*HR) * (NP_Y - 2*HR) );
    double reference_norm = (double) (NT)*2;
    #elif PROBLEM_TYPE==1
    double f_active_points = (double) ( (NP_X - 2*HR) );
    double reference_norm = (double) (NT)*1;
    #endif

    refNorm /= f_active_points;

    if( ABS( refNorm - reference_norm ) > EPSILON )
    {
        PRINTF( "ERROR: L1 norm = %f, Reference L1 norm = %f\n", refNorm, reference_norm);
    }
    else
    {
        PRINTF( "SUCCESS: L1 norm = %f, Reference L1 norm = %f\n", refNorm, reference_norm);
        PRINTF( "Solution validates\n" );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t* PTR_globalH = depv[0].ptr;

    ocrDbDestroy( DBK_globalH );

    ocrShutdown();

    return NULL_GUID;
}

