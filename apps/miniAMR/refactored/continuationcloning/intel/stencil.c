// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// Portions Copyright (2016) Intel Corporation.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "proto.h"

// This routine does the stencil calculations.
void stencil_calc(Globals_t * const glbl, int var)
{
   int n, i, j, k, in;
   double sb, sm, sf, work[x_block_size+2][y_block_size+2][z_block_size+2];
   OBTAIN_ACCESS_TO_sorted_list
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells
   Block_t *bp;
   Cell_t  *cp;
   TRACE

   if (stencil == 7) {
      for (in = 0; in < sorted_index[num_refine+1]; in++) {
         n = sorted_list[in].n;
         bp = &blocks[n];
         cp = &cells[n];
         if (bp->number >= 0) {
            for (i = 1; i <= x_block_size; i++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++)
                     work[i][j][k] = ((*cp)[var][i-1][j  ][k  ] +
                                (*cp)[var][i  ][j-1][k  ] +
                                (*cp)[var][i  ][j  ][k-1] +
                                (*cp)[var][i  ][j  ][k  ] +
                                (*cp)[var][i  ][j  ][k+1] +
                                (*cp)[var][i  ][j+1][k  ] +
                                (*cp)[var][i+1][j  ][k  ])/7.0;
            for (i = 1; i <= x_block_size; i++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++)
                     (*cp)[var][i][j][k] = work[i][j][k];
         }
      }
   } else {
      for (in = 0; in < sorted_index[num_refine+1]; in++) {
         n = sorted_list[in].n;
         bp = &blocks[n];
         cp = &cells[n];
         if (bp->number >= 0) {
            for (i = 1; i <= x_block_size; i++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++) {
                     sb = (*cp)[var][i-1][j-1][k-1] +
                          (*cp)[var][i-1][j-1][k  ] +
                          (*cp)[var][i-1][j-1][k+1] +
                          (*cp)[var][i-1][j  ][k-1] +
                          (*cp)[var][i-1][j  ][k  ] +
                          (*cp)[var][i-1][j  ][k+1] +
                          (*cp)[var][i-1][j+1][k-1] +
                          (*cp)[var][i-1][j+1][k  ] +
                          (*cp)[var][i-1][j+1][k+1];
                     sm = (*cp)[var][i  ][j-1][k-1] +
                          (*cp)[var][i  ][j-1][k  ] +
                          (*cp)[var][i  ][j-1][k+1] +
                          (*cp)[var][i  ][j  ][k-1] +
                          (*cp)[var][i  ][j  ][k  ] +
                          (*cp)[var][i  ][j  ][k+1] +
                          (*cp)[var][i  ][j+1][k-1] +
                          (*cp)[var][i  ][j+1][k  ] +
                          (*cp)[var][i  ][j+1][k+1];
                     sf = (*cp)[var][i+1][j-1][k-1] +
                          (*cp)[var][i+1][j-1][k  ] +
                          (*cp)[var][i+1][j-1][k+1] +
                          (*cp)[var][i+1][j  ][k-1] +
                          (*cp)[var][i+1][j  ][k  ] +
                          (*cp)[var][i+1][j  ][k+1] +
                          (*cp)[var][i+1][j+1][k-1] +
                          (*cp)[var][i+1][j+1][k  ] +
                          (*cp)[var][i+1][j+1][k+1];
                     work[i][j][k] = (sb + sm + sf)/27.0;
                  }
            for (i = 1; i <= x_block_size; i++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++)
                     (*cp)[var][i][j][k] = work[i][j][k];
         }
      }
   }
}
