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
#define NMAX 2000
#endif
#define N NMAX

static real_t x1[NMAX], x2[NMAX], a[NMAX][NMAX], y_1[NMAX], y_2[NMAX];
static real_t Tx1[NMAX], Tx2[NMAX], Ta[NMAX][NMAX], Ty_1[NMAX], Ty_2[NMAX];

#define mvt(A_var, X1_var, X2_var, Y1_var, Y2_var) {		\
    int i,j;                                                \
    for (i=0; i<N; i++) {                                   \
      for (j=0; j<N; j++) {                                 \
        X1_var[i] = X1_var[i] + A_var[i][j] * Y1_var[j];	\
      }                                                     \
    }                                                       \
    for (i=0; i<N; i++) {                                   \
      for (j=0; j<N; j++) {                                 \
        X2_var[i] = X2_var[i] + A_var[j][i] * Y2_var[j];	\
      }                                                     \
    }                                                       \
}


void initialize() {
  int i,j;

  for (i = 0; i < N; i++) {
    x1[i] = x2[i] = Tx1[i] = Tx2[i] = i*0.5+2.3;
    y_1[i] = y_2[i] = Ty_1[i] = Ty_2[i] = i*0.5+2.3;
    for (j = 0; j < N; j++) {
      a[i][j] = Ta[i][j]  = i*j*CST(0.5)+CST(2.3);
    }
  }
}


void initialize_once() {
  initialize();
}

void show() {
  print_submatrix("a", a, N, N, 0, 8, 0, 8);
  print_submatrix("a", a, N, N, N-8, N, N-8, N);
}

#pragma rstream map
void mv(real_t A[NMAX][NMAX], real_t B[NMAX], real_t C[NMAX], real_t D[NMAX], real_t E[NMAX]) {
  mvt(A, B, C, D, E)
}
void kernel() {
  mv(a, x1, x2, y_1, y_2);
}

int check() {
  mvt(Ta, Tx1, Tx2, Ty_1, Ty_2);

  return check_matrices("a", "Ta", a, Ta, NMAX, NMAX);
}

// Number of flops in the kernel
double flops_per_trial() {
  return (double) (mul_flop+add_flop)*2*NMAX*NMAX;
}
int nb_samples = NMAX*NMAX;
char const * function_name = "mvt";
