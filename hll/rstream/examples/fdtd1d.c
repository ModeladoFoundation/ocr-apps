/***---**--*
 *
 *  Copyright (C) 2009 Reservoir Labs. All rights reserved.
 *
 *--**---***/

/*
 * $Id$
 *
 * These tests have been taken from the GCC Graphite google group:
 *     http://groups.google.com/group/gcc-graphite?hl=en
 *
 * The tests are specifically mentioned on this thread (note the line break in
 * the url):
 *     http://groups.google.com/group/gcc-graphite/browse_thread/thread/
 *     d8e2b8eb1d1f4e29/63a39b41608e8dec?hl=en&q=polyhedra+benchmarks&pli=1
 *
 * All tests have been refactored to fit into the R-Stream micro_kernel
 * benchmark format.  This test can be viewed in it's original form in:
 *     testing/polyhedral_benchmarks/PolyKernels.tgz
 */


// OPERA/tile-cc rights

#include "micro_kernels.h"
#include <stdio.h>
#include <math.h>

#ifndef NMAX
#define NMAX 5000
#endif
#ifndef TMAX
#define TMAX 5000
#endif
#define N NMAX
#define T TMAX

static real_t e[NMAX], h[NMAX];
static real_t ee[NMAX], hh[NMAX];

#define fdtd1d(E_var, H_var) {					\
    int t,i;							\
    for (t=0; t < T; t++) {					\
      for (i=1; i < N-1; i++) {					\
        E_var[i] -= CST(0.5)*(H_var[i]-H_var[i-1]); \
      }								\
      for (i=0; i < N-1; i++) {					\
        H_var[i] -= CST(0.7)*(E_var[i+1]-E_var[i]); \
      }								\
    }								\
  }								\


void initialize()
{
  int i;

  for (i = 0; i < NMAX; i++) {
    e[i] = ee[i] = i;
    h[i] = hh[i] = i*CST(0.5);
  }
}

void initialize_once() {
  initialize();
}

void show(){
  int i;
  for (i=0; i<8; i++) {
    fprintf(stderr,"e[%d] = %10.10f\n", i, e[i]);
  }

  for (i=N-8; i<N; i++) {
    fprintf(stderr,"e[%d] = %10.10f\n", i, e[i]);
  }
}

#pragma rstream map
void fd(real_t A[NMAX], real_t B[NMAX]) {
  fdtd1d(A, B)
}
void kernel(){
  fd(e, h);
}

int check() {
  fdtd1d(ee, hh);

  return check_vectors("e", "ee", e, ee, NMAX);
}

// Number of flops in the kernel
double flops_per_trial() {
  return (double) (mul_flop+add_flop)*2*NMAX*TMAX;
}
int nb_samples = NMAX*TMAX;
char const * function_name = "fdtd1d";
