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
// compute the upper triangular part of the jacobian matrix
//---------------------------------------------------------------------
void jacu_HTA()
{
   int dummy = 0;
  
  HTA_map_h7s1(HTA_LEAF_LEVEL(u_HTA), jacu_compute_HTA, du_HTA, au_HTA, bu_HTA, cu_HTA, rho_i_HTA, qs_HTA, u_HTA, &dummy);
  
  // Check this!!! Apparently syncronization is not required
  // sync_boundary(du_HTA);
  // sync_boundary(au_HTA);
  // sync_boundary(bu_HTA);
  // sync_boundary(cu_HTA);
  
}

void jacu_compute_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, HTA* s6_tile, HTA* s7_tile, void *scalar)
{  
  
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k;
  double r43;
  double c1345;
  double c34;
  double tmp1, tmp2, tmp3;
  
  int i_first, j_first, i_last, j_last, k_first, k_last;
  int x, y, z;
  
  //int nm_tile = s3_tile->flat_size->values[3];
  int nx_tile = s5_tile->flat_size.values[2];
  int ny_tile = s5_tile->flat_size.values[1];
  int nz_tile = s5_tile->flat_size.values[0]; 
    
  double (*du_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s1_tile);
  double (*au_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s2_tile);
  double (*bu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s3_tile);
  double (*cu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s4_tile);
  double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s5_tile);
  double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s6_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s7_tile);
  
  // FIXME: Trick to obtain the nd_idx of the tiles
  // Get global tile nd_index first
  //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1); // tile dimensions
  Tuple nd_size = s7_tile->nd_tile_dimensions;
  Tuple nd_idx = s7_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(s5_tile->rank, &nd_size, &nd_idx);  
  
  x = nd_idx.values[2];
  y = nd_idx.values[1];
  z = nd_idx.values[0];

  if (x == 0) i_first = 3; // 2nd element
  else i_first = 2;
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 2;
  if (z == 0) k_first = 3; // 2nd element
  else k_first = 2;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; // orig. loop: i = iend - 1; i >= ist; i--
  else i_last = nx_tile - 3;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; // orig. loop: j = jend - 1; j >= jst; j--
  else j_last = ny_tile - 3;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; // orig. loop: k = nz-2; k > 0; k--
  else k_last = nz_tile - 3;
  
  r43 = ( 4.0 / 3.0 );
  c1345 = C1 * C3 * C4 * C5;
  c34 = C3 * C4;
    
  for(k = k_last; k >= k_first; k--) {
    for (j = j_last; j >= j_first; j--) {
      for (i = i_last; i >= i_first; i--) {    
      //---------------------------------------------------------------------
      // form the block daigonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      du_tile[k][j][i][0] = 1.0 + dt * 2.0 * ( tx1 * dx1 + ty1 * dy1 + tz1 * dz1 );
      du_tile[k][j][i][5] = 0.0;
      du_tile[k][j][i][10] = 0.0;
      du_tile[k][j][i][15] = 0.0;
      du_tile[k][j][i][20] = 0.0;

      du_tile[k][j][i][1] =  dt * 2.0
        * ( - tx1 * r43 - ty1 - tz1 )
        * ( c34 * tmp2 * u_tile[k][j][i][1] );
      du_tile[k][j][i][6] =  1.0
        + dt * 2.0 * c34 * tmp1 
        * (  tx1 * r43 + ty1 + tz1 )
        + dt * 2.0 * ( tx1 * dx2 + ty1 * dy2 + tz1 * dz2 );
      du_tile[k][j][i][11] = 0.0;
      du_tile[k][j][i][16] = 0.0;
      du_tile[k][j][i][21] = 0.0;

      du_tile[k][j][i][2] = dt * 2.0
        * ( - tx1 - ty1 * r43 - tz1 )
        * ( c34 * tmp2 * u_tile[k][j][i][2] );
      du_tile[k][j][i][7] = 0.0;
      du_tile[k][j][i][12] = 1.0
        + dt * 2.0 * c34 * tmp1
        * (  tx1 + ty1 * r43 + tz1 )
        + dt * 2.0 * ( tx1 * dx3 + ty1 * dy3 + tz1 * dz3 );
      du_tile[k][j][i][17] = 0.0;
      du_tile[k][j][i][22] = 0.0;

      du_tile[k][j][i][3] = dt * 2.0
        * ( - tx1 - ty1 - tz1 * r43 )
        * ( c34 * tmp2 * u_tile[k][j][i][3] );
      du_tile[k][j][i][8] = 0.0;
      du_tile[k][j][i][13] = 0.0;
      du_tile[k][j][i][18] = 1.0
        + dt * 2.0 * c34 * tmp1
        * (  tx1 + ty1 + tz1 * r43 )
        + dt * 2.0 * ( tx1 * dx4 + ty1 * dy4 + tz1 * dz4 );
      du_tile[k][j][i][23] = 0.0;

      du_tile[k][j][i][4] = -dt * 2.0
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

      du_tile[k][j][i][9] = dt * 2.0
        * ( tx1 * ( r43*c34 - c1345 )
          + ty1 * (     c34 - c1345 )
          + tz1 * (     c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][1];
      du_tile[k][j][i][14] = dt * 2.0
        * ( tx1 * ( c34 - c1345 )
          + ty1 * ( r43*c34 -c1345 )
          + tz1 * ( c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][2];
      du_tile[k][j][i][19] = dt * 2.0
        * ( tx1 * ( c34 - c1345 )
          + ty1 * ( c34 - c1345 )
          + tz1 * ( r43*c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][3];
      du_tile[k][j][i][24] = 1.0
        + dt * 2.0 * ( tx1 + ty1 + tz1 ) * c1345 * tmp1
        + dt * 2.0 * ( tx1 * dx5 + ty1 * dy5 + tz1 * dz5 );
	
      //---------------------------------------------------------------------
      // form the first block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j][i+1][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      au_tile[k][j][i][0] = - dt * tx1 * dx1;
      au_tile[k][j][i][5] =   dt * tx2;
      au_tile[k][j][i][10] =   0.0;
      au_tile[k][j][i][15] =   0.0;
      au_tile[k][j][i][20] =   0.0;

      au_tile[k][j][i][1] =  dt * tx2
        * ( - ( u_tile[k][j][i+1][1] * tmp1 ) * ( u_tile[k][j][i+1][1] * tmp1 )
            + C2 * qs_tile[k][j][i+1][0] * tmp1 )
        - dt * tx1 * ( - r43 * c34 * tmp2 * u_tile[k][j][i+1][1] );
      au_tile[k][j][i][6] =  dt * tx2
        * ( ( 2.0 - C2 ) * ( u_tile[k][j][i+1][1] * tmp1 ) )
        - dt * tx1 * ( r43 * c34 * tmp1 )
        - dt * tx1 * dx2;
      au_tile[k][j][i][11] =  dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][2] * tmp1 ) );
      au_tile[k][j][i][16] =  dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][3] * tmp1 ) );
      au_tile[k][j][i][21] =  dt * tx2 * C2 ;

      au_tile[k][j][i][2] =  dt * tx2
        * ( - ( u_tile[k][j][i+1][1] * u_tile[k][j][i+1][2] ) * tmp2 )
        - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][2] );
      au_tile[k][j][i][7] =  dt * tx2 * ( u_tile[k][j][i+1][2] * tmp1 );
      au_tile[k][j][i][12] =  dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
        - dt * tx1 * ( c34 * tmp1 )
        - dt * tx1 * dx3;
      au_tile[k][j][i][17] = 0.0;
      au_tile[k][j][i][22] = 0.0;

      au_tile[k][j][i][3] = dt * tx2
        * ( - ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][3] ) * tmp2 )
        - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][3] );
      au_tile[k][j][i][8] = dt * tx2 * ( u_tile[k][j][i+1][3] * tmp1 );
      au_tile[k][j][i][13] = 0.0;
      au_tile[k][j][i][18] = dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
        - dt * tx1 * ( c34 * tmp1 )
        - dt * tx1 * dx4;
      au_tile[k][j][i][23] = 0.0;

      au_tile[k][j][i][4] = dt * tx2
        * ( ( C2 * 2.0 * qs_tile[k][j][i+1][0]
            - C1 * u_tile[k][j][i+1][4] )
        * ( u_tile[k][j][i+1][1] * tmp2 ) )
        - dt * tx1
        * ( - ( r43*c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] )
            - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][2] )
            - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][3] )
            - c1345 * tmp2 * u_tile[k][j][i+1][4] );
      au_tile[k][j][i][9] = dt * tx2
        * ( C1 * ( u_tile[k][j][i+1][4] * tmp1 )
            - C2
            * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] * tmp2
              + qs_tile[k][j][i+1][0] * tmp1 ) )
        - dt * tx1
        * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][1];
      au_tile[k][j][i][14] = dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][1] ) * tmp2 )
        - dt * tx1
        * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][2];
      au_tile[k][j][i][19] = dt * tx2
        * ( - C2 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][1] ) * tmp2 )
        - dt * tx1
        * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][3];
      au_tile[k][j][i][24] = dt * tx2
        * ( C1 * ( u_tile[k][j][i+1][1] * tmp1 ) )
        - dt * tx1 * c1345 * tmp1
        - dt * tx1 * dx5;
      //---------------------------------------------------------------------
      // form the second block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k][j+1][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      bu_tile[k][j][i][0] = - dt * ty1 * dy1;
      bu_tile[k][j][i][5] =   0.0;
      bu_tile[k][j][i][10] =  dt * ty2;
      bu_tile[k][j][i][15] =   0.0;
      bu_tile[k][j][i][20] =   0.0;

      bu_tile[k][j][i][1] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
        - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][1] );
      bu_tile[k][j][i][6] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
        - dt * ty1 * ( c34 * tmp1 )
        - dt * ty1 * dy2;
      bu_tile[k][j][i][11] =  dt * ty2 * ( u_tile[k][j+1][i][1] * tmp1 );
      bu_tile[k][j][i][16] = 0.0;
      bu_tile[k][j][i][21] = 0.0;

      bu_tile[k][j][i][2] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][2] * tmp1 ) * ( u_tile[k][j+1][i][2] * tmp1 )
            + C2 * ( qs_tile[k][j+1][i][0] * tmp1 ) )
        - dt * ty1 * ( - r43 * c34 * tmp2 * u_tile[k][j+1][i][2] );
      bu_tile[k][j][i][7] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][1] * tmp1 ) );
      bu_tile[k][j][i][12] =  dt * ty2 * ( ( 2.0 - C2 )
          * ( u_tile[k][j+1][i][2] * tmp1 ) )
        - dt * ty1 * ( r43 * c34 * tmp1 )
        - dt * ty1 * dy3;
      bu_tile[k][j][i][17] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][3] * tmp1 ) );
      bu_tile[k][j][i][22] =  dt * ty2 * C2;

      bu_tile[k][j][i][3] =  dt * ty2
        * ( - ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
        - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][3] );
      bu_tile[k][j][i][8] = 0.0;
      bu_tile[k][j][i][13] =  dt * ty2 * ( u_tile[k][j+1][i][3] * tmp1 );
      bu_tile[k][j][i][18] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
        - dt * ty1 * ( c34 * tmp1 )
        - dt * ty1 * dy4;
      bu_tile[k][j][i][23] = 0.0;

      bu_tile[k][j][i][4] =  dt * ty2
        * ( ( C2 * 2.0 * qs_tile[k][j+1][i][0]
            - C1 * u_tile[k][j+1][i][4] )
        * ( u_tile[k][j+1][i][2] * tmp2 ) )
        - dt * ty1
        * ( - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][1]*u_tile[k][j+1][i][1])
            - ( r43*c34 - c1345 )*tmp3*(u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2])
            - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][3]*u_tile[k][j+1][i][3])
            - c1345*tmp2*u_tile[k][j+1][i][4] );
      bu_tile[k][j][i][9] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
        - dt * ty1
        * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][1];
      bu_tile[k][j][i][14] =  dt * ty2
        * ( C1 * ( u_tile[k][j+1][i][4] * tmp1 )
            - C2 
            * ( qs_tile[k][j+1][i][0] * tmp1
              + u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2] * tmp2 ) )
        - dt * ty1
        * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][2];
      bu_tile[k][j][i][19] =  dt * ty2
        * ( - C2 * ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
        - dt * ty1 * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][3];
      bu_tile[k][j][i][24] =  dt * ty2
        * ( C1 * ( u_tile[k][j+1][i][2] * tmp1 ) )
        - dt * ty1 * c1345 * tmp1
        - dt * ty1 * dy5;
	
      //---------------------------------------------------------------------
      // form the third block sub-diagonal
      //---------------------------------------------------------------------
      tmp1 = rho_i_tile[k+1][j][i][0];
      tmp2 = tmp1 * tmp1;
      tmp3 = tmp1 * tmp2;

      cu_tile[k][j][i][0] = - dt * tz1 * dz1;
      cu_tile[k][j][i][5] =   0.0;
      cu_tile[k][j][i][10] =   0.0;
      cu_tile[k][j][i][15] = dt * tz2;
      cu_tile[k][j][i][20] =   0.0;

      cu_tile[k][j][i][1] = dt * tz2
        * ( - ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][1] );
      cu_tile[k][j][i][6] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * c34 * tmp1
        - dt * tz1 * dz2;
      cu_tile[k][j][i][11] = 0.0;
      cu_tile[k][j][i][16] = dt * tz2 * ( u_tile[k+1][j][i][1] * tmp1 );
      cu_tile[k][j][i][21] = 0.0;

      cu_tile[k][j][i][2] = dt * tz2
        * ( - ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][2] );
      cu_tile[k][j][i][7] = 0.0;
      cu_tile[k][j][i][12] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * ( c34 * tmp1 )
        - dt * tz1 * dz3;
      cu_tile[k][j][i][17] = dt * tz2 * ( u_tile[k+1][j][i][2] * tmp1 );
      cu_tile[k][j][i][22] = 0.0;

      cu_tile[k][j][i][3] = dt * tz2
        * ( - ( u_tile[k+1][j][i][3] * tmp1 ) * ( u_tile[k+1][j][i][3] * tmp1 )
            + C2 * ( qs_tile[k+1][j][i][0] * tmp1 ) )
        - dt * tz1 * ( - r43 * c34 * tmp2 * u_tile[k+1][j][i][3] );
      cu_tile[k][j][i][8] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][1] * tmp1 ) );
      cu_tile[k][j][i][13] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][2] * tmp1 ) );
      cu_tile[k][j][i][18] = dt * tz2 * ( 2.0 - C2 )
        * ( u_tile[k+1][j][i][3] * tmp1 )
        - dt * tz1 * ( r43 * c34 * tmp1 )
        - dt * tz1 * dz4;
      cu_tile[k][j][i][23] = dt * tz2 * C2;

      cu_tile[k][j][i][4] = dt * tz2
        * ( ( C2 * 2.0 * qs_tile[k+1][j][i][0]
            - C1 * u_tile[k+1][j][i][4] )
                 * ( u_tile[k+1][j][i][3] * tmp2 ) )
        - dt * tz1
        * ( - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][1]*u_tile[k+1][j][i][1])
            - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][2]*u_tile[k+1][j][i][2])
            - ( r43*c34 - c1345 )* tmp3 * (u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3])
            - c1345 * tmp2 * u_tile[k+1][j][i][4] );
      cu_tile[k][j][i][9] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][1];
      cu_tile[k][j][i][14] = dt * tz2
        * ( - C2 * ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
        - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][2];
      cu_tile[k][j][i][19] = dt * tz2
        * ( C1 * ( u_tile[k+1][j][i][4] * tmp1 )
            - C2
            * ( qs_tile[k+1][j][i][0] * tmp1
              + u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3] * tmp2 ) )
        - dt * tz1 * ( r43*c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][3];
      cu_tile[k][j][i][24] = dt * tz2
        * ( C1 * ( u_tile[k+1][j][i][3] * tmp1 ) )
        - dt * tz1 * c1345 * tmp1
        - dt * tz1 * dz5;
      }
    }
  } 
  
}


// void jacu_diagonal_a_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, void *scalar)
// {  
//   
//   //---------------------------------------------------------------------
//   // local variables
//   //---------------------------------------------------------------------
//   int i, j, k;
//   double r43;
//   double c1345;
//   double c34;
//   double tmp1, tmp2, tmp3;
//   
// //    int TILES_X = 1;
// //    int TILES_Y = 1;
// //    int TILES_Z = 3;
//   
//   int i_first, j_first, i_last, j_last, k_first, k_last;
//   int x, y, z;
//   
//   //int nm_tile = s3_tile->flat_size->values[3];
//   int nx_tile = s5_tile->flat_size.values[2];
//   int ny_tile = s5_tile->flat_size.values[1];
//   int nz_tile = s5_tile->flat_size.values[0]; 
//     
//   double (*du_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s1_tile);
//   double (*au_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s2_tile);
//   double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s3_tile);
//   double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s4_tile);
//   double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s5_tile);
//   
//   // FIXME: Trick to obtain the nd_idx of the tiles
//   // Get global tile nd_index first
//   //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1); // tile dimensions
//   Tuple nd_size = s5_tile->nd_tile_dimensions;
//   Tuple nd_idx = s5_tile->nd_rank;
//   //Tuple_init_zero(&nd_idx, 4); // this tile index
//   //Tuple_1d_to_nd_index(s5_tile->rank, &nd_size, &nd_idx);  
//   
//   x = nd_idx.values[2];
//   y = nd_idx.values[1];
//   z = nd_idx.values[0];
// 
//   if (x == 0) i_first = 3; // 2nd element
//   else i_first = 2;
//   if (y == 0) j_first = 3; // 2nd element
//   else j_first = 2;
//   if (z == 0) k_first = 3; // 2nd element
//   else k_first = 2;
//   if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; // orig. loop: i = iend - 1; i >= ist; i--
//   else i_last = nx_tile - 3;
//   if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; // orig. loop: j = jend - 1; j >= jst; j--
//   else j_last = ny_tile - 3;
//   if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; // orig. loop: k = nz-2; k > 0; k--
//   else k_last = nz_tile - 3;
//   
//   r43 = ( 4.0 / 3.0 );
//   c1345 = C1 * C3 * C4 * C5;
//   c34 = C3 * C4;
//     
//   for(k = k_last; k >= k_first; k--) {
//     for (j = j_last; j >= j_first; j--) {
//       for (i = i_last; i >= i_first; i--) {    
//       //---------------------------------------------------------------------
//       // form the block daigonal
//       //---------------------------------------------------------------------
//       tmp1 = rho_i_tile[k][j][i][0];
//       tmp2 = tmp1 * tmp1;
//       tmp3 = tmp1 * tmp2;
// 
//       du_tile[k][j][i][0] = 1.0 + dt * 2.0 * ( tx1 * dx1 + ty1 * dy1 + tz1 * dz1 );
//       du_tile[k][j][i][5] = 0.0;
//       du_tile[k][j][i][10] = 0.0;
//       du_tile[k][j][i][15] = 0.0;
//       du_tile[k][j][i][20] = 0.0;
// 
//       du_tile[k][j][i][1] =  dt * 2.0
//         * ( - tx1 * r43 - ty1 - tz1 )
//         * ( c34 * tmp2 * u_tile[k][j][i][1] );
//       du_tile[k][j][i][6] =  1.0
//         + dt * 2.0 * c34 * tmp1 
//         * (  tx1 * r43 + ty1 + tz1 )
//         + dt * 2.0 * ( tx1 * dx2 + ty1 * dy2 + tz1 * dz2 );
//       du_tile[k][j][i][11] = 0.0;
//       du_tile[k][j][i][16] = 0.0;
//       du_tile[k][j][i][21] = 0.0;
// 
//       du_tile[k][j][i][2] = dt * 2.0
//         * ( - tx1 - ty1 * r43 - tz1 )
//         * ( c34 * tmp2 * u_tile[k][j][i][2] );
//       du_tile[k][j][i][7] = 0.0;
//       du_tile[k][j][i][12] = 1.0
//         + dt * 2.0 * c34 * tmp1
//         * (  tx1 + ty1 * r43 + tz1 )
//         + dt * 2.0 * ( tx1 * dx3 + ty1 * dy3 + tz1 * dz3 );
//       du_tile[k][j][i][17] = 0.0;
//       du_tile[k][j][i][22] = 0.0;
// 
//       du_tile[k][j][i][3] = dt * 2.0
//         * ( - tx1 - ty1 - tz1 * r43 )
//         * ( c34 * tmp2 * u_tile[k][j][i][3] );
//       du_tile[k][j][i][8] = 0.0;
//       du_tile[k][j][i][13] = 0.0;
//       du_tile[k][j][i][18] = 1.0
//         + dt * 2.0 * c34 * tmp1
//         * (  tx1 + ty1 + tz1 * r43 )
//         + dt * 2.0 * ( tx1 * dx4 + ty1 * dy4 + tz1 * dz4 );
//       du_tile[k][j][i][23] = 0.0;
// 
//       du_tile[k][j][i][4] = -dt * 2.0
//         * ( ( ( tx1 * ( r43*c34 - c1345 )
//                 + ty1 * ( c34 - c1345 )
//                 + tz1 * ( c34 - c1345 ) ) * ( u_tile[k][j][i][1]*u_tile[k][j][i][1] )
//               + ( tx1 * ( c34 - c1345 )
//                 + ty1 * ( r43*c34 - c1345 )
//                 + tz1 * ( c34 - c1345 ) ) * ( u_tile[k][j][i][2]*u_tile[k][j][i][2] )
//               + ( tx1 * ( c34 - c1345 )
//                 + ty1 * ( c34 - c1345 )
//                 + tz1 * ( r43*c34 - c1345 ) ) * (u_tile[k][j][i][3]*u_tile[k][j][i][3])
//             ) * tmp3
//             + ( tx1 + ty1 + tz1 ) * c1345 * tmp2 * u_tile[k][j][i][4] );
// 
//       du_tile[k][j][i][9] = dt * 2.0
//         * ( tx1 * ( r43*c34 - c1345 )
//           + ty1 * (     c34 - c1345 )
//           + tz1 * (     c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][1];
//       du_tile[k][j][i][14] = dt * 2.0
//         * ( tx1 * ( c34 - c1345 )
//           + ty1 * ( r43*c34 -c1345 )
//           + tz1 * ( c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][2];
//       du_tile[k][j][i][19] = dt * 2.0
//         * ( tx1 * ( c34 - c1345 )
//           + ty1 * ( c34 - c1345 )
//           + tz1 * ( r43*c34 - c1345 ) ) * tmp2 * u_tile[k][j][i][3];
//       du_tile[k][j][i][24] = 1.0
//         + dt * 2.0 * ( tx1 + ty1 + tz1 ) * c1345 * tmp1
//         + dt * 2.0 * ( tx1 * dx5 + ty1 * dy5 + tz1 * dz5 );
// 	
//       //---------------------------------------------------------------------
//       // form the first block sub-diagonal
//       //---------------------------------------------------------------------
//       tmp1 = rho_i_tile[k][j][i+1][0];
//       tmp2 = tmp1 * tmp1;
//       tmp3 = tmp1 * tmp2;
// 
//       au_tile[k][j][i][0] = - dt * tx1 * dx1;
//       au_tile[k][j][i][5] =   dt * tx2;
//       au_tile[k][j][i][10] =   0.0;
//       au_tile[k][j][i][15] =   0.0;
//       au_tile[k][j][i][20] =   0.0;
// 
//       au_tile[k][j][i][1] =  dt * tx2
//         * ( - ( u_tile[k][j][i+1][1] * tmp1 ) * ( u_tile[k][j][i+1][1] * tmp1 )
//             + C2 * qs_tile[k][j][i+1][0] * tmp1 )
//         - dt * tx1 * ( - r43 * c34 * tmp2 * u_tile[k][j][i+1][1] );
//       au_tile[k][j][i][6] =  dt * tx2
//         * ( ( 2.0 - C2 ) * ( u_tile[k][j][i+1][1] * tmp1 ) )
//         - dt * tx1 * ( r43 * c34 * tmp1 )
//         - dt * tx1 * dx2;
//       au_tile[k][j][i][11] =  dt * tx2
//         * ( - C2 * ( u_tile[k][j][i+1][2] * tmp1 ) );
//       au_tile[k][j][i][16] =  dt * tx2
//         * ( - C2 * ( u_tile[k][j][i+1][3] * tmp1 ) );
//       au_tile[k][j][i][21] =  dt * tx2 * C2 ;
// 
//       au_tile[k][j][i][2] =  dt * tx2
//         * ( - ( u_tile[k][j][i+1][1] * u_tile[k][j][i+1][2] ) * tmp2 )
//         - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][2] );
//       au_tile[k][j][i][7] =  dt * tx2 * ( u_tile[k][j][i+1][2] * tmp1 );
//       au_tile[k][j][i][12] =  dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
//         - dt * tx1 * ( c34 * tmp1 )
//         - dt * tx1 * dx3;
//       au_tile[k][j][i][17] = 0.0;
//       au_tile[k][j][i][22] = 0.0;
// 
//       au_tile[k][j][i][3] = dt * tx2
//         * ( - ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][3] ) * tmp2 )
//         - dt * tx1 * ( - c34 * tmp2 * u_tile[k][j][i+1][3] );
//       au_tile[k][j][i][8] = dt * tx2 * ( u_tile[k][j][i+1][3] * tmp1 );
//       au_tile[k][j][i][13] = 0.0;
//       au_tile[k][j][i][18] = dt * tx2 * ( u_tile[k][j][i+1][1] * tmp1 )
//         - dt * tx1 * ( c34 * tmp1 )
//         - dt * tx1 * dx4;
//       au_tile[k][j][i][23] = 0.0;
// 
//       au_tile[k][j][i][4] = dt * tx2
//         * ( ( C2 * 2.0 * qs_tile[k][j][i+1][0]
//             - C1 * u_tile[k][j][i+1][4] )
//         * ( u_tile[k][j][i+1][1] * tmp2 ) )
//         - dt * tx1
//         * ( - ( r43*c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] )
//             - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][2] )
//             - (     c34 - c1345 ) * tmp3 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][3] )
//             - c1345 * tmp2 * u_tile[k][j][i+1][4] );
//       au_tile[k][j][i][9] = dt * tx2
//         * ( C1 * ( u_tile[k][j][i+1][4] * tmp1 )
//             - C2
//             * ( u_tile[k][j][i+1][1]*u_tile[k][j][i+1][1] * tmp2
//               + qs_tile[k][j][i+1][0] * tmp1 ) )
//         - dt * tx1
//         * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][1];
//       au_tile[k][j][i][14] = dt * tx2
//         * ( - C2 * ( u_tile[k][j][i+1][2]*u_tile[k][j][i+1][1] ) * tmp2 )
//         - dt * tx1
//         * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][2];
//       au_tile[k][j][i][19] = dt * tx2
//         * ( - C2 * ( u_tile[k][j][i+1][3]*u_tile[k][j][i+1][1] ) * tmp2 )
//         - dt * tx1
//         * (  c34 - c1345 ) * tmp2 * u_tile[k][j][i+1][3];
//       au_tile[k][j][i][24] = dt * tx2
//         * ( C1 * ( u_tile[k][j][i+1][1] * tmp1 ) )
//         - dt * tx1 * c1345 * tmp1
//         - dt * tx1 * dx5;
// 	
//       }
//     }
//   } 
//   
// }
// 
// 
// void jacu_b_c_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile, HTA* s4_tile, HTA* s5_tile, void *scalar)
// {  
//   //---------------------------------------------------------------------
//   // local variables
//   //---------------------------------------------------------------------
//   int i, j, k;
//   double r43;
//   double c1345;
//   double c34;
//   double tmp1, tmp2, tmp3;
//   
// //    int TILES_X = 1;
// //    int TILES_Y = 1;
// //    int TILES_Z = 3;
//   
//   int i_first, j_first, i_last, j_last, k_first, k_last;
//   int x, y, z;
//   
//   int nx_tile = s5_tile->flat_size.values[2];
//   int ny_tile = s5_tile->flat_size.values[1];
//   int nz_tile = s5_tile->flat_size.values[0]; 
//   
//   double (*bu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s1_tile);
//   double (*cu_tile)[ny_tile][nx_tile][25] = (double (*)[ny_tile][nx_tile][25])HTA_get_ptr_raw_data(s2_tile);
//   double (*rho_i_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s3_tile);
//   double (*qs_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s4_tile);
//   double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s5_tile);
//   
//   // FIXME: Trick to obtain the nd_idx of the tiles
//   // Get global tile nd_index first
//   //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1); // tile dimensions
//   Tuple nd_size = s5_tile->nd_tile_dimensions;
//   Tuple nd_idx = s5_tile->nd_rank;
//   //Tuple_init_zero(&nd_idx, 4); // this tile index
//   //Tuple_1d_to_nd_index(s5_tile->rank, &nd_size, &nd_idx);
//   
//   x = nd_idx.values[2];
//   y = nd_idx.values[1];
//   z = nd_idx.values[0];
//   
//   if (x == 0) i_first = 3; // 2nd element
//   else i_first = 2;
//   if (y == 0) j_first = 3; // 2nd element
//   else j_first = 2;
//   if (z == 0) k_first = 3; // 2nd element
//   else k_first = 2;
//   if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; // orig. loop: i = iend - 1; i >= ist; i--
//   else i_last = nx_tile - 3;
//   if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; // orig. loop: j = jend - 1; j >= jst; j--
//   else j_last = ny_tile - 3;
//   if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; // orig. loop: k = nz-2; k > 0; k--
//   else k_last = nz_tile - 3;
//   
//   r43 = ( 4.0 / 3.0 );
//   c1345 = C1 * C3 * C4 * C5;
//   c34 = C3 * C4;
//   
//   for(k = k_last; k >= k_first; k--) {    
//     for (j = j_last; j >= j_first; j--) {
//       for (i = i_last; i >= i_first; i--) {    
//       //---------------------------------------------------------------------
//       // form the second block sub-diagonal
//       //---------------------------------------------------------------------
//       tmp1 = rho_i_tile[k][j+1][i][0];
//       tmp2 = tmp1 * tmp1;
//       tmp3 = tmp1 * tmp2;
// 
//       bu_tile[k][j][i][0] = - dt * ty1 * dy1;
//       bu_tile[k][j][i][5] =   0.0;
//       bu_tile[k][j][i][10] =  dt * ty2;
//       bu_tile[k][j][i][15] =   0.0;
//       bu_tile[k][j][i][20] =   0.0;
// 
//       bu_tile[k][j][i][1] =  dt * ty2
//         * ( - ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
//         - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][1] );
//       bu_tile[k][j][i][6] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
//         - dt * ty1 * ( c34 * tmp1 )
//         - dt * ty1 * dy2;
//       bu_tile[k][j][i][11] =  dt * ty2 * ( u_tile[k][j+1][i][1] * tmp1 );
//       bu_tile[k][j][i][16] = 0.0;
//       bu_tile[k][j][i][21] = 0.0;
// 
//       bu_tile[k][j][i][2] =  dt * ty2
//         * ( - ( u_tile[k][j+1][i][2] * tmp1 ) * ( u_tile[k][j+1][i][2] * tmp1 )
//             + C2 * ( qs_tile[k][j+1][i][0] * tmp1 ) )
//         - dt * ty1 * ( - r43 * c34 * tmp2 * u_tile[k][j+1][i][2] );
//       bu_tile[k][j][i][7] =  dt * ty2
//         * ( - C2 * ( u_tile[k][j+1][i][1] * tmp1 ) );
//       bu_tile[k][j][i][12] =  dt * ty2 * ( ( 2.0 - C2 )
//           * ( u_tile[k][j+1][i][2] * tmp1 ) )
//         - dt * ty1 * ( r43 * c34 * tmp1 )
//         - dt * ty1 * dy3;
//       bu_tile[k][j][i][17] =  dt * ty2
//         * ( - C2 * ( u_tile[k][j+1][i][3] * tmp1 ) );
//       bu_tile[k][j][i][22] =  dt * ty2 * C2;
// 
//       bu_tile[k][j][i][3] =  dt * ty2
//         * ( - ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
//         - dt * ty1 * ( - c34 * tmp2 * u_tile[k][j+1][i][3] );
//       bu_tile[k][j][i][8] = 0.0;
//       bu_tile[k][j][i][13] =  dt * ty2 * ( u_tile[k][j+1][i][3] * tmp1 );
//       bu_tile[k][j][i][18] =  dt * ty2 * ( u_tile[k][j+1][i][2] * tmp1 )
//         - dt * ty1 * ( c34 * tmp1 )
//         - dt * ty1 * dy4;
//       bu_tile[k][j][i][23] = 0.0;
// 
//       bu_tile[k][j][i][4] =  dt * ty2
//         * ( ( C2 * 2.0 * qs_tile[k][j+1][i][0]
//             - C1 * u_tile[k][j+1][i][4] )
//         * ( u_tile[k][j+1][i][2] * tmp2 ) )
//         - dt * ty1
//         * ( - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][1]*u_tile[k][j+1][i][1])
//             - ( r43*c34 - c1345 )*tmp3*(u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2])
//             - (     c34 - c1345 )*tmp3*(u_tile[k][j+1][i][3]*u_tile[k][j+1][i][3])
//             - c1345*tmp2*u_tile[k][j+1][i][4] );
//       bu_tile[k][j][i][9] =  dt * ty2
//         * ( - C2 * ( u_tile[k][j+1][i][1]*u_tile[k][j+1][i][2] ) * tmp2 )
//         - dt * ty1
//         * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][1];
//       bu_tile[k][j][i][14] =  dt * ty2
//         * ( C1 * ( u_tile[k][j+1][i][4] * tmp1 )
//             - C2 
//             * ( qs_tile[k][j+1][i][0] * tmp1
//               + u_tile[k][j+1][i][2]*u_tile[k][j+1][i][2] * tmp2 ) )
//         - dt * ty1
//         * ( r43*c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][2];
//       bu_tile[k][j][i][19] =  dt * ty2
//         * ( - C2 * ( u_tile[k][j+1][i][2]*u_tile[k][j+1][i][3] ) * tmp2 )
//         - dt * ty1 * ( c34 - c1345 ) * tmp2 * u_tile[k][j+1][i][3];
//       bu_tile[k][j][i][24] =  dt * ty2
//         * ( C1 * ( u_tile[k][j+1][i][2] * tmp1 ) )
//         - dt * ty1 * c1345 * tmp1
//         - dt * ty1 * dy5;
// 	
//       //---------------------------------------------------------------------
//       // form the third block sub-diagonal
//       //---------------------------------------------------------------------
//       tmp1 = rho_i_tile[k+1][j][i][0];
//       tmp2 = tmp1 * tmp1;
//       tmp3 = tmp1 * tmp2;
// 
//       cu_tile[k][j][i][0] = - dt * tz1 * dz1;
//       cu_tile[k][j][i][5] =   0.0;
//       cu_tile[k][j][i][10] =   0.0;
//       cu_tile[k][j][i][15] = dt * tz2;
//       cu_tile[k][j][i][20] =   0.0;
// 
//       cu_tile[k][j][i][1] = dt * tz2
//         * ( - ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
//         - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][1] );
//       cu_tile[k][j][i][6] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
//         - dt * tz1 * c34 * tmp1
//         - dt * tz1 * dz2;
//       cu_tile[k][j][i][11] = 0.0;
//       cu_tile[k][j][i][16] = dt * tz2 * ( u_tile[k+1][j][i][1] * tmp1 );
//       cu_tile[k][j][i][21] = 0.0;
// 
//       cu_tile[k][j][i][2] = dt * tz2
//         * ( - ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
//         - dt * tz1 * ( - c34 * tmp2 * u_tile[k+1][j][i][2] );
//       cu_tile[k][j][i][7] = 0.0;
//       cu_tile[k][j][i][12] = dt * tz2 * ( u_tile[k+1][j][i][3] * tmp1 )
//         - dt * tz1 * ( c34 * tmp1 )
//         - dt * tz1 * dz3;
//       cu_tile[k][j][i][17] = dt * tz2 * ( u_tile[k+1][j][i][2] * tmp1 );
//       cu_tile[k][j][i][22] = 0.0;
// 
//       cu_tile[k][j][i][3] = dt * tz2
//         * ( - ( u_tile[k+1][j][i][3] * tmp1 ) * ( u_tile[k+1][j][i][3] * tmp1 )
//             + C2 * ( qs_tile[k+1][j][i][0] * tmp1 ) )
//         - dt * tz1 * ( - r43 * c34 * tmp2 * u_tile[k+1][j][i][3] );
//       cu_tile[k][j][i][8] = dt * tz2
//         * ( - C2 * ( u_tile[k+1][j][i][1] * tmp1 ) );
//       cu_tile[k][j][i][13] = dt * tz2
//         * ( - C2 * ( u_tile[k+1][j][i][2] * tmp1 ) );
//       cu_tile[k][j][i][18] = dt * tz2 * ( 2.0 - C2 )
//         * ( u_tile[k+1][j][i][3] * tmp1 )
//         - dt * tz1 * ( r43 * c34 * tmp1 )
//         - dt * tz1 * dz4;
//       cu_tile[k][j][i][23] = dt * tz2 * C2;
// 
//       cu_tile[k][j][i][4] = dt * tz2
//         * ( ( C2 * 2.0 * qs_tile[k+1][j][i][0]
//             - C1 * u_tile[k+1][j][i][4] )
//                  * ( u_tile[k+1][j][i][3] * tmp2 ) )
//         - dt * tz1
//         * ( - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][1]*u_tile[k+1][j][i][1])
//             - ( c34 - c1345 ) * tmp3 * (u_tile[k+1][j][i][2]*u_tile[k+1][j][i][2])
//             - ( r43*c34 - c1345 )* tmp3 * (u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3])
//             - c1345 * tmp2 * u_tile[k+1][j][i][4] );
//       cu_tile[k][j][i][9] = dt * tz2
//         * ( - C2 * ( u_tile[k+1][j][i][1]*u_tile[k+1][j][i][3] ) * tmp2 )
//         - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][1];
//       cu_tile[k][j][i][14] = dt * tz2
//         * ( - C2 * ( u_tile[k+1][j][i][2]*u_tile[k+1][j][i][3] ) * tmp2 )
//         - dt * tz1 * ( c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][2];
//       cu_tile[k][j][i][19] = dt * tz2
//         * ( C1 * ( u_tile[k+1][j][i][4] * tmp1 )
//             - C2
//             * ( qs_tile[k+1][j][i][0] * tmp1
//               + u_tile[k+1][j][i][3]*u_tile[k+1][j][i][3] * tmp2 ) )
//         - dt * tz1 * ( r43*c34 - c1345 ) * tmp2 * u_tile[k+1][j][i][3];
//       cu_tile[k][j][i][24] = dt * tz2
//         * ( C1 * ( u_tile[k+1][j][i][3] * tmp1 ) )
//         - dt * tz1 * c1345 * tmp1
//         - dt * tz1 * dz5;
// 	
//       }
//     }
//   } 
// 
// }

