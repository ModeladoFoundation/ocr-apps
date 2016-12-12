// IRAD rights

/***---**--*
 *
 *  Copyright (C) 2007 Reservoir Labs. All rights reserved.
 *
 *--**---***/

/*
 * $Id$
 */


#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "micro_kernels.h"

// Options
int check_results        = 0;
int full_check_results   = 0;
int max_trials           = 1;
int verbose              = 0;
int show_results         = 0;
char * nb_procs          = "0";
int max_errors           = 10;
double epsilon           = 1e-5;
int count_full_tiles     = 0;
int count_non_full_tiles = 0;
char * results_filename  = (char *) NULL;

// keep it global: some benchmarks refer to it as an extern global variable
int PARAMS[16];

static void * coldify_caches();
static double get_time_in_seconds();
static void process_arguments(int argc, char* const argv[]);

#define JUNK_SIZE_X86 (1<<22)
long long int *junk;

#ifdef MAESTRO_TARGET
#include <arch/cycle.h>
#endif

#ifdef ENABLE_HUGE_PAGES
extern void finalize();
#endif

#ifdef CELL_TARGET
double CELL_total_time_in_spus(void);
#endif

#ifdef GPU_TARGET
#include "driver_types.h"
#include "cuda_runtime_api.h"
void CUDAMEMCPY(void *A, void * B, unsigned long C, int D) {
    cudaMemcpy(A, B, (size_t)(C), (enum cudaMemcpyKind)(D));
}

int CUDAMALLOC(void **A, unsigned long n) {
   return cudaMalloc(A, n);
}

int CUDAFREE(void *ptr) {
   return cudaFree(ptr);
}
#endif

#ifdef SWARM_TARGET
#warning Using SWARM backend.
#include <eti/swarm_convenience.h>
extern swarm_Runtime_t * rswSwarmRuntime;
swarm_Runtime_t * rswSwarmRuntime;
#endif // SWARM

// -------- OCR specific entry point and functions ----------
#ifdef OCR_TARGET
#include <ocr.h>

// the main design of the EDTs is intended to support asynchronous kernels
// even though kernels are synchronous by default (they were aysnch in HTA)

// begin EDT, realizing all the initialization of a kernel and launches it
ocrGuid_t beginEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    initialize();
    coldify_caches();

    *((double *) depv[0].ptr) -= get_time_in_seconds();

    kernel();

    return depv[0].guid;
}

// end EDT, realizing all the finalization of a kernel once it is done
ocrGuid_t endEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    *((double *) depv[0].ptr) += get_time_in_seconds();
    return depv[0].guid;
}

// shutdown EDT run once all the kernel trials have run
ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    double time = *((double *) depv[0].ptr);
    int rc;

    if (check_results) {
	if (verbose) {
	    printf("[ checking results ]\n");
	}
	rc = check();
    } else {
	rc = 0;
    }

    if (show_results) {
	show();
    }

    double flops_per_second = flops_per_trial() * max_trials / time;
    double flops_per_measure = flops_per_second / 1e9;

    printf("trials=%d time=%gs %s=%g %s\n",
	   max_trials,
	   time,
	   "gflop/s",
	   flops_per_measure,
	   (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
    fprintf(stderr, "%s , %s , %d, %g , %g, %d\n", function_name, nb_procs,
	    nb_samples, time / max_trials, flops_per_trial(), rc);

    if (results_filename!=NULL) {
	FILE * results_file = fopen(results_filename, "a");
	fprintf(results_file, "%s , %s , %d, %g , %g, %d\n", function_name,
		nb_procs, nb_samples, time / max_trials, flops_per_trial(), rc);
	fclose(results_file);
    }

    if (rc) {
        ocrAbort(rc);
        return NULL_GUID;
    }

    ocrShutdown();
    return NULL_GUID;
}

// main EDT (actual entry point of the program)
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;
    void *argsPtr = depv[0].ptr;

    u64 argc = getArgc(argsPtr);
    char **argv = malloc(argc * sizeof(*argv));
    for (i = 0; i < argc; ++i) {
        argv[i] = getArgv(argsPtr, i);
    }
    process_arguments(argc, argv);
    free(argv);

    initialize_once();

    // create two EDTs per trial
    ocrGuid_t *edtG = malloc(2 * max_trials * sizeof(*edtG));
    ocrGuid_t *edtOutEvG = malloc(2 * max_trials * sizeof(*edtOutEvG));
    for (i = 0; i < max_trials; i++) {
        ocrGuid_t tmplBG, tmplEG;
        ocrEdtTemplateCreate(&tmplBG, beginEdt, 0, 1);
        ocrEdtCreate(edtG + 2 * i, tmplBG, 0, NULL, EDT_PARAM_DEF, NULL,
            EDT_PROP_FINISH, NULL_HINT, edtOutEvG + 2 * i);
        ocrEdtTemplateDestroy(tmplBG);

        // the begin EDTs are finish EDTs so the DB they return will not be
        // transmitted...
        ocrEdtTemplateCreate(&tmplEG, endEdt, 0, 2);
        ocrEdtCreate(edtG + 2 * i + 1, tmplEG, 0, NULL, EDT_PARAM_DEF, NULL,
            EDT_PROP_NONE, NULL_HINT, edtOutEvG + 2 * i + 1);
        ocrEdtTemplateDestroy(tmplEG);

        // create the chain of EDTs
        if (i > 0) {
            ocrAddDependence(edtOutEvG[2 * i - 1], edtG[2 * i], 0, DB_MODE_RW);
        }
        // slot 1 of end EDTs is used by the DB to remain consistent
        ocrAddDependence(edtOutEvG[2 * i], edtG[2 * i + 1], 1, DB_MODE_RW);
    }

    // create the shutdown EDT and link it to the last endEdt
    ocrGuid_t tmplG, shutEdtG;
    ocrEdtTemplateCreate(&tmplG, shutdownEdt, 0, 1);
    ocrEdtCreate(&shutEdtG, tmplG, 0, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
        NULL_HINT, NULL);
    ocrEdtTemplateDestroy(tmplG);
    ocrAddDependence(edtOutEvG[2 * max_trials - 1], shutEdtG, 0, DB_MODE_RW);
    free(edtOutEvG);

    // use the first slot of all the EDTs to transmit an execution time counter
    double *time;
    ocrGuid_t timeG;
    ocrDbCreate(&timeG, (void **) &time, sizeof(*time), DB_PROP_NONE, NULL_HINT,
        NO_ALLOC);
    *time = 0;
    ocrDbRelease(timeG);

    // start the execution
    ocrAddDependence(timeG, edtG[0], 0, DB_MODE_RW);

    // in the mean time, transmit the time DB to all the end EDTs
    for (i = 0; i < max_trials; ++i) {
        ocrAddDependence(timeG, edtG[2 * i + 1], 0, DB_MODE_RW);
    }

    free(edtG);
    return NULL_GUID;
}

#endif // OCR_TARGET

// -------- End of OCR specific functions ----------

#ifdef CLUSTER_GA_TARGET
#include <rstream_ga.h>
#endif

#ifdef RSTREAM_PAPI
/* Papi headers */
# include <papi.h>
/* Linux x86 headers */
# include <sched.h>

int retval;
int EventSet = PAPI_NULL;
long_long values[1];
long_long all_values[128];
const char *all_names[128];
char descr[PAPI_MAX_STR_LEN];
PAPI_event_info_t evinfo;


// const unsigned int eventlist[] = {
//     PAPI_TOT_CYC,
//     PAPI_TOT_INS,
//     PAPI_VEC_INS,
//     PAPI_VEC_SP,
//     PAPI_VEC_DP,
//     PAPI_BR_INS,
//     PAPI_BR_MSP,
//     PAPI_FP_INS,
//     PAPI_L1_DCA,
//     PAPI_L1_DCM,
//     PAPI_L1_LDM,
//     PAPI_L1_STM,
//     //    PAPI_L1_ICA,
//     //    PAPI_L1_ICM,
//     PAPI_L2_DCA,
//     PAPI_L2_DCM,
//     PAPI_L2_LDM,
//     PAPI_L2_STM,
//     //    PAPI_L2_ICA,
//     //    PAPI_L2_ICM,
//     PAPI_PRF_DM,
//     PAPI_TLB_TL,
//     PAPI_TLB_DM,
//     //    PAPI_TLB_IM,
//     PAPI_MEM_RCY,
//     PAPI_MEM_WCY,
//     PAPI_RES_STL,
//     0
// };3

// const char* eventnames[] = {
//     "Total                   cycles:\t",
//     "Instrs.              completed:\t",
//     "SIMD              instructions:\t",
//     "Single prec. SIMD instructions:\t",
//     "Double prec. SIMD instructions:\t",
//     "Branch            instructions:\t",
//     "Cond.  BR              mispred:\t",
//     "FP                instructions:\t",
//     "L1 D cache              access:\t",
//     "L1 D cache              misses:\t",
//     "L1 LD                   misses:\t",
//     "L1 ST                   misses:\t",
//     //    "L1 I cache              access:\t",
//     //    "L1 I cache              misses:\t",
//     "L2 D cache              access:\t",
//     "L2 D cache              misses:\t",
//     "L2 LD                   misses:\t",
//     "L2 ST                   misses:\t",
//     //    "L2 I cache              access:\t",
//     //    "L2 I cache              misses:\t",
//     "D pref cache              miss:\t",
//     "D-TLB              real misses:\t",
//     "D-TLB                   misses:\t",
//     //    "I-TLB                   misses:\t",
//     "Cycles stalled memory    reads:\t",
//     "Cycles stalled memory   writes:\t",
//     "Cycles stalled on any resource:\t",
//     0
// };

const unsigned int eventlist[] = {
    PAPI_FP_STAL,
    // PAPI_MEM_SCY,
    // PAPI_RES_STL,
    0
};

const char* eventnames[] = {
    "FP STALL:\t",
    // "MEM STALL:\t",
    // "TOTAL STALL:\t",
    0
};

void papi_test_fail(char *file, int line, char *call, int retval)
{
    char buf[128];

    memset(buf, '\0', sizeof(buf));
    if (retval != 0)
	fprintf(stdout,"%-40s FAILED\nLine # %d\n", file, line);
    else {
	fprintf(stdout,"%-40s SKIPPED\n", file);
	fprintf(stdout,"Line # %d\n", line);
    }
    if (retval == PAPI_ESYS) {
	sprintf(buf, "System error in %s", call);
	perror(buf);
    } else if (retval > 0) {
	fprintf(stdout,"Error: %s\n", call);
    } else if (retval == 0) {
	fprintf(stdout,"Error: %s\n", call);
    } else {
	char errstring[PAPI_MAX_STR_LEN];
	PAPI_perror(retval, errstring, PAPI_MAX_STR_LEN);
	fprintf(stdout,"Error in %s: %s\n", call, errstring);
    }
    fprintf(stdout,"\n");
    if ( PAPI_is_initialized() ) PAPI_shutdown();
    exit(1);
}

void papi_clean_cache() {
    long double* cache_cleaner =
	(long double*) malloc (150000 * sizeof (long double));
    int cache_iter;

    for (cache_iter = 0; cache_iter < 150000; ++cache_iter)
	cache_cleaner[cache_iter] = M_PI * cache_iter;
}

void papi_check_failures_and_start(int evid) {
    if (PAPI_get_event_info(eventlist[evid], &evinfo) != PAPI_OK)
	papi_test_fail(__FILE__, __LINE__, "PAPI_get_event_info", retval);


    if ((retval = PAPI_start(EventSet)) != PAPI_OK)
	papi_test_fail(__FILE__, __LINE__, "PAPI_start", retval);
}

void papi_init() {
    if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
	papi_test_fail(__FILE__, __LINE__, "PAPI_library_init", retval);

    if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK)
	papi_test_fail(__FILE__, __LINE__, "PAPI_create_eventset", retval);

    /* Use FIFO scheduler to limit OS interference. */
    struct sched_param schedParam;
    schedParam.sched_priority = 99;
    sched_setscheduler(0, SCHED_FIFO, &schedParam);
}
#endif

static void setParams(int opt) {
    int i;
    char c;

    int val = 0, index = 0, sign = 1;
    for (i=0; i<strlen(optarg); i++) {
	c = optarg[i];

	if (c < '0' || c > '9') {
	    if (val > 0) {
		if (index >= 16) {
		    printf("ERROR: Cannot specify more than 16 parameter values\n");
		    exit(1);
		}
		printf("Parameter %d Value: %d\n", index, val);
		PARAMS[index++] = val * sign;
	    } else if (val == 0 && c == '-') {
		sign = -1;
	    }
	    val = 0;
	    sign = 1;
	    continue;
	}

	val = val*10 + (c - '0');
    }

    if (val > 0) {
	if (index >= 16) {
	    printf("ERROR: Cannot specify more than 16 parameter values\n");
	    exit(1);
	}
	printf("Parameter %d Value: %d\n", index, val);
	PARAMS[index++] = val * sign;
    }

    for (i = index; i<16; i++) {
	PARAMS[i] = -1;
    }
}

// Parses command line arguments.
static void process_arguments(int argc, char* const argv[]) {
    int opt;
    int error = 0;
    const char * prog = argv[0];


    while ((opt = getopt(argc, argv, "a:cft:T:vpP:e:E:r:")) >= 0) {
	switch (opt) {
	case 'a': setParams(opt);            break;
	case 'c': check_results = 1;            break;
	case 'f': full_check_results = 1;            break;
	case 'T': max_trials    = atol(optarg); break;
	case 'v': verbose = 1; break;
	case 'p': show_results = 1; break;
	case 'P': nb_procs = strdup(optarg); break;
	case 'e': epsilon = atof(optarg); break;
	case 'E': max_errors = atol(optarg); break;
	case 'r': results_filename = optarg; break;
	default:  error = 1;
	    fprintf(stderr, "unrecognized option '-%c'\n", optopt);
	    break;
	}
    }

    if (max_trials <= 0) {
        error = 1;
    }

    if (error) {
	printf(
	       "Usage: %s [-c][-T <trials>][-v]\n"
	       "\t-a add parameters and specify their values\n"
	       "\t-c enable results checking\n"
	       "\t-T <trials> specifiy the number of trials to run (default 1)\n"
	       "\t-v print extra messages during execution\n"
	       "\t-p show a portion of the results\n"
	       "\t-P <proc> declare that the program runs on proc processors\n"
	       "\t-e <float> use the given error bounds\n"
	       "\t-E <num>  limit to the given number of errors\n"
	       "\t-r <filename> append performance results in a CSV format to <filename>.\n",
	       prog
	       );
	exit(1);
    }
}

static void * coldify_caches() {
    int i, j;
#if defined (__TILECC__)
    // The TILE64 and Maestro architecture only has 64K of cache.
    size_t size = 65536/sizeof(long long int);
#else
    size_t size = JUNK_SIZE_X86;
#endif
    junk = (long long int *)malloc(size*sizeof(long long int));

    // We don't want to use OpenMP to initialize the memory on Tilera for
    // two reasons: (1) it is really slow, and (2) we only need to invalidate
    // the home cache in tile 0.
#if !defined (__TILECC__)
#pragma omp parallel for private(i)
#endif
    for (i=0; i < size; i+=8) {
	// try to have each core touch one or more disjoint cache lines
	for (j=i; j<=i+7; j++) {
	    junk[j] += j;
	}
    }

    return junk;
}

// Return the current time in seconds
static double get_time_in_seconds(void) {

    // Note: the TRIPS simulator does not support gettimeofday
#if defined(__trips__)
    return 0.0;
#else
#if defined(MAESTRO_TARGET)
    return (double) get_cycle_count();
#else
    struct timeval tv;
    int _tod = gettimeofday(&tv, NULL);
    if (_tod != 0) abort();
    return (double) tv.tv_sec + tv.tv_usec * 1e-6;
#endif
#endif
}

#ifndef OCR_TARGET
// The main function
int main(int argc, char * const argv[]) {
    double t0, t1,
	total_time, total_SPU_time, total_GPU_time,
	flops_per_second, flops_SPU_per_second, flops_GPU_per_second;
    int i, rc;

    process_arguments(argc, argv);

    initialize_once();

    if (verbose) {
	printf("[ %s: starting tests ]\n", argv[0]);
    }

    total_time = 0;
    total_SPU_time = 0;
    total_GPU_time = 0;

#ifdef GPU_TARGET
    GPU_init_device();

  #if !defined(CUDA_PLUS_HOST)
    t0 = get_time_in_seconds();
    GPU_initialize_once();
    t1 = get_time_in_seconds();
    total_time += t1 - t0;
  #endif
#endif

#ifdef SWARM_TARGET
    // start the Swarm runtime without giving it an initial codelet.
    rswSwarmRuntime = swarm_posix_startRuntime(NULL, NULL, NULL, NULL);
#endif // SWARM

#ifdef CLUSTER_GA_TARGET
    rga_init();
#endif

    // PAPI Wrapper Loop
#ifdef RSTREAM_PAPI
    int evid, eviditer;

    papi_init();

    for (evid = 0; eventlist[evid] != 0; evid++) {
	printf("Working on eventid: %d\t%s\n", evid, eventnames[evid]);
	//PAPI_event_code_to_name(eventlist[evid], descr);
	if (PAPI_add_event(EventSet, eventlist[evid]) != PAPI_OK)
	    continue;

	papi_clean_cache();
	papi_check_failures_and_start(evid);
#endif

#if defined(MC_MKL) && !defined(RSTREAM_PAPI)
	// Ignore the first trial in MKL which includes the overhead of
	// loading the library code.
	// Only if PAPI is not used otherwise it will mess up the perf counters.
	max_trials++;
#endif

	for (i = 0; i < max_trials; i++) {
	    initialize();
#if !defined(RSTREAM_PAPI)
	    // No cache coldification that pollutes PAPI output !
	    coldify_caches();
#endif
	    t0 = get_time_in_seconds();

	    kernel();

#ifdef GPU_TARGET
	    cudaThreadSynchronize();
#endif
	    t1 = get_time_in_seconds();

#if defined(MC_MKL) && !defined(RSTREAM_PAPI)
	    if (i >= 1) {
		// Ignore the first trial (i == 0) in MKL which includes the overhead of
		// loading the library code. But count the i == 1 twice to
		// account for the missed time !
		total_time += t1 - t0;
	    }
#else
		total_time += t1 - t0;
#endif

#ifdef GPU_TARGET
	    total_GPU_time += t1 - t0;
#endif
#ifdef CELL_TARGET
	    total_SPU_time += CELL_total_time_in_spus();
#endif

#if !defined(RSTREAM_PAPI)
	    // No cache coldification that pollutes PAPI output !
	    // printf("junk = %ld\n", junk[((int)rand())%(JUNK_SIZE_X86)]);
	    free(junk);
#endif
	}

#if defined(MC_MKL) && !defined(RSTREAM_PAPI)
	// Revert ignore the first trial in MKL which includes the overhead of
	// loading the library code.
	max_trials--;
#endif

#ifdef GPU_TARGET
  #if !defined(CUDA_PLUS_HOST)
	t0 = get_time_in_seconds();
	GPU_finalize_once();
	t1 = get_time_in_seconds();
	total_time += t1 - t0;
  #endif
#endif


#ifdef RSTREAM_PAPI
	// End PAPI Wrapper Loop
	if ((retval = PAPI_read(EventSet, &values[0])) != PAPI_OK)
	    papi_test_fail(__FILE__, __LINE__, "PAPI_read", retval);
	if ((retval = PAPI_stop(EventSet,NULL)) != PAPI_OK)
	    papi_test_fail(__FILE__, __LINE__, "PAPI_stop", retval);

	all_values[evid] = values[0];
	all_names[evid] = eventnames[evid];

	if ((retval = PAPI_remove_event(EventSet, eventlist[evid])) != PAPI_OK)
	    papi_test_fail(__FILE__, __LINE__, "PAPI_remove_event", retval);
    } //End PAPI Wrapper loop body

    if ((retval = PAPI_destroy_eventset(&EventSet)) != PAPI_OK)
	papi_test_fail(__FILE__, __LINE__, "PAPI_destroy_eventset", retval);

    // Output measure results.
    printf ("\n\n\n****************\n****************\n****************\nValues:\n");
    for (eviditer = 0; eviditer < evid; ++eviditer)
	printf ("%s%32llu \n", all_names[eviditer], all_values[eviditer]);
    printf ("\n");
#endif

    // Print internal monitoring stats if needed
    PRINT_MONITORING_STATS() ;

#ifdef CLUSTER_GA_TARGET
    rga_exit();
#endif

    if (check_results) {
	if (verbose) {
	    printf("[ %s: checking results ]\n", argv[0]);
	}
	rc = check();
    } else {
	rc = 0;
    }

    if (show_results) {
	show();
    }

    flops_per_second = flops_per_trial() * max_trials / total_time;

#ifdef CELL_TARGET
    flops_SPU_per_second = flops_per_trial() * max_trials / total_SPU_time;
    printf("%s: trials=%d time=%gs gflop/s=%g gflops_SPU/s=%g %s\n",
	   argv[0],
	   max_trials,
	   total_time,
	   flops_per_second/1e9,
	   flops_SPU_per_second/1e9,
	   (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
    fprintf(stderr, "%s , %s , %d, %g , %g, %g, %d, %s\n", function_name, nb_procs,
	    nb_samples, total_time/max_trials, total_SPU_time/max_trials, flops_per_trial(), rc,
	    (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
#else
#ifdef GPU_TARGET
    flops_GPU_per_second = flops_per_trial() * max_trials / total_GPU_time;
    printf("%s: trials=%d time=%gs time_GPU=%gs gflop/s=%g gflops_GPU/s=%g %s\n",
	   argv[0],
	   max_trials,
	   total_time,
	   total_GPU_time,
	   flops_per_second/1e9,
	   flops_GPU_per_second/1e9,
	   (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
    fprintf(stderr, "%s , %s , %d, %g , %g, %g, %d, %s\n", function_name, nb_procs,
	    nb_samples, total_time/max_trials, total_GPU_time/max_trials, flops_per_trial(), rc,
	    (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));

    cudaError_t err1 = cudaGetLastError();
    if (err1 != cudaSuccess) {
	fprintf(stderr, "Error in kernel: %s\n", cudaGetErrorString(err1));
	exit(1);
    }

    cudaError_t err = cudaThreadSynchronize();
    if (err != cudaSuccess) {
	fprintf(stderr, "Error in thread synchronize: %s\n", cudaGetErrorString(err));
	exit(1);
    }
#else
#ifdef MAESTRO_TARGET
#define PERF_MEASURE "flop/cycle"
    double flops_per_measure = flops_per_second;
#else
#define PERF_MEASURE "gflop/s"
    double flops_per_measure = flops_per_second / 1e9;
#endif


    printf("%s: trials=%d time=%gs %s=%g %s\n",
	   argv[0],
	   max_trials,
	   total_time,
	   PERF_MEASURE,
	   flops_per_measure,
	   (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
    fprintf(stderr, "%s , %s , %d, %g , %g, %d\n", function_name, nb_procs,
	    nb_samples, total_time/max_trials, flops_per_trial(), rc);

    if (results_filename!=NULL) {
	FILE * results_file = fopen(results_filename, "a");
	fprintf(results_file, "%s , %s , %d, %g , %g, %d\n", function_name,
		nb_procs, nb_samples, total_time/max_trials, flops_per_trial(),
		rc);
	fclose(results_file);
    }
#endif
#endif

#ifdef SWARM_TARGET
    // Exit the swarm runtime
    swarm_shutdownRuntime(rswSwarmRuntime);
#endif // SWARM

#ifdef ENABLE_HUGE_PAGES
    finalize();
#endif

    if (nb_procs[0]!='0') free(nb_procs);
    return rc;
}
#endif  // OCR_TARGET

// _________________________ Matrix routines __________________________________
//

void print_submatrix(const char * name, const void * A,
                     int m, int n, int i0, int i1, int j0, int j1) {
    const real_t * M = (const real_t *)A;
    int i, j;

    printf("____________ %s[%d-%d][%d-%d] of %s[%d][%d]___________\n",
	   name, i0, i1-1, j0, j1-1, name, m, n);
    for (i = i0; i < i1; i++) {
	for (j = j0; j < j1; j++) {
	    if (j > j0) printf(" ");
	    //          printf("%10.10f", (double) M[i*n+j]);
	    printf("%4lg", M[i*n+j]);
	}
	printf("\n");
    }
}

void print_complex_submatrix(const char * name, const void * A,
			     int m, int n, int i0, int i1, int j0, int j1) {
    const real_t * M = (const real_t *)A;
    int i, j;

    printf("____________ %s[%d-%d][%d-%d] of %s[%d][%d]___________\n",
	   name, i0, i1-1, j0, j1-1, name, m, n);
    for (i = i0; i < i1; i++) {
	for (j = j0; j < j1; j++) {
	    if (j > j0) printf(" ");
	    //          printf("[%10.10f, %10.10f]", (double) M[2*(i*n+j)],
	    //                                       (double) M[2*(i*n+j)+1]);
	    printf("[%lg, %lg]", (double) M[2*(i*n+j)],
		   (double) M[2*(i*n+j)+1]);
	}
	printf("\n");
    }
}

void print_int_submatrix(const char * name, const void * A,
			 int m, int n, int i0, int i1, int j0, int j1) {
    const int * M = (const int *)A;
    int i, j;

    printf("____________ %s[%d-%d][%d-%d] of %s[%d][%d]___________\n",
	   name, i0, i1-1, j0, j1-1, name, m, n);
    for (i = i0; i < i1; i++) {
	for (j = j0; j < j1; j++) {
	    if (j > j0) printf(" ");
	    printf("%d", M[i*n+j]);
	}
	printf("\n");
    }
}


//
//  Test whether a and b are equal (where epsilon is the error).
//  I.e.,
//
//  |a-b| <= epsilon |a|   or |a-b| <= epsilon |b|
//
int fequal(double x, double y) {
    double diff;
    int isnan_x, isnan_y;
    int isinf_x, isinf_y;

    isnan_x = isnan(x);
    isnan_y = isnan(y);
    if (isnan_x && isnan_y) return 1;
    if (isnan_x || isnan_y) return 0;

    isinf_x = isinf(x);
    isinf_y = isinf(y);

    if (isinf_x && isinf_y) {
	return (x > 0 && y > 0) || (x < 0 && y < 0);
    }
    if (isinf_x || isinf_y) return 0;

    diff = fabs(x-y);
    // NTV: 03/30/2009 need this case distinction otherwise comparison against
    // 0.0 will always fail.
    if ((x == 0.0 && y != 0.0) ||
	(x != 0.0 && y == 0.0)) {
	return diff <= epsilon;
    }

    // NTV: 05/06/2009 need this case to rule out differences due to
    // imprecisions close to 0 on cell. Ideally it would only be activated on
    // cell.
    if (x <= epsilon && -epsilon <= x &&
	y <= epsilon && -epsilon <= y) {
	return diff <= epsilon;
    }

    return diff <= epsilon * fabs(x) ||
	diff <= epsilon * fabs(y);
}

int check_matrices(const char * name1, const char * name2,
                   const void * A, const void * B,
                   int m, int n) {
    int i, j;
    const real_t * X = (const real_t *)A;
    const real_t * Y = (const real_t *)B;
    int errors = 0;

    for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	    real_t x = X[i*n+j];
	    real_t y = Y[i*n+j];
	    if (! fequal(x, y)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d]=%lg) != (%s[%d][%d]=%lg) (diff=%lg)\n",
		       name1, i, j, x,
		       name2, i, j, y,
		       x-y);
	    }
	}
    }
EXIT:

    if (errors > 0) {
	printf("Number of errors=%d (max reported errors %d)\n",
	       errors, max_errors);
    }

    return errors > 0;
}

/*
   Checks for a submatrix embedded in a bigger matrix at (0,0).
   This version must be used in parametric codes when the initialization and
   the check are not performed on the full static array.
   This usually happens when we conservatively allocate a big chunk of memory
   but we do not want to initialize it all and compare it all for performance
   issues.
             ____________________
            |                    |
            |_____________       |
            |            |       |
            |            |       |
            |____________|_______|
*/
int check_matrices_portions(const char * name1, const char * name2,
			    const void * A, const void * B,
			    int m, int n, int Ndim) {
    int i, j;
    const real_t * X = (const real_t *)A;
    const real_t * Y = (const real_t *)B;
    int errors = 0;

    for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	    real_t x = X[i*Ndim+j];
	    real_t y = Y[i*Ndim+j];
	    if (! fequal(x, y)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d]=%lg) != (%s[%d][%d]=%lg) (diff=%lg)\n",
		       name1, i, j, x,
		       name2, i, j, y,
		       x-y);
	    }
	}
    }
EXIT:

    if (errors > 0) {
	printf("Number of errors=%d (max reported errors %d)\n",
	       errors, max_errors);
    }

    return errors > 0;
}

int check_complex_matrices(const char * name1, const char * name2,
                           const void * A, const void * B,
                           int m, int n) {
    int i, j;
    const real_t * X = (const real_t *)A;
    const real_t * Y = (const real_t *)B;
    int errors = 0;

    for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	    real_t x_r = X[2*(i*n+j)];
	    real_t y_r = Y[2*(i*n+j)];
	    real_t x_c = X[2*(i*n+j)+1];
	    real_t y_c = Y[2*(i*n+j)+1];
	    if (! fequal(x_r, y_r)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d][0]=%lg) != (%s[%d][%d][0]=%lg) (diff=%lg)\n",
		       name1, i, j, x_r,
		       name2, i, j, y_r,
		       x_r-y_r);
	    }
	    if (! fequal(x_c, y_c)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d][1]=%lg) != (%s[%d][%d][1]=%lg) (diff=%lg)\n",
		       name1, i, j, x_c,
		       name2, i, j, y_c,
		       x_c-y_c);
	    }
	}
    }
EXIT:

    if (errors > 0) {
	printf("Number of errors=%d (max reported errors %d)\n",
	       errors, max_errors);
    }

    return errors > 0;
}

/*
   Checks for a complex submatrix embedded in a bigger matrix at (0,0).
   This version must be used in parametric codes when the initialization and
   the check are not performed on the full static array.
   This usually happens when we conservatively allocate a big chunk of memory
   but we do not want to initialize it all and compare it all for performance
   issues.
             ____________________
            |                    |
            |_____________       |
            |            |       |
            |            |       |
            |____________|_______|
*/
int check_complex_matrices_portions(const char * name1, const char * name2,
				    const void * A, const void * B,
				    int m, int n, int Ndim) {
    int i, j;
    const real_t * X = (const real_t *)A;
    const real_t * Y = (const real_t *)B;
    int errors = 0;

    for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	    real_t x_r = X[2*(i*Ndim+j)];
	    real_t y_r = Y[2*(i*Ndim+j)];
	    real_t x_c = X[2*(i*Ndim+j)+1];
	    real_t y_c = Y[2*(i*Ndim+j)+1];
	    if (! fequal(x_r, y_r)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d][0]=%lg) != (%s[%d][%d][0]=%lg) (diff=%lg)\n",
		       name1, i, j, x_r,
		       name2, i, j, y_r,
		       x_r-y_r);
	    }
	    if (! fequal(x_c, y_c)) {
		if (++errors >= max_errors) goto EXIT;
		printf("(%s[%d][%d][1]=%lg) != (%s[%d][%d][1]=%lg) (diff=%lg)\n",
		       name1, i, j, x_c,
		       name2, i, j, y_c,
		       x_c-y_c);
	    }
	}
    }
EXIT:

    if (errors > 0) {
	printf("Number of errors=%d (max reported errors %d)\n",
	       errors, max_errors);
    }

    return errors > 0;
}

int check_vectors(const char * name1, const char * name2,
		  const void * SX, const void * SY,
		  int m) {
    int i;
    const real_t * X = (const real_t *)SX;
    const real_t * Y = (const real_t *)SY;
    int errors = 0;

    for (i = 0; i < m; i++) {
	real_t x = X[i];
	real_t y = Y[i];
	if (! fequal(x, y)) {
            if (++errors >= max_errors) goto VEXIT;
            printf("(%s[%d]=%lg) != (%s[%d]=%lg) (diff=%lg)\n",
		   name1, i, x,
		   name2, i, y,
		   x-y);
	}
    }
VEXIT:

    if (errors > 0) {
	printf("Number of errors=%d (max reported errors %d)\n",
	       errors, max_errors);
    }

    return errors > 0;
}

int check_int_vectors(const char * name1, const char * name2,
                  const void * SX, const void * SY,
                  int m) {
    int i;
    const int * X = (const int *)SX;
    const int * Y = (const int *)SY;
    int errors = 0;

    for (i = 0; i < m; i++) {
        int x = X[i];
        int y = Y[i];
        if (x != y) {
            if (++errors >= max_errors) goto VEXIT;
            printf("(%s[%d]=%d) != (%s[%d]=%d) (diff=%d)\n",
                   name1, i, x,
                   name2, i, y,
                   x-y);
        }
    }
VEXIT:

    if (errors > 0) {
        printf("Number of errors=%d (max reported errors %d)\n",
               errors, max_errors);
    }

    return errors > 0;
}

#ifdef GPU_TARGET
void GPU_init_device() {
    //Device Initialization
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    if (deviceCount == 0) {
	printf("No device supporting CUDA");
	exit(-1);
    }
    cudaSetDevice(0);
}
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
