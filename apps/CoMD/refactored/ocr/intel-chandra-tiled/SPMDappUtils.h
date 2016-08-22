#include "ocr.h"
#include "ocr-std.h"

#include <math.h>

#define _OCR_TASK_FNC_(X) ocrGuid_t X( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )

#define MOD(a,b) ((((a)%(b))+(b))%(b))

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

typedef struct
{
    u64 id;
    u64 edtGridDims[3];
} PRM_initEdt_t;

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e);
void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id);

void splitDimension_Cart3D(s64 Num_procs, s64* Num_procsx, s64* Num_procsy, s64* Num_procsz);
static inline int globalRankFromCoords_Cart3D( int id_x, int id_y, int id_z, int NR_X, int NR_Y, int NR_Z );
void globalCoordsFromRank_Cart3D( int id, int NR_X, int NR_Y, int NR_Z, int* id_x, int* id_y, int* id_z );
static inline int getPolicyDomainID_Cart3D( int b, u64* edtGridDims, u64* pdGridDims );

void forkSpmdEdts_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), s64* edtGridDims, ocrGuid_t* spmdDepv );
void forkSpmdEdts_staticScheduler_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), s64* edtGridDims, ocrGuid_t* spmdDepv );

void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id)
{
    s64 N = ub_g - lb_g + 1;
    s64 s, e;

    s64 r;

    for( r = 0; r < R; r++ )
    {
        s = r*N/R + lb_g;
        e = (r+1)*N/R + lb_g - 1;
        if( s <= i && i <= e )
            break;
    }

    *id = r;
}

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e)
{
    s64 N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;
}

static inline int globalRankFromCoords_Cart2D( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}

void globalCoordsFromRank_Cart3D( int id, int NR_X, int NR_Y, int NR_Z, int* id_x, int* id_y, int* id_z )
{
    *id_x = id % NR_X;
    *id_y = (id / NR_X) % NR_Y;
    *id_z = (id / NR_X) / NR_Y;
}

static inline int globalRankFromCoords_Cart3D( int id_x, int id_y, int id_z, int NR_X, int NR_Y, int NR_Z )
{
    return NR_X*NR_Y*id_z + NR_X*id_y + id_x;
}

static inline int getPoliyDomainID_Cart2D( int b, u64* edtGridDims, u64* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = b/edtGridDims[0];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];

    s64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    s64 pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);

    //Each edt, with id=b, is mapped to a PD. The mapping is similar to how the link cells map to
    //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
    //And, a 3-D subgrid of edts is mapped to a PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords_Cart2D(pd_x, pd_y, PD_X, PD_Y);

    return mapToPD;
}

static inline int getPolicyDomainID_Cart3D( int b, u64* edtGridDims, u64* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = (b/edtGridDims[0])%edtGridDims[1];
    int id_z = (b/edtGridDims[0])/edtGridDims[1];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];
    int PD_Z = pdGridDims[2];

    s64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    s64 pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);
    s64 pd_z; getPartitionID(id_z, 0, edtGridDims[2]-1, PD_Z, &pd_z);

    //Each linkcell, with id=b, is mapped to a PD. The mapping is similar to how the link cells map to
    //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
    //And, a 3-D subgrid of linkcells is mapped to a PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords_Cart3D(pd_x, pd_y, pd_z, PD_X, PD_Y, PD_Z);
    //PRINTF("%d linkCell %d %d %d, policy domain %d: %d %d %d\n", b, id_x, id_y, id_z, pd, PD_X, PD_Y, PD_Z);

    return mapToPD;
}

void splitDimension_Cart2D(s64 Num_procs, s64* Num_procsx, s64* Num_procsy)
{
    s64 nx, ny;

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

void splitDimension_Cart3D(s64 Num_procs, s64* Num_procsx, s64* Num_procsy, s64* Num_procsz)
{
    s64 nx, ny, nz;

    nz = (int) pow(Num_procs+1,0.33);
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
        if (!(Num_procs%nz))
        {
            nx = Num_procs/ny;
            break;
        }
    }

    *Num_procsy = ny;

    *Num_procsx = Num_procs/(*Num_procsy);
}

void forkSpmdEdts_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), s64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nRanks = edtGridDims[0]*edtGridDims[1]*edtGridDims[2];

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 2-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, sizeof(PRM_initEdt_t)/sizeof(u64), 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    PRM_initEdt_t PRM_initEdt;
    PRM_initEdt.edtGridDims[0] = edtGridDims[0];
    PRM_initEdt.edtGridDims[1] = edtGridDims[1];
    PRM_initEdt.edtGridDims[2] = edtGridDims[2];

    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    s64 PD_X, PD_Y, PD_Z;
    splitDimension_Cart3D( affinityCount, &PD_X, &PD_Y, &PD_Z ); //Split available PDs into a 2-D grid

    s64 pdGridDims[3] = { PD_X, PD_Y, PD_Z };

    for( i = 0; i < nRanks; ++i )
    {
        PRM_initEdt.id = i;

        int pd = getPolicyDomainID_Cart3D( i, edtGridDims, pdGridDims );
        PRINTF("id %d map PD %d\n", i, pd);
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );

        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_initEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( spmdDepv[0], EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_init, 1, DB_MODE_RO );
    }
}

#ifdef USE_STATIC_SCHEDULER

typedef struct
{
    u64 PD_id;
    s64 edtGridDims[3];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*);
} PDinitEdt_paramv_t;

_OCR_TASK_FNC_( PDinitEdt )
{
    PDinitEdt_paramv_t* PTR_PDinitEdt_paramv = (PDinitEdt_paramv_t*) paramv;

    u64 PD_id;
    s64 edtGridDims[3];
    //
    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    s64 PD_X, PD_Y, PD_Z;
    splitDimension_Cart3D( affinityCount, &PD_X, &PD_Y, &PD_Z ); //Split available PDs into a 2-D grid

    s64 pdGridDims[3] = { PD_X, PD_Y, PD_Z };

    PD_id = PTR_PDinitEdt_paramv->PD_id;
    edtGridDims[0] = PTR_PDinitEdt_paramv->edtGridDims[0];
    edtGridDims[1] = PTR_PDinitEdt_paramv->edtGridDims[1];
    edtGridDims[2] = PTR_PDinitEdt_paramv->edtGridDims[2];
    ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*) = PTR_PDinitEdt_paramv->initEdt;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    void *PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t *PTR_globalParamH = (globalParamH_t *) depv[1].ptr;

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, 1, 2 );

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    s64 edtGridDims_lb_x, edtGridDims_ub_x;
    s64 edtGridDims_lb_y, edtGridDims_ub_y;
    s64 edtGridDims_lb_z, edtGridDims_ub_z;

    partition_bounds(PD_X, 0, edtGridDims[0]-1, pdGridDims[0], &edtGridDims_lb_x, &edtGridDims_ub_x);
    partition_bounds(PD_Y, 0, edtGridDims[1]-1, pdGridDims[1], &edtGridDims_lb_y, &edtGridDims_ub_y);
    partition_bounds(PD_Z, 0, edtGridDims[2]-1, pdGridDims[2], &edtGridDims_lb_z, &edtGridDims_ub_z);

    PRM_initEdt_t PRM_initEdt;
    PRM_initEdt.edtGridDims[0] = edtGridDims[0];
    PRM_initEdt.edtGridDims[1] = edtGridDims[1];
    PRM_initEdt.edtGridDims[2] = edtGridDims[2];

    s64 i, j, k;
    for( k = edtGridDims_lb_z; j <= edtGridDims_ub_z ; ++k )
    for( j = edtGridDims_lb_y; j <= edtGridDims_ub_y ; ++j )
    for( i = edtGridDims_lb_x; i <= edtGridDims_ub_x ; ++i )
    {
        u64 myRank = globalRankFromCoords_Cart3D( i, j, k, edtGridDims[0], edtGridDims[1], edtGridDims[2] );
        //PRINTF("id %d map PD %d\n", myRank, PD_id);
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_DISPERSE,  OCR_HINT_EDT_DISPERSE_NEAR );

        PRM_initEdt.id = myRank;
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&PRM_initEdt, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //initEdt
        ocrAddDependence( DBK_cmdLineArgs, EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( DBK_globalParamH, EDT_init, 1, DB_MODE_RO );
    }
}

void forkSpmdEdts_staticScheduler_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), s64* edtGridDims, ocrGuid_t* spmdDepv )
{
    u64 i;
    u64 nPDs;
    ocrAffinityCount( AFFINITY_PD, &nPDs );

    ocrGuid_t DBK_cmdLineArgs = spmdDepv[0];
    ocrGuid_t DBK_globalParamH = spmdDepv[1];

    //The EDTs are mapped to the policy domains (nodes) through EDT hints
    //There are more subdomains than the # of policy domains.
    //A 2-d subgrid of "subdomains"/ranks/EDTs get mapped to a single policy domain
    //to minimize data movement between the policy domains

    ocrGuid_t PDinitTML, EDT_PDinit;
    PDinitEdt_paramv_t PDinitEdt_paramv;
    ocrEdtTemplateCreate( &PDinitTML, PDinitEdt, sizeof(PDinitEdt_paramv_t)/sizeof(u64), 2 );

    PDinitEdt_paramv.initEdt = initEdt;

    PDinitEdt_paramv.edtGridDims[0] = edtGridDims[0];
    PDinitEdt_paramv.edtGridDims[1] = edtGridDims[1];
    PDinitEdt_paramv.edtGridDims[2] = edtGridDims[2];

    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );

    ocrGuid_t PDaffinityGuid;

    for( i = 0; i < nPDs; ++i )
    {
        int pd = i;
        //PRINTF("id %d map PD %d\n", i, pd);
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );

        PDinitEdt_paramv.PD_id = pd;

        ocrEdtCreate( &EDT_PDinit, PDinitTML, EDT_PARAM_DEF, (u64*)&PDinitEdt_paramv, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //PDinitEdt
        ocrAddDependence( spmdDepv[0], EDT_PDinit, 0, DB_MODE_RO );
        ocrAddDependence( spmdDepv[1], EDT_PDinit, 1, DB_MODE_RO );
    }
}
#endif  //USE_STATIC_SCHEDULER
