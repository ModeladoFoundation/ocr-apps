// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
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

#include <math.h>
#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "timer.h"
#include "proto.h"

// Generate check sum for a variable over all active blocks.
double check_sum(Globals_t * const glbl, int const var)
{
   OBTAIN_ACCESS_TO_sorted_list
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells
   typedef struct {
      Frame_Header_t myFrame;
      int n, in, i, j, k;
      double sum, gsum, block_sum, t1, t2, t3;
      struct {
         Block_t *bp;
         Cell_t  *cp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__check_sum_t;

//#define n                 (lcl->n)  // Overloaded symbol.  Just use fully-qualified symbol in the code
#define in                (lcl->in)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define sum               (lcl->sum)
#define gsum              (lcl->gsum)
#define block_sum         (lcl->block_sum)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define bp                (lcl->pointers.bp)
#define cp                (lcl->pointers.cp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__check_sum_t)
   TRACE

   t1 = timer();

   sum = 0.0;
   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      bp = &blocks[lcl->n];
      cp = &cells[lcl->n];
      if (bp->number >= 0) {
         block_sum = 0.0;
         for (i = 1; i <= x_block_size; i++)
            for (j = 1; j <= y_block_size; j++)
               for (k = 1; k <= z_block_size; k++)
                  block_sum += (*cp)[var][i][j][k];
         sum += block_sum;
      }
   }

   t2 = timer();

   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &sum, &gsum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   t3 = timer();
   timer_cs_red += t3 - t2;
   timer_cs_calc += t2 - t1;
   total_red++;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(gsum)
   SUSPENDABLE_FUNCTION_EPILOGUE
//#undef  n
#undef  in
#undef  i
#undef  j
#undef  k
#undef  sum
#undef  gsum
#undef  block_sum
#undef  t1
#undef  t2
#undef  t3
#undef  bp
#undef  cp
}
