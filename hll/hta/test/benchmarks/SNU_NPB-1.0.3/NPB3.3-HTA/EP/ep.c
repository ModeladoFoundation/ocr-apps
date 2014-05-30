//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an OpenMP C version of the NPB EP code. This OpenMP  //
//  C version is developed by the Center for Manycore Programming at Seoul //
//  National University and derived from the OpenMP Fortran versions in    //
//  "NPB3.3-OMP" developed by NAS.                                         //
//                                                                         //
//  Permission to use, copy, distribute and modify this software for any   //
//  purpose with or without fee is hereby granted. This software is        //
//  provided "as is" without express or implied warranty.                  //
//                                                                         //
//  Information on NPB 3.3, including the technical report, the original   //
//  specifications, source code, results and information on how to submit  //
//  new results, is available at:                                          //
//                                                                         //
//           http://www.nas.nasa.gov/Software/NPB/                         //
//                                                                         //
//  Send comments or suggestions for this OpenMP C version to              //
//  cmp@aces.snu.ac.kr                                                     //
//                                                                         //
//          Center for Manycore Programming                                //
//          School of Computer Science and Engineering                     //
//          Seoul National University                                      //
//          Seoul 151-744, Korea                                           //
//                                                                         //
//          E-mail:  cmp@aces.snu.ac.kr                                    //
//                                                                         //
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
// Authors: Sangmin Seo, Jungwon Kim, Jun Lee, Jeongho Nah, Gangwon Jo,    //
//          and Jaejin Lee                                                 //
//-------------------------------------------------------------------------//

//---------------------------------------------------------------------
//      program EMBAR
//---------------------------------------------------------------------
//   M is the Log_2 of the number of complex pairs of uniform (0, 1) random
//   numbers.  MK is the Log_2 of the size of each batch of uniform random
//   numbers.  MK can be set for convenience on a given system, since it does
//   not affect the results.
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "type.h"
#include "npbparams.h"
#include "randdp.h"
#include "timers.h"
#include "print_results.h"

#include "HTA.h"
#include "HTA_operations.h"

#define MAX(X,Y)  (((X) > (Y)) ? (X) : (Y))

#define MK        16
#define MM        (M - MK)
#define NN        (1 << MM)
#define NK        (1 << MK)
#define NQ        10
#define EPSILON   1.0e-8
#define A         1220703125.0
#define S         271828183.0

static HTA *x;      // x is an 1D HTA with PROC tiles
static HTA *qq;     // qq is an 1D HTA with PROC tiles
static HTA *sxh;     
static HTA *syh;     
static double q[NQ]; 
static int PROC = 1;

static int np;
static double an;
static logical timers_enabled;

void ep_kernel(HTA *qq_HTA, HTA *x_HTA, HTA *sx_HTA, HTA *sy_HTA) { 
  if (timers_enabled) timer_start(3);
  int k_offset = qq_HTA->rank * np; // np is the number of patches per processor
  double *x = (double *) HTA_get_ptr_raw_data(x_HTA);
  double *qq = (double *) HTA_get_ptr_raw_data(qq_HTA);
  double *sxh = (double *) HTA_get_ptr_raw_data(sx_HTA);
  double *syh = (double *) HTA_get_ptr_raw_data(sy_HTA);
  double sx = 0.0, sy = 0.0;

  for (int k = 0; k < np; k++) {
    int kk = k_offset + k; 
    double t1 = S;
    double t2 = an;
    double t3, t4;

    // Find starting seed t1 for this kk.
    for (int i = 1; i <= 100; i++) {
      int ik = kk / 2;
      if ((2 * ik) != kk) t3 = randlc(&t1, t2);
      if (ik == 0) break;
      t3 = randlc(&t2, t2);
      kk = ik;
    }

    //--------------------------------------------------------------------
    //  Compute uniform pseudorandom numbers.
    //--------------------------------------------------------------------
    if (timers_enabled) timer_start(2);
    vranlc(2 * NK, &t1, A, x);
    if (timers_enabled) timer_stop(2);

    //--------------------------------------------------------------------
    //  Compute Gaussian deviates by acceptance-rejection method and 
    //  tally counts in concentri//square annuli.  This loop is not 
    //  vectorizable. 
    //--------------------------------------------------------------------
    if (timers_enabled) timer_start(1);

    for (int i = 0; i < NK; i++) {
      double x1 = 2.0 * x[2*i] - 1.0;
      double x2 = 2.0 * x[2*i+1] - 1.0;
      t1 = x1 * x1 + x2 * x2;
      if (t1 <= 1.0) {
        t2    = sqrt(-2.0 * log(t1) / t1);
        t3    = (x1 * t2);
        t4    = (x2 * t2);
        int l = MAX(fabs(t3), fabs(t4));
        qq[l] = qq[l] + 1.0;
        sx    = sx + t3;
        sy    = sy + t4;
      }
    }
    if (timers_enabled) timer_stop(1);
  }
  *sxh = sx;
  *syh = sy;
  if (timers_enabled) timer_stop(3);
}

int hta_main(int argc, char *argv[])
{
  double Mops, t1, t2, tt;
  double sx, sy, gc, tm;
  double sx_verify_value, sy_verify_value, sx_err, sy_err;
  int    i, nit;
  int    j;
  logical verified; 

  double dum[3] = {1.0, 1.0, 1.0};
  char   size[16];

  FILE *fp;

  if (argc == 2)
  {
      PROC = atoi(argv[1]);
  }

  if ((fp = fopen("timer.flag", "r")) == NULL) {
    timers_enabled = false;
  } else {
    timers_enabled = true;
    fclose(fp);
  }

  //--------------------------------------------------------------------
  //  Because the size of the problem is too large to store in a 32-bit
  //  integer for some classes, we put it into a string (for printing).
  //  Have to strip off the decimal point put in there by the floating
  //  point print statement (internal file)
  //--------------------------------------------------------------------

  sprintf(size, "%15.0lf", pow(2.0, M+1));
  j = 14;
  if (size[j] == '.') j--;
  size[j+1] = '\0';
  printf("\n\n NAS Parallel Benchmarks (NPB3.3-OMP-C) - EP Benchmark\n");
  printf("\n Number of random numbers generated: %15s\n", size);
  //printf("\n Number of available threads:          %13d\n", omp_get_max_threads());

  verified = false;

  //--------------------------------------------------------------------
  //  Compute the number of "batches" of random number pairs generated 
  //  per processor. Adjust if the number of processors does not evenly 
  //  divide the total number
  //--------------------------------------------------------------------

  np = NN / PROC; 

  //--------------------------------------------------------------------
  //  Call the random number generator functions and initialize
  //  the x-array to reduce the effects of paging on the timings.
  //  Also, call all mathematical functions that are used. Make
  //  sure these initializations cannot be eliminated as dead code.
  //--------------------------------------------------------------------

  vranlc(0, &dum[0], dum[1], &dum[2]);
  dum[0] = randlc(&dum[1], dum[2]);

  // HTA x initializtion
  Tuple tp0 = Tuple_create(2, PROC, 1);
  Tuple fs0 = Tuple_create(2, PROC, 2 * NK);
  Dist dist0;
  Dist_init(&dist0, 0);
  x = HTA_create(2, 2, &fs0, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);

  // HTA qq initialization
  Tuple tp3 = Tuple_create(2, PROC, 1);
  Tuple fs3 = Tuple_create(2, PROC, NQ);
  qq = HTA_create(2, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp3);

  // HTA sxh initialization
  Tuple tp4 = Tuple_create(2, PROC, 1);
  Tuple fs4 = Tuple_create(2, PROC, 1);
  sxh = HTA_create(2, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp4);

  // HTA syh initialization
  syh = HTA_create(2, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp4);

  double initval_x = -1.0e99;
  HTA_map_h1s1(HTA_LEAF_LEVEL(x), H1S1_INIT, x, &initval_x); 


  // init sxh and syh
  double initval_zero = 0.0;
  HTA_map_h1s1(HTA_LEAF_LEVEL(sxh), H1S1_INIT, sxh, &initval_zero); 
  HTA_map_h1s1(HTA_LEAF_LEVEL(syh), H1S1_INIT, syh, &initval_zero); 

  Mops = log(sqrt(fabs(MAX(1.0, 1.0))));   

  // FIXME: timer for HTA version
  //#pragma omp parallel
  //{
  timer_clear(0);
    if (timers_enabled) timer_clear(1);
    if (timers_enabled) timer_clear(2);
    if (timers_enabled) timer_clear(3);
  //}
  timer_start(0);

  // FIXME: not necessary?
  //t1 = A;
  //vranlc(0, &t1, A, x);

  //--------------------------------------------------------------------
  //  Compute AN = A ^ (2 * NK) (mod 2^46).
  //--------------------------------------------------------------------

  t1 = A;

  for (i = 0; i < MK + 1; i++) {
    t2 = randlc(&t1, t1);
  }

  an = t1;
  tt = S;
  gc = 0.0;
  sx = 0.0;
  sy = 0.0;

  for (i = 0; i < NQ; i++) {
    q[i] = 0.0;
  }

  HTA_map_h1s1(HTA_LEAF_LEVEL(qq), H1S1_INIT, qq, &initval_zero); 

  HTA_map_h4(HTA_LEAF_LEVEL(qq), ep_kernel, qq, x, sxh, syh);

  HTA* q_result = HTA_partial_reduce(REDUCE_SUM, qq, 0, &initval_zero);
  HTA_to_array(q_result, q);

  HTA_full_reduce(REDUCE_SUM, &sx, sxh);
  HTA_full_reduce(REDUCE_SUM, &sy, syh);
  
  for (i = 0; i < NQ; i++) {
    gc = gc + q[i];
  }

  timer_stop(0);
  tm = timer_read(0);
  nit = 0;
  verified = true;
  if (M == 24) {
    sx_verify_value = -3.247834652034740e+3;
    sy_verify_value = -6.958407078382297e+3;
  } else if (M == 25) {
    sx_verify_value = -2.863319731645753e+3;
    sy_verify_value = -6.320053679109499e+3;
  } else if (M == 28) {
    sx_verify_value = -4.295875165629892e+3;
    sy_verify_value = -1.580732573678431e+4;
  } else if (M == 30) {
    sx_verify_value =  4.033815542441498e+4;
    sy_verify_value = -2.660669192809235e+4;
  } else if (M == 32) {
    sx_verify_value =  4.764367927995374e+4;
    sy_verify_value = -8.084072988043731e+4;
  } else if (M == 36) {
    sx_verify_value =  1.982481200946593e+5;
    sy_verify_value = -1.020596636361769e+5;
  } else if (M == 40) {
    sx_verify_value = -5.319717441530e+05;
    sy_verify_value = -3.688834557731e+05;
  } else {
    verified = false;
  }

  if (verified) {
    sx_err = fabs((sx - sx_verify_value) / sx_verify_value);
    sy_err = fabs((sy - sy_verify_value) / sy_verify_value);
    verified = ((sx_err <= EPSILON) && (sy_err <= EPSILON));
  }

  Mops = pow(2.0, M+1) / tm / 1000000.0;

  printf("\nEP Benchmark Results:\n\n");
  printf("CPU Time =%10.4lf\n", tm);
  printf("N = 2^%5d\n", M);
  printf("No. Gaussian Pairs = %15.0lf\n", gc);
  printf("Sums = %25.15lE %25.15lE\n", sx, sy);
  printf("Counts: \n");
  for (i = 0; i < NQ; i++) {
    printf("%3d%15.0lf\n", i, q[i]);
  }

  print_results("EP", CLASS, M+1, 0, 0, nit,
      tm, Mops, 
      "Random numbers generated",
      verified, NPBVERSION, COMPILETIME, CS1,
      CS2, CS3, CS4, CS5, CS6, CS7);

  if (timers_enabled) {
    char rec_name[256];
    sprintf(rec_name, "rec/ep.%c.%d.rec", CLASS, PROC);
    FILE* fp_rec = fopen(rec_name, "a");
    if (tm <= 0.0) tm = 1.0;
    tt = timer_read(0);
    printf("\nTotal time:     %9.3lf (%6.2lf)\n", tt, tt*100.0/tm);
    fprintf(fp_rec, "%9.3lf ", tt);
    tt = timer_read(1);
    printf("Gaussian pairs: %9.3lf (%6.2lf)\n", tt, tt*100.0/tm);
    fprintf(fp_rec, "%9.3lf ", tt);
    tt = timer_read(2);
    printf("Random numbers: %9.3lf (%6.2lf)\n", tt, tt*100.0/tm);
    fprintf(fp_rec, "%9.3lf ", tt);
    tt = timer_read(3);
    printf("Kernel exec:    %9.3lf (%6.2lf)\n", tt, tt*100.0/tm);
    fprintf(fp_rec, "%9.3lf\n", tt);
    fclose(fp_rec);
  }

  HTA_destroy(x);
  HTA_destroy(qq);
  HTA_destroy(q_result);
  HTA_destroy(sxh);
  HTA_destroy(syh);
  assert(verified);
  return (verified)?0:-1;
}

