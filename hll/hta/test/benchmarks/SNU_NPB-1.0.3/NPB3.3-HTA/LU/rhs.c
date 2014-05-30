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
// compute the right hand sides
//---------------------------------------------------------------------
void rhs_HTA()
{
    int dummy = 0;
    
    if (timeron) timer_start(t_rhs);
    
    // Initialization rsd, rho_i and qs
    //HTA_map_h5s1(HTA_LEAF_LEVEL(rsd_HTA), init_rhs_HTA, rho_i_HTA, qs_HTA, u_HTA, frct_HTA, rsd_HTA, &dummy); 
    
    //sync_boundary(qs_HTA);
    //sync_boundary(rho_i_HTA);
    
    //HTA_map_h4(HTA_LEAF_LEVEL(rsd_HTA), rhs_flux_HTA, rsd_HTA, u_HTA, rho_i_HTA, qs_HTA);
    HTA_map_h5s1(HTA_LEAF_LEVEL(rsd_HTA), rhs_compute_HTA, frct_HTA, rsd_HTA, u_HTA, rho_i_HTA, qs_HTA, &dummy);
    sync_boundary(rsd_HTA);
    
    if (timeron) timer_stop(t_rhs);
}

void rhs_compute_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, void *scalar)
{
  
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  double q;
  double tmp, utmp[ISIZ3+4][6], rtmp[ISIZ3+4][5];
  double u21, u31, u41;
  double u21i, u31i, u41i, u51i;
  double u21j, u31j, u41j, u51j;
  double u21k, u31k, u41k, u51k;
  double u21im1, u31im1, u41im1, u51im1;
  double u21jm1, u31jm1, u41jm1, u51jm1;
  double u21km1, u31km1, u41km1, u51km1;
  double flux_temp[ISIZ1+4][5];
  
//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;
  
  int i_first, j_first, k_first, i_last, j_last, k_last;
  int x, y, z;
  
  //int nm_tile = s1_tile->flat_size->values[3]; // 5 for u and rsd, 1 qs and rho_i
  int nx_tile = s1_tile->flat_size.values[2];
  int ny_tile = s1_tile->flat_size.values[1];
  int nz_tile = s1_tile->flat_size.values[0];

  double (*frct_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s1_tile);
  double (*rsd_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s2_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s3_tile);
  double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s4_tile);
  double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s5_tile);
  
  
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
  // These limits because frct is synchronized
    if (x == 0) i_first = 2;
    else i_first = 0;
    if (y == 0) j_first = 2;
    else j_first = 0;
    if (z == 0) k_first = 2;
    else k_first = 0;
    if (x == nd_size.values[2] - 1) i_last = nx_tile-2;
    else i_last = nx_tile;
    if (y == nd_size.values[1] - 1) j_last = ny_tile-2;
    else j_last = ny_tile;
    if (z == nd_size.values[0] - 1) k_last = nz_tile-2;
    else k_last = nz_tile;
  
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	for (i = i_first; i < i_last; i++) {
	  tmp = 1.0 / u_tile[k][j][i][0];
	  rho_i_tile[k][j][i][0] = tmp;
	  qs_tile[k][j][i][0] = 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
                              + u_tile[k][j][i][2] * u_tile[k][j][i][2]
                              + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
			    * tmp;
	  for (m = 0; m < 5; m++) {
	    rsd_tile[k][j][i][m] = - frct_tile[k][j][i][m];
	  }
	}
      }
    }
  
  
  //---------------------------------------------------------------------
  // xi-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
  else j_last = ny_tile - 2;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
  else k_last = nz_tile - 2;
  
  for (k = k_first; k < k_last; k++) {
    for (j = j_first; j < j_last; j++) {
      
      i_first = 1; // Because later flux[i-1] and flux[i+1] is required
      i_last = nx_tile-1;// Because later flux[i-1] and flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) {
        flux_temp[i][0] = u_tile[k][j][i][1];
        u21 = u_tile[k][j][i][1] * rho_i_tile[k][j][i][0];

        q = qs_tile[k][j][i][0]; 

        flux_temp[i][1] = u_tile[k][j][i][1] * u21 + C2 * ( u_tile[k][j][i][4] - q );
        flux_temp[i][2] = u_tile[k][j][i][2] * u21;
        flux_temp[i][3] = u_tile[k][j][i][3] * u21;
        flux_temp[i][4] = ( C1 * u_tile[k][j][i][4] - C2 * q ) * u21;
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
      else i_last = nx_tile - 2;

      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] =  rsd_tile[k][j][i][m]
            - tx2 * ( flux_temp[i+1][m] - flux_temp[i-1][m] );
	  }
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      i_last = nx_tile - 1; // Because later flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) {
        tmp = rho_i_tile[k][j][i][0];

        u21i = tmp * u_tile[k][j][i][1];
        u31i = tmp * u_tile[k][j][i][2];
        u41i = tmp * u_tile[k][j][i][3];
        u51i = tmp * u_tile[k][j][i][4];

        tmp = rho_i_tile[k][j][i-1][0];

        u21im1 = tmp * u_tile[k][j][i-1][1];
        u31im1 = tmp * u_tile[k][j][i-1][2];
        u41im1 = tmp * u_tile[k][j][i-1][3];
        u51im1 = tmp * u_tile[k][j][i-1][4];

        flux_temp[i][1] = (4.0/3.0) * tx3 * (u21i-u21im1);
        flux_temp[i][2] = tx3 * ( u31i - u31im1 );
        flux_temp[i][3] = tx3 * ( u41i - u41im1 );
        flux_temp[i][4] = 0.50 * ( 1.0 - C1*C5 )
          * tx3 * ( ( u21i*u21i     + u31i*u31i     + u41i*u41i )
                  - ( u21im1*u21im1 + u31im1*u31im1 + u41im1*u41im1 ) )
          + (1.0/6.0)
          * tx3 * ( u21i*u21i - u21im1*u21im1 )
          + C1 * C5 * tx3 * ( u51i - u51im1 );
      }
      
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 3;
      else i_last = nx_tile -2;

      for (i = i_first; i < i_last; i++) {
        rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0]
          + dx1 * tx1 * (        u_tile[k][j][i-1][0]
                         - 2.0 * u_tile[k][j][i][0]
                         +       u_tile[k][j][i+1][0] );
        rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1]
          + tx3 * C3 * C4 * ( flux_temp[i+1][1] - flux_temp[i][1] )
          + dx2 * tx1 * (        u_tile[k][j][i-1][1]
                         - 2.0 * u_tile[k][j][i][1]
                         +       u_tile[k][j][i+1][1] );
        rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2]
          + tx3 * C3 * C4 * ( flux_temp[i+1][2] - flux_temp[i][2] )
          + dx3 * tx1 * (        u_tile[k][j][i-1][2]
                         - 2.0 * u_tile[k][j][i][2]
                         +       u_tile[k][j][i+1][2] );
        rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3]
          + tx3 * C3 * C4 * ( flux_temp[i+1][3] - flux_temp[i][3] )
          + dx4 * tx1 * (        u_tile[k][j][i-1][3]
                         - 2.0 * u_tile[k][j][i][3]
                         +       u_tile[k][j][i+1][3] );
        rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4]
          + tx3 * C3 * C4 * ( flux_temp[i+1][4] - flux_temp[i][4] )
          + dx5 * tx1 * (        u_tile[k][j][i-1][4]
                         - 2.0 * u_tile[k][j][i][4]
                         +       u_tile[k][j][i+1][4] );
      }

      //---------------------------------------------------------------------
      // Fourth-order dissipation
      //---------------------------------------------------------------------
       if(x == 0) 
       {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j][3][m] = rsd_tile[k][j][3][m]
	    - dssp * ( + 5.0 * u_tile[k][j][3][m]
                     - 4.0 * u_tile[k][j][4][m]
                     +       u_tile[k][j][5][m] );
	  rsd_tile[k][j][4][m] = rsd_tile[k][j][4][m]
	    - dssp * ( - 4.0 * u_tile[k][j][3][m]
                     + 6.0 * u_tile[k][j][4][m]
                     - 4.0 * u_tile[k][j][5][m]
                     +       u_tile[k][j][6][m] );
	}
      }

      if (x == 0) i_first = 5; // 4th element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 5; 
      else i_last = nx_tile - 2;
      
      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rsd_tile[k][j][i][m]
            - dssp * (         u_tile[k][j][i-2][m]
                       - 4.0 * u_tile[k][j][i-1][m]
                       + 6.0 * u_tile[k][j][i][m]
                       - 4.0 * u_tile[k][j][i+1][m]
                       +       u_tile[k][j][i+2][m] );
        }
      }

      if(x == nd_size.values[2] - 1) 
      {
	i_last = nx_tile - 2;
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j][i_last-3][m] = rsd_tile[k][j][i_last-3][m]
	    - dssp * (         u_tile[k][j][i_last-5][m]
                     - 4.0 * u_tile[k][j][i_last-4][m]
                     + 6.0 * u_tile[k][j][i_last-3][m]
                     - 4.0 * u_tile[k][j][i_last-2][m] );
	  rsd_tile[k][j][i_last-2][m] = rsd_tile[k][j][i_last-2][m]
	    - dssp * (         u_tile[k][j][i_last-4][m]
                     - 4.0 * u_tile[k][j][i_last-3][m]
                     + 5.0 * u_tile[k][j][i_last-2][m] );	  
	}
      }     
     }
   }
  
  
  //---------------------------------------------------------------------
  // eta-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
  else i_last = nx_tile - 2;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
  else k_last = nz_tile - 2;
  
  for (k = k_first; k < k_last; k++) {
    for (i = i_first; i < i_last; i++) {
      
      j_first = 1; // Because later flux_temp[j-1] and flux_temp[j+1] is required
      j_last = ny_tile - 1;// Because later flux_temp[j-1] and flux_temp[j+1] is required
      
      for (j = j_first; j < j_last; j++) {
        flux_temp[j][0] = u_tile[k][j][i][2];
        u31 = u_tile[k][j][i][2] * rho_i_tile[k][j][i][0];

        q = qs_tile[k][j][i][0];

        flux_temp[j][1] = u_tile[k][j][i][1] * u31;
        flux_temp[j][2] = u_tile[k][j][i][2] * u31 + C2 * (u_tile[k][j][i][4]-q);
        flux_temp[j][3] = u_tile[k][j][i][3] * u31;
        flux_temp[j][4] = ( C1 * u_tile[k][j][i][4] - C2 * q ) * u31;
      }

      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
      else j_last = ny_tile - 2;
      
      for (j = j_first; j < j_last; j++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] =  rsd_tile[k][j][i][m]
            - ty2 * ( flux_temp[j+1][m] - flux_temp[j-1][m] );
        }
      }

      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      j_last = ny_tile - 1; // Because later flux[j+1] is required
      
      for (j = j_first; j < j_last; j++) {
        tmp = rho_i_tile[k][j][i][0];

        u21j = tmp * u_tile[k][j][i][1];
        u31j = tmp * u_tile[k][j][i][2];
        u41j = tmp * u_tile[k][j][i][3];
        u51j = tmp * u_tile[k][j][i][4];

        tmp = rho_i_tile[k][j-1][i][0];
        u21jm1 = tmp * u_tile[k][j-1][i][1];
        u31jm1 = tmp * u_tile[k][j-1][i][2];
        u41jm1 = tmp * u_tile[k][j-1][i][3];
        u51jm1 = tmp * u_tile[k][j-1][i][4];

        flux_temp[j][1] = ty3 * ( u21j - u21jm1 );
        flux_temp[j][2] = (4.0/3.0) * ty3 * (u31j-u31jm1);
        flux_temp[j][3] = ty3 * ( u41j - u41jm1 );
        flux_temp[j][4] = 0.50 * ( 1.0 - C1*C5 )
          * ty3 * ( ( u21j*u21j     + u31j*u31j     + u41j*u41j )
                  - ( u21jm1*u21jm1 + u31jm1*u31jm1 + u41jm1*u41jm1 ) )
          + (1.0/6.0)
          * ty3 * ( u31j*u31j - u31jm1*u31jm1 )
          + C1 * C5 * ty3 * ( u51j - u51jm1 );
      }
      
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3;
      else j_last = ny_tile -2;

      for (j = j_first; j < j_last; j++) {
        rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0]
          + dy1 * ty1 * (         u_tile[k][j-1][i][0]
                          - 2.0 * u_tile[k][j][i][0]
                          +       u_tile[k][j+1][i][0] );

        rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1]
          + ty3 * C3 * C4 * ( flux_temp[j+1][1] - flux_temp[j][1] )
          + dy2 * ty1 * (         u_tile[k][j-1][i][1]
                          - 2.0 * u_tile[k][j][i][1]
                          +       u_tile[k][j+1][i][1] );

        rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2]
          + ty3 * C3 * C4 * ( flux_temp[j+1][2] - flux_temp[j][2] )
          + dy3 * ty1 * (         u_tile[k][j-1][i][2]
                          - 2.0 * u_tile[k][j][i][2]
                          +       u_tile[k][j+1][i][2] );

        rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3]
          + ty3 * C3 * C4 * ( flux_temp[j+1][3] - flux_temp[j][3] )
          + dy4 * ty1 * (         u_tile[k][j-1][i][3]
                          - 2.0 * u_tile[k][j][i][3]
                          +       u_tile[k][j+1][i][3] );

        rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4]
          + ty3 * C3 * C4 * ( flux_temp[j+1][4] - flux_temp[j][4] )
          + dy5 * ty1 * (         u_tile[k][j-1][i][4]
                          - 2.0 * u_tile[k][j][i][4]
                          +       u_tile[k][j+1][i][4] );
      }
    } // end of i
    
    //---------------------------------------------------------------------
    // fourth-order dissipation
    //---------------------------------------------------------------------
   
    if (x == 0) i_first = 3; // 2nd element
    else i_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
    else i_last = nx_tile - 2;
    
    if(y == 0) {
      for (i = i_first; i < i_last; i++) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][3][i][m] = rsd_tile[k][3][i][m]
	    - dssp * ( + 5.0 * u_tile[k][3][i][m]
                     - 4.0 * u_tile[k][4][i][m]
                     +       u_tile[k][5][i][m] );
	  rsd_tile[k][4][i][m] = rsd_tile[k][4][i][m]
	    - dssp * ( - 4.0 * u_tile[k][3][i][m]
                     + 6.0 * u_tile[k][4][i][m]
                     - 4.0 * u_tile[k][5][i][m]
                     +       u_tile[k][6][i][m] );
	}
      }
    }

    if (y == 0) j_first = 5; // 4th element
    else j_first = 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 5; 
    else j_last = ny_tile - 2;
      
    for (j = j_first; j < j_last; j++) {
      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rsd_tile[k][j][i][m]
            - dssp * (         u_tile[k][j-2][i][m]
                       - 4.0 * u_tile[k][j-1][i][m]
                       + 6.0 * u_tile[k][j][i][m]
                       - 4.0 * u_tile[k][j+1][i][m]
                       +       u_tile[k][j+2][i][m] );
        }
      }
    }

    if(y == nd_size.values[1] - 1) {
      j_last = ny_tile - 2;
      for (i = i_first; i < i_last; i++) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j_last-3][i][m] = rsd_tile[k][j_last-3][i][m]
	    - dssp * (         u_tile[k][j_last-5][i][m]
                     - 4.0 * u_tile[k][j_last-4][i][m]
                     + 6.0 * u_tile[k][j_last-3][i][m]
                     - 4.0 * u_tile[k][j_last-2][i][m] );
	  rsd_tile[k][j_last-2][i][m] = rsd_tile[k][j_last-2][i][m]
	    - dssp * (         u_tile[k][j_last-4][i][m]
                     - 4.0 * u_tile[k][j_last-3][i][m]
                     + 5.0 * u_tile[k][j_last-2][i][m] );
	}
      }
    }  
}

  //---------------------------------------------------------------------
  // zeta-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
  else i_last = nx_tile - 2;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
  else j_last = ny_tile - 2;
  
  for (j = j_first; j < j_last; j++) {
    for (i = i_first; i < i_last; i++) {
      
      if (z == 0) k_first = 2; 
      else k_first = 0; // Because later utemp[k+2], utemp[k-2], flux[k-1] and flux[k+1] is required
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 2; 
      else k_last = nz_tile;
      
      for (k = k_first; k < k_last; k++) {
        utmp[k][0] = u_tile[k][j][i][0];
        utmp[k][1] = u_tile[k][j][i][1];
        utmp[k][2] = u_tile[k][j][i][2];
        utmp[k][3] = u_tile[k][j][i][3];
        utmp[k][4] = u_tile[k][j][i][4];
        utmp[k][5] = rho_i_tile[k][j][i][0];
      }
      
      for (k = k_first; k < k_last; k++) {
        flux_temp[k][0] = utmp[k][3];
        u41 = utmp[k][3] * utmp[k][5];

        q = qs_tile[k][j][i][0];

        flux_temp[k][1] = utmp[k][1] * u41;
        flux_temp[k][2] = utmp[k][2] * u41;
        flux_temp[k][3] = utmp[k][3] * u41 + C2 * (utmp[k][4]-q);
        flux_temp[k][4] = ( C1 * utmp[k][4] - C2 * q ) * u41;
      }
      
      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
      else k_last = nz_tile - 2;

      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          rtmp[k][m] =  rsd_tile[k][j][i][m]
            - tz2 * ( flux_temp[k+1][m] - flux_temp[k-1][m] );
        }
      }
      
      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      k_last = nz_tile - 1; // Because later flux[k+1] is required

      for (k = k_first; k < k_last; k++) {
        tmp = utmp[k][5];

        u21k = tmp * utmp[k][1];
        u31k = tmp * utmp[k][2];
        u41k = tmp * utmp[k][3];
        u51k = tmp * utmp[k][4];

        tmp = utmp[k-1][5];

        u21km1 = tmp * utmp[k-1][1];
        u31km1 = tmp * utmp[k-1][2];
        u41km1 = tmp * utmp[k-1][3];
        u51km1 = tmp * utmp[k-1][4];

        flux_temp[k][1] = tz3 * ( u21k - u21km1 );
        flux_temp[k][2] = tz3 * ( u31k - u31km1 );
        flux_temp[k][3] = (4.0/3.0) * tz3 * (u41k-u41km1);
        flux_temp[k][4] = 0.50 * ( 1.0 - C1*C5 )
          * tz3 * ( ( u21k*u21k     + u31k*u31k     + u41k*u41k )
                  - ( u21km1*u21km1 + u31km1*u31km1 + u41km1*u41km1 ) )
          + (1.0/6.0)
          * tz3 * ( u41k*u41k - u41km1*u41km1 )
          + C1 * C5 * tz3 * ( u51k - u51km1 );
      }
      
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3;
      else k_last = nz_tile-2;
      
      for (k = k_first; k < k_last; k++) {
        rtmp[k][0] = rtmp[k][0]
          + dz1 * tz1 * (         utmp[k-1][0]
                          - 2.0 * utmp[k][0]
                          +       utmp[k+1][0] );
        rtmp[k][1] = rtmp[k][1]
          + tz3 * C3 * C4 * ( flux_temp[k+1][1] - flux_temp[k][1] )
          + dz2 * tz1 * (         utmp[k-1][1]
                          - 2.0 * utmp[k][1]
                          +       utmp[k+1][1] );
        rtmp[k][2] = rtmp[k][2]
          + tz3 * C3 * C4 * ( flux_temp[k+1][2] - flux_temp[k][2] )
          + dz3 * tz1 * (         utmp[k-1][2]
                          - 2.0 * utmp[k][2]
                          +       utmp[k+1][2] );
        rtmp[k][3] = rtmp[k][3]
          + tz3 * C3 * C4 * ( flux_temp[k+1][3] - flux_temp[k][3] )
          + dz4 * tz1 * (         utmp[k-1][3]
                          - 2.0 * utmp[k][3]
                          +       utmp[k+1][3] );
        rtmp[k][4] = rtmp[k][4]
          + tz3 * C3 * C4 * ( flux_temp[k+1][4] - flux_temp[k][4] )
          + dz5 * tz1 * (         utmp[k-1][4]
                          - 2.0 * utmp[k][4]
                          +       utmp[k+1][4] );
	  
      }

      //---------------------------------------------------------------------
      // fourth-order dissipation
      //---------------------------------------------------------------------
       if(z == 0) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[3][j][i][m] = rtmp[3][m]
          - dssp * ( + 5.0 * utmp[3][m]
                     - 4.0 * utmp[4][m]
                     +       utmp[5][m] );
	  rsd_tile[4][j][i][m] = rtmp[4][m]
          - dssp * ( - 4.0 * utmp[3][m]
                     + 6.0 * utmp[4][m]
                     - 4.0 * utmp[5][m]
                     +       utmp[6][m] );
	}
      }

      if (z == 0) k_first = 5; // 4th element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 5; 
      else k_last = nz_tile - 2;
      
      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rtmp[k][m]
            - dssp * (         utmp[k-2][m]
                       - 4.0 * utmp[k-1][m]
                       + 6.0 * utmp[k][m]
                       - 4.0 * utmp[k+1][m]
                       +       utmp[k+2][m] );
        }
      }

      if(z == nd_size.values[0] - 1) {
      k_last = nz_tile - 2;
	for (m = 0; m < 5; m++) {
	  rsd_tile[k_last-3][j][i][m] = rtmp[k_last-3][m]
	    - dssp * (         utmp[k_last-5][m]
                     - 4.0 * utmp[k_last-4][m]
                     + 6.0 * utmp[k_last-3][m]
                     - 4.0 * utmp[k_last-2][m] );
	  rsd_tile[k_last-2][j][i][m] = rtmp[k_last-2][m]
	    - dssp * (         utmp[k_last-4][m]
                     - 4.0 * utmp[k_last-3][m]
                     + 5.0 * utmp[k_last-2][m] );
	}
      }
    }
  }
  
}

void rhs_flux_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile)
{
  
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  double q;
  double tmp, utmp[ISIZ3+4][6], rtmp[ISIZ3+4][5];
  double u21, u31, u41;
  double u21i, u31i, u41i, u51i;
  double u21j, u31j, u41j, u51j;
  double u21k, u31k, u41k, u51k;
  double u21im1, u31im1, u41im1, u51im1;
  double u21jm1, u31jm1, u41jm1, u51jm1;
  double u21km1, u31km1, u41km1, u51km1;
  double flux_temp[ISIZ1+4][5];
  
//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;
  
  int i_first, j_first, k_first, i_last, j_last, k_last;
  int x, y, z;
  
  //int nm_tile = s1_tile->flat_size->values[3]; // 5 for u and rsd, 1 qs and rho_i
  int nx_tile = s1_tile->flat_size.values[2];
  int ny_tile = s1_tile->flat_size.values[1];
  int nz_tile = s1_tile->flat_size.values[0];

  
  double (*rsd_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s1_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s2_tile);
  double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s3_tile);
  double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s4_tile);
  
  
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
   
  //---------------------------------------------------------------------
  // xi-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
  else j_last = ny_tile - 2;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
  else k_last = nz_tile - 2;
  
  for (k = k_first; k < k_last; k++) {
    for (j = j_first; j < j_last; j++) {
      
      i_first = 1; // Because later flux[i-1] and flux[i+1] is required
      i_last = nx_tile-1;// Because later flux[i-1] and flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) {
        flux_temp[i][0] = u_tile[k][j][i][1];
        u21 = u_tile[k][j][i][1] * rho_i_tile[k][j][i][0];

        q = qs_tile[k][j][i][0]; 

        flux_temp[i][1] = u_tile[k][j][i][1] * u21 + C2 * ( u_tile[k][j][i][4] - q );
        flux_temp[i][2] = u_tile[k][j][i][2] * u21;
        flux_temp[i][3] = u_tile[k][j][i][3] * u21;
        flux_temp[i][4] = ( C1 * u_tile[k][j][i][4] - C2 * q ) * u21;
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
      else i_last = nx_tile - 2;

      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] =  rsd_tile[k][j][i][m]
            - tx2 * ( flux_temp[i+1][m] - flux_temp[i-1][m] );
	  }
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      i_last = nx_tile - 1; // Because later flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) {
        tmp = rho_i_tile[k][j][i][0];

        u21i = tmp * u_tile[k][j][i][1];
        u31i = tmp * u_tile[k][j][i][2];
        u41i = tmp * u_tile[k][j][i][3];
        u51i = tmp * u_tile[k][j][i][4];

        tmp = rho_i_tile[k][j][i-1][0];

        u21im1 = tmp * u_tile[k][j][i-1][1];
        u31im1 = tmp * u_tile[k][j][i-1][2];
        u41im1 = tmp * u_tile[k][j][i-1][3];
        u51im1 = tmp * u_tile[k][j][i-1][4];

        flux_temp[i][1] = (4.0/3.0) * tx3 * (u21i-u21im1);
        flux_temp[i][2] = tx3 * ( u31i - u31im1 );
        flux_temp[i][3] = tx3 * ( u41i - u41im1 );
        flux_temp[i][4] = 0.50 * ( 1.0 - C1*C5 )
          * tx3 * ( ( u21i*u21i     + u31i*u31i     + u41i*u41i )
                  - ( u21im1*u21im1 + u31im1*u31im1 + u41im1*u41im1 ) )
          + (1.0/6.0)
          * tx3 * ( u21i*u21i - u21im1*u21im1 )
          + C1 * C5 * tx3 * ( u51i - u51im1 );
      }
      
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 3;
      else i_last = nx_tile -2;

      for (i = i_first; i < i_last; i++) {
        rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0]
          + dx1 * tx1 * (        u_tile[k][j][i-1][0]
                         - 2.0 * u_tile[k][j][i][0]
                         +       u_tile[k][j][i+1][0] );
        rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1]
          + tx3 * C3 * C4 * ( flux_temp[i+1][1] - flux_temp[i][1] )
          + dx2 * tx1 * (        u_tile[k][j][i-1][1]
                         - 2.0 * u_tile[k][j][i][1]
                         +       u_tile[k][j][i+1][1] );
        rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2]
          + tx3 * C3 * C4 * ( flux_temp[i+1][2] - flux_temp[i][2] )
          + dx3 * tx1 * (        u_tile[k][j][i-1][2]
                         - 2.0 * u_tile[k][j][i][2]
                         +       u_tile[k][j][i+1][2] );
        rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3]
          + tx3 * C3 * C4 * ( flux_temp[i+1][3] - flux_temp[i][3] )
          + dx4 * tx1 * (        u_tile[k][j][i-1][3]
                         - 2.0 * u_tile[k][j][i][3]
                         +       u_tile[k][j][i+1][3] );
        rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4]
          + tx3 * C3 * C4 * ( flux_temp[i+1][4] - flux_temp[i][4] )
          + dx5 * tx1 * (        u_tile[k][j][i-1][4]
                         - 2.0 * u_tile[k][j][i][4]
                         +       u_tile[k][j][i+1][4] );
      }

      //---------------------------------------------------------------------
      // Fourth-order dissipation
      //---------------------------------------------------------------------
       if(x == 0) 
       {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j][3][m] = rsd_tile[k][j][3][m]
	    - dssp * ( + 5.0 * u_tile[k][j][3][m]
                     - 4.0 * u_tile[k][j][4][m]
                     +       u_tile[k][j][5][m] );
	  rsd_tile[k][j][4][m] = rsd_tile[k][j][4][m]
	    - dssp * ( - 4.0 * u_tile[k][j][3][m]
                     + 6.0 * u_tile[k][j][4][m]
                     - 4.0 * u_tile[k][j][5][m]
                     +       u_tile[k][j][6][m] );
	}
      }

      if (x == 0) i_first = 5; // 4th element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 5; 
      else i_last = nx_tile - 2;
      
      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rsd_tile[k][j][i][m]
            - dssp * (         u_tile[k][j][i-2][m]
                       - 4.0 * u_tile[k][j][i-1][m]
                       + 6.0 * u_tile[k][j][i][m]
                       - 4.0 * u_tile[k][j][i+1][m]
                       +       u_tile[k][j][i+2][m] );
        }
      }

      if(x == nd_size.values[2] - 1) 
      {
	i_last = nx_tile - 2;
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j][i_last-3][m] = rsd_tile[k][j][i_last-3][m]
	    - dssp * (         u_tile[k][j][i_last-5][m]
                     - 4.0 * u_tile[k][j][i_last-4][m]
                     + 6.0 * u_tile[k][j][i_last-3][m]
                     - 4.0 * u_tile[k][j][i_last-2][m] );
	  rsd_tile[k][j][i_last-2][m] = rsd_tile[k][j][i_last-2][m]
	    - dssp * (         u_tile[k][j][i_last-4][m]
                     - 4.0 * u_tile[k][j][i_last-3][m]
                     + 5.0 * u_tile[k][j][i_last-2][m] );	  
	}
      }     
     }
   }
  
  
  //---------------------------------------------------------------------
  // eta-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
  else i_last = nx_tile - 2;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
  else k_last = nz_tile - 2;
  
  for (k = k_first; k < k_last; k++) {
    for (i = i_first; i < i_last; i++) {
      
      j_first = 1; // Because later flux_temp[j-1] and flux_temp[j+1] is required
      j_last = ny_tile - 1;// Because later flux_temp[j-1] and flux_temp[j+1] is required
      
      for (j = j_first; j < j_last; j++) {
        flux_temp[j][0] = u_tile[k][j][i][2];
        u31 = u_tile[k][j][i][2] * rho_i_tile[k][j][i][0];

        q = qs_tile[k][j][i][0];

        flux_temp[j][1] = u_tile[k][j][i][1] * u31;
        flux_temp[j][2] = u_tile[k][j][i][2] * u31 + C2 * (u_tile[k][j][i][4]-q);
        flux_temp[j][3] = u_tile[k][j][i][3] * u31;
        flux_temp[j][4] = ( C1 * u_tile[k][j][i][4] - C2 * q ) * u31;
      }

      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
      else j_last = ny_tile - 2;
      
      for (j = j_first; j < j_last; j++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] =  rsd_tile[k][j][i][m]
            - ty2 * ( flux_temp[j+1][m] - flux_temp[j-1][m] );
        }
      }

      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      j_last = ny_tile - 1; // Because later flux[j+1] is required
      
      for (j = j_first; j < j_last; j++) {
        tmp = rho_i_tile[k][j][i][0];

        u21j = tmp * u_tile[k][j][i][1];
        u31j = tmp * u_tile[k][j][i][2];
        u41j = tmp * u_tile[k][j][i][3];
        u51j = tmp * u_tile[k][j][i][4];

        tmp = rho_i_tile[k][j-1][i][0];
        u21jm1 = tmp * u_tile[k][j-1][i][1];
        u31jm1 = tmp * u_tile[k][j-1][i][2];
        u41jm1 = tmp * u_tile[k][j-1][i][3];
        u51jm1 = tmp * u_tile[k][j-1][i][4];

        flux_temp[j][1] = ty3 * ( u21j - u21jm1 );
        flux_temp[j][2] = (4.0/3.0) * ty3 * (u31j-u31jm1);
        flux_temp[j][3] = ty3 * ( u41j - u41jm1 );
        flux_temp[j][4] = 0.50 * ( 1.0 - C1*C5 )
          * ty3 * ( ( u21j*u21j     + u31j*u31j     + u41j*u41j )
                  - ( u21jm1*u21jm1 + u31jm1*u31jm1 + u41jm1*u41jm1 ) )
          + (1.0/6.0)
          * ty3 * ( u31j*u31j - u31jm1*u31jm1 )
          + C1 * C5 * ty3 * ( u51j - u51jm1 );
      }
      
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3;
      else j_last = ny_tile -2;

      for (j = j_first; j < j_last; j++) {
        rsd_tile[k][j][i][0] = rsd_tile[k][j][i][0]
          + dy1 * ty1 * (         u_tile[k][j-1][i][0]
                          - 2.0 * u_tile[k][j][i][0]
                          +       u_tile[k][j+1][i][0] );

        rsd_tile[k][j][i][1] = rsd_tile[k][j][i][1]
          + ty3 * C3 * C4 * ( flux_temp[j+1][1] - flux_temp[j][1] )
          + dy2 * ty1 * (         u_tile[k][j-1][i][1]
                          - 2.0 * u_tile[k][j][i][1]
                          +       u_tile[k][j+1][i][1] );

        rsd_tile[k][j][i][2] = rsd_tile[k][j][i][2]
          + ty3 * C3 * C4 * ( flux_temp[j+1][2] - flux_temp[j][2] )
          + dy3 * ty1 * (         u_tile[k][j-1][i][2]
                          - 2.0 * u_tile[k][j][i][2]
                          +       u_tile[k][j+1][i][2] );

        rsd_tile[k][j][i][3] = rsd_tile[k][j][i][3]
          + ty3 * C3 * C4 * ( flux_temp[j+1][3] - flux_temp[j][3] )
          + dy4 * ty1 * (         u_tile[k][j-1][i][3]
                          - 2.0 * u_tile[k][j][i][3]
                          +       u_tile[k][j+1][i][3] );

        rsd_tile[k][j][i][4] = rsd_tile[k][j][i][4]
          + ty3 * C3 * C4 * ( flux_temp[j+1][4] - flux_temp[j][4] )
          + dy5 * ty1 * (         u_tile[k][j-1][i][4]
                          - 2.0 * u_tile[k][j][i][4]
                          +       u_tile[k][j+1][i][4] );
      }
    } // end of i
    
    //---------------------------------------------------------------------
    // fourth-order dissipation
    //---------------------------------------------------------------------
   
    if (x == 0) i_first = 3; // 2nd element
    else i_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
    else i_last = nx_tile - 2;
    
    if(y == 0) {
      for (i = i_first; i < i_last; i++) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][3][i][m] = rsd_tile[k][3][i][m]
	    - dssp * ( + 5.0 * u_tile[k][3][i][m]
                     - 4.0 * u_tile[k][4][i][m]
                     +       u_tile[k][5][i][m] );
	  rsd_tile[k][4][i][m] = rsd_tile[k][4][i][m]
	    - dssp * ( - 4.0 * u_tile[k][3][i][m]
                     + 6.0 * u_tile[k][4][i][m]
                     - 4.0 * u_tile[k][5][i][m]
                     +       u_tile[k][6][i][m] );
	}
      }
    }

    if (y == 0) j_first = 5; // 4th element
    else j_first = 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 5; 
    else j_last = ny_tile - 2;
      
    for (j = j_first; j < j_last; j++) {
      for (i = i_first; i < i_last; i++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rsd_tile[k][j][i][m]
            - dssp * (         u_tile[k][j-2][i][m]
                       - 4.0 * u_tile[k][j-1][i][m]
                       + 6.0 * u_tile[k][j][i][m]
                       - 4.0 * u_tile[k][j+1][i][m]
                       +       u_tile[k][j+2][i][m] );
        }
      }
    }

    if(y == nd_size.values[1] - 1) {
      j_last = ny_tile - 2;
      for (i = i_first; i < i_last; i++) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[k][j_last-3][i][m] = rsd_tile[k][j_last-3][i][m]
	    - dssp * (         u_tile[k][j_last-5][i][m]
                     - 4.0 * u_tile[k][j_last-4][i][m]
                     + 6.0 * u_tile[k][j_last-3][i][m]
                     - 4.0 * u_tile[k][j_last-2][i][m] );
	  rsd_tile[k][j_last-2][i][m] = rsd_tile[k][j_last-2][i][m]
	    - dssp * (         u_tile[k][j_last-4][i][m]
                     - 4.0 * u_tile[k][j_last-3][i][m]
                     + 5.0 * u_tile[k][j_last-2][i][m] );
	}
      }
    }  
}

  //---------------------------------------------------------------------
  // zeta-direction flux differences
  //---------------------------------------------------------------------
  // sync_boundary due to overlapping
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
  else i_last = nx_tile - 2;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
  else j_last = ny_tile - 2;
  
  for (j = j_first; j < j_last; j++) {
    for (i = i_first; i < i_last; i++) {
      
      if (z == 0) k_first = 2; 
      else k_first = 0; // Because later utemp[k+2], utemp[k-2], flux[k-1] and flux[k+1] is required
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 2; 
      else k_last = nz_tile;
      
      for (k = k_first; k < k_last; k++) {
        utmp[k][0] = u_tile[k][j][i][0];
        utmp[k][1] = u_tile[k][j][i][1];
        utmp[k][2] = u_tile[k][j][i][2];
        utmp[k][3] = u_tile[k][j][i][3];
        utmp[k][4] = u_tile[k][j][i][4];
        utmp[k][5] = rho_i_tile[k][j][i][0];
      }
      
      for (k = k_first; k < k_last; k++) {
        flux_temp[k][0] = utmp[k][3];
        u41 = utmp[k][3] * utmp[k][5];

        q = qs_tile[k][j][i][0];

        flux_temp[k][1] = utmp[k][1] * u41;
        flux_temp[k][2] = utmp[k][2] * u41;
        flux_temp[k][3] = utmp[k][3] * u41 + C2 * (utmp[k][4]-q);
        flux_temp[k][4] = ( C1 * utmp[k][4] - C2 * q ) * u41;
      }
      
      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
      else k_last = nz_tile - 2;

      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          rtmp[k][m] =  rsd_tile[k][j][i][m]
            - tz2 * ( flux_temp[k+1][m] - flux_temp[k-1][m] );
        }
      }
      
      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      k_last = nz_tile - 1; // Because later flux[k+1] is required

      for (k = k_first; k < k_last; k++) {
        tmp = utmp[k][5];

        u21k = tmp * utmp[k][1];
        u31k = tmp * utmp[k][2];
        u41k = tmp * utmp[k][3];
        u51k = tmp * utmp[k][4];

        tmp = utmp[k-1][5];

        u21km1 = tmp * utmp[k-1][1];
        u31km1 = tmp * utmp[k-1][2];
        u41km1 = tmp * utmp[k-1][3];
        u51km1 = tmp * utmp[k-1][4];

        flux_temp[k][1] = tz3 * ( u21k - u21km1 );
        flux_temp[k][2] = tz3 * ( u31k - u31km1 );
        flux_temp[k][3] = (4.0/3.0) * tz3 * (u41k-u41km1);
        flux_temp[k][4] = 0.50 * ( 1.0 - C1*C5 )
          * tz3 * ( ( u21k*u21k     + u31k*u31k     + u41k*u41k )
                  - ( u21km1*u21km1 + u31km1*u31km1 + u41km1*u41km1 ) )
          + (1.0/6.0)
          * tz3 * ( u41k*u41k - u41km1*u41km1 )
          + C1 * C5 * tz3 * ( u51k - u51km1 );
      }
      
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3;
      else k_last = nz_tile-2;
      
      for (k = k_first; k < k_last; k++) {
        rtmp[k][0] = rtmp[k][0]
          + dz1 * tz1 * (         utmp[k-1][0]
                          - 2.0 * utmp[k][0]
                          +       utmp[k+1][0] );
        rtmp[k][1] = rtmp[k][1]
          + tz3 * C3 * C4 * ( flux_temp[k+1][1] - flux_temp[k][1] )
          + dz2 * tz1 * (         utmp[k-1][1]
                          - 2.0 * utmp[k][1]
                          +       utmp[k+1][1] );
        rtmp[k][2] = rtmp[k][2]
          + tz3 * C3 * C4 * ( flux_temp[k+1][2] - flux_temp[k][2] )
          + dz3 * tz1 * (         utmp[k-1][2]
                          - 2.0 * utmp[k][2]
                          +       utmp[k+1][2] );
        rtmp[k][3] = rtmp[k][3]
          + tz3 * C3 * C4 * ( flux_temp[k+1][3] - flux_temp[k][3] )
          + dz4 * tz1 * (         utmp[k-1][3]
                          - 2.0 * utmp[k][3]
                          +       utmp[k+1][3] );
        rtmp[k][4] = rtmp[k][4]
          + tz3 * C3 * C4 * ( flux_temp[k+1][4] - flux_temp[k][4] )
          + dz5 * tz1 * (         utmp[k-1][4]
                          - 2.0 * utmp[k][4]
                          +       utmp[k+1][4] );
	  
      }

      //---------------------------------------------------------------------
      // fourth-order dissipation
      //---------------------------------------------------------------------
       if(z == 0) {
	for (m = 0; m < 5; m++) {
	  rsd_tile[3][j][i][m] = rtmp[3][m]
          - dssp * ( + 5.0 * utmp[3][m]
                     - 4.0 * utmp[4][m]
                     +       utmp[5][m] );
	  rsd_tile[4][j][i][m] = rtmp[4][m]
          - dssp * ( - 4.0 * utmp[3][m]
                     + 6.0 * utmp[4][m]
                     - 4.0 * utmp[5][m]
                     +       utmp[6][m] );
	}
      }

      if (z == 0) k_first = 5; // 4th element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 5; 
      else k_last = nz_tile - 2;
      
      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] = rtmp[k][m]
            - dssp * (         utmp[k-2][m]
                       - 4.0 * utmp[k-1][m]
                       + 6.0 * utmp[k][m]
                       - 4.0 * utmp[k+1][m]
                       +       utmp[k+2][m] );
        }
      }

      if(z == nd_size.values[0] - 1) {
      k_last = nz_tile - 2;
	for (m = 0; m < 5; m++) {
	  rsd_tile[k_last-3][j][i][m] = rtmp[k_last-3][m]
	    - dssp * (         utmp[k_last-5][m]
                     - 4.0 * utmp[k_last-4][m]
                     + 6.0 * utmp[k_last-3][m]
                     - 4.0 * utmp[k_last-2][m] );
	  rsd_tile[k_last-2][j][i][m] = rtmp[k_last-2][m]
	    - dssp * (         utmp[k_last-4][m]
                     - 4.0 * utmp[k_last-3][m]
                     + 5.0 * utmp[k_last-2][m] );
	}
      }
    }
  }
  
}


void init_rhs_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, void *scalar)
{
    int i, j, k, m;
    int i_first, j_first, k_first, i_last, j_last, k_last;
    int x, y, z;
    double tmp;
  
    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];
    
    double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
    double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);
    double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s3_tile);
    double (*frct_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s4_tile);
    double (*rsd_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s5_tile);
    
    // Get global tile nd_index first
    //Tuple* nd_size = s2->tiling; // tile dimensions
    Tuple nd_size = s5_tile->nd_tile_dimensions;
    Tuple nd_idx = s5_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);
       
    x = nd_idx.values[2];
    y = nd_idx.values[1];
    z = nd_idx.values[0];
    
    // sync_boundary due to overlapping
    // These limits because frct is synchronized
    if (x == 0) i_first = 2;
    else i_first = 0;
    if (y == 0) j_first = 2;
    else j_first = 0;
    if (z == 0) k_first = 2;
    else k_first = 0;
    if (x == nd_size.values[2] - 1) i_last = nx_tile-2;
    else i_last = nx_tile;
    if (y == nd_size.values[1] - 1) j_last = ny_tile-2;
    else j_last = ny_tile;
    if (z == nd_size.values[0] - 1) k_last = nz_tile-2;
    else k_last = nz_tile;
  
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	for (i = i_first; i < i_last; i++) {
	  tmp = 1.0 / u_tile[k][j][i][0];
	  rho_i_tile[k][j][i][0] = tmp;
	  qs_tile[k][j][i][0] = 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
                              + u_tile[k][j][i][2] * u_tile[k][j][i][2]
                              + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
			    * tmp;
	  for (m = 0; m < 5; m++) {
	    rsd_tile[k][j][i][m] = - frct_tile[k][j][i][m];
	  }
	}
      }
    } 
}


