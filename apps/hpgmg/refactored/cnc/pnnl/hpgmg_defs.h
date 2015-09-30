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

#ifndef _CNCOCR_HPGMG_TYPES_H_
#define _CNCOCR_HPGMG_TYPES_H_

// Per dimension

// NUM_BLOCKS = 2, NUM_CELLS = 16, NUM_LEVELS = 6
// NUM_BLOCKS = 2, NUM_CELLS = 32, NUM_LEVELS = 7
// NUM_BLOCKS = 2, NUM_CELLS = 64, NUM_LEVELS = 8
// NUM_BLOCKS = 4, NUM_CELLS = 64, NUM_LEVELS = 9
// NUM_BLOCKS = 4, NUM_CELLS = 128, NUM_LEVELS = 10 ...

#define NUM_BLOCKS 2 // NUM_BLOCKS must be square
#define NUM_CELLS 16 // NUM_CELLS must be square

#define NUM_LEVELS 6 // Should drop NUM_CELLS to 1 by bottom level
                     // cells restrict by 2 each level until blocks^3 = cells
                     // at that point the blocks will restrict instead
#define NUM_CYCLES 10

#define GHOSTS 1

// Smoother options
#define CHEBYSHEV_DEGREE 4 // i.e. one degree-4 polynomial smoother
#define NUM_SMOOTHS 1

// Boundary to exchange
#define FACE 1
#define EDGE 2
#define CORNER 4

// For restricting faces
#define NONE 8
#define FACE_X 1
#define FACE_Y 2
#define FACE_Z 4

typedef struct hpgmgArguments {
  /* TODO: Add struct members.
   * Use this struct to pass all arguments for
   * graph initialization. This should not contain any
   * pointers (unless you know you'll only be executing
   * in shared memory and thus passing pointers is safe).
   */
} hpgmgArgs;

#endif /*_CNCOCR_HPGMG_TYPES_H_*/

// Math functions -- TODO: Replace these with a math library
double sqrt(double x);
double tanh(double x);
double fabs(double x);
int isinf(double x);

// Helper functions
int block_size(int i, int j, int k);
void initialize_block(double* block_, int cells_i, int cells_j, int cells_k);
void copy(double* to, double* from, int num_cells);
void print(double* target, int num_cells);


// From HPGMG
void initialize(double* beta_x, double* beta_y, double* beta_z,
    double* alpha, double* u_true, double* f, int target_i,
    int target_j, int target_k, int num_blocks, int num_cells);

void evaluate_beta(double x, double y, double z, double *B, double *Bx,
    double *By, double *Bz);

void evaluate_u(double x, double y, double z, double *U, double *Ux, double *Uy,
    double *Uz, double *Uxx, double *Uyy, double *Uzz, int isPeriodic);

double rebuild_operator(double* dinv, double* beta_x, double* beta_y,
    double* beta_z, double* alpha, int target_i, int target_j, int target_k,
    int num_blocks, int num_cells);

void restrict_cells(double* to, double* from, int num_cells, int face);

void restrict_blocks(double* target, int target_i, int target_j, int target_k,
    int num_cells, int face, double* n0, double* n1, double* n2, double* n3,
    double* n4, double* n5, double* n6, double* n7);

void interpolate_blocks(double* to, int num_cells, int i, int j, int k,
    double* previous_level, double* previous_segment);

void interpolate_cells(double* to, double* prev_level, double* prev_segment,
    int num_cells);

void apply_boundary_condition(double* target, int target_i, int target_j,
    int target_k, int num_blocks, int num_cells);

void exchange_boundary(double* target, int target_i, int target_j, int target_k,
    int boundary, int num_blocks, int num_cells, double* n0, double* n1,
    double* n2, double* n3, double* n4, double* n5, double* n6, double* n7,
    double* n8, double* n9, double* n10, double* n11, double* n12, double* n14,
    double* n15, double* n16, double* n17, double* n18, double* n19,
    double* n20, double* n21, double* n22, double* n23, double* n24,
    double* n25,double* n26);

void exchange_faces(double* target, int target_i, int target_j, int target_k,
    int boundary, int num_blocks, int num_cells, double* n4, double* n10,
    double* n12, double* n14, double* n16, double* n22);

void smooth(double* u0, double* u0_, double* u1, double* beta_x, double* beta_y,
    double* beta_z, double* dinv, double* f, int num_blocks, int num_cells,
    double dominant_eigenvalue_of_DinvA, int s);

void residual(double* u, double* f_, double* u_, double* beta_x, double* beta_y,
    double* beta_z, int num_blocks, int num_cells);

void iterative_solve(double* u[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* r0_id[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* dinv[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_x[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_y[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_z[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    int num_blocks, int num_cells);

void scale_vector(double* to, double* from, double scale, int num_cells);

void add_vectors(double* to, double* from_a, double* from_b, double scale_a,
    double scale_b, int num_cells);

void mul_vectors(double* to, double* from_a, double* from_b, double scale,
    int num_cells);

double dot(double* to, double* from, int num_cells);

double norm(double* to, int num_cells);
