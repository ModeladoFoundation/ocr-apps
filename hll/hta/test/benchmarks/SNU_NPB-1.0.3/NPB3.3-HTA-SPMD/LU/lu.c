//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an OpenMP C version of the NPB LU code. This OpenMP  //
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
//   program applu
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//   driver for the performance evaluation of the solver for
//   five coupled parabolic/elliptic partial differential equations.
//
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "applu.incl"
#include "timers.h"
#include "print_results.h"

#include "HTA.h"
#include "HTA_operations.h"
#include "Comm.h"

//#include <gperftools/profiler.h>

int PROC_X = 1;
int PROC_Y = 1;
int PROC_Z = 1;
int PROC = 1;

int check_bound = -1;

//---------------------------------------------------------------------
// grid
//---------------------------------------------------------------------
/* common/cgcon/ */
double dxi, deta, dzeta;
double tx1, tx2, tx3;
double ty1, ty2, ty3;
double tz1, tz2, tz3;
int nx, ny, nz;
int nx0, ny0, nz0;
int ist, iend;
int jst, jend;
int ii1, ii2;
int ji1, ji2;
int ki1, ki2;

//---------------------------------------------------------------------
// dissipation
//---------------------------------------------------------------------
/* common/disp/ */
double dx1, dx2, dx3, dx4, dx5;
double dy1, dy2, dy3, dy4, dy5;
double dz1, dz2, dz3, dz4, dz5;
double dssp;

//---------------------------------------------------------------------
// field variables and residuals
// to improve cache performance, second two dimensions padded by 1
// for even number sizes only.
// Note: corresponding array (called "v") in routines blts, buts,
// and l2norm are similarly padded
//---------------------------------------------------------------------
/* common/cvar/ */
// double u    [ISIZ3][ISIZ2/2*2+1][ISIZ1/2*2+1][5];
// double rsd  [ISIZ3][ISIZ2/2*2+1][ISIZ1/2*2+1][5];
// double frct [ISIZ3][ISIZ2/2*2+1][ISIZ1/2*2+1][5];
// double flux [ISIZ1][5];
// double qs   [ISIZ3][ISIZ2/2*2+1][ISIZ1/2*2+1];
// double rho_i[ISIZ3][ISIZ2/2*2+1][ISIZ1/2*2+1];

// double test[ISIZ3+(4*PROC_Z)][ISIZ2+(4*PROC_Y)][ISIZ1+(4*PROC_X)][5];

//---------------------------------------------------------------------
// output control parameters
//---------------------------------------------------------------------
/* common/cprcon/ */
int ipr, inorm;

//---------------------------------------------------------------------
// newton-raphson iteration control parameters
//---------------------------------------------------------------------
/* common/ctscon/ */
//double dt, omega, tolrsd[5], rsdnm[5], errnm[5], frc, ttotal;
double dt, tolrsd[5], omega, ttotal;
int itmax, invert;

/* common/cjac/ */
// double a[ISIZ2][ISIZ1/2*2+1][5][5];
// double b[ISIZ2][ISIZ1/2*2+1][5][5];
// double c[ISIZ2][ISIZ1/2*2+1][5][5];
// double d[ISIZ2][ISIZ1/2*2+1][5][5];

/* common/cjacu/ */
// double au[ISIZ2][ISIZ1/2*2+1][5][5];
// double bu[ISIZ2][ISIZ1/2*2+1][5][5];
// double cu[ISIZ2][ISIZ1/2*2+1][5][5];
// double du[ISIZ2][ISIZ1/2*2+1][5][5];


//---------------------------------------------------------------------
// coefficients of the exact solution
//---------------------------------------------------------------------
/* common/cexact/ */
double ce[5][13];

//---------------------------------------------------------------------
// timers
//---------------------------------------------------------------------
/* common/timer/ */
double maxtime;
logical timeron;

HTA* ptr_rsd_HTA[128];

#define CREATE_TILE_SELECTION(rgn, zstart, zend, ystart, yend, xstart, xend) \
{ \
    Range_init(&(rgn##_tiles_range_x),  xstart, xend, 1, 0); \
    Range_init(&(rgn##_tiles_range_y),  ystart, yend, 1, 0); \
    Range_init(&(rgn##_tiles_range_z),  zstart, zend, 1, 0); \
    Range_init(&(rgn##_tiles_range_w),  0, -1, 1, 0); \
    Region_init(&(rgn##_tiles_sel), 4, rgn##_tiles_range_z, rgn##_tiles_range_y, rgn##_tiles_range_x, rgn##_tiles_range_w); \
    HTA_set_abs_region_with_hta(h, &(rgn##_tiles_sel)); \
    Region_to_boolean_array(&(rgn##_tiles_sel), &h->tiling, rgn##_sel); \
}

/*
#define CREATE_TILE_SELECTION(rgn, zstart, zend, ystart, yend, xstart, xend) \
{ \
    Range_init(&(rgn##_tiles_range_x),  xstart, xend, 1, 0); \
    Range_init(&(rgn##_tiles_range_y),  ystart, yend, 1, 0); \
    Range_init(&(rgn##_tiles_range_z),  zstart, zend, 1, 0); \
    Range_init(&(rgn##_tiles_range_w),  0, -1, 1, 0); \
    Region_init(&(rgn##_tiles_sel), 4, rgn##_tiles_range_z, rgn##_tiles_range_y, rgn##_tiles_range_x, rgn##_tiles_range_w); \
}*/

#define CREATE_ELEM_SELECTION(rgn, zstart, zend, ystart, yend, xstart, xend) \
{ \
    Range_init(&(rgn##_elem_range_x), xstart, xend, 1, 0); \
    Range_init(&(rgn##_elem_range_y), ystart, yend, 1, 0); \
    Range_init(&(rgn##_elem_range_z), zstart, zend, 1, 0); \
    Range_init(&(rgn##_elem_range_w), 0, -1, 1, 0); \
    Region_init(&(rgn##_elem_sel), 4, rgn##_elem_range_z, rgn##_elem_range_y, rgn##_elem_range_x, rgn##_elem_range_w); \
}


int hta_main(int argc, char *argv[], int pid)
{
  char Class;
  logical verified;
  double mflops;

  double t, tmax, trecs[t_last+1];
  int i;
  char *t_names[t_last+3];
  //int PROC;

  // Old Global Variables
  double errnm[5], rsdnm[5], frc;

  if (argc >= 4)
  {
      PROC_X = atoi(argv[3]);
      PROC_Y = atoi(argv[2]);
      PROC_Z = atoi(argv[1]);
  }

  if (argc == 5) PROC = atoi(argv[4]);
  else PROC = omp_get_max_threads();

  SET_SPMD_PID(pid)
  //---------------------------------------------------------------------
  // Setup info for timers
  //---------------------------------------------------------------------
  MASTER_EXEC_START
  FILE *fp;
  if ((fp = fopen("timer.flag", "r")) != NULL) {
    timeron = true;
    t_names[t_total] = "total";
    //t_names[t_rhsx] = "rhsx";
    //t_names[t_rhsy] = "rhsy";
    //t_names[t_rhsz] = "rhsz";
    t_names[t_rhs] = "rhs";
    //t_names[t_jacld] = "jacld";
    t_names[t_blts] = "blts+jacld";
    //t_names[t_jacu] = "jacu";
    t_names[t_buts] = "buts+jacu";
    t_names[t_add] = "add";
    t_names[t_l2norm] = "l2norm";
    t_names[t_sync_rhs] = "sync_rhs";
    t_names[t_sync_blts] = "sync_blts";
    t_names[t_sync_buts] = "sync_buts";
    fclose(fp);
  } else {
    timeron = false;
  }
  MASTER_EXEC_END

  // Calculate the number of processors in each dimension
  int pi[3];
  double log_p  = log((double)(PROC)+0.0001)/log(2.0);
  int dx     = (int)(log_p/3);
  pi[0] = (int) pow(2.0, dx);
  int dy     = (int)((log_p-dx)/2);
  pi[1]  = (int) pow (2.0, dy);
  pi[2]  = PROC/(pi[0]*pi[1]);

  //pi[0] = PROC_Z;
  //pi[1] = PROC_Y;
  //pi[2] = PROC_X;

  //---------------------------------------------------------------------
  // HTA definitions
  //---------------------------------------------------------------------
  HTA* u_HTA;
  HTA* rsd_HTA;
  HTA* frct_HTA;
  HTA* qs_HTA;
  HTA* rho_i_HTA;
  HTA* sum_HTA;
  HTA* err_HTA;

  Tuple t1 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple fs1 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 5);
  Dist dist0;
  Tuple mesh;
  //Tuple_init(&mesh, 4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple_init(&mesh, 4, pi[2], pi[1]*pi[0], 1, 1, 1);
  //Tuple_init(&mesh, 4, pi[2]*pi[1]*pi[0], 1, 1, 1, 1);
  Dist_init(&dist0, DIST_CYCLIC, &mesh);

  u_HTA = HTA_create_with_pid(pid, 4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  rsd_HTA = HTA_create_with_pid(pid, 4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  frct_HTA = HTA_create_with_pid(pid, 4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  //---------------------------------------------------------------------
  Tuple t2 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1); // Using 4D for convenience
  Tuple fs2 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 1);
  rho_i_HTA = HTA_create_with_pid(pid, 4, 2, &fs2, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t2);
  qs_HTA = HTA_create_with_pid(pid, 4, 2, &fs2, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t2);
  //---------------------------------------------------------------------
  Tuple t4 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple fs4 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 5);
  sum_HTA = HTA_create_with_pid(pid, 4, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t4); // Required by l2norm
  err_HTA = HTA_create_with_pid(pid, 4, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t4); // Required by error
  //---------------------------------------------------------------------

#ifdef SPMD_PIL
    MASTER_PRINTF(" (%d) Using SPMD model \n", pid);
    ptr_rsd_HTA[pid] = rsd_HTA;
#else
    printf(" Using fork-join model \n");
    ptr_rsd_HTA[0] = rsd_HTA;
#endif

  //---------------------------------------------------------------------
  // read input data
  //---------------------------------------------------------------------
  MASTER_EXEC_START

  read_input();
    printf(" Using (z : %d, y : %d, x : %d) tiles.\n", PROC_Z, PROC_Y, PROC_X);
    printf(" Mesh pz: %d py: %d px: %d\n\n", pi[2], pi[1]*pi[0], 1);
    //---------------------------------------------------------------------
    // set up domain sizes
    //---------------------------------------------------------------------
    domain();
    //---------------------------------------------------------------------
    // set up coefficients
    //---------------------------------------------------------------------
    setcoeff();

  MASTER_EXEC_SYNC_END

  //---------------------------------------------------------------------
  // set the boundary values for dependent variables
  //---------------------------------------------------------------------
  setbv_HTA(u_HTA);

  //---------------------------------------------------------------------
  // set the initial values for dependent variables
  //---------------------------------------------------------------------
  setiv_HTA(u_HTA);

  //---------------------------------------------------------------------
  // compute the forcing term based on prescribed exact solution
  //---------------------------------------------------------------------
  erhs_HTA(pid, frct_HTA, rsd_HTA);

  //---------------------------------------------------------------------
  // perform one SSOR iteration to touch all data pages
  //---------------------------------------------------------------------
  ssor_HTA(1, pid, rsdnm, u_HTA, rsd_HTA, frct_HTA, qs_HTA, rho_i_HTA, sum_HTA);

  //---------------------------------------------------------------------
  // reset the boundary and initial values
  //---------------------------------------------------------------------
  setbv_HTA(u_HTA);
  setiv_HTA(u_HTA);

  //---------------------------------------------------------------------
  // perform the SSOR iterations
  //---------------------------------------------------------------------
  //ProfilerStart("lu_profiler.log");
  ssor_HTA(itmax, pid, rsdnm, u_HTA, rsd_HTA, frct_HTA, qs_HTA, rho_i_HTA, sum_HTA);
  //ProfilerStop();
  //---------------------------------------------------------------------
  // compute the solution error
  //---------------------------------------------------------------------
  error_HTA(errnm, pid, u_HTA, err_HTA);

  //---------------------------------------------------------------------
  // compute the surface integral
  //---------------------------------------------------------------------
  pintgr_HTA(pid, pi, &frc, u_HTA);

  //---------------------------------------------------------------------
  // verification test
  //---------------------------------------------------------------------
  MASTER_EXEC_START
  verify ( rsdnm, errnm, frc, &Class, &verified );
  mflops = (double)itmax * (1984.77 * (double)nx0
      * (double)ny0
      * (double)nz0
      - 10923.3 * pow(((double)(nx0+ny0+nz0)/3.0), 2.0)
      + 27770.9 * (double)(nx0+ny0+nz0)/3.0
      - 144010.0)
    / (maxtime*1000000.0);

  print_results("LU", Class, nx0,
                ny0, nz0, itmax,
                maxtime, mflops, "          floating point", verified,
                NPBVERSION, COMPILETIME, CS1, CS2, CS3, CS4, CS5, CS6,
                "(none)");



  //---------------------------------------------------------------------
  // More timers
  //---------------------------------------------------------------------

  if (timeron) {
    for (i = 1; i <= t_last; i++) {
      trecs[i] = timer_read(i);
    }
    tmax = maxtime;
    if (tmax == 0.0) tmax = 1.0;

    char rec_name[256];
    sprintf(rec_name, "rec_spmd/lu.%c.%d.rec", Class, PROC);
    FILE* fp_rec = fopen(rec_name, "a");
    fprintf(fp_rec, "%9.4f ", tmax);

    printf("  SECTION     Time (secs)\n");
    for (i = 1; i <= t_last; i++) {
      if (i == 1 || i >= 2) fprintf(fp_rec, "%9.4f ", trecs[i]);
      printf("  %-8s:%9.4f  (%6.2f%%)\n",
          t_names[i], trecs[i], trecs[i]*100./tmax);
      //if (i == t_rhs) {
      //  t = trecs[t_rhsx] + trecs[t_rhsy] + trecs[t_rhsz];
      //  printf("     --> %8s:%9.4f  (%6.2f%%)\n", "sub-rhs", t, t*100./tmax);
      //  t = trecs[i] - t;
      //  printf("     --> %8s:%9.4f  (%6.2f%%)\n", "rest-rhs", t, t*100./tmax);
      //}
    }
    //printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[12], timer_read(12), timer_read(12)*100./tmax);
    //printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[13], timer_read(13), timer_read(13)*100./tmax);
    //printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[14], timer_read(14), timer_read(14)*100./tmax);
    fprintf(fp_rec, "\n");
    fclose(fp_rec);
  }
  MASTER_EXEC_END

  HTA_destroy(rsd_HTA);
  HTA_destroy(frct_HTA);
  HTA_destroy(rho_i_HTA);
  HTA_destroy(qs_HTA);
  HTA_destroy(u_HTA);
  HTA_destroy(sum_HTA);
  HTA_destroy(err_HTA);

  comm_bcast(pid, 0, &verified, sizeof(logical));
  return (verified)?0:-1;
}

//**************************************************************
// Iterators for assign (boundaries = 2 elements)
//**************************************************************

int Tuple_iterate_src_0(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 4;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
        if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_0b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = flat_size->values[dim_trav] - 4;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 4;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
        if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_1(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 4;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_1b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = flat_size->values[dim_trav] - 4;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 4;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_2(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 4;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_2b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = flat_size->values[dim_trav] - 4;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 4;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_0(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = flat_size->values[dim_trav] - 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim];
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
      return 1; // next element found
    }
}

int Tuple_iterate_dest_0b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 0;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = 0;
	max = 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_1(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = flat_size->values[dim_trav] - 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim];
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_1b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 0;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = 0;
	max = 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_2(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = flat_size->values[dim_trav] - 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;

    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim];
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_2b(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 0;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-2; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
     if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
     }
     else if(dim == dim_trav) {
	min = 0;
	max = 2;
     }
     else {
	min = 2;
	max = flat_size->values[dim] - 2;
     }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }

    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}


//**************************************************************
// Iterators for assign_sel (boundaries = 1 element)
//**************************************************************

int Tuple_iterate_src_0_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 3;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
        if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_0b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = flat_size->values[dim_trav] - 4;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 3;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
        if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_1_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 3;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_1b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = flat_size->values[dim_trav] - 4;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 3;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_2_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = 2;
	max = 3;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_src_2b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = flat_size->values[dim_trav] - 4;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
      //Tuple_init_zero(iter, total_dim);
      //for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
      //iter->values[dim_trav] = flat_size->values[dim_trav] - 4;
      //iter->values[3] = 0; // Using 4D in LU for convinience
      //*idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) { // not iterate through 4D
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 3;
	max = flat_size->values[dim] - 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_0_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = flat_size->values[dim_trav] - 2;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim] - 1;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
      }
      else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
      return 1; // next element found
    }
}

int Tuple_iterate_dest_0b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 0;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 0;
      iter->values[1] = 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = 1;
	max = 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_1_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = flat_size->values[dim_trav] - 2;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim] - 1;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_1b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 1;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 0;
      iter->values[2] = 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = 1;
	max = 2;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_2_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = flat_size->values[dim_trav] - 2;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-1; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = flat_size->values[dim_trav] - 2;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;

    for(dim = total_dim - 1; dim >= 0; dim--) {
      if(dim == dim_trav) {
	min = flat_size->values[dim] - 2;
	max = flat_size->values[dim] - 1;
      }
      else if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
      }
      else {
	min = 2;
	max = flat_size->values[dim] - 2;
      }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}

int Tuple_iterate_dest_2b_sel(Tuple *flat_size, Tuple *iter, int *idx)
{
    int dim, min, max;
    int total_dim = flat_size->dim;

    int dim_trav = 2;
    if(*idx < 0) { // Initialize iterator
      int mul = 1;
      *idx = 0;
      iter->dim = 4;
      //iter->height = 1;
      iter->values[0] = 2;
      iter->values[1] = 2;
      iter->values[2] = 0;
      iter->values[3] = 0;
      for(int i = 3; i >= 0; i--) {
	 *idx += iter->values[i] * mul;
	 mul *= flat_size->values[i];
      }
//       Tuple_init_zero(iter, total_dim);
//       for(int i = 0; i < total_dim-2; i++) iter->values[i] = 2;
//       //iter->values[3] = 0; // Using 4D in LU for convinience
//       iter->values[dim_trav] = 0;
//       *idx = Tuple_nd_to_1d_index(iter, flat_size);
      return 1;
    }

    int done = 0;
    for(dim = total_dim - 1; dim >= 0; dim--) {
     if(dim == 3) {
	min = 0;
	max = flat_size->values[dim];
     }
     else if(dim == dim_trav) {
	min = 1;
	max = 2;
     }
     else {
	min = 2;
	max = flat_size->values[dim] - 2;
     }
        if(iter->values[dim] + 1 < max)
        {
            iter->values[dim]++;
	    done = 1;
            break; // for(dim)
        }
        else
        {
            iter->values[dim] = min;
        }
    }

    if(done == 0)
        return 0; // invalid iterator increment
    else {
      if(dim == 3) { // Optimization: idx_new = idx_old++;
	  *idx = *idx + 1;
	}
	else {
	  //*idx = Tuple_nd_to_1d_index(iter, flat_size);
	  *idx = 0;
	  int mul = 1;
	  for(int i = 3; i >= 0; i--) {
	    *idx += iter->values[i] * mul;
	    mul *= flat_size->values[i];
	  }
	}
        return 1; // next element found
    }
}



#if 1

void sync_boundary_assign_sel(HTA* h, int total, int iter, int propag)
{
#if 1

//    Range lhs_tiles_range_x, lhs_tiles_range_y, lhs_tiles_range_z, lhs_tiles_range_w;
//    Range rhs_tiles_range_x, rhs_tiles_range_y, rhs_tiles_range_z, rhs_tiles_range_w;
//    Region lhs_tiles_sel, rhs_tiles_sel;
//    Range lhs_elem_range_x, lhs_elem_range_y, lhs_elem_range_z, lhs_elem_range_w;
//    Range rhs_elem_range_x, rhs_elem_range_y, rhs_elem_range_z, rhs_elem_range_w;
//    Region lhs_elem_sel, rhs_elem_sel;

   assert(h->height == 2); //FIXME
   int num_tiles = Tuple_product(&h->tiling); // Only work if height = 2
   int lhs_sel[num_tiles];
   int rhs_sel[num_tiles];

    // synchronize yx plane
    if(h->tiling.values[0] > 1) { // synchronize neighbors only if num of tiles at z direction is larger than 1
        if(propag > 0 &&  iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 0, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: x
	  HTA_fill_boolean_array(h, 0, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: x
	  //CREATE_ELEM_SELECTION(lhs,  0,  1,  0, -1,  0, -1); // z[0], y[all+2], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs, -4, -3,  0, -1,  0, -1); // z[n-1], y[all+2], x[all+2]
// 	  CREATE_ELEM_SELECTION(lhs,  0,  1,  2, -3,  2, -3); // z[0], y[all+2], x[all+2]
// 	  CREATE_ELEM_SELECTION(rhs, -4, -3,  2, -3,  2, -3); // z[n-1], y[all+2], x[all+2]
//        HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0b_sel, h, rhs_sel, Tuple_iterate_src_0b_sel, h);
	}
	if (propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 0, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: x
	  HTA_fill_boolean_array(h, 0, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: x
	  //CREATE_ELEM_SELECTION(lhs, -2, -1,  0, -1,  0, -1);
	  //CREATE_ELEM_SELECTION(rhs, 2, 3,  0, -1,  0, -1);
// 	  CREATE_ELEM_SELECTION(lhs, -2, -1,  2, -3,  2, -3);
// 	  CREATE_ELEM_SELECTION(rhs, 2, 3,  2, -3,  2, -3);
// 	  HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0_sel, h, rhs_sel, Tuple_iterate_src_0_sel, h);
	}
    }

    // synchronize zx plane (y direction)
    if(h->tiling.values[1] > 1) { // synchronize neighbors only if num of tiles at y direction is larger than 1
        if(propag > 0 && iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 1, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: y
	  HTA_fill_boolean_array(h, 1, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: y
	  //CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  0, -1); // z[all], y[0], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  0, -1); // z[all], y[n-1], x[all+2]
// 	  CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  2, -3); // z[all], y[0], x[all+2]
// 	  CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  2, -3); // z[all], y[n-1], x[all+2]
//        HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1b_sel, h, rhs_sel, Tuple_iterate_src_1b_sel, h);
	}
	if (propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 1, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: y
	  HTA_fill_boolean_array(h, 1, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: y
	  //CREATE_ELEM_SELECTION(lhs,  2, -3, -2, -1, 0, -1);
	  //CREATE_ELEM_SELECTION(rhs,  2, -3, 2, 3, 0, -1);
// 	  CREATE_ELEM_SELECTION(lhs,  2, -3, -2, -1, 2, -3);
// 	  CREATE_ELEM_SELECTION(rhs,  2, -3, 2, 3, 2, -3);
//        HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1_sel, h, rhs_sel, Tuple_iterate_src_1_sel, h);
	}
    }

    // 1 synchronize zy plane (x direction)
    if(h->tiling.values[2] > 1) { // synchronize neighbors only if num of tiles at x direction is larger than 1
        // select tiles in the LHS
        if(propag > 0 && iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 2, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: z
	  HTA_fill_boolean_array(h, 2, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: z
// 	  CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  0,  1);
// 	  CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, -4, -3);
//        HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2b_sel, h, rhs_sel, Tuple_iterate_src_2b_sel, h);
        }
	if (propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 2, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: z
	  HTA_fill_boolean_array(h, 2, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: z
/*	  CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  -2,  -1);
	  CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, 2, 3)*/;
//        HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2_sel, h, rhs_sel, Tuple_iterate_src_2_sel, h);
	}
    }

#else
    HTA_tile_to_hta2(HTA_LEAF_LEVEL(h), sync_boundary_HTA, h, h);
    HTA_barrier(pid);
#endif

}

#endif


void sync_boundary_assign(HTA* h)
{
#if 1
    Range lhs_elem_range_x, lhs_elem_range_y, lhs_elem_range_z, lhs_elem_range_w;
    Range lhs_tiles_range_x, lhs_tiles_range_y, lhs_tiles_range_z, lhs_tiles_range_w;
    Range rhs_elem_range_x, rhs_elem_range_y, rhs_elem_range_z, rhs_elem_range_w;
    Range rhs_tiles_range_x, rhs_tiles_range_y, rhs_tiles_range_z, rhs_tiles_range_w;
    Region lhs_elem_sel, rhs_elem_sel;
    Region lhs_tiles_sel, rhs_tiles_sel;

    int num_tiles = Tuple_product(&h->tiling);
    int lhs_sel[num_tiles];
    int rhs_sel[num_tiles];

        // synchronize yx plane
    //if(pid == 0) printf("processing z direction\n");
    if(h->tiling.values[0] > 1) { // synchronize neighbors only if num of tiles at z direction is larger than 1
        // 3.1 backward
        CREATE_TILE_SELECTION(lhs,  1, -1,  0, -1,  0, -1);
        CREATE_TILE_SELECTION(rhs,  0, -2,  0, -1,  0, -1);
	if(check_bound == -1) {
	  // elem selection not necessary if custom
// 	  CREATE_ELEM_SELECTION(lhs,  0,  1,  2, -3,  2, -3);
// 	  CREATE_ELEM_SELECTION(rhs, -4, -3,  2, -3,  2, -3);
// 	  HTA_assign_with_region(&lhs_tiles_sel, &lhs_elem_sel, h, &rhs_tiles_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0b, h, rhs_sel, Tuple_iterate_src_0b, h);
	}
	else {
	  CREATE_ELEM_SELECTION(lhs,  0,  1,  0, -1,  0, -1); // z[0], y[all+2], x[all+2]
	  CREATE_ELEM_SELECTION(rhs, -4, -3,  0, -1,  0, -1); // z[n-1], y[all+2], x[all+2]
// 	  HTA_assign_with_region(&lhs_tiles_sel, &lhs_elem_sel, h, &rhs_tiles_sel, &rhs_elem_sel, h);
 	  HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	}
        // 3.2 forward
	if(check_bound == -1) {
	  // elem selection not necessary if custom
// 	  CREATE_ELEM_SELECTION(lhs,  2,  3,  2, -3,  2, -3);
// 	  CREATE_ELEM_SELECTION(rhs, -2, -1,  2, -3,  2, -3);
// 	  HTA_assign_with_region(&rhs_tiles_sel, &rhs_elem_sel, h, &lhs_tiles_sel, &lhs_elem_sel, h);
	  HTA_assign_custom(rhs_sel, Tuple_iterate_dest_0, h, lhs_sel, Tuple_iterate_src_0, h);
	}
	else {
	  CREATE_ELEM_SELECTION(lhs,  2,  3,  0, -1,  0, -1); // z[1], y[all+2], x[all+2]
	  CREATE_ELEM_SELECTION(rhs, -2, -1,  0, -1,  0, -1); // z[n], y[all+2], x[all+2]
// 	  HTA_assign_with_region(&rhs_tiles_sel, &rhs_elem_sel, h, &lhs_tiles_sel, &lhs_elem_sel, h);
 	  HTA_assign(rhs_sel, &rhs_elem_sel, h, lhs_sel, &lhs_elem_sel, h);
	}
    }

    // synchronize zx plane (y direction)
    //if(pid == 0) printf("processing y direction\n");
    if(h->tiling.values[1] > 1) { // synchronize neighbors only if num of tiles at y direction is larger than 1
        // 2.1 upward
        CREATE_TILE_SELECTION(lhs,  0, -1,  1, -1,  0, -1);
        CREATE_TILE_SELECTION(rhs,  0, -1,  0, -2,  0, -1);
	if(check_bound == -1) {
	  // elem selection not necessary if custom
// 	  CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  2, -3);
// 	  CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  2, -3);
// 	  HTA_assign_with_region(&lhs_tiles_sel, &lhs_elem_sel, h, &rhs_tiles_sel, &rhs_elem_sel, h);
	  HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1b, h, rhs_sel, Tuple_iterate_src_1b, h);
	}
	else {
	  CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  0, -1); // z[all], y[0], x[all+2]
	  CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  0, -1); // z[all], y[n-1], x[all+2]
// 	  HTA_assign_with_region(&lhs_tiles_sel, &lhs_elem_sel, h, &rhs_tiles_sel, &rhs_elem_sel, h);
 	  HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
	}
        // 2.2 downward (lhs send, rhs receive)
	if(check_bound == -1) {
	  // elem selection not necessary if custom
// 	  CREATE_ELEM_SELECTION(lhs,  2, -3,  2,  3,  2, -3); // z[all], y[1], x[all+2]
// 	  CREATE_ELEM_SELECTION(rhs,  2, -3, -2, -1,  2, -3); // z[all], y[n], x[all+2]
// 	  HTA_assign_with_region(&rhs_tiles_sel, &rhs_elem_sel, h, &lhs_tiles_sel, &lhs_elem_sel, h);
	  HTA_assign_custom(rhs_sel, Tuple_iterate_dest_1, h, lhs_sel, Tuple_iterate_src_1, h);
	}
	else {
	  CREATE_ELEM_SELECTION(lhs,  2, -3,  2,  3,  0, -1); // z[all], y[1], x[all+2]
	  CREATE_ELEM_SELECTION(rhs,  2, -3, -2, -1,  0, -1); // z[all], y[n], x[all+2]
// 	  HTA_assign_with_region(&rhs_tiles_sel, &rhs_elem_sel, h, &lhs_tiles_sel, &lhs_elem_sel, h);
 	  HTA_assign(rhs_sel, &rhs_elem_sel, h, lhs_sel, &lhs_elem_sel, h);
	}
    }

    // 1 synchronize zy plane (x direction)
    if(h->tiling.values[2] > 1) { // synchronize neighbors only if num of tiles at x direction is larger than 1
        // 1.1 right
        // select tiles in the LHS
        CREATE_TILE_SELECTION(lhs, 0, -1, 0, -1,  1, -1);
        CREATE_TILE_SELECTION(rhs, 0, -1, 0, -1,  0, -2);
//         CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  0,  1);
//         CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, -4, -3);
//      HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
// 	HTA_assign_with_region(&lhs_tiles_sel, &lhs_elem_sel, h, &rhs_tiles_sel, &rhs_elem_sel, h);
	HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2b, h, rhs_sel, Tuple_iterate_src_2b, h);

        // 1.2 left
        // Tile selection is swapped, rhs here is the receiving end and lhs is the sending
//        CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  2,  3);
//        CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, -2, -1);
//      HTA_assign(rhs_sel, &rhs_elem_sel, h, lhs_sel, &lhs_elem_sel, h);
//      HTA_assign_with_region(&rhs_tiles_sel, &rhs_elem_sel, h, &lhs_tiles_sel, &lhs_elem_sel, h);
	HTA_assign_custom(rhs_sel, Tuple_iterate_dest_2, h, lhs_sel, Tuple_iterate_src_2, h);
    }


#else
    HTA_tile_to_hta2(HTA_LEAF_LEVEL(h), sync_boundary_HTA, h, h);
    HTA_barrier(pid);
#endif

}

#if 0

void sync_boundary_assign_sel(HTA* h, int total, int iter, int propag)
{
#if 1

   Range lhs_tiles_range_x, lhs_tiles_range_y, lhs_tiles_range_z, lhs_tiles_range_w;
   Range rhs_tiles_range_x, rhs_tiles_range_y, rhs_tiles_range_z, rhs_tiles_range_w;
   Region lhs_tiles_sel, rhs_tiles_sel;
//    Range lhs_elem_range_x, lhs_elem_range_y, lhs_elem_range_z, lhs_elem_range_w;
//    Range rhs_elem_range_x, rhs_elem_range_y, rhs_elem_range_z, rhs_elem_range_w;
//    Region lhs_elem_sel, rhs_elem_sel;

   assert(h->height == 2); //FIXME
   int num_tiles = Tuple_product(&h->tiling); // Only work if height = 2
   int lhs_sel[num_tiles];
   int rhs_sel[num_tiles];

    // 1 synchronize zy plane (x direction)
    if(h->tiling.values[2] > 1) { // synchronize neighbors only if num of tiles at x direction is larger than 1
        // 1.1 right
        // select tiles in the LHS
        if(propag >= 0 && iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 2, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: z
	  HTA_fill_boolean_array(h, 2, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: z
	  //CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  0,  1);
	  //CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, -4, -3);
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2b, h, rhs_sel, Tuple_iterate_src_2b, h);
	}
	else if(propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 2, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: z
	  HTA_fill_boolean_array(h, 2, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: z
	  //CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  -2,  -1);
	  //CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, 2, 3);
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2, h, rhs_sel, Tuple_iterate_src_2, h);
	}
	else if(propag < 0 && iter == 0) { // // sync forward last iter of buts
	  CREATE_TILE_SELECTION(lhs, 0, -1, 0, -1,  1, -1);
	  CREATE_TILE_SELECTION(rhs, 0, -1, 0, -1,  0, -2);
	  //CREATE_ELEM_SELECTION(lhs, 2, -3, 2, -3,  0,  1);
	  //CREATE_ELEM_SELECTION(rhs, 2, -3, 2, -3, -4, -3);
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_2b, h, rhs_sel, Tuple_iterate_src_2b, h);
	}
    }

    // synchronize zx plane (y direction)
    if(h->tiling.values[1] > 1) { // synchronize neighbors only if num of tiles at y direction is larger than 1
        // 2.1 upward
        if(propag >= 0 && iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 1, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: y
	  HTA_fill_boolean_array(h, 1, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: y
	  //CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  2, -3); // z[all], y[0], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  2, -3); // z[all], y[n-1], x[all+2]
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1b, h, rhs_sel, Tuple_iterate_src_1b, h);
	}
	else if (propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 1, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: y
	  HTA_fill_boolean_array(h, 1, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: y
	  //CREATE_ELEM_SELECTION(lhs,  2, -3, -2, -1, 2, -3);
	  //CREATE_ELEM_SELECTION(rhs,  2, -3, 2, 3, 2, -3);
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1, h, rhs_sel, Tuple_iterate_src_1, h);
	}
	else if(propag < 0 && iter == 0) { // sync forward last iter of buts
	  CREATE_TILE_SELECTION(lhs,  0, -1,  1, -1,  0, -1);
	  CREATE_TILE_SELECTION(rhs,  0, -1,  0, -2,  0, -1);
	  //CREATE_ELEM_SELECTION(lhs,  2, -3,  0,  1,  2, -3); // z[all], y[0], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs,  2, -3, -4, -3,  2, -3); // z[all], y[n-1], x[all+2]
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_1b, h, rhs_sel, Tuple_iterate_src_1b, h);
	}
    }

    // synchronize yx plane
    if(h->tiling.values[0] > 1) { // synchronize neighbors only if num of tiles at z direction is larger than 1
        // 3.1 backward
        if(propag >= 0 && iter < total) { // Propagation increasing along dimension
	  HTA_fill_boolean_array(h, 0, 0, 1, iter, rhs_sel); // rhs, propag: inc, dim: x
	  HTA_fill_boolean_array(h, 0, 1, 1, iter+1, lhs_sel); // lhs, propag: inc, dim: x
	  //CREATE_ELEM_SELECTION(lhs,  0,  1,  2, -3,  2, -3); // z[0], y[all+2], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs, -4, -3,  2, -3,  2, -3); // z[n-1], y[all+2], x[all+2]
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0b, h, rhs_sel, Tuple_iterate_src_0b, h);
	}
	else if (propag < 0 && iter > 0) {
	  HTA_fill_boolean_array(h, 0, 0, -1, iter, rhs_sel); // rhs, propag: dec, dim: x
	  HTA_fill_boolean_array(h, 0, 1, -1, iter-1, lhs_sel); // lhs, propag: dec, dim: x
	  //CREATE_ELEM_SELECTION(lhs, -2, -1,  2, -3,  2, -3);
	  //CREATE_ELEM_SELECTION(rhs, 2, 3,  2, -3,  2, -3);
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0, h, rhs_sel, Tuple_iterate_src_0, h);
	}
	else if(propag < 0 && iter == 0) { // sync forward last iter of buts
	  CREATE_TILE_SELECTION(lhs,  1, -1,  0, -1,  0, -1);
	  CREATE_TILE_SELECTION(rhs,  0, -2,  0, -1,  0, -1);
	  //CREATE_ELEM_SELECTION(lhs,  0,  1,  2, -3,  2, -3); // z[0], y[all+2], x[all+2]
	  //CREATE_ELEM_SELECTION(rhs, -4, -3,  2, -3,  2, -3); // z[n-1], y[all+2], x[all+2]
          //HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);
          HTA_assign_custom(lhs_sel, Tuple_iterate_dest_0b, h, rhs_sel, Tuple_iterate_src_0b, h);
	}
    }

#else
    HTA_tile_to_hta2(HTA_LEAF_LEVEL(h), sync_boundary_HTA, h, h);
    HTA_barrier(pid);
#endif

}

#endif

//  FIXME: this should be a collective operation instead of a map
void sync_boundary(int pid, HTA* h)
{
    HTA_tile_to_hta2(HTA_LEAF_LEVEL(h), sync_boundary_HTA, h, h);
    HTA_barrier(pid);
}




// Actively write necessary boundaries from other neighbor tiles
// Writes don't have to wait because all writes go directly to where
// the boundaries are.

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
     n0t = t->flat_size.values[3]; \
     n1t = t->flat_size.values[2]; \
     n2t = t->flat_size.values[1]; \
     n3t = t->flat_size.values[0]; \
     double (*zt)[n2t][n1t][n0t] = (double (*)[n2t][n1t][n0t])HTA_get_ptr_raw_data(t); \

void sync_boundary_HTA(HTA* s1_tile, HTA* s2)
{
    HTA* t;
    int n0t, n1t, n2t, n3t;
    int i1, i2, i3, m;

    int n0s = s1_tile->flat_size.values[3];
    int n1s = s1_tile->flat_size.values[2];
    int n2s = s1_tile->flat_size.values[1];
    int n3s = s1_tile->flat_size.values[0];

    double (*zs)[n2s][n1s][n0s] = (double (*)[n2s][n1s][n0s])HTA_get_ptr_raw_data(s1_tile);

    // Get global tile nd_index first
    Tuple* nd_size = &s2->tiling; // tile dimensions
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    Tuple target_idx;
    Tuple_init_zero(&target_idx, 4); // target tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);

    int x = nd_idx.values[2];
    int y = nd_idx.values[1];
    int z = nd_idx.values[0];

    if(check_bound < 0 || (x + y + z) == check_bound) {

    //printf("(%d) Synchronizing boundaries tile (%d,%d,%d) -- check = %d\n", s1_tile->pid, x, y, z, check_bound);

    // Update the surfaces
    // x+1
    X_PLUS_1();
    Y_SAME();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
            for(i2 = 2; i2 < n2t - 2; i2++)
                for(m = 0; m < n0s; m++) {
		  zt[i3][i2][0][m] = zs[i3][i2][n1s-4][m];
		  zt[i3][i2][1][m] = zs[i3][i2][n1s-3][m];
		}
    }
    // x-1
    X_MINUS_1();
    Y_SAME();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
            for(i2 = 2; i2 < n2t - 2; i2++)
	      for(m = 0; m < n0s; m++) {
		zt[i3][i2][n1t-2][m] = zs[i3][i2][2][m];
		zt[i3][i2][n1t-1][m] = zs[i3][i2][3][m];
	      }
    }

    // y+1
    X_SAME();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
            for(i1 = 2; i1 < n1t - 2; i1++)
	      for(m = 0; m < n0s; m++) {
		zt[i3][0][i1][m] = zs[i3][n2s-4][i1][m];
		zt[i3][1][i1][m] = zs[i3][n2s-3][i1][m];
	      }
    }
    // y-1
    X_SAME();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
            for(i1 = 2; i1 < n1t - 2; i1++)
	      for(m = 0; m < n0s; m++) {
		zt[i3][n2t-2][i1][m] = zs[i3][2][i1][m];
		zt[i3][n2t-1][i1][m] = zs[i3][3][i1][m];
	      }
    }
    // z+1
    X_SAME();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2= 2; i2 < n2t - 2; i2++)
            for(i1 = 2; i1 < n1t - 2; i1++)
	      for(m = 0; m < n0s; m++) {
		zt[0][i2][i1][m] = zs[n3s-4][i2][i1][m];
		zt[1][i2][i1][m] = zs[n3s-3][i2][i1][m];

	      }
    }
    // z-1
    X_SAME();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2= 2; i2 < n2t - 2; i2++)
            for(i1 = 2; i1 < n1t - 2; i1++)
	      for(m = 0; m < n0s; m++) {
		zt[n3t-2][i2][i1][m] = zs[2][i2][i1][m];
		zt[n3t-1][i2][i1][m] = zs[3][i2][i1][m];
	      }
    }
   } // End If condition

   if(check_bound == -2) { // Only for pintgr.c
    // Update the vectors
    // x+1, y+1, z
    X_PLUS_1();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
	  for(m = 0; m < n0s; m++) {
	    zt[i3][0][0][m] = zs[i3][n2s-4][n1s-4][m];
	    zt[i3][0][1][m] = zs[i3][n2s-4][n1s-3][m];
	    zt[i3][1][0][m] = zs[i3][n2s-3][n1s-4][m];
	    zt[i3][1][1][m] = zs[i3][n2s-3][n1s-3][m];
	  }
    }
    // x+1, y-1, z
    X_PLUS_1();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
	  for(m = 0; m < n0s; m++) {
	    zt[i3][n2t-1][0][m] = zs[i3][3][n1s-4][m];
	    zt[i3][n2t-1][1][m] = zs[i3][3][n1s-3][m];
	    zt[i3][n2t-2][0][m] = zs[i3][2][n1s-4][m];
	    zt[i3][n2t-2][1][m] = zs[i3][2][n1s-3][m];
	  }
    }
    // x-1, y-1, z
    X_MINUS_1();
    Y_MINUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
	   for(m = 0; m < n0s; m++) {
	    zt[i3][n2t-1][n1t-1][m] = zs[i3][3][3][m];
	    zt[i3][n2t-1][n1t-2][m] = zs[i3][3][2][m];
	    zt[i3][n2t-2][n1t-1][m] = zs[i3][2][3][m];
	    zt[i3][n2t-2][n1t-2][m] = zs[i3][2][2][m];
	   }
    }
    // x-1, y+1, z
    X_MINUS_1();
    Y_PLUS_1();
    Z_SAME();
    {
        GET_TILE_PTR()
        for(i3 = 2; i3 < n3t - 2; i3++)
	  for(m = 0; m < n0s; m++) {
            zt[i3][0][n1t-1][m] = zs[i3][n2s-4][3][m];
	    zt[i3][1][n1t-1][m] = zs[i3][n2s-3][3][m];
	    zt[i3][0][n1t-2][m] = zs[i3][n2s-4][2][m];
	    zt[i3][1][n1t-2][m] = zs[i3][n2s-3][2][m];
	  }
    }
    // x, y+1, z+1
    X_SAME();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 2; i1 < n1t - 2; i1++)
	  for(m = 0; m < n0s; m++) {
	    zt[0][0][i1][m] = zs[n3s-4][n2s-4][i1][m];
	    zt[0][1][i1][m] = zs[n3s-4][n2s-3][i1][m];
	    zt[1][0][i1][m] = zs[n3s-3][n2s-4][i1][m];
	    zt[1][1][i1][m] = zs[n3s-3][n2s-3][i1][m];
	  }
    }
    // x, y-1, z+1
    X_SAME();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 2; i1 < n1t - 2; i1++)
	   for(m = 0; m < n0s; m++) {
	     zt[0][n2t-1][i1][m] = zs[n3s-4][3][i1][m];
	     zt[0][n2t-2][i1][m] = zs[n3s-4][2][i1][m];
	     zt[1][n2t-1][i1][m] = zs[n3s-3][3][i1][m];
	     zt[1][n2t-2][i1][m] = zs[n3s-3][2][i1][m];
	   }
    }
    // x, y-1, z-1
    X_SAME();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 2; i1 < n1t - 2; i1++)
	  for(m = 0; m < n0s; m++) {
	    zt[n3t-1][n2t-1][i1][m] = zs[3][3][i1][m];
	    zt[n3t-1][n2t-2][i1][m] = zs[3][2][i1][m];
	    zt[n3t-2][n2t-1][i1][m] = zs[2][3][i1][m];
	    zt[n3t-2][n2t-2][i1][m] = zs[2][2][i1][m];
	  }
    }
    // x, y+1, z-1
    X_SAME();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i1 = 2; i1 < n1t - 2; i1++)
	   for(m = 0; m < n0s; m++) {
	   zt[n3t-1][0][i1][m] = zs[3][n2s-4][i1][m];
	   zt[n3t-1][1][i1][m] = zs[3][n2s-3][i1][m];
	   zt[n3t-2][0][i1][m] = zs[2][n2s-4][i1][m];
	   zt[n3t-2][1][i1][m] = zs[2][n2s-3][i1][m];
	  }
    }
    // x+1, y, z+1
    X_PLUS_1();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 2; i2 < n2t - 2; i2++)
	  for(m = 0; m < n0s; m++) {
	    zt[0][i2][0][m] = zs[n3s-4][i2][n1s-4][m];
	    zt[0][i2][1][m] = zs[n3s-4][i2][n1s-3][m];
	    zt[1][i2][0][m] = zs[n3s-3][i2][n1s-4][m];
	    zt[1][i2][1][m] = zs[n3s-3][i2][n1s-3][m];
	  }
    }
    // x-1, y, z+1
    X_MINUS_1();
    Y_SAME();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 2; i2 < n2t - 2; i2++)
	  for(m = 0; m < n0s; m++) {
	    zt[0][i2][n1t-1][m] = zs[n3s-4][i2][3][m];
	    zt[0][i2][n1t-2][m] = zs[n3s-4][i2][2][m];
	    zt[1][i2][n1t-1][m] = zs[n3s-3][i2][3][m];
	    zt[1][i2][n1t-2][m] = zs[n3s-3][i2][2][m];
	  }
    }
    // x-1, y, z-1
    X_MINUS_1();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 2; i2 < n2t - 2; i2++)
	   for(m = 0; m < n0s; m++) {
	     zt[n3t-1][i2][n1t-1][m] = zs[3][i2][3][m];
	     zt[n3t-1][i2][n1t-2][m] = zs[3][i2][2][m];
	     zt[n3t-2][i2][n1t-1][m] = zs[2][i2][3][m];
	     zt[n3t-2][i2][n1t-2][m] = zs[2][i2][2][m];
	   }
    }
    // x+1, y, z-1
    X_PLUS_1();
    Y_SAME();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
        for(i2 = 2; i2 < n2t - 2; i2++)
	  for(m = 0; m < n0s; m++) {
	    zt[n3t-1][i2][0][m] = zs[3][i2][n1s-4][m];
	    zt[n3t-1][i2][1][m] = zs[3][i2][n1s-3][m];
	    zt[n3t-2][i2][0][m] = zs[2][i2][n1s-4][m];
	    zt[n3t-2][i2][1][m] = zs[2][i2][n1s-3][m];
	  }
    }

    // Update points
    // x+1, y+1, z+1
    X_PLUS_1();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[0][0][0][m] = zs[n3s-4][n2s-4][n1s-4][m];
	  zt[0][0][1][m] = zs[n3s-4][n2s-4][n1s-3][m];
	  zt[0][1][0][m] = zs[n3s-4][n2s-3][n1s-4][m];
	  zt[0][1][1][m] = zs[n3s-4][n2s-3][n1s-3][m];
	  zt[1][0][0][m] = zs[n3s-3][n2s-4][n1s-4][m];
	  zt[1][0][1][m] = zs[n3s-3][n2s-4][n1s-3][m];
	  zt[1][1][0][m] = zs[n3s-3][n2s-3][n1s-4][m];
	  zt[1][1][1][m] = zs[n3s-3][n2s-3][n1s-3][m];
	}
    }
    // x+1, y+1, z-1
    X_PLUS_1();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[n3t-1][0][0][m] = zs[3][n2s-4][n1s-4][m];
	  zt[n3t-1][0][1][m] = zs[3][n2s-4][n1s-3][m];
	  zt[n3t-1][1][0][m] = zs[3][n2s-3][n1s-4][m];
	  zt[n3t-1][1][1][m] = zs[3][n2s-3][n1s-3][m];
	  zt[n3t-2][0][0][m] = zs[2][n2s-4][n1s-4][m];
	  zt[n3t-2][0][1][m] = zs[2][n2s-4][n1s-3][m];
	  zt[n3t-2][1][0][m] = zs[2][n2s-3][n1s-4][m];
	  zt[n3t-2][1][1][m] = zs[2][n2s-3][n1s-3][m];
	}
    }
    // x+1, y-1, z+1
    X_PLUS_1();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[0][n2t-1][0][m] = zs[n3s-4][3][n1s-4][m];
	  zt[0][n2t-1][1][m] = zs[n3s-4][3][n1s-3][m];
	  zt[0][n2t-2][0][m] = zs[n3s-4][2][n1s-4][m];
	  zt[0][n2t-2][1][m] = zs[n3s-4][2][n1s-3][m];
	  zt[1][n2t-1][0][m] = zs[n3s-3][3][n1s-4][m];
	  zt[1][n2t-1][1][m] = zs[n3s-3][3][n1s-3][m];
	  zt[1][n2t-2][0][m] = zs[n3s-3][2][n1s-4][m];
	  zt[1][n2t-2][1][m] = zs[n3s-3][2][n1s-3][m];
	}
    }
    // x+1, y-1, z-1
    X_PLUS_1();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[n3t-1][n2t-1][0][m] = zs[3][3][n1s-4][m];
	  zt[n3t-1][n2t-1][1][m] = zs[3][3][n1s-3][m];
	  zt[n3t-1][n2t-2][0][m] = zs[3][2][n1s-4][m];
	  zt[n3t-1][n2t-2][1][m] = zs[3][2][n1s-3][m];
	  zt[n3t-2][n2t-1][0][m] = zs[2][3][n1s-4][m];
	  zt[n3t-2][n2t-1][1][m] = zs[2][3][n1s-3][m];
	  zt[n3t-2][n2t-2][0][m] = zs[2][2][n1s-4][m];
	  zt[n3t-2][n2t-2][1][m] = zs[2][2][n1s-3][m];
	}
    }
    // x-1, y+1, z+1
    X_MINUS_1();
    Y_PLUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[0][0][n1t-1][m] = zs[n3s-4][n2s-4][3][m];
	  zt[0][0][n1t-2][m] = zs[n3s-4][n2s-4][2][m];
	  zt[0][1][n1t-1][m] = zs[n3s-4][n2s-3][3][m];
	  zt[0][1][n1t-2][m] = zs[n3s-4][n2s-3][2][m];
	  zt[1][0][n1t-1][m] = zs[n3s-3][n2s-4][3][m];
	  zt[1][0][n1t-2][m] = zs[n3s-3][n2s-4][2][m];
	  zt[1][1][n1t-1][m] = zs[n3s-3][n2s-3][3][m];
	  zt[1][1][n1t-2][m] = zs[n3s-3][n2s-3][2][m];
	}
    }
    // x-1, y+1, z-1
    X_MINUS_1();
    Y_PLUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[n3t-1][0][n1t-1][m] = zs[3][n2s-4][3][m];
	  zt[n3t-1][0][n1t-2][m] = zs[3][n2s-4][2][m];
	  zt[n3t-1][1][n1t-1][m] = zs[3][n2s-3][3][m];
	  zt[n3t-1][1][n1t-2][m] = zs[3][n2s-3][2][m];
	  zt[n3t-2][0][n1t-1][m] = zs[2][n2s-4][3][m];
	  zt[n3t-2][0][n1t-2][m] = zs[2][n2s-4][2][m];
	  zt[n3t-2][1][n1t-1][m] = zs[2][n2s-3][3][m];
	  zt[n3t-2][1][n1t-2][m] = zs[2][n2s-3][2][m];
	}
    }
    // x-1, y-1, z+1
    X_MINUS_1();
    Y_MINUS_1();
    Z_PLUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[0][n2t-1][n1t-1][m] = zs[n3s-4][3][3][m];
	  zt[0][n2t-1][n1t-2][m] = zs[n3s-4][3][2][m];
	  zt[0][n2t-2][n1t-1][m] = zs[n3s-4][2][3][m];
	  zt[0][n2t-2][n1t-2][m] = zs[n3s-4][2][2][m];
	  zt[1][n2t-1][n1t-1][m] = zs[n3s-3][3][3][m];
	  zt[1][n2t-1][n1t-2][m] = zs[n3s-3][3][2][m];
	  zt[1][n2t-2][n1t-1][m] = zs[n3s-3][2][3][m];
	  zt[1][n2t-2][n1t-2][m] = zs[n3s-3][2][2][m];
	}
    }
    // x-1, y-1, z-1
    X_MINUS_1();
    Y_MINUS_1();
    Z_MINUS_1();
    {
        GET_TILE_PTR()
	for(m = 0; m < n0s; m++) {
	  zt[n3t-1][n2t-1][n1t-1][m] = zs[3][3][3][m];
	  zt[n3t-1][n2t-1][n1t-2][m] = zs[3][3][2][m];
	  zt[n3t-1][n2t-2][n1t-1][m] = zs[3][2][3][m];
	  zt[n3t-1][n2t-2][n1t-2][m] = zs[3][2][2][m];
	  zt[n3t-2][n2t-1][n1t-1][m] = zs[2][3][3][m];
	  zt[n3t-2][n2t-1][n1t-2][m] = zs[2][3][2][m];
	  zt[n3t-2][n2t-2][n1t-1][m] = zs[2][2][3][m];
	  zt[n3t-2][n2t-2][n1t-2][m] = zs[2][2][2][m];
	}
    }
  } // End if condition
}

