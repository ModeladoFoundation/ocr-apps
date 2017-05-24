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

#define MAX_NUM_BLOCKS 500        //--max_blocks - maximun number of blocks per core
#define TARGET_ACTIVE 0           //--target_active - (>= 0) target number of blocks per core, none if 0
#define TARGET_MAX 0              //--target_max - (>= 0) max number of blocks per core, none if 0
#define TARGET_MIN 0              //--target_min - (>= 0) min number of blocks per core, none if 0
#define NUM_REFINE 5              //--num_refine - (>= 0) number of levels of refinement
#define UNIFORM_REFINE 0          //--uniform_refine - if 1, then grid is uniformly refined
#define X_BLOCK_SIZE 10           //--nx - block size x (even && > 0)
#define Y_BLOCK_SIZE 10           //--ny - block size y (even && > 0)
#define Z_BLOCK_SIZE 10           //--nz - block size z (even && > 0)
#define NUM_VARS 40               //--num_vars - number of variables (> 0)
#define COMM_VARS 0               //--comm_vars - number of vars to communicate together
#define INIT_BLOCK_X 1            //--init_x - initial blocks in x (> 0)
#define INIT_BLOCK_Y 1            //--init_y - initial blocks in y (> 0)
#define INIT_BLOCK_Z 1            //--init_z - initial blocks in z (> 0)
#define REORDER 1                 //--reorder - ordering of blocks if initial number > 1
#define NPX 1                     //--npx - (0 < npx <= num_pes)
#define NPY 1                     //--npy - (0 < npy <= num_pes)
#define NPZ 1                     //--npz - (0 < npz <= num_pes)
#define INBALANCE 0               //--inbalance - percentage inbalance to trigger inbalance
#define REFINE_FREQ 5             //--refine_freq - frequency (in timesteps) of checking for refinement
#define REPORT_DIFFUSION 0        //--report_diffusion - (>= 0) none if 0
#define ERROR_TOL 8               //--error_tol - (e^{-error_tol} ; >= 0)
#define NUM_TSTEPS 20             //--num_tsteps - number of timesteps (> 0)
#define STAGES_PER_TS 20          //--stages_per_ts - number of comm/calc stages per timestep
#define CHECKSUM_FREQ 5           //--checksum_freq - number of stages between checksums
#define STENCIL 7                 //--stencil - 7 or 27 point (27 will not work with refinement (except uniform))
#define REPORT_PERF 4             //--report_perf - 0, 1, 2
#define PLOT_FREQ 0               //--plot_freq - 0, 1, 2
#define NUM_OBJECTS 0             //--num_objects - (>= 0) number of objects to cause refinement
                                  //--object - type, position, movement, size, size rate of change
#define LB_OPT 0                  //--lb_opt - load balancing - 0 none, 1 each refine, 2 each refine phase
#define BLOCK_CHANGE 0            //--block_change - (>= 0) number of levels a block can change in a timestep
#define CODE 0                    //--code - closely minic communication of different codes
                                    //         0 minimal sends, 1 send ghosts, 2 send ghosts and process on send
#define PERMUTE 0                 //--permute - altenates directions in communication
#define NONBLOCKING 1             //--blocking_send - use blocking sends instead of nonblocking
#define REFINE_GHOST 0            //--refine_ghost - use full extent of block (including ghosts) to determine if block is refined

#define MAX_OBJECTS 10            //Max objects that the user can specify without having to rebuild the code
#define MAX_REFINE_LEVELS 8       //Max refinement levels that the user can specify without having to rebuild the code
#define MAX_NUM_VARS 40           //Max number of vars the user can specify without having to rebuild the code
