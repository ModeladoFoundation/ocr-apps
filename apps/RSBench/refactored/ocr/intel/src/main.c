/*
    Author: Chandra S. Martha
    Copywrite Intel Corporation 2015

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.

    This file is also subject to the license agreement located in the file LICENSE in the current directory.
*/

/* OCR implemenation of RSBench (mimicking MPI+OpenMP implementation)
 * A rank structure is created to manage each rank's settings and data.
 * Datablocks are created for each nuclide to store the cross-section data
 * 3 datablock per nuclide: 1 for poles, 1 for the windows and 1 for the K0RS values
 * See the dataH_t struct type for ocr datablock organization
 */

#include "ocr.h"
#include "rsbench.h"
#include "timers.h"

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
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
    s64 id; // rank id
} settingsH_t;

typedef struct
{
    ocrGuid_t DBK_nPoles_nuclide; //int n_poles[n_nuclides]
    ocrGuid_t DBK_nWindows_nuclide; //int n_windows[n_nuclides]
    ocrGuid_t DBK_pole_DBguids_nuclide; //ocrGuid_t pole_guids[n_nuclides]
    ocrGuid_t DBK_window_DBguids_nuclide; //ocrGuid_t window_guids[n_nuclides]
    ocrGuid_t DBK_pseudoK0RS_DBguids_nuclide; //ocrGuid_t pseudo_K0RS_guids[n_nuclides]
    ocrGuid_t DBK_numNucs_mat; //int num_nucs[n_mats];
    ocrGuid_t DBK_nuclideIDs_DBguids_mat; //ocrGuid_t nuclideIDs_DBguids[n_mats]; Each material has a nuclideID list
    ocrGuid_t DBK_nuclideConcs_DBguids_mat; //ocrGuid nuclideConcs_DBguids[n_mats]; //Each material has nuclide concentrations list
} dataH_t;

typedef struct
{
    ocrGuid_t TML_FNC_rankLookup;
    ocrGuid_t TML_FNC_macroxs;
    ocrGuid_t TML_FNC_microxsAggregator;
} rankTemplateH_t;

typedef struct
{
    ocrGuid_t DBK_settingsH; //-->settingsH_t
    ocrGuid_t DBK_dataH; //--> dataH_t
    ocrGuid_t DBK_templatesH;
} rankH_t;

typedef struct
{
    ocrGuid_t DBK_timers;
    ocrGuid_t DBK_InputsH; // Commandline parameters/settings
    ocrGuid_t DBK_InputsHs; //-> Broadcasted parameters/settings Inputs[]
    ocrGuid_t DBK_rankHs; //--> rankH_t[]
} globalH_t;

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_init_InputsH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_init_dataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    //ocrGuid_t FNC_init_materials(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_globalComputeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_rankMultiLookupSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_rankLookup(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_macroxs(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                        ocrGuid_t FNC_microxsAggregator(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_timer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_settingsInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    Inputs* PTR_InputsH_0 = depv[0].ptr;

    PTR_InputsH_0->nprocs = 1;
    PTR_InputsH_0->nthreads = (s32) paramv[0];
    PTR_InputsH_0->n_nuclides = (s32) paramv[1];
    PTR_InputsH_0->n_mats = (s32) paramv[2];
    PTR_InputsH_0->lookups = (s32) paramv[3];
    PTR_InputsH_0->avg_n_poles = (s32) paramv[4];
    PTR_InputsH_0->avg_n_windows = (s32) paramv[5];
    PTR_InputsH_0->HM = (HM_size) paramv[6];
    PTR_InputsH_0->numL = (s32) paramv[7];
    PTR_InputsH_0->doppler = (s32) paramv[8];

    //PRINTF("t %d i %ld m %ld g %ld l %ld s %s\n",
    //        PTR_InputsH_0->nthreads, PTR_InputsH_0->n_nuclides, PTR_InputsH_0->n_mats, PTR_InputsH_0->n_gridpoints, PTR_InputsH_0->lookups, PTR_InputsH_0->HM);

    return NULL_GUID;
}

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    border_print();
    center_print("INITIALIZATION", 79);
    border_print();

    ocrGuid_t DBK_InputsH_0 = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    Inputs* PTR_InputsH_0 = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;

    u64 nprocs = PTR_InputsH_0->nprocs;

    Inputs* PTR_InputsH;
    ocrDbCreate( &(PTR_globalH->DBK_InputsH), (void **) &PTR_InputsH, sizeof(Inputs),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    //settingsInit
    ocrGuid_t TS_init_InputsH_OET;
    ocrEventCreate( &TS_init_InputsH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_InputsH;

    TS_init_InputsH.FNC = FNC_init_InputsH;
    ocrEdtTemplateCreate( &TS_init_InputsH.TML, TS_init_InputsH.FNC, 0, 2 );

    ocrEdtCreate( &TS_init_InputsH.EDT, TS_init_InputsH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_init_InputsH.OET );

    ocrAddDependence( TS_init_InputsH.OET, TS_init_InputsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_init_InputsH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_init_InputsH.EDT, _idep++, DB_MODE_RW );

    ocrGuid_t* PTR_InputsHs;
    ocrDbCreate( &PTR_globalH->DBK_InputsHs, (void **) &PTR_InputsHs, sizeof(ocrGuid_t)*nprocs,
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t* PTR_rankHs;
    ocrDbCreate( &PTR_globalH->DBK_rankHs, (void **) &PTR_rankHs, sizeof(ocrGuid_t)*nprocs,
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    MyOcrTaskStruct_t TS_rankInitSpawner; _paramc = 0; _depc = 4;

    TS_rankInitSpawner.FNC = FNC_rankInitSpawner;
    ocrEdtTemplateCreate( &TS_rankInitSpawner.TML, TS_rankInitSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_rankInitSpawner.EDT, TS_rankInitSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL ); //Fires up individual EDTs to allocate subdomain DBs, EDTs

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->DBK_InputsHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_InputsH_OET, TS_rankInitSpawner.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_init_InputsH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    Inputs* PTR_InputsH_0 = depv[0].ptr;
    Inputs* PTR_InputsH = depv[1].ptr;

    PTR_InputsH->nprocs = PTR_InputsH_0->nprocs;
    PTR_InputsH->nthreads = PTR_InputsH_0->nthreads;
    PTR_InputsH->n_nuclides = PTR_InputsH_0->n_nuclides;
    PTR_InputsH->n_mats = PTR_InputsH_0->n_mats;
    PTR_InputsH->lookups = PTR_InputsH_0->lookups;
    PTR_InputsH->avg_n_poles = PTR_InputsH_0->avg_n_poles;
    PTR_InputsH->avg_n_windows = PTR_InputsH_0->avg_n_windows;
    PTR_InputsH->HM = PTR_InputsH_0->HM;
    PTR_InputsH->numL = PTR_InputsH_0->numL;
    PTR_InputsH->doppler = PTR_InputsH_0->doppler;

    return NULL_GUID;
}

ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_InputsHs = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    Inputs *PTR_InputsH = depv[0].ptr;
    ocrGuid_t* PTR_InputsHs = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 nprocs = PTR_InputsH->nprocs;

    //spawn N intializer EDTs
    //PRINTF("#Subdomains %d\n", nprocs);

    MyOcrTaskStruct_t TS_rankInit; _paramc = 1; _depc = 3;

    u64 init_paramv[1] = {-1}; //each spawned EDT gets its rank or id

    TS_rankInit.FNC = FNC_rankInit;
    ocrEdtTemplateCreate( &TS_rankInit.TML, TS_rankInit.FNC, _paramc, _depc );

    s64 i;
    for( i = 0; i < nprocs; i++ )
    {
        Inputs* PTR_InputsH;
        ocrDbCreate( &(PTR_InputsHs[i]), (void **) &PTR_InputsH, sizeof(Inputs),
                     DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

        rankH_t *PTR_rankH;
        ocrDbCreate( &(PTR_rankHs[i]), (void **) &PTR_rankH, sizeof(rankH_t),
                     DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate( &TS_rankInit.EDT, TS_rankInit.TML,
                      EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankInit.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_InputsHs[i], TS_rankInit.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_rankHs[i], TS_rankInit.EDT, _idep++, DB_MODE_RW );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH_0 = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DBK_InputsH = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_rankH = (ocrGuid_t) depv[2].guid;

    rankH_t *PTR_rankH = (rankH_t*) depv[2].ptr;

    //settingsInit
    ocrGuid_t TS_init_InputsH_OET;
    ocrEventCreate( &TS_init_InputsH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_InputsH; _paramc = 0; _depc = 2;

    TS_init_InputsH.FNC = FNC_init_InputsH;
    ocrEdtTemplateCreate( &TS_init_InputsH.TML, TS_init_InputsH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_InputsH.EDT, TS_init_InputsH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_init_InputsH.OET );

    ocrAddDependence( TS_init_InputsH.OET, TS_init_InputsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_init_InputsH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_InputsH, TS_init_InputsH.EDT, _idep++, DB_MODE_RW );

    settingsH_t *PTR_settingsH;
    ocrDbCreate( &(PTR_rankH->DBK_settingsH), (void **) &PTR_settingsH, sizeof(settingsH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    dataH_t *PTR_dataH;
    ocrDbCreate( &(PTR_rankH->DBK_dataH), (void **) &PTR_dataH, sizeof(dataH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    rankTemplateH_t *PTR_rankTemplateH;
    ocrDbCreate( &(PTR_rankH->DBK_templatesH), (void **) &PTR_rankTemplateH, sizeof(rankTemplateH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 1; _depc = 5;

    TS_init_rankH.FNC = FNC_init_rankH;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_rankH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_rankH->DBK_settingsH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_dataH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankH->DBK_templatesH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_init_InputsH_OET, TS_init_rankH.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s64 id = (s64) paramv[0];

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    dataH_t *PTR_dataH = (dataH_t*) depv[2].ptr;
    rankTemplateH_t *PTR_rankTemplateH = (rankTemplateH_t*) depv[3].ptr;

    PTR_settingsH->id = id;

    int n_nuclides = PTR_InputsH->n_nuclides;
    int n_mats = PTR_InputsH->n_mats;
    //PRINTF("nuclides %d mats %d\n", n_nuclides, n_mats);

    int *PTR_nPoles_nuclide, *PTR_nWindows_nuclide;
    ocrGuid_t *PTR_pole_DBguids_nuclide, *PTR_window_DBguids_nuclide, *PTR_pseudoK0RS_DBguids_nuclide;
    ocrDbCreate( &(PTR_dataH->DBK_nPoles_nuclide), (void **) &PTR_nPoles_nuclide, sizeof(int)*(n_nuclides),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_nWindows_nuclide), (void **) &PTR_nWindows_nuclide, sizeof(int)*(n_nuclides),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_pole_DBguids_nuclide), (void **) &PTR_pole_DBguids_nuclide, sizeof(ocrGuid_t)*(n_nuclides),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_window_DBguids_nuclide), (void **) &PTR_window_DBguids_nuclide, sizeof(ocrGuid_t)*(n_nuclides),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_pseudoK0RS_DBguids_nuclide), (void **) &PTR_pseudoK0RS_DBguids_nuclide, sizeof(ocrGuid_t)*(n_nuclides),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    int *PTR_numNucs_mat;
    ocrDbCreate( &(PTR_dataH->DBK_numNucs_mat), (void **) &PTR_numNucs_mat, sizeof(int)*(n_mats),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrGuid_t *PTR_nuclideIDs_DBguids_mat;
    ocrDbCreate( &(PTR_dataH->DBK_nuclideIDs_DBguids_mat), (void **) &PTR_nuclideIDs_DBguids_mat, sizeof(ocrGuid_t)*(n_mats),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrGuid_t *PTR_nuclideConcs_DBguids_mat;
    ocrDbCreate( &(PTR_dataH->DBK_nuclideConcs_DBguids_mat), (void **) &PTR_nuclideConcs_DBguids_mat, sizeof(ocrGuid_t)*(n_mats),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    _paramc = 2; _depc = 10;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_rankLookup), FNC_rankLookup, _paramc, _depc );
    _paramc = 3; _depc = 9;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_macroxs), FNC_macroxs, _paramc, _depc );
    _paramc = 3; _depc = EDT_PARAM_UNK;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_microxsAggregator), FNC_microxsAggregator, _paramc, _depc );

    MyOcrTaskStruct_t TS_init_dataH; _paramc = 0; _depc = 10;

    TS_init_dataH.FNC = FNC_init_dataH;
    ocrEdtTemplateCreate( &TS_init_dataH.TML, TS_init_dataH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_dataH.EDT, TS_init_dataH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_dataH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_settingsH, TS_init_dataH.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_dataH->DBK_nPoles_nuclide, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_nWindows_nuclide, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_pole_DBguids_nuclide, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_window_DBguids_nuclide, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_pseudoK0RS_DBguids_nuclide, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_numNucs_mat, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_nuclideIDs_DBguids_mat, TS_init_dataH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_dataH->DBK_nuclideConcs_DBguids_mat, TS_init_dataH.EDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t FNC_init_dataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep, _iparam;

    _idep = 0;
    ocrGuid_t DBK_InputsH = depv[_idep++].guid;
    ocrGuid_t DBK_settingsH = depv[_idep++].guid;
    ocrGuid_t DBK_nPoles_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_nWindows_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pole_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_window_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pseudoK0RS_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_numNucs_mat = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideIDs_DBguids_mat = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideConcs_DBguids_mat = depv[_idep++].guid;

    _iparam = 0;
    Inputs *PTR_InputsH = (Inputs*) depv[_iparam++].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[_iparam++].ptr;
    int* PTR_nPoles_nuclide = depv[_iparam++].ptr;
    int* PTR_nWindows_nuclide = depv[_iparam++].ptr;
    ocrGuid_t* PTR_pole_DBguids_nuclide = depv[_iparam++].ptr;
    ocrGuid_t* PTR_window_DBguids_nuclide = depv[_iparam++].ptr;
    ocrGuid_t* PTR_pseudoK0RS_DBguids_nuclide = depv[_iparam++].ptr;
    int *PTR_numNucs_mat = (int*) depv[_iparam++].ptr;
    ocrGuid_t *PTR_nuclideIDs_DBguids_mat = (ocrGuid_t *) depv[_iparam++].ptr;
    ocrGuid_t *PTR_nuclideConcs_DBguids_mat = (ocrGuid_t *) depv[_iparam++].ptr;

    int n_nuclides = PTR_InputsH->n_nuclides;
    int n_mats = PTR_InputsH->n_mats;

    s32 i;

    // Allocate & fill energy grids
    PRINTF("Generating resonance distributions...\n");
    generate_n_poles( *PTR_InputsH, PTR_nPoles_nuclide);

    // Allocate & fill Window grids
    PRINTF("Generating window distributions...\n");
    generate_n_windows( *PTR_InputsH, PTR_nWindows_nuclide );

    // Get material data
    PRINTF("Loading Hoogenboom-Martin material data...\n");
    get_materials( *PTR_InputsH, PTR_numNucs_mat, PTR_nuclideIDs_DBguids_mat, PTR_nuclideConcs_DBguids_mat);

    // Prepare full resonance grid
    PRINTF("Generating resonance parameter grid...\n");
    generate_poles( *PTR_InputsH, PTR_nPoles_nuclide, PTR_pole_DBguids_nuclide );

    // Prepare full Window grid
    PRINTF("Generating window parameter grid...\n");
    generate_window_params( *PTR_InputsH, PTR_nWindows_nuclide, PTR_nPoles_nuclide, PTR_window_DBguids_nuclide );

    //Prepare 0K Resonances
    PRINTF("Generating 0K l_value data...\n");
    generate_pseudo_K0RS( *PTR_InputsH, PTR_pseudoK0RS_DBguids_nuclide );

    return NULL_GUID;
}

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    border_print();
    center_print("SIMULATION", 79);
    border_print();

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t *PTR_globalH = depv[0].ptr;

    timer* PTR_timers;
    ocrDbCreate( &PTR_globalH->DBK_timers, (void**) &PTR_timers, sizeof(timer)*number_of_timers,
                 0, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
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

    ocrGuid_t TS_globalComputeSpawner_OET;
    ocrEventCreate( &TS_globalComputeSpawner_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalComputeSpawner; _paramc = 0; _depc = 3;

    TS_globalComputeSpawner.FNC = FNC_globalComputeSpawner;
    ocrEdtTemplateCreate( &TS_globalComputeSpawner.TML, TS_globalComputeSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalComputeSpawner.EDT, TS_globalComputeSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalComputeSpawner.OET );

    ocrAddDependence( TS_globalComputeSpawner.OET, TS_globalComputeSpawner_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->DBK_InputsHs, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_CONST );

    MyOcrTaskStruct_t TS_timer; _paramc = 0; _depc = 3;

    TS_timer.FNC = FNC_timer;
    ocrEdtTemplateCreate( &TS_timer.TML, TS_timer.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_timer.EDT, TS_timer.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &TS_timer.OET );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_timer.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_timers, TS_timer.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalComputeSpawner_OET, TS_timer.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_timer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    Inputs *PTR_InputsH = depv[0].ptr;
    timer* PTR_timers = depv[1].ptr;

    profile_stop( total_timer, PTR_timers );
    double runtime = get_elapsed_time( total_timer, PTR_timers );

    PRINTF("\n");
    print_results( *PTR_InputsH, runtime );

    return NULL_GUID;
}

ocrGuid_t FNC_globalComputeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("\n%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_InputsHs = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    ocrGuid_t* PTR_InputsHs = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 NR = PTR_InputsH->nprocs;
    u64 NL = PTR_InputsH->lookups;

    u64 i;

    //PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankCompute; _paramc = 4; _depc = 2;

    u64 compute_paramv[4];

    TS_rankCompute.FNC = FNC_rankCompute;
    ocrEdtTemplateCreate( &TS_rankCompute.TML, TS_rankCompute.FNC, _paramc, _depc );
    u64 ilookup = 0, NL_SYNC = 1024;

    for( i = 0; i < NR; i++ )
    {
        compute_paramv[0] = (u64) i;
        compute_paramv[1] = (u64) NL_SYNC;
        compute_paramv[2] = (u64) ilookup;
        compute_paramv[3] = (u64) MIN( ilookup + NL_SYNC - 1, NL - 1 );

        ocrEdtCreate( &TS_rankCompute.EDT, TS_rankCompute.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

        _idep = 0;
        ocrAddDependence( PTR_InputsHs[i], TS_rankCompute.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_rankHs[i], TS_rankCompute.EDT, _idep++, DB_MODE_CONST );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_rankH =  depv[1].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    rankH_t* PTR_rankH = (rankH_t*) depv[1].ptr;

    u64 id = (u64) paramv[0];
    u64 NL_SYNC = (u64) paramv[1];
    u64 ilookup = (u64) paramv[2];
    u64 ilookup_e = (u64) paramv[3];

    u64 NL = PTR_InputsH->lookups;

    //PRINTF("%s Lookups %d to %d\n", __func__, ilookup, ilookup_e);

    if( ilookup < NL )
    {
        ocrGuid_t TS_rankMultiLookupSpawner_OET;
        MyOcrTaskStruct_t TS_rankMultiLookupSpawner; _paramc = 3; _depc = 3;
        u64 TS_rankMultiLookupSpawner_paramv[3] = {id, ilookup, ilookup_e};

        ocrEventCreate( &TS_rankMultiLookupSpawner_OET, OCR_EVENT_STICKY_T, false );

        TS_rankMultiLookupSpawner.FNC = FNC_rankMultiLookupSpawner;
        ocrEdtTemplateCreate( &TS_rankMultiLookupSpawner.TML, TS_rankMultiLookupSpawner.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiLookupSpawner.EDT, TS_rankMultiLookupSpawner.TML,
                      EDT_PARAM_DEF, TS_rankMultiLookupSpawner_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_rankMultiLookupSpawner.OET );

        ocrAddDependence( TS_rankMultiLookupSpawner.OET, TS_rankMultiLookupSpawner_OET, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankMultiLookupSpawner.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_rankH->DBK_dataH, TS_rankMultiLookupSpawner.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_rankH->DBK_templatesH, TS_rankMultiLookupSpawner.EDT, _idep++, DB_MODE_CONST );

        ilookup += NL_SYNC;

        if( ilookup < NL )
        {
            MyOcrTaskStruct_t TS_rankCompute; _paramc = 4; _depc = 3;
            u64 mainComputeManager_paramv[4] = { id, NL_SYNC, ilookup, MIN( ilookup + NL_SYNC - 1, NL - 1) };

            TS_rankCompute.FNC = FNC_rankCompute;
            ocrEdtTemplateCreate( &TS_rankCompute.TML, TS_rankCompute.FNC, _paramc, _depc );

            ocrEdtCreate( &TS_rankCompute.EDT, TS_rankCompute.TML,
                          EDT_PARAM_DEF, mainComputeManager_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

            _idep = 0;
            ocrAddDependence( DBK_InputsH, TS_rankCompute.EDT, _idep++, DB_MODE_CONST );
            ocrAddDependence( DBK_rankH, TS_rankCompute.EDT, _idep++, DB_MODE_CONST );
            ocrAddDependence( TS_rankMultiLookupSpawner_OET, TS_rankCompute.EDT, _idep++, DB_MODE_NULL );
        }
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankMultiLookupSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_dataH =  depv[1].guid;
    ocrGuid_t DBK_templatesH = depv[2].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    dataH_t* PTR_dataH = (dataH_t*) depv[1].ptr;
    rankTemplateH_t* PTR_rankTemplateH = (rankTemplateH_t*) depv[2].ptr;

    u64 id = (u64) paramv[0];
    u64 ilookup_b = (u64) paramv[1];
    u64 ilookup_e = (u64) paramv[2];

    int lookups = PTR_InputsH->lookups;

    #if 1
    MyOcrTaskStruct_t TS_rankLookup; _paramc = 2; _depc = 10;

    u64 compute_paramv[2];

    for( u64 ilookup = ilookup_b; ilookup <= ilookup_e; ilookup++ )
    {
        //PRINTF("%d\n", ilookup);

        if( ilookup % 1000 == 0 )
            PRINTF("\rCalculating XS's... (%.02f%% completed)",
                    (ilookup / ( (double)lookups ) * 100.0 ) );

        compute_paramv[0] = (u64) id;
        compute_paramv[1] = (u64) ilookup;

        #if 1
        ocrEdtCreate( &TS_rankLookup.EDT, PTR_rankTemplateH->TML_FNC_rankLookup,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_templatesH, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_nPoles_nuclide, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_nWindows_nuclide, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_pole_DBguids_nuclide, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_window_DBguids_nuclide, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_pseudoK0RS_DBguids_nuclide, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_numNucs_mat, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_nuclideIDs_DBguids_mat, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_dataH->DBK_nuclideConcs_DBguids_mat, TS_rankLookup.EDT, _idep++, DB_MODE_CONST );
        #endif

    }
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_rankLookup(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep, _iparam;

    _idep = 0;
    ocrGuid_t DBK_InputsH = depv[_idep++].guid;
    ocrGuid_t DBK_templatesH = depv[_idep++].guid;
    ocrGuid_t DBK_nPoles_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_nWindows_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pole_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_window_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pseudoK0RS_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_numNucs_mat = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideIDs_DBguids_mat = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideConcs_DBguids_mat = depv[_idep++].guid;

    _idep = 0;
    Inputs* PTR_InputsH = depv[_idep++].ptr;
    rankTemplateH_t* PTR_rankTemplateH = (rankTemplateH_t*) depv[_idep++].ptr;
    int* PTR_nPoles_nuclide = (int*) depv[_idep++].ptr;
    int* PTR_nWindows_nuclide = (int*) depv[_idep++].ptr;
    ocrGuid_t* PTR_pole_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    ocrGuid_t* PTR_window_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    ocrGuid_t* PTR_pseudoK0RS_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    int *PTR_numNucs_mat = (int*) depv[_idep++].ptr;
    ocrGuid_t *PTR_nuclideIDs_DBguids_mat = depv[_idep++].ptr;
    ocrGuid_t *PTR_nuclideConcs_DBguids_mat = depv[_idep++].ptr;

    u64 id = (u64) paramv[0]; //we need worker_id here in the rank
    u64 ilookup = (u64) paramv[1];

    unsigned long seed = (ilookup+1)*19+17;

    double p_energy = rn(&seed);
    int mat = pick_mat(&seed);

    u64* PTR_p_energy_u64 = (u64*) &p_energy;

    #if 1
    MyOcrTaskStruct_t TS_macroxs; _paramc = 3; _depc = 6;
    u64 compute_paramv[3];

    //TS_macroxs.FNC = FNC_macroxs;
    //ocrEdtTemplateCreate( &TS_macroxs.TML, TS_macroxs.FNC, _paramc, _depc );

    compute_paramv[0] = (u64) *PTR_p_energy_u64;
    compute_paramv[1] = (u64) mat;
    compute_paramv[2] = (u64) PTR_numNucs_mat[mat];

    ocrEdtCreate( &TS_macroxs.EDT, PTR_rankTemplateH->TML_FNC_macroxs,
                  EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_templatesH, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nPoles_nuclide, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nWindows_nuclide, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_pole_DBguids_nuclide, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_window_DBguids_nuclide, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_pseudoK0RS_DBguids_nuclide, TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_nuclideIDs_DBguids_mat[mat], TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( PTR_nuclideConcs_DBguids_mat[mat], TS_macroxs.EDT, _idep++, DB_MODE_CONST );
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_macroxs(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_InputsH = depv[_idep++].guid;
    ocrGuid_t DBK_templatesH = depv[_idep++].guid;
    ocrGuid_t DBK_nPoles_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_nWindows_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pole_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_window_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_pseudoK0RS_DBguids_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideIDs_DBguids_mat_i = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideConcs_DBguids_mat_i = depv[_idep++].guid;

    _idep = 0;
    Inputs* PTR_InputsH = depv[_idep++].ptr;
    rankTemplateH_t* PTR_rankTemplateH = (rankTemplateH_t*) depv[_idep++].ptr;
    int* PTR_nPoles_nuclide = (int*) depv[_idep++].ptr;
    int* PTR_nWindows_nuclide = (int*) depv[_idep++].ptr;
    ocrGuid_t* PTR_pole_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    ocrGuid_t* PTR_window_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    ocrGuid_t* PTR_pseudoK0RS_DBguids_nuclide = (ocrGuid_t*) depv[_idep++].ptr;
    int *PTR_nuclideIDs_DBguids_mat_i = depv[_idep++].ptr;
    int *PTR_nuclideConcs_DBguids_mat_i = depv[_idep++].ptr;

    u64 p_energy_u64 = paramv[0];
    double* PTR_p_energy = (double*) &p_energy_u64;
    double p_energy = *PTR_p_energy;

    int mat = paramv[1];
    u64 num_nucs = paramv[2];

    #if 1
    MyOcrTaskStruct_t TS_microxsAggregator; _paramc = 3; _depc = 5 + 3*num_nucs;

    //TS_microxsAggregator.FNC = FNC_microxsAggregator;
    //ocrEdtTemplateCreate( &TS_microxsAggregator.TML, TS_microxsAggregator.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_microxsAggregator.EDT, PTR_rankTemplateH->TML_FNC_microxsAggregator,
                  EDT_PARAM_DEF, paramv, _depc, NULL,
                  EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nuclideIDs_DBguids_mat_i, TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nuclideConcs_DBguids_mat_i, TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nPoles_nuclide, TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_nWindows_nuclide, TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );

    int p_nuc;

    for( int j = 0; j < num_nucs; j++ )
    {
        p_nuc = PTR_nuclideIDs_DBguids_mat_i[j];
        ocrAddDependence( PTR_pole_DBguids_nuclide[p_nuc], TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_window_DBguids_nuclide[p_nuc], TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_pseudoK0RS_DBguids_nuclide[p_nuc], TS_microxsAggregator.EDT, _idep++, DB_MODE_CONST );
    }
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_microxsAggregator(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_InputsH = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideIDs_DBguids_mat_i = depv[_idep++].guid;
    ocrGuid_t DBK_nuclideConcs_DBguids_mat_i = depv[_idep++].guid;
    ocrGuid_t DBK_nPoles_nuclide = depv[_idep++].guid;
    ocrGuid_t DBK_nWindows_nuclide = depv[_idep++].guid;

    _idep = 0;
    Inputs* PTR_InputsH = depv[_idep++].ptr;
    int* PTR_nucl_index_list_i = (int*) depv[_idep++].ptr;
    double* PTR_nucl_concs_list_i = (double*) depv[_idep++].ptr;
    int* PTR_nPoles_nuclide = (int*) depv[_idep++].ptr;
    int* PTR_nWindows_nuclide = (int*) depv[_idep++].ptr;

    u64 p_energy_u64 = paramv[0];
    double* PTR_p_energy = (double*) &p_energy_u64;
    double p_energy = *PTR_p_energy;

    int mat = paramv[1];
    u64 num_nucs = paramv[2];

    double xs_vector[5];
    double macro_xs[4] = {0., 0., 0., 0.};
    complex double sigTfactors[4];

    CalcDataPtrs data = { PTR_nPoles_nuclide, PTR_nWindows_nuclide, NULL, NULL, NULL };

    int p_nuc;
    double conc;

    for( int i = 0; i < num_nucs; i++ )
    {
        double micro_xs[4];
        int p_nuc = PTR_nucl_index_list_i[i];

        Pole* PTR_pole_DBguids_nuclide_inuc = depv[_idep++].ptr;
        Window* PTR_window_DBguids_nuclide_inuc = depv[_idep++].ptr;
        double* PTR_pseudoK0RS_DBguids_nuclide_inuc = depv[_idep++].ptr;

        data.poles_inuc = PTR_pole_DBguids_nuclide_inuc;
        data.windows_inuc = PTR_window_DBguids_nuclide_inuc;
        data.pseudo_K0RS_inuc = PTR_pseudoK0RS_DBguids_nuclide_inuc;

        if( PTR_InputsH->doppler == 1 )
            calculate_micro_xs_doppler( micro_xs, p_nuc, p_energy, *PTR_InputsH, data, sigTfactors );
        else
            calculate_micro_xs( micro_xs, p_nuc, p_energy, *PTR_InputsH, data, sigTfactors);

        //printf("m %d p_nuc %d e %f xs_0 %f xs_1 %f xs_2 %f xs_3 %f\n", mat, p_nuc, p_energy, micro_xs[0], micro_xs[1], micro_xs[2], micro_xs[3]);
        //fflush(stdout);

        conc = PTR_nucl_concs_list_i[i];
        for( int j = 0; j < 4; j++ )
        {
            macro_xs[j] += micro_xs[j] * conc;
        }
    }

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("\n%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    int version = 13;
    int nprocs = 1;

    void* programArgv = depv[0].ptr;
    int argc = getArgc(programArgv);

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI( argc, programArgv );

    // Print-out of Input Summary
    print_input_summary( in );

    ocrGuid_t DBK_InputsH_0;
    Inputs* PTR_Inputs;

    ocrDbCreate( &DBK_InputsH_0, (void **) &PTR_Inputs, sizeof(Inputs),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t TS_settingsInit_OET;
    ocrEventCreate( &TS_settingsInit_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_settingsInit; _paramc = 9; _depc = 1;

    u64 settingsInit_paramv[] = { (u64) in.nthreads, (u64) in.n_nuclides, (u64) in.n_mats,
                                   (u64) in.lookups, (u64) in.avg_n_poles, (u64) in.avg_n_windows, (u64) in.HM,
                                   (u64) in.numL, (u64) in.doppler };

    TS_settingsInit.FNC = FNC_settingsInit;
    ocrEdtTemplateCreate( &TS_settingsInit.TML, TS_settingsInit.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_settingsInit.EDT, TS_settingsInit.TML,
                  EDT_PARAM_DEF, settingsInit_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_settingsInit.OET );

    ocrAddDependence( TS_settingsInit.OET, TS_settingsInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_settingsInit.EDT, _idep++, DB_MODE_RW );

    ocrGuid_t DBK_globalH;
    globalH_t* PTR_globalH;

    ocrDbCreate( &DBK_globalH, (void **) &PTR_globalH, sizeof(globalH_t),
                 DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t TS_globalInit_OET, TS_globalCompute_OET, TS_globalFinalize_OET;

    ocrEventCreate( &TS_globalInit_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalCompute_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalFinalize_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalInit, TS_globalCompute, TS_globalFinalize;
    _paramc = 0; _depc = 3;

    TS_globalInit.FNC = FNC_globalInit;
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_globalInit.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_globalH, TS_globalInit.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_settingsInit_OET, TS_globalInit.EDT, _idep++, DB_MODE_NULL );

    _paramc = 0; _depc = 2;
    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    _paramc = 0; _depc = 2;
    TS_globalFinalize.FNC = FNC_globalFinalize;
    ocrEdtTemplateCreate( &TS_globalFinalize.TML, TS_globalFinalize.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalFinalize.EDT, TS_globalFinalize.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &TS_globalFinalize.OET );

    ocrAddDependence( TS_globalFinalize.OET, TS_globalFinalize_OET, 0, DB_MODE_NULL);

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalFinalize.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( TS_globalCompute_OET, TS_globalFinalize.EDT, _idep++, DB_MODE_NULL);

    return NULL_GUID;
}
