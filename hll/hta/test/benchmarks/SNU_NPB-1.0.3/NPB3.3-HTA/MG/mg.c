//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an OpenMP C version of the NPB MG code. This OpenMP  //
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
// program mg
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#ifdef _OPENMP
//#include <omp.h>
//#endif

#include "globals.h"
#include "randdp.h"
#include "timers.h"
#include "print_results.h"
#include "HTA.h"
#include "HTA_operations.h"

static int PROC = 1;
static double zero = 0.0;
static void setup_HTA(int nprocs);
static void mg3P();
static void psinv_HTA(HTA* r, HTA* u, int k);
static void psinv_on_leaves(HTA* r_tile, HTA* u_tile);
static void resid_HTA(HTA* u_tile, HTA* v_tile, HTA* r_tile, int k);
static void resid_on_leaves(HTA* u_tile, HTA* v_tile, HTA* r_tile);
static void rprj3_HTA(HTA* r_HTA, HTA* s_HTA, int k);
static void rprj3_on_leaves(HTA* s_tile, HTA* r_tile);
static void interp_HTA(HTA* z_HTA, HTA* u_HTA, int k);
static void interp_on_leaves(HTA* z_tile, HTA* u_tile);
static void norm2u3_HTA(HTA* z,
                    double *rnm2, double *rnmu,
                    int nx, int ny, int nz, int k);
static void rep_nrm(HTA* u_HTA, char *title, int kk);
static void comm3_HTA(HTA* h);
static void showall(HTA* z_HTA);
static double power(double a, int n);
static void bubble(double ten[][2], int j1[][2], int j2[][2], int j3[][2],
                   int m, int ind);
static void zero3(void *oz, int n1, int n2, int n3);
static void zran3(void *oz, int nx1, int ny1, int nz1);


//-------------------------------------------------------------------------c
// These arrays are in common because they are quite large
// and probably shouldn't be allocated on the stack. They
// are always passed as subroutine args. 
//-------------------------------------------------------------------------c
/* commcon /noautom/ */
//static double u[NR];
static double v_array[NR];
//static double r[NR];
static HTA* u_HTA[MAXLEVEL];
static HTA* v_HTA;
//static HTA* tmp_HTA[MAXLEVEL];
static HTA* r_HTA[MAXLEVEL];

/* common /grid/ */
//static int is1, is2, is3, ie1, ie2, ie3;

/* common /rans_save/ starts */
double starts[NM];


static double a[4], c[4];
int hta_main(int argc, char *argv[])
{
  //-------------------------------------------------------------------------c
  // k is the current level. It is passed down through subroutine args
  // and is NOT global. it is the current iteration
  //-------------------------------------------------------------------------c
  int k, it;
  double t, tinit, mflops;


  double rnm2, rnmu, epsilon ;
  int nit;
  double nn, verify_value, err;
  logical verified;

  int i;
  char *t_names[T_last];
  double tmax;

  for (i = T_init; i < T_last; i++) {
    timer_clear(i);
  }
  if (argc == 2)
  {
      PROC = atoi(argv[1]);
  }

  timer_start(T_init);

  //---------------------------------------------------------------------
  // Read in and broadcast input data
  //---------------------------------------------------------------------
  FILE *fp;
  if ((fp = fopen("timer.flag", "r")) != NULL) {
    timeron = true;
    t_names[T_init] = "init";
    t_names[T_bench] = "benchmk";
    t_names[T_mg3P] = "mg3P";
    t_names[T_psinv] = "psinv";
    t_names[T_resid] = "resid";
    t_names[T_rprj3] = "rprj3";
    t_names[T_interp] = "interp";
    t_names[T_norm2] = "norm2";
    t_names[T_comm3] = "comm3";
    fclose(fp);
  } else {
    timeron = false;
  }

  printf("\n\n NAS Parallel Benchmarks (NPB3.3-OMP-C) - MG Benchmark\n\n");

  if ((fp = fopen("mg.input", "r")) != NULL) {
    int result;
    printf(" Reading from input file mg.input\n");
    result = fscanf(fp, "%d\n", &lt);
    while (fgetc(fp) != '\n');
    result = fscanf(fp, "%d%d%d", &nx[lt], &ny[lt], &nz[lt]);
    while (fgetc(fp) != '\n');
    result = fscanf(fp, "%d", &nit);
    while (fgetc(fp) != '\n');
    for (i = 0; i <= 7; i++) {
      result = fscanf(fp, "%d", &debug_vec[i]);
    }
    fclose(fp);
  } else {
    printf(" No input file. Using compiled defaults \n");
    lt = LT_DEFAULT;
    nit = NIT_DEFAULT;
    nx[lt] = NX_DEFAULT;
    ny[lt] = NY_DEFAULT;
    nz[lt] = NZ_DEFAULT;
    for (i = 0; i <= 7; i++) {
      debug_vec[i] = DEBUG_DEFAULT;
    }
  }

  if ( (nx[lt] != ny[lt]) || (nx[lt] != nz[lt]) ) {
    Class = 'U';
  } else if ( nx[lt] == 32 && nit == 4 ) {
    Class = 'S';
  } else if ( nx[lt] == 128 && nit == 4 ) {
    Class = 'W';
  } else if ( nx[lt] == 256 && nit == 4 ) {  
    Class = 'A';
  } else if ( nx[lt] == 256 && nit == 20 ) {
    Class = 'B';
  } else if ( nx[lt] == 512 && nit == 20 ) {  
    Class = 'C';
  } else if ( nx[lt] == 1024 && nit == 50 ) {  
    Class = 'D';
  } else if ( nx[lt] == 2048 && nit == 50 ) {  
    Class = 'E';
  } else {
    Class = 'U';
  }

  //---------------------------------------------------------------------
  // Use these for debug info:
  //---------------------------------------------------------------------
  //    debug_vec(0) = 1 !=> report all norms
  //    debug_vec(1) = 1 !=> some setup information
  //    debug_vec(1) = 2 !=> more setup information
  //    debug_vec(2) = k => at level k or below, show result of resid
  //    debug_vec(3) = k => at level k or below, show result of psinv
  //    debug_vec(4) = k => at level k or below, show result of rprj
  //    debug_vec(5) = k => at level k or below, show result of interp
  //    debug_vec(6) = 1 => (unused)
  //    debug_vec(7) = 1 => (unused)
  //---------------------------------------------------------------------
  a[0] = -8.0/3.0;
  a[1] =  0.0;
  a[2] =  1.0/6.0;
  a[3] =  1.0/12.0;
      
  if (Class == 'A' || Class == 'S' || Class =='W') {
    //---------------------------------------------------------------------
    // Coefficients for the S(a) smoother
    //---------------------------------------------------------------------
    c[0] =  -3.0/8.0;
    c[1] =  +1.0/32.0;
    c[2] =  -1.0/64.0;
    c[3] =   0.0;
  } else {
    //---------------------------------------------------------------------
    // Coefficients for the S(b) smoother
    //---------------------------------------------------------------------
    c[0] =  -3.0/17.0;
    c[1] =  +1.0/33.0;
    c[2] =  -1.0/61.0;
    c[3] =   0.0;
  }
  lb = 1;
  k  = lt;

  setup_HTA(PROC);
  zran3(v_array, nx[lt], ny[lt], nz[lt]);
  norm2u3_HTA(v_HTA, &rnm2, &rnmu, nx[lt], ny[lt], nz[lt], lt);

  printf(" Size: %4dx%4dx%4d  (class %c)\n", nx[lt], ny[lt], nz[lt], Class);
  printf(" Iterations:                  %5d\n", nit);
  printf(" Number of available threads: %5d\n", omp_get_max_threads());
  printf("\n");

  resid_HTA(u_HTA[lt], v_HTA, r_HTA[lt], k);
  norm2u3_HTA(r_HTA[lt], &rnm2, &rnmu, nx[lt], ny[lt], nz[lt], lt);
  //old2 = rnm2;
  //oldu = rnmu;

  //---------------------------------------------------------------------
  // One iteration for startup
  //---------------------------------------------------------------------
  mg3P();
  resid_HTA(u_HTA[lt], v_HTA, r_HTA[lt], k);
  // FIXME: release the memory on heap allocated for HTAs
  setup_HTA(PROC);
  zran3(v_array, nx[lt], ny[lt], nz[lt]);

  timer_stop(T_init);
  tinit = timer_read(T_init);

  printf(" Initialization time: %15.3f seconds\n\n", tinit);

  for (i = T_bench; i < T_last; i++) {
    timer_clear(i);
  }

  timer_start(T_bench);

  if (timeron) timer_start(T_resid2);
  resid_HTA(u_HTA[lt], v_HTA, r_HTA[lt], k);
  if (timeron) timer_stop(T_resid2);
  norm2u3_HTA(r_HTA[lt], &rnm2, &rnmu, nx[lt], ny[lt], nz[lt], lt);
  //old2 = rnm2;
  //oldu = rnmu;

  for (it = 1; it <= nit; it++) {
    if ((it == 1) || (it == nit) || ((it % 5) == 0)) {
      printf("  iter %3d\n", it);
    }
    if (timeron) timer_start(T_mg3P);
    mg3P();
    if (timeron) timer_stop(T_mg3P);
    if (timeron) timer_start(T_resid2);
    resid_HTA(u_HTA[lt], v_HTA, r_HTA[lt], k);
    if (timeron) timer_stop(T_resid2);
  }

  norm2u3_HTA(r_HTA[lt], &rnm2, &rnmu, nx[lt], ny[lt], nz[lt], lt);

  timer_stop(T_bench);

  t = timer_read(T_bench);

  verified = false;
  verify_value = 0.0;

  printf("\n Benchmark completed\n");

  epsilon = 1.0e-8;
  if (Class != 'U') {
    if (Class == 'S') {
      verify_value = 0.5307707005734e-04;
    } else if (Class == 'W') {
      verify_value = 0.6467329375339e-05;
    } else if (Class == 'A') {
      verify_value = 0.2433365309069e-05;
    } else if (Class == 'B') {
      verify_value = 0.1800564401355e-05;
    } else if (Class == 'C') {
      verify_value = 0.5706732285740e-06;
    } else if (Class == 'D') {
      verify_value = 0.1583275060440e-09;
    } else if (Class == 'E') {
      verify_value = 0.5630442584711e-10;
    }

    err = fabs( rnm2 - verify_value ) / verify_value;
    if (err <= epsilon) {
      verified = true;
      printf(" VERIFICATION SUCCESSFUL\n");
      printf(" L2 Norm is %20.13E\n", rnm2);
      printf(" Error is   %20.13E\n", err);
    } else {
      verified = false;
      printf(" VERIFICATION FAILED\n");
      printf(" L2 Norm is             %20.13E\n", rnm2);
      printf(" The correct L2 Norm is %20.13E\n", verify_value);
    }
  } else {
    verified = false;
    printf(" Problem size unknown\n");
    printf(" NO VERIFICATION PERFORMED\n");
    printf(" L2 Norm is %20.13E\n", rnm2);
  }

  nn = 1.0 * nx[lt] * ny[lt] * nz[lt];

  if (t != 0.0) {
    mflops = 58.0 * nit * nn * 1.0e-6 / t;
  } else {
    mflops = 0.0;
  }

  print_results("MG", Class, nx[lt], ny[lt], nz[lt], 
                nit, t,
                mflops, "          floating point", 
                verified, NPBVERSION, COMPILETIME,
                CS1, CS2, CS3, CS4, CS5, CS6, CS7);

  //---------------------------------------------------------------------
  // More timers
  //---------------------------------------------------------------------
  if (timeron) {
    tmax = timer_read(T_bench);
    if (tmax == 0.0) tmax = 1.0;
    char rec_name[256];
    sprintf(rec_name, "rec/mg.%c.%d.rec", Class, PROC);
    FILE* fp_rec = fopen(rec_name, "a");
    fprintf(fp_rec, "%9.4f ", (verified)?tmax:-1.0);

    printf("  SECTION   Time (secs)\n");
    for (i = T_bench; i < T_last; i++) {
      t = timer_read(i);
      fprintf(fp_rec, "%9.4f ", t);
      if (i == T_resid2) {
        t = timer_read(T_resid) - t;
        printf("    --> %8s:%9.3f  (%6.2f%%)\n", "mg-resid", t, t*100./tmax);
      } else {
        printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[i], t, t*100./tmax);
      }
    }
    fprintf(fp_rec, "\n");
    fclose(fp_rec);
  }

  return 0;
}
 

static int pi[3]; // number of processors in each dimension
void setup_HTA (int nprocs)
{    
  int di[3];
  int ov[3];
  int ng[MAXLEVEL+1][3];
  int k, ax;
  double log_p  = log((double)(nprocs)+0.0001)/log(2.0);
  
  // calculate the number of processors in each dimension
  int dx     = (int)(log_p/3);
  pi[0] = (int) pow(2.0, dx);
  int dy     = (int)((log_p-dx)/2);
  pi[1]  = (int) pow (2.0, dy);
  pi[2]  = nprocs/(pi[0]*pi[1]);
  
  ng[lt][0] = nx[lt];
  ng[lt][1] = ny[lt];
  ng[lt][2] = nz[lt];
  
  // compute grid sizes for each level
  for (k = lt-1; k >= 1; k--) {
    for (ax = 0; ax < 3; ax++) {
      ng[k][ax] = ng[k+1][ax]/2;
    }
  }
  
  for (k = lt; k >= 1; k--) {
    nx[k] = ng[k][0];
    ny[k] = ng[k][1];
    nz[k] = ng[k][2];
  }

  // allocate HTAs for grids at each level
  Dist dist0;
  Dist_init(&dist0, 0);
  for (int k = lt; k >=1; k--)
  {
    // Check if the size is too small for distributed partition
    // Use one partition if it is too small
    if(ng[k][2] / pi[2] == 0 || ng[k][1] / pi[1] == 0 || ng[k][0] / pi[0] == 0)
    {
        for(int i = 0; i < 3; i++)
        {
            ov[i] = 2;
            di[i] = 1; // set PEs to 1
        }
    }
    else
    {
        for(int i = 0; i < 3; i++)
        {
            di[i] = pi[i]; 
            ov[i] = di[i] * 2;
        }
    }
    Tuple t0 = Tuple_create(3, di[2], di[1], di[0]); // PEs
    Tuple fs = Tuple_create(3, ng[k][2] + ov[2], ng[k][1] + ov[1], ng[k][0] + ov[0]); // includes overlapped region

    r_HTA[k] = HTA_create(3, 2, &fs, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);
    u_HTA[k] = HTA_create(3, 2, &fs, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);
    //tmp_HTA[k] = HTA_create(3, 2, &fs, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);

    // there is only one (the finest) grid for v_HTA
    if(k == lt) {
        v_HTA = HTA_create(3, 2, &fs, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);
        HTA_map_h1s1(HTA_LEAF_LEVEL(v_HTA), H1S1_INIT, v_HTA, &zero);
    }

    //printf("Allocating grids at level k = %d\n", k);
    //printf("Distribution of PEs:");
    //Tuple_print(t0, 0);
    //printf("Size of each dimension: ");
    //printf("x = %d, y = %d, z = %d\n", ng[k][0], ng[k][1], ng[k][2]);
    //Tuple_print(fs, 0);
    //printf("Size of leaves: ");
    //Tuple_print(r_HTA[k]->tiles[0]->flat_size, 0);
      
    HTA_map_h1s1(HTA_LEAF_LEVEL(r_HTA[k]), H1S1_INIT, r_HTA[k], &zero);
    HTA_map_h1s1(HTA_LEAF_LEVEL(u_HTA[k]), H1S1_INIT, u_HTA[k], &zero);
  }
}

//---------------------------------------------------------------------
// multigrid V-cycle routine
//---------------------------------------------------------------------
//static void mg3P(double u[], double v[], double r[],
//                 double a[4], double c[4], int n1, int n2, int n3)
static void mg3P()
{
  int j, k;

  //---------------------------------------------------------------------
  // down cycle.
  // restrict the residual from the find grid to the coarse
  //---------------------------------------------------------------------
  for (k = lt; k >= lb+1; k--) {
    j = k - 1;
    rprj3_HTA(r_HTA[k], r_HTA[j], k);
  }
  
  k = lb;
  //---------------------------------------------------------------------
  // compute an approximate solution on the coarsest grid
  //---------------------------------------------------------------------
  //
  // Works with residual from this point on
  //
  HTA_map_h1s1(HTA_LEAF_LEVEL(u_HTA[k]), H1S1_INIT, u_HTA[k], &zero);
  psinv_HTA(r_HTA[k], u_HTA[k], k); // residual of the coarsest level stored in u

  for (k = lb+1; k <= lt-1; k++) {
    j = k - 1;

    //---------------------------------------------------------------------
    // prolongate from level k-1  to k
    //---------------------------------------------------------------------
    HTA_map_h1s1(HTA_LEAF_LEVEL(u_HTA[k]), H1S1_INIT, u_HTA[k], &zero);
    interp_HTA(u_HTA[j], u_HTA[k], k);

    //---------------------------------------------------------------------
    // compute residual for level k
    //---------------------------------------------------------------------
    resid_HTA(u_HTA[k], r_HTA[k], r_HTA[k], k);

    //---------------------------------------------------------------------
    // apply smoother
    //---------------------------------------------------------------------
    psinv_HTA(r_HTA[k], u_HTA[k], k);
  }

  //
  // At this point, get the residual for the finest grid
  //
  
  j = lt - 1;
  k = lt;
  interp_HTA(u_HTA[j], u_HTA[k], k);
  resid_HTA(u_HTA[k], v_HTA, r_HTA[k], k);
  psinv_HTA(r_HTA[k], u_HTA[k], k);
}


//---------------------------------------------------------------------
// psinv applies an approximate inverse as smoother:  u = u + Cr
//
// This  implementation costs  15A + 4M per result, where
// A and M denote the costs of Addition and Multiplication.  
// Presuming coefficient c(3) is zero (the NPB assumes this,
// but it is thus not a general case), 2A + 1M may be eliminated,
// resulting in 13A + 3M.
// Note that this vectorizes, and is also fine for cache 
// based machines.  
//---------------------------------------------------------------------
static void psinv_on_leaves(HTA* r_tile, HTA* u_tile)
{
  int n3 = r_tile->flat_size.values[0];
  int n2 = r_tile->flat_size.values[1];
  int n1 = r_tile->flat_size.values[2];
  double (*r)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(r_tile);
  double (*u)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(u_tile);

  int i3, i2, i1;

  double r1[M], r2[M];
  for (i3 = 1; i3 < n3-1; i3++) {
    for (i2 = 1; i2 < n2-1; i2++) {
      for (i1 = 0; i1 < n1; i1++) {
        r1[i1] = r[i3][i2-1][i1] + r[i3][i2+1][i1]
               + r[i3-1][i2][i1] + r[i3+1][i2][i1];
        r2[i1] = r[i3-1][i2-1][i1] + r[i3-1][i2+1][i1]
               + r[i3+1][i2-1][i1] + r[i3+1][i2+1][i1];
      }
      for (i1 = 1; i1 < n1-1; i1++) {
        u[i3][i2][i1] = u[i3][i2][i1]
                      + c[0] * r[i3][i2][i1]
                      + c[1] * ( r[i3][i2][i1-1] + r[i3][i2][i1+1]
                               + r1[i1] )
                      + c[2] * ( r2[i1] + r1[i1-1] + r1[i1+1] );
        //--------------------------------------------------------------------
        // Assume c[3] = 0    (Enable line below if c[3] not= 0)
        //--------------------------------------------------------------------
        //            + c[3] * ( r2[i1-1] + r2[i1+1] )
        //--------------------------------------------------------------------
      }
    }
  }
}

static void psinv_HTA(HTA* r, HTA* u, int k)
{
  if (timeron) timer_start(T_psinv);
  HTA_map_h2(HTA_LEAF_LEVEL(r), psinv_on_leaves, r, u);
  if (timeron) timer_stop(T_psinv);

  comm3_HTA(u);

  if (debug_vec[0] >= 1) {
    rep_nrm(u, "   psinv", k);
  }

  if (debug_vec[3] >= k) {
    showall(u);
  }
}

//---------------------------------------------------------------------
// resid computes the residual:  r = v - Au
//
// This  implementation costs  15A + 4M per result, where
// A and M denote the costs of Addition (or Subtraction) and 
// Multiplication, respectively. 
// Presuming coefficient a(1) is zero (the NPB assumes this,
// but it is thus not a general case), 3A + 1M may be eliminated,
// resulting in 12A + 3M.
// Note that this vectorizes, and is also fine for cache 
// based machines.  
//---------------------------------------------------------------------
static void resid_HTA(HTA* u_HTA, HTA* v_HTA, HTA* r_HTA, int k)
{
  if (timeron) timer_start(T_resid);
  HTA_map_h3(HTA_LEAF_LEVEL(u_HTA), resid_on_leaves, u_HTA, v_HTA, r_HTA);
  if (timeron) timer_stop(T_resid);

  //---------------------------------------------------------------------
  // exchange boundary data 
  //---------------------------------------------------------------------
  comm3_HTA(r_HTA);
  if (debug_vec[0] >= 1) {
    rep_nrm(r_HTA, "   resid", k);
  }

  if (debug_vec[2] >= k) {
    showall(r_HTA);
  }
}

static void resid_on_leaves(HTA* u_tile, HTA* v_tile, HTA* r_tile)
{
  int n1 = u_tile->flat_size.values[2];
  int n2 = u_tile->flat_size.values[1];
  int n3 = u_tile->flat_size.values[0];

  double (*u)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(u_tile);
  double (*v)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(v_tile);
  double (*r)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(r_tile);

  int i3, i2, i1;
  double u1[M], u2[M];

  for (i3 = 1; i3 < n3-1; i3++) {
    for (i2 = 1; i2 < n2-1; i2++) {
      for (i1 = 0; i1 < n1; i1++) {
        u1[i1] = u[i3][i2-1][i1] + u[i3][i2+1][i1]
               + u[i3-1][i2][i1] + u[i3+1][i2][i1];
        u2[i1] = u[i3-1][i2-1][i1] + u[i3-1][i2+1][i1]
               + u[i3+1][i2-1][i1] + u[i3+1][i2+1][i1];
      }
      for (i1 = 1; i1 < n1-1; i1++) {
        r[i3][i2][i1] = v[i3][i2][i1]
                      - a[0] * u[i3][i2][i1]
        //-------------------------------------------------------------------
        //  Assume a[1] = 0      (Enable 2 lines below if a[1] not= 0)
        //-------------------------------------------------------------------
        //            - a[1] * ( u[i3][i2][i1-1] + u[i3][i2][i1+1]
        //                     + u1[i1] )
        //-------------------------------------------------------------------
                      - a[2] * ( u2[i1] + u1[i1-1] + u1[i1+1] )
                      - a[3] * ( u2[i1-1] + u2[i1+1] );
      }
    }
  }
}


//---------------------------------------------------------------------
// rprj3 projects onto the next coarser grid, 
// using a trilinear Finite Element projection:  s = r' = P r
//     
// This  implementation costs  20A + 4M per result, where
// A and M denote the costs of Addition and Multiplication.  
// Note that this vectorizes, and is also fine for cache 
// based machines.  
//---------------------------------------------------------------------
static void rprj3_on_leaves(HTA* s_tile, HTA* r_tile) // s is the coarser one
{
    int j3, j2, j1, i3, i2, i1, d1, d2, d3;
    double x1[M], y1[M], x2, y2;

    int m3k = r_tile->flat_size.values[0];
    int m2k = r_tile->flat_size.values[1];
    int m1k = r_tile->flat_size.values[2];
    int m3j = s_tile->flat_size.values[0];
    int m2j = s_tile->flat_size.values[1];
    int m1j = s_tile->flat_size.values[2];

    double (*r)[m2k][m1k] = (double (*)[m2k][m1k])HTA_get_ptr_raw_data(r_tile);
    double (*s)[m2j][m1j] = (double (*)[m2j][m1j])HTA_get_ptr_raw_data(s_tile);
    if (m1k == 3) {
      d1 = 2;
    } else {
      d1 = 1;
    }

    if (m2k == 3) {
      d2 = 2;
    } else {
      d2 = 1;
    }

    if (m3k == 3) {
      d3 = 2;
    } else {
      d3 = 1;
    }

    for (j3 = 1; j3 < m3j-1; j3++) {
      i3 = 2*j3-d3;
      for (j2 = 1; j2 < m2j-1; j2++) {
        i2 = 2*j2-d2;

        for (j1 = 1; j1 < m1j; j1++) {
          i1 = 2*j1-d1;
          x1[i1] = r[i3+1][i2  ][i1] + r[i3+1][i2+2][i1]
                 + r[i3  ][i2+1][i1] + r[i3+2][i2+1][i1];
          y1[i1] = r[i3  ][i2  ][i1] + r[i3+2][i2  ][i1]
                 + r[i3  ][i2+2][i1] + r[i3+2][i2+2][i1];
        }

        for (j1 = 1; j1 < m1j-1; j1++) {
          i1 = 2*j1-d1;
          y2 = r[i3  ][i2  ][i1+1] + r[i3+2][i2  ][i1+1]
             + r[i3  ][i2+2][i1+1] + r[i3+2][i2+2][i1+1];
          x2 = r[i3+1][i2  ][i1+1] + r[i3+1][i2+2][i1+1]
             + r[i3  ][i2+1][i1+1] + r[i3+2][i2+1][i1+1];
          s[j3][j2][j1] =
                  0.5 * r[i3+1][i2+1][i1+1]
                + 0.25 * (r[i3+1][i2+1][i1] + r[i3+1][i2+1][i1+2] + x2)
                + 0.125 * (x1[i1] + x1[i1+2] + y2)
                + 0.0625 * (y1[i1] + y1[i1+2]);
        }
      }
    }
}

// Remap boundary accesses to the real location directly
#define BOUNDARY_CHECK(i, m) ((i == 0) ? m-2 : ((i == m-1) ? 0 : i - 1))

#define GET_VALUE_AT(val, i3, i2, i1) \
      { \
          c.values[0] = BOUNDARY_CHECK(i3, m3k); \
          c.values[1] = BOUNDARY_CHECK(i2, m2k); \
          c.values[2] = BOUNDARY_CHECK(i1, m1k); \
          HTA_nd_global_to_tile_index(&flat_size, nd_num_tiles, &c, &tile_nd_index, &tile_local_index); \
          tile_local_index.values[0] += 1; \
          tile_local_index.values[1] += 1; \
          tile_local_index.values[2] += 1; \
          val = *(double*)HTA_access_element(r_HTA, acc); \
      }

//          printf("tile dimension: "); \
//          Tuple_print(nd_num_tiles, 0); \
//          printf("global element index: "); \
//          Tuple_print(c, 0); \
//          printf("tile nd index: "); \
//          Tuple_print(tile_nd_index, 0); \

static void rprj3_collective(HTA* s_tile, HTA* r_HTA) // s is the coarser one
{
    int j3, j2, j1, i3, i2, i1, d1, d2, d3;
    double x1[M], y1[M], x2, y2;

    //printf("rprj3_collective starts\n");

    int m3j = s_tile->flat_size.values[0];
    int m2j = s_tile->flat_size.values[1];
    int m1j = s_tile->flat_size.values[2];
    int m3k = (m3j - 2) * 2 + 2;
    int m2k = (m2j - 2) * 2 + 2;
    int m1k = (m1j - 2) * 2 + 2;

    //double (*r)[m2k][m1k] = (double (*)[m2k][m1k])HTA_get_ptr_raw_data(r_tile);
    double (*s)[m2j][m1j] = (double (*)[m2j][m1j])HTA_get_ptr_raw_data(s_tile);
    if (m1k == 3) {
      d1 = 2;
    } else {
      d1 = 1;
    }

    if (m2k == 3) {
      d2 = 2;
    } else {
      d2 = 1;
    }

    if (m3k == 3) {
      d3 = 2;
    } else {
      d3 = 1;
    }

    Tuple flat_size = Tuple_create(3, m3k-2, m2k-2, m1k-2);
    Tuple* nd_num_tiles = r_HTA->tiling;
    Tuple tile_nd_index;
    Tuple_init_zero(&tile_nd_index, 3);
    Tuple tile_local_index;
    Tuple_init_zero(&tile_local_index, 3);
    Tuple *acc[2] = {&tile_nd_index, &tile_local_index};
    Tuple c;
    Tuple_init_zero(&c, 3);
    double val[4];

    for (j3 = 1; j3 < m3j-1; j3++) {
      i3 = 2*j3-d3;
      for (j2 = 1; j2 < m2j-1; j2++) {
        i2 = 2*j2-d2;
        for (j1 = 1; j1 < m1j; j1++) {
          i1 = 2*j1-d1;

          //x1[i1] = r[i3+1][i2  ][i1] + r[i3+1][i2+2][i1]
          //       + r[i3  ][i2+1][i1] + r[i3+2][i2+1][i1];
          GET_VALUE_AT(val[0], i3+1, i2, i1);
          GET_VALUE_AT(val[1], i3+1, i2+2, i1);
          GET_VALUE_AT(val[2], i3, i2+1, i1);
          GET_VALUE_AT(val[3], i3+2, i2+1, i1);
          x1[i1] = val[0] + val[1] + val[2] + val[3];

          //y1[i1] = r[i3  ][i2  ][i1] + r[i3+2][i2  ][i1]
          //       + r[i3  ][i2+2][i1] + r[i3+2][i2+2][i1];
          GET_VALUE_AT(val[0], i3, i2, i1);
          GET_VALUE_AT(val[1], i3+2, i2, i1);
          GET_VALUE_AT(val[2], i3, i2+2, i1);
          GET_VALUE_AT(val[3], i3+2, i2+2, i1);
          y1[i1] = val[0] + val[1] + val[2] + val[3];
        }

        for (j1 = 1; j1 < m1j-1; j1++) {
          i1 = 2*j1-d1;
          //y2 = r[i3  ][i2  ][i1+1] + r[i3+2][i2  ][i1+1]
          //   + r[i3  ][i2+2][i1+1] + r[i3+2][i2+2][i1+1];
          GET_VALUE_AT(val[0], i3, i2, i1+1);
          GET_VALUE_AT(val[1], i3+2, i2, i1+1);
          GET_VALUE_AT(val[2], i3, i2+2, i1+1);
          GET_VALUE_AT(val[3], i3+2, i2+2, i1+1);
          y2 = val[0] + val[1] + val[2] + val[3];
          //x2 = r[i3+1][i2  ][i1+1] + r[i3+1][i2+2][i1+1]
          //   + r[i3  ][i2+1][i1+1] + r[i3+2][i2+1][i1+1];
          GET_VALUE_AT(val[0], i3+1, i2, i1+1);
          GET_VALUE_AT(val[1], i3+1, i2+2, i1+1);
          GET_VALUE_AT(val[2], i3, i2+1, i1+1);
          GET_VALUE_AT(val[3], i3+2, i2+1, i1+1);
          x2 = val[0] + val[1] + val[2] + val[3];
          GET_VALUE_AT(val[0], i3+1, i2+1, i1+1);
          GET_VALUE_AT(val[1], i3+1, i2+1, i1);
          GET_VALUE_AT(val[2], i3+1, i2+1, i1+2);
          s[j3][j2][j1] =
               //   0.5 * r[i3+1][i2+1][i1+1]
                  0.5 * val[0]
               // + 0.25 * (r[i3+1][i2+1][i1] + r[i3+1][i2+1][i1+2] + x2)
                + 0.25 * (val[1] + val[2] + x2)
                + 0.125 * (x1[i1] + x1[i1+2] + y2)
                + 0.0625 * (y1[i1] + y1[i1+2]);
        }
      }
    }
}
static void rprj3_HTA(HTA* r_HTA, HTA* s_HTA, int k)
{
  if (timeron) timer_start(T_rprj3);

  if(Tuple_product(r_HTA->tiling) == Tuple_product(s_HTA->tiling))
      HTA_map_h2(HTA_LEAF_LEVEL(r_HTA), rprj3_on_leaves, s_HTA, r_HTA);
  else
      rprj3_collective(s_HTA->tiles[0], r_HTA); // TODO: performance can be improved if done in parallel
  if (timeron) timer_stop(T_rprj3);

  comm3_HTA(s_HTA);

  if (debug_vec[0] >= 1) {
    rep_nrm(s_HTA, "   rprj3", k-1);
  }

  if (debug_vec[4] >= k) {
    showall(s_HTA);
  }
}


//---------------------------------------------------------------------
// interp adds the trilinear interpolation of the correction
// from the coarser grid to the current approximation:  u = u + Qu'
//     
// Observe that this  implementation costs  16A + 4M, where
// A and M denote the costs of Addition and Multiplication.  
// Note that this vectorizes, and is also fine for cache 
// based machines.  Vector machines may get slightly better 
// performance however, with 8 separate "do i1" loops, rather than 4.
//---------------------------------------------------------------------
static void interp_on_leaves(HTA* z_tile, HTA* u_tile)
{
  int mm3 = z_tile->flat_size.values[0];
  int mm2 = z_tile->flat_size.values[1];
  int mm1 = z_tile->flat_size.values[2];
  int n3 = u_tile->flat_size.values[0];
  int n2 = u_tile->flat_size.values[1];
  int n1 = u_tile->flat_size.values[2];
  double (*z)[mm2][mm1] = (double (*)[mm2][mm1])HTA_get_ptr_raw_data(z_tile);
  double (*u)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(u_tile);

  int i3, i2, i1, d1, d2, d3, t1, t2, t3;

  // note that m = 1037 in globals.h but for this only need to be
  // 535 to handle up to 1024^3
  //      integer m
  //      parameter( m=535 )
  double z1[M], z2[M], z3[M];

  if (n1 != 3 && n2 != 3 && n3 != 3) {
    for (i3 = 0; i3 < mm3-1; i3++) {
      for (i2 = 0; i2 < mm2-1; i2++) {
        for (i1 = 0; i1 < mm1; i1++) {
          z1[i1] = z[i3][i2+1][i1] + z[i3][i2][i1];
          z2[i1] = z[i3+1][i2][i1] + z[i3][i2][i1];
          z3[i1] = z[i3+1][i2+1][i1] + z[i3+1][i2][i1] + z1[i1];
        }

        for (i1 = 0; i1 < mm1-1; i1++) {
          u[2*i3][2*i2][2*i1] = u[2*i3][2*i2][2*i1]
                              + z[i3][i2][i1];
          u[2*i3][2*i2][2*i1+1] = u[2*i3][2*i2][2*i1+1]
                                + 0.5 * (z[i3][i2][i1+1] + z[i3][i2][i1]);
        }
        for (i1 = 0; i1 < mm1-1; i1++) {
          u[2*i3][2*i2+1][2*i1] = u[2*i3][2*i2+1][2*i1]
                                + 0.5 * z1[i1];
          u[2*i3][2*i2+1][2*i1+1] = u[2*i3][2*i2+1][2*i1+1]
                                  + 0.25 * (z1[i1] + z1[i1+1]);
        }
        for (i1 = 0; i1 < mm1-1; i1++) {
          u[2*i3+1][2*i2][2*i1] = u[2*i3+1][2*i2][2*i1]
                                  + 0.5 * z2[i1];
          u[2*i3+1][2*i2][2*i1+1] = u[2*i3+1][2*i2][2*i1+1]
                                  + 0.25 * (z2[i1] + z2[i1+1]);
        }
        for (i1 = 0; i1 < mm1-1; i1++) {
          u[2*i3+1][2*i2+1][2*i1] = u[2*i3+1][2*i2+1][2*i1]
                                  + 0.25 * z3[i1];
          u[2*i3+1][2*i2+1][2*i1+1] = u[2*i3+1][2*i2+1][2*i1+1]
                                    + 0.125 * (z3[i1] + z3[i1+1]);
        }
      }
    }
  } else {
    if (n1 == 3) {
      d1 = 2;
      t1 = 1;
    } else {
      d1 = 1;
      t1 = 0;
    }

    if (n2 == 3) {
      d2 = 2;
      t2 = 1;
    } else {
      d2 = 1;
      t2 = 0;
    }

    if (n3 == 3) {
      d3 = 2;
      t3 = 1;
    } else {
      d3 = 1;
      t3 = 0;
    }

    for (i3 = d3; i3 <= mm3-1; i3++) {
      for (i2 = d2; i2 <= mm2-1; i2++) {
        for (i1 = d1; i1 <= mm1-1; i1++) {
          u[2*i3-d3-1][2*i2-d2-1][2*i1-d1-1] = 
            u[2*i3-d3-1][2*i2-d2-1][2*i1-d1-1]
            + z[i3-1][i2-1][i1-1];
        }
        for (i1 = 1; i1 <= mm1-1; i1++) {
          u[2*i3-d3-1][2*i2-d2-1][2*i1-t1-1] = 
            u[2*i3-d3-1][2*i2-d2-1][2*i1-t1-1]
            + 0.5 * (z[i3-1][i2-1][i1] + z[i3-1][i2-1][i1-1]);
        }
      }
      for (i2 = 1; i2 <= mm2-1; i2++) {
        for (i1 = d1; i1 <= mm1-1; i1++) {
          u[2*i3-d3-1][2*i2-t2-1][2*i1-d1-1] = 
            u[2*i3-d3-1][2*i2-t2-1][2*i1-d1-1]
            + 0.5 * (z[i3-1][i2][i1-1] + z[i3-1][i2-1][i1-1]);
        }
        for (i1 = 1; i1 <= mm1-1; i1++) {
          u[2*i3-d3-1][2*i2-t2-1][2*i1-t1-1] = 
            u[2*i3-d3-1][2*i2-t2-1][2*i1-t1-1]
            + 0.25 * (z[i3-1][i2][i1] + z[i3-1][i2-1][i1]
                    + z[i3-1][i2][i1-1] + z[i3-1][i2-1][i1-1]);
        }
      }
    }

    for (i3 = 1; i3 <= mm3-1; i3++) {
      for (i2 = d2; i2 <= mm2-1; i2++) {
        for (i1 = d1; i1 <= mm1-1; i1++) {
          u[2*i3-t3-1][2*i2-d2-1][2*i1-d1-1] = 
            u[2*i3-t3-1][2*i2-d2-1][2*i1-d1-1]
            + 0.5 * (z[i3][i2-1][i1-1] + z[i3-1][i2-1][i1-1]);
        }
        for (i1 = 1; i1 <= mm1-1; i1++) {
          u[2*i3-t3-1][2*i2-d2-1][2*i1-t1-1] = 
            u[2*i3-t3-1][2*i2-d2-1][2*i1-t1-1]
            + 0.25 * (z[i3  ][i2-1][i1] + z[i3  ][i2-1][i1-1]
                    + z[i3-1][i2-1][i1] + z[i3-1][i2-1][i1-1]);
        }
      }
      for (i2 = 1; i2 <= mm2-1; i2++) {
        for (i1 = d1; i1 <= mm1-1; i1++) {
          u[2*i3-t3-1][2*i2-t2-1][2*i1-d1-1] = 
            u[2*i3-t3-1][2*i2-t2-1][2*i1-d1-1]
            + 0.25 * (z[i3  ][i2][i1-1] + z[i3  ][i2-1][i1-1]
                    + z[i3-1][i2][i1-1] + z[i3-1][i2-1][i1-1]);
        }
        for (i1 = 1; i1 <= mm1-1; i1++) {
          u[2*i3-t3-1][2*i2-t2-1][2*i1-t1-1] = 
            u[2*i3-t3-1][2*i2-t2-1][2*i1-t1-1]
            + 0.125 * (z[i3  ][i2][i1  ] + z[i3  ][i2-1][i1  ]
                     + z[i3  ][i2][i1-1] + z[i3  ][i2-1][i1-1]
                     + z[i3-1][i2][i1  ] + z[i3-1][i2-1][i1  ]
                     + z[i3-1][i2][i1-1] + z[i3-1][i2-1][i1-1]);
        }
      }
    }
  }
}

static void distribute_interp_results(HTA* dummy, HTA* u_tile, HTA* x_tile)
{
    // iterate through u_tile elements and add appropriate offset values 
    // to get the interpolation results

    int n3 = u_tile->flat_size.values[0];
    int n2 = u_tile->flat_size.values[1];
    int n1 = u_tile->flat_size.values[2];
    int m2 = x_tile->flat_size.values[1];
    int m1 = x_tile->flat_size.values[2];
    double (*zu)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(u_tile);
    double (*zx)[m2][m1] = (double (*)[m2][m1])HTA_get_ptr_raw_data(x_tile);

    int i3, i2, i1;
    int j3, j2, j1;
    int off1, off2, off3;

    Tuple nd_size = Tuple_create(3, pi[2], pi[1], pi[0]);
    Tuple nd_idx;
    Tuple_init_zero(&nd_idx, 3);
    Tuple_1d_to_nd_index(u_tile->rank, &nd_size, &nd_idx);

    // FIXME: Assuming all tiles have the same dimensions. 
    //        It will not work if tile size is not regular
    off3 = nd_idx.values[0] * (n3 - 2);
    off2 = nd_idx.values[1] * (n2 - 2);
    off1 = nd_idx.values[2] * (n1 - 2);

    for(i3 = 0; i3 < n3; i3++)
    {
        j3 = i3 + off3;
        for(i2 = 0; i2 < n2; i2++)
        {
            j2 = i2 + off2;
            for(i1 = 0; i1 < n1; i1++)
            {
                j1 = i1 + off1;
                zu[i3][i2][i1] = zx[j3][j2][j1];
            }
        }
    }
}

// z_HTA is coarse and u_HTA is finer
static void interp_HTA(HTA* z_HTA, HTA* u_HTA, int k)
{
  if (timeron) timer_start(T_interp);
  if(Tuple_product(z_HTA->tiling) == Tuple_product(u_HTA->tiling))
  {
    HTA_map_h2(HTA_LEAF_LEVEL(z_HTA), interp_on_leaves, z_HTA, u_HTA);
  }
  else
  {
    // Interpolate to a temporary HTA* and then distribute the value to tiles
    // FIXME: easier to implement but might not be very efficient
    Tuple fs;
    Tuple_init_zero(&fs, 3);
    Dist dist0;
    Dist_init(&dist0, 0);
    fs.values[0] = z_HTA->flat_size.values[0] * 2 - 2;
    fs.values[1] = z_HTA->flat_size.values[1] * 2 - 2;
    fs.values[2] = z_HTA->flat_size.values[2] * 2 - 2;
    HTA* x_HTA = HTA_create(3, 1, &fs, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 0, NULL);
    HTA_map_h1s1(HTA_LEAF_LEVEL(x_HTA), H1S1_INIT, x_HTA, &zero);
    interp_on_leaves(z_HTA->tiles[0], x_HTA);
    HTA_tile_to_hta(HTA_LEAF_LEVEL(u_HTA), distribute_interp_results, u_HTA, u_HTA, x_HTA);
    HTA_destroy(x_HTA);
  }
  if (timeron) timer_stop(T_interp);

  if (debug_vec[0] >= 1) {
    rep_nrm(z_HTA, "z: inter", k-1);
    rep_nrm(u_HTA, "u: inter", k);
  }

  if (debug_vec[5] >= k) {
    showall(z_HTA);
    showall(u_HTA);
  }
}


//---------------------------------------------------------------------
// norm2u3 evaluates approximations to the L2 norm and the
// uniform (or L-infinity or Chebyshev) norm, under the
// assumption that the boundaries are periodic or zero.  Add the
// boundaries in with half weight (quarter weight on the edges
// and eighth weight at the corners) for inhomogeneous boundaries.
//---------------------------------------------------------------------
void SUMSQUARE(HTA* d, HTA* dummy, void* x)
{
  int n1 = d->flat_size.values[2];
  int n2 = d->flat_size.values[1];
  int n3 = d->flat_size.values[0];

  int i3, i2, i1;

  double (*zd)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(d);
  double sum = *(double*)x;

  for(i3 = 0; i3 < n3; i3++)
      for(i2 = 0; i2 < n2; i2++)
          for(i1 = 0; i1 < n1; i1++)
          {
              if(!(i1 == 0 || i2 == 0 || i3 == 0 || i1 == n1-1 || i2 == n2-1 || i3 == n3-1))
                sum += pow(zd[i3][i2][i1], 2.0);
          }

  *(double*)x = sum;
}

void MAXABS(HTA* d, HTA* dummy, void* x)
{
  int n1 = d->flat_size.values[2];
  int n2 = d->flat_size.values[1];
  int n3 = d->flat_size.values[0];

  int i3, i2, i1;

  double (*zd)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(d);
  double maxval = *(double*)x;

  for(i3 = 0; i3 < n3; i3++)
      for(i2 = 0; i2 < n2; i2++)
          for(i1 = 0; i1 < n1; i1++)
          {
              if(!(i1 == 0 || i2 == 0 || i3 == 0 || i1 == n1-1 || i2 == n2-1 || i3 == n3-1))
                if(zd[i3][i2][i1] > maxval)
                    maxval = zd[i3][i2][i1];
          }

  *(double*)x = maxval;
}

static void norm2u3_HTA(HTA* z,
                    double *rnm2, double *rnmu,
                    int nx, int ny, int nz, int k)
{
  double s;
  //int i3, i2, i1;

  double dn, max_rnmu;
  if (timeron) timer_start(T_norm2);
  dn = 1.0*nx*ny*nz;

  s = 0.0;
  max_rnmu = 0.0;

  HTA_reduce_h2(REDUCE_SUM, SUMSQUARE, &s, z, z);
  HTA_reduce_h2(REDUCE_MAX, MAXABS, &max_rnmu, z, z);

  *rnmu = max_rnmu;

  *rnm2 = sqrt(s / dn);
  if (timeron) timer_stop(T_norm2);
}
//---------------------------------------------------------------------
// report on norm
//---------------------------------------------------------------------
//static void rep_nrm(void *u, int n1, int n2, int n3, char *title, int kk)
static void rep_nrm(HTA* u_HTA, char *title, int kk)
{
  double rnm2, rnmu;

  norm2u3_HTA(u_HTA, &rnm2, &rnmu, nx[kk], ny[kk], nz[kk], kk);
  printf(" Level%2d in %8s: norms =%21.14E%21.14E\n", kk, title, rnm2, rnmu);
}


//---------------------------------------------------------------------
// comm3 organizes the communication on all borders 
//---------------------------------------------------------------------

// Actively write necessary boundaries from other neighbor tiles
// Writes don't have to wait because all writes go directly to where
// the boundaries are. Needs to write to 26 neighbors (6 surfaces,
// 12 vectors and 8 corner points)

#define IDX_INC(i) \
    do { \
    target_idx.values[i] = (nd_idx.values[i] + 1) % nd_size->values[i]; \
    } while(0)

#define IDX_DEC(i) \
    do { \
    target_idx.values[i] = (nd_idx.values[i] == 0)? nd_size->values[i] - 1 : nd_idx.values[i] - 1; \
    } while(0)

#define IDX_SAME(i) \
    do { \
    target_idx.values[i] = nd_idx.values[i];\
    } while(0)

#define X_PLUS_1() IDX_INC(2)
#define Y_PLUS_1() IDX_INC(1)
#define Z_PLUS_1() IDX_INC(0)

#define X_MINUS_1() IDX_DEC(2)
#define Y_MINUS_1() IDX_DEC(1)
#define Z_MINUS_1() IDX_DEC(0)

#define X_SAME() IDX_SAME(2)
#define Y_SAME() IDX_SAME(1)
#define Z_SAME() IDX_SAME(0)

#define GET_TILE_PTR() \
     t = HTA_pick_one_tile(s2, &target_idx); \
     n1t = t->flat_size.values[2]; \
     n2t = t->flat_size.values[1]; \
     n3t = t->flat_size.values[0]; \
     double (*zt)[n2t][n1t] = (double (*)[n2t][n1t])HTA_get_ptr_raw_data(t); \

void sync_boundary(HTA* d_tile, HTA* s1_tile, HTA* s2)
{
    HTA* t;
    //int d;
    int n1t, n2t, n3t;
    int i1, i2, i3;

    int n1 = s1_tile->flat_size.values[2];
    int n2 = s1_tile->flat_size.values[1];
    int n3 = s1_tile->flat_size.values[0];
    double (*zs)[n2][n1] = (double (*)[n2][n1])HTA_get_ptr_raw_data(s1_tile);

    // Get globa tile nd index first
    Tuple* nd_size = s2->tiling; // tile dimensions
    Tuple nd_idx;
    Tuple_init_zero(&nd_idx, 3); // this tile index
    Tuple target_idx;
    Tuple_init_zero(&target_idx, 3); // target tile index
    Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);

    // Update the 6 surfaces (index differ by 1 at only dimension)
    // x+1
    X_PLUS_1();
    Y_SAME();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            for(i2 = 1; i2 < n2t - 1; i2++)
                zt[i3][i2][0] = zs[i3][i2][n1-2];
    }
    // x-1
    X_MINUS_1();
    Y_SAME();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            for(i2 = 1; i2 < n2t - 1; i2++)
                zt[i3][i2][n1t-1] = zs[i3][i2][1];
    }

    // y+1
    X_SAME();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            for(i1 = 1; i1 < n1t - 1; i1++)
                zt[i3][0][i1] = zs[i3][n2-2][i1];
    }
    // y-1
    X_SAME();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            for(i1 = 1; i1 < n1t - 1; i1++)
                zt[i3][n2t-1][i1] = zs[i3][1][i1];
    }
    // z+1
    X_SAME();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2= 1; i2 < n2t - 1; i2++)
            for(i1 = 1; i1 < n1t - 1; i1++)
                zt[0][i2][i1] = zs[n3-2][i2][i1];
    }
    // z-1
    X_SAME();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2= 1; i2 < n2t - 1; i2++)
            for(i1 = 1; i1 < n1t - 1; i1++)
                zt[n3t-1][i2][i1] = zs[1][i2][i1];
    }

    // 12 vectors
    // x+1, y+1, z
    X_PLUS_1();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            zt[i3][0][0] = zs[i3][n2-2][n1-2];
    }
    // x+1, y-1, z
    X_PLUS_1();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            zt[i3][n2t-1][0] = zs[i3][1][n1-2];
    }
    // x-1, y-1, z
    X_MINUS_1();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            zt[i3][n2t-1][n1t-1] = zs[i3][1][1];
    }
    // x-1, y+1, z
    X_MINUS_1();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 1; i3 < n3t - 1; i3++)
            zt[i3][0][n1t-1] = zs[i3][n2-2][1];
    }
    // x, y+1, z+1
    X_SAME();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 1; i1 < n1t - 1; i1++)
            zt[0][0][i1] = zs[n3-2][n2-2][i1];
    }
    // x, y-1, z+1
    X_SAME();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 1; i1 < n1t - 1; i1++)
            zt[0][n2t-1][i1] = zs[n3-2][1][i1];
    }
    // x, y-1, z-1
    X_SAME();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 1; i1 < n1t - 1; i1++)
            zt[n3t-1][n2t-1][i1] = zs[1][1][i1];
    }
    // x, y+1, z-1
    X_SAME();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 1; i1 < n1t - 1; i1++)
            zt[n3t-1][0][i1] = zs[1][n2-2][i1];
    }
    // x+1, y, z+1
    X_PLUS_1();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 1; i2 < n2t - 1; i2++)
            zt[0][i2][0] = zs[n3-2][i2][n1-2];
    }
    // x-1, y, z+1
    X_MINUS_1();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 1; i2 < n2t - 1; i2++)
            zt[0][i2][n1t-1] = zs[n3-2][i2][1];
    }
    // x-1, y, z-1
    X_MINUS_1();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 1; i2 < n2t - 1; i2++)
            zt[n3t-1][i2][n1t-1] = zs[1][i2][1];
    }
    // x+1, y, z-1
    X_PLUS_1();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 1; i2 < n2t - 1; i2++)
            zt[n3t-1][i2][0] = zs[1][i2][n1-2];
    }

    // 8 points
    // x+1, y+1, z+1
    X_PLUS_1();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        zt[0][0][0] = zs[n3-2][n2-2][n1-2];
    }
    // x+1, y+1, z-1
    X_PLUS_1();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        zt[n3t-1][0][0] = zs[1][n2-2][n1-2];
    }
    // x+1, y-1, z+1
    X_PLUS_1();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        zt[0][n2t-1][0] = zs[n3-2][1][n1-2];
    }
    // x+1, y-1, z-1
    X_PLUS_1();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        zt[n3t-1][n2t-1][0] = zs[1][1][n1-2];
    }
    // x-1, y+1, z+1
    X_MINUS_1();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        zt[0][0][n1t-1] = zs[n3-2][n2-2][1];
    }
    // x-1, y+1, z-1
    X_MINUS_1();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        zt[n3t-1][0][n1t-1] = zs[1][n2-2][1];
    }
    // x-1, y-1, z+1
    X_MINUS_1();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        zt[0][n2t-1][n1t-1] = zs[n3-2][1][1];
    }
    // x-1, y-1, z-1
    X_MINUS_1();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        zt[n3t-1][n2t-1][n1t-1] = zs[1][1][1];
    }

}

//  FIXME: this should be a collective operation instead of a map
static void comm3_HTA(HTA* h)
{
    HTA_tile_to_hta(HTA_LEAF_LEVEL(h), sync_boundary, h, h, h);
}

//---------------------------------------------------------------------
// zran3  loads +1 at ten randomly chosen points,
// loads -1 at a different ten random points,
// and zero elsewhere.
//---------------------------------------------------------------------
// ** NOTICE: this is not timed so it's perform sequentially and the
// **         results are copied to HTA later
//---------------------------------------------------------------------
static void zran3(void *oz, int nx1, int ny1, int nz1)
{
  double (*z)[ny1][nx1] = (double (*)[ny1][nx1])oz;

  int i0, mm0, mm1;

  int i1, i2, i3, d1, e1, e2, e3;
  double xx, x0, x1, a1, a2, ai;

  const int mm = 10;
  const double a = pow(5.0, 13.0);
  const double x = 314159265.0;
  double ten[mm][2], best;
  int i, j1[mm][2], j2[mm][2], j3[mm][2];
  int jg[4][mm][2];

  double rdummy;

  a1 = power(a, nx1);
  a2 = power(a, nx1*ny1);

  zero3(z, nx1, ny1, nz1);

  //i = is1-2+nx1*(is2-2+ny1*(is3-2));

  //ai = power(a, i);
  //d1 = ie1 - is1 + 1;
  d1 = nx1;
  //e1 = ie1 - is1 + 2;
  e1 = nx1;
  //e2 = ie2 - is2 + 2;
  e2 = ny1;
  //e3 = ie3 - is3 + 2;
  e3 = nz1;
  x0 = x;
  //rdummy = randlc(&x0, ai);

  //---------------------------------------------------------------------
  // save the starting seeds for the following loop
  //---------------------------------------------------------------------
  for (i3 = 0; i3 < nz1; i3++) {
    starts[i3] = x0;
    rdummy = randlc(&x0, a2);
  }

  //---------------------------------------------------------------------
  // fill array
  //---------------------------------------------------------------------
  for (i3 = 0; i3 < nz1; i3++) {
    x1 = starts[i3];
    for (i2 = 0; i2 < ny1; i2++) {
      xx = x1;
      vranlc(d1, &xx, a, &(z[i3][i2][0]));
      rdummy = randlc(&x1, a1);
    }
  }

  //---------------------------------------------------------------------
  // comm3(z,n1,n2,n3);
  // showall(z,n1,n2,n3);
  //---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // each thread looks for twenty candidates
  //---------------------------------------------------------------------
  for (i = 0; i < mm; i++) {
    ten[i][1] = 0.0;
    j1[i][1] = 0;
    j2[i][1] = 0;
    j3[i][1] = 0;
    ten[i][0] = 1.0;
    j1[i][0] = 0;
    j2[i][0] = 0;
    j3[i][0] = 0;
  }

  for (i3 = 0; i3 < nz1; i3++) {
    double (*zi3)[nx1] = z[i3];
    for (i2 = 0; i2 < ny1; i2++) {
      for (i1 = 0; i1 < nx1; i1++) {
        if (zi3[i2][i1] > ten[0][1]) {
          ten[0][1] = zi3[i2][i1];
          j1[0][1] = i1;
          j2[0][1] = i2;
          j3[0][1] = i3;
          bubble(ten, j1, j2, j3, mm, 1);
        }
        if (zi3[i2][i1] < ten[0][0]) {
          ten[0][0] = zi3[i2][i1];
          j1[0][0] = i1;
          j2[0][0] = i2;
          j3[0][0] = i3;
          bubble(ten, j1, j2, j3, mm, 0);
        }
      }
    }
  }

  //---------------------------------------------------------------------
  // Now which of these are globally best?
  //---------------------------------------------------------------------
  i1 = mm - 1;
  i0 = mm - 1; 
  for (i = mm - 1; i >= 0; i--) {
    best = 0.0;
    if (best < ten[i1][1]) {
      jg[0][i][1] = 0;
      //jg[1][i][1] = is1 - 2 + j1[i1][1];
      //jg[2][i][1] = is2 - 2 + j2[i1][1];
      //jg[3][i][1] = is3 - 2 + j3[i1][1];
      jg[1][i][1] = j1[i1][1];
      jg[2][i][1] = j2[i1][1];
      jg[3][i][1] = j3[i1][1];
      i1 = i1-1;
    } else {
      jg[0][i][1] = 0;
      jg[1][i][1] = 0;
      jg[2][i][1] = 0;
      jg[3][i][1] = 0;
    }

    best = 1.0;
    if (best > ten[i0][0]) {
      jg[0][i][0] = 0;
      //jg[1][i][0] = is1 - 2 + j1[i0][0];
      //jg[2][i][0] = is2 - 2 + j2[i0][0];
      //jg[3][i][0] = is3 - 2 + j3[i0][0];
      jg[1][i][0] = j1[i0][0];
      jg[2][i][0] = j2[i0][0];
      jg[3][i][0] = j3[i0][0];
      i0 = i0-1;
    } else {
      jg[0][i][0] = 0;
      jg[1][i][0] = 0;
      jg[2][i][0] = 0;
      jg[3][i][0] = 0;
    }

  }
  mm1 = 0;
  mm0 = 0;

  // FIXME: This will not work for the distributed memory model
  // NOTICE: the coordinates generated are off by 1 to pure OMP version
  Tuple flat_size = Tuple_create(3, nz1, ny1, nx1);
  Tuple *nd_num_tiles = v_HTA->tiling;
  Tuple tile_nd_index;
  Tuple_init_zero(&tile_nd_index, 3);
  Tuple tile_local_index;
  Tuple_init_zero(&tile_local_index, 3);
  Tuple *acc[2] = {&tile_nd_index, &tile_local_index};
  Tuple c;
  Tuple_init_zero(&c, 3);
  double *ptr;
  for (i = mm-1; i >= mm0; i--) {
    //printf("%d, %d, %d is set to -1.0\n", jg[3][i][0], jg[2][i][0], jg[1][i][0]);
    // the index within HTA with overlapped tiles is different
    // must know which tile the element is and add correct offset to write to correct
    // location in overlapped tile
    // nd global index to tile index (of the original non overlapped HTA)
    c.values[0] = jg[3][i][0]; 
    c.values[1] = jg[2][i][0]; 
    c.values[2] = jg[1][i][0]; 
    HTA_nd_global_to_tile_index(&flat_size, nd_num_tiles, &c, &tile_nd_index, &tile_local_index);
    tile_local_index.values[0] += 1;
    tile_local_index.values[1] += 1;
    tile_local_index.values[2] += 1;
    ptr = (double*)HTA_access_element(v_HTA, acc);
    *ptr = -1.0;
  }
  for (i = mm-1; i >= mm1; i--) {
    //printf("%d, %d, %d is set to +1.0\n", jg[3][i][1], jg[2][i][1], jg[1][i][1]);
    c.values[0] = jg[3][i][1];
    c.values[1] = jg[2][i][1];
    c.values[2] = jg[1][i][1];
    HTA_nd_global_to_tile_index(&flat_size, nd_num_tiles, &c, &tile_nd_index, &tile_local_index);
    tile_local_index.values[0] += 1;
    tile_local_index.values[1] += 1;
    tile_local_index.values[2] += 1;
    ptr = (double*)HTA_access_element(v_HTA, acc);
    *ptr = +1.0;
  }
  comm3_HTA(v_HTA);
}


static void showall(HTA* z_HTA)
{
  int n3 = z_HTA->flat_size.values[0];
  int n2 = z_HTA->flat_size.values[1];
  int n1 = z_HTA->flat_size.values[2];

  int i1, i2, i3;
  int m1, m2, m3;

  m1 = min(n1, 18);
  m2 = min(n2, 14);
  m3 = min(n3, 18);

  printf("m3 = %d, m2 = %d, m1 = %d\n", m3, m2, m1);
  Tuple c;
  Tuple_init_zero(&c, 3);
  printf("   \n");
  for (i3 = 0; i3 < m3; i3++) {
    for (i2 = 0; i2 < m2; i2++) {
      for (i1 = 0; i1 < m1; i1++) {
        c.values[0] = i3;
        c.values[1] = i2;
        c.values[2] = i1;
        
        double* ptr = HTA_flat_access(z_HTA, &c);
        printf("%6.3f", *ptr);
      }
      printf("\n");
    }
    printf("  - - - - - - - \n");
  }
  printf("   \n");
}


//---------------------------------------------------------------------
// power  raises an integer, disguised as a double
// precision real, to an integer power
//---------------------------------------------------------------------
static double power(double a, int n)
{
  double aj;
  int nj;
  double rdummy;
  double power;

  power = 1.0;
  nj = n;
  aj = a;

  while (nj != 0) {
    if ((nj % 2) == 1) rdummy = randlc(&power, aj);
    rdummy = randlc(&aj, aj);
    nj = nj/2;
  }

  return power;
}


//---------------------------------------------------------------------
// bubble        does a bubble sort in direction dir
//---------------------------------------------------------------------
static void bubble(double ten[][2], int j1[][2], int j2[][2], int j3[][2],
                   int m, int ind)
{
  double temp;
  int i, j_temp;

  if (ind == 1) {
    for (i = 0; i < m-1; i++) {
      if (ten[i][ind] > ten[i+1][ind]) {
        temp = ten[i+1][ind];
        ten[i+1][ind] = ten[i][ind];
        ten[i][ind] = temp;

        j_temp = j1[i+1][ind];
        j1[i+1][ind] = j1[i][ind];
        j1[i][ind] = j_temp;

        j_temp = j2[i+1][ind];
        j2[i+1][ind] = j2[i][ind];
        j2[i][ind] = j_temp;

        j_temp = j3[i+1][ind];
        j3[i+1][ind] = j3[i][ind];
        j3[i][ind] = j_temp;
      } else {
        return;
      }
    }
  } else {
    for (i = 0; i < m-1; i++) {
      if (ten[i][ind] < ten[i+1][ind]) {

        temp = ten[i+1][ind];
        ten[i+1][ind] = ten[i][ind];
        ten[i][ind] = temp;

        j_temp = j1[i+1][ind];
        j1[i+1][ind] = j1[i][ind];
        j1[i][ind] = j_temp;

        j_temp = j2[i+1][ind];
        j2[i+1][ind] = j2[i][ind];
        j2[i][ind] = j_temp;

        j_temp = j3[i+1][ind];
        j3[i+1][ind] = j3[i][ind];
        j3[i][ind] = j_temp;
      } else {
        return;
      }
    }
  }
}


static void zero3(void *oz, int n1, int n2, int n3)
{
  double (*z)[n2][n1] = (double (*)[n2][n1])oz;

  int i1, i2, i3;

  //#pragma omp parallel for default(shared) private(i1,i2,i3)
  for (i3 = 0; i3 < n3; i3++) {
    for (i2 = 0; i2 < n2; i2++) {
      for (i1 = 0; i1 < n1; i1++) {
        z[i3][i2][i1] = 0.0;
      }
    }
  }
}

