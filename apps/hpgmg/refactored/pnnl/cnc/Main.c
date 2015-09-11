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

int cncMain(int argc, char *argv[]) {

  // Create a new graph context
  hpgmgCtx *context = hpgmg_create();
  hpgmgArgs *args = NULL;

  context->num_blocks = NUM_BLOCKS;
  context->num_cells = NUM_CELLS;
  context->num_levels = NUM_LEVELS;
  context->num_cycles = NUM_CYCLES;
  context->num_smooth_cycles = CHEBYSHEV_DEGREE * NUM_SMOOTHS;
  if((CHEBYSHEV_DEGREE*NUM_SMOOTHS)&1){
    printf("error... CHEBYSHEV_DEGREE*NUM_SMOOTHS must be even\n");
    CNC_ABORT(1);
  }

  int level;
  int num_blocks = NUM_BLOCKS;
  int num_cells = NUM_CELLS;

  for(level = 0; level < context->num_levels; level++) {
    context->restrict_blocks[level] = 0;
  }

  for(level = 0; level < context->num_levels; level++) {
    context->num_blocks_per_level[level] = num_blocks;
    context->num_cells_per_level[level] = num_cells;
    context->dominant_eigenvalue_of_DinvA[level] = -1e9;
    if(num_cells == num_blocks*num_blocks*num_blocks) {
      context->restrict_blocks[level] = num_cells == 1 ? 0 : 1;
      num_cells *= 2;
      num_blocks /= 2;
    }
    num_cells /= 2;
    if(num_blocks == 0) num_blocks = 1;
    if(num_cells == 0) num_cells = 1;
  }

/*
  //for debugging inputs
  for(level = 0; level < context->num_levels; level++) {
    printf("level: %d, blocks: %d, cells: %d, restrict blocks %d\n", level,
        context->num_blocks_per_level[level],
        context->num_cells_per_level[level],
        context->restrict_blocks[level]);
  }
*/
  // Launch the graph for execution
  hpgmg_launch(args, context);

  // Exit when the graph execution completes
  CNC_SHUTDOWN_ON_FINISH(context);

  return 0;
}

/*******************************************************************************
 * Math functions, TODO: Replace these with a math library
 */
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
  return -1.0;
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

double fabs(double x) {
  if(x < 0)
    return -x;
  return x;
}

int isinf(double x) {
  return 0; // probably should actually check...
}

/*******************************************************************************
 * Graph helper functions
 */
int block_size(int i, int j, int k) {
  return sizeof(double*) * (i * j * k);
}

void initialize_block(double* block_, int cells_i, int cells_j, int cells_k) {

  double (*block)[cells_j][cells_k] = (double(*)[cells_j][cells_k])block_;

  int i, j, k;
  for(k = 0; k < cells_k; k++) {
    for(j = 0; j < cells_j; j++) {
      for(i = 0; i < cells_i; i++) {
        block[i][j][k] = 0.0;
      }
    }
  }
}


void copy(double* to_, double* from_, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_;

  int i, j, k;
  for(k = 0; k < num_cells+2*GHOSTS; k++) {
    for(j = num_cells+2*GHOSTS-1; j >= 0; j--) {
      for(i = 0; i < num_cells+2*GHOSTS; i++) {
        to[i][j][k] = from[i][j][k];
      }
    }
  }
}


void print(double* target_, int num_cells) {

  double (*target)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])target_;

  int i, j, k;
  for(k = 0; k < num_cells+2*GHOSTS; k++) {
    for(j = num_cells+2*GHOSTS-1; j >= 0; j--) {
      for(i = 0; i < num_cells+2*GHOSTS; i++) {
        printf("%.17g\t", target[i][j][k]);
      }
      printf("\n");
    }
    printf("\n\n");
  }
  printf("\n\n\n");
}

/******************************************************************************
 * HPGMG functions
 */
void initialize(double* beta_x_, double* beta_y_, double* beta_z_,
    double* alpha_, double*  u_true_, double* f_, int target_i, int target_j,
    int target_k, int num_blocks, int num_cells) {

  double (*beta_x)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_x_;
  double (*beta_y)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_y_;
  double (*beta_z)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_z_;
  double (*alpha)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])alpha_;
  double (*u_true)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u_true_;
  double (*f)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])f_;

  int cell_i, cell_j, cell_k;
  int box_i = target_i * num_cells;
  int box_j = target_j * num_cells;
  int box_k = target_k * num_cells;

  double h0 = 1.0/((double)num_blocks*(double)num_cells);
  double a = 0.0;
  double b = 1.0; // Poisson

  // Include high face
  for(cell_i = 0; cell_i <= num_cells; cell_i++) {
    for(cell_j = 0; cell_j <= num_cells; cell_j++) {
      for(cell_k = 0; cell_k <= num_cells; cell_k++) {
        double x = h0*(box_i + cell_i + 0.5);
        double y = h0*(box_j + cell_j + 0.5);
        double z = h0*(box_k + cell_k + 0.5);

        double A,B,Bx,By,Bz,Bi,Bj,Bk;
        double U,Ux,Uy,Uz,Uxx,Uyy,Uzz;

        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        A  = 1.0;
        B  = 1.0;
        Bx = 0.0;
        By = 0.0;
        Bz = 0.0;
        Bi = 1.0;
        Bj = 1.0;
        Bk = 1.0;
        // face-centered value of Beta for beta_i
        evaluate_beta(x-h0*0.5  ,y         ,z         ,&Bi,&Bx,&By,&Bz);
        // face-centered value of Beta for beta_j
        evaluate_beta(x         ,y-h0*0.5  ,z         ,&Bj,&Bx,&By,&Bz);
        // face-centered value of Beta for beta_k
        evaluate_beta(x         ,y         ,z-h0*0.5  ,&Bk,&Bx,&By,&Bz);
        // cell-centered value of Beta
        evaluate_beta(x         ,y         ,z         ,&B ,&Bx,&By,&Bz);
        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // (level->boundary_condition.type == BC_PERIODIC) );
        evaluate_u(x,y,z,&U,&Ux,&Uy,&Uz,&Uxx,&Uyy,&Uzz,0);
        double F = a*A*U - b*((Bx*Ux + By*Uy + Bz*Uz) + B*(Uxx + Uyy + Uzz));

        // Offset by the ghosts
        beta_x[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bi;
        beta_y[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bj;
        beta_z[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = Bk;
        alpha[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = A;
        u_true[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = U;
        f[cell_i+GHOSTS][cell_j+GHOSTS][cell_k+GHOSTS] = F;
      }
    }
  }
}

void evaluate_beta(double x, double y, double z, double *B, double *Bx,
    double *By, double *Bz){

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
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  *B  =           c1+c2*tanh( c3*(r-0.25) );
  *Bx = c2*c3*rx*(1-pow(tanh( c3*(r-0.25) ),2));
  *By = c2*c3*ry*(1-pow(tanh( c3*(r-0.25) ),2));
  *Bz = c2*c3*rz*(1-pow(tanh( c3*(r-0.25) ),2));
}

void evaluate_u(double x, double y, double z, double *U, double *Ux, double *Uy,
    double *Uz, double *Uxx, double *Uyy, double *Uzz, int isPeriodic){
  // should be continuous in u, u', u'', u''', and u'''' to guarantee high order
  // and periodic boundaries
  // v(w) = ???
  // u(x,y,z) = v(x)v(y)v(z)
  // If Periodic, then the integral of the RHS should sum to zero.
  //   Setting shift=1.0 should ensure that the integrals of X, Y, or Z should
  //   sum to zero...
  //   That should(?) make the integrals of u,ux,uy,uz,uxx,uyy,uzz sum to zero
  //   and thus make the integral of f sum to zero
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
  double X     =  2.0*pow(x,6)-  6.0*pow(x,5)+ 5.0*pow(x,4)-1.0*pow(x,2)+shift;
  double Y     =  2.0*pow(y,6)-  6.0*pow(y,5)+ 5.0*pow(y,4)-1.0*pow(y,2)+shift;
  double Z     =  2.0*pow(z,6)-  6.0*pow(z,5)+ 5.0*pow(z,4)-1.0*pow(z,2)+shift;
  double Xx    = 12.0*pow(x,5)- 30.0*pow(x,4)+20.0*pow(x,3)-2.0*x;
  double Yy    = 12.0*pow(y,5)- 30.0*pow(y,4)+20.0*pow(y,3)-2.0*y;
  double Zz    = 12.0*pow(z,5)- 30.0*pow(z,4)+20.0*pow(z,3)-2.0*z;
  double Xxx   = 60.0*pow(x,4)-120.0*pow(x,3)+60.0*pow(x,2)-2.0;
  double Yyy   = 60.0*pow(y,4)-120.0*pow(y,3)+60.0*pow(y,2)-2.0;
  double Zzz   = 60.0*pow(z,4)-120.0*pow(z,3)+60.0*pow(z,2)-2.0;
  *U     = X   * Y   * Z;
  *Ux    = Xx  * Y   * Z;
  *Uy    = X   * Yy  * Z;
  *Uz    = X   * Y   * Zz;
  *Uxx   = Xxx * Y   * Z;
  *Uyy   = X   * Yyy * Z;
  *Uzz   = X   * Y   * Zzz;
}

double rebuild_operator(double* dinv_, double* beta_x_, double* beta_y_,
    double* beta_z_, double* alpha_, int target_i, int target_j, int target_k,
    int num_blocks, int num_cells) {

  double (*dinv)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])dinv_;
  double (*beta_x)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_x_;
  double (*beta_y)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_y_;
  double (*beta_z)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_z_;
  double (*alpha)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])alpha_;

  int cell_i, cell_j, cell_k;
  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h_level2inv = 1.0/(h_level*h_level);
  double block_eigenvalue = -1e9;

  // Poisson
  double a=0.0;
  double b=1.0;

  for(cell_k = 0; cell_k < num_cells; cell_k++) {
    for(cell_j = 0; cell_j < num_cells; cell_j++) {
      for(cell_i = 0; cell_i < num_cells; cell_i++) {

        double c0alpha = alpha[cell_i+1][cell_j+1][cell_k+1];
        double c0x = beta_x[cell_i+1][cell_j+1][cell_k+1];
        double c0y = beta_y[cell_i+1][cell_j+1][cell_k+1];
        double c0z = beta_z[cell_i+1][cell_j+1][cell_k+1];
        double c1x = beta_x[cell_i+2][cell_j+1][cell_k+1];
        double c2y = beta_y[cell_i+1][cell_j+2][cell_k+1];
        double c3z = beta_z[cell_i+1][cell_j+1][cell_k+2];

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
            l1inv = 1.0/(Aii); // as suggested by eq 6.5 in Baker et al,
                               // "Multigrid smoothers for ultra-parallel
                               // computing: additional theory and discussion"
          else
            l1inv = 1.0/(Aii+0.5*sumAbsAij);		//
        */
        // inverse of the diagonal Aii
        dinv[cell_i+1][cell_j+1][cell_k+1] = dinv_;
        // l1inv[cell_i+1][cell_j+1][cell_k+1] = l1inv; // <-- not used
        double Di = (Aii + sumAbsAij)/Aii;
        // upper limit to Gershgorin disc == bound on dominant eigenvalue
        if(Di>block_eigenvalue)
          block_eigenvalue=Di;
      }
    }
  }

  return block_eigenvalue;
}

void restrict_blocks(double* target_, int target_i, int target_j, int target_k,
    int num_cells, int face, double* n0, double* n1, double* n2, double* n3,
    double* n4, double* n5, double* n6, double* n7) {

  if((target_i % 2) != 0 || (target_j & 2) != 0 || (target_k % 2) != 0) {
    return; // Nothing to do
  }

  int i, j, k, cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  int cells = (num_cells/2)+2*GHOSTS;
  double* neighbors[2][2][2];
  int sizeof_block = block_size(cells, cells, cells);

  for(k = 0; k < 2; k++) {
    for(j = 0; j < 2; j++) {
      for(i = 0; i < 2; i++) {
        neighbors[i][j][k] = cncLocalAlloc(sizeof_block);
        initialize_block(neighbors[i][j][k], cells, cells, cells);
      }
    }
  }

  restrict_cells(neighbors[0][0][0], n0, num_cells/2, face);
  restrict_cells(neighbors[1][0][0], n1, num_cells/2, face);
  restrict_cells(neighbors[0][1][0], n2, num_cells/2, face);
  restrict_cells(neighbors[1][1][0], n3, num_cells/2, face);
  restrict_cells(neighbors[0][0][1], n4, num_cells/2, face);
  restrict_cells(neighbors[1][0][1], n5, num_cells/2, face);
  restrict_cells(neighbors[0][1][1], n6, num_cells/2, face);
  restrict_cells(neighbors[1][1][1], n7, num_cells/2, face);

  // Move the data into our output block
  for(k = 0; k < 2; k++) {
    for(j = 0; j < 2; j++) {
      for(i = 0; i < 2; i++) {
        for(cell_k = GHOSTS; cell_k < num_cells/2+GHOSTS*2; cell_k++) {
          for(cell_j = GHOSTS; cell_j < num_cells/2+GHOSTS*2; cell_j++) {
            for(cell_i = GHOSTS; cell_i < num_cells/2+GHOSTS*2; cell_i++) {
              double (*target)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])target_;
              double (*neighbor)[(num_cells/2)+2*GHOSTS][(num_cells/2)+2*GHOSTS]
                  =(double(*)[(num_cells/2)+2*GHOSTS][(num_cells/2)+2*GHOSTS])
                  neighbors[i][j][k];
              cell_i_ = cell_i + i * (num_cells/2);
              cell_j_ = cell_j + j * (num_cells/2);
              cell_k_ = cell_k + k * (num_cells/2);
              target[cell_i_][cell_j_][cell_k_] =
                  neighbor[cell_i][cell_j][cell_k];
            }
          }
        }
      }
    }
  }

  for(k = 0; k < 2; k++) {
    for(j = 0; j < 2; j++) {
      for(i = 0; i < 2; i++) {
        cncLocalFree(neighbors[i][j][k]);
      }
    }
  }
}

void restrict_cells(double* to_, double* from_, int num_cells, int face) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from)[(2*num_cells)+2*GHOSTS][(2*num_cells)+2*GHOSTS] =
      (double(*)[(2*num_cells)+2*GHOSTS][(2*num_cells)+2*GHOSTS])from_;

  int cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  if(face & NONE) {
    for(cell_k = GHOSTS, cell_k_ = GHOSTS;
        cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
      for(cell_j = GHOSTS, cell_j_ = GHOSTS;
          cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
        for(cell_i = GHOSTS, cell_i_ = GHOSTS;
            cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {
          to[cell_i][cell_j][cell_k] =
              (   from[cell_i_][cell_j_][cell_k_] +
                  from[cell_i_+1][cell_j_][cell_k_] +
                  from[cell_i_][cell_j_+1][cell_k_] +
                  from[cell_i_+1][cell_j_+1][cell_k_] +
                  from[cell_i_][cell_j_][cell_k_+1] +
                  from[cell_i_+1][cell_j_][cell_k_+1] +
                  from[cell_i_][cell_j_+1][cell_k_+1] +
                  from[cell_i_+1][cell_j_+1][cell_k_+1]) * 0.125;
        }
      }
    }
  }

  if(face & FACE_X) {
    for(cell_k = GHOSTS, cell_k_ = GHOSTS;
        cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
      for(cell_j = GHOSTS, cell_j_ = GHOSTS;
          cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
        for(cell_i = GHOSTS, cell_i_ = GHOSTS;
            cell_i <= num_cells+GHOSTS; cell_i++, cell_i_+=2) {
          to[cell_i][cell_j][cell_k] = (
              from[cell_i_][cell_j_][cell_k_] +
              from[cell_i_][cell_j_+1][cell_k_] +
              from[cell_i_][cell_j_][cell_k_+1] +
              from[cell_i_][cell_j_+1][cell_k_+1]) * 0.25;
        }
      }
    }
  }

  if(face & FACE_Y) {
    for(cell_k = GHOSTS, cell_k_ = GHOSTS;
        cell_k < num_cells+GHOSTS; cell_k++, cell_k_+=2) {
      for(cell_j = GHOSTS, cell_j_ = GHOSTS;
          cell_j <= num_cells+GHOSTS; cell_j++, cell_j_+=2) {
        for(cell_i = GHOSTS, cell_i_ = GHOSTS;
            cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {
          to[cell_i][cell_j][cell_k] = (
              from[cell_i_][cell_j_][cell_k_] +
              from[cell_i_+1][cell_j_][cell_k_] +
              from[cell_i_][cell_j_][cell_k_+1] +
              from[cell_i_+1][cell_j_][cell_k_+1]) * 0.25;
        }
      }
    }
  }

  if(face & FACE_Z) {
    for(cell_k = GHOSTS, cell_k_ = GHOSTS;
        cell_k <= num_cells+GHOSTS; cell_k++, cell_k_+=2) {
      for(cell_j = GHOSTS, cell_j_ = GHOSTS;
          cell_j < num_cells+GHOSTS; cell_j++, cell_j_+=2) {
        for(cell_i = GHOSTS, cell_i_ = GHOSTS;
            cell_i < num_cells+GHOSTS; cell_i++, cell_i_+=2) {
          to[cell_i][cell_j][cell_k] =	(
              from[cell_i_][cell_j_][cell_k_] +
              from[cell_i_][cell_j_+1][cell_k_] +
              from[cell_i_+1][cell_j_][cell_k_] +
              from[cell_i_+1][cell_j_+1][cell_k_]) * 0.25;
        }
      }
    }
  }
}


void interpolate_blocks(double* to_, int num_cells, int i, int j, int k,
    double* previous_level_, double* previous_segment_) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*previous_level)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])previous_level_;
  double (*previous_segment)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])previous_segment_;

  int cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;

  for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
        to[cell_i][cell_j][cell_k] = previous_segment[cell_i][cell_j][cell_k];
      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS + ((k%2) * num_cells/2);
      cell_k < num_cells+GHOSTS; cell_k++, cell_k_ += (cell_k%2 ? 1: 0)) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS + ((j%2) * num_cells/2);
        cell_j < num_cells+GHOSTS; cell_j++, cell_j_ += (cell_j%2 ? 1: 0)) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS + ((i%2) * num_cells/2);
          cell_i < num_cells+GHOSTS; cell_i++, cell_i_ += (cell_i%2 ? 1: 0)) {
        to[cell_i][cell_j][cell_k] += previous_level[cell_i_][cell_j_][cell_k_];
      }
    }
  }
}


void interpolate_cells(double* to_, double* previous_level_,
    double* previous_segment_, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*previous_level)[(num_cells/2)+2*GHOSTS][(num_cells/2)+2*GHOSTS] =
      (double(*)[(num_cells/2)+2*GHOSTS][(num_cells/2)+2*GHOSTS])
      previous_level_;
  double (*previous_segment)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])previous_segment_;

  int cell_i, cell_j, cell_k, cell_i_, cell_j_, cell_k_;
  for(cell_k = 0; cell_k < num_cells+2*GHOSTS; cell_k++) {
    for(cell_j = 0; cell_j < num_cells+2*GHOSTS; cell_j++) {
      for(cell_i = 0; cell_i < num_cells+2*GHOSTS; cell_i++) {
        to[cell_i][cell_j][cell_k] = previous_segment[cell_i][cell_j][cell_k];
      }
    }
  }

  for(cell_k = GHOSTS, cell_k_ = GHOSTS;
      cell_k < num_cells+GHOSTS; cell_k++, cell_k_ += (cell_k%2 ? 1: 0)) {
    for(cell_j = GHOSTS, cell_j_ = GHOSTS;
        cell_j < num_cells+GHOSTS; cell_j++, cell_j_ += (cell_j%2 ? 1: 0)) {
      for(cell_i = GHOSTS, cell_i_ = GHOSTS;
          cell_i < num_cells+GHOSTS; cell_i++, cell_i_ += (cell_i%2 ? 1: 0)) {
        to[cell_i][cell_j][cell_k] += previous_level[cell_i_][cell_j_][cell_k_];
      }
    }
  }
}


void apply_boundary_condition(double* target_, int target_i, int target_j,
    int target_k, int num_blocks, int num_cells) {

  double (*target)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])target_;

  int cell_i, cell_j, cell_k, ghost;

  for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {
        // Left edge
        if(target_i == 0 && cell_i == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target[cell_i-(GHOSTS-ghost)][cell_j][cell_k] =
                -target[cell_i+ghost][cell_j][cell_k];
          }
        }
        // Right edge
        if(target_i == num_blocks - 1 && cell_i == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target[cell_i+ghost][cell_j][cell_k] =
                -target[cell_i-(GHOSTS-ghost)][cell_j][cell_k];
          }
        }
        // Front
        if(target_j == 0 && cell_j == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target[cell_i][cell_j-(GHOSTS-ghost)][cell_k] =
                -target[cell_i][cell_j+ghost][cell_k];
          }
        }
        // Back
        if(target_j == num_blocks - 1 && cell_j == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target[cell_i][cell_j+ghost][cell_k] =
                -target[cell_i][cell_j-(GHOSTS-ghost)][cell_k];
          }
        }
        // Bottom
        if(target_k == 0 && cell_k == GHOSTS) {
          for(ghost = 0; ghost < GHOSTS; ghost++) {
            target[cell_i][cell_j][cell_k-(GHOSTS-ghost)] =
                -target[cell_i][cell_j][cell_k+ghost];
          }
        }
        // Top
        if(target_k == num_blocks - 1 && cell_k == num_cells + GHOSTS - 1) {
          for(ghost = 1; ghost <= GHOSTS; ghost++) {
            target[cell_i][cell_j][cell_k+ghost] =
                -target[cell_i][cell_j][cell_k-(GHOSTS-ghost)];
          }
        }
      }
    }
  }
}


void exchange_boundary(double* target_, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells, double* n0_,
    double* n1_, double* n2_, double* n3_, double* n4_, double* n5_,
    double* n6_, double* n7_, double* n8_, double* n9_, double* n10_,
    double* n11_, double* n12_, double* n14_, double* n15_, double* n16_,
    double* n17_, double* n18_, double* n19_, double* n20_, double* n21_,
    double* n22_, double* n23_, double* n24_, double* n25_, double* n26_) {

  double (*target)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])target_;
  double (*n0)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n0_;
  double (*n1)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n1_;
  double (*n2)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n2_;
  double (*n3)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n3_;
  double (*n4)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n4_;
  double (*n5)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n5_;
  double (*n6)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n6_;
  double (*n7)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n7_;
  double (*n8)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n8_;
  double (*n9)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n9_;
  double (*n10)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n10_;
  double (*n11)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n11_;
  double (*n12)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n12_;
  double (*n14)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n14_;
  double (*n15)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n15_;
  double (*n16)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n16_;
  double (*n17)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n17_;
  double (*n18)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n18_;
  double (*n19)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n19_;
  double (*n20)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n20_;
  double (*n21)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n21_;
  double (*n22)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n22_;
  double (*n23)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n23_;
  double (*n24)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n24_;
  double (*n25)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n25_;
  double (*n26)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n26_;

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
        if( // Edge cases
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
            target[i_][j_][k_] = n0[_i][_j][_k];
          }
        }
      }
    }

    // Corner 2
    if(has_neighbor[2] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n2[_i][_j][_k];
          }
        }
      }
    }

    // Corner 6
    if(has_neighbor[6] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n6[_i][_j][_k];
          }
        }
      }
    }

    // Corner 8
    if(has_neighbor[8] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n8[_i][_j][_k];
          }
        }
      }
    }

    // Corner 18
    if(has_neighbor[18] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n18[_i][_j][_k];
          }
        }
      }
    }

    // Corner 20
    if(has_neighbor[20] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n20[_i][_j][_k];
          }
        }
      }
    }

    // Corner 24
    if(has_neighbor[24] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n24[_i][_j][_k];
          }
        }
      }
    }

    // Corner 26
    if(has_neighbor[26] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n26[_i][_j][_k];
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
            target[i_][j_][k_] = n1[_i][_j][_k];
          }
        }
      }
    }

    // Edge 3
    if(has_neighbor[3] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n3[_i][_j][_k];
          }
        }
      }
    }

    // Edge 5
    if(has_neighbor[5] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n5[_i][_j][_k];
          }
        }
      }
    }

    // Edge 7
    if(has_neighbor[7] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n7[_i][_j][_k];
          }
        }
      }
    }

    // Edge 19
    if(has_neighbor[19] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n19[_i][_j][_k];
          }
        }
      }
    }

    // Edge 21
    if(has_neighbor[21] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n21[_i][_j][_k];
          }
        }
      }
    }

    // Edge 23
    if(has_neighbor[23] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n23[_i][_j][_k];
          }
        }
      }
    }

    // Edge 25
    if(has_neighbor[25] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n25[_i][_j][_k];
          }
        }
      }
    }

    // Edge 9
    if(has_neighbor[9] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n9[_i][_j][_k];
          }
        }
      }
    }

    // Edge 11
    if(has_neighbor[11] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n11[_i][_j][_k];
          }
        }
      }
    }

    // Edge 15
    if(has_neighbor[15] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n15[_i][_j][_k];
          }
        }
      }
    }

    // Edge 17
    if(has_neighbor[17] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n17[_i][_j][_k];
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
            target[i_][j_][k_] = n4[_i][_j][_k];
          }
        }
      }
    }

    // Face 10
    if(has_neighbor[10] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n10[_i][_j][_k];
          }
        }
      }
    }

    // Face 12
    if(has_neighbor[12] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n12[_i][_j][_k];
          }
        }
      }
    }

    // Face 14
    if(has_neighbor[14] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n14[_i][_j][_k];
          }
        }
      }
    }

    // Face 16
    if(has_neighbor[16] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n16[_i][_j][_k];
          }
        }
      }
    }

    // Face 22
    if(has_neighbor[22] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n22[_i][_j][_k];
          }
        }
      }
    }
  }
}

void exchange_faces(double* target_, int target_i, int target_j,
    int target_k, int boundary, int num_blocks, int num_cells, double* n4_,
    double* n10_, double* n12_, double* n14_, double* n16_, double* n22_) {

  double (*target)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])target_;
  double (*n4)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n4_;
  double (*n10)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n10_;
  double (*n12)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n12_;
  double (*n14)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n14_;
  double (*n16)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n16_;
  double (*n22)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])n22_;


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
        if( // Edge cases
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

  if(boundary & FACE) {
    // Faces [4, 10, 12, 14, 16, 22]
    // Face 4
    if(has_neighbor[4] >= 0) {
      for(k = 0, _k = max, k_ = ghost_min; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n4[_i][_j][_k];
          }
        }
      }
    }

    // Face 10
    if(has_neighbor[10] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = max, j_ = ghost_min; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n10[_i][_j][_k];
          }
        }
      }
    }

    // Face 12
    if(has_neighbor[12] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = max, i_ = ghost_min; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n12[_i][_j][_k];
          }
        }
      }
    }

    // Face 14
    if(has_neighbor[14] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = ghost_max; i < min; i_++, _i++, i++) {
            target[i_][j_][k_] = n14[_i][_j][_k];
          }
        }
      }
    }

    // Face 16
    if(has_neighbor[16] >= 0) {
      for(k = 0, _k = min, k_ = min; k < max; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = ghost_max; j < min; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n16[_i][_j][_k];
          }
        }
      }
    }

    // Face 22
    if(has_neighbor[22] >= 0) {
      for(k = 0, _k = min, k_ = ghost_max; k < min; k_++, _k++, k++) {
        for(j = 0, _j = min, j_ = min; j < max; j_++, _j++, j++) {
          for(i = 0, _i = min, i_ = min; i < max; i_++, _i++, i++) {
            target[i_][j_][k_] = n22[_i][_j][_k];
          }
        }
      }
    }
  }
}

void smooth(double* u0__, double* u0___, double* u1_, double* beta_x_,
    double* beta_y_, double* beta_z_, double* dinv_, double* f_,
    int num_blocks, int num_cells, double dominant_eigenvalue_of_DinvA, int s) {

  double (*u0)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u0__;
  double (*u0_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u0___;
  double (*u1)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u1_;
  double (*beta_x)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_x_;
  double (*beta_y)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_y_;
  double (*beta_z)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_z_;
  double (*dinv)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])dinv_;
  double (*f)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])f_;

  // double a=0.0;
  double b=1.0; // Poisson

  int cell_i, cell_j, cell_k;

  // Input
  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h2inv = 1.0/(h_level*h_level);

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // compute the Chebyshev coefficients...
  double beta_     = 1.000*dominant_eigenvalue_of_DinvA;
  //double alpha    = 0.300000*beta;
  //double alpha    = 0.250000*beta;
  //double alpha    = 0.166666*beta;
  double alpha    = 0.125000*beta_;
  double theta    = 0.5*(beta_+alpha);  // center of the spectral ellipse
  double delta    = 0.5*(beta_-alpha);  // major axis?
  double sigma = theta/delta;
  double rho_n = 1/sigma;  // rho_0
  double chebyshev_c1[CHEBYSHEV_DEGREE]; // + c1*(x_n-x_nm1) == rho_n*rho_nm1
  double chebyshev_c2[CHEBYSHEV_DEGREE];  // + c2*(b-Ax_n)
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

        double lambda = dinv[cell_i][cell_j][cell_k];
        // limit polynomial to degree CHEBYSHEV_DEGREE.
        double c1 = chebyshev_c1[s%CHEBYSHEV_DEGREE];
        // limit polynomial to degree CHEBYSHEV_DEGREE.
        double c2 = chebyshev_c2[s%CHEBYSHEV_DEGREE];
        double rhs = f[cell_i][cell_j][cell_k];

        double Ax_n = -b*h2inv*(
            + beta_x[cell_i+1][cell_j][cell_k]*
            ( u1[cell_i+1][cell_j][cell_k] - u1[cell_i][cell_j][cell_k] )
            + beta_x[cell_i][cell_j][cell_k]*
            ( u1[cell_i-1][cell_j][cell_k] - u1[cell_i][cell_j][cell_k] )
            + beta_y[cell_i][cell_j+1][cell_k]*
            ( u1[cell_i][cell_j+1][cell_k] - u1[cell_i][cell_j][cell_k] )
            + beta_y[cell_i][cell_j][cell_k]*
            ( u1[cell_i][cell_j-1][cell_k] - u1[cell_i][cell_j][cell_k] )
            + beta_z[cell_i][cell_j][cell_k+1]*
            ( u1[cell_i][cell_j][cell_k+1] - u1[cell_i][cell_j][cell_k] )
            + beta_z[cell_i][cell_j][cell_k]*
            ( u1[cell_i][cell_j][cell_k-1] - u1[cell_i][cell_j][cell_k] )
        );
        double x_n = u1[cell_i][cell_j][cell_k];
        double x_nm1 = u0_[cell_i][cell_j][cell_k];
        u0[cell_i][cell_j][cell_k] = x_n+c1*(x_n-x_nm1)+c2*lambda*(rhs-Ax_n);
      }
    }
  }
}


void residual(double* u__, double* f__, double* u___, double* beta_x_,
    double* beta_y_, double* beta_z_, int num_blocks, int num_cells) {

  double (*u)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u__;
  double (*f_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])f__;
  double (*u_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])u___;
  double (*beta_x)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_x_;
  double (*beta_y)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_y_;
  double (*beta_z)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])beta_z_;

  int cell_i, cell_j, cell_k;

  // double a=0.0;
  double b=1.0; // Poisson

  double h_level = 1.0/((double)num_blocks*(double)num_cells);
  double h2inv = 1.0/(h_level*h_level);

  for(cell_k = GHOSTS; cell_k < num_cells+GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells+GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells+GHOSTS; cell_i++) {
        double Ax = -b*h2inv*(
            + beta_x[cell_i+1][cell_j][cell_k]*
            ( u_[cell_i+1][cell_j][cell_k] - u_[cell_i][cell_j][cell_k] )
            + beta_x[cell_i][cell_j][cell_k]*
            ( u_[cell_i-1][cell_j][cell_k] - u_[cell_i][cell_j][cell_k] )
            + beta_y[cell_i][cell_j+1][cell_k]*
            ( u_[cell_i][cell_j+1][cell_k] - u_[cell_i][cell_j][cell_k] )
            + beta_y[cell_i][cell_j][cell_k]*
            ( u_[cell_i][cell_j-1][cell_k] - u_[cell_i][cell_j][cell_k] )
            + beta_z[cell_i][cell_j][cell_k+1]*
            ( u_[cell_i][cell_j][cell_k+1] - u_[cell_i][cell_j][cell_k] )
            + beta_z[cell_i][cell_j][cell_k]*
            ( u_[cell_i][cell_j][cell_k-1] - u_[cell_i][cell_j][cell_k] )
        );
        u[cell_i][cell_j][cell_k] = f_[cell_i][cell_j][cell_k] - Ax;
      }
    }
  }
}


void iterative_solve(
    double* u[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* r0_id[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* dinv[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_x[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_y[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    double* beta_z[NUM_BLOCKS][NUM_BLOCKS][NUM_BLOCKS],
    int num_blocks, int num_cells) {

  // double a=0.0;
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

  int cells = num_cells + 2*GHOSTS;
  int sizeof_block = block_size(cells, cells, cells);

  double* r_id[num_blocks][num_blocks][num_blocks];
  double* p_id[num_blocks][num_blocks][num_blocks];
  double* q_id[num_blocks][num_blocks][num_blocks];
  double* As_id[num_blocks][num_blocks][num_blocks];
  double* Ap_id[num_blocks][num_blocks][num_blocks];
  double* x_id[num_blocks][num_blocks][num_blocks];
  double* s_id[num_blocks][num_blocks][num_blocks];
  double* t_id[num_blocks][num_blocks][num_blocks];
  double* temp[num_blocks][num_blocks][num_blocks];

  for(k = 0; k < num_blocks; k++) {
    for(j = 0; j < num_blocks; j++) {
      for(i = 0; i < num_blocks; i++) {
        r_id[i][j][k] = cncLocalAlloc(sizeof_block);
        p_id[i][j][k] = cncLocalAlloc(sizeof_block);
        q_id[i][j][k] = cncLocalAlloc(sizeof_block);
        As_id[i][j][k] = cncLocalAlloc(sizeof_block);
        Ap_id[i][j][k] = cncLocalAlloc(sizeof_block);
        x_id[i][j][k] = cncLocalAlloc(sizeof_block);
        s_id[i][j][k] = cncLocalAlloc(sizeof_block);
        t_id[i][j][k] = cncLocalAlloc(sizeof_block);
        temp[i][j][k] = cncLocalAlloc(sizeof_block);
        initialize_block(r_id[i][j][k], cells, cells, cells);
        initialize_block(p_id[i][j][k], cells, cells, cells);
        initialize_block(q_id[i][j][k], cells, cells, cells);
        initialize_block(As_id[i][j][k], cells, cells, cells);
        initialize_block(Ap_id[i][j][k], cells, cells, cells);
        initialize_block(x_id[i][j][k], cells, cells, cells);
        initialize_block(s_id[i][j][k], cells, cells, cells);
        initialize_block(t_id[i][j][k], cells, cells, cells);
        initialize_block(temp[i][j][k], cells, cells, cells);
      }
    }
  }

  for(k = 0; k < num_blocks; k++) {
    for(j = 0; j < num_blocks; j++) {
      for(i = 0; i < num_blocks; i++) {
        // r[] = r0[], p[] = r0[]
        scale_vector(r_id[i][j][k], r0_id[i][j][k], 1.0, num_cells);
        scale_vector(p_id[i][j][k], r0_id[i][j][k], 1.0, num_cells);
        // the norm of the initial residual...
        r_dot_r0 += dot(r_id[i][j][k], r0_id[i][j][k], num_cells);
        // r_dot_r0 = dot(r,r0)
        double norm_of_r0_block = norm(r_id[i][j][k], num_cells);
        if(norm_of_r0_block > norm_of_r0)
          norm_of_r0 = norm_of_r0_block;
        // entered BiCGStab with exact solution
        if(r_dot_r0   == 0.0){BiCGStabConverged=1;}
        // entered BiCGStab with exact solution
        if(norm_of_r0 == 0.0){BiCGStabConverged=1;}
      }
    }
  }

  int done = 0;
  // while(not done){
  while( (j_<jMax) && (!BiCGStabFailed) && (!BiCGStabConverged) ) {
    j_++;

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          //   q[] = Dinv[]*p[]
          mul_vectors(q_id[i][j][k],dinv[i][j][k],p_id[i][j][k],1.0,num_cells);
        }
      }
    }

    // apply_op(level,Ap_id,q_id,a,b); Ap[] = AM^{-1}(p)
    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          double* n[27];
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
          exchange_boundary(q_id[i][j][k], i, j, k, FACE, num_blocks, num_cells,
              n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9],
              n[10], n[11], n[12], n[14], n[15], n[16], n[17], n[18], n[19],
              n[20], n[21], n[22], n[23], n[24], n[25], n[26]);
          apply_boundary_condition(q_id[i][j][k], i, j, k,num_blocks,num_cells);
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
                double (*beta_x_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_x[i][j][k];
                double (*beta_y_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_y[i][j][k];
                double (*beta_z_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_z[i][j][k];
                double (*q_id_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    q_id[i][j][k];
                double (*Ap_id_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    Ap_id[i][j][k];
                Ap_id_[cell_i][cell_j][cell_k] = -b*h2inv*(
                    + beta_x_[cell_i+1][cell_j][cell_k]*
                    (q_id_[cell_i+1][cell_j][cell_k] -
                        q_id_[cell_i][cell_j][cell_k] )
                    + beta_x_[cell_i][cell_j][cell_k]*
                    ( q_id_[cell_i-1][cell_j][cell_k] -
                        q_id_[cell_i][cell_j][cell_k] )
                    + beta_y_[cell_i][cell_j+1][cell_k]*
                    ( q_id_[cell_i][cell_j+1][cell_k] -
                        q_id_[cell_i][cell_j][cell_k] )
                    + beta_y_[cell_i][cell_j][cell_k]*
                    ( q_id_[cell_i][cell_j-1][cell_k] -
                        q_id_[cell_i][cell_j][cell_k] )
                    + beta_z_[cell_i][cell_j][cell_k+1]*
                    ( q_id_[cell_i][cell_j][cell_k+1] -
                        q_id_[cell_i][cell_j][cell_k] )
                    + beta_z_[cell_i][cell_j][cell_k]*
                    ( q_id_[cell_i][cell_j][cell_k-1] -
                        q_id_[cell_i][cell_j][cell_k] )
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
          //   Ap_dot_r0 = dot(Ap,r0)
          Ap_dot_r0 += dot(Ap_id[i][j][k], r0_id[i][j][k], num_cells);
          if(Ap_dot_r0 == 0.0){ //   pivot breakdown ???
            BiCGStabFailed=1;
            printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
            done = 1;
          }
        }
      }
    }
    if(done) break;

    double alpha = r_dot_r0 / Ap_dot_r0; //   alpha = r_dot_r0 / Ap_dot_r0

    if(isinf(alpha)){
      BiCGStabFailed = 2;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }                                    //   pivot breakdown ???

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          //   x_id[] = x_id[] + alpha*q[], s[] = r[]   - alpha*Ap[]
          add_vectors(u[i][j][k], u[i][j][k], q_id[i][j][k], 1.0, alpha,
              num_cells);
          add_vectors(s_id[i][j][k], r_id[i][j][k], Ap_id[i][j][k], 1.0, -alpha,
              num_cells);
          // r_dot_r0 = dot(r,r0)
          double norm_of_s_block = norm(s_id[i][j][k], num_cells);
          if(norm_of_s_block > norm_of_s)
            norm_of_s = norm_of_s_block;
        }
      }
    }
    // FIX - redundant??  if As_dot_As==0, then As must be 0 which implies s==0
    if(norm_of_s == 0.0){
      BiCGStabConverged = 1;
      break;
    }

    if(norm_of_s < desired_reduction_in_norm * norm_of_r0){
      BiCGStabConverged=1;
      break;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          mul_vectors(t_id[i][j][k], dinv[i][j][k], s_id[i][j][k], 1.0,
              num_cells); // t[] = Dinv[]*s[]
        }
      }
    }

    // apply_op(level,As_id,t_id,a,b); //   As = AM^{-1}(s)
    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          double* n[27];
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
          exchange_boundary(t_id[i][j][k], i, j, k, FACE, num_blocks, num_cells,
              n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9],
              n[10], n[11], n[12], n[14], n[15], n[16], n[17], n[18], n[19],
              n[20], n[21], n[22], n[23], n[24], n[25], n[26]);
          apply_boundary_condition(t_id[i][j][k], i, j, k,num_blocks,num_cells);
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
                double (*beta_x_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_x[i][j][k];
                double (*beta_y_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_y[i][j][k];
                double (*beta_z_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    beta_z[i][j][k];
                double (*t_id_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    t_id[i][j][k];
                double (*As_id_)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
                    (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])
                    As_id[i][j][k];
                As_id_[cell_i][cell_j][cell_k] = -b*h2inv*(
                    + beta_x_[cell_i+1][cell_j][cell_k]*
                    ( t_id_[cell_i+1][cell_j][cell_k] -
                        t_id_[cell_i][cell_j][cell_k] )
                    + beta_x_[cell_i][cell_j][cell_k]*
                    ( t_id_[cell_i-1][cell_j][cell_k] -
                        t_id_[cell_i][cell_j][cell_k] )
                    + beta_y_[cell_i][cell_j+1][cell_k]*
                    ( t_id_[cell_i][cell_j+1][cell_k] -
                        t_id_[cell_i][cell_j][cell_k] )
                    + beta_y_[cell_i][cell_j][cell_k]*
                    ( t_id_[cell_i][cell_j-1][cell_k] -
                        t_id_[cell_i][cell_j][cell_k] )
                    + beta_z_[cell_i][cell_j][cell_k+1]*
                    ( t_id_[cell_i][cell_j][cell_k+1] -
                        t_id_[cell_i][cell_j][cell_k] )
                    + beta_z_[cell_i][cell_j][cell_k]*
                    ( t_id_[cell_i][cell_j][cell_k-1] -
                        t_id_[cell_i][cell_j][cell_k] )
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
          As_dot_As += dot(As_id[i][j][k], As_id[i][j][k], num_cells);
          As_dot_s  += dot(As_id[i][j][k], s_id[i][j][k], num_cells);
        }
      }
    }

    if(As_dot_As == 0.0) { // converged ?
      BiCGStabConverged=1;
      break;
    }

    double omega = As_dot_s / As_dot_As; //   omega = As_dot_s / As_dot_As
    if(omega == 0.0){
      BiCGStabFailed=3;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }                                    //   stabilization breakdown ???
    if(isinf(omega)){
      BiCGStabFailed=4;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }                                    //   stabilization breakdown ???

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          add_vectors(x_id[i][j][k], x_id[i][j][k], t_id[i][j][k], 1.0, omega,
              num_cells); // x_id[] = x_id[] + alpha*q[]
          add_vectors(r_id[i][j][k], s_id[i][j][k], As_id[i][j][k], 1.0, -omega,
              num_cells); // s[] = r[] - alpha*Ap[]   (intermediate residual?)
          // r_dot_r0 = dot(r,r0)
          double norm_of_r_block = norm(r_id[i][j][k], num_cells);
          if(norm_of_r_block > norm_of_r)
            norm_of_r = norm_of_r_block;
        }
      }
    }

    if(norm_of_r == 0.0){
      BiCGStabConverged=1;
      break;
    }
    if(norm_of_r < desired_reduction_in_norm*norm_of_r0){
      BiCGStabConverged=1;
      break;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          //   r_dot_r0_new = dot(r,r0)
          double r_dot_r0_new_block = dot(r_id[i][j][k], r0_id[i][j][k],
              num_cells);
          if(r_dot_r0_new < r_dot_r0_new_block)
            r_dot_r0_new = r_dot_r0_new_block;
        }
      }
    }

    // Lanczos breakdown ???
    if(r_dot_r0_new == 0.0){
      BiCGStabFailed=5;
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }

    // ??? beta = (r_dot_r0_new/r_dot_r0) * (alpha/omega)
    double beta_ = (r_dot_r0_new/r_dot_r0) * (alpha/omega);
    if(isinf(beta_)){
      printf("BiCGStab Failed... error = %d\n", BiCGStabFailed);
      break;
    }

    for(k = 0; k < num_blocks; k++) {
      for(j = 0; j < num_blocks; j++) {
        for(i = 0; i < num_blocks; i++) {
          add_vectors(temp[i][j][k], p_id[i][j][k], Ap_id[i][j][k], 1.0, -omega,
              num_cells);  //   VECTOR_TEMP = (p[]-omega*Ap[])
          add_vectors(p_id[i][j][k], r_id[i][j][k], temp[i][j][k], 1.0, beta_,
              num_cells);  //   p[] = r[] + beta*(p[]-omega*Ap[])
        }
      }
    }
    r_dot_r0 = r_dot_r0_new; // r_dot_r0 = r_dot_r0_new (save old r_dot_r0)
  }

  for(k = 0; k < num_blocks; k++) {
    for(j = 0; j < num_blocks; j++) {
      for(i = 0; i < num_blocks; i++) {
        cncLocalFree(r_id[i][j][k]);
        cncLocalFree(p_id[i][j][k]);
        cncLocalFree(q_id[i][j][k]);
        cncLocalFree(As_id[i][j][k]);
        cncLocalFree(Ap_id[i][j][k]);
        cncLocalFree(x_id[i][j][k]);
        cncLocalFree(s_id[i][j][k]);
        cncLocalFree(t_id[i][j][k]);
        cncLocalFree(temp[i][j][k]);
      }
    }
  }
}


void scale_vector(double* to_, double* from_, double scale, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_;

  int cell_i, cell_j, cell_k;

  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        to[cell_i][cell_j][cell_k] = scale * from[cell_i][cell_j][cell_k];
      }
    }
  }
}


void add_vectors(double* to_, double* from_a_, double* from_b_, double scale_a,
    double scale_b, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from_a)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_a_;
  double (*from_b)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_b_;

  int cell_i, cell_j, cell_k;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        double value_a = scale_a * from_a[cell_i][cell_j][cell_k];
        double value_b = scale_b * from_b[cell_i][cell_j][cell_k];
        to[cell_i][cell_j][cell_k] = value_a + value_b;
      }
    }
  }
}


void mul_vectors(double* to_, double* from_a_, double* from_b_, double scale,
    int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from_a)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_a_;
  double (*from_b)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_b_;

  int cell_i, cell_j, cell_k;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        double value_a = scale * from_a[cell_i][cell_j][cell_k];
        double value_b = scale * from_b[cell_i][cell_j][cell_k];
        to[cell_i][cell_j][cell_k] = value_a * value_b;
      }
    }
  }
}


double dot(double* to_, double* from_, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;
  double (*from)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])from_;

  int cell_i, cell_j, cell_k;
  double dot_product = 0.0;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        double from_ = from[cell_i][cell_j][cell_k];
        double to_ = to[cell_i][cell_j][cell_k];
        dot_product += to_ * from_;
      }
    }
  }
  return dot_product;
}

double norm(double* to_, int num_cells) {

  double (*to)[num_cells+2*GHOSTS][num_cells+2*GHOSTS] =
      (double(*)[num_cells+2*GHOSTS][num_cells+2*GHOSTS])to_;

  int cell_i, cell_j, cell_k;
  double max = 0.0;
  for(cell_k = GHOSTS; cell_k < num_cells + GHOSTS; cell_k++) {
    for(cell_j = GHOSTS; cell_j < num_cells + GHOSTS; cell_j++) {
      for(cell_i = GHOSTS; cell_i < num_cells + GHOSTS; cell_i++) {
        double value = fabs(to[cell_i][cell_j][cell_k]);
        if(value > max)
          max = value;
      }
    }
  }
  return max;
}
