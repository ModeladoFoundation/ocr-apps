#include "ocr.h"
#include "ocr-std.h"

#include "stdlib.h"

#define SUBDOMAIN_SIZE 1
#define NSUBDOMAINS 4
#define HALO_RADIUS 1
#define NTIMESTEPS 10
#define NTIMESTEPS_SYNC 10

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
    ocrEdt_t FUNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t EVT;
} MyOcrTaskStruct_t;

typedef struct
{
    s32 lb, ub, N, id, itimestep;
    ocrGuid_t DB_xIn, DB_xOut;

    ocrGuid_t EVNT_lsend_finish[2], EVNT_rsend_finish[2], EVNT_lrecv_finish[2], EVNT_rrecv_finish[2];
    ocrGuid_t EVNT_lsend_ack[2], EVNT_rsend_ack[2];
} memberHandle_t;

typedef struct
{
    s64 count;
    ocrGuid_t member[]; //each member is a guid containing a handle of memberHandle_t
} globalHandle_t;

ocrGuid_t FUNC_mainInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FUNC_initSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FUNC_init(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FUNC_mainCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
    ocrGuid_t FUNC_mainComputeManager(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
        ocrGuid_t FUNC_computeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
            ocrGuid_t FUNC_compute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                ocrGuid_t FUNC_iterate(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
                    ocrGuid_t FUNC_resetEvents(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FUNC_mainFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FUNC_mainInit(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    u64 nsubdomains                 = paramv[0];

    ocrGuid_t DB_globalHandle       = depv[0].guid;
    globalHandle_t *CP_globalHandle = depv[0].ptr;

    CP_globalHandle->count          = (s64) nsubdomains;

    u64 initSpawner_paramv[1]  = { nsubdomains };

    MyOcrTaskStruct_t TS_initSpawner; _paramc = 1; _depc = 1;

    TS_initSpawner.FUNC = FUNC_initSpawner;
    ocrEdtTemplateCreate( &TS_initSpawner.TML, TS_initSpawner.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_initSpawner.EDT, TS_initSpawner.TML,
                          EDT_PARAM_DEF, initSpawner_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_FINISH, NULL_GUID, NULL );

    _idep = 0;
    ocrAddDependence(     DB_globalHandle, TS_initSpawner.EDT, _idep++, DB_MODE_ITW ); //Fires up individual EDTs to allocated subdomain DBs, EDTs

    return NULL_GUID;
}

ocrGuid_t FUNC_initSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    u32 nsubdomains                 = (u32) paramv[0];

    globalHandle_t *CP_globalHandle = depv[0].ptr;

    //spawn N intializer EDTs
    PRINTF("#Subdomains %d\n", NSUBDOMAINS);

    MyOcrTaskStruct_t TS_init; _paramc = 1; _depc = 1;

    u64 init_paramv[1];

    TS_init.FUNC = FUNC_init;
    ocrEdtTemplateCreate( &TS_init.TML, TS_init.FUNC, _paramc, _depc );

    s64 i;
    for( i = 0; i < nsubdomains; i++ )
    {
        memberHandle_t *CP_memberHandle;
        ocrDbCreate(  &(CP_globalHandle->member[i]), (void **) &CP_memberHandle, sizeof(memberHandle_t),
                      DB_PROP_NONE, NULL_GUID, NO_ALLOC );

        init_paramv[0] = (u64) i;
        ocrEdtCreate(     &TS_init.EDT, TS_init.TML,
                          EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( CP_globalHandle->member[i], TS_init.EDT, _idep++, DB_MODE_ITW );
    }

    return NULL_GUID;
}

ocrGuid_t FUNC_init(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);

    s32 subdomainID                 = (s32) paramv[0];

    ocrGuid_t DB_memberHandle       = (ocrGuid_t) depv[0].guid;

    memberHandle_t *CP_memberHandle = (memberHandle_t*) depv[0].ptr;

    double *xIn, *xOut;
    ocrDbCreate(    &(CP_memberHandle->DB_xIn), (void **) &xIn, sizeof(double)*(SUBDOMAIN_SIZE+2*HALO_RADIUS),
                    DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbCreate(    &(CP_memberHandle->DB_xOut), (void **) &xOut, sizeof(double)*(SUBDOMAIN_SIZE),
                    DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    CP_memberHandle->N = SUBDOMAIN_SIZE*NSUBDOMAINS;
    CP_memberHandle->id = subdomainID;
    CP_memberHandle->lb = subdomainID*SUBDOMAIN_SIZE;
    CP_memberHandle->ub = (subdomainID+1)*SUBDOMAIN_SIZE-1;

    CP_memberHandle->itimestep = 0;

    s64 i;

    for( i = 0; i < SUBDOMAIN_SIZE; i++ )
    {
        xIn[i+HALO_RADIUS] = (double) ( CP_memberHandle->lb + i );
        xOut[i] = 0.;
    }

    for( i = 0; i < HALO_RADIUS; i++ )
    {
        xIn[i] = -111.;
        xIn[SUBDOMAIN_SIZE+HALO_RADIUS+i] = -111.;
    }

    if( subdomainID == 0)
    {
        for( i = -HALO_RADIUS; i < 0; i++ )
            xIn[i+HALO_RADIUS] = (double) ( CP_memberHandle->lb + i );
    }

    if( subdomainID == NSUBDOMAINS-1)
    {
        for( i = SUBDOMAIN_SIZE; i < SUBDOMAIN_SIZE+HALO_RADIUS; i++ )
            xIn[i+HALO_RADIUS] = (double) ( CP_memberHandle->lb + i );
    }

    for( i = 0; i < SUBDOMAIN_SIZE+2*HALO_RADIUS; i++ )
        PRINTF("ID: %d xIn[%d]=%f\n", subdomainID, i, xIn[i]);

    for( i = 0; i < 2; i++ )
    {
        ocrEventCreate( &CP_memberHandle->EVNT_lrecv_finish[i], OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &CP_memberHandle->EVNT_rrecv_finish[i], OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &CP_memberHandle->EVNT_lsend_finish[i], OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &CP_memberHandle->EVNT_rsend_finish[i], OCR_EVENT_STICKY_T, true );
        ocrEventCreate( &CP_memberHandle->EVNT_lsend_ack[i], OCR_EVENT_STICKY_T, false );
        ocrEventCreate( &CP_memberHandle->EVNT_rsend_ack[i], OCR_EVENT_STICKY_T, false );
    }

    ocrDbRelease( DB_memberHandle );

    return NULL_GUID;
}

ocrGuid_t FUNC_mainCompute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    u64 nsubdomains     = NSUBDOMAINS;
    u64 ntimesteps      = NTIMESTEPS;
    u64 ntimesteps_sync = NTIMESTEPS_SYNC;

    u64 itimestep   = 1;

    ocrGuid_t DB_globalHandle       = depv[0].guid;

    globalHandle_t *CP_globalHandle = depv[0].ptr;

    MyOcrTaskStruct_t TS_mainComputeManager; _paramc = 4; _depc = 1;

    u64 computeSpawner_paramv[4] = { nsubdomains, ntimesteps, ntimesteps_sync, itimestep };

    TS_mainComputeManager.FUNC = FUNC_mainComputeManager;
    ocrEdtTemplateCreate(   &TS_mainComputeManager.TML, TS_mainComputeManager.FUNC, _paramc, _depc );

    ocrEdtCreate(           &TS_mainComputeManager.EDT, TS_mainComputeManager.TML,
                            EDT_PARAM_DEF, computeSpawner_paramv, EDT_PARAM_DEF, NULL,
                            EDT_PROP_FINISH, NULL_GUID, &TS_mainComputeManager.EVT );

    _idep = 0;
    ocrAddDependence(       DB_globalHandle, TS_mainComputeManager.EDT, _idep++, DB_MODE_ITW );

    return NULL_GUID;
}

ocrGuid_t FUNC_mainComputeManager(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 _paramc, _depc, _idep;

    u64 nsubdomains = paramv[0];
    u64 ntimesteps  = paramv[1];
    u64 ntimesteps_sync = paramv[2];
    u64 itimestep   = paramv[3];

    PRINTF("%s Timestep %d\n", __func__, itimestep);

    if( itimestep <= NTIMESTEPS )
    {
        ocrGuid_t DB_globalHandle       = depv[0].guid;

        globalHandle_t *CP_globalHandle = depv[0].ptr;

        ocrGuid_t TS_computeSpawner_EVT;
        MyOcrTaskStruct_t TS_computeSpawner; _paramc = 4; _depc = 1;

        ocrEventCreate(         &TS_computeSpawner_EVT, OCR_EVENT_STICKY_T, false );

        TS_computeSpawner.FUNC = FUNC_computeSpawner;
        ocrEdtTemplateCreate(   &TS_computeSpawner.TML, TS_computeSpawner.FUNC, _paramc, _depc );

        ocrEdtCreate(           &TS_computeSpawner.EDT, TS_computeSpawner.TML,
                                EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                                EDT_PROP_FINISH, NULL_GUID, &TS_computeSpawner.EVT );

        ocrAddDependence(       TS_computeSpawner.EVT, TS_computeSpawner_EVT, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence(       DB_globalHandle, TS_computeSpawner.EDT, _idep++, DB_MODE_ITW );

        itimestep += NTIMESTEPS_SYNC;

        if( itimestep <= NTIMESTEPS )
        {
            MyOcrTaskStruct_t TS_mainComputeManager; _paramc = 4; _depc = 2;
            u64 mainComputeManager_paramv[4] = { nsubdomains, ntimesteps, ntimesteps_sync, itimestep };

            TS_mainComputeManager.FUNC = FUNC_mainComputeManager;
            ocrEdtTemplateCreate(   &TS_mainComputeManager.TML, TS_mainComputeManager.FUNC, _paramc, _depc );

            ocrEdtCreate(           &TS_mainComputeManager.EDT, TS_mainComputeManager.TML,
                                    EDT_PARAM_DEF, mainComputeManager_paramv, EDT_PARAM_DEF, NULL,
                                    EDT_PROP_FINISH, NULL_GUID, &TS_mainComputeManager.EVT );

            _idep = 0;
            ocrAddDependence(       DB_globalHandle, TS_mainComputeManager.EDT, _idep++, DB_MODE_ITW );
            ocrAddDependence(       TS_computeSpawner_EVT, TS_mainComputeManager.EDT, _idep++, DB_MODE_NULL );
        }
    }

    return NULL_GUID;
}

ocrGuid_t FUNC_computeSpawner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    u64 nsubdomains = paramv[0];
    u64 ntimesteps  = paramv[1];
    u64 ntimesteps_sync = paramv[2];
    u64 itimestep   = paramv[3];

    globalHandle_t *CP_globalHandle = depv[0].ptr;

    u64 i;

    PRINTF("#Subdomains %d\n", nsubdomains);

    MyOcrTaskStruct_t TS_compute; _paramc = 3; _depc = 3;

    u64 compute_paramv[3];

    TS_compute.FUNC = FUNC_compute;
    ocrEdtTemplateCreate( &TS_compute.TML, TS_compute.FUNC, _paramc, _depc );

    PRINTF("Timestep # %3d\n", itimestep);
    for( i = 0; i < nsubdomains; i++ )
    {
        compute_paramv[0] = (u64) i;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) MIN( itimestep + ntimesteps_sync - 1, ntimesteps );

        ocrEdtCreate( &TS_compute.EDT, TS_compute.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( CP_globalHandle->member[i], TS_compute.EDT, _idep++, DB_MODE_ITW );
        ocrAddDependence( (i!=0) ? CP_globalHandle->member[i-1] : NULL_GUID, TS_compute.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (i!=nsubdomains-1) ? CP_globalHandle->member[i+1] : NULL_GUID, TS_compute.EDT, _idep++, DB_MODE_RO );
    }

    return NULL_GUID;
}

ocrGuid_t FUNC_compute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];
    s32 ntimestep_l = (s32) paramv[2];

    memberHandle_t* CP_memberHandle     = (memberHandle_t*) depv[0].ptr;
    memberHandle_t* CP_memberHandle_l   = (memberHandle_t*) depv[1].ptr;
    memberHandle_t* CP_memberHandle_r   = (memberHandle_t*) depv[2].ptr;

    MyOcrTaskStruct_t TS_iterate; _paramc = 3; _depc = 3;

    TS_iterate.FUNC = FUNC_iterate;
    ocrEdtTemplateCreate( &TS_iterate.TML, TS_iterate.FUNC, _paramc, _depc );

    ocrEdtCreate( &TS_iterate.EDT, TS_iterate.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, NULL_GUID, &TS_iterate.EVT);

    ocrGuid_t TS_iterate_EVT;
    ocrEventCreate( &TS_iterate_EVT, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_iterate.EVT, TS_iterate_EVT,   0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( (ocrGuid_t) depv[0].guid, TS_iterate.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (ocrGuid_t) depv[1].guid, TS_iterate.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( (ocrGuid_t) depv[2].guid, TS_iterate.EDT, _idep++, DB_MODE_RO );

    itimestep += 1;

    if( itimestep <= ntimestep_l )
    {
        MyOcrTaskStruct_t TS_compute; _paramc = 3; _depc = 4;

        u64 compute_paramv[3];
        compute_paramv[0] = (u64) subdomainID;
        compute_paramv[1] = (u64) itimestep;
        compute_paramv[2] = (u64) ntimestep_l;

        TS_compute.FUNC = FUNC_compute;
        ocrEdtTemplateCreate( &TS_compute.TML, TS_compute.FUNC, _paramc, _depc );

        ocrEdtCreate( &TS_compute.EDT, TS_compute.TML,
                      EDT_PARAM_DEF, compute_paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_GUID, NULL );

        _idep = 0;
        ocrAddDependence( (ocrGuid_t) depv[0].guid, TS_compute.EDT, _idep++, DB_MODE_ITW );
        ocrAddDependence( (ocrGuid_t) depv[1].guid, TS_compute.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( (ocrGuid_t) depv[2].guid, TS_compute.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( TS_iterate_EVT,           TS_compute.EDT, _idep++, DB_MODE_NULL ); //the input event needs to be destroyed. But how and when?

    }

    return NULL_GUID;

}

ocrGuid_t FUNC_iterate(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;

    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_memberHandle = (ocrGuid_t) depv[0].guid;

    memberHandle_t* CP_memberHandle   = (memberHandle_t*) depv[0].ptr;
    memberHandle_t* CP_memberHandle_l = (memberHandle_t*) depv[1].ptr;
    memberHandle_t* CP_memberHandle_r = (memberHandle_t*) depv[2].ptr;

    s32 phase = itimestep%2;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    MyOcrTaskStruct_t TS_lsend; _paramc = 2; _depc = 1;

    u64 TS_lsend_paramv[2] = { paramv[0], paramv[1]};

    TS_lsend.FUNC = FUNC_lsend;
    ocrEdtTemplateCreate( &TS_lsend.TML, TS_lsend.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_lsend.EDT, TS_lsend.TML,
                          EDT_PARAM_DEF, TS_lsend_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_lsend.EVT);

    ocrAddDependence(     TS_lsend.EVT, CP_memberHandle->EVNT_lsend_finish[phase], 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( CP_memberHandle->DB_xIn, TS_lsend.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_rsend; _paramc = 2; _depc = 1;

    u64 TS_rsend_paramv[2] = { paramv[0], paramv[1] };

    TS_rsend.FUNC = FUNC_rsend;
    ocrEdtTemplateCreate( &TS_rsend.TML, TS_rsend.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_rsend.EDT, TS_rsend.TML,
                          EDT_PARAM_DEF, TS_rsend_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_rsend.EVT);

    ocrAddDependence(     TS_rsend.EVT, CP_memberHandle->EVNT_rsend_finish[phase], 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( CP_memberHandle->DB_xIn, TS_rsend.EDT, _idep++, DB_MODE_RO );

    //Receives happen via event dependencies; Use the event as a dependency for the receive to read the message DB.
    //Receive EDTs depend on the neighbor events. So, how do you know the neighbor events??
    MyOcrTaskStruct_t TS_lrecv; _paramc = 3; _depc = 2;

    u64 TS_lrecv_paramv[3];
    TS_lrecv_paramv[0] = paramv[0];
    TS_lrecv_paramv[1] = paramv[1];
    TS_lrecv_paramv[2] = (subdomainID!=0)?CP_memberHandle_l->EVNT_rsend_ack[phase]:NULL_GUID;

    TS_lrecv.FUNC = FUNC_lrecv;
    ocrEdtTemplateCreate( &TS_lrecv.TML, TS_lrecv.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_lrecv.EDT, TS_lrecv.TML,
                          EDT_PARAM_DEF, TS_lrecv_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_lrecv.EVT);

    ocrAddDependence(     TS_lrecv.EVT, CP_memberHandle->EVNT_lrecv_finish[phase], 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence( CP_memberHandle->DB_xIn, TS_lrecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence( (subdomainID!=0)?CP_memberHandle_l->EVNT_rsend_finish[phase]:NULL_GUID, TS_lrecv.EDT, _idep++, DB_MODE_ITW );

    MyOcrTaskStruct_t TS_rrecv; _paramc = 3; _depc = 2;

    u64 TS_rrecv_paramv[3];
    TS_rrecv_paramv[0] = paramv[0];
    TS_rrecv_paramv[1] = paramv[1];
    TS_rrecv_paramv[2] = (subdomainID!=NSUBDOMAINS-1)?CP_memberHandle_r->EVNT_lsend_ack[phase]:NULL_GUID;

    TS_rrecv.FUNC = FUNC_rrecv;
    ocrEdtTemplateCreate( &TS_rrecv.TML, TS_rrecv.FUNC, _paramc,_depc );

    ocrEdtCreate(         &TS_rrecv.EDT, TS_rrecv.TML,
                          EDT_PARAM_DEF, TS_rrecv_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_rrecv.EVT);

    ocrAddDependence(     TS_rrecv.EVT, CP_memberHandle->EVNT_rrecv_finish[phase], 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence(     CP_memberHandle->DB_xIn, TS_rrecv.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence(     (subdomainID!=NSUBDOMAINS-1)?CP_memberHandle_r->EVNT_lsend_finish[phase]:NULL_GUID, TS_rrecv.EDT, _idep++, DB_MODE_ITW );

    ocrGuid_t TS_verify_EVT;
    ocrEventCreate(       &TS_verify_EVT, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_verify; _paramc = 2; _depc = 3;

    TS_verify.FUNC = FUNC_verify;
    ocrEdtTemplateCreate( &TS_verify.TML, TS_verify.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_verify.EDT, TS_verify.TML,
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID,
                          &TS_verify.EVT );

    ocrAddDependence(     TS_verify.EVT, TS_verify_EVT, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence(     CP_memberHandle->DB_xIn, TS_verify.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence(     CP_memberHandle->EVNT_lrecv_finish[phase], TS_verify.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence(     CP_memberHandle->EVNT_rrecv_finish[phase], TS_verify.EDT, _idep++, DB_MODE_NULL );

    ocrGuid_t TS_update_EVT;
    ocrEventCreate(       &TS_update_EVT, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update; _paramc = 2; _depc = 4;

    TS_update.FUNC = FUNC_update;
    ocrEdtTemplateCreate( &TS_update.TML, TS_update.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_update.EDT, TS_update.TML,
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_update.EVT );

    ocrAddDependence(     TS_update.EVT, TS_update_EVT, 0, DB_MODE_RO );

    _idep = 0;
    ocrAddDependence(     CP_memberHandle->DB_xIn, TS_update.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence(     CP_memberHandle->EVNT_lsend_finish[phase], TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence(     CP_memberHandle->EVNT_rsend_finish[phase], TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence(     TS_verify_EVT, TS_update.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_resetEvents; _paramc = 2; _depc = 4;

    TS_resetEvents.FUNC = FUNC_resetEvents;
    ocrEdtTemplateCreate( &TS_resetEvents.TML, TS_resetEvents.FUNC, _paramc, _depc );

    ocrEdtCreate(         &TS_resetEvents.EDT, TS_resetEvents.TML,
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, NULL_GUID, &TS_resetEvents.EVT );

    _idep = 0;
    ocrAddDependence(     DB_memberHandle, TS_resetEvents.EDT, _idep++, DB_MODE_ITW );
    ocrAddDependence(     (subdomainID!=0)?CP_memberHandle->EVNT_lsend_ack[phase]:NULL_GUID, TS_resetEvents.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence(     (subdomainID!=NSUBDOMAINS-1)?CP_memberHandle->EVNT_rsend_ack[phase]:NULL_GUID, TS_resetEvents.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence(     TS_update_EVT, TS_resetEvents.EDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

ocrGuid_t FUNC_resetEvents(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_memberHandle       = (ocrGuid_t) depv[0].guid;
    memberHandle_t* CP_memberHandle = (memberHandle_t*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    s32 phase = (itimestep)%2;

    ocrEventDestroy( CP_memberHandle->EVNT_lsend_finish[phase] );
    ocrEventDestroy( CP_memberHandle->EVNT_rsend_finish[phase] );
    ocrEventDestroy( CP_memberHandle->EVNT_lsend_ack[phase]    );
    ocrEventDestroy( CP_memberHandle->EVNT_rsend_ack[phase]    );
    ocrEventDestroy( CP_memberHandle->EVNT_lrecv_finish[phase] );
    ocrEventDestroy( CP_memberHandle->EVNT_rrecv_finish[phase] );

    ocrEventCreate( &CP_memberHandle->EVNT_lsend_finish[phase], OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &CP_memberHandle->EVNT_rsend_finish[phase], OCR_EVENT_STICKY_T, true);
    ocrEventCreate( &CP_memberHandle->EVNT_lsend_ack[phase], OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &CP_memberHandle->EVNT_rsend_ack[phase], OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &CP_memberHandle->EVNT_lrecv_finish[phase], OCR_EVENT_STICKY_T, false);
    ocrEventCreate( &CP_memberHandle->EVNT_rrecv_finish[phase], OCR_EVENT_STICKY_T, false);

    return NULL_GUID;

}

ocrGuid_t FUNC_lsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn = (ocrGuid_t) depv[0].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    double* lsend;
    ocrGuid_t DB_lsend;
    ocrDbCreate(    &DB_lsend, (void **) &lsend,
                    sizeof(double)*HALO_RADIUS,
                    DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i;
    for( i = 0; i < HALO_RADIUS; i++ )
    {
        lsend[i] = xIn[HALO_RADIUS+i];
        PRINTF("lsend: id=%d i=%ld %f\n", subdomainID, HALO_RADIUS+i, lsend[i]);
    }

    if( subdomainID != 0 )
    {
        PRINTF("ID=%d FUNC_lsend lsend guid is %lu\n", subdomainID, DB_lsend);
        return DB_lsend;
    }
    else
    {
        PRINTF("ID=%d FUNC_lsend lsend guid is NULL\n", subdomainID);
        return NULL_GUID;
    }

}

ocrGuid_t FUNC_rsend(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn = (ocrGuid_t) depv[0].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    double* rsend;
    ocrGuid_t DB_rsend;
    ocrDbCreate(    &DB_rsend, (void **) &rsend,  sizeof(double)*HALO_RADIUS,
                    DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    int i;
    for( i = 0; i < HALO_RADIUS; i++ )
    {
        rsend[i] = xIn[SUBDOMAIN_SIZE+i];
        PRINTF("rsend: id=%d i=%ld %f\n", subdomainID, SUBDOMAIN_SIZE+i, rsend[i]);
    }

    if( subdomainID != NSUBDOMAINS - 1 )
    {
        PRINTF("ID=%d FUNC_rsend rsend guid is %lu\n", subdomainID, DB_rsend);
        return DB_rsend;
    }
    else
    {
        PRINTF("ID=%d FUNC_lsend lsend guid is NULL\n", subdomainID);
        return NULL_GUID;
    }

}

ocrGuid_t FUNC_lrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn    = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DB_rsend  = (ocrGuid_t) depv[1].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    int i;
    if( DB_rsend != NULL_GUID || subdomainID != 0 )
    {
        double* rsent = (double*) depv[1].ptr;

        for( i = 0; i < HALO_RADIUS; i++ )
        {
            xIn[i] = rsent[i];
            PRINTF("lrecv: id=%d i=%ld %f\n", subdomainID, i, xIn[i]);
        }

        ocrGuid_t EVNT_rsend_ack = (ocrGuid_t) paramv[2];
        ocrEventSatisfy(EVNT_rsend_ack, NULL_GUID);

        PRINTF("ID=%d FUNC_lrecv lrecv guid is %lu\n", subdomainID, DB_rsend);
        ocrDbDestroy(DB_rsend);

    }

    return NULL_GUID;
}

ocrGuid_t FUNC_rrecv(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn    = (ocrGuid_t) depv[0].guid;
    ocrGuid_t DB_lsend  = (ocrGuid_t) depv[1].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    int i;
    if( DB_lsend != NULL_GUID || subdomainID != NSUBDOMAINS - 1)
    {
        double* lsent = (double*) depv[1].ptr;

        for( i = 0; i < HALO_RADIUS; i++ )
        {
            xIn[HALO_RADIUS+SUBDOMAIN_SIZE+i] = lsent[i];
            PRINTF("rrecv: id=%d i=%ld %f\n", subdomainID, HALO_RADIUS+SUBDOMAIN_SIZE+i, xIn[HALO_RADIUS+SUBDOMAIN_SIZE+i]);
        }

        ocrGuid_t EVNT_lsend_ack = (ocrGuid_t) paramv[2];
        ocrEventSatisfy(EVNT_lsend_ack, NULL_GUID);

        PRINTF("ID=%d FUNC_rrecv rrecv guid is %lu\n", subdomainID, DB_lsend);
        ocrDbDestroy(DB_lsend);
    }

    return NULL_GUID;
}

ocrGuid_t FUNC_verify(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn = (ocrGuid_t) depv[0].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    s64 i;
    for( i = -HALO_RADIUS; i < SUBDOMAIN_SIZE+HALO_RADIUS; i++ )
    {
        if( (subdomainID == 0 && i < 0) || (subdomainID == NSUBDOMAINS - 1 && i >= SUBDOMAIN_SIZE) )
            continue;

        PRINTF("ID: %d xIn[%d]=%f\n", subdomainID, i+HALO_RADIUS, xIn[i+HALO_RADIUS]);

        if( xIn[i+HALO_RADIUS] != (double) ( subdomainID*SUBDOMAIN_SIZE + i + itimestep - 1) )
            break;
    }

    if( i != SUBDOMAIN_SIZE+HALO_RADIUS )
        PRINTF("ID: %d Verification failed! timestep %d\n", subdomainID, itimestep);
    else
        PRINTF("ID: %d Verification passed! timestep %d\n", subdomainID, itimestep);

    return NULL_GUID;
}

ocrGuid_t FUNC_update(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 subdomainID = (s32) paramv[0];
    s32 itimestep   = (s32) paramv[1];

    ocrGuid_t DB_xIn = (ocrGuid_t) depv[0].guid;

    double* xIn = (double*) depv[0].ptr;

    PRINTF("ID:%d %s timestep %d\n", subdomainID, __func__, itimestep);

    s64 i;
    for( i = 0; i < SUBDOMAIN_SIZE; i++ )
        xIn[i+HALO_RADIUS] = (double) ( subdomainID*SUBDOMAIN_SIZE + i + itimestep);

    if( (subdomainID == NSUBDOMAINS - 1) && (itimestep == 1))
    {
        //PRINTF("ID:%d Sleeping for 10 seconds timestep %d\n", subdomainID, itimestep );
        //sleep(10);
    }

    return NULL_GUID;
}

ocrGuid_t FUNC_mainFinalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);

    ocrGuid_t DB_globalHandle = depv[0].guid;

    ocrDbDestroy( DB_globalHandle );

    ocrShutdown();

    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("%s\n", __func__);

    ocrGuid_t DB_globalHandle;
    globalHandle_t* CP_globalHandle;

    ocrDbCreate( &DB_globalHandle, (void **) &CP_globalHandle, sizeof(globalHandle_t) + sizeof(ocrGuid_t)*(NSUBDOMAINS),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );
    ocrDbRelease( DB_globalHandle );

    ocrGuid_t TS_mainInit_EVT, TS_mainCompute_EVT, TS_mainFinalize_EVT;

    ocrEventCreate( &TS_mainInit_EVT, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_mainCompute_EVT, OCR_EVENT_STICKY_T, false );
    ocrEventCreate( &TS_mainFinalize_EVT, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_mainInit, TS_mainCompute, TS_mainFinalize;

    u64 mainInit_paramv[1] = { NSUBDOMAINS };

    TS_mainInit.FUNC = FUNC_mainInit;
    ocrEdtTemplateCreate( &TS_mainInit.TML, TS_mainInit.FUNC, 1, 1 );

    ocrEdtCreate(         &TS_mainInit.EDT, TS_mainInit.TML,
                          EDT_PARAM_DEF, mainInit_paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_FINISH, NULL_GUID, &TS_mainInit.EVT );

    ocrAddDependence(     TS_mainInit.EVT, TS_mainInit_EVT, 0, DB_DEFAULT_MODE );

    TS_mainCompute.FUNC = FUNC_mainCompute;
    ocrEdtTemplateCreate( &TS_mainCompute.TML, TS_mainCompute.FUNC, 0, 2 );

    ocrEdtCreate(         &TS_mainCompute.EDT, TS_mainCompute.TML,
                          EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                          EDT_PROP_FINISH, NULL_GUID, &TS_mainCompute.EVT );

    ocrAddDependence(     TS_mainCompute.EVT, TS_mainCompute_EVT, 0, DB_DEFAULT_MODE );

    TS_mainFinalize.FUNC = FUNC_mainFinalize;
    ocrEdtTemplateCreate( &TS_mainFinalize.TML, TS_mainFinalize.FUNC, 0, 2 );

    ocrEdtCreate(         &TS_mainFinalize.EDT, TS_mainFinalize.TML,
                          EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                          EDT_PROP_FINISH, NULL_GUID, &TS_mainFinalize.EVT );

    ocrAddDependence(     TS_mainFinalize.EVT, TS_mainFinalize_EVT, 0, DB_DEFAULT_MODE);

    ocrAddDependence( DB_globalHandle, TS_mainInit.EDT, 0, DB_MODE_ITW );

    ocrAddDependence( DB_globalHandle, TS_mainCompute.EDT, 0, DB_MODE_RO );
    ocrAddDependence( TS_mainInit_EVT, TS_mainCompute.EDT, 1, DB_DEFAULT_MODE);

    ocrAddDependence( DB_globalHandle,    TS_mainFinalize.EDT, 0, DB_MODE_ITW );
    ocrAddDependence( TS_mainCompute_EVT, TS_mainFinalize.EDT, 1, DB_DEFAULT_MODE );

    return NULL_GUID;
}
