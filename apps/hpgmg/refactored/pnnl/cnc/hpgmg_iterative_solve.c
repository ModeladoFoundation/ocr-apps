/*
 *  Copyright (c) 2014, The Regents of the University of California, through
 *  Lawrence Berkeley National Laboratory and UChicago Argonne, LLC.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/orother materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Portions contributed by Battelle Memorial Institute developed under Contract
 *  Number DE-SC0008717 with the U.S. Department of Energy for the operation of
 *  the Pacific Northwest National Laboratory: Copyright 2015, Battelle Memorial
 *  Institute.  Contributed under the terms of the original license.
 *
 *  Authors
 *   v.CnC: Ellen Porter (ellen.porter@pnnl.gov)
 *          Luke Rodriguez (luke.rodriguez@pnnl.gov)
 *          Pacific Northwest National Lab
 *   HPGMG: Samuel Williams (SWWilliams@lbl.gov)
 *          Lawrence Berkeley National Lab
 *
 */

#include "hpgmg.h"

/**
 * Step function definition for "iterative_solve"
 */
void hpgmg_iterative_solve(cncTag_t cycle, double ****f_in,
    double ****beta_x_in, double ****beta_y_in, double ****beta_z_in,
    double ****dinv_in, hpgmgCtx *ctx) {

  int level = ctx->num_levels - 1;
  int num_cells = ctx->num_cells_per_level[level];
  int num_blocks = ctx->num_blocks_per_level[level];
  int cells = ctx->num_cells_per_level[level]+2*GHOSTS;
  int sizeof_block = block_size(cells, cells, cells);

  //
  // INPUTS
  //
  double* r0_id[num_blocks][num_blocks][num_blocks];
  double* dinv[num_blocks][num_blocks][num_blocks];
  double* beta_x[num_blocks][num_blocks][num_blocks];
  double* beta_y[num_blocks][num_blocks][num_blocks];
  double* beta_z[num_blocks][num_blocks][num_blocks];
  double* u[num_blocks][num_blocks][num_blocks];

  {
    s64 _i, _j, _k;
    for (_i = 0; _i < ctx->num_blocks; _i++) {
      for (_j = 0; _j < ctx->num_blocks; _j++) {
        for (_k = 0; _k < ctx->num_blocks; _k++) {
          u[_i][_j][_k] = cncItemAlloc(sizeof_block);
          initialize_block(u[_i][_j][_k], cells, cells, cells);
        }
      }
    }
  }

  {
    s64 _i, _j, _k;
    for (_i = 0; _i < num_blocks; _i++) {
      for (_j = 0; _j < num_blocks; _j++) {
        for (_k = 0; _k < num_blocks; _k++) {
          r0_id[_i][_j][_k] = f_in[_i][_j][_k];
          dinv[_i][_j][_k] = dinv_in[_i][_j][_k];
          beta_x[_i][_j][_k] = beta_x_in[_i][_j][_k];
          beta_y[_i][_j][_k] = beta_y_in[_i][_j][_k];
          beta_z[_i][_j][_k] = beta_z_in[_i][_j][_k];
        }
      }
    }
  }

  iterative_solve(u, r0_id, dinv, beta_x, beta_y, beta_z, num_blocks,
      num_cells);

  //
  // OUTPUTS
  //
  { // Put "u_out" items
    s64 _i, _j, _k;
    for (_i = 0; _i < ctx->num_blocks; _i++) {
      for (_j = 0; _j < ctx->num_blocks; _j++) {
        for (_k = 0; _k < ctx->num_blocks; _k++) {
          cncPut_u(u[_i][_j][_k], ctx->num_smooth_cycles*2+1, cycle, 1,
              ctx->num_levels-1, _i, _j, _k, ctx);
        }
      }
    }
  }

}
