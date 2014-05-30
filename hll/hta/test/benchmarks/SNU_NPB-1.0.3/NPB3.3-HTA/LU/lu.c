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

int PROC_X = 1;
int PROC_Y = 1;
int PROC_Z = 1;

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
double dt, omega, tolrsd[5], rsdnm[5], errnm[5], frc, ttotal;
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

int hta_main(int argc, char *argv[])
{
  char Class;
  logical verified;
  double mflops;

  double t, tmax, trecs[t_last+1];
  int i;
  char *t_names[t_last+3];
  int PROC;
  
  if (argc >= 4)
  {
      PROC_X = atoi(argv[3]);
      PROC_Y = atoi(argv[2]);
      PROC_Z = atoi(argv[1]);
  } 
  
  if (argc == 5) PROC = atoi(argv[4]);
  else PROC = omp_get_max_threads();
  
  //---------------------------------------------------------------------
  // Setup info for timers
  //---------------------------------------------------------------------
  FILE *fp;
  if ((fp = fopen("timer.flag", "r")) != NULL) {
    timeron = true;
    t_names[t_total] = "total";
    t_names[t_rhsx] = "rhsx";
    t_names[t_rhsy] = "rhsy";
    t_names[t_rhsz] = "rhsz";
    t_names[t_rhs] = "rhs";
    t_names[t_jacld] = "jacld";
    t_names[t_blts] = "blts";
    t_names[t_jacu] = "jacu";
    t_names[t_buts] = "buts";
    t_names[t_add] = "add";
    t_names[t_l2norm] = "l2norm";
    t_names[t_sync_blts] = "sync_blts";
    t_names[t_sync_buts] = "sync_buts";
    fclose(fp);
  } else {
    timeron = false;
  }
  
  //---------------------------------------------------------------------
  // HTA definitions
  //---------------------------------------------------------------------
  Tuple t1 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple fs1 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 5);
  Dist dist0;
  Dist_init(&dist0, 0);
  u_HTA = HTA_create(4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  rsd_HTA = HTA_create(4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  frct_HTA = HTA_create(4, 2, &fs1, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t1);
  //---------------------------------------------------------------------
  Tuple t2 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1); // Using 4D for convenience
  Tuple fs2 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 1);
  rho_i_HTA = HTA_create(4, 2, &fs2, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t2);
  qs_HTA = HTA_create(4, 2, &fs2, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t2);
  //---------------------------------------------------------------------
//  Tuple t3 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
//  Tuple fs3 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 25); // Original size: [Isiz2][Isiz1][5][5]
//  a_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   b_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   c_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   d_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   au_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   bu_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   cu_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
//   du_HTA = HTA_create(4, 2, &fs3, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t3);
  //---------------------------------------------------------------------
  Tuple t4 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple fs4 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 5); 
  sum_HTA = HTA_create(4, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t4); // Required by l2norm
  err_HTA = HTA_create(4, 2, &fs4, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t4); // Required by error
  //---------------------------------------------------------------------
  
  //---------------------------------------------------------------------
  // read input data
  //---------------------------------------------------------------------
  read_input();

  printf(" Using (z : %d, y : %d, x : %d) tiles.\n\n", PROC_Z, PROC_Y, PROC_X);
//   if(ISIZ3%PROC_Z != 0 || ISIZ2%PROC_Y != 0 || ISIZ1%PROC_X != 0) {
//     printf(" Error! Only regular tiles are allowed in this implementation. Change PROC and TILES values.\n\n");
//     exit(1);
//   }
  //---------------------------------------------------------------------
  // set up domain sizes
  //---------------------------------------------------------------------
  domain();

  //---------------------------------------------------------------------
  // set up coefficients
  //---------------------------------------------------------------------
  setcoeff();

  //---------------------------------------------------------------------
  // set the boundary values for dependent variables
  //---------------------------------------------------------------------
  setbv_HTA();

  //---------------------------------------------------------------------
  // set the initial values for dependent variables
  //---------------------------------------------------------------------
  setiv_HTA();

  //---------------------------------------------------------------------
  // compute the forcing term based on prescribed exact solution
  //---------------------------------------------------------------------
  erhs_HTA();

  //---------------------------------------------------------------------
  // perform one SSOR iteration to touch all data pages
  //---------------------------------------------------------------------
  ssor_HTA(1);
 
  //---------------------------------------------------------------------
  // reset the boundary and initial values
  //---------------------------------------------------------------------
  setbv_HTA();
  setiv_HTA();
 
  //---------------------------------------------------------------------
  // perform the SSOR iterations
  //---------------------------------------------------------------------
  ssor_HTA(itmax);
  
  //---------------------------------------------------------------------
  // compute the solution error
  //---------------------------------------------------------------------
  error_HTA(errnm);
  //---------------------------------------------------------------------
  // compute the surface integral
  //---------------------------------------------------------------------
  pintgr_HTA();

  //---------------------------------------------------------------------
  // verification test
  //---------------------------------------------------------------------
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
    sprintf(rec_name, "rec/lu.%c.%d.rec", Class, PROC);
    FILE* fp_rec = fopen(rec_name, "a");
    fprintf(fp_rec, "%9.4f ", tmax);
    
    printf("  SECTION     Time (secs)\n");
    for (i = 1; i <= t_last; i++) {     
      if (i == 1 || i > 4) fprintf(fp_rec, "%9.4f ", trecs[i]);
      printf("  %-8s:%9.4f  (%6.2f%%)\n",
          t_names[i], trecs[i], trecs[i]*100./tmax);
      if (i == t_rhs) {
        t = trecs[t_rhsx] + trecs[t_rhsy] + trecs[t_rhsz];
        printf("     --> %8s:%9.4f  (%6.2f%%)\n", "sub-rhs", t, t*100./tmax);
        t = trecs[i] - t;
        printf("     --> %8s:%9.4f  (%6.2f%%)\n", "rest-rhs", t, t*100./tmax);
      }
    }
    printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[12], timer_read(12), timer_read(12)*100./tmax);
    printf("  %-8s:%9.3f  (%6.2f%%)\n", t_names[13], timer_read(13), timer_read(13)*100./tmax);
    fprintf(fp_rec, "\n");
    fclose(fp_rec);
  }

  return 0;
}
 
//  FIXME: this should be a collective operation instead of a map
void sync_boundary(HTA* h)
{
    HTA_tile_to_hta(HTA_LEAF_LEVEL(h), sync_boundary_HTA, h, h, h);
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

void sync_boundary_HTA(HTA* d_tile, HTA* s1_tile, HTA* s2)
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
    Tuple* nd_size = s2->tiling; // tile dimensions
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    Tuple target_idx;
    Tuple_init_zero(&target_idx, 4); // target tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);

    int x = nd_idx.values[2];
    int y = nd_idx.values[1];
    int z = nd_idx.values[0];
    
    if(check_bound < 0 || (x + y + z) == check_bound) {
    
    //printf("Synchronizing boundaries tile (%d,%d,%d) -- check = %d\n", x, y, z, check_bound);
    
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

