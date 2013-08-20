//
// Roger A. Golliver Intel 2012 for UHPC project
//
// Simple wrappers to change SAR's fftw calls to fft based on Reservoir's fft
//

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <assert.h>

#ifdef RAG_SIM
#include "xe-codelet.h"
#include "xe_memory.h"
#include "xe_console.h"
#include "xe_global.h"
#endif

#ifdef RAG_AFL
#include "codelet.h"
#include "rmd_afl_all.h"
#endif

typedef float fftwf_complex[2];

typedef struct fftwf_plan_st { int dir; int Pof2; int hcSize; int SIZE; fftwf_complex *hcOut; fftwf_complex *hcIn; } fftwf_plan;

#define FFTW_FORWARD  ( 1)
#define FFTW_BACKWARD (-1)
#define FFTW_ESTIMATE ( 0)

fftwf_plan fftwf_plan_dft_1d(int hcSize, fftwf_complex *hcIn, fftwf_complex *hcOut, int dir, int est);

void fftwf_destroy_plan(fftwf_plan plan);

fftwf_complex *fftwf_malloc(size_t hcSize);

void fftwf_free(fftwf_complex *ptr);

void fftwf_execute(fftwf_plan plan);
