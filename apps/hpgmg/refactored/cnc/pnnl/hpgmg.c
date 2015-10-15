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

void hpgmg_cncInitialize(hpgmgArgs *args, hpgmgCtx *ctx) {

  double* _beta_x[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
  double* _beta_y[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
  double* _beta_z[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
  double* _alpha[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];

  { // Initialize beta, alpha, u_true, f, dinv
    int level, i, j, k;
    for (level = 0; level < ctx->num_levels; level++) {
      for (k = 0; k < ctx->num_blocks; k++) {
        for (j = 0; j < ctx->num_blocks; j++) {
          for (i = 0; i < ctx->num_blocks; i++) {

            int num_cells = ctx->num_cells_per_level[level];
            int num_blocks = ctx->num_blocks_per_level[level];
            int cells = ctx->num_cells_per_level[level]+2*GHOSTS;
            int sizeof_block = block_size(cells, cells, cells);

            double* beta_x = cncItemAlloc(sizeof_block);
            double* beta_y = cncItemAlloc(sizeof_block);
            double* beta_z = cncItemAlloc(sizeof_block);
            double* alpha  = cncItemAlloc(sizeof_block);
            double* u_true = cncItemAlloc(sizeof_block);
            double* dinv = cncItemAlloc(sizeof_block);

            initialize_block(beta_x, cells, cells, cells);
            initialize_block(beta_y, cells, cells, cells);
            initialize_block(beta_z, cells, cells, cells);
            initialize_block(alpha, cells, cells, cells);
            initialize_block(u_true, cells, cells, cells);
            initialize_block(dinv, cells, cells, cells);

            if(level == 0) {
              double *f = cncItemAlloc(sizeof_block);
              initialize_block(f, cells, cells, cells);
              initialize(beta_x, beta_y, beta_z, alpha, u_true, f, i, j, k,
                  num_blocks, num_cells);
              cncPut_f(f, 0, level, i, j, k, ctx);
            } else {
              if(!ctx->restrict_blocks[level-1]) {

                restrict_cells(alpha, _alpha[level-1][i][j][k], num_cells,
                    NONE);
                restrict_cells(beta_x, _beta_x[level-1][i][j][k], num_cells,
                    FACE_X);
                restrict_cells(beta_y, _beta_y[level-1][i][j][k], num_cells,
                    FACE_Y);
                restrict_cells(beta_z, _beta_z[level-1][i][j][k], num_cells,
                    FACE_Z);

              } else {
                if((i % 2) == 0 && (j % 2) == 0 && (k % 2) == 0) {
                  restrict_blocks(beta_x, i, j, k, num_cells, FACE_X,
                      _beta_x[level-1][i][j][k],
                      _beta_x[level-1][i+1][j][k],
                      _beta_x[level-1][i][j+1][k],
                      _beta_x[level-1][i+1][j+1][k],
                      _beta_x[level-1][i][j][k+1],
                      _beta_x[level-1][i+1][j][k+1],
                      _beta_x[level-1][i][j+1][k+1],
                      _beta_x[level-1][i+1][j+1][k+1]);
                  restrict_blocks(beta_y, i, j, k, num_cells, FACE_Y,
                      _beta_y[level-1][i][j][k],
                      _beta_y[level-1][i+1][j][k],
                      _beta_y[level-1][i][j+1][k],
                      _beta_y[level-1][i+1][j+1][k],
                      _beta_y[level-1][i][j][k+1],
                      _beta_y[level-1][i+1][j][k+1],
                      _beta_y[level-1][i][j+1][k+1],
                      _beta_y[level-1][i+1][j+1][k+1]);
                  restrict_blocks(beta_z, i, j, k, num_cells, FACE_Z,
                      _beta_z[level-1][i][j][k],
                      _beta_z[level-1][i+1][j][k],
                      _beta_z[level-1][i][j+1][k],
                      _beta_z[level-1][i+1][j+1][k],
                      _beta_z[level-1][i][j][k+1],
                      _beta_z[level-1][i+1][j][k+1],
                      _beta_z[level-1][i][j+1][k+1],
                      _beta_z[level-1][i+1][j+1][k+1]);
                  restrict_blocks(alpha, i, j, k, num_cells, NONE,
                      _alpha[level-1][i][j][k],
                      _alpha[level-1][i+1][j][k],
                      _alpha[level-1][i][j+1][k],
                      _alpha[level-1][i+1][j+1][k],
                      _alpha[level-1][i][j][k+1],
                      _alpha[level-1][i+1][j][k+1],
                      _alpha[level-1][i][j+1][k+1],
                      _alpha[level-1][i+1][j+1][k+1]);
                }
              }
            }

            double dominant_eigenvalue_of_DinvA = rebuild_operator(dinv, beta_x,
                beta_y, beta_z, alpha, i, j, k, num_blocks, num_cells);

            if(ctx->dominant_eigenvalue_of_DinvA[level] <
                dominant_eigenvalue_of_DinvA) {
              ctx->dominant_eigenvalue_of_DinvA[level] =
                  dominant_eigenvalue_of_DinvA;
            }

            if(ctx->dominant_eigenvalue_of_DinvA[level] <= 0.0)
              printf("dominant_eigenvalue_of_DinvA <= 0.0 !\n");

            _beta_x[level][i][j][k] = beta_x;
            _beta_y[level][i][j][k] = beta_y;
            _beta_z[level][i][j][k] = beta_z;
            _alpha[level][i][j][k] =  alpha;

            cncPut_beta_x(beta_x, level, i, j, k, ctx);
            cncPut_beta_y(beta_y, level, i, j, k, ctx);
            cncPut_beta_z(beta_z, level, i, j, k, ctx);
            cncPut_alpha(alpha, level, i, j, k, ctx);
            cncPut_u_true(u_true, level, i, j, k, ctx);
            cncPut_dinv(dinv, level, i, j, k, ctx);
          }
        }
      }
    }
  }

  { // Put "u0" items
    s64 _i, _j, _k;
    for (_i = 0; _i < ctx->num_blocks; _i++) {
      for (_j = 0; _j < ctx->num_blocks; _j++) {
        for (_k = 0; _k < ctx->num_blocks; _k++) {
          int cells = ctx->num_cells_per_level[0]+2*GHOSTS;
          int sizeof_block = block_size(cells, cells, cells);
          double *u0 = cncItemAlloc(sizeof_block);
          initialize_block(u0, cells, cells, cells);
          cncPut_u(u0, 1, 0, 0, 0, _i, _j, _k, ctx);
        }
      }
    }
  }

  { // Put "u1" items
    s64 _cycle, _level, _i, _j, _k;
    for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
      for (_level = 1; _level < ctx->num_levels-1; _level++) {
        for (_i = 0; _i < ctx->num_blocks; _i++) {
          for (_j = 0; _j < ctx->num_blocks; _j++) {
            for (_k = 0; _k < ctx->num_blocks; _k++) {
              int cells = ctx->num_cells_per_level[_level]+2*GHOSTS;
              int sizeof_block = block_size(cells, cells, cells);
              double *u1 = cncItemAlloc(sizeof_block);
              initialize_block(u1, cells, cells, cells);
              cncPut_u(u1, 1, _cycle, 0, _level, _i, _j, _k, ctx);
            }
          }
        }
      }
    }
  }
  { // Prescribe "exchange_boundaries_apply_bc" steps
    s64 _u_id, _cycle, _level, _i, _j, _k;
    for (_u_id = 0; _u_id < ctx->num_smooth_cycles+1; _u_id++) {
      for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
        for (_level = 0; _level < ctx->num_levels-1; _level++) {
          for (_i = 0; _i < ctx->num_blocks; _i++) {
            for (_j = 0; _j < ctx->num_blocks; _j++) {
              for (_k = 0; _k < ctx->num_blocks; _k++) {
                cncPrescribe_exchange_boundaries_apply_bc(_u_id, _cycle, 0,
                    _level, _i, _j, _k, ctx);
              }
            }
          }
        }
      }
    }
  }

  { // Prescribe "exchange_boundaries_apply_bc" steps
    s64 _u_id, _cycle, _level, _i, _j, _k;
    for (_u_id = 0; _u_id < ctx->num_smooth_cycles; _u_id++) {
      for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
        for (_level = 0; _level < ctx->num_levels-1; _level++) {
          for (_i = 0; _i < ctx->num_blocks; _i++) {
            for (_j = 0; _j < ctx->num_blocks; _j++) {
              for (_k = 0; _k < ctx->num_blocks; _k++) {
                cncPrescribe_exchange_boundaries_apply_bc(_u_id, _cycle, 1,
                    _level, _i, _j, _k, ctx);
              }
            }
          }
        }
      }
    }
  }

  { // Prescribe "smooth" steps
    s64 _u_id, _cycle, _segment, _level, _i, _j, _k;
    for (_u_id = 0; _u_id < ctx->num_smooth_cycles; _u_id++) {
      for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
        for (_segment = 0; _segment < 2; _segment++) {
          for (_level = 0; _level < ctx->num_levels-1; _level++) {
            for (_i = 0; _i < ctx->num_blocks; _i++) {
              for (_j = 0; _j < ctx->num_blocks; _j++) {
                for (_k = 0; _k < ctx->num_blocks; _k++) {
                  cncPrescribe_smooth(_u_id, _cycle, _segment, _level, _i, _j,
                      _k, ctx);
                }
              }
            }
          }
        }
      }
    }
  }

  { // Prescribe "compute_residaul" steps
    s64 _cycle, _level, _i, _j, _k;
    for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
      for (_level = 0; _level < ctx->num_levels-1; _level++) {
        for (_i = 0; _i < ctx->num_blocks; _i++) {
          for (_j = 0; _j < ctx->num_blocks; _j++) {
            for (_k = 0; _k < ctx->num_blocks; _k++) {
              cncPrescribe_compute_residaul(_cycle, _level, _i, _j, _k, ctx);
            }
          }
        }
      }
    }
  }

  { // Prescribe "restrict_grid" steps
    s64 _cycle, _level, _i, _j, _k;
    for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
      for (_level = 0; _level < ctx->num_levels-1; _level++) {
        for (_i = 0; _i < ctx->num_blocks; _i++) {
          for (_j = 0; _j < ctx->num_blocks; _j++) {
            for (_k = 0; _k < ctx->num_blocks; _k++) {
              cncPrescribe_restrict_grid(_cycle, _level, _i, _j, _k, ctx);
            }
          }
        }
      }
    }
  }

  { // Prescribe "interpolate" steps
    s64 _cycle, _level, _i, _j, _k;
    for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
      for (_level = 0; _level < ctx->num_levels-1; _level++) {
        for (_i = 0; _i < ctx->num_blocks; _i++) {
          for (_j = 0; _j < ctx->num_blocks; _j++) {
            for (_k = 0; _k < ctx->num_blocks; _k++) {
              cncPrescribe_interpolate(_cycle, _level, _i, _j, _k, ctx);
            }
          }
        }
      }
    }
  }

  { // Prescribe "iterative_solve" steps
    s64 _cycle;
    for (_cycle = 0; _cycle < ctx->num_cycles; _cycle++) {
      cncPrescribe_iterative_solve(_cycle, ctx);
    }
  }

  // Set finalizer function's tag
  hpgmg_await(ctx);

}

void hpgmg_cncFinalize(double ****u, double ****beta_x, double ****beta_y,
    double ****beta_z, double ****dinv, double ****f, hpgmgCtx *ctx) {

  int num_blocks = ctx->num_blocks;
  int num_cells = ctx->num_cells;

  double norm_of_residual = 0.0;
  double norm_of_dinv = 0.0;
  double norm_of_f = 0.0;

  {
    s64 i, j, k;
    for (k = 0; k < ctx->num_blocks; k++) {
      for (j = 0; j < ctx->num_blocks; j++) {
        for (i = 0; i < ctx->num_blocks; i++) {

          // Exchange boundaries one more time
          double* n[27];
          int i_, j_, k_;
          int local_id = 0;
          for(k_ = k - 1; k_ <= k + 1; k_++) {
            for(j_ = j - 1; j_ <= j + 1; j_++) {
              for(i_ = i - 1; i_ <= i + 1; i_++) {
                if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < num_blocks && j_ <
                    num_blocks && k_ < num_blocks) {
                  n[local_id] = u[i_][j_][k_];
                } else {
                  n[local_id] = u[i][j][k]; // Self
                }
                local_id++;
              }
            }
          }

          int cells = num_cells + 2*GHOSTS;
          int sizeof_block = block_size(cells, cells, cells);
          double *u_exchanged = cncLocalAlloc(sizeof_block);
          double *residual_output =cncLocalAlloc(sizeof_block);
          initialize_block(u_exchanged, cells, cells, cells);
          initialize_block(residual_output, cells, cells, cells);

          copy(u_exchanged, u[i][j][k], num_cells);

          exchange_boundary(u_exchanged, i, j, k, FACE, num_blocks, num_cells,
              n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9],
              n[10], n[11], n[12], n[14], n[15], n[16], n[17], n[18], n[19],
              n[20], n[21], n[22], n[23], n[24], n[25], n[26]);

          apply_boundary_condition(u_exchanged, i, j, k, num_blocks, num_cells);

          // Compute final residual
          residual(residual_output, f[i][j][k], u_exchanged, beta_x[i][j][k],
              beta_y[i][j][k], beta_z[i][j][k], num_blocks, num_cells);

          double norm_of_residual_block = norm(residual_output, num_cells);
          if(norm_of_residual_block > norm_of_residual)
            norm_of_residual = norm_of_residual_block;

          double norm_of_dinv_block = norm(dinv[i][j][k], num_cells);
          if(norm_of_dinv_block > norm_of_dinv)
            norm_of_dinv = norm_of_dinv_block;

          double norm_of_f_block = norm(f[i][j][k], num_cells);
          if(norm_of_f_block > norm_of_f)
            norm_of_f = norm_of_f_block;

          cncLocalFree(u_exchanged);
          cncLocalFree(residual_output);
        }
      }
    }
  }

  norm_of_dinv = 1.0; // Hard coded to 1...

  printf("Normal of residual %.17g\n", norm_of_residual);
  printf("norm_of_f: %.17g\n", norm_of_f);
  printf("norm_of_dinv: %.17g\n", norm_of_dinv);
  printf("%.17g\n", norm_of_residual/norm_of_f);
  printf("%.17g\n", norm_of_residual/norm_of_dinv);
}
