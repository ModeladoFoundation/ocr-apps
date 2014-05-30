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
#include "applu.incl"

void pintgr_HTA()
{
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  double frc1 = 0.0, frc2 = 0.0, frc3 = 0.0;
  double zero = 0.0;
  HTA* phi1_HTA;
  HTA* phi2_HTA;
  
  Tuple t0 = Tuple_create(4, PROC_Z, PROC_Y, PROC_X, 1);
  Tuple fs0 = Tuple_create(4, ISIZ3+(4*PROC_Z), ISIZ2+(4*PROC_Y), ISIZ1+(4*PROC_X), 1);
  Dist dist0;
  Dist_init(&dist0, 0);
  phi1_HTA = HTA_create(4, 2, &fs0, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);
  phi2_HTA = HTA_create(4, 2, &fs0, 0, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, t0);
  
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi1_HTA), H1S1_INIT, phi1_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi2_HTA), H1S1_INIT, phi2_HTA, &zero);
  
  check_bound = -2;
  sync_boundary(u_HTA);
  
  HTA_map_h3(HTA_LEAF_LEVEL(u_HTA), pintgr_init1_HTA, phi1_HTA, phi2_HTA, u_HTA); 
  HTA* r1 = HTA_partial_reduce(REDUCE_SUM, phi1_HTA, 0, &zero);
  HTA* r2 = HTA_partial_reduce(REDUCE_SUM, phi2_HTA, 0, &zero); 
  HTA_reduce_h2(REDUCE_SUM, frc1_calc, &frc1, r1, r2);
  
  frc1 = dxi * deta * frc1;
  
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi1_HTA), H1S1_INIT, phi1_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi2_HTA), H1S1_INIT, phi2_HTA, &zero);
  
  HTA_map_h3(HTA_LEAF_LEVEL(u_HTA), pintgr_init2_HTA, phi1_HTA, phi2_HTA, u_HTA);
  HTA* r3 = HTA_partial_reduce(REDUCE_SUM, phi1_HTA, 1, &zero);
  HTA* r4 = HTA_partial_reduce(REDUCE_SUM, phi2_HTA, 1, &zero); 
  HTA_reduce_h2(REDUCE_SUM, frc2_calc, &frc2, r3, r4);
  
  frc2 = dxi * dzeta * frc2;
  
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi1_HTA), H1S1_INIT, phi1_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(phi2_HTA), H1S1_INIT, phi2_HTA, &zero);
  
  HTA_map_h3(HTA_LEAF_LEVEL(u_HTA), pintgr_init3_HTA, phi1_HTA, phi2_HTA, u_HTA);
  HTA* r5 = HTA_partial_reduce(REDUCE_SUM, phi1_HTA, 2, &zero);
  HTA* r6 = HTA_partial_reduce(REDUCE_SUM, phi2_HTA, 2, &zero); 
  HTA_reduce_h2(REDUCE_SUM, frc3_calc, &frc3, r5, r6);
  
  frc3 = deta * dzeta * frc3;
  
  frc = 0.25 * ( frc1 + frc2 + frc3 );
  printf("\n\n     surface integral = %12.5E\n\n\n", frc);

}

void frc1_calc(HTA* s1_tile, HTA* s2_tile, void* s)
{
    int i, j;
    int i_first, j_first, i_last, j_last;
    int x, y;
    double sum = 0;

//    int TILES_X = 1;
//    int TILES_Y = 1;
    
    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    
    double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
    double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);

    // FIXME: Trick to obtain the nd_idx of the tiles
    // Get global tile nd_index first
    //Tuple nd_size = Tuple_create(4, 1, TILES_Y, TILES_X, 1);
    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, &nd_size, &nd_idx);
    
    x = nd_idx.values[2];
    y = nd_idx.values[1];
  
    if (x == 0) i_first = 3; // 2nd element
    else i_first = 2;
    if (y == 0) j_first = 3; // 2nd element
    else j_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; 
    else i_last = nx_tile - 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 5; 
    else j_last = ny_tile - 2;
    
    for(j = j_first; j < j_last; j++) {
      for(i = i_first; i < i_last; i++) {
        sum = sum + (  phi1_tile[0][j][i][0]
                     + phi1_tile[0][j][i+1][0]
                     + phi1_tile[0][j+1][i][0]
                     + phi1_tile[0][j+1][i+1][0]
                     + phi2_tile[0][j][i][0]
                     + phi2_tile[0][j][i+1][0]
                     + phi2_tile[0][j+1][i][0]
                     + phi2_tile[0][j+1][i+1][0]);
      }
    }
    
    *((double*)s) = sum;
}

void frc2_calc(HTA* s1_tile, HTA* s2_tile, void* s)
{
    int i, k;
    int i_first, i_last, k_first, k_last;
    int x, z;
    double sum = 0;
    
//    int TILES_X = 1;
//    int TILES_Z = 3;

    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];
    
    double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
    double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);

    // FIXME: Trick to obtain the nd_idx of the tiles
    // Get global tile nd_index first
    //Tuple nd_size = Tuple_create(4, TILES_Z, 1, TILES_X, 1);
    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, &nd_size, &nd_idx);
    
    x = nd_idx.values[2];
    z = nd_idx.values[0];
    
    if (x == 0) i_first = 3; // 2nd element
    else i_first = 2; 
    if (z == 0) k_first = 4; // ki1
    else k_first = 2; 
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 4; 
    else i_last = nx_tile - 2; 
    if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; 
    else k_last = nz_tile - 2; 
    
    for(k = k_first; k < k_last; k++) {
      for(i = i_first; i < i_last; i++) {
      sum = sum + (  phi1_tile[k][0][i][0]
                     + phi1_tile[k][0][i+1][0]
                     + phi1_tile[k+1][0][i][0]
                     + phi1_tile[k+1][0][i+1][0]
                     + phi2_tile[k][0][i][0]
                     + phi2_tile[k][0][i+1][0]
                     + phi2_tile[k+1][0][i][0]
                     + phi2_tile[k+1][0][i+1][0] );
      }
    }
    
    *((double*)s) = sum;
}

void frc3_calc(HTA* s1_tile, HTA* s2_tile, void* s)
{
    int j, k;
    int j_first, j_last, k_first, k_last;
    int y, z;
    double sum = 0;
    
//    int TILES_Y = 1;
//    int TILES_Z = 3;

    int nx_tile = s1_tile->flat_size.values[2];
    int ny_tile = s1_tile->flat_size.values[1];
    int nz_tile = s1_tile->flat_size.values[0];
    
    double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
    double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);

    // FIXME: Trick to obtain the nd_idx of the tiles
    // Get global tile nd_index first
    //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, 1, 1);
    Tuple nd_size = s1_tile->nd_tile_dimensions;
    Tuple nd_idx = s1_tile->nd_rank;
    //Tuple_init_zero(&nd_idx, 4); // this tile index
    //Tuple_1d_to_nd_index(s1_tile->rank, &nd_size, &nd_idx);
    
    y = nd_idx.values[1];
    z = nd_idx.values[0];
    
    if (y == 0) j_first = 3; 
    else j_first = 2;
    if (z == 0) k_first = 4; 
    else k_first = 2; 
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 5; 
    else j_last = ny_tile-2; 
    if (z == nd_size.values[0] - 1) k_last = nz_tile - 4; 
    else k_last = nz_tile-2; 
    
    for(k = k_first; k < k_last; k++) {
      for(j = j_first; j < j_last; j++) {
      sum = sum + (  phi1_tile[k][j][0][0]
                     + phi1_tile[k][j+1][0][0]
                     + phi1_tile[k+1][j][0][0]
                     + phi1_tile[k+1][j+1][0][0]
                     + phi2_tile[k][j][0][0]
                     + phi2_tile[k][j+1][0][0]
                     + phi2_tile[k+1][j][0][0]
                     + phi2_tile[k+1][j+1][0][0] );
      }
    }
    
    *((double*)s) = sum;
}

void pintgr_init1_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile)
{
  int i, j, k;
  int i_first, j_first, i_last, j_last;
  int x, y, z;
  
//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;
  
  int nx_tile = s3_tile->flat_size.values[2];
  int ny_tile = s3_tile->flat_size.values[1];
  int nz_tile = s3_tile->flat_size.values[0];  
  
  double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
  double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s3_tile);
  
  // FIXME: Trick to obtain the nd_idx of the tiles
  // Get global tile nd_index first
  //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1);
  Tuple nd_size = s3_tile->nd_tile_dimensions;
  Tuple nd_idx = s3_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(s3_tile->rank, &nd_size, &nd_idx);
  
  x = nd_idx.values[2];
  y = nd_idx.values[1];
  z = nd_idx.values[0];
  
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 0; // For overlapping 
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 0; // For overlapping
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; 
  else i_last = nx_tile; // For overlapping
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; 
  else j_last = ny_tile; // For overlapping
  
  if(z == 0) {
    k = 4; // k = ki1
    for (j = j_first; j < j_last; j++) {
      for (i = i_first; i < i_last; i++) {
	phi1_tile[0][j][i][0] = C2*(  u_tile[k][j][i][4]
          - 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
                    + u_tile[k][j][i][2] * u_tile[k][j][i][2]
                    + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
                   / u_tile[k][j][i][0] );
      }
    }
  }

  if(z == nd_size.values[0]-1) {
   k = nz_tile - 4; //k = ki2 - 1;
    for (j = j_first; j < j_last; j++) {
      for (i = i_first; i < i_last; i++) {
	phi2_tile[0][j][i][0] = C2*(  u_tile[k][j][i][4]
	    - 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
		      + u_tile[k][j][i][2] * u_tile[k][j][i][2]
		      + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
		    / u_tile[k][j][i][0] );
      }
    }
  }
  
}

void pintgr_init2_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile)
{
  int i, j, k;
  int i_first, i_last, k_first, k_last;
  int x, y, z;
  
//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;
  
  int nx_tile = s3_tile->flat_size.values[2];
  int ny_tile = s3_tile->flat_size.values[1];
  int nz_tile = s3_tile->flat_size.values[0]; 
  
  double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
  double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s3_tile);
  
  // FIXME: Trick to obtain the nd_idx of the tiles
  // Get global tile nd_index first
  //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1);
  Tuple nd_size = s3_tile->nd_tile_dimensions;
  Tuple nd_idx = s3_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(s3_tile->rank, &nd_size, &nd_idx);
  
  x = nd_idx.values[2];
  y = nd_idx.values[1];
  z = nd_idx.values[0];
  
  if (x == 0) i_first = 3; // 2nd element
  else i_first = 0; // For overlapping 
  if (z == 0) k_first = 4; // ki1
  else k_first = 0; // For overlapping
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; 
  else i_last = nx_tile; // For overlapping
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; 
  else k_last = nz_tile; // For overlapping
  
  if(y == 0) {
    j = 3;
    for (k = k_first; k < k_last; k++) {
      for (i = i_first; i < i_last; i++) {
	phi1_tile[k][0][i][0] = C2*(  u_tile[k][j][i][4]
          - 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
                    + u_tile[k][j][i][2] * u_tile[k][j][i][2]
                    + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
                   / u_tile[k][j][i][0] );
      }
    }
  }

  if(y == nd_size.values[1]-1) {
    j = ny_tile - 4; 
    for (k = k_first; k < k_last; k++) {
      for (i = i_first; i < i_last; i++) {
	phi2_tile[k][0][i][0] = C2*(  u_tile[k][j-1][i][4]
          - 0.50 * (  u_tile[k][j-1][i][1] * u_tile[k][j-1][i][1]
                    + u_tile[k][j-1][i][2] * u_tile[k][j-1][i][2]
                    + u_tile[k][j-1][i][3] * u_tile[k][j-1][i][3] )
                   / u_tile[k][j-1][i][0] );
      }
    }
  }
  
}

void pintgr_init3_HTA(HTA* s1_tile, HTA* s2_tile, HTA* s3_tile)
{
  int i, j, k;
  int j_first, j_last, k_first, k_last;
  int x, y, z;
  
//    int TILES_X = 1;
//    int TILES_Y = 1;
//    int TILES_Z = 3;
  
  int nx_tile = s3_tile->flat_size.values[2];
  int ny_tile = s3_tile->flat_size.values[1];
  int nz_tile = s3_tile->flat_size.values[0];  
  
  double (*phi1_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s1_tile);
  double (*phi2_tile)[ny_tile][nx_tile][1] = (double (*)[ny_tile][nx_tile][1])HTA_get_ptr_raw_data(s2_tile);
  double (*u_tile)[ny_tile][nx_tile][5] = (double (*)[ny_tile][nx_tile][5])HTA_get_ptr_raw_data(s3_tile);
  
  // FIXME: Trick to obtain the nd_idx of the tiles
  // Get global tile nd_index first
  //Tuple nd_size = Tuple_create(4, TILES_Z, TILES_Y, TILES_X, 1);
  Tuple nd_size = s3_tile->nd_tile_dimensions;
  Tuple nd_idx = s3_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(s3_tile->rank, &nd_size, &nd_idx);
  
  x = nd_idx.values[2];
  y = nd_idx.values[1];
  z = nd_idx.values[0];
  
  if (y == 0) j_first = 3; // 2nd element
  else j_first = 0; // For overlapping 
  if (z == 0) k_first = 4; // ki1
  else k_first = 0; // For overlapping
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 4; 
  else j_last = ny_tile; // For overlapping
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; 
  else k_last = nz_tile; // For overlapping
  
  if(x == 0) {
    i = 3;
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	phi1_tile[k][j][0][0] = C2*(  u_tile[k][j][i][4]
	    - 0.50 * (  u_tile[k][j][i][1] * u_tile[k][j][i][1]
		      + u_tile[k][j][i][2] * u_tile[k][j][i][2]
		      + u_tile[k][j][i][3] * u_tile[k][j][i][3] )
		    / u_tile[k][j][i][0] );
      }
    }
  }

  if(x == nd_size.values[2]-1) {
    i = nx_tile - 3; 
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	phi2_tile[k][j][0][0] = C2*(  u_tile[k][j][i-1][4]
	    - 0.50 * (  u_tile[k][j][i-1][1] * u_tile[k][j][i-1][1]
		      + u_tile[k][j][i-1][2] * u_tile[k][j][i-1][2]
		      + u_tile[k][j][i-1][3] * u_tile[k][j][i-1][3] )
		    / u_tile[k][j][i-1][0] );
      }
    }
  }
  
}

