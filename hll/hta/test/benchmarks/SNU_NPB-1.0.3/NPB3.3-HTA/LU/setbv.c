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
// set the boundary values of dependent variables
//---------------------------------------------------------------------
void setbv_HTA()
{ 
  double zero = 0.0;
  
  HTA_map_h1s1(HTA_LEAF_LEVEL(u_HTA), H1S1_INIT, u_HTA, &zero);
  
  //HTA_tile_to_hta(HTA_LEAF_LEVEL(u_HTA), setbv_op_HTA, u_HTA, u_HTA, u_HTA);
  HTA_map_h1(HTA_LEAF_LEVEL(u_HTA), setbv_op_HTA, u_HTA);
  
}

//void setbv_op_HTA(HTA* dest_tile, HTA* u_tile, HTA* u_complete)
void setbv_op_HTA(HTA* u_tile)
{
  int i, j, k, m;
  int i_first, j_first, k_first, i_last, j_last, k_last;
  int i_offset, j_offset, k_offset, x, y, z;
  double temp1[5], temp2[5];
  
  int nm_tile = u_tile->flat_size.values[3]; // Always 5
  int nx_tile = u_tile->flat_size.values[2];
  int ny_tile = u_tile->flat_size.values[1];
  int nz_tile = u_tile->flat_size.values[0];
  
  // Get global tile nd_index first
  //Tuple* nd_size = u_complete->tiling; // tile dimensions
  Tuple nd_size = u_tile->nd_tile_dimensions;
  Tuple nd_idx = u_tile->nd_rank;
  //Tuple_init_zero(&nd_idx, 4); // this tile index
  //Tuple_1d_to_nd_index(u_tile->rank, nd_size, &nd_idx);
  
  double (*u_temp)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(u_tile);
  
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
  
  i_offset = u_tile->nd_element_offset.values[2] - (4 * x) - 2; 
  j_offset = u_tile->nd_element_offset.values[1] - (4 * y) - 2;
  k_offset = u_tile->nd_element_offset.values[0] - (4 * z) - 2;
  
  //printf("Tile (%d,%d,%d) -- Offset (z : %d(%d), y : %d(%d), x : %d(%d))\n", z, y, x, k_offset, new_off_k, j_offset, new_off_j, i_offset, new_off_i);
  //Tuple_print(&u_tile->nd_element_offset);
  
  //---------------------------------------------------------------------
  // set the dependent variable values along the top and bottom faces
  //---------------------------------------------------------------------
  if(z == 0) { // bottom
   for (j = j_first; j < j_last; j++) {
    for (i = i_first; i < i_last; i++) {
      exact(i_offset + i, j_offset + j, 0, temp1 );
      for (m = 0; m < 5; m++) {
        u_temp[k_first][j][i][m] = temp1[m];
      }
    }
   }
  }
  if(z == nd_size.values[0]-1) { // top
   for (j = j_first; j < j_last; j++) {
    for (i = i_first; i < i_last; i++) {
      exact( i_offset + i, j_offset + j, k_offset + k_last-1, temp2 );
      for (m = 0; m < 5; m++) {
        u_temp[k_last-1][j][i][m] = temp2[m];
      }
    }
   }
  }
  //---------------------------------------------------------------------
  // set the dependent variable values along north and south faces
  //---------------------------------------------------------------------
  if(y == 0) { // north
   for (k = k_first; k < k_last; k++) {
    for (i = i_first; i < i_last; i++) {
      exact( i_offset + i, 0, k_offset + k, temp1 );
      for (m = 0; m < 5; m++) {
        u_temp[k][j_first][i][m] = temp1[m];
      }
    }
   }
  }
  if(y == nd_size.values[1]-1) { // south
   for (k = k_first; k < k_last; k++) {
    for (i = i_first; i < i_last; i++) {
      exact( i_offset + i, j_offset + j_last-1, k_offset + k, temp2 );
      for (m = 0; m < 5; m++) {
        u_temp[k][j_last-1][i][m] = temp2[m];
      }
    }
   }
  }
  //---------------------------------------------------------------------
  // set the dependent variable values along east and west faces
  //---------------------------------------------------------------------
  if(x == 0) { // east
     for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
        exact( 0, j_offset + j, k_offset + k, temp1 );
        for (m = 0; m < 5; m++) {
          u_temp[k][j][i_first][m] = temp1[m];
        }
      }
    }
  }
  if(x == nd_size.values[2]-1) { // west
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
        exact( i_offset + i_last-1, j_offset + j, k_offset + k, temp2 );
        for (m = 0; m < 5; m++) {
          u_temp[k][j][i_last-1][m] = temp2[m];
        }
      }
    }
  }
  
}  