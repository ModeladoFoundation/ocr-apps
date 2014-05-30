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
// set the initial values of independent variables based on tri-linear
// interpolation of boundary values in the computational space.
//
//---------------------------------------------------------------------

void setiv_HTA()
{ 
  //HTA_tile_to_hta(HTA_LEAF_LEVEL(u_HTA), setiv_op_HTA, u_HTA, u_HTA, u_HTA);
  HTA_map_h1(HTA_LEAF_LEVEL(u_HTA), setiv_op_HTA, u_HTA);
}

//void setiv_op_HTA(HTA* dest_HTA, HTA* u_tile, HTA* u_complete)
void setiv_op_HTA(HTA* u_tile)
{
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  int i_first, j_first, k_first, i_last, j_last, k_last;
  int i_offset, j_offset, k_offset, x, y, z;
  int i_real, j_real, k_real;
  
  double xi, eta, zeta;
  double pxi, peta, pzeta;
  double ue_1jk[5], ue_nx0jk[5], ue_i1k[5];
  double ue_iny0k[5], ue_ij1[5], ue_ijnz[5];

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
  if (x == 0) i_first = 3; // Bottom: 2nd element
  else i_first = 0;
  if (y == 0) j_first = 3; // North: 2nd element
  else j_first = 0;
  if (z == 0) k_first = 3; // East: 2nd element
  else k_first = 0;
  if (x == nd_size.values[2] - 1) i_last = nx_tile - 3; // Top: one before last element
  else i_last = nx_tile;
  if (y == nd_size.values[1] - 1) j_last = ny_tile - 3; // South: one before last element
  else j_last = ny_tile;
  if (z == nd_size.values[0] - 1) k_last = nz_tile - 3; // West: one before last element
  else k_last = nz_tile;
  
  //FIXME: assuming regular tiles: problem with irregular tiles!!!
  //i_offset = x * (nx_tile - 4) - 2;
  //j_offset = y * (ny_tile - 4) - 2;
  //k_offset = z * (nz_tile - 4) - 2; 
  
  i_offset = u_tile->nd_element_offset.values[2] - (4 * x) - 2; 
  j_offset = u_tile->nd_element_offset.values[1] - (4 * y) - 2;
  k_offset = u_tile->nd_element_offset.values[0] - (4 * z) - 2;
  
  for (k = k_first; k < k_last; k++) {
    k_real = k + k_offset;
    zeta = (double)k_real / (nz-1);
    for (j = j_first; j < j_last; j++) {
      j_real = j + j_offset;
      eta = (double)j_real / (ny0-1);
      for (i = i_first; i < i_last; i++) {
	i_real = i + i_offset;
        xi = (double)i_real / (nx0-1);
        exact(0, j_real, k_real, ue_1jk);
        exact(nx0-1, j_real, k_real, ue_nx0jk);
        exact(i_real, 0, k_real, ue_i1k);
        exact(i_real, ny0-1, k_real, ue_iny0k);
        exact(i_real, j_real, 0, ue_ij1);
        exact(i_real, j_real, nz-1, ue_ijnz);

        for (m = 0; m < 5; m++) {
          pxi =   ( 1.0 - xi ) * ue_1jk[m]
                        + xi   * ue_nx0jk[m];
          peta =  ( 1.0 - eta ) * ue_i1k[m]
                        + eta   * ue_iny0k[m];
          pzeta = ( 1.0 - zeta ) * ue_ij1[m]
                        + zeta   * ue_ijnz[m];

          u_temp[k][j][i][m] = pxi + peta + pzeta
            - pxi * peta - peta * pzeta - pzeta * pxi
            + pxi * peta * pzeta;
        }
      }
    }
  }
}
