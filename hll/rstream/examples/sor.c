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
 *     testing/polyhedral_benchmarks/polybench2.tgz
 */

// OPERA/tile-cc rights

#include "micro_kernels.h"
#include <stdio.h>
#include <math.h>

#define SIZE1 10000
#define SIZE2 10000

static real_t P[SIZE1][SIZE2];
static real_t Q[SIZE1][SIZE2];

#define sor(D) {							\
    int i, j;								\
    for(i=1; i<SIZE1-1; i++) {						\
      for(j=1; j<SIZE2-1; j++) {					\
	D[i][j] = (D[i][j] + D[i][j-1] + D[i][j+1] +			\
		   D[i-1][j] + D[i+1][j]) / 5;				\
      }									\
    }									\
  }

void initialize()
{
  int i, j;
  for(i=0; i<SIZE1; i++){
    for(j=0; j<SIZE2; j++){
      P[i][j] = 0.30 + i*j*0.5;
      Q[i][j] = 0.30 + i*j*0.5;
    }
  }
}

void initialize_once()
{
  initialize();
}

// Pretty-prints the data
void show() {
  print_submatrix("P", P, SIZE1, SIZE2, 0, 8, 0, 8);
  print_submatrix("P", P, SIZE1, SIZE2, SIZE1-8, SIZE1, SIZE2-8, SIZE2);
}

#pragma rstream map
void so(real_t A[SIZE1][SIZE2]) {
  sor(A)
}

void kernel() {
  so(P);
}

int check()
{
  sor(Q);

  return check_matrices("P", "Q", P, Q, SIZE1, SIZE2);
}

// Number of flops in the kernel
double flops_per_trial() {
  return (double) (mul_flop+add_flop)*SIZE1*SIZE2;
}
int nb_samples = SIZE1*SIZE2;
char const * function_name = "sor";
