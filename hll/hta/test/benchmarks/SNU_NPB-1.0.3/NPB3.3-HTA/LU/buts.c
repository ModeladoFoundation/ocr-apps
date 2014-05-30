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

#include "applu.incl"
#include "timers.h"

//---------------------------------------------------------------------
// 
// compute the regular-sparse, block upper triangular solution:
// 
// v <-- ( U-inv ) * v
// 
//---------------------------------------------------------------------
void buts_HTA(int iter)
{
  int w, val;
  
  // FIXME: h5s2 is required to pass omega = 1.2  
  for(w = iter; w >= 0; w--) { // Wavefront propagation to calculate rsd_HTA
      val = w;
      check_bound = w;
      //HTA_map_h5s1(HTA_LEAF_LEVEL(rsd_HTA), buts_compute_HTA, au_HTA, bu_HTA, cu_HTA, du_HTA, rsd_HTA, &val);
      HTA_map_h4s1(HTA_LEAF_LEVEL(rsd_HTA), jacu_buts_compute2_HTA, rsd_HTA, rho_i_HTA, qs_HTA, u_HTA, &val);
      if (timeron) timer_start(t_sync_buts);
      sync_boundary(rsd_HTA);
      if (timeron) timer_stop(t_sync_buts);
  }
  
  check_bound = -1;
}

void jacu_buts_compute2_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, void *scalar)
{
  
  double r43;
  double c1345;
  double c34;
  double tmp, tmp1, tmp2, tmp3;
  
  int i, j, k, m;
  int i_first, j_first, i_last, j_last, k_first, k_last;
  int x, y, z;
  double tmat[5][5];
  // FIXME: h4s2 is required to pass omega and k, omega = 1.2
  double omega = 1.2;
  
  double du[25], au[25], bu[25], cu[25];
  //double tv[ny_tile][nx_tile][5];
  double tv[5];
  
  Tuple nd_size = s4_tile->nd_tile_dimensions;
  Tuple nd_idx = s4_tile->nd_rank;
  
  x = nd_idx.values[2];
  y = nd_idx.values[1];
  z = nd_idx.values[0];
  
  int selection = *((int*)scalar);
  
  if(x + y + z == selection) { // Wavefront propagation: only tiles with i + j + k == val
  
  int nx_tile = s4_tile->flat_size.values[2];
  int ny_tile = s4_tile->flat_size.values[1];
  int nz_tile = s4_tile->flat_size.values[0]; 
      
  double (*rsd_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s1_tile);
  // Required by jacu
  double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);
  double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s3_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s4_tile);
    
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; // orig. loop i = iend - 1; i >= ist; i--
  else i_last = nx_tile - 3;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; // orig. loop j = jend - 1; j >= jst; j--
  else j_last = ny_tile - 3;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; // orig. loop: k = nz-2; k > 0; k--
  else k_last = nz_tile - 3;
  
  r43 = ( 4.0 / 3.0 );
  c1345 = C1 * C3 * C4 * C5;
  c34 = C3 * C4;
  
  for(k = k_last; k >= k_first; k--) {
    for (j = j_last; j >= j_first; j--) {
      for (i = i_last; i >= i_first; i--) { 
      // ******************************************************
      // From now on JACU
      // ******************************************************
      //---------------------------------------------------------------------
      // form the block daigonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      du[0] = 1.0 + dt * 2.0 * ( tx1 * dx1 + ty1 * dy1 + tz1 * dz1 );
      du[5] = 0.0;
      du[10] = 0.0;
      du[15] = 0.0;
      du[20] = 0.0;

      du[1] =  dt * 2.0
        * ( - tx1 * r43 - ty1 - tz1 )
        * ( c34 * tmp2 * u_tile[k][j][i][1] );
      du[6] =  1.0
        + dt * 2.0 * c34 * tmp1 
        * (  tx1 * r43 + ty1 + tz1 )
        + dt * 2.0 * ( tx1 * dx2 + ty1 * dy2 + tz1 * dz2 );
      du[11] = 0.0;
      du[16] = 0.0;
      du[21] = 0.0;

      du[2] = dt * 2.0
        * ( - tx1 - ty1 * r43 - tz1 )
        * ( c34 * tmp2 * u_tile[k][j][i][2] );
      du[7] = 0.0;
      du[12] = 1.0
        + dt * 2.0 * c34 * tmp1
        * (  tx1 + ty1 * r43 + tz1 )
        + dt * 2.0 * ( tx1 * dx3 + ty1 * dy3 + tz1 * dz3 );
      du[17] = 0.0;
      du[22] = 0.0;

      du[3] = dt * 2.0
        * ( - tx1 - ty1 - tz1 * r43 )
        * ( c34 * tmp2 * u_tile[k][j][i][3] );
      du[8] = 0.0;
      du[13] = 0.0;
      du[18] = 1.0
        + dt * 2.0 * c34 * tmp1
        * (  tx1 + ty1 + tz1 * r43 )
        + dt * 2.0 * ( tx1 * dx4 + ty1 * dy4 + tz1 * dz4 );
      du[23] = 0.0;

      du[4] = -dt * 2.0
        * ( ( ( tx1 * ( r43*c34 - c1345 )
                + ty1 * ( c34 - c1345 )
                + tz1 * ( c34 - c1345 ) ) * ( u_tile[k][j][i][1]*u_tile[k][j][i][1] )
              + ( tx1 * ( c34 - c1345 )
                + ty1 * ( r43*c34 - c1345 )
                + tz1 * ( c34 - c1345 ) ) * ( u_tile[k][j][i][2]*u_tile[k][j][i][2] )
              + ( tx1 * ( c34 - c1345 )
                + ty1 * ( c34 - c1345 )
                + tz1 * ( r43*c34 - c1345 ) ) * (u_tile[k][j][i][3]*u_tile[k][j][i][3])
            ) * tmp3
            + ( tx1 + ty1 + tz1 ) * c1345 * tmp2 * u_tile[k][j][i][4] );

      du[9] = dt * 2.0
        * ( tx1 * ( r43*c34 - c1345 )
          + ty1 * (     c34 - c1345 )
          + tz1 * (     c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][1];
      du[14] = dt * 2.0
        * ( tx1 * ( c34 - c1345 )
          + ty1 * ( r43*c34 -c1345 )
          + tz1 * ( c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][2];
      du[19] = dt * 2.0
        * ( tx1 * ( c34 - c1345 )
          + ty1 * ( c34 - c1345 )
          + tz1 * ( r43*c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][3];
      du[24] = 1.0
        + dt * 2.0 * ( tx1 + ty1 + tz1 ) * c1345 * tmp1
        + dt * 2.0 * ( tx1 * dx5 + ty1 * dy5 + tz1 * dz5 );
	
      //---------------------------------------------------------------------
      // form the first block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j][i+1][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      au[0] = - dt * tx1 * dx1;
      au[5] =   dt * tx2;
      au[10] =   0.0;
      au[15] =   0.0;
      au[20] =   0.0;

      au[1] =  dt * tx2
        * ( - ( u_tile[k][j][i+1][1] * tmp1 ) * ( u_tile[k][j][i+1][1] * tmp1 )
            + C2 * qs_tile[k][j][i+1][0] * tmp1 )
        - dt * tx1 * ( - r43 * c34 * tmp2 * u_tile[k][j][i+1][1] );
      au[6] =  dt * tx2
        * ( ( 2.0 - C2 ) * ( u_tile[k][j][i+1][1] * tmp1 ) )
        - dt * tx1 * ( r43 * c34 * tmp1 )
        - dt * tx1 * dx2;
      au[11] =  dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][2] * tmp1 ) );
      au[16] =  dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][3] * tmp1 ) );
      au[21] =  dt * tx2 * C2 ;

      au[2] =  dt * tx2
        * ( - ( u_tile[k][j][i+1][1] * u_tile[k][j][i+1][2] ) * tmp2 )
        - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][2] );
      au[7] =  dt * tx2 * ( u_tile[k][j][i+1][2] * tmp1 );
      au[12] =  dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
        - dt * tx1 * ( c34 * tmp1 )
        - dt * tx1 * dx3;
      au[17] = 0.0;
      au[22] = 0.0;

      au[3] = dt * tx2
        * ( - ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][3] ) * tmp2 )
        - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][3] );
      au[8] = dt * tx2 * ( u_tile[k][j][i+1][3] * tmp1 );
      au[13] = 0.0;
      au[18] = dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
        - dt * tx1 * ( c34 * tmp1 )
        - dt * tx1 * dx4;
      au[23] = 0.0;

      au[4] = dt * tx2
        * ( ( C2 * 2.0 * qs_tile[k][j][i+1][0]
            - C1 * u_tile[k][j][i+1][4] )
        * ( u_tile[k][j][i+1][1] * tmp2 ) )
        - dt * tx1
        * ( - ( r43*c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] )
            - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][2] )
            - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][3] )
            - c1345 * tmp2 * u_tile[k][j][i+1][4] );
      au[9] = dt * tx2
        * ( C1 * ( u_tile[k][j][i+1][4] * tmp1 )
            - C2
            * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] * tmp2
              + qs_tile[k][j][i+1][0] * tmp1 ) )
        - dt * tx1
        * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][1];
      au[14] = dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][1] ) * tmp2 )
        - dt * tx1
        * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][2];
      au[19] = dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][1] ) * tmp2 )
        - dt * tx1
        * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][3];
      au[24] = dt * tx2
        * ( C1 * ( u_tile[k][j][i+1][1] * tmp1 ) )
        - dt * tx1 * c1345 * tmp1
        - dt * tx1 * dx5;
      //---------------------------------------------------------------------
      // form the second block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j+1][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      bu[0] = - dt * ty1 * dy1;
      bu[5] =   0.0;
      bu[10] =  dt * ty2;
      bu[15] =   0.0;
      bu[20] =   0.0;

      bu[1] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
        - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][1] );
      bu[6] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
        - dt * ty1 * ( c34 * tmp1 )
        - dt * ty1 * dy2;
      bu[11] =  dt * ty2 * ( u_tile[k][j+1][i][1] * tmp1 );
      bu[16] = 0.0;
      bu[21] = 0.0;

      bu[2] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][2] * tmp1 ) * ( u_tile[k][j+1][i][2] * tmp1 )
            + C2 * ( qs_tile[k][j+1][i][0] * tmp1 ) )
        - dt * ty1 * ( - r43 * c34 * tmp2 * u_tile[k][j+1][i][2] );
      bu[7] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][1] * tmp1 ) );
      bu[12] =  dt * ty2 * ( ( 2.0 - C2 )
          * ( u_tile[k][j+1][i][2] * tmp1 ) )
        - dt * ty1 * ( r43 * c34 * tmp1 )
        - dt * ty1 * dy3;
      bu[17] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][3] * tmp1 ) );
      bu[22] =  dt * ty2 * C2;

      bu[3] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
        - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][3] );
      bu[8] = 0.0;
      bu[13] =  dt * ty2 * ( u_tile[k][j+1][i][3] * tmp1 );
      bu[18] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
        - dt * ty1 * ( c34 * tmp1 )
        - dt * ty1 * dy4;
      bu[23] = 0.0;

      bu[4] =  dt * ty2
        * ( ( C2 * 2.0 * qs_tile[k][j+1][i][0]
            - C1 * u_tile[k][j+1][i][4] )
        * ( u_tile[k][j+1][i][2] * tmp2 ) )
        - dt * ty1
        * ( - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][1]*u_tile[k][j+1][i][1])
            - ( r43*c34 - c1345 )*tmp3*(u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2])
            - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][3]*u_tile[k][j+1][i][3])
            - c1345*tmp2*u_tile[k][j+1][i][4] );
      bu[9] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
        - dt * ty1
        * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][1];
      bu[14] =  dt * ty2
        * ( C1 * ( u_tile[k][j+1][i][4] * tmp1 )
            - C2 
            * ( qs_tile[k][j+1][i][0] * tmp1
              + u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2] * tmp2 ) )
        - dt * ty1
        * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][2];
      bu[19] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
        - dt * ty1 * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][3];
      bu[24] =  dt * ty2
        * ( C1 * ( u_tile[k][j+1][i][2] * tmp1 ) )
        - dt * ty1 * c1345 * tmp1
        - dt * ty1 * dy5;
	
      //---------------------------------------------------------------------
      // form the third block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k+1][j][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      cu[0] = - dt * tz1 * dz1;
      cu[5] =   0.0;
      cu[10] =   0.0;
      cu[15] = dt * tz2;
      cu[20] =   0.0;

      cu[1] = dt * tz2
        * ( - ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][1] );
      cu[6] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * c34 * tmp1
        - dt * tz1 * dz2;
      cu[11] = 0.0;
      cu[16] = dt * tz2 * ( u_tile[k+1][j][i][1] * tmp1 );
      cu[21] = 0.0;

      cu[2] = dt * tz2
        * ( - ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][2] );
      cu[7] = 0.0;
      cu[12] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * ( c34 * tmp1 )
        - dt * tz1 * dz3;
      cu[17] = dt * tz2 * ( u_tile[k+1][j][i][2] * tmp1 );
      cu[22] = 0.0;

      cu[3] = dt * tz2
        * ( - ( u_tile[k+1][j][i][3] * tmp1 ) * ( u_tile[k+1][j][i][3] * tmp1 )
            + C2 * ( qs_tile[k+1][j][i][0] * tmp1 ) )
        - dt * tz1 * ( - r43 * c34 * tmp2 * u_tile[k+1][j][i][3] );
      cu[8] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][1] * tmp1 ) );
      cu[13] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][2] * tmp1 ) );
      cu[18] = dt * tz2 * ( 2.0 - C2 )
        * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * ( r43 * c34 * tmp1 )
        - dt * tz1 * dz4;
      cu[23] = dt * tz2 * C2;

      cu[4] = dt * tz2
        * ( ( C2 * 2.0 * qs_tile[k+1][j][i][0]
            - C1 * u_tile[k+1][j][i][4] )
                 * ( u_tile[k+1][j][i][3] * tmp2 ) )
        - dt * tz1
        * ( - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][1]*u_tile[k+1][j][i][1])
            - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][2]*u_tile[k+1][j][i][2])
            - ( r43*c34 - c1345 )* tmp3 * (u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3])
            - c1345 * tmp2 * u_tile[k+1][j][i][4] );
      cu[9] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][1];
      cu[14] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][2];
      cu[19] = dt * tz2
        * ( C1 * ( u_tile[k+1][j][i][4] * tmp1 )
            - C2
            * ( qs_tile[k+1][j][i][0] * tmp1
              + u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3] * tmp2 ) )
        - dt * tz1 * ( r43*c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][3];
      cu[24] = dt * tz2
        * ( C1 * ( u_tile[k+1][j][i][3] * tmp1 ) )
        - dt * tz1 * c1345 * tmp1
        - dt * tz1 * dz5;
      
      //*******************************************************
      // From now on BUTS
      // ******************************************************	
	for (m = 0; m < 5; m++) {
	  tv[m] = 
	      omega * (  cu[m] * rsd_tile[k+1][j][i][0]
                   + cu[5+m] * rsd_tile[k+1][j][i][1]
                   + cu[10+m] * rsd_tile[k+1][j][i][2]
                   + cu[15+m] * rsd_tile[k+1][j][i][3]
                   + cu[20+m] * rsd_tile[k+1][j][i][4] );
	 }
	  
	for (m = 0; m < 5; m++) {
	    tv[m] = tv[m]
	      + omega * ( bu[m] * rsd_tile[k][j+1][i][0]
		      + au[m] * rsd_tile[k][j][i+1][0]
		      + bu[5+m] * rsd_tile[k][j+1][i][1]
		      + au[5+m] * rsd_tile[k][j][i+1][1]
		      + bu[10+m] * rsd_tile[k][j+1][i][2]
		      + au[10+m] * rsd_tile[k][j][i+1][2]
		      + bu[15+m] * rsd_tile[k][j+1][i][3]
		      + au[15+m] * rsd_tile[k][j][i+1][3]
		      + bu[20+m] * rsd_tile[k][j+1][i][4]
		      + au[20+m] * rsd_tile[k][j][i+1][4] );  
	}
      //---------------------------------------------------------------------
      // diagonal block inversion
      //---------------------------------------------------------------------
      for (m = 0; m < 5; m++) {
        tmat[m][0] = du[m];
        tmat[m][1] = du[5+m];
        tmat[m][2] = du[10+m];
        tmat[m][3] = du[15+m];
        tmat[m][4] = du[20+m];
      }

      tmp1 = 1.0 / tmat[0][0];
      tmp = tmp1 * tmat[1][0];
      tmat[1][1] =  tmat[1][1] - tmp * tmat[0][1];
      tmat[1][2] =  tmat[1][2] - tmp * tmat[0][2];
      tmat[1][3] =  tmat[1][3] - tmp * tmat[0][3];
      tmat[1][4] =  tmat[1][4] - tmp * tmat[0][4];
      tv[1] = tv[1] - tv[0] * tmp;

      tmp = tmp1 * tmat[2][0];
      tmat[2][1] =  tmat[2][1] - tmp * tmat[0][1];
      tmat[2][2] =  tmat[2][2] - tmp * tmat[0][2];
      tmat[2][3] =  tmat[2][3] - tmp * tmat[0][3];
      tmat[2][4] =  tmat[2][4] - tmp * tmat[0][4];
      tv[2] = tv[2] - tv[0] * tmp;

      tmp = tmp1 * tmat[3][0];
      tmat[3][1] =  tmat[3][1] - tmp * tmat[0][1];
      tmat[3][2] =  tmat[3][2] - tmp * tmat[0][2];
      tmat[3][3] =  tmat[3][3] - tmp * tmat[0][3];
      tmat[3][4] =  tmat[3][4] - tmp * tmat[0][4];
      tv[3] = tv[3] - tv[0] * tmp;

      tmp = tmp1 * tmat[4][0];
      tmat[4][1] =  tmat[4][1] - tmp * tmat[0][1];
      tmat[4][2] =  tmat[4][2] - tmp * tmat[0][2];
      tmat[4][3] =  tmat[4][3] - tmp * tmat[0][3];
      tmat[4][4] =  tmat[4][4] - tmp * tmat[0][4];
      tv[4] = tv[4] - tv[0] * tmp;

      tmp1 = 1.0 / tmat[1][1];
      tmp = tmp1 * tmat[2][1];
      tmat[2][2] =  tmat[2][2] - tmp * tmat[1][2];
      tmat[2][3] =  tmat[2][3] - tmp * tmat[1][3];
      tmat[2][4] =  tmat[2][4] - tmp * tmat[1][4];
      tv[2] = tv[2] - tv[1] * tmp;

      tmp = tmp1 * tmat[3][1];
      tmat[3][2] =  tmat[3][2] - tmp * tmat[1][2];
      tmat[3][3] =  tmat[3][3] - tmp * tmat[1][3];
      tmat[3][4] =  tmat[3][4] - tmp * tmat[1][4];
      tv[3] = tv[3] - tv[1] * tmp;

      tmp = tmp1 * tmat[4][1];
      tmat[4][2] =  tmat[4][2] - tmp * tmat[1][2];
      tmat[4][3] =  tmat[4][3] - tmp * tmat[1][3];
      tmat[4][4] =  tmat[4][4] - tmp * tmat[1][4];
      tv[4] = tv[4] - tv[1] * tmp;

      tmp1 = 1.0 / tmat[2][2];
      tmp = tmp1 * tmat[3][2];
      tmat[3][3] =  tmat[3][3] - tmp * tmat[2][3];
      tmat[3][4] =  tmat[3][4] - tmp * tmat[2][4];
      tv[3] = tv[3] - tv[2] * tmp;

      tmp = tmp1 * tmat[4][2];
      tmat[4][3] =  tmat[4][3] - tmp * tmat[2][3];
      tmat[4][4] =  tmat[4][4] - tmp * tmat[2][4];
      tv[4] = tv[4] - tv[2] * tmp;

      tmp1 = 1.0 / tmat[3][3];
      tmp = tmp1 * tmat[4][3];
      tmat[4][4] =  tmat[4][4] - tmp * tmat[3][4];
      tv[4] = tv[4] - tv[3] * tmp;

      //---------------------------------------------------------------------
      // back substitution
      //---------------------------------------------------------------------
      tv[4] = tv[4] / tmat[4][4];

      tv[3] = tv[3] - tmat[3][4] * tv[4];
      tv[3] = tv[3] / tmat[3][3];

      tv[2] = tv[2]
        - tmat[2][3] * tv[3]
        - tmat[2][4] * tv[4];
      tv[2] = tv[2] / tmat[2][2];

      tv[1] = tv[1]
        - tmat[1][2] * tv[2]
        - tmat[1][3] * tv[3]
        - tmat[1][4] * tv[4];
      tv[1] = tv[1] / tmat[1][1];

      tv[0] = tv[0]
        - tmat[0][1] * tv[1]
        - tmat[0][2] * tv[2]
        - tmat[0][3] * tv[3]
        - tmat[0][4] * tv[4];
      tv[0] = tv[0] / tmat[0][0];

      rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0] - tv[0];
      rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1] - tv[1];
      rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2] - tv[2];
      rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3] - tv[3];
      rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4] - tv[4];
      
	}
      }	
    }
  } // End If
 
}

// void buts_compute_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, void *scalar)
// {
//   
//   int i, j, k, m;
//   int i_first, j_first, i_last, j_last, k_first, k_last;
//   int x, y, z;
//   double tmp, tmp1;
//   double tmat[5][5];
//   // FIXME: h5s2 is required to pass omega and k, omega = 1.2
//   double omega = 1.2;
//   
// //    int TILES_X = 1;
// //    int TILES_Y = 1;
// //    int TILES_Z = 3;
//   
//   int nx_tile = s5_tile->flat_size.values[2];
//   int ny_tile = s5_tile->flat_size.values[1];
//   int nz_tile = s5_tile->flat_size.values[0]; 
//     
//   double tv[ny_tile][nx_tile][5];
//   
//   double (*au_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s1_tile);
//   double (*bu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s2_tile);
//   double (*cu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s3_tile);
//   double (*du_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s4_tile);
//   double (*rsd_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s5_tile);
//   int selection = *((int*)scalar);
//   
//   // FIXME: Trick to obtain the nd_idx of the tiles
//     // Get global tile nd_index first
//     //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1); // tile dimensions
//     Tuple nd_size = s5_tile->nd_tile_dimensions;
//     Tuple nd_idx = s5_tile->nd_rank;
//     //Tuple_init_zero(&nd_idx, 4); // this tile index
//     //Tuple_1d_to_nd_index(s5_tile->rank, &nd_size, &nd_idx);
//   
//   x = nd_idx.values[2];
//   y = nd_idx.values[1];
//   z = nd_idx.values[0];
//   
//   if(x + y + z == selection) { // Wavefront propagation: only tiles with i + j + k == val
//     
//   if (x == 0) i_first = 3; // 2nd element
//   else i_first = 2;
//   if (y == 0) j_first = 3; // 2nd element
//   else j_first = 2;
//   if (z == 0) k_first = 3; // 2nd element
//   else k_first = 2;
//   if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; // orig. loop i = iend - 1; i >= ist; i--
//   else i_last = nx_tile - 3;
//   if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; // orig. loop j = jend - 1; j >= jst; j--
//   else j_last = ny_tile - 3;
//   if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; // orig. loop: k = nz-2; k > 0; k--
//   else k_last = nz_tile - 3;
//   
//   for(k = k_last; k >= k_first; k--) {
//     for (j = j_last; j >= j_first; j--) {
//       for (i = i_last; i >= i_first; i--) { 
// 	for (m = 0; m < 5; m++) {
// 	  tv[j][i][m] = 
// 	      omega * (  cu_tile[k][j][i][m] * rsd_tile[k+1][j][i][0]
//                    + cu_tile[k][j][i][5+m] * rsd_tile[k+1][j][i][1]
//                    + cu_tile[k][j][i][10+m] * rsd_tile[k+1][j][i][2]
//                    + cu_tile[k][j][i][15+m] * rsd_tile[k+1][j][i][3]
//                    + cu_tile[k][j][i][20+m] * rsd_tile[k+1][j][i][4] );
// 	 }
// 	  
// 	for (m = 0; m < 5; m++) {
// 	    tv[j][i][m] = tv[j][i][m]
// 	      + omega * ( bu_tile[k][j][i][m] * rsd_tile[k][j+1][i][0]
// 		      + au_tile[k][j][i][m] * rsd_tile[k][j][i+1][0]
// 		      + bu_tile[k][j][i][5+m] * rsd_tile[k][j+1][i][1]
// 		      + au_tile[k][j][i][5+m] * rsd_tile[k][j][i+1][1]
// 		      + bu_tile[k][j][i][10+m] * rsd_tile[k][j+1][i][2]
// 		      + au_tile[k][j][i][10+m] * rsd_tile[k][j][i+1][2]
// 		      + bu_tile[k][j][i][15+m] * rsd_tile[k][j+1][i][3]
// 		      + au_tile[k][j][i][15+m] * rsd_tile[k][j][i+1][3]
// 		      + bu_tile[k][j][i][20+m] * rsd_tile[k][j+1][i][4]
// 		      + au_tile[k][j][i][20+m] * rsd_tile[k][j][i+1][4] );  
// 	}
//       //---------------------------------------------------------------------
//       // diagonal block inversion
//       //---------------------------------------------------------------------
//       for (m = 0; m < 5; m++) {
//         tmat[m][0] = du_tile[k][j][i][m];
//         tmat[m][1] = du_tile[k][j][i][5+m];
//         tmat[m][2] = du_tile[k][j][i][10+m];
//         tmat[m][3] = du_tile[k][j][i][15+m];
//         tmat[m][4] = du_tile[k][j][i][20+m];
//       }
// 
//       tmp1 = 1.0 / tmat[0][0];
//       tmp = tmp1 * tmat[1][0];
//       tmat[1][1] =  tmat[1][1] - tmp * tmat[0][1];
//       tmat[1][2] =  tmat[1][2] - tmp * tmat[0][2];
//       tmat[1][3] =  tmat[1][3] - tmp * tmat[0][3];
//       tmat[1][4] =  tmat[1][4] - tmp * tmat[0][4];
//       tv[j][i][1] = tv[j][i][1] - tv[j][i][0] * tmp;
// 
//       tmp = tmp1 * tmat[2][0];
//       tmat[2][1] =  tmat[2][1] - tmp * tmat[0][1];
//       tmat[2][2] =  tmat[2][2] - tmp * tmat[0][2];
//       tmat[2][3] =  tmat[2][3] - tmp * tmat[0][3];
//       tmat[2][4] =  tmat[2][4] - tmp * tmat[0][4];
//       tv[j][i][2] = tv[j][i][2] - tv[j][i][0] * tmp;
// 
//       tmp = tmp1 * tmat[3][0];
//       tmat[3][1] =  tmat[3][1] - tmp * tmat[0][1];
//       tmat[3][2] =  tmat[3][2] - tmp * tmat[0][2];
//       tmat[3][3] =  tmat[3][3] - tmp * tmat[0][3];
//       tmat[3][4] =  tmat[3][4] - tmp * tmat[0][4];
//       tv[j][i][3] = tv[j][i][3] - tv[j][i][0] * tmp;
// 
//       tmp = tmp1 * tmat[4][0];
//       tmat[4][1] =  tmat[4][1] - tmp * tmat[0][1];
//       tmat[4][2] =  tmat[4][2] - tmp * tmat[0][2];
//       tmat[4][3] =  tmat[4][3] - tmp * tmat[0][3];
//       tmat[4][4] =  tmat[4][4] - tmp * tmat[0][4];
//       tv[j][i][4] = tv[j][i][4] - tv[j][i][0] * tmp;
// 
//       tmp1 = 1.0 / tmat[1][1];
//       tmp = tmp1 * tmat[2][1];
//       tmat[2][2] =  tmat[2][2] - tmp * tmat[1][2];
//       tmat[2][3] =  tmat[2][3] - tmp * tmat[1][3];
//       tmat[2][4] =  tmat[2][4] - tmp * tmat[1][4];
//       tv[j][i][2] = tv[j][i][2] - tv[j][i][1] * tmp;
// 
//       tmp = tmp1 * tmat[3][1];
//       tmat[3][2] =  tmat[3][2] - tmp * tmat[1][2];
//       tmat[3][3] =  tmat[3][3] - tmp * tmat[1][3];
//       tmat[3][4] =  tmat[3][4] - tmp * tmat[1][4];
//       tv[j][i][3] = tv[j][i][3] - tv[j][i][1] * tmp;
// 
//       tmp = tmp1 * tmat[4][1];
//       tmat[4][2] =  tmat[4][2] - tmp * tmat[1][2];
//       tmat[4][3] =  tmat[4][3] - tmp * tmat[1][3];
//       tmat[4][4] =  tmat[4][4] - tmp * tmat[1][4];
//       tv[j][i][4] = tv[j][i][4] - tv[j][i][1] * tmp;
// 
//       tmp1 = 1.0 / tmat[2][2];
//       tmp = tmp1 * tmat[3][2];
//       tmat[3][3] =  tmat[3][3] - tmp * tmat[2][3];
//       tmat[3][4] =  tmat[3][4] - tmp * tmat[2][4];
//       tv[j][i][3] = tv[j][i][3] - tv[j][i][2] * tmp;
// 
//       tmp = tmp1 * tmat[4][2];
//       tmat[4][3] =  tmat[4][3] - tmp * tmat[2][3];
//       tmat[4][4] =  tmat[4][4] - tmp * tmat[2][4];
//       tv[j][i][4] = tv[j][i][4] - tv[j][i][2] * tmp;
// 
//       tmp1 = 1.0 / tmat[3][3];
//       tmp = tmp1 * tmat[4][3];
//       tmat[4][4] =  tmat[4][4] - tmp * tmat[3][4];
//       tv[j][i][4] = tv[j][i][4] - tv[j][i][3] * tmp;
// 
//       //---------------------------------------------------------------------
//       // back substitution
//       //---------------------------------------------------------------------
//       tv[j][i][4] = tv[j][i][4] / tmat[4][4];
// 
//       tv[j][i][3] = tv[j][i][3] - tmat[3][4] * tv[j][i][4];
//       tv[j][i][3] = tv[j][i][3] / tmat[3][3];
// 
//       tv[j][i][2] = tv[j][i][2]
//         - tmat[2][3] * tv[j][i][3]
//         - tmat[2][4] * tv[j][i][4];
//       tv[j][i][2] = tv[j][i][2] / tmat[2][2];
// 
//       tv[j][i][1] = tv[j][i][1]
//         - tmat[1][2] * tv[j][i][2]
//         - tmat[1][3] * tv[j][i][3]
//         - tmat[1][4] * tv[j][i][4];
//       tv[j][i][1] = tv[j][i][1] / tmat[1][1];
// 
//       tv[j][i][0] = tv[j][i][0]
//         - tmat[0][1] * tv[j][i][1]
//         - tmat[0][2] * tv[j][i][2]
//         - tmat[0][3] * tv[j][i][3]
//         - tmat[0][4] * tv[j][i][4];
//       tv[j][i][0] = tv[j][i][0] / tmat[0][0];
// 
//       rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0] - tv[j][i][0];
//       rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1] - tv[j][i][1];
//       rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2] - tv[j][i][2];
//       rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3] - tv[j][i][3];
//       rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4] - tv[j][i][4];
//       
// 	}
//       }	
//     }
//   } // End If
//  
// }