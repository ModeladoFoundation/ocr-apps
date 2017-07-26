#include <math.h>

#include "ocrAppUtils.h"
#include "extensions/ocr-affinity.h" //needed for affinity

#ifdef USE_STATIC_SCHEDULER
typedef struct
{
    u64 PD_id;
    u64 edtGridDims[1];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*);
} PDinit1dEdt_paramv_t;

typedef struct
{
    u64 PD_id;
    u64 edtGridDims[2];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*);
} PDinit2dEdt_paramv_t;

typedef struct
{
    u64 PD_id;
    u64 edtGridDims[3];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*);
} PDinit3dEdt_paramv_t;
#endif

void createEventHelper(ocrGuid_t * evtGuid, u32 nbDeps)
{
    ocrEventParams_t params;
    params.EVENT_COUNTED.nbDeps = nbDeps;
    ocrEventCreateParams(evtGuid, OCR_EVENT_COUNTED_T, false, &params);
}

void getAffinityHintsForDBandEdt( ocrHint_t* PTR_myDbkAffinityHNT, ocrHint_t* PTR_myEdtAffinityHNT )
{
    ocrGuid_t currentAffinity = NULL_GUID;

    ocrHintInit( PTR_myEdtAffinityHNT, OCR_HINT_EDT_T );
    ocrHintInit( PTR_myDbkAffinityHNT, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( PTR_myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( PTR_myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
#endif

}

void getPartitionID(u64 i, u64 lb_g, u64 ub_g, u64 R, u64* id)
{
    u64 N = ub_g - lb_g + 1;
    u64 s, e;

    u64 r;

    for( r = 0; r < R; r++ )
    {
        s = r*N/R + lb_g;
        e = (r+1)*N/R + lb_g - 1;
        if( s <= i && i <= e )
            break;
    }

    *id = r;
}

void partition_bounds(u64 id, u64 lb_g, u64 ub_g, u64 R, u64* s, u64* e)
{
    u64 N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;
}

int globalRankFromCoords_Cart1D( int id_x, int NR_X )
{
    return id_x;
}

int globalRankFromCoords_Cart2D( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}

int globalRankFromCoords_Cart3D( int id_x, int id_y, int id_z, int NR_X, int NR_Y, int NR_Z )
{
    return NR_X*NR_Y*id_z + NR_X*id_y + id_x;
}

void globalCoordsFromRank_Cart1D( int id, int NR_X, int* id_x )
{
    *id_x = id % NR_X;
}

void globalCoordsFromRank_Cart2D( int id, int NR_X, int NR_Y, int* id_x, int* id_y )
{
    *id_x = id % NR_X;
    *id_y = id / NR_X;
}

void globalCoordsFromRank_Cart3D( int id, int NR_X, int NR_Y, int NR_Z, int* id_x, int* id_y, int* id_z )
{
    *id_x = id % NR_X;
    *id_y = (id / NR_X) % NR_Y;
    *id_z = (id / NR_X) / NR_Y;
}

int getPolicyDomainID_Cart1D( int b, u64* edtGridDims, u64* pdGridDims )
{
    int id_x = b%edtGridDims[0];

    int PD_X = pdGridDims[0];

    u64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);

    //Each edt, with id=b, is mapped to a PD.
    //
    int mapToPD = globalRankFromCoords_Cart1D(pd_x, PD_X);

    return mapToPD;
}

int getPolicyDomainID_Cart2D( int b, u64* edtGridDims, u64* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = b/edtGridDims[0];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];

    u64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    u64 pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);

    //Each edt, with id=b, is mapped to a PD.
    //All the PDs are arranged as a 2-D grid. The 2-D EDT grid is mapped to the PDs.
    //The mapping strategy involes maping a 2-D subgrid of edts to a single PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords_Cart2D(pd_x, pd_y, PD_X, PD_Y);

    return mapToPD;
}

int getPolicyDomainID_Cart3D( int b, u64* edtGridDims, u64* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = (b/edtGridDims[0])%edtGridDims[1];
    int id_z = (b/edtGridDims[0])/edtGridDims[1];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];
    int PD_Z = pdGridDims[2];

    u64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    u64 pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);
    u64 pd_z; getPartitionID(id_z, 0, edtGridDims[2]-1, PD_Z, &pd_z);

    //Each edt, with id=b, is mapped to a PD.
    //All the PDs are arranged as a 3-D grid. The 3-D EDT grid is mapped to the PDs.
    //The mapping strategy involes maping a 3-D subgrid of edts to a single PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords_Cart3D(pd_x, pd_y, pd_z, PD_X, PD_Y, PD_Z);

    return mapToPD;
}

void splitDimension_Cart2D(u64 Num_procs, u64* Num_procsx, u64* Num_procsy)
{
    u64 nx, ny;

    nx = (int) sqrt(Num_procs+1);
    for(; nx>0; nx--)
    {
        if (!(Num_procs%nx))
        {
            ny = Num_procs/nx;
            break;
        }
    }
    *Num_procsx = nx; *Num_procsy = (Num_procs)/(*Num_procsx);
}

void splitDimension_Cart3D(u64 Num_procs, u64* Num_procsx, u64* Num_procsy, u64* Num_procsz)
{
    u64 nx, ny, nz;

    nz = (int) pow(Num_procs+1,1.0/3.0);
    for(; nz>0; nz--)
    {
        if (!(Num_procs%nz))
        {
            ny = Num_procs/nz;
            break;
        }
    }
    *Num_procsz = nz;

    Num_procs = Num_procs/nz;

    ny = (int) sqrt(Num_procs+1);
    for(; ny>0; ny--)
    {
        if (!(Num_procs%ny))
        {
            nx = Num_procs/ny;
            break;
        }
    }

    *Num_procsy = ny;

    *Num_procsx = Num_procs/(*Num_procsy);
}

void forkSpmdEdts_Cart1D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nRanks = edtGridDims[0];

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 1-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init1dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    PRM_init1dEdt_t PRM_init1dEdt;
    PRM_init1dEdt.edtGridDims[0] = edtGridDims[0];

    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    PRINTF("Using affinity API: Count %"PRIu64"\n", affinityCount);
#else
    PRINTF("NOT Using affinity API\n");
#endif
    u64 PD_X = affinityCount;

    u64 pdGridDims[1] = { PD_X };

    for( i = 0; i < nRanks; ++i )
    {
        PRM_init1dEdt.id = i;

        int pd = getPolicyDomainID_Cart1D( i, edtGridDims, pdGridDims );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_init1dEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( spmdDepv[0], EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_init, 1, DB_MODE_RO );
    }
}

void forkSpmdEdts_Cart2D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nRanks = edtGridDims[0]*edtGridDims[1];

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 2-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init2dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    PRM_init2dEdt_t PRM_init2dEdt;
    PRM_init2dEdt.edtGridDims[0] = edtGridDims[0];
    PRM_init2dEdt.edtGridDims[1] = edtGridDims[1];

    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    PRINTF("Using affinity API: Count %"PRIu64"\n", affinityCount);
#else
    PRINTF("NOT Using affinity API\n");
#endif
    u64 PD_X, PD_Y;
    splitDimension_Cart2D( affinityCount, &PD_X, &PD_Y ); //Split available PDs into a 2-D grid

    u64 pdGridDims[2] = { PD_X, PD_Y };

    for( i = 0; i < nRanks; ++i )
    {
        PRM_init2dEdt.id = i;

        int pd = getPolicyDomainID_Cart2D( i, edtGridDims, pdGridDims );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_init2dEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( spmdDepv[0], EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_init, 1, DB_MODE_RO );
    }
}

void forkSpmdEdts_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nRanks = edtGridDims[0]*edtGridDims[1]*edtGridDims[2];

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 3-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init3dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    PRM_init3dEdt_t PRM_init3dEdt;
    PRM_init3dEdt.edtGridDims[0] = edtGridDims[0];
    PRM_init3dEdt.edtGridDims[1] = edtGridDims[1];
    PRM_init3dEdt.edtGridDims[2] = edtGridDims[2];

    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    PRINTF("Using affinity API: Count %"PRIu64"\n", affinityCount);
#else
    PRINTF("NOT Using affinity API\n");
#endif
    u64 PD_X, PD_Y, PD_Z;
    splitDimension_Cart3D( affinityCount, &PD_X, &PD_Y, &PD_Z ); //Split available PDs into a 3-D grid
    DEBUG_PRINTF(("PD_X PD_Y PD_Z %d %d %d\n", PD_X, PD_Y, PD_Z));

    u64 pdGridDims[3] = { PD_X, PD_Y, PD_Z };

    for( i = 0; i < nRanks; ++i )
    {
        PRM_init3dEdt.id = i;

        int pd = getPolicyDomainID_Cart3D( i, edtGridDims, pdGridDims );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_init3dEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( spmdDepv[0], EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_init, 1, DB_MODE_RO );
    }
}

#ifdef USE_STATIC_SCHEDULER

_OCR_TASK_FNC_( PDinit1dEdt )
{
    PDinit1dEdt_paramv_t* PTR_PDinit1dEdt_paramv = (PDinit1dEdt_paramv_t*) paramv;

    u64 PD_id;
    u64 edtGridDims[1];
    //
    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
#endif
    u64 PD_X = affinityCount;
    u64 pdGridDims[1] = { PD_X };

    PD_id = PTR_PDinit1dEdt_paramv->PD_id;
    edtGridDims[0] = PTR_PDinit1dEdt_paramv->edtGridDims[0];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*) = PTR_PDinit1dEdt_paramv->initEdt;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init1dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    u64 edtGridDims_lb_x, edtGridDims_ub_x;

    int pd_x;

    globalCoordsFromRank_Cart1D( PD_id, pdGridDims[0], &pd_x );
    DEBUG_PRINTF(("PD_id %d pdGridDims[0] %d pd_x %d\n",PD_id, pdGridDims[0], pd_x ));

    partition_bounds(pd_x, 0, edtGridDims[0]-1, pdGridDims[0], &edtGridDims_lb_x, &edtGridDims_ub_x);

    PRM_init1dEdt_t PRM_initEdt;
    PRM_initEdt.edtGridDims[0] = edtGridDims[0];

    u64 i;
    for( i = edtGridDims_lb_x; i <= edtGridDims_ub_x ; ++i )
    {
        u64 myRank = globalRankFromCoords_Cart1D( i, edtGridDims[0] );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", myRank, PD_id));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_DISPERSE,  OCR_HINT_EDT_DISPERSE_NEAR );
#endif
        PRM_initEdt.id = myRank;
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_initEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( DBK_cmdLineArgs, EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( DBK_globalParamH, EDT_init, 1, DB_MODE_RO );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( PDinit2dEdt )
{
    PDinit2dEdt_paramv_t* PTR_PDinit2dEdt_paramv = (PDinit2dEdt_paramv_t*) paramv;

    u64 PD_id;
    u64 edtGridDims[2];
    //
    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
#endif
    u64 PD_X, PD_Y;
    splitDimension_Cart2D( affinityCount, &PD_X, &PD_Y ); //Split available PDs into a 2-D grid

    u64 pdGridDims[2] = { PD_X, PD_Y };

    PD_id = PTR_PDinit2dEdt_paramv->PD_id;
    edtGridDims[0] = PTR_PDinit2dEdt_paramv->edtGridDims[0];
    edtGridDims[1] = PTR_PDinit2dEdt_paramv->edtGridDims[1];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*) = PTR_PDinit2dEdt_paramv->initEdt;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init2dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    u64 edtGridDims_lb_x, edtGridDims_ub_x;
    u64 edtGridDims_lb_y, edtGridDims_ub_y;

    int pd_x, pd_y;

    globalCoordsFromRank_Cart2D( PD_id, pdGridDims[0], pdGridDims[1], &pd_x, &pd_y );
    DEBUG_PRINTF(("PD_id %d pdGridDims[0] %d pdGridDims[1] %d pd_x %d pd_y %d\n",PD_id, pdGridDims[0], pdGridDims[1], pd_x, pd_y ));

    partition_bounds(pd_x, 0, edtGridDims[0]-1, pdGridDims[0], &edtGridDims_lb_x, &edtGridDims_ub_x);
    partition_bounds(pd_y, 0, edtGridDims[1]-1, pdGridDims[1], &edtGridDims_lb_y, &edtGridDims_ub_y);

    PRM_init2dEdt_t PRM_init2dEdt;
    PRM_init2dEdt.edtGridDims[0] = edtGridDims[0];
    PRM_init2dEdt.edtGridDims[1] = edtGridDims[1];

    u64 i, j;
    for( j = edtGridDims_lb_y; j <= edtGridDims_ub_y ; ++j )
    for( i = edtGridDims_lb_x; i <= edtGridDims_ub_x ; ++i )
    {
        u64 myRank = globalRankFromCoords_Cart2D( i, j, edtGridDims[0], edtGridDims[1] );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", myRank, PD_id));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_DISPERSE,  OCR_HINT_EDT_DISPERSE_NEAR );
#endif
        PRM_init2dEdt.id = myRank;
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_init2dEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( DBK_cmdLineArgs, EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( DBK_globalParamH, EDT_init, 1, DB_MODE_RO );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( PDinit3dEdt )
{
    PDinit3dEdt_paramv_t* PTR_PDinit3dEdt_paramv = (PDinit3dEdt_paramv_t*) paramv;

    u64 PD_id;
    u64 edtGridDims[3];
    //
    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
#endif
    u64 PD_X, PD_Y, PD_Z;
    splitDimension_Cart3D( affinityCount, &PD_X, &PD_Y, &PD_Z ); //Split available PDs into a 3-D grid

    u64 pdGridDims[3] = { PD_X, PD_Y, PD_Z };

    PD_id = PTR_PDinit3dEdt_paramv->PD_id;
    edtGridDims[0] = PTR_PDinit3dEdt_paramv->edtGridDims[0];
    edtGridDims[1] = PTR_PDinit3dEdt_paramv->edtGridDims[1];
    edtGridDims[2] = PTR_PDinit3dEdt_paramv->edtGridDims[2];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*) = PTR_PDinit3dEdt_paramv->initEdt;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_init3dEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    u64 edtGridDims_lb_x, edtGridDims_ub_x;
    u64 edtGridDims_lb_y, edtGridDims_ub_y;
    u64 edtGridDims_lb_z, edtGridDims_ub_z;

    int pd_x, pd_y, pd_z;

    globalCoordsFromRank_Cart3D( PD_id, pdGridDims[0], pdGridDims[1], pdGridDims[2], &pd_x, &pd_y, &pd_z );
    DEBUG_PRINTF(("PD_id %d pdGridDims[0] %d pdGridDims[1] %d pdGridDims[2] %d pd_x %d pd_y %d pd_z %d\n",PD_id, pdGridDims[0], pdGridDims[1], pdGridDims[2], pd_x, pd_y, pd_z ));

    partition_bounds(pd_x, 0, edtGridDims[0]-1, pdGridDims[0], &edtGridDims_lb_x, &edtGridDims_ub_x);
    partition_bounds(pd_y, 0, edtGridDims[1]-1, pdGridDims[1], &edtGridDims_lb_y, &edtGridDims_ub_y);
    partition_bounds(pd_z, 0, edtGridDims[2]-1, pdGridDims[2], &edtGridDims_lb_z, &edtGridDims_ub_z);

    PRM_init3dEdt_t PRM_init3dEdt;
    PRM_init3dEdt.edtGridDims[0] = edtGridDims[0];
    PRM_init3dEdt.edtGridDims[1] = edtGridDims[1];
    PRM_init3dEdt.edtGridDims[2] = edtGridDims[2];

    u64 i, j, k;
    for( k = edtGridDims_lb_z; k <= edtGridDims_ub_z ; ++k )
    for( j = edtGridDims_lb_y; j <= edtGridDims_ub_y ; ++j )
    for( i = edtGridDims_lb_x; i <= edtGridDims_ub_x ; ++i )
    {
        u64 myRank = globalRankFromCoords_Cart3D( i, j, k, edtGridDims[0], edtGridDims[1], edtGridDims[2] );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", myRank, PD_id));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_DISPERSE,  OCR_HINT_EDT_DISPERSE_NEAR );
#endif
        PRM_init3dEdt.id = myRank;
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_init3dEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( DBK_cmdLineArgs, EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( DBK_globalParamH, EDT_init, 1, DB_MODE_RO );
    }

    return NULL_GUID;
}

void forkSpmdEdts_staticScheduler_Cart1D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nPDs=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &nPDs );
#endif

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 1-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t PDinit1dTML, EDT_PDinit1d;
    PDinit1dEdt_paramv_t PDinit1dEdt_paramv;
    ocrEdtTemplateCreate( &PDinit1dTML, PDinit1dEdt, sizeof(PDinit1dEdt_paramv_t)/sizeof(u64), 2 );

    PDinit1dEdt_paramv.initEdt = initEdt;

    PDinit1dEdt_paramv.edtGridDims[0] = edtGridDims[0];

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    for( i = 0; i < nPDs; ++i )
    {
        int pd = i;
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        PDinit1dEdt_paramv.PD_id = pd;

        ocrEdtCreate( &EDT_PDinit1d, PDinit1dTML, EDT_PARAM_DEF, (u64*)&PDinit1dEdt_paramv, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //PDinit1dEdt
        ocrAddDependence( spmdDepv[0], EDT_PDinit1d, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_PDinit1d, 1, DB_MODE_RO );
    }
}

void forkSpmdEdts_staticScheduler_Cart2D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nPDs=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &nPDs );
#endif

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 2-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t PDinit2dTML, EDT_PDinit2d;
    PDinit2dEdt_paramv_t PDinit2dEdt_paramv;
    ocrEdtTemplateCreate( &PDinit2dTML, PDinit2dEdt, sizeof(PDinit2dEdt_paramv_t)/sizeof(u64), 2 );

    PDinit2dEdt_paramv.initEdt = initEdt;

    PDinit2dEdt_paramv.edtGridDims[0] = edtGridDims[0];
    PDinit2dEdt_paramv.edtGridDims[1] = edtGridDims[1];

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    for( i = 0; i < nPDs; ++i )
    {
        int pd = i;
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        PDinit2dEdt_paramv.PD_id = pd;

        ocrEdtCreate( &EDT_PDinit2d, PDinit2dTML, EDT_PARAM_DEF, (u64*)&PDinit2dEdt_paramv, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //PDinit2dEdt
        ocrAddDependence( spmdDepv[0], EDT_PDinit2d, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_PDinit2d, 1, DB_MODE_RO );
    }
}

void forkSpmdEdts_staticScheduler_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nPDs=1;
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityCount( AFFINITY_PD, &nPDs );
#endif

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 3-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t PDinit3dTML, EDT_PDinit3d;
    PDinit3dEdt_paramv_t PDinit3dEdt_paramv;
    ocrEdtTemplateCreate( &PDinit3dTML, PDinit3dEdt, sizeof(PDinit3dEdt_paramv_t)/sizeof(u64), 2 );

    PDinit3dEdt_paramv.initEdt = initEdt;

    PDinit3dEdt_paramv.edtGridDims[0] = edtGridDims[0];
    PDinit3dEdt_paramv.edtGridDims[1] = edtGridDims[1];
    PDinit3dEdt_paramv.edtGridDims[2] = edtGridDims[2];

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    for( i = 0; i < nPDs; ++i )
    {
        int pd = i;
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        PDinit3dEdt_paramv.PD_id = pd;

        ocrEdtCreate( &EDT_PDinit3d, PDinit3dTML, EDT_PARAM_DEF, (u64*)&PDinit3dEdt_paramv, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //PDinit3dEdt
        ocrAddDependence( spmdDepv[0], EDT_PDinit3d, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_PDinit3d, 1, DB_MODE_RO );
    }
}
#endif  //USE_STATIC_SCHEDULER
