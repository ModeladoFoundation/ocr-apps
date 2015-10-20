#include "ocr.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<complex.h>

#ifdef PAPI
#include "papi.h"
#endif

#define PI 3.14159265359

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

//typedef struct{
//  int * n_poles;
//  int * n_windows;
//  Materials materials;
//  Pole ** poles;
//  Window ** windows;
//  double ** pseudo_K0RS;
//} CalcDataPtrs;

typedef struct
{
    int* n_poles;
    int* n_windows;
    Pole* poles_inuc;
    Window* windows_inuc;
    double* pseudo_K0RS_inuc;
} CalcDataPtrs;


// io.c
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int( int a );
Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
void print_input_summary(Inputs input);
void print_results(Inputs input, double runtime);

// init.c
void generate_n_poles( Inputs input, int *R );
void generate_n_windows( Inputs input, int *R );
void generate_poles( Inputs input, int * n_poles, ocrGuid_t* PTR_pole_DBguids_nuclide );
void generate_window_params( Inputs input, int * n_windows, int * n_poles, ocrGuid_t* PTR_window_DBguids_nuclide );
void generate_pseudo_K0RS( Inputs input, ocrGuid_t* PTR_pseudoK0RS_DBguids_nuclide );

// material.c
void load_num_nucs(Inputs input, int* num_nucs);
void load_mats( Inputs input, int * num_nucs, ocrGuid_t* PTR_nuclideIDs_DBguids_mat);
void load_concs( int * num_nucs, ocrGuid_t* PTR_nuclideConcs_DBguids_mat );
int pick_mat( unsigned long * seed );
void get_materials(Inputs input, int* PTR_mat_num_nucs, ocrGuid_t* PTR_nuclideIDs_DBguids_mat, ocrGuid_t* PTR_nuclideConcs_DBguids_mat);

// utils.c
double rn(unsigned long * seed);
size_t get_mem_estimate( Inputs input );

// rs_kernel.c
//void calculate_macro_xs( double * macro_xs, int mat, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors );
void calculate_micro_xs( double * micro_xs, int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors);
void calculate_micro_xs_doppler( double * micro_xs, int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors);
void calculate_sig_T( int nuc, double E, Inputs input, CalcDataPtrs data, complex double * sigTfactors );

// papi.c
void counter_init( int *eventset, int *num_papi_events );
void counter_stop( int * eventset, int num_papi_events );
