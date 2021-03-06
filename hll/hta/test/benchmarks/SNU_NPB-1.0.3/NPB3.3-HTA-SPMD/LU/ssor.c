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

#include <stdio.h>
// #ifdef _OPENMP
// #include <omp.h>
// #endif
#include "applu.incl"
#include "timers.h"

//---------------------------------------------------------------------
// Thread synchronization for pipeline operation
//---------------------------------------------------------------------
// /* common /threadinfo1/ */
// int isync[ISIZ2+1];
// /* common /threadinfo2/ */
// int mthreadnum, iam;
// #pragma omp threadprivate(mthreadnum,iam)

//---------------------------------------------------------------------
// to perform pseudo-time stepping SSOR iterations
// for five nonlinear pde's.
//---------------------------------------------------------------------
void ssor_HTA(int niter, int pid, double rsd[5], HTA *u_HTA, HTA *rsd_HTA, HTA *frct_HTA, HTA *qs_HTA,
	      HTA *rho_i_HTA, HTA *sum_HTA)
{
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, wavefront;
  int istep;
  double tmp, tmp2;
  double delunm[5];

  double zero = 0.0;

  SET_SPMD_PID(pid)

  //---------------------------------------------------------------------
  // begin pseudo-time stepping iterations
  //---------------------------------------------------------------------
  tmp = 1.0 / ( omega * ( 2.0 - omega ) );

  //---------------------------------------------------------------------
  // initialize a,b,c,d to zero (guarantees that page tables have been
  // formed, if applicable on given architecture, before timestepping).
  //---------------------------------------------------------------------
  HTA_map_h1s1(HTA_LEAF_LEVEL(rsd_HTA), H1S1_INIT, rsd_HTA, &zero);

  MASTER_EXEC_START
  for (i = 1; i <= t_last; i++) {
    timer_clear(i);
  }
  MASTER_EXEC_END
  //---------------------------------------------------------------------
  // compute the steady-state residuals
  //---------------------------------------------------------------------
  rhs_HTA(pid, u_HTA, rsd_HTA, frct_HTA, qs_HTA, rho_i_HTA);

  //---------------------------------------------------------------------
  // compute the L2 norms of newton iteration residuals
  //---------------------------------------------------------------------
  l2norm_HTA(pid, rsd, rsd_HTA, sum_HTA);

  MASTER_EXEC_START
  for (i = 1; i <= t_last; i++) {
     timer_clear(i);
   }
  timer_start(1);
  MASTER_EXEC_END

  // Number of iteration of the wavefront computations
  wavefront = PROC_X + PROC_Y + PROC_Z - 3;
  MASTER_PRINTF(" Wavefront iterations: %d\n", wavefront+1);

  //---------------------------------------------------------------------
  // the timestep loop
  //---------------------------------------------------------------------
  for (istep = 1; istep <= niter; istep++) {
    if ((istep % 20) == 0 || istep == itmax || istep == 1) {
      if (niter > 1) MASTER_PRINTF(" Time step %4d\n", istep);
    }

    //---------------------------------------------------------------------
    // perform SSOR iteration
    //---------------------------------------------------------------------
    //MASTER_EXEC_START
    //if (timeron) timer_start(t_rhs);
    //MASTER_EXEC_END
    //tmp2 = dt;
    //HTA_map_h1s1(HTA_LEAF_LEVEL(rsd_HTA), mulscalar, rsd_HTA, &tmp2);
    // Now included in blts
    //MASTER_EXEC_START
    //if (timeron) timer_stop(t_rhs);
    //MASTER_EXEC_END
    //  ---------------------------------------------------------------------
    //  form the lower triangular part of the jacobian matrix
    //  ---------------------------------------------------------------------
    //if (timeron) timer_start(t_jacld);
    //jacld_HTA();
    //if (timeron) timer_stop(t_jacld);

    //---------------------------------------------------------------------
    // perform the lower triangular solution
    //---------------------------------------------------------------------
    MASTER_EXEC_START
    if (timeron) timer_start(t_blts);
    MASTER_EXEC_END
    blts_HTA(wavefront, pid, u_HTA, rsd_HTA, qs_HTA, rho_i_HTA);
    MASTER_EXEC_START
    if (timeron) timer_stop(t_blts);
    MASTER_EXEC_END
    //---------------------------------------------------------------------
    // form the strictly upper triangular part of the jacobian matrix
    //---------------------------------------------------------------------
    //if (timeron) timer_start(t_jacu);
    //jacu_HTA(); // Loop is not necessary
    //if (timeron) timer_stop(t_jacu);
    //---------------------------------------------------------------------
    // perform the upper triangular solution
    //---------------------------------------------------------------------
    MASTER_EXEC_START
    if (timeron) timer_start(t_buts);
    MASTER_EXEC_END
    buts_HTA(wavefront, pid, u_HTA, rsd_HTA, qs_HTA, rho_i_HTA);
    MASTER_EXEC_START
    if (timeron) timer_stop(t_buts);
    MASTER_EXEC_END
    //---------------------------------------------------------------------
    // update the variables
    //---------------------------------------------------------------------
    MASTER_EXEC_START
    if (timeron) timer_start(t_add);
    MASTER_EXEC_END
    //tmp2 = tmp;
    //HTA_map_h2s1(HTA_LEAF_LEVEL(rsd_HTA), muladd, u_HTA, rsd_HTA, &tmp2);
    // Now included in buts
    sync_boundary_assign(u_HTA);
    MASTER_EXEC_START
    if (timeron) timer_stop(t_add);
    MASTER_EXEC_END
    //---------------------------------------------------------------------
    // compute the max-norms of newton iteration corrections
    //---------------------------------------------------------------------
    if ( (istep % inorm) == 0 ) {
      MASTER_EXEC_START
      if (timeron) timer_start(t_l2norm);
      MASTER_EXEC_END
      l2norm_HTA(pid, delunm, rsd_HTA, sum_HTA);
      MASTER_EXEC_START
      if (timeron) timer_stop(t_l2norm);
      MASTER_EXEC_END
      /*
      if ( ipr == 1 ) {
        printf(" \n RMS-norm of SSOR-iteration correction "
               "for first pde  = %12.5E\n"
               " RMS-norm of SSOR-iteration correction "
               "for second pde = %12.5E\n"
               " RMS-norm of SSOR-iteration correction "
               "for third pde  = %12.5E\n"
               " RMS-norm of SSOR-iteration correction "
               "for fourth pde = %12.5E\n",
               " RMS-norm of SSOR-iteration correction "
               "for fifth pde  = %12.5E\n",
               delunm[0], delunm[1], delunm[2], delunm[3], delunm[4]);
      } else if ( ipr == 2 ) {
        printf("(%5d,%15.6f)\n", istep, delunm[4]);
      }
      */
    }

    //---------------------------------------------------------------------
    // compute the steady-state residuals
    //---------------------------------------------------------------------
    rhs_HTA(pid, u_HTA, rsd_HTA, frct_HTA, qs_HTA, rho_i_HTA);

    //---------------------------------------------------------------------
    // compute the max-norms of newton iteration residuals
    //---------------------------------------------------------------------
    if ( ((istep % inorm ) == 0 ) || ( istep == itmax ) ) {
      MASTER_EXEC_START
      if (timeron) timer_start(t_l2norm);
      MASTER_EXEC_END
      l2norm_HTA(pid, rsd, rsd_HTA, sum_HTA);
      MASTER_EXEC_START
      if (timeron) timer_stop(t_l2norm);
      MASTER_EXEC_END
    }

    //---------------------------------------------------------------------
    // check the newton-iteration residuals against the tolerance levels
    //---------------------------------------------------------------------
    if ( ( rsd[0] < tolrsd[0] ) && ( rsd[1] < tolrsd[1] ) &&
         ( rsd[2] < tolrsd[2] ) && ( rsd[3] < tolrsd[3] ) &&
         ( rsd[4] < tolrsd[4] ) ) {
      //if (ipr == 1 ) {
      MASTER_PRINTF(" \n convergence was achieved after %4d pseudo-time steps\n",
          istep);
      //}
      break;
    }
   }
  MASTER_EXEC_START
  timer_stop(1);
  maxtime = timer_read(1);
  MASTER_EXEC_END

}

void muladd(HTA* s1_tile, HTA* s2_tile, void* scalar) {

    int i, j, k, m;
    int i_first, j_first, k_first, i_last, j_last, k_last;
    int x, y, z;

//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;

    int nm_tile = s1_tile->flat_size.values[3]; // Always 5
    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];

    double (*u_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s1_tile);
    double (*rsd_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s2_tile);
    double val = *((double*)scalar);

    // FIXME: Trick to obtain the nd_idx of the tiles
    // Get global tile nd_index first
    //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1); // tile dimensions
    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, &nd_size, &nd_idx);

    x = nd_idx.values[2];
    y = nd_idx.values[1];
    z = nd_idx.values[0];

    // sync_boundary due to overlapping
    if (x == 0) i_first = 3; // Bottom: 2nd element
    else i_first = 2;
    if (y == 0) j_first = 3; // North: 2nd element
    else j_first = 2;
    if (z == 0) k_first = 3; // East: 2nd element
    else k_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Top: one before last element
    else i_last = nx_tile - 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // South: one before last element
    else j_last = ny_tile - 2;
    if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // West: one before last element
    else k_last = nz_tile - 2;

    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	for (i = i_first; i < i_last; i++) {
	  for (m = 0; m < 5; m++) {
	    u_tile[k][j][i][m] = u_tile[k][j][i][m] + (val * rsd_tile[k][j][i][m]);
	  }
	}
      }
    }
}


void mulscalar(HTA* s1_tile, void* scalar) {

    int i, j, k;
    int nm_tile = s1_tile->flat_size.values[3];
    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];

    double (*rsd_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s1_tile);
    double val = *((double*)scalar);

    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;

    int i_first, j_first, k_first, i_last, j_last, k_last;

    int x = nd_idx.values[2];
    int y = nd_idx.values[1];
    int z = nd_idx.values[0];

    // sync_boundary due to overlapping
    // These limits because frct is synchronized
    if (x == 0) i_first = 2;
    else i_first = 2;
    if (y == 0) j_first = 2;
    else j_first = 2;
    if (z == 0) k_first = 2;
    else k_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile-2;
    else i_last = nx_tile - 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile-2;
    else j_last = ny_tile - 2;
    if (z == nd_size.values[0] - 1) k_last = nz_tile-2;
    else k_last = nz_tile - 2;

    // NOTE: Original loop; k = 1; k < nz -1. We use these indices because rsd uses overlapping + 2
//     for (k = 0; k < nz_tile; k++) {
//       for (j = 0; j < ny_tile; j++) {
// 	for (i = 0; i < nx_tile ; i++) {
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	for (i = i_first; i < i_last; i++) {
	  //for (m = 0; m < 5; m++) {
	    rsd_tile[k][j][i][0] = val * rsd_tile[k][j][i][0];
	    rsd_tile[k][j][i][1] = val * rsd_tile[k][j][i][1];
	    rsd_tile[k][j][i][2] = val * rsd_tile[k][j][i][2];
	    rsd_tile[k][j][i][3] = val * rsd_tile[k][j][i][3];
	    rsd_tile[k][j][i][4] = val * rsd_tile[k][j][i][4];
	  //}
	}
      }
    }
}


