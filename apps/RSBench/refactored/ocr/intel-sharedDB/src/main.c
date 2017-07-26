/*
    Author: Chandra S. Martha
    Copyright Intel Corporation 2016

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.

    This file is also subject to the license agreement located in the file LICENSE in the current directory.
*/

/* OCR implemenation of XSBench (mimicking MPI+OpenMP implementation)
 * Uses "shared" datablocks for pole and window data, respectively similar to reference OpenMP
 * code.
 */

#include "ocr.h"
#include "ocrAppUtils.h"

#include "rsbench.h"
#include "timers.h"

#include "reduction.h"
#include "ocrAppUtils.h"

ocrGuid_t iterationsPerThreadEdt( EDT_ARGS );
ocrGuid_t lookUpKernelPerThreadEdt( EDT_ARGS );

typedef struct
{
    int tid;
    u64 ibegin, iend;
    int lastGeneration;
} PRM_iterationsPerThread_t;

typedef struct
{
    int nthreads, tid;
    int generation;
    ocrDBK_t DBK_seed, DBK_xs;
    ocrDBK_t DBK_sigTfactors;
    ocrDBK_t DBK_reductionVars;
    ocrDBK_t rpLoopReductionDBK;
    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;
} PRM_perThread_t;

ocrGuid_t FNC_globalFinalize( EDT_ARGS )
{
    DEBUG_PRINTF(("\n%s\n", __func__));
    s32 _paramc, _depc, _idep;

    ocrShutdown();
    return NULL_GUID;
}

void initOcrObjects( rankH_t* PTR_rankH, u64 myRank, u64 nRanks )
{
    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u32 _paramc, _depc, _idep;

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;

    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
    PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;

    reductionPrivate_t* rpPerfTimerPTR;
    ocrDbCreate( &PTR_rankH->rpPerfTimerDBK, (void**) &rpPerfTimerPTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

    ocrEdtTemplateCreate( &PTR_rankTemplateH->iterationsPerThreadTML, iterationsPerThreadEdt, (sizeof(PRM_iterationsPerThread_t)+sizeof(u64)-1)/sizeof(u64), 15 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->lookUpKernelPerThreadTML, lookUpKernelPerThreadEdt, (sizeof(PRM_perThread_t)+sizeof(u64)-1)/sizeof(u64), 3 );

    rpPerfTimerPTR->nrank = nRanks;
    rpPerfTimerPTR->myrank = myRank;
    rpPerfTimerPTR->ndata = 1;
    rpPerfTimerPTR->reductionOperator = REDUCTION_F8_MAX;
    rpPerfTimerPTR->rangeGUID = PTR_globalOcrParamH->perfTimerReductionRangeGUID;
    rpPerfTimerPTR->reductionTML = NULL_GUID;
    rpPerfTimerPTR->new = 1;  //first time
    rpPerfTimerPTR->type = ALLREDUCE;

    ocrEventCreate(&(PTR_rankH->rpPerfTimerEVT), OCR_EVENT_STICKY_T, true );
    rpPerfTimerPTR->returnEVT = PTR_rankH->rpPerfTimerEVT;

    ocrGuidRangeCreate( &(PTR_rankH->loopReductionRangeGUID), PTR_in->nthreads, GUID_USER_EVENT_STICKY );
    ocrEventCreate(&(PTR_rankH->rpLoopReductionEVT), OCR_EVENT_STICKY_T, true );

    ocrEventCreate(&(PTR_rankH->loopCompletionLatchEVT), OCR_EVENT_LATCH_T, false );

    u32 k = 0;
    while( k < PTR_in->nthreads )
    {
        ocrEventSatisfySlot(PTR_rankH->loopCompletionLatchEVT, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
        k++;
    }

    DEBUG_PRINTF(("nRanks %d myRank %d\n", nRanks, myRank));
}

void initSimulation(rankDataH_t* PTR_rankDataH, rankH_t* PTR_rankH, u64 mype)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    srand(time(NULL));

    Inputs in = PTR_rankH->globalParamH.in;

    if( mype == 0 )
    {
        border_print();
        center_print("INITIALIZATION", 79);
        border_print();
    }

    profile_start( init_timer, &PTR_rankDataH->timers );

    // =====================================================================
    // Prepare Pole Paremeter Grids
    // =====================================================================
    // Allocate & fill energy grids
    if( mype == 0 ) PRINTF("Generating resonance distributions...\n");
    int * n_poles = generate_n_poles( &PTR_rankDataH->DBK_n_poles, in );

    // Allocate & fill Window grids
    if( mype == 0 ) PRINTF("Generating window distributions...\n");
    int * n_windows = generate_n_windows( &PTR_rankDataH->DBK_n_windows, in );

    // Get material data
    if( mype == 0 ) PRINTF("Loading Hoogenboom-Martin material data...\n");
    Materials M = get_materials( PTR_rankDataH, in );

    // Prepare full resonance grid
    if( mype == 0 ) PRINTF("Generating resonance parameter grid...\n");
    Pole ** poles = generate_poles( &PTR_rankDataH->DBK_poles, &PTR_rankDataH->DBK_pole_ptrs, in, n_poles );

    // Prepare full Window grid
    if( mype == 0 ) PRINTF("Generating window parameter grid...\n");
    Window ** windows = generate_window_params( &PTR_rankDataH->DBK_windows, &PTR_rankDataH->DBK_window_ptrs, in, n_windows, n_poles);

    //Prepare 0K Resonances
    if( mype == 0 ) PRINTF("Generating 0K l_value data...\n");
    double ** pseudo_K0RS = generate_pseudo_K0RS( &PTR_rankDataH->DBK_pseudo_K0RS, &PTR_rankDataH->DBK_pseudo_K0RS_ptrs, in );

    ocrDbRelease( PTR_rankDataH->DBK_n_poles );
    ocrDbRelease( PTR_rankDataH->DBK_n_windows);
    ocrDbRelease( PTR_rankDataH->DBK_poles );
    ocrDbRelease( PTR_rankDataH->DBK_windows );
    ocrDbRelease( PTR_rankDataH->DBK_pseudo_K0RS );
    ocrDbRelease( PTR_rankDataH->DBK_num_nucs );
    ocrDbRelease( PTR_rankDataH->DBK_mats_all );
    ocrDbRelease( PTR_rankDataH->DBK_concs_all );
    ocrDbRelease( PTR_rankDataH->DBK_pole_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_window_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_pseudo_K0RS_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_mat_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_conc_ptrs );

    profile_stop( init_timer, &PTR_rankDataH->timers );

    double init_time = get_elapsed_time( init_timer, &PTR_rankDataH->timers );
    if( mype == 0 ) PRINTF("Initialization Complete. (%.2f seconds)\n", init_time);
}

ocrGuid_t iterationsPerThreadEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    PRM_iterationsPerThread_t* PTR_PRM_iterationsPerThread = (PRM_iterationsPerThread_t*) paramv;

    int tid = PTR_PRM_iterationsPerThread->tid;
    s64 ibegin = PTR_PRM_iterationsPerThread->ibegin;
    s64 iend = PTR_PRM_iterationsPerThread->iend;

    DEBUG_PRINTF(("begin %d end %d\n", ibegin, iend));

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;
    ocrDBK_t DBK_seed = depv[_idep++].guid;
    ocrDBK_t DBK_xs = depv[_idep++].guid;
    ocrDBK_t DBK_sigTfactors = depv[_idep++].guid;
    ocrDBK_t DBK_reductionVars = depv[_idep++].guid;
    ocrDBK_t DBK_n_poles = depv[_idep++].guid;
    ocrDBK_t DBK_n_windows = depv[_idep++].guid;
    ocrDBK_t DBK_poles = depv[_idep++].guid;
    ocrDBK_t DBK_windows = depv[_idep++].guid;
    ocrDBK_t DBK_pseudo_K0RS = depv[_idep++].guid;
    ocrDBK_t DBK_num_nucs = depv[_idep++].guid;
    ocrDBK_t DBK_mats_all = depv[_idep++].guid;
    ocrDBK_t DBK_concs_all = depv[_idep++].guid;
    ocrDBK_t rpLoopReductionDBK = NULL_GUID;
    if( PTR_PRM_iterationsPerThread->lastGeneration )
    rpLoopReductionDBK = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;
    u64* seed = depv[_idep++].ptr;
    double* xs = depv[_idep++].ptr;
    complex double* sigTfactors = depv[_idep++].ptr;
    u64* reductionVars = depv[_idep++].ptr;
    int* n_poles = depv[_idep++].ptr;
    int* n_windows = depv[_idep++].ptr;
    Pole* poles_all = depv[_idep++].ptr;
    Window* windows_all = depv[_idep++].ptr;
    double* pseudo_K0RS_all = depv[_idep++].ptr;
    int* num_nucs = depv[_idep++].ptr;
    int* mats_all = depv[_idep++].ptr;
    double* concs_all = depv[_idep++].ptr;
    reductionPrivate_t* rpLoopReductionPTR = NULL;
    if( PTR_PRM_iterationsPerThread->lastGeneration )
    rpLoopReductionPTR = depv[_idep++].ptr;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    int mype = PTR_rankH->myRank;

    Inputs in;
    memcpy(&in, PTR_in, sizeof(Inputs));

    ocrDBK_t DBK_pole_ptrs;
    Pole ** poles;
    ocrDbCreate( &DBK_pole_ptrs, (void **) &poles, in.n_nuclides*sizeof(Pole *),
                    0, NULL_HINT, NO_ALLOC );
    assign_pole_ptrs( in.n_nuclides, n_poles, poles_all, poles );

    ocrDBK_t DBK_window_ptrs;
    Window ** windows;
    ocrDbCreate( &DBK_window_ptrs, (void **) &windows, in.n_nuclides*sizeof(Window *),
                    0, NULL_HINT, NO_ALLOC );
    assign_window_ptrs( in.n_nuclides, n_windows, windows_all, windows );

    ocrDBK_t DBK_pseudo_K0RS_ptrs;
    double ** pseudo_K0RS;
    ocrDbCreate( &DBK_pseudo_K0RS_ptrs, (void **) &pseudo_K0RS, in.n_nuclides*sizeof(double *),
                    0, NULL_HINT, NO_ALLOC );
    assign_pseudo_K0RS_ptrs( in.n_nuclides, in.numL, pseudo_K0RS_all, pseudo_K0RS );

    ocrDBK_t DBK_mat_ptrs, DBK_conc_ptrs;
    int ** mats;
    ocrDbCreate( &DBK_mat_ptrs, (void **) &mats, 12*sizeof(int *),
                    0, NULL_HINT, NO_ALLOC );

    double ** concs;
    ocrDbCreate( &DBK_conc_ptrs, (void **) &concs, 12*sizeof(double *),
                    0, NULL_HINT, NO_ALLOC );

    assign_mats_ptrs( num_nucs, mats_all, mats );
    assign_concs_ptrs( num_nucs, concs_all, concs );

    Materials materials;
    materials.num_nucs = num_nucs;
    materials.mats = mats;
    materials.concs = concs;

    CalcDataPtrs data;
    data.n_poles = n_poles;
    data.n_windows = n_windows;
    data.materials = materials;
    data.poles = poles;
    data.windows = windows;
    data.pseudo_K0RS = pseudo_K0RS;

    double macro_xs[4] = { 0., 0., 0., 0. };
    double E;
    int mat;

    u64* g_abrarov = &reductionVars[0];
    u64* g_alls = &reductionVars[1];

    u64 abrarov = 0;
    u64 alls = 0;

    for( int i = ibegin; i <= iend; i++ )
    {
        // Status text
        if( INFO && mype == 0 && tid == 0 && i % 1000 == 0 )
            PRINTF("\rCalculating XS's... (%.1f%% completed)",
                    (i / ( (double)in.lookups / (double) in.nthreads ))
                    / (double) in.nthreads * 100.0);

        // Randomly pick an energy and material for the particle
        mat      = pick_mat(seed);
        E = rn(seed);

        // debugging
        //DEBUG_PRINTF(("E = %f mat = %d\n", E, mat));

        // This returns the macro_xs_vector, but we're not going
        // to do anything with it in this program, so return value
        // is written over.
        calculate_macro_xs( macro_xs, mat, E, in, data, sigTfactors, &abrarov, &alls );
        *g_abrarov += abrarov;
        *g_alls += alls;

        // Results are copied onto heap to avoid some compiler
        // flags (-flto) from optimizing out function call
        memcpy(xs, macro_xs, 4*sizeof(double));
        DEBUG_PRINTF(("%.5f %d %.5f %.5f %.5f %.5f %.5f\n",
               E, mat,
               macro_xs[0],
               macro_xs[1],
               macro_xs[2],
               macro_xs[3],
               0.));
    }

    if( PTR_PRM_iterationsPerThread->lastGeneration )
    {
        DEBUG_PRINTF(("Thread %d reached the end of iterations\n", tid));
        ocrEventSatisfySlot( PTR_rankH->loopCompletionLatchEVT, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
        reductionLaunch( rpLoopReductionPTR, rpLoopReductionDBK, reductionVars );
    }

    ocrDbDestroy( DBK_pole_ptrs );
    ocrDbDestroy( DBK_window_ptrs );
    ocrDbDestroy( DBK_pseudo_K0RS_ptrs );
    ocrDbDestroy( DBK_mat_ptrs );
    ocrDbDestroy( DBK_conc_ptrs );

    return NULL_GUID;
}

ocrGuid_t lookUpKernelPerThreadEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    PRM_perThread_t* PTR_PRM_perThread = (PRM_perThread_t*) paramv;
    int tid = PTR_PRM_perThread->tid;
    int nthreads = PTR_PRM_perThread->nthreads;
    int generation = PTR_PRM_perThread->generation;
    ocrDBK_t DBK_seed = PTR_PRM_perThread->DBK_seed;
    ocrDBK_t DBK_xs = PTR_PRM_perThread->DBK_xs;
    ocrDBK_t DBK_sigTfactors = PTR_PRM_perThread->DBK_sigTfactors;
    ocrDBK_t DBK_reductionVars = PTR_PRM_perThread->DBK_reductionVars;
    ocrDBK_t rpLoopReductionDBK = PTR_PRM_perThread->rpLoopReductionDBK;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;
#if defined(ENABLE_EXTENSION_AFFINITY) && defined(SINGLE_RUN_ACROSS_PD)
    myDbkAffinityHNT = PTR_PRM_perThread->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_PRM_perThread->myEdtAffinityHNT;
#endif

#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
#endif

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    PRM_iterationsPerThread_t PRM_iterationsPerThread;

    u64 lb_g, ub_g;

    switch(SCHEDULER_TYPE)
    {
        case 0:
            lb_g = 0;
            ub_g = PTR_in->lookups-1;
            break;

        case 1:
            lb_g = generation*(CHUNK_SIZE*nthreads);
            ub_g = MIN(lb_g + (CHUNK_SIZE*nthreads)-1, PTR_in->lookups-1);
            break;

        default:
            PRINTF("Scheduler NOT supported\n");
            break;
    }

    PRM_iterationsPerThread.tid = tid;
    partition_bounds( tid, lb_g, ub_g, nthreads, &PRM_iterationsPerThread.ibegin, &PRM_iterationsPerThread.iend );

    PRM_iterationsPerThread.lastGeneration = 0;
    if( ub_g == PTR_in->lookups-1 ) PRM_iterationsPerThread.lastGeneration = 1;

    // NOTE that depending on the nthreads and lookups, it may turn out that
    // PRM_iterationsPerThread.ibegin > PRM_iterationsPerThread.iend
    // In which case, the corresponding tasks will be "dummy" tasks
    // It simplifies the "loop" completion logic
    ocrGuid_t iterationsPerThreadEDT, iterationsPerThreadOEVT, iterationsPerThreadOEVTS;

    ocrEdtCreate( &iterationsPerThreadEDT, PTR_rankTemplateH->iterationsPerThreadTML, EDT_PARAM_DEF, (u64*)&PRM_iterationsPerThread, EDT_PARAM_DEF, NULL,
                    EDT_PROP_FINISH, &myEdtAffinityHNT, &iterationsPerThreadOEVT ); //iterationsPerThreadEdt

    createEventHelper( &iterationsPerThreadOEVTS, 1);
    ocrAddDependence( iterationsPerThreadOEVT, iterationsPerThreadOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_rankDataH, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_seed, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xs, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_sigTfactors, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_reductionVars, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_n_poles, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_n_windows, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_poles, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_windows, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_pseudo_K0RS, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_num_nucs, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_mats_all, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_concs_all, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    if( PRM_iterationsPerThread.lastGeneration )
    ocrAddDependence( rpLoopReductionDBK, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    else
    ocrAddDependence( NULL_GUID, iterationsPerThreadEDT, _idep++, DB_MODE_RW );

    if( ub_g < PTR_in->lookups-1 )
    {
        PTR_PRM_perThread->generation++;

        ocrGuid_t lookUpKernelPerThreadEDT;

#ifdef ENABLE_SPAWNING_HINT
        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*) PTR_PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinitySpawnHNT, NULL ); //lookUpKernelPerThreadEdt
#else
        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*) PTR_PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //lookUpKernelPerThreadEdt
#endif

        _idep = 0;
        ocrAddDependence( DBK_rankH, lookUpKernelPerThreadEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankDataH, lookUpKernelPerThreadEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( iterationsPerThreadOEVTS, lookUpKernelPerThreadEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

ocrGuid_t lookUpKernelEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid; //DB_MODE_RO
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;    //DB_MODE_RO

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;
#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
#endif

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 nthreads = PTR_in->nthreads;

    PRM_perThread_t PRM_perThread;
    PRM_perThread.nthreads = nthreads;
    PRM_perThread.generation = 0;

    int mype = PTR_rankH->myRank;

    //Query the number of policy domains (nodes) available for the run
    //Map the SPMD EDTs onto the policy domains
    u64 affinityCount=1;
#if defined(ENABLE_EXTENSION_AFFINITY) && defined(SINGLE_RUN_ACROSS_PD)
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    if( mype == 0 ) PRINTF("Using affinity API: Count %"PRIu64"\n", affinityCount);
#else
    PRINTF("NOT Using affinity API\n");
#endif
    u64 PD_X = affinityCount;

    if( mype == 0 )
    {
        PRINTF("\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();

        PRINTF("Beginning Simulation.\n");
        #ifndef STATUS
        PRINTF("Calculating XS's...\n");
        #endif
    }

    for( int i = 0; i < nthreads; ++i )
    {
        u64 tid = i;
        PRM_perThread.tid = tid;

        ocrDBK_t DBK_seed;
        u64 *seed;
        ocrDbCreate( &DBK_seed, (void**) &seed, sizeof(u64), 0, NULL_HINT, NO_ALLOC );
        *seed = time(NULL)+1;
        *seed += tid;
        ocrDbRelease( DBK_seed );

        ocrDBK_t DBK_xs;
        double* xs;
        ocrDbCreate( &DBK_xs, (void**) &xs, 4*sizeof(double), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( DBK_xs );

        ocrDBK_t DBK_sigTfactors;
        complex double* sigTfactors;
        ocrDbCreate( &DBK_sigTfactors, (void**) &sigTfactors, PTR_in->numL*sizeof(complex double), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( DBK_sigTfactors );

        ocrDBK_t DBK_reductionVars;
        u64* reductionVars;
        ocrDbCreate( &DBK_reductionVars, (void**) &reductionVars, 2*sizeof(u64), 0, NULL_HINT, NO_ALLOC );
        reductionVars[0] = 0;
        reductionVars[1] = 0;
        ocrDbRelease( DBK_reductionVars );

        ocrDBK_t rpLoopReductionDBK;
        reductionPrivate_t* rpLoopReductionPTR;
        ocrDbCreate( &rpLoopReductionDBK, (void**) &rpLoopReductionPTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

        rpLoopReductionPTR->nrank = nthreads;
        rpLoopReductionPTR->myrank = i;
        rpLoopReductionPTR->ndata = 2;
        rpLoopReductionPTR->reductionOperator = REDUCTION_U8_ADD;
        rpLoopReductionPTR->rangeGUID = PTR_rankH->loopReductionRangeGUID;
        rpLoopReductionPTR->reductionTML = NULL_GUID;
        rpLoopReductionPTR->new = 1;  //first time
        rpLoopReductionPTR->type = REDUCE;

        rpLoopReductionPTR->returnEVT = NULL_GUID;
        if( i == 0 ) rpLoopReductionPTR->returnEVT = PTR_rankH->rpLoopReductionEVT;

        ocrDbRelease( rpLoopReductionDBK );

        u64 pdGridDims[1] = { PD_X };
        u64 edtGridDims[1] = { nthreads };

        int pd = getPolicyDomainID_Cart1D( i, edtGridDims, pdGridDims );
        DEBUG_PRINTF(("id %"PRIu64" map PD %"PRId32"\n", i, pd));

#if defined(ENABLE_EXTENSION_AFFINITY) && defined(SINGLE_RUN_ACROSS_PD)
        ocrGuid_t PDaffinityGuid;
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
        ocrSetHintValue( &myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

        PRM_perThread.DBK_seed = DBK_seed;
        PRM_perThread.DBK_xs = DBK_xs;
        PRM_perThread.DBK_sigTfactors = DBK_sigTfactors;
        PRM_perThread.DBK_reductionVars = DBK_reductionVars;
        PRM_perThread.rpLoopReductionDBK = rpLoopReductionDBK;
        PRM_perThread.rpLoopReductionDBK = rpLoopReductionDBK;
        PRM_perThread.myEdtAffinityHNT = myEdtAffinityHNT;
        PRM_perThread.myDbkAffinityHNT = myDbkAffinityHNT;

        ocrGuid_t lookUpKernelPerThreadEDT;

#ifdef ENABLE_SPAWNING_HINT
        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*)&PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinitySpawnHNT, NULL ); //lookUpKernelPerThreadEdt
#else
        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*)&PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL );  //lookUpKernelPerThreadEdt
#endif

        _idep = 0;
        ocrAddDependence( DBK_rankH, lookUpKernelPerThreadEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_rankDataH, lookUpKernelPerThreadEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( NULL_GUID, lookUpKernelPerThreadEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

ocrGuid_t launchReductionEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;
    ocrDBK_t rpPerfTimerDBK = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;
    reductionPrivate_t* rpPerfTimerPTR = depv[_idep++].ptr;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    profile_stop( total_timer, &PTR_rankDataH->timers );

    double runtime = get_elapsed_time( total_timer, &PTR_rankDataH->timers );

    reductionLaunch( rpPerfTimerPTR, rpPerfTimerDBK, &runtime );

    return NULL_GUID;
}

ocrGuid_t summaryEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;
    ocrDBK_t maxTimeDBK = depv[_idep++].guid;
    ocrDBK_t DBK_reductionVars = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;
    double* runtime = depv[_idep++].ptr;
    u64* reductionVars = depv[_idep++].ptr;

    u64* g_abrarov = &reductionVars[0];
    u64* g_alls = &reductionVars[1];

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    int mype = PTR_rankH->myRank;
    int nprocs = PTR_rankH->nRanks;

    if( mype == 0)
    {
        PRINTF("\n" );
        PRINTF("Simulation complete.\n" );

        // Print / Save Results and Exit
        print_results( *PTR_in, mype, *runtime, nprocs, *g_abrarov, *g_alls);
    }

    //Destroy DBK, events, etc.
    ocrDbDestroy( PTR_rankDataH->DBK_n_poles );
    ocrDbDestroy( PTR_rankDataH->DBK_n_windows);
    ocrDbDestroy( PTR_rankDataH->DBK_poles );
    ocrDbDestroy( PTR_rankDataH->DBK_windows );
    ocrDbDestroy( PTR_rankDataH->DBK_pseudo_K0RS );
    ocrDbDestroy( PTR_rankDataH->DBK_num_nucs );
    ocrDbDestroy( PTR_rankDataH->DBK_mats_all );
    ocrDbDestroy( PTR_rankDataH->DBK_concs_all );
    //ocrDbDestroy( PTR_rankDataH->DBK_pole_ptrs );
    //ocrDbDestroy( PTR_rankDataH->DBK_window_ptrs );
    //ocrDbDestroy( PTR_rankDataH->DBK_pseudo_K0RS_ptrs );
    //ocrDbDestroy( PTR_rankDataH->DBK_mat_ptrs );
    //ocrDbDestroy( PTR_rankDataH->DBK_conc_ptrs );

    ocrEventDestroy( PTR_rankH->rpPerfTimerEVT );

    if( mype == 0)
        ocrEventSatisfy(PTR_globalOcrParamH->finalOnceEVT, NULL_GUID );

    return NULL_GUID;
}

ocrGuid_t FNC_initSimulation( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrDBK_t DBK_rankDataH = depv[1].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankDataH_t* PTR_rankDataH = depv[1].ptr;

    u64 myRank = PTR_rankH->myRank;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 nthreads = PTR_in->nthreads;
    initOcrObjects( PTR_rankH, myRank, PTR_rankH->nRanks );

    initSimulation( PTR_rankDataH, PTR_rankH, myRank );

    int i;
    for( i = 0; i < number_of_timers; i++ )
    {
        PTR_rankDataH->timers[i].start = 0;
        PTR_rankDataH->timers[i].total = 0;
        PTR_rankDataH->timers[i].count = 0;
        PTR_rankDataH->timers[i].elapsed = 0;
    }
    profile_start( total_timer, &PTR_rankDataH->timers );

    ocrDBK_t rpPerfTimerDBK = PTR_rankH->rpPerfTimerDBK;
    ocrEVT_t rpPerfTimerEVT = PTR_rankH->rpPerfTimerEVT;
    ocrEVT_t rpLoopReductionEVT = PTR_rankH->rpLoopReductionEVT;

    ocrEVT_t loopCompletionLatchEVT = PTR_rankH->loopCompletionLatchEVT;

    ocrDbRelease( DBK_rankDataH );
    ocrDbRelease( DBK_rankH );

    //Cross-section lookup kernel
    ocrGuid_t lookUpKernelTML, lookUpKernelEDT, lookUpKernelOEVT, lookUpKernelOEVTS;
    ocrEdtTemplateCreate( &lookUpKernelTML, lookUpKernelEdt, 0, 2 );

#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
    ocrEdtCreate( &lookUpKernelEDT, lookUpKernelTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinitySpawnHNT, NULL );
#else
    ocrEdtCreate( &lookUpKernelEDT, lookUpKernelTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );
#endif

    ocrEdtTemplateDestroy( lookUpKernelTML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, lookUpKernelEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_rankDataH, lookUpKernelEDT, _idep++, DB_MODE_RO );

    //Create a summary EDT; stops the timer and launches reduction
    ocrGuid_t launchReductionTML, launchReductionEDT, launchReductionOEVT, launchReductionOEVTS;
    ocrEdtTemplateCreate( &launchReductionTML, launchReductionEdt, 0, 4 );

    ocrEdtCreate( &launchReductionEDT, launchReductionTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    ocrEdtTemplateDestroy( launchReductionTML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, launchReductionEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_rankDataH, launchReductionEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpPerfTimerDBK, launchReductionEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( loopCompletionLatchEVT, launchReductionEDT, _idep++, DB_MODE_NULL );

    //Create a wrapUp EDT that depends on the reduction output
    ocrGuid_t wrapUpTML, wrapUpEDT, wrapUpOEVT, wrapUpOEVTS;
    ocrEdtTemplateCreate( &wrapUpTML, summaryEdt, 0, 4 );

    ocrEdtCreate( &wrapUpEDT, wrapUpTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    ocrEdtTemplateDestroy( wrapUpTML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, wrapUpEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_rankDataH, wrapUpEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpPerfTimerEVT, wrapUpEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpLoopReductionEVT, wrapUpEDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t FNC_xsbenchMain( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
    s32 _idep, _paramc, _depc;

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    Inputs* PTR_Inputs = &(PTR_rankH->globalParamH.in);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 myRank = PTR_rankH->myRank;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrDBK_t DBK_rankDataH;
    rankDataH_t* PTR_rankDataH;

    ocrDbCreate( &(DBK_rankDataH), (void **) &PTR_rankDataH, sizeof(rankDataH_t),
                 DB_PROP_NONE, &myDbkAffinityHNT, NO_ALLOC );

    ocrDbRelease(DBK_rankDataH);

    MyOcrTaskStruct_t TS_initSimulation; _paramc = 0; _depc = 2;

    TS_initSimulation.FNC = FNC_initSimulation;
    ocrEdtTemplateCreate( &TS_initSimulation.TML, TS_initSimulation.FNC, _paramc, _depc );

#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
    ocrEdtCreate( &TS_initSimulation.EDT, TS_initSimulation.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinitySpawnHNT, NULL );
#else
    ocrEdtCreate( &TS_initSimulation.EDT, TS_initSimulation.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );
#endif

    ocrEdtTemplateDestroy( TS_initSimulation.TML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_initSimulation.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_rankDataH, TS_initSimulation.EDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

typedef struct
{
    u64 myRank;

} PRM_channelSetupEdt_t;

ocrGuid_t channelSetupEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
    s32 _idep, _paramc, _depc;

    PRM_channelSetupEdt_t* PRM_channelSetupEdt = (PRM_channelSetupEdt_t*) paramv;

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    Inputs* PTR_Inputs = &(PTR_rankH->globalParamH.in);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 myRank = PRM_channelSetupEdt->myRank;

    ocrDbRelease(DBK_rankH);

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    MyOcrTaskStruct_t TS_xsbenchMain; _paramc = 0; _depc = 1;

    TS_xsbenchMain.FNC = FNC_xsbenchMain;
    ocrEdtTemplateCreate( &TS_xsbenchMain.TML, TS_xsbenchMain.FNC, _paramc, _depc );

#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
    ocrEdtCreate( &TS_xsbenchMain.EDT, TS_xsbenchMain.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinitySpawnHNT, NULL );
#else
    ocrEdtCreate( &TS_xsbenchMain.EDT, TS_xsbenchMain.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );
#endif

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_xsbenchMain.EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t initEdt( EDT_ARGS )
{
    PRM_init1dEdt_t* PTR_PRM_initEdt = (PRM_init1dEdt_t*) paramv;

    u64 myRank = PTR_PRM_initEdt->id;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    void *PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t *PTR_globalParamH = (globalParamH_t *) depv[1].ptr;

    ocrGuid_t DBK_rankH;
    rankH_t *PTR_rankH;

    //DB creation calls
    ocrDbCreate( &DBK_rankH, (void**) &PTR_rankH, sizeof(rankH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy( &(PTR_rankH->globalParamH), PTR_globalParamH, sizeof(globalParamH_t) );

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;
    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
    PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;

    s32 gx = PTR_PRM_initEdt->edtGridDims[0];

    PTR_rankH->myRank = myRank;
    PTR_rankH->nRanks = gx;

    int ix0, ix;

    globalCoordsFromRank_Cart1D( myRank, gx, &ix0 );

    ocrGuid_t channelSetupTML, stickyEVT;
    ocrEDT_t channelSetupEDT;

    PRM_channelSetupEdt_t PRM_channelSetupEdt;
    PRM_channelSetupEdt.myRank = myRank;

    ocrEdtTemplateCreate( &channelSetupTML, channelSetupEdt, sizeof(PRM_channelSetupEdt_t)/sizeof(u64), 1 );
#ifdef ENABLE_SPAWNING_HINT
    ocrHint_t myEdtAffinitySpawnHNT = myEdtAffinityHNT;
    ocrSetHintValue(&myEdtAffinitySpawnHNT, OCR_HINT_EDT_SPAWNING, 1);
    ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&PRM_channelSetupEdt, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                    &myEdtAffinitySpawnHNT, NULL );
#else
    ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&PRM_channelSetupEdt, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                    &myEdtAffinityHNT, NULL );
#endif

    ocrDbRelease(DBK_rankH);

    ocrAddDependence( DBK_rankH, channelSetupEDT, 0, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t wrapUpEdt( EDT_ARGS )
{
    PRINTF("Shutting down\n");
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    void * PTR_cmdLineArgs = depv[0].ptr;
    u32 argc = getArgc( PTR_cmdLineArgs );

    //Pack the PTR_cmdLineArgs into the "cannonical" char** argv
    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
       argv[a] = getArgv( PTR_cmdLineArgs, a );

    int version = 13;
    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI( argc, argv );

    int nprocs = in.nprocs;

    // Print-out of Input Summary
    print_input_summary( in );

    u64 nRanks = nprocs;

    //Create OCR objects to pass down to the child EDTs
    //for collective operatations among the child EDTs
    globalOcrParamH_t ocrParamH;
    ocrGuidRangeCreate( &(ocrParamH.perfTimerReductionRangeGUID), nRanks, GUID_USER_EVENT_STICKY );

    ocrGuid_t wrapUpTML, wrapUpEDT;
    ocrEdtTemplateCreate( &wrapUpTML, wrapUpEdt, 0, 1 );
    ocrEdtCreate( &wrapUpEDT, wrapUpTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEventCreate( &(ocrParamH.finalOnceEVT), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG );
    ocrEdtTemplateDestroy( wrapUpTML );

    ocrAddDependence( ocrParamH.finalOnceEVT, wrapUpEDT, 0, DB_MODE_RO );

    //A datablock to store the commandline and the OCR objectes created above
    ocrGuid_t DBK_globalParamH;
    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &DBK_globalParamH, (void**) &PTR_globalParamH, sizeof(globalParamH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy( &(PTR_globalParamH->in), &in, sizeof(Inputs) );
    memcpy( &(PTR_globalParamH->ocrParamH), &ocrParamH, sizeof(globalOcrParamH_t) );

    //Release the changes so they are visible for any dependent EDTs below
    ocrDbRelease( DBK_globalParamH );

    u64 edtGridDims[1] = { nprocs };

    //All SPMD EDTs depend on the following dependencies
    ocrGuid_t spmdDepv[2] = { DBK_cmdLineArgs, DBK_globalParamH };

    //Fork SPMD EDTs where each SPMD task is a Monte-Carlo instance of the XSbench
    forkSpmdEdts_Cart1D( initEdt, edtGridDims, spmdDepv );

    return NULL_GUID;
}
