#include "ocr.h"
#include "XSbench_header.h"

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
    s64 id; // rank id, local bounds, local points
} settingsH_t;

typedef struct
{
    int num_nucs;
    ocrGuid_t DBK_nucl_index_list;
    ocrGuid_t DBK_nucl_concs_list;
} materialH_t;

typedef struct
{
    ocrGuid_t DBK_nuclide_grids;
    ocrGuid_t DBK_uEnergy, DBK_uEnergy_grid;
    ocrGuid_t DBK_materials;
} dataH_t;

typedef struct
{
    ocrGuid_t DBK_settingsH; //-->settingsH_t
    ocrGuid_t DBK_dataH; //--> dataH_t
    ocrGuid_t DBK_eventHs[2]; //--> eventH_t[2]
} rankH_t;

typedef struct
{
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
                    ocrGuid_t FNC_init_uEnergy(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                        ocrGuid_t FNC_init_uEnergy_i(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_init_materials(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                        ocrGuid_t FNC_init_materials_is(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FNC_globalComputeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FNC_rankCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FNC_rankMultiLookupSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FNC_rankLookup(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FNC_microxs(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                        ocrGuid_t FNC_microxs_0(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                            ocrGuid_t FNC_microxs_1(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_settingsInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    Inputs* PTR_InputsH_0 = depv[0].ptr;

    PTR_InputsH_0->nprocs = (s32) 1;
    PTR_InputsH_0->nthreads = (s32) paramv[0];
    PTR_InputsH_0->n_isotopes = (s64) paramv[1];
    PTR_InputsH_0->n_mats = (s64) paramv[2];
    PTR_InputsH_0->n_gridpoints = (s64) paramv[3];
    PTR_InputsH_0->lookups = (s64) paramv[4];
    strcpy( PTR_InputsH_0->HM, "small" );

    PRINTF("t %d i %ld m %ld g %ld l %ld s %s\n",
            PTR_InputsH_0->nthreads, PTR_InputsH_0->n_isotopes, PTR_InputsH_0->n_mats, PTR_InputsH_0->n_gridpoints, PTR_InputsH_0->lookups, PTR_InputsH_0->HM);

    return NULL_GUID;
}

ocrGuid_t FNC_globalInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH_0 = depv[0].guid;
    ocrGuid_t DBK_globalH = depv[1].guid;

    Inputs* PTR_InputsH_0 = depv[0].ptr;
    globalH_t* PTR_globalH = depv[1].ptr;

    u64 nprocs = PTR_InputsH_0->nprocs;

    Inputs* PTR_InputsH;
    ocrDbCreate( &(PTR_globalH->DBK_InputsH), (void **) &PTR_InputsH, sizeof(Inputs),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    //settingsInit
    ocrGuid_t TS_init_InputsH_OET;
    ocrEventCreate( &TS_init_InputsH_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_init_InputsH;

    TS_init_InputsH.FNC = FNC_init_InputsH;
    ocrEdtTemplateCreate( &TS_init_InputsH.TML, TS_init_InputsH.FNC, 0, 2 );

    ocrEdtCreate( &TS_init_InputsH.EDT, TS_init_InputsH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, &TS_init_InputsH.OET );

    ocrAddDependence( TS_init_InputsH.OET, TS_init_InputsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_init_InputsH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_init_InputsH.EDT, _idep++, DB_MODE_ITW );

    ocrGuid_t* PTR_InputsHs;
    ocrDbCreate( &PTR_globalH->DBK_InputsHs, (void **) &PTR_InputsHs, sizeof(ocrGuid_t)*nprocs,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t* PTR_rankHs;
    ocrDbCreate( &PTR_globalH->DBK_rankHs, (void **) &PTR_rankHs, sizeof(ocrGuid_t)*nprocs,
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    MyOcrTaskStruct_t TS_rankInitSpawner; _paramc = 0; _depc = 4;

    TS_rankInitSpawner.FNC = FNC_rankInitSpawner;
    ocrEdtTemplateCreate( &TS_rankInitSpawner.TML, TS_rankInitSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_rankInitSpawner.EDT, TS_rankInitSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL ); //Fires up individual EDTs to allocate subdomain DBs, EDTs

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_rankInitSpawner.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_InputsHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_rankInitSpawner.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_init_InputsH_OET, TS_rankInitSpawner.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_init_InputsH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    Inputs* PTR_InputsH_0 = depv[0].ptr;
    Inputs* PTR_InputsH = depv[1].ptr;

    PTR_InputsH->nprocs = PTR_InputsH_0->nprocs;
    PTR_InputsH->nthreads = PTR_InputsH_0->nthreads;
    PTR_InputsH->n_isotopes = PTR_InputsH_0->n_isotopes;
    PTR_InputsH->n_mats = PTR_InputsH_0->n_mats;
    PTR_InputsH->n_gridpoints = PTR_InputsH_0->n_gridpoints;
    PTR_InputsH->lookups = PTR_InputsH_0->lookups;
    strcpy( PTR_InputsH->HM, PTR_InputsH_0->HM );

    return NULL_GUID;
}

ocrGuid_t FNC_rankInitSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_InputsHs = depv[1].guid;
    ocrGuid_t DBK_rankHs = depv[2].guid;

    Inputs *PTR_InputsH = depv[0].ptr;
    ocrGuid_t* PTR_InputsHs = depv[1].ptr;
    ocrGuid_t* PTR_rankHs = depv[2].ptr;

    u64 nprocs = PTR_InputsH->nprocs;

    //spawn N intializer EDTs
    PRINTF("#Subdomains %d\n", nprocs);

    MyOcrTaskStruct_t TS_rankInit; _paramc = 1; _depc = 3;

    u64 init_paramv[1] = {-1}; //each spawned EDT gets its rank or id

    TS_rankInit.FNC = FNC_rankInit;
    ocrEdtTemplateCreate( &TS_rankInit.TML, TS_rankInit.FNC, _paramc, _depc );

    s64 i;
    for( i = 0; i < nprocs; i++ )
    {
        Inputs* PTR_InputsH;
        ocrDbCreate( &(PTR_InputsHs[i]), (void **) &PTR_InputsH, sizeof(Inputs),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        rankH_t *PTR_rankH;
        ocrDbCreate( &(PTR_rankHs[i]), (void **) &PTR_rankH, sizeof(rankH_t),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate( &TS_rankInit.EDT, TS_rankInit.TML,
                      EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankInit.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_InputsHs[i], TS_rankInit.EDT, _idep++, DB_MODE_ITW );
        ocrAddDependence( PTR_rankHs[i], TS_rankInit.EDT, _idep++, DB_MODE_ITW );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_rankInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
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
                  EDT_PROP_NONE, NULL_GUID, &TS_init_InputsH.OET );

    ocrAddDependence( TS_init_InputsH.OET, TS_init_InputsH_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_init_InputsH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_InputsH, TS_init_InputsH.EDT, _idep++, DB_MODE_ITW );

    settingsH_t *PTR_settingsH;
    ocrDbCreate( &(PTR_rankH->DBK_settingsH), (void **) &PTR_settingsH, sizeof(settingsH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    dataH_t *PTR_dataH;
    ocrDbCreate( &(PTR_rankH->DBK_dataH), (void **) &PTR_dataH, sizeof(dataH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    #if 0
    s32 i;
    for( i = 0; i < 2; i++ )
    {
        eventH_t* PTR_eventH;
        ocrDbCreate(    &(PTR_rankH->DBK_eventHs[i]), (void **) &PTR_eventH, sizeof(eventH_t),
                        DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    }
    #endif

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 1; _depc = 4;

    TS_init_rankH.FNC = FNC_init_rankH;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_rankH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_settingsH, TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_rankH->DBK_dataH, TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    //ocrAddDependence( PTR_rankH->DBK_eventHs[0], TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    //ocrAddDependence( PTR_rankH->DBK_eventHs[1], TS_init_rankH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_init_InputsH_OET, TS_init_rankH.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FNC_init_rankH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s64 id = (s64) paramv[0];

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    dataH_t *PTR_dataH = (dataH_t*) depv[2].ptr;

    PTR_settingsH->id = id;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_mats = PTR_InputsH->n_mats;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    ocrGuid_t *PTR_nuclide_grids, *PTR_uEnergy_grid;
    double* PTR_uEnergy;
    ocrDbCreate( &(PTR_dataH->DBK_nuclide_grids), (void **) &PTR_nuclide_grids, sizeof(ocrGuid_t)*(n_isotopes),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_uEnergy), (void **) &PTR_uEnergy, sizeof(double)*(n_isotopes*n_gridpoints),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &(PTR_dataH->DBK_uEnergy_grid), (void **) &PTR_uEnergy_grid, sizeof(ocrGuid_t)*(n_isotopes*n_gridpoints),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t *PTR_material_guids;
    ocrDbCreate( &(PTR_dataH->DBK_materials), (void **) &PTR_material_guids, sizeof(ocrGuid_t)*(n_mats),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    MyOcrTaskStruct_t TS_init_dataH; _paramc = 0; _depc = 6;

    TS_init_dataH.FNC = FNC_init_dataH;
    ocrEdtTemplateCreate( &TS_init_dataH.TML, TS_init_dataH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_dataH.EDT, TS_init_dataH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_dataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_init_dataH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_dataH->DBK_nuclide_grids, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_dataH->DBK_uEnergy, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_dataH->DBK_uEnergy_grid, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( PTR_dataH->DBK_materials, TS_init_dataH.EDT, _idep++, DB_MODE_ITW );

    #if 0
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
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_init_dataH(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_nuclide_grids = depv[2].guid;
    ocrGuid_t DBK_uEnergy = depv[3].guid;
    ocrGuid_t DBK_uEnergy_grid = depv[4].guid;
    ocrGuid_t DBK_materials = depv[5].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    ocrGuid_t *PTR_nuclide_grids = (ocrGuid_t*) depv[2].ptr;
    double *PTR_uEnergy = (double*) depv[3].ptr;
    ocrGuid_t *PTR_uEnergy_grid = (ocrGuid_t*) depv[4].ptr;
    ocrGuid_t *PTR_material_guids = (ocrGuid_t*) depv[5].ptr;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_mats = PTR_InputsH->n_mats;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    NuclideGridPoint* PTR_nuclide_data;

    int (*cmp) (const void *, const void *);
    cmp = NGP_compare;

    s32 i;

    for( i = 0; i < n_isotopes; i++ )
    {
        ocrDbCreate( &(PTR_nuclide_grids[i]), (void **) &PTR_nuclide_data, sizeof(NuclideGridPoint)*(n_gridpoints),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        for( long j = 0; j < n_gridpoints; j++ )
        {
            PTR_nuclide_data[j].energy        = ((double)rand()/(double)RAND_MAX);
            PTR_nuclide_data[j].total_xs      = ((double)rand()/(double)RAND_MAX);
            PTR_nuclide_data[j].elastic_xs    = ((double)rand()/(double)RAND_MAX);
            PTR_nuclide_data[j].absorbtion_xs = ((double)rand()/(double)RAND_MAX);
            PTR_nuclide_data[j].fission_xs    = ((double)rand()/(double)RAND_MAX);
            PTR_nuclide_data[j].nu_fission_xs = ((double)rand()/(double)RAND_MAX);

            //PRINTF("Init p %d e %f xs_0 %f xs_1 %f xs_2 %f xs_3 %f xs_4 %f\n", i, PTR_nuclide_data[j].energy, PTR_nuclide_data[j].total_xs, PTR_nuclide_data[j].elastic_xs, PTR_nuclide_data[j].absorbtion_xs, PTR_nuclide_data[j].fission_xs, PTR_nuclide_data[j].nu_fission_xs);
        }

        //sort_nuclide_grids

        qsort( PTR_nuclide_data, n_gridpoints, sizeof(NuclideGridPoint), cmp );

        for( long j = 0; j < n_gridpoints; j++ )
        {
            PTR_uEnergy[i*n_gridpoints + j] = PTR_nuclide_data[j].energy;
        }
    }

    qsort( PTR_uEnergy, n_isotopes*n_gridpoints, sizeof(double), cmp );

    int* PTR_xs_indices;

    for( i = 0; i < n_gridpoints*n_isotopes; i++ )
    {
        ocrDbCreate( &(PTR_uEnergy_grid[i]), (void **) &PTR_xs_indices, sizeof(int)*n_isotopes,
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    }

    MyOcrTaskStruct_t TS_init_uEnergy; _paramc = 0; _depc = 5;

    TS_init_uEnergy.FNC = FNC_init_uEnergy;
    ocrEdtTemplateCreate( &TS_init_uEnergy.TML, TS_init_uEnergy.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_uEnergy.EDT, TS_init_uEnergy.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_uEnergy.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_init_uEnergy.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_nuclide_grids, TS_init_uEnergy.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_uEnergy, TS_init_uEnergy.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_uEnergy_grid, TS_init_uEnergy.EDT, _idep++, DB_MODE_RO );

    materialH_t* PTR_materialH;

    for( i = 0; i < n_mats; i++ )
    {
        ocrDbCreate( &(PTR_material_guids[i]), (void **) &PTR_materialH, sizeof(materialH_t),
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    }

    MyOcrTaskStruct_t TS_init_materials; _paramc = 0; _depc = 3;

    TS_init_materials.FNC = FNC_init_materials;
    ocrEdtTemplateCreate( &TS_init_materials.TML, TS_init_materials.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_materials.EDT, TS_init_materials.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_materials.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_init_materials.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_materials, TS_init_materials.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t FNC_init_uEnergy(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_nuclide_grids = depv[2].guid;
    ocrGuid_t DBK_uEnergy = depv[3].guid;
    ocrGuid_t DBK_uEnergy_grid = depv[4].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    ocrGuid_t *PTR_nuclide_grids = (ocrGuid_t*) depv[2].ptr;
    double *PTR_uEnergy = (double*) depv[3].ptr;
    ocrGuid_t *PTR_uEnergy_grid = (ocrGuid_t*) depv[4].ptr;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    s32 i, j, k;
    double quarry;

    MyOcrTaskStruct_t TS_init_uEnergy_i; _paramc = 2; _depc = 4+n_isotopes;
    u64 TS_init_uEnergy_i_paramv[2];

    TS_init_uEnergy_i.FNC = FNC_init_uEnergy_i;
    ocrEdtTemplateCreate( &TS_init_uEnergy_i.TML, TS_init_uEnergy_i.FNC, _paramc, _depc );

    for( k = 0; k < n_gridpoints*n_isotopes; k++ )
    {
        i = k;//*10000;
        quarry = PTR_uEnergy[i]; //paramv[1];
        u64* PTR_quarry_u64 = (u64*) &quarry;
        TS_init_uEnergy_i_paramv[0] = i;
        TS_init_uEnergy_i_paramv[1] = *PTR_quarry_u64;
        ocrEdtCreate( &TS_init_uEnergy_i.EDT, TS_init_uEnergy_i.TML,
                      EDT_PARAM_DEF, TS_init_uEnergy_i_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_init_uEnergy_i.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_settingsH, TS_init_uEnergy_i.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_uEnergy, TS_init_uEnergy_i.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_uEnergy_grid[i], TS_init_uEnergy_i.EDT, _idep++, DB_MODE_ITW );

        for( j = 0; j < n_isotopes; j++ )
        {
            ocrAddDependence( PTR_nuclide_grids[j], TS_init_uEnergy_i.EDT, _idep++, DB_MODE_RO );
        }
    }

    return NULL_GUID;
}

ocrGuid_t FNC_init_uEnergy_i(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_uEnergy = depv[2].guid;
    ocrGuid_t DBK_uEnergy_grid_i = depv[3].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    double *PTR_uEnergy = (double*) depv[2].ptr;
    int* PTR_xs_indices_i = (int*) depv[3].ptr;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    s32 i = paramv[0];
    s32 j;

    //double quarry = PTR_uEnergy[i]; //paramv[1];
    u64 quarry_u64 = paramv[1];
    double* PTR_quarry = (double*) &quarry_u64;
    double quarry = *PTR_quarry;
    //PRINTF("here2 quarry %f\n", quarry);
    int thread_id = 0;
    //if( thread_id == 0 && i % 200 == 0 )
        printf("\rAligning Unionized Grid...(%.0lf%% complete)",
               100.0 * (double) i / (n_isotopes*n_gridpoints /
                                     1/*omp_get_num_threads()*/)     );
    _idep = 4;
    for( long j = 0; j < n_isotopes; j++ )
    {
      // j is the nuclide i.d.
      // log n binary search
      NuclideGridPoint* PTR_nuclide_data_j = depv[_idep+j].ptr;
      PTR_xs_indices_i[j] =
          binary_search( PTR_nuclide_data_j, quarry, n_gridpoints);
      //PRINTF("Index %d\n", PTR_xs_indices_i[j]);
    }

    return NULL_GUID;
}

ocrGuid_t FNC_init_materials(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;
    ocrGuid_t DBK_materials = depv[2].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;
    ocrGuid_t *PTR_material_guids = (ocrGuid_t*) depv[2].ptr;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_mats = PTR_InputsH->n_mats;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    s32 i, j, k;

    MyOcrTaskStruct_t TS_init_materials_is; _paramc = 0; _depc = 2+n_mats;

    TS_init_materials_is.FNC = FNC_init_materials_is;
    ocrEdtTemplateCreate( &TS_init_materials_is.TML, TS_init_materials_is.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_materials_is.EDT, TS_init_materials_is.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_init_materials_is.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_settingsH, TS_init_materials_is.EDT, _idep++, DB_MODE_RO );

    for( j = 0; j < n_mats; j++ )
    {
        ocrAddDependence( PTR_material_guids[j], TS_init_materials_is.EDT, _idep++, DB_MODE_ITW );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_init_materials_is(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_settingsH = depv[1].guid;

    Inputs *PTR_InputsH = (Inputs*) depv[0].ptr;
    settingsH_t *PTR_settingsH = (settingsH_t*) depv[1].ptr;

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_mats = PTR_InputsH->n_mats;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    s32 i, j, k;

    int num_nucs[12] = { 34, 5, 4, 4, 27, 21, 21, 21, 21, 21, 9, 9};

    // Material 0 is a special case (fuel). The H-M small reactor uses
    // 34 nuclides, while H-M larges uses 300.
    if( n_isotopes == 68 )
        num_nucs[0]  = 34; // HM Small is 34, H-M Large is 321
    else
        num_nucs[0]  = 321; // HM Small is 34, H-M Large is 321

    // Small H-M has 34 fuel nuclides
    int mats0_Sml[] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                     8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                     54, 55, 56, 18, 23, 41 }; //fuel
    // Large H-M has 300 fuel nuclides
    int mats0_Lrg[321] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                     8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                     54, 55, 56, 18, 23, 41 }; //fuel
    for( int i = 0; i < 321-34; i++ )
        mats0_Lrg[34+i] = 68 + i; // H-M large adds nuclides to fuel only

    // These are the non-fuel materials
    int mats1[] =  { 63, 64, 65, 66, 67 }; // cladding
    int mats2[] =  { 24, 41, 4, 5 }; // cold borated water
    int mats3[] =  { 24, 41, 4, 5 }; // hot borated water
    int mats4[] =  { 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, 27, 28, 29,
                     30, 31, 32, 26, 49, 50, 51, 11, 12, 13, 14, 6, 16,
                     17 }; // RPV
    int mats5[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                     49, 50, 51, 11, 12, 13, 14 }; // lower radial reflector
    int mats6[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                     49, 50, 51, 11, 12, 13, 14 }; // top reflector / plate
    int mats7[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                     49, 50, 51, 11, 12, 13, 14 }; // bottom plate
    int mats8[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                     49, 50, 51, 11, 12, 13, 14 }; // bottom nozzle
    int mats9[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                     49, 50, 51, 11, 12, 13, 14 }; // top nozzle
    int mats10[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // top of FA's
    int mats11[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // bottom FA's


    ocrGuid_t nucl_index_list;
    ocrGuid_t nucl_concs_list;
    int* mats_ptr;

    _idep = 2;
    for( j = 0; j < n_mats; j++ )
    {
        materialH_t* PTR_material_guids_j = depv[_idep++].ptr;
        PTR_material_guids_j->num_nucs = num_nucs[j];
        int num_nucs_j = PTR_material_guids_j->num_nucs;

        int* PTR_nucl_index_list;
        ocrDbCreate( &(PTR_material_guids_j->DBK_nucl_index_list), (void **) &PTR_nucl_index_list, sizeof(int)*num_nucs_j,
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        double* PTR_nucl_concs_list;;
        ocrDbCreate( &(PTR_material_guids_j->DBK_nucl_concs_list), (void **) &PTR_nucl_concs_list, sizeof(double)*num_nucs_j,
                     DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        for( int i = 0; i < num_nucs_j; i++ )
            PTR_nucl_concs_list[i] = (double) rand() / (double) RAND_MAX;

        switch(j)
        {
            case 0:
                if( n_isotopes == 68 )
                    mats_ptr = mats0_Sml;
                else
                    mats_ptr = mats0_Lrg;
                break;
            case 1:
                mats_ptr = mats1;
                break;
            case 2:
                mats_ptr = mats2;
                break;
            case 3:
                mats_ptr = mats3;
                break;
            case 4:
                mats_ptr = mats4;
                break;
            case 5:
                mats_ptr = mats5;
                break;
            case 6:
                mats_ptr = mats6;
                break;
            case 7:
                mats_ptr = mats7;
                break;
            case 8:
                mats_ptr = mats8;
                break;
            case 9:
                mats_ptr = mats9;
                break;
            case 10:
                mats_ptr = mats10;
                break;
            case 11:
                mats_ptr = mats11;
                break;
            default:
                PRINTF("Material not found!\n");
                break;
        }

        memcpy( PTR_nucl_index_list,  mats_ptr,  num_nucs_j  * sizeof(int) );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_globalCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrGuid_t DBK_globalH = depv[0].guid;

    globalH_t *PTR_globalH = depv[0].ptr;

    MyOcrTaskStruct_t TS_globalComputeSpawner; _paramc = 0; _depc = 3;

    TS_globalComputeSpawner.FNC = FNC_globalComputeSpawner;
    ocrEdtTemplateCreate( &TS_globalComputeSpawner.TML, TS_globalComputeSpawner.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalComputeSpawner.EDT, TS_globalComputeSpawner.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( PTR_globalH->DBK_InputsH, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_InputsHs, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_globalH->DBK_rankHs, TS_globalComputeSpawner.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t FNC_globalComputeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("\n%s\n", __func__);
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

    PRINTF("#Subdomains %d\n", NR);

    MyOcrTaskStruct_t TS_rankCompute; _paramc = 4; _depc = 2;

    u64 compute_paramv[4];

    TS_rankCompute.FNC = FNC_rankCompute;
    ocrEdtTemplateCreate( &TS_rankCompute.TML, TS_rankCompute.FNC, _paramc, _depc );
    u64 ilookup = 0, NL_SYNC = 100;

    for( i = 0; i < NR; i++ )
    {
        compute_paramv[0] = (u64) i;
        compute_paramv[1] = (u64) NL_SYNC;
        compute_paramv[2] = (u64) ilookup;
        compute_paramv[3] = (u64) MIN( ilookup + NL_SYNC - 1, NL - 1 );

        ocrEdtCreate( &TS_rankCompute.EDT, TS_rankCompute.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( PTR_InputsHs[i], TS_rankCompute.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_rankHs[i], TS_rankCompute.EDT, _idep++, DB_MODE_RO );
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
        MyOcrTaskStruct_t TS_rankMultiLookupSpawner; _paramc = 3; _depc = 2;
        u64 TS_rankMultiLookupSpawner_paramv[3] = {id, ilookup, ilookup_e};

        ocrEventCreate( &TS_rankMultiLookupSpawner_OET, OCR_EVENT_STICKY_T, false );

        TS_rankMultiLookupSpawner.FNC = FNC_rankMultiLookupSpawner;
        ocrEdtTemplateCreate( &TS_rankMultiLookupSpawner.TML, TS_rankMultiLookupSpawner.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_rankMultiLookupSpawner.EDT, TS_rankMultiLookupSpawner.TML,
                      EDT_PARAM_DEF, TS_rankMultiLookupSpawner_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_GUID, &TS_rankMultiLookupSpawner.OET );

        ocrAddDependence( TS_rankMultiLookupSpawner.OET, TS_rankMultiLookupSpawner_OET, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankMultiLookupSpawner.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_rankH->DBK_dataH, TS_rankMultiLookupSpawner.EDT, _idep++, DB_MODE_RO );

        ilookup += NL_SYNC;

        if( ilookup < NL )
        {
            MyOcrTaskStruct_t TS_rankCompute; _paramc = 4; _depc = 3;
            u64 mainComputeManager_paramv[4] = { id, NL_SYNC, ilookup, MIN( ilookup + NL_SYNC - 1, NL - 1) };

            TS_rankCompute.FNC = FNC_rankCompute;
            ocrEdtTemplateCreate( &TS_rankCompute.TML, TS_rankCompute.FNC, _paramc, _depc );

            ocrEdtCreate( &TS_rankCompute.EDT, TS_rankCompute.TML,
                          EDT_PARAM_DEF, mainComputeManager_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, NULL );

            _idep = 0;
            ocrAddDependence( DBK_InputsH, TS_rankCompute.EDT, _idep++, DB_MODE_RO );
            ocrAddDependence( DBK_rankH, TS_rankCompute.EDT, _idep++, DB_MODE_RO );
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

    Inputs* PTR_InputsH = depv[0].ptr;
    dataH_t* PTR_dataH = (dataH_t*) depv[1].ptr;

    u64 id = (u64) paramv[0];
    u64 ilookup_b = (u64) paramv[1];
    u64 ilookup_e = (u64) paramv[2];

    int lookups = PTR_InputsH->lookups;

    #if 1
    MyOcrTaskStruct_t TS_rankLookup; _paramc = 2; _depc = 5;

    u64 compute_paramv[2];

    TS_rankLookup.FNC = FNC_rankLookup;
    ocrEdtTemplateCreate( &TS_rankLookup.TML, TS_rankLookup.FNC, _paramc, _depc );

    for( u64 ilookup = ilookup_b; ilookup <= ilookup_e; ilookup++ )
    {
        //PRINTF("%d\n", ilookup);

        if( ilookup % 1000 == 0 )
            printf("\rCalculating XS's... (%.0lf%% completed)",
                    (ilookup / ( (double)lookups ) * 100.0 ) );

        compute_paramv[0] = (u64) id;
        compute_paramv[1] = (u64) ilookup;

        ocrEdtCreate( &TS_rankLookup.EDT, TS_rankLookup.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( DBK_InputsH, TS_rankLookup.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_dataH->DBK_nuclide_grids, TS_rankLookup.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_dataH->DBK_uEnergy, TS_rankLookup.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_dataH->DBK_uEnergy_grid, TS_rankLookup.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( PTR_dataH->DBK_materials, TS_rankLookup.EDT, _idep++, DB_MODE_RO );

    }
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_rankLookup(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_nuclide_grids =  depv[1].guid;
    ocrGuid_t DBK_uEnergy =  depv[2].guid;
    ocrGuid_t DBK_uEnergy_grid =  depv[3].guid;
    ocrGuid_t DBK_materials =  depv[4].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    ocrGuid_t *PTR_nuclide_grids = (ocrGuid_t*) depv[1].ptr;
    double *PTR_uEnergy = (double*) depv[2].ptr;
    ocrGuid_t *PTR_uEnergy_grid = (ocrGuid_t*) depv[3].ptr;
    ocrGuid_t *PTR_material_guids = (ocrGuid_t*) depv[4].ptr;

    u64 id = (u64) paramv[0]; //we need worker_id here in the rank
    u64 ilookup = (u64) paramv[1];

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    unsigned long seed = rand(); //(ilookup+1)*19+17;

    double p_energy = rn(&seed);
    int mat = pick_mat(&seed);

    u64* PTR_p_energy_u64 = (u64*) &p_energy;

    long idx = 0;
    // binary search for energy on unionized energy grid (UEG)
    idx = grid_search_double( n_isotopes * n_gridpoints, p_energy,
                       PTR_uEnergy);
    //PRINTF("e %f m %d idx %d\n", p_energy, mat, idx);

    #if 1
    MyOcrTaskStruct_t TS_microxs; _paramc = 2; _depc = 4;
    u64 compute_paramv[2];

    TS_microxs.FNC = FNC_microxs;
    ocrEdtTemplateCreate( &TS_microxs.TML, TS_microxs.FNC, _paramc, _depc );

    compute_paramv[0] = (u64) *PTR_p_energy_u64;
    compute_paramv[1] = (u64) mat;

    ocrEdtCreate( &TS_microxs.EDT, TS_microxs.TML,
                  EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_microxs.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_nuclide_grids, TS_microxs.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_uEnergy_grid[idx], TS_microxs.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_material_guids[mat], TS_microxs.EDT, _idep++, DB_MODE_RO );
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_microxs(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_nuclide_grids =  depv[1].guid;
    ocrGuid_t DBK_uEnergy_grid_i =  depv[2].guid;
    ocrGuid_t DBK_materials_i =  depv[3].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    ocrGuid_t *PTR_nuclide_grids = (ocrGuid_t*) depv[1].ptr;
    int* PTR_xs_indices = (int*) depv[2].ptr;;
    materialH_t *PTR_materials_i = (materialH_t*) depv[3].ptr;

    u64 p_energy_u64 = paramv[0];
    double* PTR_p_energy = (double*) &p_energy_u64;
    double p_energy = *PTR_p_energy;

    int mat = paramv[1];

    u64 num_nucs = PTR_materials_i->num_nucs;

    #if 1
    MyOcrTaskStruct_t TS_microxs_0; _paramc = 3; _depc = 5;
    u64 compute_paramv[3] = {p_energy_u64, num_nucs, mat};

    TS_microxs_0.FNC = FNC_microxs_0;
    ocrEdtTemplateCreate( &TS_microxs_0.TML, TS_microxs_0.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_microxs_0.EDT, TS_microxs_0.TML,
                  EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_microxs_0.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_nuclide_grids, TS_microxs_0.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_uEnergy_grid_i, TS_microxs_0.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_materials_i->DBK_nucl_index_list, TS_microxs_0.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_materials_i->DBK_nucl_concs_list, TS_microxs_0.EDT, _idep++, DB_MODE_RO );
    #endif

    return NULL_GUID;
}

ocrGuid_t FNC_microxs_0(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_nuclide_grids =  depv[1].guid;
    ocrGuid_t DBK_uEnergy_grid_i =  depv[2].guid;
    ocrGuid_t DBK_nucl_index_list = depv[3].guid;
    ocrGuid_t DBK_nucl_concs_list = depv[4].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    ocrGuid_t *PTR_nuclide_grids = (ocrGuid_t*) depv[1].ptr;
    int* PTR_xs_indices = (int*) depv[2].ptr;;
    int* PTR_nucl_index_list = (int*) depv[3].ptr;
    double* PTR_nucl_concs_list = (double*) depv[4].ptr;

    u64 p_energy_u64 = paramv[0];
    double* PTR_p_energy = (double*) &p_energy_u64;
    double p_energy = *PTR_p_energy;

    u64 num_nucs = paramv[1];
    int mat = paramv[2];

    #if 1
    MyOcrTaskStruct_t TS_microxs_1; _paramc = 3; _depc = 4 + num_nucs;
    u64 compute_paramv[3] = {p_energy_u64, num_nucs, mat};

    TS_microxs_1.FNC = FNC_microxs_1;
    ocrEdtTemplateCreate( &TS_microxs_1.TML, TS_microxs_1.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_microxs_1.EDT, TS_microxs_1.TML,
                  EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH, TS_microxs_1.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_uEnergy_grid_i, TS_microxs_1.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_nucl_index_list, TS_microxs_1.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_nucl_concs_list, TS_microxs_1.EDT, _idep++, DB_MODE_RO );
    #endif

    int p_nuc;

    for( int j = 0; j < num_nucs; j++ )
    {
        p_nuc = PTR_nucl_index_list[j];
        ocrAddDependence( PTR_nuclide_grids[p_nuc], TS_microxs_1.EDT, _idep++, DB_MODE_RO );
    }

    return NULL_GUID;
}

ocrGuid_t FNC_microxs_1(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_InputsH = depv[0].guid;
    ocrGuid_t DBK_uEnergy_grid_i =  depv[1].guid;
    ocrGuid_t DBK_nucl_index_list = depv[2].guid;
    ocrGuid_t DBK_nucl_concs_list = depv[3].guid;

    Inputs* PTR_InputsH = depv[0].ptr;
    int* PTR_xs_indices = (int*) depv[1].ptr;;
    int* PTR_nucl_index_list = (int*) depv[2].ptr;
    double* PTR_nucl_concs_list = (double*) depv[3].ptr;

    u64 p_energy_u64 = paramv[0];
    double* PTR_p_energy = (double*) &p_energy_u64;
    double p_energy = *PTR_p_energy;

    u64 num_nucs = paramv[1];
    int mat = paramv[2];

    int n_isotopes = PTR_InputsH->n_isotopes;
    int n_gridpoints = PTR_InputsH->n_gridpoints;

    double xs_vector[5];
    double macro_xs_vector[5];

    int p_nuc;
    double conc;

    _idep = 4;
    for( int j = 0; j < num_nucs; j++ )
    {
        NuclideGridPoint* PTR_nuclide_data_j = depv[_idep+j].ptr;

        p_nuc = PTR_nucl_index_list[j];

        calculate_micro_xs_new( p_energy,
                            n_gridpoints, PTR_xs_indices[p_nuc],
                            PTR_nuclide_data_j, xs_vector );

        //PRINTF("m %d p_nuc %d e %f xs_0 %f xs_1 %f xs_2 %f xs_3 %f xs_4 %f\n", mat, p_nuc, p_energy, xs_vector[0], xs_vector[1], xs_vector[2], xs_vector[3], xs_vector[4]);

        conc = PTR_nucl_concs_list[j];
        for( int k = 0; k < 5; k++ )
            macro_xs_vector[k] += xs_vector[k] * conc;
    }

    return NULL_GUID;
}

ocrGuid_t FNC_globalFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("\n%s\n", __func__);
    s32 _paramc, _depc, _idep;

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    u32 _paramc, _depc, _idep;

    int version = 13;
    int nprocs = 1;

    void* programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI( argc, programArgv );

    // Print-out of Input Summary
    print_inputs( in, nprocs, version );

    ocrGuid_t DBK_InputsH_0;
    Inputs* PTR_Inputs;

    ocrDbCreate( &DBK_InputsH_0, (void **) &PTR_Inputs, sizeof(Inputs),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TS_settingsInit_OET;
    ocrEventCreate( &TS_settingsInit_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_settingsInit; _paramc = 5; _depc = 1;

    u64 settingsInit_paramv[5] = { (u64) in.nthreads, (u64) in.n_isotopes, (u64) in.n_mats, (u64) in.n_gridpoints, (u64) in.lookups };

    TS_settingsInit.FNC = FNC_settingsInit;
    ocrEdtTemplateCreate( &TS_settingsInit.TML, TS_settingsInit.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_settingsInit.EDT, TS_settingsInit.TML,
                  EDT_PARAM_DEF, settingsInit_paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_settingsInit.OET );

    ocrAddDependence( TS_settingsInit.OET, TS_settingsInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_settingsInit.EDT, _idep++, DB_MODE_ITW );

    ocrGuid_t DBK_globalH;
    globalH_t* PTR_globalH;

    ocrDbCreate( &DBK_globalH, (void **) &PTR_globalH, sizeof(globalH_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TS_globalInit_OET, TS_globalCompute_OET, TS_globalFinalize_OET;

    ocrEventCreate( &TS_globalInit_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalCompute_OET, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_globalFinalize_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_globalInit, TS_globalCompute, TS_globalFinalize;
    _paramc = 0; _depc = 2;

    TS_globalInit.FNC = FNC_globalInit;
    ocrEdtTemplateCreate( &TS_globalInit.TML, TS_globalInit.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalInit.EDT, TS_globalInit.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalInit.OET );

    ocrAddDependence( TS_globalInit.OET, TS_globalInit_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_InputsH_0, TS_globalInit.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_globalH, TS_globalInit.EDT, _idep++, DB_MODE_ITW );

    TS_globalCompute.FNC = FNC_globalCompute;
    ocrEdtTemplateCreate( &TS_globalCompute.TML, TS_globalCompute.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalCompute.EDT, TS_globalCompute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalCompute.OET );

    ocrAddDependence( TS_globalCompute.OET, TS_globalCompute_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalCompute.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( TS_globalInit_OET, TS_globalCompute.EDT, _idep++, DB_MODE_NULL);

    TS_globalFinalize.FNC = FNC_globalFinalize;
    ocrEdtTemplateCreate( &TS_globalFinalize.TML, TS_globalFinalize.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_globalFinalize.EDT, TS_globalFinalize.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_globalFinalize.OET );

    ocrAddDependence( TS_globalFinalize.OET, TS_globalFinalize_OET, 0, DB_MODE_NULL);

    _idep = 0;
    ocrAddDependence( DBK_globalH, TS_globalFinalize.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( TS_globalCompute_OET, TS_globalFinalize.EDT, _idep++, DB_MODE_NULL);

    return NULL_GUID;
}
