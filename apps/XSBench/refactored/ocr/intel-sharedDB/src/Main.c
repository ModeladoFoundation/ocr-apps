/*
    Author: Chandra S. Martha
    Copyright Intel Corporation 2015

    This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
    and cannot be distributed without it. This notice cannot be removed or modified.

    This file is also subject to the license agreement located in the file LICENSE in the current directory.
*/

/* OCR implemenation of XSBench (mimicking MPI+OpenMP implementation)
 * Uses "shared" datablocks for Nuclide and unionied energy grids, respectively similar to reference OpenMP
 * code.
 */

#include "ocr.h"
#include "ocrAppUtils.h"
#include "SPMDappUtils.h"

#include "XSbench_header.h"
#include "timers.h"

#include "reduction.h"

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

    ocrEdtTemplateCreate( &PTR_rankTemplateH->iterationsPerThreadTML, iterationsPerThreadEdt, (sizeof(PRM_iterationsPerThread_t)+sizeof(u64)-1)/sizeof(u64), 11 );
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

	// rand() is only used in the serial initialization stages.
	// A custom RNG is used in parallel portions.
	#ifdef VERIFICATION
	srand(26);
	#else
	srand(time(NULL));
	#endif

    Inputs in = PTR_rankH->globalParamH.in;

	// =====================================================================
	// Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
	// =====================================================================

	// Allocate & fill energy grids
	if( mype == 0) PRINTF("Generating Nuclide Energy Grids...\n");

	#ifdef VERIFICATION
    u64* seed_v;
    ocrDbCreate( &PTR_rankDataH->DBK_seed_v, (void**) &seed_v, sizeof(u64), 0, NULL_HINT, NO_ALLOC );
    *seed_v = 1337;
    u64* hash;
    ocrDbCreate( &PTR_rankDataH->DBK_hash, (void**) &hash, sizeof(u64), 0, NULL_HINT, NO_ALLOC );
    *hash = 0;
	#endif

	NuclideGridPoint ** nuclide_grids = gpmatrix(&PTR_rankDataH->DBK_nuclide_grids, &PTR_rankDataH->DBK_nuclide_grid_ptrs,
                                                    in.n_isotopes,in.n_gridpoints);

	#ifdef VERIFICATION
	generate_grids_v( nuclide_grids, in.n_isotopes, in.n_gridpoints, seed_v );
	#else
	generate_grids( nuclide_grids, in.n_isotopes, in.n_gridpoints );
	#endif

	// Sort grids by energy
	if( mype == 0) PRINTF("Sorting Nuclide Energy Grids...\n");
	sort_nuclide_grids( nuclide_grids, in.n_isotopes, in.n_gridpoints );

	// Prepare Unionized Energy Grid Framework
	GridPoint * energy_grid = generate_energy_grid( &PTR_rankDataH->DBK_uEnergy_grid, &PTR_rankDataH->DBK_xs_grid, in.n_isotopes,
	                          in.n_gridpoints, nuclide_grids, mype );

	// Double Indexing. Filling in energy_grid with pointers to the
	// nuclide_energy_grids.
	set_grid_ptrs( energy_grid, nuclide_grids, in.n_isotopes, in.n_gridpoints, mype );

	// Get material data
	if( mype == 0 )
		PRINTF("Loading Mats...\n");
	int *num_nucs  = load_num_nucs( &PTR_rankDataH->DBK_num_nucs, in.n_isotopes);
	int **mats     = load_mats(PTR_rankDataH, num_nucs, in.n_isotopes);

	#ifdef VERIFICATION
	double **concs = load_concs_v(PTR_rankDataH, num_nucs, seed_v);
	#else
	double **concs = load_concs(PTR_rankDataH, num_nucs);
	#endif

	#ifdef VERIFICATION
    ocrDbRelease( PTR_rankDataH->DBK_seed_v );
    ocrDbRelease( PTR_rankDataH->DBK_hash );
    #endif

    ocrDbRelease( PTR_rankDataH->DBK_nuclide_grids );
    ocrDbRelease( PTR_rankDataH->DBK_nuclide_grid_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_uEnergy_grid );
    ocrDbRelease( PTR_rankDataH->DBK_xs_grid );
    ocrDbRelease( PTR_rankDataH->DBK_num_nucs );
    ocrDbRelease( PTR_rankDataH->DBK_mat_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_conc_ptrs );
    ocrDbRelease( PTR_rankDataH->DBK_mats_all );
    ocrDbRelease( PTR_rankDataH->DBK_concs_all );

	//for( int i = 0; i < 12; i++ )
    //{
    //    ocrDbRelease( PTR_rankDataH->DBK_mats[i] );
    //    ocrDbRelease( PTR_rankDataH->DBK_concs[i] );
    //}

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
    ocrDBK_t DBK_nuclide_grids = depv[_idep++].guid;
    ocrDBK_t DBK_uEnergy_grid = depv[_idep++].guid;
    ocrDBK_t DBK_xs_grid = depv[_idep++].guid;
    ocrDBK_t DBK_num_nucs = depv[_idep++].guid;
    ocrDBK_t DBK_mats_all = depv[_idep++].guid;
    ocrDBK_t DBK_concs_all = depv[_idep++].guid;
    #ifdef VERIFICATION
    ocrDBK_t DBK_hash = depv[_idep++].guid;
    #endif

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;
    u64* seed = depv[_idep++].ptr;
    double* xs = depv[_idep++].ptr;
	NuclideGridPoint * nuclide_grids_linear = depv[_idep++].ptr;
	GridPoint * uEnergy_grid = depv[_idep++].ptr;
    int* xs_grid = depv[_idep++].ptr;
    int* num_nucs = depv[_idep++].ptr;
    int* mats_all = depv[_idep++].ptr;
    double* concs_all = depv[_idep++].ptr;
    #ifdef VERIFICATION
    u64* vhash = depv[_idep++].ptr;
    #endif

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    int mype = PTR_rankH->myRank;

	NuclideGridPoint ** nuclide_grids;

    int n_isotopes = PTR_in->n_isotopes;
    int n_gridpoints = PTR_in->n_gridpoints;
	long n_unionized_grid_points = n_isotopes*n_gridpoints;

    ocrDBK_t DBK_nuclide_grid_ptrs;
    ocrDbCreate( &DBK_nuclide_grid_ptrs, (void **) &nuclide_grids, n_isotopes*sizeof(NuclideGridPoint *),
                    0, NULL_HINT, NO_ALLOC );

    assignNuclideGridPtrs( nuclide_grids_linear, nuclide_grids, n_isotopes, n_gridpoints);

    assignEnergyGridXsPtrs( xs_grid, uEnergy_grid, n_unionized_grid_points, n_isotopes);

    ocrDBK_t DBK_mat_ptrs, DBK_conc_ptrs;
	int ** mats;
    ocrDbCreate( &DBK_mat_ptrs, (void **) &mats, 12*sizeof(int *),
                    0, NULL_HINT, NO_ALLOC );

	double ** concs;
    ocrDbCreate( &DBK_conc_ptrs, (void **) &concs, 12*sizeof(double *),
                    0, NULL_HINT, NO_ALLOC );

    assign_mats_ptrs( num_nucs, mats_all, mats );
    assign_concs_ptrs( num_nucs, concs_all, concs );

    Inputs in;
    memcpy(&in, PTR_in, sizeof(Inputs));

    double macro_xs_vector[5] = { 0., 0., 0., 0., 0. };
    double p_energy;
    int mat;

    for( int i = ibegin; i <= iend; i++ )
    {

        // Status text
        if( INFO && mype == 0 && tid == 0 && i % 1000 == 0 )
        	PRINTF("\rCalculating XS's... (%.1f%% completed)",
        			(i / ( (double)in.lookups / (double) in.nthreads ))
        			/ (double) in.nthreads * 100.0);

        // Randomly pick an energy and material for the particle
        #ifdef VERIFICATION
        {
        	p_energy = rn_v(seed);
        	mat      = pick_mat(seed);
        }
        #else
        p_energy = rn(seed);
        mat      = pick_mat(seed);
        #endif

        // debugging
        DEBUG_PRINTF(("E = %f mat = %d\n", p_energy, mat));

        // This returns the macro_xs_vector, but we're not going
        // to do anything with it in this program, so return value
        // is written over.
        calculate_macro_xs( p_energy, mat, in.n_isotopes,
                            in.n_gridpoints, num_nucs, concs,
                            uEnergy_grid, nuclide_grids, mats,
                            macro_xs_vector );

        // Copy results from above function call onto heap
        // so that compiler cannot optimize function out
        // (only occurs if -flto flag is used)
        memcpy(xs, macro_xs_vector, 5*sizeof(double));

        // Verification hash calculation
        // This method provides a consistent hash accross
        // architectures and compilers.
        #ifdef VERIFICATION
        char line[256];
        DEBUG_PRINTF(("%.5f %d %.5f %.5f %.5f %.5f %.5f\n",
               p_energy, mat,
        	   macro_xs_vector[0],
        	   macro_xs_vector[1],
        	   macro_xs_vector[2],
        	   macro_xs_vector[3],
        	   macro_xs_vector[4]));
        sprintf(line, "%.5f %d %.5f %.5f %.5f %.5f %.5f",
               p_energy, mat,
        	   macro_xs_vector[0],
        	   macro_xs_vector[1],
        	   macro_xs_vector[2],
        	   macro_xs_vector[3],
        	   macro_xs_vector[4]);
        unsigned long vhash_local = hash(line, 10000);
        *vhash += vhash_local;
        #endif
    }

    if( PTR_PRM_iterationsPerThread->lastGeneration )
    {
        DEBUG_PRINTF(("Thread %d reached the end of iterations\n", tid));
        ocrEventSatisfySlot( PTR_rankH->loopCompletionLatchEVT, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
    }

    ocrDbDestroy( DBK_nuclide_grid_ptrs );
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
    #ifdef VERIFICATION
    ocrDBK_t DBK_seed_v = PTR_PRM_perThread->DBK_seed;
    #else
    ocrDBK_t DBK_seed = PTR_PRM_perThread->DBK_seed;
    #endif
    ocrDBK_t DBK_xs = PTR_PRM_perThread->DBK_xs;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_rankDataH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;

    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

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
    #ifdef VERIFICATION
    ocrAddDependence( DBK_seed_v, iterationsPerThreadEDT, _idep++, DB_MODE_EW );
    #else
    ocrAddDependence( DBK_seed, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    #endif
    ocrAddDependence( DBK_xs, iterationsPerThreadEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( PTR_rankDataH->DBK_nuclide_grids, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_uEnergy_grid, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_xs_grid, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_num_nucs, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_mats_all, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankDataH->DBK_concs_all, iterationsPerThreadEDT, _idep++, DB_MODE_RO );
    #ifdef VERIFICATION
    ocrAddDependence( PTR_rankDataH->DBK_hash, iterationsPerThreadEDT, _idep++, DB_MODE_EW );
    #else
    ocrAddDependence( NULL_GUID, iterationsPerThreadEDT, _idep++, DB_MODE_EW );
    #endif

    if( ub_g < PTR_in->lookups-1 )
    {
        PTR_PRM_perThread->generation++;

        ocrGuid_t lookUpKernelPerThreadEDT;

        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*) PTR_PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //lookUpKernelPerThreadEdt

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

    Inputs* PTR_in = &(PTR_rankH->globalParamH.in);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 nthreads = PTR_in->nthreads;

    PRM_perThread_t PRM_perThread;
    PRM_perThread.nthreads = nthreads;
    PRM_perThread.generation = 0;

    int mype = PTR_rankH->myRank;

	if( mype == 0 )
	{
		printf("\n");
		border_print();
		center_print("SIMULATION", 79);
		border_print();
	}

    for( int i = 0; i < nthreads; ++i )
    {
        u64 tid = i;
        PRM_perThread.tid = tid;

        ocrDBK_t DBK_seed;
        #ifdef VERIFICATION
        DBK_seed = PTR_rankDataH->DBK_seed_v;
        #else
        u64 *seed;
        ocrDbCreate( &DBK_seed, (void**) &seed, sizeof(u64), 0, NULL_HINT, NO_ALLOC );
        *seed = (tid+1)*19+17;
        ocrDbRelease( DBK_seed );
        #endif

        ocrDBK_t DBK_xs;
        double* xs;
        ocrDbCreate( &DBK_xs, (void**) &xs, 5*sizeof(double), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( DBK_xs );

        PRM_perThread.DBK_seed = DBK_seed;
        PRM_perThread.DBK_xs = DBK_xs;

        ocrGuid_t lookUpKernelPerThreadEDT;

        ocrEdtCreate( &lookUpKernelPerThreadEDT, PTR_rankTemplateH->lookUpKernelPerThreadTML, EDT_PARAM_DEF, (u64*)&PRM_perThread, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

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
	#ifdef VERIFICATION
    ocrDBK_t DBK_hash = depv[_idep++].guid;
    #endif

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    rankDataH_t* PTR_rankDataH = depv[_idep++].ptr;
    double* runtime = depv[_idep++].ptr;
	#ifdef VERIFICATION
    u64 vhash = *(u64*)(depv[_idep++].ptr);
    #else
    u64 vhash = 0;
    #endif

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
	    print_results( *PTR_in, mype, *runtime, nprocs, vhash );
	}

    //Destroy DBK, events, etc.
	#ifdef VERIFICATION
    ocrDbDestroy( PTR_rankDataH->DBK_seed_v );
    ocrDbDestroy( PTR_rankDataH->DBK_hash );
    #endif

    ocrDbDestroy( PTR_rankDataH->DBK_nuclide_grids );
    ocrDbDestroy( PTR_rankDataH->DBK_nuclide_grid_ptrs );
    ocrDbDestroy( PTR_rankDataH->DBK_uEnergy_grid );
    ocrDbDestroy( PTR_rankDataH->DBK_xs_grid );
    ocrDbDestroy( PTR_rankDataH->DBK_num_nucs );
    ocrDbDestroy( PTR_rankDataH->DBK_mat_ptrs );
    ocrDbDestroy( PTR_rankDataH->DBK_conc_ptrs );
    ocrDbDestroy( PTR_rankDataH->DBK_mats_all );
    ocrDbDestroy( PTR_rankDataH->DBK_concs_all );

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

    ocrEVT_t loopCompletionLatchEVT = PTR_rankH->loopCompletionLatchEVT;

	#ifdef VERIFICATION
    ocrDBK_t DBK_hash = PTR_rankDataH->DBK_hash;
    #endif

    ocrDbRelease( DBK_rankDataH );
    ocrDbRelease( DBK_rankH );

    //Cross-section lookup kernel
    ocrGuid_t lookUpKernelTML, lookUpKernelEDT, lookUpKernelOEVT, lookUpKernelOEVTS;
    ocrEdtTemplateCreate( &lookUpKernelTML, lookUpKernelEdt, 0, 2 );

    ocrEdtCreate( &lookUpKernelEDT, lookUpKernelTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

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
    ocrAddDependence( DBK_rankH, wrapUpEDT, _idep++, DB_MODE_RW ); //TODO
    ocrAddDependence( DBK_rankDataH, wrapUpEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpPerfTimerEVT, wrapUpEDT, _idep++, DB_MODE_RO );
	#ifdef VERIFICATION
    ocrAddDependence( DBK_hash, wrapUpEDT, _idep++, DB_MODE_RO );
    #else
    ocrAddDependence( NULL_GUID, wrapUpEDT, _idep++, DB_MODE_NULL );
    #endif

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

    ocrEdtCreate( &TS_initSimulation.EDT, TS_initSimulation.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

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

    ocrEdtCreate( &TS_xsbenchMain.EDT, TS_xsbenchMain.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

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
    getAffinityHintsForDBandEdt( &PTR_rankH->myDbkAffinityHNT, &PTR_rankH->myEdtAffinityHNT );

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
    ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&PRM_channelSetupEdt, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                    &PTR_rankH->myEdtAffinityHNT, NULL );

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
    print_inputs( in, nprocs, version );

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
