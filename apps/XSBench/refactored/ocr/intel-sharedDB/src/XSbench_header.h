#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<math.h>

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

// Structures
typedef struct{
    double energy;
    double total_xs;
    double elastic_xs;
    double absorbtion_xs;
    double fission_xs;
    double nu_fission_xs;
} NuclideGridPoint;

typedef struct{
    double energy;
    int * xs_ptrs;
} GridPoint;

typedef struct{
    int nprocs;
    int nthreads;
    long n_isotopes;
    int n_mats;
    long n_gridpoints;
    int lookups;
    char HM[10];
} Inputs;

typedef struct
{
    timer timers[number_of_timers];

    ocrDBK_t DBK_nuclide_grids, DBK_nuclide_grid_ptrs;
    ocrDBK_t DBK_uEnergy_grid;
    ocrDBK_t DBK_xs_grid;
    ocrDBK_t DBK_num_nucs;
    ocrDBK_t DBK_mats_all;
    ocrDBK_t DBK_concs_all;
    //ocrDBK_t DBK_mats[12], DBK_concs[12];
    ocrDBK_t DBK_mat_ptrs, DBK_conc_ptrs;

	#ifdef VERIFICATION
    ocrDBK_t DBK_seed_v, DBK_hash;
    #endif
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
    ocrEVT_t loopCompletionLatchEVT;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
} rankH_t;

// Function Prototypes
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);

NuclideGridPoint ** gpmatrix(ocrDBK_t *DBK_nuclide_grids, ocrDBK_t *DBK_nuclide_grid_ptrs, size_t m, size_t n);

void gpmatrix_free( NuclideGridPoint ** M );

int NGP_compare( const void * a, const void * b );

void generate_grids( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints );
void generate_grids_v( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints, u64* seed );

void sort_nuclide_grids( NuclideGridPoint ** nuclide_grids, long n_isotopes,
                         long n_gridpoints );

GridPoint * generate_energy_grid( ocrDBK_t* DBK_uEnergy_grid, ocrDBK_t* DBK_xs_grid, long n_isotopes, long n_gridpoints,
                                  NuclideGridPoint ** nuclide_grids, int mype);

void set_grid_ptrs( GridPoint * energy_grid, NuclideGridPoint ** nuclide_grids,
                    long n_isotopes, long n_gridpoints, int mype );

int binary_search( NuclideGridPoint * A, double quarry, int n );

void calculate_macro_xs(   double p_energy, int mat, long n_isotopes,
                           long n_gridpoints, int * restrict num_nucs,
                           double ** restrict concs,
                           GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids,
                           int ** restrict mats,
                           double * restrict macro_xs_vector );

void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
                           long n_gridpoints,
                           GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids, int idx,
                           double * restrict xs_vector );
void calculate_micro_xs_new(   double p_energy,
                           long n_gridpoints, int xs_indices_nuc,
                           NuclideGridPoint * restrict PTR_nuclide_data_j,
                           double * restrict xs_vector );

long grid_search( long n, double quarry, GridPoint * A);
long grid_search_double( long n, double quarry, double * A);

int * load_num_nucs( ocrDBK_t* DBK_num_nucs, long n_isotopes);
int ** load_mats( rankDataH_t* PTR_rankDataH, int * num_nucs, long n_isotopes );
double ** load_concs( rankDataH_t* PTR_rankDataH, int * num_nucs );
double ** load_concs_v( rankDataH_t* PTR_rankDataH, int * num_nucs, u64* seed );
int pick_mat(u64* seed);
double rn(u64* seed);
void counter_stop( int * eventset, int num_papi_events );
void counter_init( int * eventset, int * num_papi_events );
void do_flops(void);
void do_loads( int nuc,
               NuclideGridPoint ** restrict nuclide_grids,
               long n_gridpoints );
Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
double rn_v(u64* seed);
double round_double( double input );
unsigned long hash(unsigned char *str, int nbins);
size_t estimate_mem_usage( Inputs in );
void print_inputs(Inputs in, int nprocs, int version);
void print_results( Inputs in, int mype, double runtime, int nprocs, u64 vhash );
void binary_dump(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid);
void binary_read(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid);

void assign_mats_ptrs( int* num_nucs, int* mats_all, int** mats );
void assign_concs_ptrs( int* num_nucs, double* concs_all, double** concs );
void assignNuclideGridPtrs( NuclideGridPoint *full, NuclideGridPoint **M, size_t m, size_t n);
void assignEnergyGridXsPtrs( int* full, GridPoint* energy_grid, long n_unionized_grid_points, int n);

#endif
