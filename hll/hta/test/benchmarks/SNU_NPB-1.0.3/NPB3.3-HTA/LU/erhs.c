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


//---------------------------------------------------------------------
//
// compute the right hand side based on exact solution
//
//---------------------------------------------------------------------
void erhs_HTA()
{
  double zero = 0.0;
  
  HTA_map_h1s1(HTA_LEAF_LEVEL(frct_HTA), H1S1_INIT, frct_HTA, &zero);
  
  HTA_map_h1(HTA_LEAF_LEVEL(rsd_HTA), erhs_init_ce_HTA, rsd_HTA);

  HTA_map_h2(HTA_LEAF_LEVEL(frct_HTA), erhs_flux_HTA, frct_HTA, rsd_HTA);
  
  sync_boundary(frct_HTA);

}

//void erhs_flux_HTA(HTA* d_tile, HTA* s1_tile, HTA* s2)
void erhs_flux_HTA(HTA* d_tile, HTA* s1_tile)
{
  
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  double q;
  double u21, u31, u41;
  double tmp;
  double u21i, u31i, u41i, u51i;
  double u21j, u31j, u41j, u51j;
  double u21k, u31k, u41k, u51k;
  double u21im1, u31im1, u41im1, u51im1;
  double u21jm1, u31jm1, u41jm1, u51jm1;
  double u21km1, u31km1, u41km1, u51km1;
  double flux_temp[ISIZ1+4][5];
  
  int i_first, j_first, k_first, i_last, j_last, k_last;
  int x, y, z;
  
  int nm_tile = s1_tile->flat_size.values[3]; // Always 5
  int nx_tile = s1_tile->flat_size.values[2];
  int ny_tile = s1_tile->flat_size.values[1];
  int nz_tile = s1_tile->flat_size.values[0];
  
  // Get global tile nd_index first
  //Tuple* nd_size = s2->tiling; // tile dimensions
  Tuple nd_size = s1_tile->nd_tile_dimensions;
  Tuple nd_idx = s1_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);
  
  double (*frct_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(d_tile);
  double (*rsd_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s1_tile);
  
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
  
  for (k = k_first; k < k_last; k++) { // Original: k = 1; k < nz - 1; 
    for (j = j_first; j < j_last; j++) { // Original: j = jst; j < jend;
         
      i_first = 1; // Because later flux[i-1] and flux[i+1] is required
      i_last = nx_tile - 1;// Because later flux[i-1] and flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) { // Original: i = 0; i < nx;
        flux_temp[i][0] = rsd_tile[k][j][i][1];
        u21 = rsd_tile[k][j][i][1] / rsd_tile[k][j][i][0];
        q = 0.50 * (  rsd_tile[k][j][i][1] * rsd_tile[k][j][i][1]
                    + rsd_tile[k][j][i][2] * rsd_tile[k][j][i][2]
                    + rsd_tile[k][j][i][3] * rsd_tile[k][j][i][3] )
                 / rsd_tile[k][j][i][0];
        flux_temp[i][1] = rsd_tile[k][j][i][1] * u21 + C2 * ( rsd_tile[k][j][i][4] - q );
        flux_temp[i][2] = rsd_tile[k][j][i][2] * u21;
        flux_temp[i][3] = rsd_tile[k][j][i][3] * u21;
        flux_temp[i][4] = ( C1 * rsd_tile[k][j][i][4] - C2 * q ) * u21;
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Next to last element
      else i_last = nx_tile - 2;
      
      for (i = i_first; i < i_last; i++) { // Original: i = ist; i < iend
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] =  frct_tile[k][j][i][m]
                    - tx2 * ( flux_temp[i+1][m] - flux_temp[i-1][m] );
        }
      }
      
      if (x == 0) i_first = 3; // 2nd element
      else i_first = 2;
      i_last = nx_tile - 1; // Because later flux[i+1] is required
      
      for (i = i_first; i < i_last; i++) { // Original: i = ist; i < nx 
        tmp = 1.0 / rsd_tile[k][j][i][0];

        u21i = tmp * rsd_tile[k][j][i][1];
        u31i = tmp * rsd_tile[k][j][i][2];
        u41i = tmp * rsd_tile[k][j][i][3];
        u51i = tmp * rsd_tile[k][j][i][4];

        tmp = 1.0 / rsd_tile[k][j][i-1][0];

        u21im1 = tmp * rsd_tile[k][j][i-1][1];
        u31im1 = tmp * rsd_tile[k][j][i-1][2];
        u41im1 = tmp * rsd_tile[k][j][i-1][3];
        u51im1 = tmp * rsd_tile[k][j][i-1][4];

        flux_temp[i][1] = (4.0/3.0) * tx3 * ( u21i - u21im1 );
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
      
      for (i = i_first; i < i_last; i++) { // Original: i = ist; i < iend;
        frct_tile[k][j][i][0] = frct_tile[k][j][i][0]
          + dx1 * tx1 * (        rsd_tile[k][j][i-1][0]
                         - 2.0 * rsd_tile[k][j][i][0]
                         +       rsd_tile[k][j][i+1][0] );
        frct_tile[k][j][i][1] = frct_tile[k][j][i][1]
          + tx3 * C3 * C4 * ( flux_temp[i+1][1] - flux_temp[i][1] )
          + dx2 * tx1 * (        rsd_tile[k][j][i-1][1]
                         - 2.0 * rsd_tile[k][j][i][1]
                         +       rsd_tile[k][j][i+1][1] );
        frct_tile[k][j][i][2] = frct_tile[k][j][i][2]
          + tx3 * C3 * C4 * ( flux_temp[i+1][2] - flux_temp[i][2] )
          + dx3 * tx1 * (        rsd_tile[k][j][i-1][2]
                         - 2.0 * rsd_tile[k][j][i][2]
                         +       rsd_tile[k][j][i+1][2] );
        frct_tile[k][j][i][3] = frct_tile[k][j][i][3]
          + tx3 * C3 * C4 * ( flux_temp[i+1][3] - flux_temp[i][3] )
          + dx4 * tx1 * (        rsd_tile[k][j][i-1][3]
                         - 2.0 * rsd_tile[k][j][i][3]
                         +       rsd_tile[k][j][i+1][3] );
        frct_tile[k][j][i][4] = frct_tile[k][j][i][4]
          + tx3 * C3 * C4 * ( flux_temp[i+1][4] - flux_temp[i][4] )
          + dx5 * tx1 * (        rsd_tile[k][j][i-1][4]
                         - 2.0 * rsd_tile[k][j][i][4]
                         +       rsd_tile[k][j][i+1][4] );
      }

      //---------------------------------------------------------------------
      // Fourth-order dissipation
      //---------------------------------------------------------------------
      if(x == 0) {
       for (m = 0; m < 5; m++) { // +2 indices original code: overlapping
	  frct_tile[k][j][3][m] = frct_tile[k][j][3][m]
	    - dssp * ( + 5.0 * rsd_tile[k][j][3][m]
		      - 4.0 * rsd_tile[k][j][4][m]
		      +       rsd_tile[k][j][5][m] );
	  frct_tile[k][j][4][m] = frct_tile[k][j][4][m]
	    - dssp * ( - 4.0 * rsd_tile[k][j][3][m]
		      + 6.0 * rsd_tile[k][j][4][m]
		      - 4.0 * rsd_tile[k][j][5][m]
		      +       rsd_tile[k][j][6][m] );
	}
      }

      if (x == 0) i_first = 5; // 4th element
      else i_first = 2;
      if (x == nd_size.values[2] - 1) i_last = nx_tile - 5; 
      else i_last = nx_tile - 2;
      
      for (i = i_first; i < i_last; i++) { // Original: i = 3; i < nx - 3;
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] = frct_tile[k][j][i][m]
            - dssp * (        rsd_tile[k][j][i-2][m]
                      - 4.0 * rsd_tile[k][j][i-1][m]
                      + 6.0 * rsd_tile[k][j][i][m]
                      - 4.0 * rsd_tile[k][j][i+1][m]
                      +       rsd_tile[k][j][i+2][m] );
        }
      }

      if(x == nd_size.values[2] - 1) {
	i_last = nx_tile - 2;
	for (m = 0; m < 5; m++) {
	  frct_tile[k][j][i_last-3][m] = frct_tile[k][j][i_last-3][m]
	    - dssp * (        rsd_tile[k][j][i_last-5][m]
		      - 4.0 * rsd_tile[k][j][i_last-4][m]
		      + 6.0 * rsd_tile[k][j][i_last-3][m]
		      - 4.0 * rsd_tile[k][j][i_last-2][m] );
	  frct_tile[k][j][i_last-2][m] = frct_tile[k][j][i_last-2][m]
	    - dssp * (        rsd_tile[k][j][i_last-4][m]
		      - 4.0 * rsd_tile[k][j][i_last-3][m]
		      + 5.0 * rsd_tile[k][j][i_last-2][m] );
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
       
      j_first = 1; // Because later flux[j-1] and flux[j+1] is required
      j_last = ny_tile - 1;// Because later flux[j-1] and flux[j+1] is required
       
       for (j = j_first; j < j_last; j++) {
        flux_temp[j][0] = rsd_tile[k][j][i][2];
        u31 = rsd_tile[k][j][i][2] / rsd_tile[k][j][i][0];
        q = 0.50 * (  rsd_tile[k][j][i][1] * rsd_tile[k][j][i][1]
                    + rsd_tile[k][j][i][2] * rsd_tile[k][j][i][2]
                    + rsd_tile[k][j][i][3] * rsd_tile[k][j][i][3] )
                 / rsd_tile[k][j][i][0];
        flux_temp[j][1] = rsd_tile[k][j][i][1] * u31;
        flux_temp[j][2] = rsd_tile[k][j][i][2] * u31 + C2 * ( rsd_tile[k][j][i][4] - q );
        flux_temp[j][3] = rsd_tile[k][j][i][3] * u31;
        flux_temp[j][4] = ( C1 * rsd_tile[k][j][i][4] - C2 * q ) * u31;
      }
      
      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // Next to last element
      else j_last = ny_tile - 2;

      for (j = j_first; j < j_last; j++) {
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] =  frct_tile[k][j][i][m]
            - ty2 * ( flux_temp[j+1][m] - flux_temp[j-1][m] );
        }
      }

      if (y == 0) j_first = 3; // 2nd element
      else j_first = 2;
      j_last = ny_tile - 1; // Because later flux[j+1] is required
      
      for (j = j_first; j < j_last; j++) {
        tmp = 1.0 / rsd_tile[k][j][i][0];

        u21j = tmp * rsd_tile[k][j][i][1];
        u31j = tmp * rsd_tile[k][j][i][2];
        u41j = tmp * rsd_tile[k][j][i][3];
        u51j = tmp * rsd_tile[k][j][i][4];

        tmp = 1.0 / rsd_tile[k][j-1][i][0];

        u21jm1 = tmp * rsd_tile[k][j-1][i][1];
        u31jm1 = tmp * rsd_tile[k][j-1][i][2];
        u41jm1 = tmp * rsd_tile[k][j-1][i][3];
        u51jm1 = tmp * rsd_tile[k][j-1][i][4];

        flux_temp[j][1] = ty3 * ( u21j - u21jm1 );
        flux_temp[j][2] = (4.0/3.0) * ty3 * ( u31j - u31jm1 );
        flux_temp[j][3] = ty3 * ( u41j - u41jm1 );
        flux_temp[j][4] = 0.50 * ( 1.0 - C1*C5 )
          * ty3 * ( ( u21j*u21j     + u31j*u31j     + u41j*u41j )
                  - ( u21jm1*u21jm1 + u31jm1*u31jm1 + u41jm1*u41jm1 ) )
          + (1.0/6.0)
          * ty3 * ( u31j*u31j - u31jm1*u31jm1 )
          + C1 * C5 * ty3 * ( u51j - u51jm1 );
      }

      if (y == nd_size.values[1] - 1) j_last = ny_tile - 3;
      else j_last = ny_tile - 2;
      
      for (j = j_first; j < j_last; j++) {
        frct_tile[k][j][i][0] = frct_tile[k][j][i][0]
          + dy1 * ty1 * (        rsd_tile[k][j-1][i][0]
                         - 2.0 * rsd_tile[k][j][i][0]
                         +       rsd_tile[k][j+1][i][0] );
        frct_tile[k][j][i][1] = frct_tile[k][j][i][1]
          + ty3 * C3 * C4 * ( flux_temp[j+1][1] - flux_temp[j][1] )
          + dy2 * ty1 * (        rsd_tile[k][j-1][i][1]
                         - 2.0 * rsd_tile[k][j][i][1]
                         +       rsd_tile[k][j+1][i][1] );
        frct_tile[k][j][i][2] = frct_tile[k][j][i][2]
          + ty3 * C3 * C4 * ( flux_temp[j+1][2] - flux_temp[j][2] )
          + dy3 * ty1 * (        rsd_tile[k][j-1][i][2]
                         - 2.0 * rsd_tile[k][j][i][2]
                         +       rsd_tile[k][j+1][i][2] );
        frct_tile[k][j][i][3] = frct_tile[k][j][i][3]
          + ty3 * C3 * C4 * ( flux_temp[j+1][3] - flux_temp[j][3] )
          + dy4 * ty1 * (        rsd_tile[k][j-1][i][3]
                         - 2.0 * rsd_tile[k][j][i][3]
                         +       rsd_tile[k][j+1][i][3] );
        frct_tile[k][j][i][4] = frct_tile[k][j][i][4]
          + ty3 * C3 * C4 * ( flux_temp[j+1][4] - flux_temp[j][4] )
          + dy5 * ty1 * (        rsd_tile[k][j-1][i][4]
                         - 2.0 * rsd_tile[k][j][i][4]
                         +       rsd_tile[k][j+1][i][4] );
      }

      //---------------------------------------------------------------------
      // fourth-order dissipation
      //---------------------------------------------------------------------
      if(y == 0) {
	for (m = 0; m < 5; m++) {
        frct_tile[k][3][i][m] = frct_tile[k][3][i][m]
          - dssp * ( + 5.0 * rsd_tile[k][3][i][m]
                     - 4.0 * rsd_tile[k][4][i][m]
                     +       rsd_tile[k][5][i][m] );
        frct_tile[k][4][i][m] = frct_tile[k][4][i][m]
          - dssp * ( - 4.0 * rsd_tile[k][3][i][m]
                     + 6.0 * rsd_tile[k][4][i][m]
                     - 4.0 * rsd_tile[k][5][i][m]
                     +       rsd_tile[k][6][i][m] );
	}
      }

      if (y == 0) j_first = 5; // 4th element
      else j_first = 2;
      if (y == nd_size.values[1] - 1) j_last = ny_tile - 5; 
      else j_last = ny_tile - 2;
      
      for (j = j_first; j < j_last; j++) {
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] = frct_tile[k][j][i][m]
            - dssp * (        rsd_tile[k][j-2][i][m]
                      - 4.0 * rsd_tile[k][j-1][i][m]
                      + 6.0 * rsd_tile[k][j][i][m]
                      - 4.0 * rsd_tile[k][j+1][i][m]
                      +       rsd_tile[k][j+2][i][m] );
        }
      }
      
    if(y == nd_size.values[1] - 1) {
      j_last = ny_tile - 2;
      for (m = 0; m < 5; m++) {
        frct_tile[k][j_last-3][i][m] = frct_tile[k][j_last-3][i][m]
          - dssp * (        rsd_tile[k][j_last-5][i][m]
                    - 4.0 * rsd_tile[k][j_last-4][i][m]
                    + 6.0 * rsd_tile[k][j_last-3][i][m]
                    - 4.0 * rsd_tile[k][j_last-2][i][m] );
        frct_tile[k][j_last-2][i][m] = frct_tile[k][j_last-2][i][m]
          - dssp * (        rsd_tile[k][j_last-4][i][m]
                    - 4.0 * rsd_tile[k][j_last-3][i][m]
                    + 5.0 * rsd_tile[k][j_last-2][i][m] );
      }
    }
    
    } // end j
  } // end k
  
  //---------------------------------------------------------------------
  // zeta-direction flux_temp differences
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
      
      k_first = 1; // Because later flux[k-1] and flux[k+1] is required
      k_last = nz_tile - 1;// Because later flux[k-1] and flux[k+1] is required
      
      for (k = k_first; k < k_last; k++) {
        flux_temp[k][0] = rsd_tile[k][j][i][3];
        u41 = rsd_tile[k][j][i][3] / rsd_tile[k][j][i][0];
        q = 0.50 * (  rsd_tile[k][j][i][1] * rsd_tile[k][j][i][1]
                    + rsd_tile[k][j][i][2] * rsd_tile[k][j][i][2]
                    + rsd_tile[k][j][i][3] * rsd_tile[k][j][i][3] )
                 / rsd_tile[k][j][i][0];
        flux_temp[k][1] = rsd_tile[k][j][i][1] * u41;
        flux_temp[k][2] = rsd_tile[k][j][i][2] * u41; 
        flux_temp[k][3] = rsd_tile[k][j][i][3] * u41 + C2 * ( rsd_tile[k][j][i][4] - q );
        flux_temp[k][4] = ( C1 * rsd_tile[k][j][i][4] - C2 * q ) * u41;
      }
      
      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // Next to last element
      else k_last = nz_tile - 2;

      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] =  frct_tile[k][j][i][m]
            - tz2 * ( flux_temp[k+1][m] - flux_temp[k-1][m] );
        }
      }

      if (z == 0) k_first = 3; // 2nd element
      else k_first = 2;
      k_last = nz_tile - 1; // Because later flux[j+1] is required
      
      for (k = k_first; k < k_last; k++) {
        tmp = 1.0 / rsd_tile[k][j][i][0];

        u21k = tmp * rsd_tile[k][j][i][1];
        u31k = tmp * rsd_tile[k][j][i][2];
        u41k = tmp * rsd_tile[k][j][i][3];
        u51k = tmp * rsd_tile[k][j][i][4];

        tmp = 1.0 / rsd_tile[k-1][j][i][0];

        u21km1 = tmp * rsd_tile[k-1][j][i][1];
        u31km1 = tmp * rsd_tile[k-1][j][i][2];
        u41km1 = tmp * rsd_tile[k-1][j][i][3];
        u51km1 = tmp * rsd_tile[k-1][j][i][4];

        flux_temp[k][1] = tz3 * ( u21k - u21km1 );
        flux_temp[k][2] = tz3 * ( u31k - u31km1 );
        flux_temp[k][3] = (4.0/3.0) * tz3 * ( u41k - u41km1 );
        flux_temp[k][4] = 0.50 * ( 1.0 - C1*C5 )
          * tz3 * ( ( u21k*u21k     + u31k*u31k     + u41k*u41k )
                  - ( u21km1*u21km1 + u31km1*u31km1 + u41km1*u41km1 ) )
          + (1.0/6.0)
          * tz3 * ( u41k*u41k - u41km1*u41km1 )
          + C1 * C5 * tz3 * ( u51k - u51km1 );
      }

      if (z == nd_size.values[0] - 1) k_last = nz_tile - 3;
      else k_last = nz_tile -2;
      
      for (k = k_first; k < k_last; k++) {
        frct_tile[k][j][i][0] = frct_tile[k][j][i][0]
          + dz1 * tz1 * (        rsd_tile[k+1][j][i][0]
                         - 2.0 * rsd_tile[k][j][i][0]
                         +       rsd_tile[k-1][j][i][0] );
        frct_tile[k][j][i][1] = frct_tile[k][j][i][1]
          + tz3 * C3 * C4 * ( flux_temp[k+1][1] - flux_temp[k][1] )
          + dz2 * tz1 * (        rsd_tile[k+1][j][i][1]
                         - 2.0 * rsd_tile[k][j][i][1]
                         +       rsd_tile[k-1][j][i][1] );
        frct_tile[k][j][i][2] = frct_tile[k][j][i][2]
          + tz3 * C3 * C4 * ( flux_temp[k+1][2] - flux_temp[k][2] )
          + dz3 * tz1 * (        rsd_tile[k+1][j][i][2]
                         - 2.0 * rsd_tile[k][j][i][2]
                         +       rsd_tile[k-1][j][i][2] );
        frct_tile[k][j][i][3] = frct_tile[k][j][i][3]
          + tz3 * C3 * C4 * ( flux_temp[k+1][3] - flux_temp[k][3] )
          + dz4 * tz1 * (        rsd_tile[k+1][j][i][3]
                         - 2.0 * rsd_tile[k][j][i][3]
                         +       rsd_tile[k-1][j][i][3] );
        frct_tile[k][j][i][4] = frct_tile[k][j][i][4]
          + tz3 * C3 * C4 * ( flux_temp[k+1][4] - flux_temp[k][4] )
          + dz5 * tz1 * (        rsd_tile[k+1][j][i][4]
                         - 2.0 * rsd_tile[k][j][i][4]
                         +       rsd_tile[k-1][j][i][4] );
      }

      //---------------------------------------------------------------------
      // fourth-order dissipation
      //---------------------------------------------------------------------
      if(z == 0) {
	for (m = 0; m < 5; m++) {
        frct_tile[3][j][i][m] = frct_tile[3][j][i][m]
          - dssp * ( + 5.0 * rsd_tile[3][j][i][m]
                     - 4.0 * rsd_tile[4][j][i][m]
                     +       rsd_tile[5][j][i][m] );
        frct_tile[4][j][i][m] = frct_tile[4][j][i][m]
          - dssp * ( - 4.0 * rsd_tile[3][j][i][m]
                     + 6.0 * rsd_tile[4][j][i][m]
                     - 4.0 * rsd_tile[5][j][i][m]
                     +       rsd_tile[6][j][i][m] );
	}
      }

      if (z == 0) k_first = 5; // 4th element
      else k_first = 2;
      if (z == nd_size.values[0] - 1) k_last = nz_tile - 5; 
      else k_last = nz_tile - 2;
      
      for (k = k_first; k < k_last; k++) {
        for (m = 0; m < 5; m++) {
          frct_tile[k][j][i][m] = frct_tile[k][j][i][m]
            - dssp * (        rsd_tile[k-2][j][i][m]
                      - 4.0 * rsd_tile[k-1][j][i][m]
                      + 6.0 * rsd_tile[k][j][i][m]
                      - 4.0 * rsd_tile[k+1][j][i][m]
                      +       rsd_tile[k+2][j][i][m] );
        }
      }

      if(z == nd_size.values[0] - 1) {
	k_last = nz_tile - 2;
	for (m = 0; m < 5; m++) {
	  frct_tile[k_last-3][j][i][m] = frct_tile[k_last-3][j][i][m]
	    - dssp * (        rsd_tile[k_last-5][j][i][m]
                    - 4.0 * rsd_tile[k_last-4][j][i][m]
                    + 6.0 * rsd_tile[k_last-3][j][i][m]
                    - 4.0 * rsd_tile[k_last-2][j][i][m] );
	  frct_tile[k_last-2][j][i][m] = frct_tile[k_last-2][j][i][m]
	    - dssp * (        rsd_tile[k_last-4][j][i][m]
                    - 4.0 * rsd_tile[k_last-3][j][i][m]
                    + 5.0 * rsd_tile[k_last-2][j][i][m] );
	}
      }
      
    } 
  }
  
}


//void erhs_init_ce_HTA(HTA* dest, HTA* s1_tile, HTA* s2)
void erhs_init_ce_HTA(HTA* s1_tile)
{
    int i, j, k, m;
    int i_first, j_first, k_first, i_last, j_last, k_last;
    int i_offset, j_offset, k_offset, x, y, z;
    int i_real, j_real, k_real;
    double xi, eta, zeta;
    
    int nm_tile = s1_tile->flat_size.values[3];
    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];
    
    // Get global tile nd_index first
    //Tuple* nd_size = s2->tiling; // tile dimensions
    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, nd_size, &nd_idx);
  
    double (*rsd_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s1_tile);
   
    x = nd_idx.values[2];
    y = nd_idx.values[1];
    z = nd_idx.values[0];
    
    // sync_boundary due to overlapping
    if (x == 0) i_first = 2;
    else i_first = 0;
    if (y == 0) j_first = 2;
    else j_first = 0;
    if (z == 0) k_first = 2;
    else k_first = 0;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 2;
    else i_last = nx_tile;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 2;
    else j_last = ny_tile;
    if (z == nd_size.values[0] - 1) k_last = nz_tile - 2;
    else k_last = nz_tile;
  
    //FIXME: assuming regular tiles: problem with irregular tiles!!!
    //i_offset = x * (nx_tile - 4) - 2;
    //j_offset = y * (ny_tile - 4) - 2;
    //k_offset = z * (nz_tile - 4) - 2;
    
    i_offset = s1_tile->nd_element_offset.values[2] - (4 * x) - 2; 
    j_offset = s1_tile->nd_element_offset.values[1] - (4 * y) - 2;
    k_offset = s1_tile->nd_element_offset.values[0] - (4 * z) - 2;
    
    for (k = k_first; k < k_last; k++) {
    k_real = k + k_offset;  
    zeta = ( (double)k_real ) / ( nz - 1 );
    for (j = j_first; j < j_last; j++) {
      j_real = j + j_offset;
      eta = ( (double)j_real ) / ( ny0 - 1 );
      for (i = i_first; i < i_last; i++) {
	i_real = i + i_offset;
        xi = ( (double)i_real ) / ( nx0 - 1 );
        for (m = 0; m < 5; m++) {
          rsd_tile[k][j][i][m] =  ce[m][0]
            + (ce[m][1]
            + (ce[m][4]
            + (ce[m][7]
            +  ce[m][10] * xi) * xi) * xi) * xi
            + (ce[m][2]
            + (ce[m][5]
            + (ce[m][8]
            +  ce[m][11] * eta) * eta) * eta) * eta
            + (ce[m][3]
            + (ce[m][6]
            + (ce[m][9]
            +  ce[m][12] * zeta) * zeta) * zeta) * zeta;
        }
      }
    }
  }
  
  
}