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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TYPE double

// Per dimension
#define NUM_BLOCKS 2
#define NUM_CELLS 16

#define NUM_LEVELS 6
#define NUM_CYCLES 10

#define GHOSTS 1

// Smoother options
#define CHEBYSHEV_DEGREE 4 // i.e. one degree-4 polynomial smoother
#define NUM_SMOOTHS 1

// Boundary to exchange
#define FACE 1
#define EDGE 2
#define CORNER 4

typedef struct hpgmg {

  int num_levels;
  int num_cycles;
  int num_smooth_cycles;
  int num_blocks[NUM_LEVELS+1];
  int num_cells[NUM_LEVELS+1];

  double dominant_eigenvalue_of_DinvA[NUM_LEVELS+1];

} hpgmg;

typedef struct block3 {
  TYPE x[NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS];
  TYPE y[NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS];
  TYPE z[NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS];
} block3;

typedef struct block {
  TYPE x[NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS][NUM_CELLS+2*GHOSTS];
} block;

/************
 * Helper functions
 */
double pow(double x, double power);
double sqrt(double x);
double tanh(double x);

void print3(block3* target, int num_cells);
void print(block* target, int num_cells);

void copy(block* to, block* from, int num_cells);

void iterative_solve(block u[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block r0_id[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block dinv[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    block3 beta[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS], int num_blocks, int num_cells);

void residual(block* residual, block* f, block* u, block3* beta, int num_blocks, int num_cells);

void smooth(block* u0, block* u0_, block* u1, block3* beta, block* dinv, block* f,
    int num_blocks, int num_cells, double dominant_eigenvalue_of_DinvA, int s);

void restrict_blocks_internal3(block3* target, int target_i, int target_j,
    int target_k, int num_cells, block3* n0, block3* n1, block3*n2,
    block3* n3, block3* n4, block3* n5, block3* n6, block3* n7);

void restrict_blocks_internal(block* target, int target_i, int target_j,
    int target_k, int num_cells, block* n0, block* n1, block*n2,
    block* n3, block* n4, block* n5, block* n6, block* n7);

void interpolate_blocks_internal(block* previous_level, int num_cells,
    block* no0, block* no1, block* no2, block* no3, block* no4, block* no5, block* no6, block* no7,
    block* ns0, block* ns1, block* ns2, block* ns3, block* ns4, block* ns5, block* ns6, block* ns7);

void interpolate_cells_internal(block* to, block* prev_level, block* prev_segment, int num_cells);

void restrict_cells_internal(block* to, block* from, int num_cells);

void restrict_cells_internal3(block3* to, block3* from, int num_cells);

double rebuild_operator(block* dinv, block3* beta, block* alpha, int target_i,
    int target_j, int target_k, int num_blocks, int num_cells);

void initialize_block(block3* beta, block* alpha, block* u_true, block* f,
    int target_i, int target_j, int target_k, int num_blocks, int num_cells);

void exchange_boundary_internal3(block3* target, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells,
    block3* n0, block3* n1, block3* n2, block3* n3, block3* n4, block3* n5, block3* n6,
    block3* n7, block3* n8, block3* n9, block3* n10, block3* n11, block3* n12, block3* n14,
    block3* n15, block3* n16, block3* n17, block3* n18, block3* n19, block3* n20, block3* n21,
    block3* n22, block3* n23, block3* n24, block3* n25, block3* n26);

void exchange_boundary_internal(block* target, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells,
    block* n0, block* n1, block* n2, block* n3, block* n4, block* n5, block* n6,
    block* n7, block* n8, block* n9, block* n10, block* n11, block* n12, block* n14,
    block* n15, block* n16, block* n17, block* n18, block* n19, block* n20, block* n21,
    block* n22, block* n23, block* n24, block* n25, block* n26);

void apply_boundary_condition_internal(block* target, int target_i, int target_j,
    int target_k, int num_blocks, int num_cells);

void evaluate_beta(double x, double y, double z, double *B, double *Bx,
    double *By, double *Bz);
void evaluate_u(double x, double y, double z, double *U, double *Ux, double *Uy,
    double *Uz, double *Uxx, double *Uyy, double *Uzz, int isPeriodic);

void scale_vector(block* to, block* from, TYPE scale, int num_cells);

void add_vectors(block* to, block* from_a, block* from_b, TYPE scale_a, TYPE scale_b, int num_cells);

void mul_vectors(block* to, block* from_a, block* from_b, TYPE scale, int num_cells);

TYPE dot(block* to, block* from, int num_cells);

TYPE norm(block* to, int num_cells);
