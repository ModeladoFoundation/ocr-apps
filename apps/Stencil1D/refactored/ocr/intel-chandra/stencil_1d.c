#include "ocr.h"
#include "ocr-std.h"
#include <stdlib.h>

//default values
#define NPOINTS 4
#define NRANKS 4
#define NTIMESTEPS 10
#define NTIMESTEPS_SYNC 5
#define ITIMESTEP0 1
#define HALO_RADIUS 1

#define NORM 1e-6

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

typedef struct
{
    s64 NP, NR; //global problem size, number of ranks
    s64 NT, NT_SYNC, IT0;
    s64 HR;
} gSettingsH_t;

typedef struct
{
    s64 id, lb, ub, np; // rank id, local bounds, local points
    s64 itimestep;
} settingsH_t;

typedef struct
{
    ocrGuid_t DBK_xIn, DBK_xOut;
} dataH_t;

typedef struct
{
    ocrGuid_t EVT_Lsend_fin, EVT_Rsend_fin;
    ocrGuid_t EVT_Lrecv_start, EVT_Rrecv_start;
    ocrGuid_t EVT_Lrecv_fin, EVT_Rrecv_fin;
} eventH_t;

typedef struct
{
    ocrGuid_t DBK_settingsH; //-->settingsH_t
    ocrGuid_t DBK_dataH; //--> dataH_t
    ocrGuid_t DBK_eventHs[2]; //--> eventH_t[2]
} rankH_t;

typedef struct
{
    s64 itimestep; //tag
    ocrGuid_t DBK_gSettingsH; // Commandline parameters/settings
    ocrGuid_t DBK_gSettingsHs; //-> Broadcasted parameters/settings gSettingsH_t[]
    ocrGuid_t DBK_rankHs; //--> rankH_t[]
} globalH_t;

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e, s64* n);

ocrGuid_t FNC_settingsInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_init_gSettingsH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_init_dataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_globalMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankMultiTimestepSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_rankMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_timestep(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e, s64* n)
{
    s64 N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;

    *n = *e - *s + 1;
}

ocrGuid_t FNC_settingsInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH_0 = depv[0].ptr;

    PTR_gSettingsH_0->NP = (s64) paramv[0];
    PTR_gSettingsH_0->NR = (s64) paramv[1];
    PTR_gSettingsH_0->NT = (s64) paramv[2];
    PTR_gSettingsH_0->NT_SYNC = (s64) paramv[3];
    PTR_gSettingsH_0->IT0 = (s64) paramv[4];
    PTR_gSettingsH_0->HR = (s64) paramv[5];

    return NULL_GUID;
}

ocrGuid_t FNC_init_gSettingsH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH_0 = depv[0].ptr;
    gSettingsH_t* PTR_gSettingsH = depv[1].ptr;

    PTR_gSettingsH->NP = PTR_gSettingsH_0->NP;
    PTR_gSettingsH->NR = PTR_gSettingsH_0->NR;
    PTR_gSettingsH->NT = PTR_gSettingsH_0->NT;
    PTR_gSettingsH->NT_SYNC = PTR_gSettingsH_0->NT_SYNC;
    PTR_gSettingsH->IT0 = PTR_gSettingsH_0->IT0;
    PTR_gSettingsH->HR = PTR_gSettingsH_0->HR;

    return NULL_GUID;
}

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH_0 = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    gSettingsH_t* PTR_gSettingsH_0 = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;

    u64 NR = PTR_gSettingsH_0->NR;
    u64 IT0 = PTR_gSettingsH_0->IT0;

    PTR_globalH->itimestep = IT0;

    gSettingsH_t* PTR_gSettingsH;
    ocrDbCreate( &(PTR_globalH->DBK_gSettingsH), (void **) &PTR_gSettingsH, sizeof(gSettingsH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    //settingsInit
    ocrGuid_t TS_init_gSettingsH_OET;
    ocrEventCreate( &TS_init_gSettingsH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_gSettingsH;

    TS_init_gSettingsH.FNC = FNC_init_gSettingsH;
    ocrEdtTemplateCreate( &TS_init_gSettingsH.TML, TS_init_gSettingsH.FNC, 0, 2 );

    ocrEdtCreate( &TS_init_gSettingsH.EDT, TS_init_gSettingsH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_init_gSettingsH.OET );

    ocrAddDependence( TS_init_gSettingsH.OET, TS_init_gSettingsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH_0, TS_init_gSettingsH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_gSettingsH, TS_init_gSettingsH.EDT, _idep++, DB_MODE_ITW );

    ocrGuid_t* PTR_gSettingsHs;
    ocrDbCreate( &PTR_globalH->DBK_gSettingsHs, (void **) &PTR_gSettingsHs, sizeof(ocrGuid_t)*NR,
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
    ocrAddDependence( PTR_globalH->DBK_gSettingsH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_gSettingsHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_init_gSettingsH_OET, TS_rankInitSpawner.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_gSettingsHs = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;
    ocrGuid_t* PTR_gSettingsHs = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 NR = PTR_gSettingsH->NR;

    //spawn N intializer EDTs
    //PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankInit; _paramc = 1; _depc = 3;

    u64 init_paramv[1] = {-1}; //each spawned EDT gets its rank or id

    TS_rankInit.FNC = FNC_rankInit;
    ocrEdtTemplateCreate( &TS_rankInit.TML, TS_rankInit.FNC, _paramc, _depc );

    s64 i;
    for( i = 0; i < NR; i++ )
    {
        gSettingsH_t* PTR_gSettingsH;
        ocrDbCreate( &(PTR_gSettingsHs[i]), (void **) &PTR_gSettingsH, sizeof(gSettingsH_t),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        rankH_t *PTR_rankH;
        ocrDbCreate( &(PTR_rankHs[i]), (void **) &PTR_rankH, sizeof(rankH_t),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate( &TS_rankInit.EDT, TS_rankInit.TML,
                      EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( DBK_gSettingsH, TS_rankInit.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_gSettingsHs[i], TS_rankInit.EDT, _idep++, DB_MODE_ITW );
        ocrAddDependence( PTR_rankHs[i], TS_rankInit.EDT, _idep++, DB_MODE_ITW );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH_0 = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DBK_gSettingsH = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_rankH = (ocrGuid_t) depv[2].guid;

    rankH_t *PTR_rankH = (rankH_t*) depv[2].ptr;

    //settingsInit
    ocrGuid_t TS_init_gSettingsH_OET;
    ocrEventCreate( &TS_init_gSettingsH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_gSettingsH; _paramc = 0; _depc = 2;

    TS_init_gSettingsH.FNC = FNC_init_gSettingsH;
    ocrEdtTemplateCreate( &TS_init_gSettingsH.TML, TS_init_gSettingsH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_gSettingsH.EDT, TS_init_gSettingsH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_init_gSettingsH.OET );

    ocrAddDependence( TS_init_gSettingsH.OET, TS_init_gSettingsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH_0, TS_init_gSettingsH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_gSettingsH, TS_init_gSettingsH.EDT, _idep++, DB_MODE_ITW );

    settingsH_t *PTR_settingsH;
    ocrDbCreate( &(PTR_rankH->DBK_settingsH), (void **) &PTR_settingsH, sizeof(settingsH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    dataH_t *PTR_dataH;
    ocrDbCreate( &(PTR_rankH->DBK_dataH), (void **) &PTR_dataH, sizeof(dataH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    s32 i;
    for( i = 0; i < 2; i++ )
    {
        eventH_t* PTR_eventH;
        ocrDbCreate(    &(PTR_rankH->DBK_eventHs[i]), (void **) &PTR_eventH, sizeof(eventH_t),
                        DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    }

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 1; _depc = 6;

    TS_init_rankH.FNC = FNC_init_rankH;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_init_rankH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_settingsH, TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_dataH, TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_eventHs[0], TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_eventHs[1], TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_init_gSettingsH_OET, TS_init_rankH.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s64 id = (s64) paramv[0];

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;

    gSettingsH_t *PTR_gSettingsH = (gSettingsH_t*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    dataH_t *PTR_dataH = (dataH_t*) depv[2].ptr;

    PTR_settingsH->id = id;
    partition_bounds( PTR_settingsH->id, 0, PTR_gSettingsH->NP-1, PTR_gSettingsH->NR,
                      &(PTR_settingsH->lb), &(PTR_settingsH->ub), &(PTR_settingsH->np));
    PTR_settingsH->itimestep = PTR_gSettingsH->IT0;

    s64 subdomain_size = PTR_settingsH->np;
    s64 HR = PTR_gSettingsH->HR;

    double *xIn, *xOut;
    ocrDbCreate( &(PTR_dataH->DBK_xIn), (void **) &xIn, sizeof(double)*(subdomain_size+2*HR),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_xOut), (void **) &xOut, sizeof(double)*(subdomain_size),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    MyOcrTaskStruct_t TS_init_dataH; _paramc = 0; _depc = 4;

    TS_init_dataH.FNC = FNC_init_dataH;
    ocrEdtTemplateCreate( &TS_init_dataH.TML, TS_init_dataH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_dataH.EDT, TS_init_dataH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_init_dataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_init_dataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_dataH->DBK_xIn, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_dataH->DBK_xOut, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );

    s64 i;
    for( i = 0; i < 2; i++ )
    {
        eventH_t *PTR_eventH = (eventH_t*) depv[3+i].ptr;

        ocrEventCreate( &PTR_eventH->EVT_Lsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Rsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Lrecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_eventH->EVT_Rrecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_eventH->EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_init_dataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);

    gSettingsH_t *PTR_gSettingsH = (gSettingsH_t*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    double *xIn = (double*) depv[2].ptr;
    double *xOut = (double*) depv[3].ptr;

    s64 np = PTR_settingsH->np;
    s64 id = PTR_settingsH->id;
    s64 lb = PTR_settingsH->lb;
    s64 ub = PTR_settingsH->ub;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;

    s64 i;

    for( i = 0; i < np; i++ )
    {
        xIn[i+HR] = (double) ( lb + i );
        xOut[i] = 0.;
    }

    for( i = 0; i < HR; i++ )
    {
        xIn[i] = -111.;
        xIn[np + HR + i] = -111.;
    }

    if( id == 0 )
    {
        for( i = -HR; i < 0; i++ )
            xIn[i+HR] = (double) ( lb + i );
    }

    if( id == NR-1)
    {
        for( i = np; i < np + HR; i++ )
            xIn[i+HR] = (double) ( lb + i );
    }

    for( i = 0; i < np + 2*HR; i++ )
        //PRINTF("ID: %d xIn[%ld]=%f\n", id, i, xIn[i]);

    return NULL_GUID;
}

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t *PTR_globalH = depv[0].ptr;

    MyOcrTaskStruct_t TS_globalMultiTimestepper; _paramc = 1; _depc = 2;

    u64 computeSpawner_paramv[1] = { NULL_GUID };

    TS_globalMultiTimestepper.FNC = FNC_globalMultiTimestepper;
    ocrEdtTemplateCreate( &TS_globalMultiTimestepper.TML, TS_globalMultiTimestepper.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalMultiTimestepper.EDT, TS_globalMultiTimestepper.TML,
                  EDT_PARAM_DEF, computeSpawner_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_gSettingsH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t FNC_globalMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t TS_rankMultiTimestepSpawner_OET_old = (ocrGuid_t) paramv[0];

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;
    globalH_t *PTR_globalH = depv[1].ptr;

    u64 itimestep = (u64) PTR_globalH->itimestep;

    u64 NT = PTR_gSettingsH->NT;
    u64 NT_SYNC = PTR_gSettingsH->NT_SYNC;

    if( TS_rankMultiTimestepSpawner_OET_old != NULL_GUID )
        ocrEventDestroy(TS_rankMultiTimestepSpawner_OET_old);

    //PRINTF("%s Timestep %d\n", __func__, itimestep);

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
        ocrAddDependence( DBK_gSettingsH, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_ITW );
        ocrAddDependence( PTR_globalH->DBK_gSettingsHs, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankMultiTimestepSpawner.EDT, _idep++, DB_MODE_RO );

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
            ocrAddDependence( DBK_gSettingsH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RO );
            ocrAddDependence( DBK_globalH, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_RO );
            ocrAddDependence( TS_rankMultiTimestepSpawner_OET, TS_globalMultiTimestepper.EDT, _idep++, DB_MODE_NULL );
        }
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankMultiTimestepSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;
    ocrGuid_t DBK_gSettingsHs = depv[2].guid;
    ocrGuid_t DBK_rankHs = depv[3].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;
    ocrGuid_t* PTR_gSettingsHs = depv[2].ptr;
    ocrGuid_t* PTR_rankHs = depv[3].ptr;

    u64 NR = PTR_gSettingsH->NR;
    u64 NT = PTR_gSettingsH->NT;
    u64 NT_SYNC = PTR_gSettingsH->NT_SYNC;

    u64 itimestep = (u64) PTR_globalH->itimestep;

    u64 i;

    //PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankMultiTimestepper; _paramc = 3; _depc = 4;

    u64 compute_paramv[3];

    TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
    ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

    //PRINTF("Timestep# %d NR = %d\n", itimestep, NR);
    for( i = 0; i < NR; i++ )
    {
        compute_paramv[0] = (u64) i;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) MIN( itimestep + NT_SYNC - 1, NT );

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( PTR_gSettingsHs[i], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_rankHs[i], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (i!=0) ? PTR_rankHs[i-1] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (i!=NR-1) ? PTR_rankHs[i+1] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
    }

    PTR_globalH->itimestep += NT_SYNC;

    return NULL_GUID;
}

ocrGuid_t FNC_rankMultiTimestepper(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_rankH =  depv[1].guid;
    ocrGuid_t DBK_rankH_l = depv[2].guid;
    ocrGuid_t DBK_rankH_r = depv[3].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;
    rankH_t* PTR_rankH = (rankH_t*) depv[1].ptr;
    rankH_t* PTR_rankH_l = (rankH_t*) depv[2].ptr;
    rankH_t* PTR_rankH_r = (rankH_t*) depv[3].ptr;

    s64 id = (s64) paramv[0];
    s64 itimestep = (s64) paramv[1];
    s64 ntimestep_l = (s64) paramv[2];

    s64 phase = itimestep%2;

    s64 NR = (s64) PTR_gSettingsH->NR;

    MyOcrTaskStruct_t TS_timestep; _paramc = 0; _depc = 6;

    TS_timestep.FNC = FNC_timestep;
    ocrEdtTemplateCreate( &TS_timestep.TML, TS_timestep.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_timestep.EDT, TS_timestep.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_timestep.OET);

    ocrGuid_t TS_timestep_OET;
    ocrEventCreate( &TS_timestep_OET, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_timestep.OET, TS_timestep_OET,   0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_settingsH, TS_timestep.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_dataH, TS_timestep.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_eventHs[phase], TS_timestep.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (id!=0) ? PTR_rankH_l->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( (id!=NR-1) ? PTR_rankH_r->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );

    itimestep += 1;

    if( itimestep <= ntimestep_l )
    {
        MyOcrTaskStruct_t TS_rankMultiTimestepper; _paramc = 3; _depc = 5;

        u64 compute_paramv[3];
        compute_paramv[0] = (u64) id;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) ntimestep_l;

        TS_rankMultiTimestepper.FNC = FNC_rankMultiTimestepper;
        ocrEdtTemplateCreate( &TS_rankMultiTimestepper.TML, TS_rankMultiTimestepper.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( DBK_gSettingsH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankH_l, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankH_r, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( TS_timestep_OET, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_timestep(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_dataH = depv[2].guid;
    ocrGuid_t DBK_eventH = depv[3].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    dataH_t* PTR_data = (dataH_t*) depv[2].ptr;
    eventH_t* PTR_events = (eventH_t*) depv[3].ptr;
    eventH_t* PTR_events_l = (eventH_t*) depv[4].ptr;
    eventH_t* PTR_events_r = (eventH_t*) depv[5].ptr;

    s64 NR = (s64) PTR_gSettingsH->NR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    MyOcrTaskStruct_t TS_Lsend; _paramc = 0; _depc = 3;

    TS_Lsend.FNC = FNC_Lsend;
    ocrEdtTemplateCreate( &TS_Lsend.TML, TS_Lsend.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Lsend.EDT, TS_Lsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Lsend.OET);

    if( id!=0 ) ocrAddDependence( TS_Lsend.OET, PTR_events_l->EVT_Rrecv_start, 0, DB_MODE_ITW );
    ocrAddDependence( TS_Lsend.OET, PTR_events->EVT_Lsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Lsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Lsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lsend.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Rsend; _paramc = 0; _depc = 3;

    TS_Rsend.FNC = FNC_Rsend;
    ocrEdtTemplateCreate( &TS_Rsend.TML, TS_Rsend.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Rsend.EDT, TS_Rsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Rsend.OET);

    if( id != NR - 1 ) ocrAddDependence( TS_Rsend.OET, PTR_events_r->EVT_Lrecv_start, 0, DB_MODE_ITW );
    ocrAddDependence( TS_Rsend.OET, PTR_events->EVT_Rsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Rsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Rsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rsend.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Lrecv; _paramc = 0; _depc = 4;

    TS_Lrecv.FNC = FNC_Lrecv;
    ocrEdtTemplateCreate( &TS_Lrecv.TML, TS_Lrecv.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Lrecv.EDT, TS_Lrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Lrecv.OET);

    ocrAddDependence( TS_Lrecv.OET, PTR_events->EVT_Lrecv_fin, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Lrecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Lrecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Lrecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (id!=0)?PTR_events->EVT_Lrecv_start:NULL_GUID, TS_Lrecv.EDT, _idep++, DB_MODE_ITW );

    MyOcrTaskStruct_t TS_Rrecv; _paramc = 0; _depc = 4;

    TS_Rrecv.FNC = FNC_Rrecv;
    ocrEdtTemplateCreate( &TS_Rrecv.TML, TS_Rrecv.FNC, _paramc,_depc );

    ocrEdtCreate( &TS_Rrecv.EDT, TS_Rrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Rrecv.OET);

    ocrAddDependence( TS_Rrecv.OET, PTR_events->EVT_Rrecv_fin, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Rrecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Rrecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Rrecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (id!=NR-1)?PTR_events->EVT_Rrecv_start:NULL_GUID, TS_Rrecv.EDT, _idep++, DB_MODE_ITW );

    ocrGuid_t TS_verify_OET;
    ocrEventCreate( &TS_verify_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_verify; _paramc = 0; _depc = 5;

    TS_verify.FNC = FNC_verify;
    ocrEdtTemplateCreate( &TS_verify.TML, TS_verify.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_verify.EDT, TS_verify.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID,
                  &TS_verify.OET );

    ocrAddDependence( TS_verify.OET, TS_verify_OET, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_verify.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_verify.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_verify.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_events->EVT_Lrecv_fin, TS_verify.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Rrecv_fin, TS_verify.EDT, _idep++, DB_MODE_NULL );

    ocrGuid_t TS_update_OET;
    ocrEventCreate( &TS_update_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update; _paramc = 0; _depc = 8;

    TS_update.FNC = FNC_update;
    ocrEdtTemplateCreate( &TS_update.TML, TS_update.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_update.EDT, TS_update.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_update.OET );

    ocrAddDependence( TS_update.OET, TS_update_OET, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( DBK_settingsH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_data->DBK_xIn, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_data->DBK_xOut, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( DBK_eventH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_events->EVT_Lsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Rsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( TS_verify_OET, TS_update.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t FNC_Lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* lsend;
    ocrGuid_t DBK_Lsend;
    ocrDbCreate( &DBK_Lsend, (void **) &lsend, sizeof(double)*HR,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i;
    for( i = 0; i < HR; i++ )
    {
        lsend[i] = xIn[HR+i];
        //PRINTF("lsend: id=%d i=%ld %f\n", id, HR+i, lsend[i]);
    }

    if( id != 0 )
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

ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np = (s64) PTR_settingsH->np;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* rsend;
    ocrGuid_t DBK_Rsend;
    ocrDbCreate( &DBK_Rsend, (void **) &rsend,  sizeof(double)*HR,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i;
    for( i = 0; i < HR; i++ )
    {
        rsend[i] = xIn[np+i];
        //PRINTF("rsend: id=%d i=%ld %f\n", id, np+i, rsend[i]);
    }

    if( id != NR - 1 )
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

ocrGuid_t FNC_Lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Rsend = (ocrGuid_t) depv[3].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* rsent = (double*) depv[3].ptr;

    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    int i;
    if( DBK_Rsend != NULL_GUID || id != 0 )
    {
        for( i = 0; i < HR; i++ )
        {
            xIn[i] = rsent[i];
            //PRINTF("lrecv: id=%d i=%ld %f\n", id, i, xIn[i]);
        }

        //PRINTF("ID=%d FNC_Lrecv lrecv guid is %lu\n", id, DBK_Rsend);
        ocrDbDestroy( DBK_Rsend );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_Rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Lsend = (ocrGuid_t) depv[3].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* lsent = (double*) depv[3].ptr;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np = (s64) PTR_settingsH->np;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    int i;
    if( DBK_Lsend != NULL_GUID || id != NR - 1)
    {
        for( i = 0; i < HR; i++ )
        {
            xIn[HR+np+i] = lsent[i];
            //PRINTF("rrecv: id=%d i=%ld %f\n", id, HR+np+i, xIn[HR+np+i]);
        }

        //PRINTF("ID=%d FNC_Rrecv rrecv guid is %lu\n", id, DBK_Lsend);
        ocrDbDestroy( DBK_Lsend );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np = (s64) PTR_settingsH->np;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    s64 i;
    for( i = -HR; i < np+HR; i++ )
    {
        if( (id == 0 && i < 0) || (id == NR - 1 && i >= np) )
            continue;

        //PRINTF("ID: %d xIn[%ld]=%f\n", id, i+HR, xIn[i+HR]);

        if( ABS( xIn[i+HR] - ( id*np + i + itimestep - 1) ) > (double) NORM )
            break;
    }

    if ( id == 0 )
    {
        if( i != np+HR )
            PRINTF("ID: %ld Verification failed! timestep %d\n", id, itimestep);
        else
            PRINTF("ID: %ld Verification passed! timestep %d\n", id, itimestep);
    }

    return NULL_GUID;
}

ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* xOut = (double*) depv[3].ptr;
    eventH_t* PTR_events = (eventH_t*) depv[4].ptr;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np = (s64) PTR_settingsH->np;
    s64 lb = (s64) PTR_settingsH->lb;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    s64 i;
    for( i = 0; i < np; i++ )
        xOut[i] = (xIn[i+HR-1]+xIn[i+HR+1]-xIn[i+HR]) + 1;

    for( i = 0; i < np; i++ )
        xIn[i+HR] = xOut[i];

    if( id == 0 )
    {
        for( i = -HR; i < 0; i++ )
            xIn[i+HR] = (double) (xIn[i+HR] + 1);
    }

    if( id == NR-1)
    {
        for( i = np; i < np + HR; i++ )
            xIn[i+HR] = (double) ( xIn[i+HR] + 1 );
    }

    if( (id == NR - 1) && (itimestep == 1))
    {
        //PRINTF("ID:%d Sleeping for 10 seconds timestep %d\n", id, itimestep );
        //sleep(10);
    }

    ocrEventDestroy( PTR_events->EVT_Lsend_fin );
    ocrEventDestroy( PTR_events->EVT_Rsend_fin );
    ocrEventDestroy( PTR_events->EVT_Lrecv_start );
    ocrEventDestroy( PTR_events->EVT_Rrecv_start );
    ocrEventDestroy( PTR_events->EVT_Lrecv_fin );
    ocrEventDestroy( PTR_events->EVT_Rrecv_fin );

    ocrEventCreate( &PTR_events->EVT_Lsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Rsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Lrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Rrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &PTR_events->EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );

    PTR_settingsH->itimestep = itimestep + 1;

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);

    ocrGuid_t DBK_globalH = depv[0].guid;

    ocrDbDestroy( DBK_globalH );

    ocrShutdown();

    return NULL_GUID;
}

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

    PRINTF("npoints %d\n", npoints);
    PRINTF("nranks %d\n", nranks);
    PRINTF("ntimesteps %d\n", ntimesteps);
    PRINTF("ntimesteps_sync %d\n", ntimesteps_sync);
    PRINTF("itimestep0 %d\n", itimestep0);
    PRINTF("halo_radius %d\n", halo_radius);

    ocrGuid_t DBK_gSettingsH_0;
    gSettingsH_t* PTR_gSettingsH_0;

    ocrDbCreate( &DBK_gSettingsH_0, (void **) &PTR_gSettingsH_0, sizeof(gSettingsH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TS_settingsInit_OET;
    ocrEventCreate( &TS_settingsInit_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_settingsInit;

    u64 settingsInit_paramv[6] = { (u64) npoints, (u64) nranks, (u64) ntimesteps, (u64) ntimesteps_sync, (u64) itimestep0, (u64) halo_radius };

    TS_settingsInit.FNC = FNC_settingsInit;
    ocrEdtTemplateCreate( &TS_settingsInit.TML, TS_settingsInit.FNC, 6, 1 );

    ocrEdtCreate( &TS_settingsInit.EDT, TS_settingsInit.TML,
                  EDT_PARAM_DEF, settingsInit_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_settingsInit.OET );

    ocrAddDependence( TS_settingsInit.OET, TS_settingsInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH_0, TS_settingsInit.EDT, _idep++, DB_MODE_ITW );

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
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH_0, TS_globalInit.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_globalInit.EDT, _idep++, DB_MODE_ITW );

    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    TS_globalFinalize.FNC = FNC_globalFinalize;
    ocrEdtTemplateCreate( &TS_globalFinalize.TML, TS_globalFinalize.FNC, 0, 2 );

    ocrEdtCreate( &TS_globalFinalize.EDT, TS_globalFinalize.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalFinalize.OET );

    ocrAddDependence( TS_globalFinalize.OET, TS_globalFinalize_OET, 0, DB_MODE_NULL);

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalFinalize.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_globalCompute_OET, TS_globalFinalize.EDT, _idep++, DB_MODE_NULL);

    return NULL_GUID;
}
