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

#include <stdlib.h>
#include <math.h>

#include "control.h"
#include "block.h"
#include "proto.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "init   "
#endif

void init(blockClone_Params_t * myParams, ocrEdtDep_t depv[]) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__init_t;

   blockClone_Deps_t * myDeps  = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta    = myDeps->meta_Dep.ptr;
   Control_t         * control = myDeps->control_Dep.ptr;
   Block_t           * block   = myDeps->block_Dep.ptr;
   double (* pCells) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (block->cells);

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__init_t)
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
   unsigned int seed = (unsigned int) (meta->xPos + (meta->yPos * control->npx) + (meta->zPos * control->npx * control->npy));
   int var, i, j, k, l;
   l = 0;
   for (var = 0; var < control->num_vars; var++) {
      for (i = 0; i <= control->x_block_size+1; i++) {
         for (j = 0; j <= control->y_block_size+1; j++) {
            for (k = 0; k <= control->z_block_size+1; k++) {
               pCells[var][i][j][k] = 999900 + l++;
            }
         }
      }

      for (i = 1; i <= control->x_block_size; i++) {
         for (j = 1; j <= control->y_block_size; j++) {
            for (k = 1; k <= control->z_block_size; k++) {
               pCells[var][i][j][k] = ((double) rand_r(&seed))/((double) RAND_MAX);
               pCells[var][i][j][k] = ((double) ((meta->xPos * 2 + i) * 100 + (meta->yPos * 2 + j) * 10 + (meta->zPos * 4 + k)));
            }
         }
      }
   }

   // Generate the golden checksum.
   CALL_SUSPENDABLE_CALLEE
   checksum_BlockContribution (myParams, depv, 0);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
} // init
