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

#include "../src/micro_kernels.h"
#include <stdio.h>
#include <math.h>

#ifndef TMAX
#define TMAX 5000
#endif
#ifndef MMAX
#define NMAX 5000
#endif
#define N NMAX
#define T TMAX

static real_t a[NMAX], b[NMAX];
static real_t aa[NMAX], bb[NMAX];

#define jac(A_var, B_var) {						\
    int t, i, j;							\
    for (t = 0; t < T; t++) {						\
      for (i = 2; i < N - 1; i++) {					\
	B_var[i] = ((real_t)333/1000) *					\
	  (A_var[i - 1] + A_var[i] + A_var[i + 1]); /*S1*/		\
      }									\
      for (j = 2; j < N - 1; j++) {					\
	A_var[j] = B_var[j]; /*S2*/					\
      }									\
    }									\
  }

void initialize() {
  int i;

  for (i = 0; i < N; i++) {
    a[i] = aa[i] = b[i] = bb[i] = i/2;
  }
}

void initialize_once() {
  initialize();
}

void show(){
  int i;

  for (i=0; i<8; i++) {
    fprintf(stderr,"e[%d] = %10.10f\n", i, a[i]);
  }

  for (i=N-8; i<N; i++) {
    fprintf(stderr,"e[%d] = %10.10f\n", i, a[i]);
  }
}

#pragma rstream map
void imp(real_t A[NMAX], real_t B[NMAX]) {
  jac(A, B)
}

void kernel(){
  imp(a,b);
}

int check() {
  jac(aa,bb);

  return check_vectors("a", "aa", a, aa, NMAX);
}

// Number of flops in the kernel
double flops_per_trial() {
  return (double) (mul_flop+add_flop)*2*NMAX*TMAX;
}
int nb_samples = NMAX*TMAX;
char const * function_name = "one_d_imperfect_jacobi";
