#include "ocr.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<complex.h>

#define PI 3.14159265359

#include<time.h>
#include<sys/time.h>

#define ENABLE_EXTENSION_LABELING

#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity

#include "ocrAppUtils.h"
#include "timers.h"

// I/O Specifiers
#define INFO 1

//0 --> "openmp" static scheduling equivalent
//1 --> dynamic scheduling equivalent
#ifndef SCHEDULER_TYPE
#define SCHEDULER_TYPE 1
#endif

//Chunk size of lookup loop if dynamic scheduling is desired
#ifndef CHUNK_SIZE
#define CHUNK_SIZE 1000
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

// typedefs
typedef enum __hm{SMALL, LARGE, XL, XXL} HM_size;

typedef struct{
    int nprocs;
    int nthreads;
    int n_nuclides;
    int n_mats;
    int lookups;
    int avg_n_poles;
    int avg_n_windows;
    HM_size HM;
    int numL;
    int doppler;
} Inputs;

typedef struct{
    int * num_nucs;
    int ** mats;
    double ** concs;
} Materials;

typedef struct{
    complex double MP_EA;
    complex double MP_RT;
    complex double MP_RA;
    complex double MP_RF;
    short int l_value;
} Pole;

typedef struct{
    double T;
    double A;
    double F;
    int start;
    int end;
} Window;

typedef struct{
  int * n_poles;
  int * n_windows;
  Materials materials;
  Pole ** poles;
  Window ** windows;
  double ** pseudo_K0RS;
} CalcDataPtrs;

typedef struct
{
    timer timers[number_of_timers];

    ocrDBK_t DBK_n_poles;
    ocrDBK_t DBK_n_windows;
    ocrDBK_t DBK_poles, DBK_pole_ptrs;
    ocrDBK_t DBK_windows, DBK_window_ptrs;
    ocrDBK_t DBK_pseudo_K0RS, DBK_pseudo_K0RS_ptrs;
    ocrDBK_t DBK_num_nucs;
    ocrDBK_t DBK_mats_all;
    ocrDBK_t DBK_concs_all;
    //ocrDBK_t DBK_mats[12], DBK_concs[12];
    ocrDBK_t DBK_mat_ptrs, DBK_conc_ptrs;

} rankDataH_t;

typedef struct
{
    ocrTML_t iterationsPerThreadTML, lookUpKernelPerThreadTML;
} rankTemplateH_t;

typedef struct
{
    ocrGuid_t perfTimerReductionRangeGUID;
    ocrEVT_t finalOnceEVT;
} globalOcrParamH_t;

typedef struct
{
    Inputs in;
    globalOcrParamH_t ocrParamH;
} globalParamH_t;

typedef struct
{
    s64 nRanks, myRank;
    globalParamH_t globalParamH;
    Inputs in;
    rankTemplateH_t rankTemplateH;
    ocrDBK_t rpPerfTimerDBK;
    ocrEVT_t rpPerfTimerEVT;
    ocrGuid_t loopReductionRangeGUID;
    ocrEVT_t rpLoopReductionEVT;
    ocrEVT_t loopCompletionLatchEVT;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
} rankH_t;

// io.c
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int( int a );
Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
void print_input_summary(Inputs input);
void print_results(Inputs input, int mype, double runtime, int nprocs, u64 g_abrarov, u64 g_alls);

// init.c
int* generate_n_poles( ocrDBK_t* DBK_n_poles, Inputs input );
int * generate_n_windows( ocrDBK_t* DBK_n_windows, Inputs input );
Pole ** generate_poles( ocrDBK_t* DBK_poles, ocrDBK_t* DBK_pole_ptrs, Inputs input, int * n_poles );
Window ** generate_window_params( ocrDBK_t* DBK_windows, ocrDBK_t* DBK_windows_ptrs, Inputs input, int * n_windows, int * n_poles );
double ** generate_pseudo_K0RS( ocrDBK_t* DBK_pseudo_K0RS, ocrDBK_t* DBK_pseudo_K0RS_ptrs, Inputs input );
void assign_pole_ptrs( int n_nuclides, int* n_poles, Pole* contiguous, Pole** R );
void assign_window_ptrs( int n_nuclides, int* n_windows, Window* contiguous, Window** R );
void assign_pseudo_K0RS_ptrs( int n_nuclides, int numL, double* contiguous, double** R );

// material.c
int * load_num_nucs(ocrDBK_t* DBK_num_nucs, long n_isotopes);
void assign_mats_ptrs( int* num_nucs, int* mats_all, int** mats );
void assign_concs_ptrs( int* num_nucs, double* concs_all, double** concs );
int ** load_mats( rankDataH_t* PTR_rankDataH, int * num_nucs, long n_isotopes );
double ** load_concs( rankDataH_t* PTR_rankDataH, int * num_nucs );
int pick_mat( unsigned long * seed );
Materials get_materials( rankDataH_t* PTR_rankDataH, Inputs input);

// utils.c
double rn(unsigned long * seed);
size_t get_mem_estimate( Inputs input );

// rs_kernel.c
double complex fast_nuclear_W( double complex Z );
void calculate_macro_xs( double * macro_xs, int mat, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors, u64 * abrarov, u64 * alls );
void calculate_micro_xs( double * micro_xs, int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors);
void calculate_micro_xs_doppler( double * micro_xs, int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors, u64 * abrarov, u64 * alls);
void calculate_sig_T( int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors );

// papi.c
void counter_init( int *eventset, int *num_papi_events );
void counter_stop( int * eventset, int num_papi_events );
