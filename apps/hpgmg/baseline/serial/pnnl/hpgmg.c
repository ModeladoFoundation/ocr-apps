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

// level, i, j, k
block3 betas[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
block alphas[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
block u_trues[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];
block dinvs[NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];

// segment, level, i, j, k
block fs[NUM_CYCLES*2+1][NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];

// smooth_cycle magic, segment, level, i, j, k
block u[(CHEBYSHEV_DEGREE * NUM_SMOOTHS)*2+3][NUM_CYCLES*2+1][NUM_LEVELS][NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS];

int debug_graph = 0;

int main() {

  int level, num_blocks, num_cells, i, j, k;
  int cell_i, cell_j, cell_k;
  int cycle, segment, smooth_cycle;

  // Initialize cnc context and global data
  hpgmg* context = malloc(sizeof(hpgmg));
  context->num_levels = NUM_LEVELS;
  context->num_cycles = NUM_CYCLES;
  context->num_smooth_cycles = CHEBYSHEV_DEGREE * NUM_SMOOTHS;
  if((CHEBYSHEV_DEGREE*NUM_SMOOTHS)&1){
    fprintf(stderr,"error... CHEBYSHEV_DEGREE*NUM_SMOOTHS must be even for the chebyshev smoother...\n");
    exit(0);
  }

  num_blocks = NUM_BLOCKS;
  num_cells = NUM_CELLS;
  for(level = 0; level < context->num_levels; level++) {
    context->num_blocks[level] = num_blocks;
    if(num_cells == 8) num_blocks = 1; // TODO: figure out when we drop # blocks
    if(level == 0 || context->num_blocks[level-1] == context->num_blocks[level]) num_cells /= 2;
    context->num_cells[level] = num_cells == 0 ? 1 : num_cells*2;
    context->dominant_eigenvalue_of_DinvA[level] = -1e9;
  }

  // Initialize beta, alpha, u_true and f
  TYPE norm_of_f = 0.0;
  TYPE norm_of_dinvf = 1.0;
  for(level = 0; level < context->num_levels; level++) {
    for(k = 0; k < NUM_BLOCKS; k++) {
      for(j = 0; j < NUM_BLOCKS; j++) {
        for(i = 0; i < NUM_BLOCKS; i++) {
          if(level == 0) {
            initialize_block(&betas[level][i][j][k], &alphas[level][i][j][k], &u_trues[level][i][j][k],
                &fs[0][0][i][j][k], i, j, k, context->num_blocks[level], context->num_cells[level]);
            TYPE norm_of_f_block = norm(&fs[0][0][i][j][k], context->num_cells[level]);
            if(norm_of_f_block > norm_of_f)
              norm_of_f = norm_of_f_block;
          } else {
            if(context->num_blocks[level - 1] == context->num_blocks[level]) {
              restrict_cells_internal3(&betas[level][i][j][k], &betas[level - 1][i][j][k], context->num_cells[level]);
              restrict_cells_internal(&alphas[level][i][j][k], &alphas[level - 1][i][j][k], context->num_cells[level]);
            } else {
              if(i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
                restrict_blocks_internal3(&betas[level][i][j][k], i, j, k, context->num_cells[level],
                    &betas[level - 1][i][j][k], &betas[level - 1][i+1][j][k],
                    &betas[level - 1][i][j+1][k], &betas[level - 1][i+1][j+1][k],
                    &betas[level - 1][i][j][k+1], &betas[level - 1][i+1][j][k+1],
                    &betas[level - 1][i][j+1][k+1], &betas[level - 1][i+1][j+1][k+1]);
                restrict_blocks_internal(&alphas[level][i][j][k], i, j, k, context->num_cells[level],
                    &alphas[level - 1][i][j][k],     &alphas[level - 1][i+1][j][k],
                    &alphas[level - 1][i][j+1][k],   &alphas[level - 1][i+1][j+1][k],
                    &alphas[level - 1][i][j][k+1],   &alphas[level - 1][i+1][j][k+1],
                    &alphas[level - 1][i][j+1][k+1], &alphas[level - 1][i+1][j+1][k+1]);
              }
            }
          }
          double dominant_eigenvalue_of_DinvA = rebuild_operator(&dinvs[level][i][j][k],
              &betas[level][i][j][k], &alphas[level][i][j][k], i, j, k,
              context->num_blocks[level], context->num_cells[level]);
          if(context->dominant_eigenvalue_of_DinvA[level] < dominant_eigenvalue_of_DinvA) {
            context->dominant_eigenvalue_of_DinvA[level] = dominant_eigenvalue_of_DinvA;
          }
          if(context->dominant_eigenvalue_of_DinvA[level] <= 0.0)
            printf("dominant_eigenvalue_of_DinvA <= 0.0 !\n");

        }
      }
    }
  }

  // Initialize temp and u to 0.0 for all levels and segments
  for(segment = 0; segment < context->num_cycles*2; segment++) {
    for(level = 0; level < context->num_levels; level++) {
      for(k = 0; k < NUM_BLOCKS; k++) {
        for(j = 0; j < NUM_BLOCKS; j++) {
          for(i = 0; i < NUM_BLOCKS; i++) {
            for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
              for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
                for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
                  if(level != 0)
                    u[1][segment][level][i][j][k].x[cell_i][cell_j][cell_k] = 0.0; // u
                  else if(level == 0 && segment == 0)
                    u[1][segment][level][i][j][k].x[cell_i][cell_j][cell_k] = 0.0; // u
                }
              }
            }
          }
        }
      }
    }
  }

  // Completes initialization.. run HPGMG V-Cycle
  for(cycle = 0, segment = 0; cycle < context->num_cycles; cycle++, segment++) {
    for(level = 0; level < context->num_levels - 1; level++) {
      for(smooth_cycle = 0; smooth_cycle < context->num_smooth_cycles; smooth_cycle++) {
        for(k = 0; k < NUM_BLOCKS; k++) {
          for(j = 0; j < NUM_BLOCKS; j++) {
            for(i = 0; i < NUM_BLOCKS; i++) {

              int in_exchange = 	smooth_cycle * 2 + 1;
              int out_exchange =	smooth_cycle * 2 + 2;

              block n[27];
              int i_, j_, k_;
              int local_id = 0;
              for(k_ = k - 1; k_ <= k + 1; k_++) {
                for(j_ = j - 1; j_ <= j + 1; j_++) {
                  for(i_ = i - 1; i_ <= i + 1; i_++) {
                    if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < context->num_blocks[level] &&
                        j_ < context->num_blocks[level] && k_ < context->num_blocks[level]) {
                      n[local_id] = u[in_exchange][segment][level][i_][j_][k_];
                    } else {
                      n[local_id] = u[in_exchange][segment][level][i][j][k]; // Self
                    }
                    local_id++;
                  }
                }
              }

              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running exchange boundaries <%d, %d, %d, %d, %d, %d, %d>\n", smooth_cycle, cycle, 0, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", in_exchange, cycle, 0, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange, cycle, 0, level, i, j, k);

              copy(	&u[out_exchange][segment][level][i][j][k],
                  &u[in_exchange][segment][level][i][j][k], context->num_cells[level]);

              exchange_boundary_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                  FACE, context->num_blocks[level], context->num_cells[level],
                  &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
                  &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
                  &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);

              apply_boundary_condition_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                  context->num_blocks[level], context->num_cells[level]);

            }
          }
        }

        for(k = 0; k < NUM_BLOCKS; k++) {
          for(j = 0; j < NUM_BLOCKS; j++) {
            for(i = 0; i < NUM_BLOCKS; i++) {

              int in_smooth_1 = 	smooth_cycle * 2;    // temp
              int in_smooth_2 = 	smooth_cycle * 2 + 2;// u
              int out_smooth = 	smooth_cycle * 2 + 3;

              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running smooth <%d, %d, %d, %d, %d, %d, %d>\n", smooth_cycle, cycle, 0, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n",
                  in_smooth_1, (smooth_cycle == 0) ? 0 : cycle, 0, (smooth_cycle == 0) ? 0: level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   u<%d, %d, %d, %d, %d, %d, %d>\n", in_smooth_2, cycle, 0, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   f<%d, %d, %d, %d, %d>\n", (level == 0) ? 0 : cycle, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", out_smooth, cycle, 0, level, i, j, k);

              // in smooth 1 is empty when smooth cycle == 0, we don't need it
              block f_in = level == 0 ? fs[0][level][i][j][k] : fs[segment][level][i][j][k];
              block u_1_in = smooth_cycle == 0 ? u[in_smooth_1][0][0][i][j][k] : u[in_smooth_1][segment][level][i][j][k];
              block u_2_in = u[in_smooth_2][segment][level][i][j][k];

              smooth(&u[out_smooth][segment][level][i][j][k], &u_1_in, &u_2_in,
                  &betas[level][i][j][k], &dinvs[level][i][j][k],
                  &f_in, context->num_blocks[level], context->num_cells[level],
                  context->dominant_eigenvalue_of_DinvA[level], smooth_cycle);

            }
          }
        }
      } // End smooth

      // Exchange boundaries, apply boundary conditions for u
      for(k = 0; k < NUM_BLOCKS; k++) {
        for(j = 0; j < NUM_BLOCKS; j++) {
          for(i = 0; i < NUM_BLOCKS; i++) {

            block n[27];
            int i_, j_, k_;
            int local_id = 0;

            int in_exchange = 	context->num_smooth_cycles * 2 + 1;
            int out_exchange =	context->num_smooth_cycles * 2 + 2;

            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running exchange boundaries <%d, %d, %d, %d, %d, %d, %d>\n", smooth_cycle, cycle, 0, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", in_exchange, cycle, 0, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange, cycle, 0, level, i, j, k);

            for(k_ = k - 1; k_ <= k + 1; k_++) {
              for(j_ = j - 1; j_ <= j + 1; j_++) {
                for(i_ = i - 1; i_ <= i + 1; i_++) {
                  if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < context->num_blocks[level] &&
                      j_ < context->num_blocks[level] && k_ < context->num_blocks[level]) {
                    n[local_id] = u[in_exchange][segment][level][i_][j_][k_];
                  } else {
                    n[local_id] = u[in_exchange][segment][level][i][j][k]; // Self
                  }
                  local_id++;
                }
              }
            }

            copy(	&u[out_exchange][segment][level][i][j][k],
                &u[in_exchange][segment][level][i][j][k], context->num_cells[level]);

            exchange_boundary_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                FACE, context->num_blocks[level], context->num_cells[level],
                &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
                &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
                &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);

            apply_boundary_condition_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                context->num_blocks[level], context->num_cells[level]);

          }
        }
      }

      // Compute the residual
      for(k = 0; k < NUM_BLOCKS; k++) {
        for(j = 0; j < NUM_BLOCKS; j++) {
          for(i = 0; i < NUM_BLOCKS; i++) {

            int out_exchange =	context->num_smooth_cycles * 2 + 2;

            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running residual <%d, %d, %d, %d, %d>\n", cycle, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange, cycle, 0, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   f<%d, %d, %d, %d, %d>\n", (level == 0) ? 0: cycle, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", 0, cycle, 1, level, i, j, k);

            block f_in = level == 0 ? fs[0][level][i][j][k] : fs[segment][level][i][j][k];
            block u_in = u[out_exchange][segment][level][i][j][k];

            residual(&u[0][segment+1][level][i][j][k], &f_in, &u_in, &betas[level][i][j][k],
                context->num_blocks[level], context->num_cells[level]);

          }
        }
      }

      // Restrict residual to f
      for(k = 0; k < NUM_BLOCKS; k++) {
        for(j = 0; j < NUM_BLOCKS; j++) {
          for(i = 0; i < NUM_BLOCKS; i++) {

            int temp = 0;

            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running restrict <%d, %d, %d, %d, %d>\n", cycle, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", 0, cycle, 1, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> f<%d, %d, %d, %d, %d>\n", cycle, level + 1, i, j, k);

            if(context->num_blocks[level] == context->num_blocks[level + 1]) {
              restrict_cells_internal(&fs[segment][level+1][i][j][k],
                  &u[temp][segment+1][level][i][j][k], context->num_cells[level+1]);

            } else {
              if(i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
                restrict_blocks_internal(&fs[segment][level+1][i][j][k], i, j, k, context->num_cells[level+1],
                    &u[temp][segment+1][level][i][j][k],     &u[temp][segment+1][level][i+1][j][k],
                    &u[temp][segment+1][level][i][j+1][k],   &u[temp][segment+1][level][i+1][j+1][k],
                    &u[temp][segment+1][level][i][j][k+1],   &u[temp][segment+1][level][i+1][j][k+1],
                    &u[temp][segment+1][level][i][j+1][k+1], &u[temp][segment+1][level][i+1][j+1][k+1]);


              }
            }
          }
        } // end block ijk
      } // end smooth cycles
    } // end levels

    int out_exchange =	context->num_smooth_cycles * 2 + 1;

    // Iterative solve
    if(debug_graph) printf("Running iterative solve <%d>\n", cycle);
    if(debug_graph) printf("\t<- f<%d, %d>\n", cycle, level);
    if(debug_graph) printf("\t-> u<%d, %d, %d, %d>\n", out_exchange, cycle, 1, level);

    iterative_solve(u[out_exchange][segment+1][level], fs[segment][level], dinvs[level],
        betas[level], context->num_blocks[level], context->num_cells[level]);

    segment++;

    for(level = context->num_levels - 2; level >= 0; level--) {
      // Interpolate
      for(k = 0; k < NUM_BLOCKS; k++) {
        for(j = 0; j < NUM_BLOCKS; j++) {
          for(i = 0; i < NUM_BLOCKS; i++) {

            int out_exchange = context->num_smooth_cycles * 2 + 1;

            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running interpolate <%d, %d, %d, %d, %d>\n", cycle, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange, cycle, 1, level + 1, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange + 1, cycle, 0, level, i, j, k);
            if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", 1, cycle, 1, level, i, j, k);


            if(context->num_blocks[level] == context->num_blocks[level + 1]) {
              interpolate_cells_internal(
                  &u[1][segment][level][i][j][k],
                  &u[out_exchange][segment][level + 1][i][j][k],
                  &u[out_exchange+1][segment - 1][level][i][j][k],
                  context->num_cells[level]);
            } else {
              if(i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
                interpolate_blocks_internal(&u[out_exchange][segment][level + 1][i][j][k], context->num_cells[level+1],
                    &u[1][segment][level][i][j][k],
                    &u[1][segment][level][i+1][j][k],
                    &u[1][segment][level][i][j+1][k],
                    &u[1][segment][level][i+1][j+1][k],
                    &u[1][segment][level][i][j][k+1],
                    &u[1][segment][level][i+1][j][k+1],
                    &u[1][segment][level][i][j+1][k+1],
                    &u[1][segment][level][i+1][j+1][k+1],
                    &u[out_exchange+1][segment - 1][level][i][j][k],
                    &u[out_exchange+1][segment - 1][level][i+1][j][k],
                    &u[out_exchange+1][segment - 1][level][i][j+1][k],
                    &u[out_exchange+1][segment - 1][level][i+1][j+1][k],
                    &u[out_exchange+1][segment - 1][level][i][j][k+1],
                    &u[out_exchange+1][segment - 1][level][i+1][j][k+1],
                    &u[out_exchange+1][segment - 1][level][i][j+1][k+1],
                    &u[out_exchange+1][segment - 1][level][i+1][j+1][k+1]);
              }
            }
          }
        }
      }

      for(smooth_cycle = 0; smooth_cycle < context->num_smooth_cycles; smooth_cycle++) {
        for(k = 0; k < NUM_BLOCKS; k++) {
          for(j = 0; j < NUM_BLOCKS; j++) {
            for(i = 0; i < NUM_BLOCKS; i++) {

              int in_exchange = 	smooth_cycle * 2 + 1;
              int out_exchange =	smooth_cycle * 2 + 2;

              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running exchange boundaries <%d, %d, %d, %d, %d, %d, %d>\n", smooth_cycle, cycle, 1, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n", in_exchange, cycle, 1, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", out_exchange, cycle, 1, level, i, j, k);

              block n[27];
              int i_, j_, k_;
              int local_id = 0;
              for(k_ = k - 1; k_ <= k + 1; k_++) {
                for(j_ = j - 1; j_ <= j + 1; j_++) {
                  for(i_ = i - 1; i_ <= i + 1; i_++) {
                    if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < context->num_blocks[level] &&
                        j_ < context->num_blocks[level] && k_ < context->num_blocks[level]) {
                      n[local_id] = u[in_exchange][segment][level][i_][j_][k_];
                    } else {
                      n[local_id] = u[in_exchange][segment][level][i][j][k]; // Self
                    }
                    local_id++;
                  }
                }
              }

              copy(	&u[out_exchange][segment][level][i][j][k],
                  &u[in_exchange][segment][level][i][j][k], context->num_cells[level]);

              exchange_boundary_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                  FACE, context->num_blocks[level], context->num_cells[level],
                  &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
                  &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
                  &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);

              apply_boundary_condition_internal(&u[out_exchange][segment][level][i][j][k], i, j, k,
                  context->num_blocks[level], context->num_cells[level]);


            }
          }
        }

        for(k = 0; k < NUM_BLOCKS; k++) {
          for(j = 0; j < NUM_BLOCKS; j++) {
            for(i = 0; i < NUM_BLOCKS; i++) {

              int in_smooth_1 = 	smooth_cycle * 2;
              int in_smooth_2 = 	smooth_cycle * 2 + 2;
              int out_smooth = 	smooth_cycle * 2 + 3;

              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("Running smooth <%d, %d, %d, %d, %d, %d, %d>\n", smooth_cycle, cycle, 1, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t<- u<%d, %d, %d, %d, %d, %d, %d>\n",
                  in_smooth_1, (smooth_cycle == 0) ? 0 : cycle, (smooth_cycle == 0) ? 0 : 1, (smooth_cycle == 0) ? 0: level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   u<%d, %d, %d, %d, %d, %d, %d>\n", in_smooth_2, cycle, 1, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   f<%d, %d, %d, %d, %d>\n", (level == 0) ? 0 : cycle, level, i, j, k);
              if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t-> u<%d, %d, %d, %d, %d, %d, %d>\n", out_smooth, cycle, 1, level, i, j, k);

              // in smooth 1 is empty when smooth cycle == 0, we don't need it
              // TODO: F is not consistent with other smooth step - fixed if we refactor <segment> tags to <cycle, segment>
              block f_in = level == 0 ? fs[0][level][i][j][k] : fs[segment - 1][level][i][j][k];
              block u_1_in = smooth_cycle == 0 ? u[in_smooth_1][0][0][i][j][k] : u[in_smooth_1][segment][level][i][j][k];
              block u_2_in = u[in_smooth_2][segment][level][i][j][k];

              smooth(&u[out_smooth][segment][level][i][j][k], &u_1_in, &u_2_in,
                  &betas[level][i][j][k], &dinvs[level][i][j][k],
                  &f_in, context->num_blocks[level], context->num_cells[level],
                  context->dominant_eigenvalue_of_DinvA[level], smooth_cycle);

              if(segment % 2 == 1 && level == 0 && smooth_cycle == context->num_smooth_cycles - 1) {
                if(i == 0 && j == 0 && k == 0 && debug_graph) printf("\t   u<%d, %d, %d, %d, %d, %d, %d>\n", 1, cycle + 1, 0, level, i, j, k);
                copy(	&u[1][segment+1][level][i][j][k],
                    &u[out_smooth][segment][level][i][j][k], context->num_cells[level]);
              }
            }
          }
        }	// end ijk
      } // end smooth
    } // end level
  } // end cycle


  // Exchange u
  // Compute the residual
  TYPE norm_of_residual = 0.0;
  for(k = 0; k < NUM_BLOCKS; k++) {
    for(j = 0; j < NUM_BLOCKS; j++) {
      for(i = 0; i < NUM_BLOCKS; i++) {

        segment = context->num_cycles * 2;
        level = 0;
        block residual_output;
        block u_exchanged;

        block n[27];
        int i_, j_, k_;
        int local_id = 0;
        for(k_ = k - 1; k_ <= k + 1; k_++) {
          for(j_ = j - 1; j_ <= j + 1; j_++) {
            for(i_ = i - 1; i_ <= i + 1; i_++) {
              if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < context->num_blocks[level] &&
                  j_ < context->num_blocks[level] && k_ < context->num_blocks[level]) {
                n[local_id] = u[1][segment][level][i_][j_][k_];
              } else {
                n[local_id] = u[1][segment][level][i][j][k]; // Self
              }
              local_id++;
            }
          }
        }

        copy(	&u_exchanged,
            &u[1][segment][level][i][j][k], context->num_cells[level]);

        exchange_boundary_internal(&u_exchanged, i, j, k,
            FACE, context->num_blocks[level], context->num_cells[level],
            &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
            &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
            &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);
        apply_boundary_condition_internal(&u_exchanged, i, j, k, context->num_blocks[level], context->num_cells[level]);

        residual(&residual_output, &fs[0][0][i][j][k], &u_exchanged, &betas[level][i][j][k],
            context->num_blocks[level], context->num_cells[level]);

        TYPE norm_of_residual_block = norm(&residual_output, context->num_cells[level]);
        if(norm_of_residual_block > norm_of_residual)
          norm_of_residual = norm_of_residual_block;
      }
    }
  }

  printf("Normal of residual %.17g\n", norm_of_residual);
  printf("norm_of_f: %.17g\n", norm_of_f);
  printf("norm_of_dinvf: %.17g\n", norm_of_dinvf);
  printf("%.17g\n", norm_of_residual/norm_of_f);
  printf("%.17g\n", norm_of_residual/norm_of_dinvf);

}

void print3(block3* target, int num_cells) {
  int i, j, k;
  for(k = 0; k < num_cells+2*GHOSTS; k++) {
    for(j = num_cells+2*GHOSTS-1; j >= 0; j--) {
      for(i = 0; i < num_cells+2*GHOSTS; i++) {
        printf("(%.17g, %.17g, %.17g)\t", target->x[i][j][k], target->y[i][j][k], target->z[i][j][k]);
      }
      printf("\n");
    }
    printf("\n\n");
  }
  printf("\n\n\n");
}

void print(block* target, int num_cells) {
  int i, j, k;
  for(k = 0; k < num_cells+2*GHOSTS; k++) {
    for(j = num_cells+2*GHOSTS-1; j >= 0; j--) {
      for(i = 0; i < num_cells+2*GHOSTS; i++) {
        printf("%.17g\t", target->x[i][j][k]);
      }
      printf("\n");
    }
    printf("\n\n");
  }
  printf("\n\n\n");
}

void copy(block* to, block* from, int num_cells) {
  int i, j, k;
  for(k = 0; k < num_cells+2*GHOSTS; k++) {
    for(j = num_cells+2*GHOSTS-1; j >= 0; j--) {
      for(i = 0; i < num_cells+2*GHOSTS; i++) {
        to->x[i][j][k] = from->x[i][j][k];
      }
    }
  }
}

void iterative_solve(block u[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block r0_id[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block dinv[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block3 beta[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS], int num_blocks, int num_cells) {

  double a=0.0;
  double b=1.0; // Poisson

  int i, j, k;
  int cell_i, cell_j, cell_k;

  int jMax=200;
  int j_=0;
  int BiCGStabFailed    = 0;
  int BiCGStabConverged = 0;

  double desired_reduction_in_norm = 1e-3;
  double r_dot_r0 = 0.0;
  double Ap_dot_r0 = 0.0;
  double norm_of_r0 = 0.0;
  double norm_of_s = 0.0;
  double norm_of_r = 0.0;
  double As_dot_As = 0.0;
  double As_dot_s  = 0.0;
  double r_dot_r0_new = 0.0;

  block r_id[num_blocks][num_blocks][num_blocks];
  block p_id[num_blocks][num_blocks][num_blocks];
  block q_id[num_blocks][num_blocks][num_blocks];
  block As_id[num_blocks][num_blocks][num_blocks];
  block Ap_id[num_blocks][num_blocks][num_blocks];
  block x_id[num_blocks][num_blocks][num_blocks];
  block s_id[num_blocks][num_blocks][num_blocks];
  block t_id[num_blocks][num_blocks][num_blocks];
  block temp[num_blocks][num_blocks][num_blocks];

  // Initialize to 0
  for(k = 0; k < num_blocks; k++) {
    for(j = 0; j < num_blocks; j++) {
      for(i = 0; i < num_blocks; i++) {
        for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
          for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
            for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
              u[i][j][k].x[cell_i][cell_j][cell_k] = 0.0;
              // Copy f
            }
          }
        }
      }
    }
  }

  for(k = 0; k < num_blocks; k++) {
    for(j = 0; j < num_blocks; j++) {
      for(i = 0; i < num_blocks; i++) {

        scale_vector(&r_id[i][j][k], &r0_id[i][j][k], 1.0, num_cells); // r[] = r0[]
        scale_vector(&p_id[i][j][k], &r0_id[i][j][k], 1.0, num_cells); // p[] = r0[]

        r_dot_r0 += dot(&r_id[i][j][k], &r0_id[i][j][k], num_cells);     // the norm of the initial residual...
        double norm_of_r0_block = norm(&r_id[i][j][k], num_cells);      // r_dot_r0 = dot(r,r0)
        if(norm_of_r0_block > norm_of_r0)
          norm_of_r0 = norm_of_r0_block;
        if(r_dot_r0   == 0.0){BiCGStabConverged=1;}                    // entered BiCGStab with exact solution
        if(norm_of_r0 == 0.0){BiCGStabConverged=1;}                    // entered BiCGStab with exact solution
      }
    }
  }


  while( (j_<jMax) && (!BiCGStabFailed) && (!BiCGStabConverged) ) {               // while(not done){
    j_++;

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          mul_vectors(&q_id[i][j][k], &dinv[i][j][k], &p_id[i][j][k], 1.0, num_cells);//   q[] = Dinv[]*p[]
        }
      }
    }

    // apply_op(level,Ap_id,q_id,a,b);                                             //   Ap[] = AM^{-1}(p)
    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          block n[27];
          int i_, j_, k_;
          int local_id = 0;
          for(k_ = k - 1; k_ <= k + 1; k_++) {
            for(j_ = j - 1; j_ <= j + 1; j_++) {
              for(i_ = i - 1; i_ <= i + 1; i_++) {
                if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < num_blocks &&
                    j_ < num_blocks && k_ < num_blocks) {
                  n[local_id] = q_id[i_][j_][k_];
                } else {
                  n[local_id] = q_id[i][j][k]; // Self
                }
                local_id++;
              }
            }
          }
          exchange_boundary_internal(&q_id[i][j][k], i, j, k, FACE, num_blocks, num_cells,
              &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
              &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
              &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);
          apply_boundary_condition_internal(&q_id[i][j][k], i, j, k, num_blocks, num_cells);
        }
      }
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {

          double h_level = 1.0/((double)num_blocks*(double)num_cells);
          double h2inv = 1.0/(h_level*h_level);

          for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
            for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
              for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {
                Ap_id[i][j][k].x[cell_i][cell_j][cell_k] = -b*h2inv*(
                    + beta[i][j][k].x[cell_i+1][cell_j][cell_k]	*( q_id[i][j][k].x[cell_i+1][cell_j][cell_k] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].x[cell_i][cell_j][cell_k]	*( q_id[i][j][k].x[cell_i-1][cell_j][cell_k] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].y[cell_i][cell_j+1][cell_k] *( q_id[i][j][k].x[cell_i][cell_j+1][cell_k] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].y[cell_i][cell_j][cell_k]	*( q_id[i][j][k].x[cell_i][cell_j-1][cell_k] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].z[cell_i][cell_j][cell_k+1]	*( q_id[i][j][k].x[cell_i][cell_j][cell_k+1] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].z[cell_i][cell_j][cell_k]	*( q_id[i][j][k].x[cell_i][cell_j][cell_k-1] - q_id[i][j][k].x[cell_i][cell_j][cell_k] )
                );
              }
            }
          }
        }
      }
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          Ap_dot_r0 += dot(&Ap_id[i][j][k], &r0_id[i][j][k], num_cells);	//   Ap_dot_r0 = dot(Ap,r0)
          if(Ap_dot_r0 == 0.0){											//   pivot breakdown ???
            BiCGStabFailed=1;
            printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
            return;
          }
        }
      }
    }

    double alpha = r_dot_r0 / Ap_dot_r0;                                        //   alpha = r_dot_r0 / Ap_dot_r0

    if(isinf(alpha)){
      BiCGStabFailed = 2;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      return;
    }                                   										//   pivot breakdown ???

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          add_vectors(&u[i][j][k], &u[i][j][k], &q_id[i][j][k], 1.0, alpha, num_cells);  //   x_id[] = x_id[] + alpha*q[]
          add_vectors(&s_id[i][j][k], &r_id[i][j][k], &Ap_id[i][j][k], 1.0, -alpha, num_cells); //  s[] = r[]   - alpha*Ap[]
          double norm_of_s_block = norm(&s_id[i][j][k], num_cells); 		 	// r_dot_r0 = dot(r,r0)
          if(norm_of_s_block > norm_of_s)
            norm_of_s = norm_of_s_block;
        }
      }
    }

    if(norm_of_s == 0.0){			//   FIX - redundant??  if As_dot_As==0, then As must be 0 which implies s==0
      BiCGStabConverged = 1;
      return;
    }

    if(norm_of_s < desired_reduction_in_norm * norm_of_r0){
      BiCGStabConverged=1;
      return;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          mul_vectors(&t_id[i][j][k], &dinv[i][j][k], &s_id[i][j][k], 1.0, num_cells); //   t[] = Dinv[]*s[]
        }
      }
    }

    // apply_op(level,As_id,t_id,a,b);                                             //   As = AM^{-1}(s)
    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          block n[27];
          int i_, j_, k_;
          int local_id = 0;
          for(k_ = k - 1; k_ <= k + 1; k_++) {
            for(j_ = j - 1; j_ <= j + 1; j_++) {
              for(i_ = i - 1; i_ <= i + 1; i_++) {
                if(i_ >= 0 && j_ >= 0 && k_ >= 0 && i_ < num_blocks &&
                    j_ < num_blocks && k_ < num_blocks) {
                  n[local_id] = t_id[i_][j_][k_];
                } else {
                  n[local_id] = t_id[i][j][k]; // Self
                }
                local_id++;
              }
            }
          }
          exchange_boundary_internal(&t_id[i][j][k], i, j, k, FACE, num_blocks, num_cells,
              &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9],
              &n[10], &n[11], &n[12], &n[14], &n[15], &n[16], &n[17], &n[18], &n[19],
              &n[20], &n[21], &n[22], &n[23], &n[24], &n[25], &n[26]);
          apply_boundary_condition_internal(&t_id[i][j][k], i, j, k, num_blocks, num_cells);
        }
      }
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {

          double h_level = 1.0/((double)num_blocks*(double)num_cells);
          double h2inv = 1.0/(h_level*h_level);

          for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
            for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
              for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {
                As_id[i][j][k].x[cell_i][cell_j][cell_k] = -b*h2inv*(
                    + beta[i][j][k].x[cell_i+1][cell_j][cell_k]	*( t_id[i][j][k].x[cell_i+1][cell_j][cell_k] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].x[cell_i][cell_j][cell_k]	*( t_id[i][j][k].x[cell_i-1][cell_j][cell_k] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].y[cell_i][cell_j+1][cell_k] *( t_id[i][j][k].x[cell_i][cell_j+1][cell_k] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].y[cell_i][cell_j][cell_k]	*( t_id[i][j][k].x[cell_i][cell_j-1][cell_k] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].z[cell_i][cell_j][cell_k+1]	*( t_id[i][j][k].x[cell_i][cell_j][cell_k+1] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                    + beta[i][j][k].z[cell_i][cell_j][cell_k]	*( t_id[i][j][k].x[cell_i][cell_j][cell_k-1] - t_id[i][j][k].x[cell_i][cell_j][cell_k] )
                );
              }
            }
          }
        }
      }
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          As_dot_As += dot(&As_id[i][j][k], &As_id[i][j][k], num_cells);
          As_dot_s  += dot(&As_id[i][j][k], &s_id[i][j][k], num_cells);
        }
      }
    }

    if(As_dot_As == 0.0) { //   converged ?
      BiCGStabConverged=1;
      return;
    }

    double omega = As_dot_s / As_dot_As;                                        //   omega = As_dot_s / As_dot_As
    if(omega == 0.0){
      BiCGStabFailed=3;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      return;
    }                                   //   stabilization breakdown ???
    if(isinf(omega)){
      BiCGStabFailed=4;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }                                   //   stabilization breakdown ???

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          add_vectors(&x_id[i][j][k], &x_id[i][j][k], &t_id[i][j][k], 1.0, omega, num_cells);  //   x_id[] = x_id[] + alpha*q[]
          add_vectors(&r_id[i][j][k], &s_id[i][j][k], &As_id[i][j][k], 1.0, -omega, num_cells);  //   s[]    = r[]    - alpha*Ap[]   (intermediate residual?)
          double norm_of_r_block = norm(&r_id[i][j][k], num_cells); 		 	// r_dot_r0 = dot(r,r0)
          if(norm_of_r_block > norm_of_r)
            norm_of_r = norm_of_r_block;
        }
      }
    }

    if(norm_of_r == 0.0){
      BiCGStabConverged=1;
      return;
    }
    if(norm_of_r < desired_reduction_in_norm*norm_of_r0){
      BiCGStabConverged=1;
      break;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          double r_dot_r0_new_block = dot(&r_id[i][j][k], &r0_id[i][j][k], num_cells);     //   r_dot_r0_new = dot(r,r0)
          if(r_dot_r0_new < r_dot_r0_new_block)
            r_dot_r0_new = r_dot_r0_new_block;
        }
      }
    }

    //   Lanczos breakdown ???
    if(r_dot_r0_new == 0.0){
      BiCGStabFailed=5;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      return;
    }

    //   ???
    double beta_ = (r_dot_r0_new/r_dot_r0) * (alpha/omega);                      //   beta = (r_dot_r0_new/r_dot_r0) * (alpha/omega)
    if(isinf(beta_)){
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      return;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          add_vectors(&temp[i][j][k], &p_id[i][j][k], &Ap_id[i][j][k], 1.0, -omega, num_cells);  //   VECTOR_TEMP = (p[]-omega*Ap[])
          add_vectors(&p_id[i][j][k], &r_id[i][j][k], &temp[i][j][k], 1.0, beta_, num_cells);  //   p[] = r[] + beta*(p[]-omega*Ap[])
        }
      }
    }

    r_dot_r0 = r_dot_r0_new;          //   r_dot_r0 = r_dot_r0_new   (save old r_dot_r0)
    break;
  }
}

void residual(block* f, block* f_, block* u, block3* beta, int num_blocks, int num_cells) {

  int cell_i, cell_j, cell_k;

  double a=0.0;
  double b=1.0; // Poisson

  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h2inv = 1.0/(h_level*h_level);

  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        f->x[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {
        double Ax = -b*h2inv*(
            + beta->x[cell_i+1][cell_j][cell_k]	*( u->x[cell_i+1][cell_j][cell_k] - u->x[cell_i][cell_j][cell_k] )
            + beta->x[cell_i][cell_j][cell_k]	*( u->x[cell_i-1][cell_j][cell_k] - u->x[cell_i][cell_j][cell_k] )
            + beta->y[cell_i][cell_j+1][cell_k]	*( u->x[cell_i][cell_j+1][cell_k] - u->x[cell_i][cell_j][cell_k] )
            + beta->y[cell_i][cell_j][cell_k]	*( u->x[cell_i][cell_j-1][cell_k] - u->x[cell_i][cell_j][cell_k] )
            + beta->z[cell_i][cell_j][cell_k+1]	*( u->x[cell_i][cell_j][cell_k+1] - u->x[cell_i][cell_j][cell_k] )
            + beta->z[cell_i][cell_j][cell_k]	*( u->x[cell_i][cell_j][cell_k-1] - u->x[cell_i][cell_j][cell_k] )
        );
        f->x[cell_i][cell_j][cell_k] = f_->x[cell_i][cell_j][cell_k] - Ax;
      }
    }
  }
}

void smooth(block* u0, block* u0_, block* u1, block3* beta, block* dinv, block* f,
    int num_blocks, int num_cells, double dominant_eigenvalue_of_DinvA, int s) {

  double a=0.0;
  double b=1.0; // Poisson

  int cell_i, cell_j, cell_k;

  // Input
  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h2inv = 1.0/(h_level*h_level);

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // compute the Chebyshev coefficients...
  double beta_     = 1.000*dominant_eigenvalue_of_DinvA;
  //double alpha    = 0.300000*beta;
  //double alpha    = 0.250000*beta;
  //double alpha    = 0.166666*beta;
  double alpha    = 0.125000*beta_;
  double theta    = 0.5*(beta_+alpha);		// center of the spectral ellipse
  double delta    = 0.5*(beta_-alpha);		// major axis?
  double sigma = theta/delta;
  double rho_n = 1/sigma;			// rho_0
  double chebyshev_c1[CHEBYSHEV_DEGREE];	// + c1*(x_n-x_nm1) == rho_n*rho_nm1
  double chebyshev_c2[CHEBYSHEV_DEGREE];	// + c2*(b-Ax_n)
  chebyshev_c1[0] = 0.0;
  chebyshev_c2[0] = 1/theta;

  int s_ = 0;
  for(s_=1;s_<CHEBYSHEV_DEGREE;s_++){
    double rho_nm1 = rho_n;
    rho_n = 1.0/(2.0*sigma - rho_nm1);
    chebyshev_c1[s_] = rho_n*rho_nm1;
    chebyshev_c2[s_] = rho_n*2.0/delta;
  }

  for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {

        double lambda = dinv->x[cell_i][cell_j][cell_k];
        double c1 = chebyshev_c1[s%CHEBYSHEV_DEGREE]; // limit polynomial to degree CHEBYSHEV_DEGREE.
        double c2 = chebyshev_c2[s%CHEBYSHEV_DEGREE]; // limit polynomial to degree CHEBYSHEV_DEGREE.
        double rhs = f->x[cell_i][cell_j][cell_k];

        double Ax_n = -b*h2inv*(
            + beta->x[cell_i+1][cell_j][cell_k]	*( u1->x[cell_i+1][cell_j][cell_k] - u1->x[cell_i][cell_j][cell_k] )
            + beta->x[cell_i][cell_j][cell_k]	*( u1->x[cell_i-1][cell_j][cell_k] - u1->x[cell_i][cell_j][cell_k] )
            + beta->y[cell_i][cell_j+1][cell_k]	*( u1->x[cell_i][cell_j+1][cell_k] - u1->x[cell_i][cell_j][cell_k] )
            + beta->y[cell_i][cell_j][cell_k]	*( u1->x[cell_i][cell_j-1][cell_k] - u1->x[cell_i][cell_j][cell_k] )
            + beta->z[cell_i][cell_j][cell_k+1]	*( u1->x[cell_i][cell_j][cell_k+1] - u1->x[cell_i][cell_j][cell_k] )
            + beta->z[cell_i][cell_j][cell_k]	*( u1->x[cell_i][cell_j][cell_k-1] - u1->x[cell_i][cell_j][cell_k] )
        );
        double x_n = u1->x[cell_i][cell_j][cell_k];
        double x_nm1 = u0_->x[cell_i][cell_j][cell_k];

        u0->x[cell_i][cell_j][cell_k] = x_n + c1*(x_n-x_nm1) + c2*lambda*(rhs-Ax_n);
      }
    }
  }
}

void restrict_blocks_internal3(block3* target, int target_i, int target_j,
    int target_k, int num_cells, block3* n0, block3* n1, block3*n2,
    block3* n3, block3* n4, block3* n5, block3* n6, block3* n7) {


  if(target_i % 2 != 0 || target_j & 2 != 0 || target_k %2 != 0) {
    return; // Nothing to do
  }

  block3 neighbors[2][2][2];
  int i, j, k, cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  // Restrict the cells
  restrict_cells_internal3(&neighbors[0][0][0], n0, num_cells/2);
  restrict_cells_internal3(&neighbors[1][0][0], n1, num_cells/2);
  restrict_cells_internal3(&neighbors[0][1][0], n2, num_cells/2);
  restrict_cells_internal3(&neighbors[1][1][0], n3, num_cells/2);
  restrict_cells_internal3(&neighbors[0][0][1], n4, num_cells/2);
  restrict_cells_internal3(&neighbors[1][0][1], n5, num_cells/2);
  restrict_cells_internal3(&neighbors[0][1][1], n6, num_cells/2);
  restrict_cells_internal3(&neighbors[1][1][1], n7, num_cells/2);

  // Initialize to 0
  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        target->x[cell_i][cell_j][cell_k] = 0.0;
        target->y[cell_i][cell_j][cell_k] = 0.0;
        target->z[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  // Move the data into our output block
  for(k = 0; k < 2; k++) {
    for(j = 0; j < 2; j++) {
      for(i = 0; i < 2; i++) {
        for(cell_k = GHOSTS; cell_k < num_cells/2+GHOSTS*2; cell_k++) {
          for(cell_j = GHOSTS; cell_j < num_cells/2+GHOSTS*2; cell_j++) {
            for(cell_i = GHOSTS; cell_i < num_cells/2+GHOSTS*2; cell_i++) {
              cell_i_ = cell_i + i * (num_cells/2);
              cell_j_ = cell_j + j * (num_cells/2);
              cell_k_ = cell_k + k * (num_cells/2);
              target->x[cell_i_][cell_j_][cell_k_] = neighbors[i][j][k].x[cell_i][cell_j][cell_k];
              target->y[cell_i_][cell_j_][cell_k_] = neighbors[i][j][k].y[cell_i][cell_j][cell_k];
              target->z[cell_i_][cell_j_][cell_k_] = neighbors[i][j][k].z[cell_i][cell_j][cell_k];
            }
          }
        }
      }
    }
  }
}

void restrict_blocks_internal(block* target, int target_i, int target_j,
    int target_k, int num_cells, block* n0, block* n1, block*n2,
    block* n3, block* n4, block* n5, block* n6, block* n7) {


  if(target_i % 2 != 0 || target_j & 2 != 0 || target_k %2 != 0) {
    return; // Nothing to do
  }

  block neighbors[2][2][2];
  int i, j, k, cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  for(cell_i = GHOSTS; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = GHOSTS; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = GHOSTS; cell_k < num_cells+2*GHOSTS; cell_k++) {
        target->x[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  // Restrict the cells
  restrict_cells_internal(&neighbors[0][0][0], n0, num_cells);
  restrict_cells_internal(&neighbors[1][0][0], n1, num_cells);
  restrict_cells_internal(&neighbors[0][1][0], n2, num_cells);
  restrict_cells_internal(&neighbors[1][1][0], n3, num_cells);
  restrict_cells_internal(&neighbors[0][0][1], n4, num_cells);
  restrict_cells_internal(&neighbors[1][0][1], n5, num_cells);
  restrict_cells_internal(&neighbors[0][1][1], n6, num_cells);
  restrict_cells_internal(&neighbors[1][1][1], n7, num_cells);

  // Move the data into our output block
  for(k = 0; k < 2; k++) {
    for(j = 0; j < 2; j++) {
      for(i = 0; i < 2; i++) {
        for(cell_k = GHOSTS; cell_k < num_cells/2+GHOSTS*2; cell_k++) {
          for(cell_j = GHOSTS; cell_j < num_cells/2+GHOSTS*2; cell_j++) {
            for(cell_i = GHOSTS; cell_i < num_cells/2+GHOSTS*2; cell_i++) {
              cell_i_ = cell_i + i * (num_cells/2);
              cell_j_ = cell_j + j * (num_cells/2);
              cell_k_ = cell_k + k * (num_cells/2);
              target->x[cell_i_][cell_j_][cell_k_] = neighbors[i][j][k].x[cell_i][cell_j][cell_k];
            }
          }
        }
      }
    }
  }
}

void restrict_cells_internal3(block3* to, block3* from, int num_cells) {

  int cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        to->x[cell_i][cell_j][cell_k] = 0.0;
        to->y[cell_i][cell_j][cell_k] = 0.0;
        to->z[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS; cell_i <= num_cells+GHOSTS; cell_i++, cell_i_+=2) {

        to->x[cell_i][cell_j][cell_k] =	(
            from->x[cell_i_][cell_j_][cell_k_] +
            from->x[cell_i_][cell_j_+1][cell_k_] +
            from->x[cell_i_][cell_j_][cell_k_+1] +
            from->x[cell_i_][cell_j_+1][cell_k_+1]) * 0.25;

      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS; cell_j <= num_cells+GHOSTS; cell_j++, cell_j_+=2) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {

        to->y[cell_i][cell_j][cell_k] =	(
            from->y[cell_i_][cell_j_][cell_k_] +
            from->y[cell_i_+1][cell_j_][cell_k_] +
            from->y[cell_i_][cell_j_][cell_k_+1] +
            from->y[cell_i_+1][cell_j_][cell_k_+1]) * 0.25;

      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS; cell_k <= num_cells+GHOSTS; cell_k++, cell_k_+=2) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {

        to->z[cell_i][cell_j][cell_k] =	(
            from->z[cell_i_][cell_j_][cell_k_] +
            from->z[cell_i_][cell_j_+1][cell_k_] +
            from->z[cell_i_+1][cell_j_][cell_k_] +
            from->z[cell_i_+1][cell_j_+1][cell_k_]) * 0.25;
      }
    }
  }
}


void interpolate_blocks_internal(block* previous_level, int num_cells,
    block* no0, block* no1, block* no2, block* no3, block* no4, block* no5, block* no6, block* no7,
    block* ns0, block* ns1, block* ns2, block* ns3, block* ns4, block* ns5, block* ns6, block* ns7) {

  block* no[8] = { no0, no1, no2, no3, no4, no5, no6, no7 };
  block* ns[8] = { ns0, ns1, ns2, ns3, ns4, ns5, ns6, ns7 };

  int block_i, block_j, block_k, i_, j_, k_, cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  int n_id = 0;
  for(k_ = 0; k_ < 2; k_++) {
    for(j_ = 0; j_ < 2; j_++) {
      for(i_ = 0; i_ < 2; i_++) {
        for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
          for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
            for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
              no[n_id]->x[cell_i][cell_j][cell_k] = ns[n_id]->x[cell_i][cell_j][cell_k];
            }
          }
        }
        for(cell_k = GHOSTS, cell_k_ = GHOSTS + (k_ * num_cells/2); cell_k < num_cells+GHOSTS; cell_k++, cell_k_ += (cell_k%2 ? 1: 0)) {
          for(cell_j = GHOSTS, cell_j_ = GHOSTS + (j_ * num_cells/2); cell_j < num_cells+GHOSTS; cell_j++, cell_j_ += (cell_j%2 ? 1: 0)) {
            for(cell_i = GHOSTS, cell_i_ = GHOSTS + (i_ * num_cells/2); cell_i < num_cells+GHOSTS; cell_i++, cell_i_ += (cell_i%2 ? 1: 0)) {
              no[n_id]->x[cell_i][cell_j][cell_k] += previous_level->x[cell_i_][cell_j_][cell_k_];
            }
          }
        }
        n_id++;
      }
    }
  }
}


void interpolate_cells_internal(block* to, block* prev_level, block* prev_segment, int num_cells) {

  int block_i, block_j, block_k, i_, j_, k_, cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;
  for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
        to->x[cell_i][cell_j][cell_k] = prev_segment->x[cell_i][cell_j][cell_k];
      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++, cell_k_ += (cell_k%2 ? 1: 0)) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++, cell_j_ += (cell_j%2 ? 1: 0)) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++, cell_i_ += (cell_i%2 ? 1: 0)) {
        to->x[cell_i][cell_j][cell_k] += prev_level->x[cell_i_][cell_j_][cell_k_];
      }
    }
  }
}


void restrict_cells_internal(block* to, block* from, int num_cells) {
  int cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        to->x[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {
        to->x[cell_i][cell_j][cell_k] =
            (		from->x[cell_i_][cell_j_][cell_k_] +
                from->x[cell_i_+1][cell_j_][cell_k_] +
                from->x[cell_i_][cell_j_+1][cell_k_] +
                from->x[cell_i_+1][cell_j_+1][cell_k_] +
                from->x[cell_i_][cell_j_][cell_k_+1] +
                from->x[cell_i_+1][cell_j_][cell_k_+1] +
                from->x[cell_i_][cell_j_+1][cell_k_+1] +
                from->x[cell_i_+1][cell_j_+1][cell_k_+1]) * 0.125;
      }
    }
  }
}

void initialize_block(block3* beta, block* alpha, block* u_true, block* f,
    int target_i, int target_j, int target_k, int num_blocks, int num_cells) {

  int cell_i, cell_j, cell_k;
  int box_i = target_i * num_cells;
  int box_j = target_j * num_cells;
  int box_k = target_k * num_cells;

  double h0 = 1.0/((double)num_blocks*(double)num_cells);
  double a = 0.0;
  double b = 1.0; // Poisson

  // Initialize to 0
  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        beta->x[cell_i][cell_j][cell_k] = 0.0;
        beta->y[cell_i][cell_j][cell_k] = 0.0;
        beta->z[cell_i][cell_j][cell_k] = 0.0;
        alpha->x[cell_i][cell_j][cell_k] = 0.0;
        u_true->x[cell_i][cell_j][cell_k] = 0.0;
        f->x[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  // Include high face
  for(cell_i = 0; cell_i <= num_cells; cell_i++) {
    for(cell_j = 0; cell_j <= num_cells; cell_j++) {
      for(cell_k = 0; cell_k <= num_cells; cell_k++) {
        double x = h0*(box_i + cell_i + 0.5);
        double y = h0*(box_j + cell_j + 0.5);
        double z = h0*(box_k + cell_k + 0.5);

        double A,B,Bx,By,Bz,Bi,Bj,Bk;
        double U,Ux,Uy,Uz,Uxx,Uyy,Uzz;

        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        A  = 1.0;
        B  = 1.0;
        Bx = 0.0;
        By = 0.0;
        Bz = 0.0;
        Bi = 1.0;
        Bj = 1.0;
        Bk = 1.0;
        evaluate_beta(x-h0*0.5  ,y         ,z         ,&Bi,&Bx,&By,&Bz); // face-centered value of Beta for beta_i
        evaluate_beta(x         ,y-h0*0.5  ,z         ,&Bj,&Bx,&By,&Bz); // face-centered value of Beta for beta_j
        evaluate_beta(x         ,y         ,z-h0*0.5  ,&Bk,&Bx,&By,&Bz); // face-centered value of Beta for beta_k
        evaluate_beta(x         ,y         ,z         ,&B ,&Bx,&By,&Bz); // cell-centered value of Beta
        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        evaluate_u(x,y,z,&U,&Ux,&Uy,&Uz,&Uxx,&Uyy,&Uzz,0);// (level->boundary_condition.type == BC_PERIODIC) );
        double F = a*A*U - b*( (Bx*Ux + By*Uy + Bz*Uz)  +  B*(Uxx + Uyy + Uzz) );

        // Offset by the ghosts
        beta->x[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bi;
        beta->y[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bj;
        beta->z[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bk;
        alpha->x[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = A;
        u_true->x[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = U;
        f->x[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = F;

      }
    }
  }

}

double rebuild_operator(block* dinv, block3* beta, block* alpha, int target_i,
    int target_j, int target_k, int num_blocks, int num_cells) {

  int cell_i, cell_j, cell_k;
  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h_level2inv = 1.0/(h_level*h_level);
  double block_eigenvalue = -1e9;

  // Poisson
  double a=0.0;
  double b=1.0;

  // Initialize to 0
  for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
        dinv->x[cell_i][cell_j][cell_k] = 0.0;
      }
    }
  }

  for(cell_k = 0; cell_k < num_cells; cell_k++) {
    for(cell_j = 0; cell_j < num_cells; cell_j++) {
      for(cell_i = 0; cell_i < num_cells; cell_i++) {

        TYPE c0alpha = alpha->x[cell_i+1][cell_j+1][cell_k+1];
        TYPE c0x = beta->x[cell_i+1][cell_j+1][cell_k+1];
        TYPE c0y = beta->y[cell_i+1][cell_j+1][cell_k+1];
        TYPE c0z = beta->z[cell_i+1][cell_j+1][cell_k+1];
        TYPE c1x = beta->x[cell_i+2][cell_j+1][cell_k+1];
        TYPE c2y = beta->y[cell_i+1][cell_j+2][cell_k+1];
        TYPE c3z = beta->z[cell_i+1][cell_j+1][cell_k+2];

        double valid_minus_1x = 1.0;
        double valid_minus_1y = 1.0;
        double valid_minus_1z = 1.0;
        double valid_plus_1x = 1.0;
        double valid_plus_1y = 1.0;
        double valid_plus_1z = 1.0;

        // Lower bounds
        if(target_i == 0 && cell_i == 0)
          valid_minus_1x = 0.0;
        if(target_j== 0 && cell_j == 0)
          valid_minus_1y = 0.0;
        if(target_k == 0 && cell_k == 0)
          valid_minus_1z = 0.0;

        // Upper bounds
        if(target_i == num_blocks - 1 && cell_i == num_cells - 1)
          valid_plus_1x = 0.0;
        if(target_j == num_blocks - 1 && cell_j == num_cells - 1)
          valid_plus_1y = 0.0;
        if(target_k == num_blocks - 1 && cell_k == num_cells - 1)
          valid_plus_1z = 0.0;

        double sumAbsAij = fabs(b*h_level2inv) * (
            fabs(c0x*valid_minus_1x )+
            fabs(c0y*valid_minus_1y )+
            fabs(c0z*valid_minus_1z )+
            fabs(c1x*valid_plus_1x )+
            fabs(c2y*valid_plus_1y )+
            fabs(c3z*valid_plus_1z )
        );

        double    Aii = a*c0alpha - b*h_level2inv*(
            c0x*(valid_minus_1x-2.0 )+
            c0y*(valid_minus_1y-2.0 )+
            c0z*(valid_minus_1z-2.0 )+
            c1x*(valid_plus_1x-2.0 )+
            c2y*(valid_plus_1y-2.0 )+
            c3z*(valid_plus_1z-2.0 )
        );

        double dinv_ = 1.0/Aii;
        /* Not used
				double l1inv = 0.0;
				if(Aii>=1.5*sumAbsAij)
					l1inv = 1.0/(Aii); 		// as suggested by eq 6.5 in Baker et al, "Multigrid smoothers for ultra-parallel computing: additional theory and discussion"...
				else
					l1inv = 1.0/(Aii+0.5*sumAbsAij);		//
         */
        dinv->x[cell_i+1][cell_j+1][cell_k+1] = dinv_; // inverse of the diagonal Aii
        // l1inv->x[cell_i+1][cell_j+1][cell_k+1] = l1inv; // <-- not used

        double Di = (Aii + sumAbsAij)/Aii;
        if(Di>block_eigenvalue)
          block_eigenvalue=Di;	// upper limit to Gershgorin disc == bound on dominant eigenvalue
      }
    }
  }

  return block_eigenvalue;
}

void exchange_boundary_internal(block* target, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells,
    block* n0, block* n1, block* n2, block* n3, block* n4, block* n5, block* n6,
    block* n7, block* n8, block* n9, block* n10, block* n11, block* n12, block* n14,
    block* n15, block* n16, block* n17, block* n18, block* n19, block* n20, block* n21,
    block* n22, block* n23, block* n24, block* n25, block* n26) {

  // 26 potential neighbors:

  //   24  25  26
  //  21  22  23
  // 18  19  20

  //   15  16  17
  //  12   X  14
  // 9   10  11

  //   6   7   8
  //  3   4   5
  // 0   1   2

  // X is target

  int has_neighbor[27];
  int i, j, k, i_, j_, k_, _i, _j, _k, local_id;

  local_id = 0;
  for(k = target_k - 1; k <= target_k + 1; k++) {
    for(j = target_j - 1; j <= target_j + 1; j++) {
      for(i = target_i - 1; i <= target_i + 1; i++) {
        // If we have a neighbor
        if(		// Edge cases
            i >= 0 && j >= 0 && k >= 0 &&
            i < num_blocks && j < num_blocks && k < num_blocks &&
            // Self
            local_id != 13) {
          has_neighbor[local_id] = i + (num_blocks * j) + (num_blocks * num_blocks * k);
        } else {
          has_neighbor[local_id] = -1;
        }
        local_id++;
      }
    }
  }

  // Corners [0, 2, 6, 8, 18, 20, 24, 26]
  int min = GHOSTS;
  int max = num_cells;
  int ghost_min = 0;
  int ghost_max = min + max;

  if(boundary & CORNER) {
    // Corner 0
    if(has_neighbor[0] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n0->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 2
    if(has_neighbor[2] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n2->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 6
    if(has_neighbor[6] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n6->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 8
    if(has_neighbor[8] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n8->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 18
    if(has_neighbor[18] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n18->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 20
    if(has_neighbor[20] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n20->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 24
    if(has_neighbor[24] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n24->x[_i][_j][_k];
          }
        }
      }
    }

    // Corner 26
    if(has_neighbor[26] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n26->x[_i][_j][_k];
          }
        }
      }
    }
  }

  if(boundary & EDGE) {
    // Edges [1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25]
    // Edge 1
    if(has_neighbor[1] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n1->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 3
    if(has_neighbor[3] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n3->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 5
    if(has_neighbor[5] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n5->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 7
    if(has_neighbor[7] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n7->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 19
    if(has_neighbor[19] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n19->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 21
    if(has_neighbor[21] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n21->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 23
    if(has_neighbor[23] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n23->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 25
    if(has_neighbor[25] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n25->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 9
    if(has_neighbor[9] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n9->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 11
    if(has_neighbor[11] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n11->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 15
    if(has_neighbor[15] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n15->x[_i][_j][_k];
          }
        }
      }
    }

    // Edge 17
    if(has_neighbor[17] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n17->x[_i][_j][_k];
          }
        }
      }
    }

  }

  if(boundary & FACE) {
    // Faces [4, 10, 12, 14, 16, 22]
    // Face 4
    if(has_neighbor[4] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n4->x[_i][_j][_k];
          }
        }
      }
    }

    // Face 10
    if(has_neighbor[10] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n10->x[_i][_j][_k];
          }
        }
      }
    }

    // Face 12
    if(has_neighbor[12] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n12->x[_i][_j][_k];
          }
        }
      }
    }

    // Face 14
    if(has_neighbor[14] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n14->x[_i][_j][_k];
          }
        }
      }
    }


    // Face 16
    if(has_neighbor[16] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n16->x[_i][_j][_k];
          }
        }
      }
    }

    // Face 22
    if(has_neighbor[22] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n22->x[_i][_j][_k];
          }
        }
      }
    }
  }
}

void exchange_boundary_internal3(block3* target, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells,
    block3* n0, block3* n1, block3* n2, block3* n3, block3* n4, block3* n5, block3* n6,
    block3* n7, block3* n8, block3* n9, block3* n10, block3* n11, block3* n12, block3* n14,
    block3* n15, block3* n16, block3* n17, block3* n18, block3* n19, block3* n20, block3* n21,
    block3* n22, block3* n23, block3* n24, block3* n25, block3* n26) {

  // 26 potential neighbors:

  //   24  25  26
  //  21  22  23
  // 18  19  20

  //   15  16  17
  //  12   X  14
  // 9   10  11

  //   6   7   8
  //  3   4   5
  // 0   1   2

  // X is target

  int has_neighbor[27];
  int i, j, k, i_, j_, k_, _i, _j, _k, local_id;

  local_id = 0;
  for(k = target_k - 1; k <= target_k + 1; k++) {
    for(j = target_j - 1; j <= target_j + 1; j++) {
      for(i = target_i - 1; i <= target_i + 1; i++) {
        // If we have a neighbor
        if(		// Edge cases
            i >= 0 && j >= 0 && k >= 0 &&
            i < num_blocks && j < num_blocks && k < num_blocks &&
            // Self
            local_id != 13) {
          has_neighbor[local_id] = i + (num_blocks * j) + (num_blocks * num_blocks * k);
        } else {
          has_neighbor[local_id] = -1;
        }
        local_id++;
      }
    }
  }

  // Corners [0, 2, 6, 8, 18, 20, 24, 26]
  int min = GHOSTS;
  int max = num_cells;
  int ghost_min = 0;
  int ghost_max = min + max;

  if(boundary & CORNER) {
    // Corner 0
    if(has_neighbor[0] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n0->x[_i][_j][_k];
            target->y[i_][j_][k_] = n0->y[_i][_j][_k];
            target->z[i_][j_][k_] = n0->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 2
    if(has_neighbor[2] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n2->x[_i][_j][_k];
            target->y[i_][j_][k_] = n2->y[_i][_j][_k];
            target->z[i_][j_][k_] = n2->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 6
    if(has_neighbor[6] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n6->x[_i][_j][_k];
            target->y[i_][j_][k_] = n6->y[_i][_j][_k];
            target->z[i_][j_][k_] = n6->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 8
    if(has_neighbor[8] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n8->x[_i][_j][_k];
            target->y[i_][j_][k_] = n8->y[_i][_j][_k];
            target->z[i_][j_][k_] = n8->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 18
    if(has_neighbor[18] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n18->x[_i][_j][_k];
            target->y[i_][j_][k_] = n18->y[_i][_j][_k];
            target->z[i_][j_][k_] = n18->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 20
    if(has_neighbor[20] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n20->x[_i][_j][_k];
            target->y[i_][j_][k_] = n20->y[_i][_j][_k];
            target->z[i_][j_][k_] = n20->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 24
    if(has_neighbor[24] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n24->x[_i][_j][_k];
            target->y[i_][j_][k_] = n24->y[_i][_j][_k];
            target->z[i_][j_][k_] = n24->z[_i][_j][_k];
          }
        }
      }
    }

    // Corner 26
    if(has_neighbor[26] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n26->x[_i][_j][_k];
            target->y[i_][j_][k_] = n26->y[_i][_j][_k];
            target->z[i_][j_][k_] = n26->z[_i][_j][_k];
          }
        }
      }
    }
  }

  if(boundary & EDGE) {
    // Edges [1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25]
    // Edge 1
    if(has_neighbor[1] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n1->x[_i][_j][_k];
            target->y[i_][j_][k_] = n1->y[_i][_j][_k];
            target->z[i_][j_][k_] = n1->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 3
    if(has_neighbor[3] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n3->x[_i][_j][_k];
            target->y[i_][j_][k_] = n3->y[_i][_j][_k];
            target->z[i_][j_][k_] = n3->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 5
    if(has_neighbor[5] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n5->x[_i][_j][_k];
            target->y[i_][j_][k_] = n5->y[_i][_j][_k];
            target->z[i_][j_][k_] = n5->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 7
    if(has_neighbor[7] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n7->x[_i][_j][_k];
            target->y[i_][j_][k_] = n7->y[_i][_j][_k];
            target->z[i_][j_][k_] = n7->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 19
    if(has_neighbor[19] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n19->x[_i][_j][_k];
            target->y[i_][j_][k_] = n19->y[_i][_j][_k];
            target->z[i_][j_][k_] = n19->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 21
    if(has_neighbor[21] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n21->x[_i][_j][_k];
            target->y[i_][j_][k_] = n21->y[_i][_j][_k];
            target->z[i_][j_][k_] = n21->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 23
    if(has_neighbor[23] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n23->x[_i][_j][_k];
            target->y[i_][j_][k_] = n23->y[_i][_j][_k];
            target->z[i_][j_][k_] = n23->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 25
    if(has_neighbor[25] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n25->x[_i][_j][_k];
            target->y[i_][j_][k_] = n25->y[_i][_j][_k];
            target->z[i_][j_][k_] = n25->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 9
    if(has_neighbor[9] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n9->x[_i][_j][_k];
            target->y[i_][j_][k_] = n9->y[_i][_j][_k];
            target->z[i_][j_][k_] = n9->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 11
    if(has_neighbor[11] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n11->x[_i][_j][_k];
            target->y[i_][j_][k_] = n11->y[_i][_j][_k];
            target->z[i_][j_][k_] = n11->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 15
    if(has_neighbor[15] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n15->x[_i][_j][_k];
            target->y[i_][j_][k_] = n15->y[_i][_j][_k];
            target->z[i_][j_][k_] = n15->z[_i][_j][_k];
          }
        }
      }
    }

    // Edge 17
    if(has_neighbor[17] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n17->x[_i][_j][_k];
            target->y[i_][j_][k_] = n17->y[_i][_j][_k];
            target->z[i_][j_][k_] = n17->z[_i][_j][_k];
          }
        }
      }
    }

  }

  if(boundary & FACE) {
    // Faces [4, 10, 12, 14, 16, 22]
    // Face 4
    if(has_neighbor[4] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n4->x[_i][_j][_k];
            target->y[i_][j_][k_] = n4->y[_i][_j][_k];
            target->z[i_][j_][k_] = n4->z[_i][_j][_k];
          }
        }
      }
    }

    // Face 10
    if(has_neighbor[10] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n10->x[_i][_j][_k];
            target->y[i_][j_][k_] = n10->y[_i][_j][_k];
            target->z[i_][j_][k_] = n10->z[_i][_j][_k];
          }
        }
      }
    }

    // Face 12
    if(has_neighbor[12] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n12->x[_i][_j][_k];
            target->y[i_][j_][k_] = n12->y[_i][_j][_k];
            target->z[i_][j_][k_] = n12->z[_i][_j][_k];
          }
        }
      }
    }

    // Face 14
    if(has_neighbor[14] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n14->x[_i][_j][_k];
            target->y[i_][j_][k_] = n14->y[_i][_j][_k];
            target->z[i_][j_][k_] = n14->z[_i][_j][_k];
          }
        }
      }
    }


    // Face 16
    if(has_neighbor[16] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n16->x[_i][_j][_k];
            target->y[i_][j_][k_] = n16->y[_i][_j][_k];
            target->z[i_][j_][k_] = n16->z[_i][_j][_k];
          }
        }
      }
    }

    // Face 22
    if(has_neighbor[22] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target->x[i_][j_][k_] = n22->x[_i][_j][_k];
            target->y[i_][j_][k_] = n22->y[_i][_j][_k];
            target->z[i_][j_][k_] = n22->z[_i][_j][_k];
          }
        }
      }
    }
  }
}


void apply_boundary_condition_internal(block* target, int target_i, int target_j,
    int target_k, int num_blocks, int num_cells) {

  int cell_i, cell_j, cell_k, ghost;

  for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {

        // Left edge
        if(target_i == 0 && cell_i == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target->x[cell_i-(GHOSTS-ghost)][cell_j][cell_k] = -target->x[cell_i+ghost][cell_j][cell_k];
          }
        }

        // Right edge
        if(target_i == num_blocks - 1 && cell_i == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target->x[cell_i+ghost][cell_j][cell_k] = -target->x[cell_i-(GHOSTS-ghost)][cell_j][cell_k];
          }
        }

        // Front
        if(target_j == 0 && cell_j == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target->x[cell_i][cell_j-(GHOSTS-ghost)][cell_k] = -target->x[cell_i][cell_j+ghost][cell_k];
          }
        }

        // Back
        if(target_j == num_blocks - 1 && cell_j == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target->x[cell_i][cell_j+ghost][cell_k] = -target->x[cell_i][cell_j-(GHOSTS-ghost)][cell_k];
          }
        }
        // Bottom
        if(target_k == 0 && cell_k == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target->x[cell_i][cell_j][cell_k-(GHOSTS-ghost)] = -target->x[cell_i][cell_j][cell_k+ghost];
          }
        }

        // Top
        if(target_k == num_blocks - 1 && cell_k == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target->x[cell_i][cell_j][cell_k+ghost] = -target->x[cell_i][cell_j][cell_k-(GHOSTS-ghost)];
          }
        }
      }
    }
  }
}


void evaluate_beta(double x, double y, double z, double *B, double *Bx, double *By, double *Bz){
  double Bmin =  1.0;
  double Bmax = 10.0;
  double c2 = (Bmax-Bmin)/2; // coefficients to affect this transition
  double c1 = (Bmax+Bmin)/2;
  double c3 = 10.0;          // how sharply (B)eta transitions
  double xcenter = 0.50;
  double ycenter = 0.50;
  double zcenter = 0.50;
  // calculate distance from center of the domain (0.5,0.5,0.5)
  double r2   = pow((x-xcenter),2) +  pow((y-ycenter),2) +  pow((z-zcenter),2);
  double r2x  = 2.0*(x-xcenter);
  double r2y  = 2.0*(y-ycenter);
  double r2z  = 2.0*(z-zcenter);
  //double r2xx = 2.0;
  //double r2yy = 2.0;
  //double r2zz = 2.0;
  double r    = pow(r2,0.5);
  double rx   = 0.5*r2x*pow(r2,-0.5);
  double ry   = 0.5*r2y*pow(r2,-0.5);
  double rz   = 0.5*r2z*pow(r2,-0.5);
  //double rxx  = 0.5*r2xx*pow(r2,-0.5) - 0.25*r2x*r2x*pow(r2,-1.5);
  //double ryy  = 0.5*r2yy*pow(r2,-0.5) - 0.25*r2y*r2y*pow(r2,-1.5);
  //double rzz  = 0.5*r2zz*pow(r2,-0.5) - 0.25*r2z*r2z*pow(r2,-1.5);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  *B  =           c1+c2*tanh( c3*(r-0.25) );
  *Bx = c2*c3*rx*(1-pow(tanh( c3*(r-0.25) ),2));
  *By = c2*c3*ry*(1-pow(tanh( c3*(r-0.25) ),2));
  *Bz = c2*c3*rz*(1-pow(tanh( c3*(r-0.25) ),2));

}

double pow(double x, double power) {
  if(power == 2.0)
    return x*x;
  if(power == 3.0)
    return x*x*x;
  if(power == 4.0)
    return x*x*x*x;
  if(power == 5.0)
    return x*x*x*x*x;
  if(power == 6.0)
    return x*x*x*x*x*x;
  if(power == 0.5)
    return sqrt(x);
  if(power == -0.5)
    return 1.0/sqrt(x);
  printf("ERROR: could not compute power: %f\n", power);
}

double sqrt(double x) {
  double j, i=0;
  double x1,x2;
  while( (i*i) <= x) i+=0.1f;
  x1=i;
  for(j=0;j<9;j++) {
    x2=x;
    x2/=x1;
    x2+=x1;
    x2/=2;
    x1=x2;
  }
  return x2;
}

double tanh(double x) {
  int i, percision = 99;
  double z = x;
  double z2 = x*x;
  x = percision + 2;
  for(i = percision; i > 0; i-=2) {
    x = i + z2 / x;
  }
  x = z / x;
  return x;
}

//------------------------------------------------------------------------------------------------------------------------------
void evaluate_u(double x, double y, double z, double *U, double *Ux, double *Uy, double *Uz, double *Uxx, double *Uyy, double *Uzz, int isPeriodic){
  // should be continuous in u, u', u'', u''', and u'''' to guarantee high order and periodic boundaries
  // v(w) = ???
  // u(x,y,z) = v(x)v(y)v(z)
  // If Periodic, then the integral of the RHS should sum to zero.
  //   Setting shift=1.0 should ensure that the integrals of X, Y, or Z should sum to zero...
  //   That should(?) make the integrals of u,ux,uy,uz,uxx,uyy,uzz sum to zero and thus make the integral of f sum to zero
  // If dirichlet, then w(0)=w(1) = 0.0
  //   Setting shift to 0 should ensure that U(x,y,z) = 0 on boundary
  //    u =    ax^6 +    bx^5 +   cx^4 +  dx^3 +  ex^2 + fx + g
  //   ux =   6ax^5 +   5bx^4 +  4cx^3 + 3dx^2 + 2ex   + f
  //  uxx =  30ax^4 +  20bx^3 + 12cx^2 + 6dx   + 2e
  // a =   42.0
  // b = -126.0
  // c =  105.0
  // d =    0.0
  // e =  -21.0
  // f =    0.0
  // g =    1.0
  double shift = 0.0;if(isPeriodic)shift= 1.0/21.0;
  double X     =  2.0*pow(x,6) -   6.0*pow(x,5) +  5.0*pow(x,4) - 1.0*pow(x,2) + shift;
  double Y     =  2.0*pow(y,6) -   6.0*pow(y,5) +  5.0*pow(y,4) - 1.0*pow(y,2) + shift;
  double Z     =  2.0*pow(z,6) -   6.0*pow(z,5) +  5.0*pow(z,4) - 1.0*pow(z,2) + shift;
  double Xx    = 12.0*pow(x,5) -  30.0*pow(x,4) + 20.0*pow(x,3) - 2.0*x;
  double Yy    = 12.0*pow(y,5) -  30.0*pow(y,4) + 20.0*pow(y,3) - 2.0*y;
  double Zz    = 12.0*pow(z,5) -  30.0*pow(z,4) + 20.0*pow(z,3) - 2.0*z;
  double Xxx   = 60.0*pow(x,4) - 120.0*pow(x,3) + 60.0*pow(x,2) - 2.0;
  double Yyy   = 60.0*pow(y,4) - 120.0*pow(y,3) + 60.0*pow(y,2) - 2.0;
  double Zzz   = 60.0*pow(z,4) - 120.0*pow(z,3) + 60.0*pow(z,2) - 2.0;
  *U     = X   * Y   * Z;
  *Ux    = Xx  * Y   * Z;
  *Uy    = X   * Yy  * Z;
  *Uz    = X   * Y   * Zz;
  *Uxx   = Xxx * Y   * Z;
  *Uyy   = X   * Yyy * Z;
  *Uzz   = X   * Y   * Zzz;
}

void scale_vector(block* to, block* from, TYPE scale, int num_cells) {
  int cell_i, cell_j, cell_k;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {

        to->x[cell_i][cell_j][cell_k] = scale * from->x[cell_i][cell_j][cell_k];

      }
    }
  }
}

void add_vectors(block* to, block* from_a, block* from_b, TYPE scale_a, TYPE scale_b, int num_cells) {
  int cell_i, cell_j, cell_k;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {

        TYPE value_a = scale_a * from_a->x[cell_i][cell_j][cell_k];
        TYPE value_b = scale_b * from_b->x[cell_i][cell_j][cell_k];

        to->x[cell_i][cell_j][cell_k] = value_a + value_b;

      }
    }
  }
}

void mul_vectors(block* to, block* from_a, block* from_b, TYPE scale, int num_cells) {
  int cell_i, cell_j, cell_k;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {

        TYPE value_a = scale * from_a->x[cell_i][cell_j][cell_k];
        TYPE value_b = scale * from_b->x[cell_i][cell_j][cell_k];

        to->x[cell_i][cell_j][cell_k] = value_a * value_b;
      }
    }
  }
}

TYPE dot(block* to, block* from, int num_cells) {
  int cell_i, cell_j, cell_k;
  TYPE dot_product = 0.0;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        TYPE from_ = from->x[cell_i][cell_j][cell_k];
        TYPE to_ = to->x[cell_i][cell_j][cell_k];
        dot_product += to_ * from_;
      }
    }
  }
  return dot_product;
}

TYPE norm(block* to, int num_cells) {
  int cell_i, cell_j, cell_k;
  TYPE max = 0.0;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        TYPE value = to->x[cell_i][cell_j][cell_k];
        if(value < 0)
          value *= -1;
        if(value > max)
          max = value;
      }
    }
  }
  return max;
}
