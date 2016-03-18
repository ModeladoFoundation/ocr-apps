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

#ifndef __BLOCK_H__
#define __BLOCK_H__

typedef struct Block_t Block_t;

#include "dbcommheader.h"
#include "control.h"

typedef struct Block_t {
   DbCommHeader_t  dbCommHeader;
   double      cells[0];
#if 0
// TODO: figure out which of these, if any, are still needed for native OCR factoring.
   int number;
   int level;
   int refine;
   int new_proc;
   int parent;       // if original block -1,
                     // else if on node, number in structure
                     // else (-2 - parent->number)
   int parent_node;
   int child_number;
   int nei_refine[6];
   int nei_level[6];  /* 0 to 5 = W, E, S, N, D, U; use -2 for boundary */
   int nei[6][2][2];  /* negative if off processor (-1 - proc) */
   int cen[3];
   //double ****array;   // This is now split out as Cells_t
#endif
} Block_t;

#define sizeof_Block_t (sizeof(Block_t) + ((control->x_block_size + 2) * (control->y_block_size + 2) * (control->z_block_size + 2) * control->num_vars * sizeof(double)))

typedef struct Face_t {
   DbCommHeader_t  dbCommHeader;
   double          cells[0];
} Face_t;

#define sizeof_Face_BasePart_t (sizeof(Face_t))


#endif // __BLOCK_H__
