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

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <ocr.h>

// These are the "control variables" that are parsed from the command line or otherwise derived therefrom.  When
// passed in to the EDT that processes a block ("DriveBlock_Func"), the contents of this datablock are READ-ONLY,
// and it is shared among all instances of that EDT.

typedef struct {
   ocrGuid_t driveBlock_Template;
   double tol;
   int x_block_size;
   int y_block_size;
   int z_block_size;
   int npx;
   int npy;
   int npz;
   int num_refine;
   int block_change;
   int uniform_refine;
   int refine_freq;
   int num_vars;
   int comm_vars;
   int num_tsteps;
   int stages_per_ts;
   int checksum_freq;
   int stencil;
   int error_tol;
   int report_diffusion;
   int report_perf;
   int plot_freq;
   int code;
   int permute;
   int refine_ghost;
   int num_objects;
   int p2[18];       // Number of elements is max number of refinement levels, plust two.
   int mesh_size[3];
} Control_t;

#define sizeof_Control_t (sizeof(Control_t))

#endif // __CONTROL_H__
