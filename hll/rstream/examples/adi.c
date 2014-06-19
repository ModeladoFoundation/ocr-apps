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

#ifndef TMAX
#define TMAX 500
#endif
#ifndef NMAX
#define NMAX 500
#endif
#define N NMAX
#define T TMAX

// +13, +23, +37
static real_t X[NMAX][NMAX], A[NMAX][NMAX], B[NMAX][NMAX];
static real_t TX[NMAX][NMAX], TA[NMAX][NMAX], TB[NMAX][NMAX];

#define adi(A_var, B_var, X_var) {					\
    int t, i1, i2;							\
    for (t = 0; t < T; t++) {						\
      /* Column Sweep */						\
      for (i1 = 0; i1 < N; i1++) {					\
	for (i2 = 1; i2 < N; i2++) {					\
	  X_var[i1][i2] = (X_var[i1][i2] -				\
			   (X_var[i1][i2-1] *				\
			    A_var[i1][i2]) / B_var[i1][i2-1]);		\
	  B_var[i1][i2] = (B_var[i1][i2] -				\
			   (A_var[i1][i2] *				\
			    A_var[i1][i2]) / B_var[i1][i2-1]);		\
	}								\
      }									\
      /* Row Sweep */							\
      for (i1=1; i1<N; i1++) {						\
	for (i2 = 0; i2 < N; i2++) {					\
	  X_var[i1][i2] = (X_var[i1][i2] -				\
			   (X_var[i1-1][i2] *				\
			    A_var[i1][i2]) / B_var[i1-1][i2]);		\
	  B_var[i1][i2] = (B_var[i1][i2] -				\
			   (A_var[i1][i2] *				\
			    A_var[i1][i2]) / B_var[i1-1][i2]);		\
	}								\
      }									\
    } /* end of t loop */						\
  }


void initialize()
{
  int i,j;
  for (i = 0; i < NMAX; i++) {
    for (j = 0; j < NMAX; j++) {
      X[i][j] = TX[i][j] = i + j + 100.23;
      A[i][j] = TA[i][j] = i * j * 3.23;
      B[i][j] = TB[i][j] = i * j * (i+j) + 55.5;
    }
  }
}

void initialize_once() {
  initialize();
}

void show(){
  print_submatrix("A", A, N, N, 0, 8, 0, 8);
  print_submatrix("A", A, N, N, N-8, N, N-8, N);
}

#pragma rstream map
void ad(real_t P[NMAX][NMAX], real_t Q[NMAX][NMAX], real_t R[NMAX][NMAX]) {
  adi(P, Q, R)
}

void kernel(){
  ad(A, B, X);
}

int check() {
  adi(TA, TB, TX);
  return check_vectors("A", "TA", A, TA, NMAX);
}

// Number of flops in the kernel
double flops_per_trial() {
  return (double) (mul_flop+add_flop)*4*NMAX*NMAX*TMAX;
}
int nb_samples = NMAX*TMAX;
char const * function_name = "adi";
