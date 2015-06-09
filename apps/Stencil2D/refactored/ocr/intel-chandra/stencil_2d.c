#include "ocr.h"
#include "ocr-std.h"
#include <stdlib.h>
#include <math.h>

//default values
#define NPOINTS 4
#define NRANKS 4
#define NTIMESTEPS 10
#define NTIMESTEPS_SYNC 5
#define ITIMESTEP0 1
#define HALO_RADIUS 2

#define EPSILON 1e-8

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

#define WEIGHT(ii,jj) weight[ii+HR+(jj+HR)*(2*HR+1)]
#define INDEXIN(i,j) ( (i+HR)+(j+HR)*(np_x+2*HR) )
#define INDEXOUT(i,j) ( (i)+(j)*(np_x) )
#define IN(i,j) ( xIn[ INDEXIN(i-ib,j-jb) ] )
#define OUT(i,j) ( xOut[ INDEXOUT(i-ib,j-jb) ] )

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
    s64 NP_X, NP_Y;
    s64 NR_X, NR_Y; //ranks in x and y directions
    s64 NT, NT_SYNC, IT0;
    s64 HR;
} gSettingsH_t;

typedef struct
{
    s64 id, lb, ub, np; // rank id, local bounds, local points
    s64 id_x, id_y, ib, ie, jb, je, np_x, np_y;
    s64 itimestep;
} settingsH_t;

typedef struct
{
    ocrGuid_t DBK_xIn, DBK_xOut, DBK_weight;
    ocrGuid_t DBK_refNorm;
} dataH_t;

typedef struct
{
    ocrGuid_t EVT_Lsend_fin, EVT_Rsend_fin;
    ocrGuid_t EVT_Lrecv_start, EVT_Rrecv_start;
    ocrGuid_t EVT_Lrecv_fin, EVT_Rrecv_fin;
    ocrGuid_t EVT_Bsend_fin, EVT_Tsend_fin;
    ocrGuid_t EVT_Brecv_start, EVT_Trecv_start;
    ocrGuid_t EVT_Brecv_fin, EVT_Trecv_fin;
    ocrGuid_t EVT_reduction;
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
    ocrGuid_t EDT_reduction;
    ocrGuid_t DBK_gSettingsH; // Commandline parameters/settings
    ocrGuid_t DBK_gSettingsHs; //-> Broadcasted parameters/settings gSettingsH_t[]
    ocrGuid_t DBK_rankHs; //--> rankH_t[]
} globalH_t;

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e);

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
                    ocrGuid_t FNC_Bsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Tsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Brecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_Trecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_reduction(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e)
{
    s64 N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;
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

    PTR_gSettingsH->NP = PTR_gSettingsH_0->NP; //squre global tile is assumed; side
    PTR_gSettingsH->NR = PTR_gSettingsH_0->NR;
    PTR_gSettingsH->NT = PTR_gSettingsH_0->NT;
    PTR_gSettingsH->NT_SYNC = PTR_gSettingsH_0->NT_SYNC;
    PTR_gSettingsH->IT0 = PTR_gSettingsH_0->IT0;
    PTR_gSettingsH->HR = PTR_gSettingsH_0->HR;

    PTR_gSettingsH->NP_X = (s64) PTR_gSettingsH_0->NP; //Simplified for now
    PTR_gSettingsH->NP_Y = (s64) PTR_gSettingsH_0->NP; //Simplified for now

    int Num_procs = PTR_gSettingsH_0->NR;
    int Num_procsx, Num_procsy;
    for (Num_procsx=(int) (sqrt(Num_procs+1)); Num_procsx>0; Num_procsx--)
    {
        if (!(Num_procs%Num_procsx))
        {
            Num_procsy = Num_procs/Num_procsx;
            break;
        }
    }

    PTR_gSettingsH->NR_X = (s64) Num_procsx;
    PTR_gSettingsH->NR_Y = (s64) Num_procsy;
    //PRINTF("NR_X = %d NR_Y = %d\n", PTR_gSettingsH->NR_X, PTR_gSettingsH->NR_Y);

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
    PTR_settingsH->id_x = id%PTR_gSettingsH->NR_X;
    PTR_settingsH->id_y = id/PTR_gSettingsH->NR_X;

    partition_bounds( PTR_settingsH->id_x, 0, PTR_gSettingsH->NP_X-1, PTR_gSettingsH->NR_X,
                      &(PTR_settingsH->ib), &(PTR_settingsH->ie) );
    partition_bounds( PTR_settingsH->id_y, 0, PTR_gSettingsH->NP_Y-1, PTR_gSettingsH->NR_Y,
                      &(PTR_settingsH->jb), &(PTR_settingsH->je) );

    PTR_settingsH->np_x = PTR_settingsH->ie - PTR_settingsH->ib + 1;
    PTR_settingsH->np_y = PTR_settingsH->je - PTR_settingsH->jb + 1;

    PTR_settingsH->itimestep = PTR_gSettingsH->IT0;

    s64 subdomain_size_x = PTR_settingsH->np_x;
    s64 subdomain_size_y = PTR_settingsH->np_y;

    s64 HR = PTR_gSettingsH->HR;

    double *xIn, *xOut, *weight, *refNorm;
    ocrDbCreate( &(PTR_dataH->DBK_xIn), (void **) &xIn, sizeof(double)*(subdomain_size_x+2*HR)*(subdomain_size_y+2*HR),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_xOut), (void **) &xOut, sizeof(double)*(subdomain_size_x)*(subdomain_size_y),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_weight), (void **) &weight, sizeof(double)*(2*HR+1)*(2*HR+1),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_refNorm), (void **) &refNorm, sizeof(double)*(1),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    MyOcrTaskStruct_t TS_init_dataH; _paramc = 0; _depc = 6;

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
    ocrAddDependence( PTR_dataH->DBK_weight, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_dataH->DBK_refNorm, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );

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

        ocrEventCreate( &PTR_eventH->EVT_Bsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Tsend_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Brecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_eventH->EVT_Trecv_start, OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &PTR_eventH->EVT_Brecv_fin, OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &PTR_eventH->EVT_Trecv_fin, OCR_EVENT_STICKY_T, false );

        ocrEventCreate( &PTR_eventH->EVT_reduction, OCR_EVENT_STICKY_T, true );
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
    double *weight = (double*) depv[4].ptr;
    double *refNorm = (double*) depv[5].ptr;

    s64 np_x = PTR_settingsH->np_x;
    s64 id_x = PTR_settingsH->id_x;
    s64 ib = PTR_settingsH->ib;
    s64 ie = PTR_settingsH->ie;

    s64 np_y = PTR_settingsH->np_y;
    s64 id_y = PTR_settingsH->id_y;
    s64 jb = PTR_settingsH->jb;
    s64 je = PTR_settingsH->je;

    s64 NR_X = PTR_gSettingsH->NR_X;
    s64 NR_Y = PTR_gSettingsH->NR_Y;
    s64 HR = PTR_gSettingsH->HR;

    s64 i, j;

    for( j = jb; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            IN(i,j) = (double) ( i ) + ( j );
            OUT(i,j) = 0.;
        }

    int ii, jj;
    /* fill the stencil weights to reflect a discrete divergence operator         */
    for (jj=-HR; jj<=HR; jj++) for (ii=-HR; ii<=HR; ii++)
        WEIGHT(ii,jj) = (double) 0.0;

    //stencil_size = 4*HR+1;
    for (ii=1; ii<=HR; ii++) {
        WEIGHT(0, ii) = WEIGHT( ii,0) =  (double) (1.0/(2.0*ii*HR));
        WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(double) (1.0/(2.0*ii*HR));
    }

    refNorm[0] = 0.0;

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
    u64 itimestep0 = (u64) (PTR_gSettingsH->IT0);

    u64 NT = PTR_gSettingsH->NT;
    u64 NT_SYNC = PTR_gSettingsH->NT_SYNC;
    u64 NR = PTR_gSettingsH->NR;

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
        ocrAddDependence( PTR_globalH->DBK_gSettingsH, PTR_globalH->EDT_reduction, _idep++, DB_MODE_RO );
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

int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
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
    u64 NR_X = PTR_gSettingsH->NR_X;
    u64 NR_Y = PTR_gSettingsH->NR_Y;
    u64 NT = PTR_gSettingsH->NT;
    u64 NT_SYNC = PTR_gSettingsH->NT_SYNC;

    u64 itimestep = (u64) PTR_globalH->itimestep;

    u64 I, id_x, id_y;

    //PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankMultiTimestepper; _paramc = 3; _depc = 7;

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
        id_y = I/NR_X;

        ocrEdtCreate( &TS_rankMultiTimestepper.EDT, TS_rankMultiTimestepper.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( PTR_gSettingsHs[I], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_rankHs[I], TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (id_x!=0) ? PTR_rankHs[globalRankFromCoords(id_x-1,id_y, NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (id_x!=NR_X-1) ? PTR_rankHs[globalRankFromCoords(id_x+1,id_y, NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (id_y!=0) ? PTR_rankHs[globalRankFromCoords(id_x,id_y-1,NR_X, NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (id_y!=NR_Y-1) ? PTR_rankHs[globalRankFromCoords(id_x,id_y+1,NR_X,NR_Y)] : NULL_GUID, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
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
    ocrGuid_t DBK_rankH_b = depv[4].guid;
    ocrGuid_t DBK_rankH_t = depv[5].guid;
    ocrGuid_t DBK_globalH = depv[6].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;
    rankH_t* PTR_rankH = (rankH_t*) depv[1].ptr;
    rankH_t* PTR_rankH_l = (rankH_t*) depv[2].ptr;
    rankH_t* PTR_rankH_r = (rankH_t*) depv[3].ptr;
    rankH_t* PTR_rankH_b = (rankH_t*) depv[4].ptr;
    rankH_t* PTR_rankH_t = (rankH_t*) depv[5].ptr;

    s64 id = (s64) paramv[0];
    s64 itimestep = (s64) paramv[1];
    s64 ntimestep_l = (s64) paramv[2];

    s64 phase = itimestep%2;

    s64 NR = (s64) PTR_gSettingsH->NR;
    u64 NR_X = PTR_gSettingsH->NR_X;
    u64 NR_Y = PTR_gSettingsH->NR_Y;
    s64 id_x = id%NR_X;
    s64 id_y = id/NR_X;

    MyOcrTaskStruct_t TS_timestep; _paramc = 0; _depc = 9;

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
    ocrAddDependence( (id_x!=0) ? PTR_rankH_l->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( (id_x!=NR_X-1) ? PTR_rankH_r->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( (id_y!=0) ? PTR_rankH_b->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( (id_y!=NR_Y-1) ? PTR_rankH_t->DBK_eventHs[phase] : NULL_GUID, TS_timestep.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_timestep.EDT, _idep++, DB_MODE_RO );

    itimestep += 1;

    if( itimestep <= ntimestep_l )
    {
        MyOcrTaskStruct_t TS_rankMultiTimestepper; _paramc = 3; _depc = 8;

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
        ocrAddDependence( DBK_rankH_b, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankH_t, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_globalH, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( TS_timestep_OET, TS_rankMultiTimestepper.EDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_timestep(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_gSettingsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_dataH = depv[2].guid;
    ocrGuid_t DBK_eventH = depv[3].guid;
    ocrGuid_t DBK_globalH = depv[8].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    dataH_t* PTR_data = (dataH_t*) depv[2].ptr;
    eventH_t* PTR_events = (eventH_t*) depv[3].ptr;
    eventH_t* PTR_events_l = (eventH_t*) depv[4].ptr;
    eventH_t* PTR_events_r = (eventH_t*) depv[5].ptr;
    eventH_t* PTR_events_b = (eventH_t*) depv[6].ptr;
    eventH_t* PTR_events_t = (eventH_t*) depv[7].ptr;

    s64 NR = (s64) PTR_gSettingsH->NR;
    s64 NR_X = (s64) PTR_gSettingsH->NR_X;
    s64 NR_Y = (s64) PTR_gSettingsH->NR_Y;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_x = (s64) PTR_settingsH->id_x;
    s64 id_y = (s64) PTR_settingsH->id_y;
    s64 itimestep = (s64) PTR_settingsH->itimestep;

    //PRINTF("%s id %d x %d y %d\n", __func__, id, id_x, id_y);
    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    MyOcrTaskStruct_t TS_Lsend; _paramc = 0; _depc = 3;

    TS_Lsend.FNC = FNC_Lsend;
    ocrEdtTemplateCreate( &TS_Lsend.TML, TS_Lsend.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Lsend.EDT, TS_Lsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Lsend.OET);

    if( id_x!=0 ) ocrAddDependence( TS_Lsend.OET, PTR_events_l->EVT_Rrecv_start, 0, DB_MODE_ITW );
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

    if( id_x != NR_X - 1 ) ocrAddDependence( TS_Rsend.OET, PTR_events_r->EVT_Lrecv_start, 0, DB_MODE_ITW );
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
    ocrAddDependence( (id_x!=0)?PTR_events->EVT_Lrecv_start:NULL_GUID, TS_Lrecv.EDT, _idep++, DB_MODE_ITW );

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
    ocrAddDependence( (id_x!=NR_X-1)?PTR_events->EVT_Rrecv_start:NULL_GUID, TS_Rrecv.EDT, _idep++, DB_MODE_ITW );


    //Y direction communication
    MyOcrTaskStruct_t TS_Bsend; _paramc = 0; _depc = 3;

    TS_Bsend.FNC = FNC_Bsend;
    ocrEdtTemplateCreate( &TS_Bsend.TML, TS_Bsend.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Bsend.EDT, TS_Bsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Bsend.OET);

    if( id_y!=0 ) ocrAddDependence( TS_Bsend.OET, PTR_events_b->EVT_Trecv_start, 0, DB_MODE_ITW );
    ocrAddDependence( TS_Bsend.OET, PTR_events->EVT_Bsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Bsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Bsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Bsend.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Tsend; _paramc = 0; _depc = 3;

    TS_Tsend.FNC = FNC_Tsend;
    ocrEdtTemplateCreate( &TS_Tsend.TML, TS_Tsend.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Tsend.EDT, TS_Tsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Tsend.OET);

    if( id_y != NR_Y - 1 ) ocrAddDependence( TS_Tsend.OET, PTR_events_t->EVT_Brecv_start, 0, DB_MODE_ITW );
    ocrAddDependence( TS_Tsend.OET, PTR_events->EVT_Tsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Tsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Tsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Tsend.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Brecv; _paramc = 0; _depc = 4;

    TS_Brecv.FNC = FNC_Brecv;
    ocrEdtTemplateCreate( &TS_Brecv.TML, TS_Brecv.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_Brecv.EDT, TS_Brecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Brecv.OET);

    ocrAddDependence( TS_Brecv.OET, PTR_events->EVT_Brecv_fin, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Brecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Brecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Brecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (id_y!=0)?PTR_events->EVT_Brecv_start:NULL_GUID, TS_Brecv.EDT, _idep++, DB_MODE_ITW );

    MyOcrTaskStruct_t TS_Trecv; _paramc = 0; _depc = 4;

    TS_Trecv.FNC = FNC_Trecv;
    ocrEdtTemplateCreate( &TS_Trecv.TML, TS_Trecv.FNC, _paramc,_depc );

    ocrEdtCreate( &TS_Trecv.EDT, TS_Trecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_Trecv.OET);

    ocrAddDependence( TS_Trecv.OET, PTR_events->EVT_Trecv_fin, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_gSettingsH, TS_Trecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_Trecv.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_Trecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (id_y!=NR_Y-1)?PTR_events->EVT_Trecv_start:NULL_GUID, TS_Trecv.EDT, _idep++, DB_MODE_ITW );

    //end

    ocrGuid_t TS_verify_OET;
    ocrEventCreate( &TS_verify_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_verify; _paramc = 0; _depc = 7;

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
    ocrAddDependence( PTR_events->EVT_Brecv_fin, TS_verify.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Trecv_fin, TS_verify.EDT, _idep++, DB_MODE_NULL );

    ocrGuid_t TS_update_OET;
    ocrEventCreate( &TS_update_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update; _paramc = 0; _depc = 13;

    TS_update.FNC = FNC_update;
    ocrEdtTemplateCreate( &TS_update.TML, TS_update.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_update.EDT, TS_update.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_update.OET );

    ocrAddDependence( TS_update.OET, TS_update_OET, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( DBK_gSettingsH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( DBK_settingsH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_data->DBK_weight, TS_update.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_data->DBK_xIn, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_data->DBK_xOut, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_data->DBK_refNorm, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( DBK_eventH, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_events->EVT_Lsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Rsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Bsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( PTR_events->EVT_Tsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
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
    s64 id_x = (s64) PTR_settingsH->id_x;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* lsend;
    ocrGuid_t DBK_Lsend;
    ocrDbCreate( &DBK_Lsend, (void **) &lsend, sizeof(double)*HR*np_y,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i, j;
    int kk = 0;
    for( j = jb; j <= je; j++ )
        for( i = ib; i < ib+HR; i++ )
        {
            lsend[kk++] = IN(i,j);
        }

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

ocrGuid_t FNC_Rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 NR_X = PTR_gSettingsH->NR_X;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_x = (s64) PTR_settingsH->id_x;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* rsend;
    ocrGuid_t DBK_Rsend;
    ocrDbCreate( &DBK_Rsend, (void **) &rsend,  sizeof(double)*HR*np_y,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i, j;
    int kk = 0;
    for( j = jb; j <= je; j++ )
        for( i = ie-HR+1; i <= ie; i++ )
        {
            rsend[kk++] = IN(i,j);
        }

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
    s64 id_x = (s64) PTR_settingsH->id_x;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    int i, j;
    if( DBK_Rsend != NULL_GUID || id_x != 0 )
    {
        int kk = 0;
        for( j = jb; j <= je; j++ )
            for( i = ib-HR; i < ib; i++ )
            {
                IN(i,j) = rsent[kk++];
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

    s64 NR_X = PTR_gSettingsH->NR_X;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_x = (s64) PTR_settingsH->id_x;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    int i, j;
    if( DBK_Lsend != NULL_GUID || id_x != NR_X - 1)
    {
        int kk = 0;
        for( j = jb; j <= je; j++ )
            for( i = ie + 1; i < ie + 1 + HR; i++ )
            {
                IN(i,j) = lsent[kk++];
            }

        //PRINTF("ID=%d FNC_Rrecv rrecv guid is %lu\n", id, DBK_Lsend);
        ocrDbDestroy( DBK_Lsend );
    }

    return NULL_GUID;
}

//y-direction
ocrGuid_t FNC_Bsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_y = (s64) PTR_settingsH->id_y;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* bsend;
    ocrGuid_t DBK_Bsend;
    ocrDbCreate( &DBK_Bsend, (void **) &bsend, sizeof(double)*HR*np_x,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i, j;
    int kk = 0;
    for( j = jb; j < jb+HR; j++ )
        for( i = ib; i <= ie; i++ )
        {
            bsend[kk++] = IN(i,j);
        }

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

ocrGuid_t FNC_Tsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;

    s64 NR_Y = PTR_gSettingsH->NR_Y;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_y = (s64) PTR_settingsH->id_y;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    double* tsend;
    ocrGuid_t DBK_Tsend;
    ocrDbCreate( &DBK_Tsend, (void **) &tsend,  sizeof(double)*HR*np_x,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i, j;
    int kk = 0;
    for( j = je - HR + 1; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            tsend[kk++] = IN(i,j);
        }

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

ocrGuid_t FNC_Brecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[3].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* tsent = (double*) depv[3].ptr;

    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_y = (s64) PTR_settingsH->id_y;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

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
        ocrDbDestroy( DBK_Tsend );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_Trecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[2].guid;
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[3].guid;

    gSettingsH_t* PTR_gSettingsH = depv[0].ptr;
    settingsH_t* PTR_settingsH = depv[1].ptr;
    double* xIn = (double*) depv[2].ptr;
    double* bsent = (double*) depv[3].ptr;

    s64 NR_Y = PTR_gSettingsH->NR_Y;
    s64 HR = PTR_gSettingsH->HR;

    s64 id = (s64) PTR_settingsH->id;
    s64 id_y = (s64) PTR_settingsH->id_y;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

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
        ocrDbDestroy( DBK_Bsend );
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
    s64 NP_X = PTR_gSettingsH->NP_X;
    s64 NP_Y = PTR_gSettingsH->NP_Y;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;

    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

    int i, j;
    double local_norm = (double) 0.0;
    for( j=MAX(jb,HR); j<=MIN(NP_Y-HR-1,je); j++ )
        for (i=MAX(ib,HR); i<=MIN(NP_X-HR-1,ie); i++)
            //local_norm += (double)ABS(OUT(i,j));
            ;

    return NULL_GUID;
}

ocrGuid_t FNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_refNorm = depv[6].guid;

    globalH_t* PTR_globalH = depv[0].ptr;
    gSettingsH_t* PTR_gSettingsH = depv[1].ptr;
    settingsH_t* PTR_settingsH = depv[2].ptr;
    double* weight = (double*) depv[3].ptr;
    double* xIn = (double*) depv[4].ptr;
    double* xOut = (double*) depv[5].ptr;
    double* refNorm = (double*) depv[6].ptr;
    eventH_t* PTR_events = (eventH_t*) depv[7].ptr;

    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;
    s64 NP_X = PTR_gSettingsH->NP_X;
    s64 NP_Y = PTR_gSettingsH->NP_Y;

    s64 id = (s64) PTR_settingsH->id;
    s64 itimestep = (s64) PTR_settingsH->itimestep;
    s64 np_x = (s64) PTR_settingsH->np_x;
    s64 np_y = (s64) PTR_settingsH->np_y;
    s64 ib = (s64) PTR_settingsH->ib;
    s64 ie = (s64) PTR_settingsH->ie;
    s64 jb = (s64) PTR_settingsH->jb;
    s64 je = (s64) PTR_settingsH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);
    //PRINTF("ID: %d ib %d ie %d jb %d je %d\n", id, ib, ie, jb, je);

    /* Apply the stencil operator */
    int i, j, ii, jj;
    for (j=MAX(jb,HR); j<=MIN(NP_Y-HR-1,je); j++)
    {
        for (i=MAX(ib,HR); i<=MIN(NP_X-HR-1,ie); i++)
        {
            for (jj=-HR; jj<=HR; jj++) {
                OUT(i,j) += WEIGHT(0,jj)*IN(i,j+jj);
            }
            for (ii=-HR; ii<0; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
            }
            for (ii=1; ii<=HR; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);

            }
        }
    }

    /* add constant to solution to force refresh of neighbor data, if any */
    for (j=jb; j<=je; j++) for (i=ib; i<=ie; i++) IN(i,j)+= 1.0;

    ocrEventDestroy( PTR_events->EVT_Lsend_fin );
    ocrEventDestroy( PTR_events->EVT_Rsend_fin );
    ocrEventDestroy( PTR_events->EVT_Lrecv_start );
    ocrEventDestroy( PTR_events->EVT_Rrecv_start );
    ocrEventDestroy( PTR_events->EVT_Lrecv_fin );
    ocrEventDestroy( PTR_events->EVT_Rrecv_fin );

    ocrEventDestroy( PTR_events->EVT_Bsend_fin );
    ocrEventDestroy( PTR_events->EVT_Tsend_fin );
    ocrEventDestroy( PTR_events->EVT_Brecv_start );
    ocrEventDestroy( PTR_events->EVT_Trecv_start );
    ocrEventDestroy( PTR_events->EVT_Brecv_fin );
    ocrEventDestroy( PTR_events->EVT_Trecv_fin );

    ocrEventCreate( &PTR_events->EVT_Lsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Rsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Lrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Rrecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &PTR_events->EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );

    ocrEventCreate( &PTR_events->EVT_Bsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Tsend_fin, OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &PTR_events->EVT_Brecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Trecv_start, OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &PTR_events->EVT_Brecv_fin, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &PTR_events->EVT_Trecv_fin, OCR_EVENT_STICKY_T, false );

    PTR_settingsH->itimestep = itimestep + 1;

    if( itimestep == PTR_gSettingsH->NT )
    {
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
        ocrEventSatisfy( PTR_events->EVT_reduction, DBK_refNorm);
        ocrAddDependence( PTR_events->EVT_reduction, PTR_globalH->EDT_reduction, 1+id, DB_MODE_RO );

    }

    return NULL_GUID;
}

ocrGuid_t FNC_reduction(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    ocrGuid_t DBK_gSettingsH = depv[0].guid;

    gSettingsH_t *PTR_gSettingsH = depv[0].ptr;

    s64 NT = PTR_gSettingsH->NT;
    s64 NR = PTR_gSettingsH->NR;
    s64 HR = PTR_gSettingsH->HR;
    s64 NP_X = PTR_gSettingsH->NP_X;
    s64 NP_Y = PTR_gSettingsH->NP_Y;

    s64 I;
    double refNorm = 0.0;

    for( I = 0; I < NR; I++ )
    {
        double* norm = depv[1+I].ptr;
        refNorm += norm[0];

    }
    double f_active_points = (double) ( (NP_X - 2*HR) * (NP_Y - 2*HR) );

    refNorm /= f_active_points;

    double reference_norm = (double) (NT)*2;

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
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_ITW );
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
