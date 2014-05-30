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
#include <math.h>
#include "applu.incl"


//---------------------------------------------------------------------
// 
// compute the solution error
// 
//---------------------------------------------------------------------
void error_HTA(double error[5])
{
    //---------------------------------------------------------------------
    // local variables
    //---------------------------------------------------------------------
    int m;
    int initval = 0;
    double error_temp[1][1][1][5];
  
    for (m = 0; m < 5; m++) {
      error_temp[0][0][0][m] = 0.0;
    }
    
    // FIXME: Be careful with this operation. Order of the reductions may influence the final sum
    // TODO: Find another easy way to do this
    //HTA_tile_to_hta(HTA_LEAF_LEVEL(u_HTA), error_sq_HTA, err_HTA, u_HTA, u_HTA);
    HTA_map_h1s1(HTA_LEAF_LEVEL(err_HTA), H1S1_INIT, err_HTA, &initval);
    HTA_map_h2(HTA_LEAF_LEVEL(u_HTA), error_sq_HTA, err_HTA, u_HTA);
    HTA* r1 = HTA_partial_reduce(REDUCE_SUM, err_HTA, 2, &initval);
    HTA* r2 = HTA_partial_reduce(REDUCE_SUM, r1, 1, &initval);
    HTA* r3 = HTA_partial_reduce(REDUCE_SUM, r2, 0, &initval);
    HTA_to_array(r3, error_temp);
    
    for (m = 0; m < 5; m++) {
      error[m] = sqrt ( error_temp[0][0][0][m] / ( (nx0-2)*(ny0-2)*(nz0-2) ));
    }
    
      /*
  printf(" \n RMS-norm of error in soln. to first pde  = %12.5E\n"
         " RMS-norm of error in soln. to second pde = %12.5E\n"
         " RMS-norm of error in soln. to third pde  = %12.5E\n"
         " RMS-norm of error in soln. to fourth pde = %12.5E\n"
         " RMS-norm of error in soln. to fifth pde  = %12.5E\n",
         error[0], error[1], error[2], error[3], error[4]);
  */

}

//void error_sq_HTA(HTA* d_tile, HTA* s1_tile, HTA* s2)
void error_sq_HTA(HTA* d_tile, HTA* s1_tile)
{
    double u000ijk[5], tmp;
    int i, j, k, m;
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
  
    double (*u_tile)[ny_tile][nx_tile][nm_tile] = (double (*)[ny_tile][nx_tile][nm_tile])HTA_get_ptr_raw_data(s1_tile);
    double (*error_local)[1][1][5] = (double (*)[1][1][5])HTA_get_ptr_raw_data(d_tile);

    for (m = 0; m < 5; m++) {
      error_local[0][0][0][m] = 0.0;
    }
    
    x = nd_idx.values[2];
    y = nd_idx.values[1];
    z = nd_idx.values[0];

    //FIXME: assuming regular tiles: problem with irregular tiles!!!
    //int i_offset = x * (nx_tile - 4) - 2;
    //int j_offset = y * (ny_tile - 4) - 2;
    //int k_offset = z * (nz_tile - 4) - 2;
    
    int i_offset = s1_tile->nd_element_offset.values[2] - (4 * x) - 2; 
    int j_offset = s1_tile->nd_element_offset.values[1] - (4 * y) - 2;
    int k_offset = s1_tile->nd_element_offset.values[0] - (4 * z) - 2;
    
    // sync_boundary due to overlapping
    if (x == 0) i_first = 3;
    else i_first = 2;
    if (y == 0) j_first = 3;
    else j_first = 2;
    if (z == 0) k_first = 3;
    else k_first = 2;
    if (x == nd_size.values[2] - 1) i_last = nx_tile - 3;
    else i_last = nx_tile - 2;
    if (y == nd_size.values[1] - 1) j_last = ny_tile - 3;
    else j_last = ny_tile - 2;
    if (z == nd_size.values[0] - 1) k_last = nz_tile - 3;
    else k_last = nz_tile - 2;
    
    
    for (k = k_first; k < k_last; k++) {
      for (j = j_first; j < j_last; j++) {
	for (i = i_first; i < i_last; i++) {
          exact( i + i_offset, j + j_offset, k + k_offset, u000ijk );
	  for (m = 0; m < 5; m++) {
	    tmp = ( u000ijk[m] - u_tile[k][j][i][m] );
	    error_local[0][0][0][m] = error_local[0][0][0][m] + tmp * tmp;
	  }
	}
      }
    }
    
}

