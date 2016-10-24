#ifndef NEKBONE_POLYBASIS_H
#define NEKBONE_POLYBASIS_H

#include "blas1.h"
#include "blas3.h"
#include "nbn_setup.h"

//!--------------------------------------------------------------------
//!
//!     Computes the Jacobi polynomial (POLY) and its derivative (PDER)
//!     of degree N at X.
//!
//!--------------------------------------------------------------------
//Taken from nekbone::speclib.f:;speclib.f::JACOBF
//It returns zero upon success.
int nbb_JACOBF(unsigned int in_polyOrder,
               double in_alpha, double in_beta,
               double in_X,    // in_X is the sample in -1<X<1
               double * o_POLY,
               double * o_PDER,
               double * o_POLYM1,
               double * o_PDERM1,
               double * o_POLYM2,
               double * o_PDERM2
               );

double nbb_GAMMAF(double X);

//!--------------------------------------------------------------------
//!
//!     Compute NP Gauss points XJAC, which are the zeros of the
//!     Jacobi polynomial J(NP) with parameters ALPHA and BETA.
//!     ALPHA and BETA determines the specific type of Gauss points.
//!     Examples:
//!     ALPHA = BETA =  0.0  ->  Legendre points
//!     ALPHA = BETA = -0.5  ->  Chebyshev points
//!
//!--------------------------------------------------------------------
//Taken from nekbone::speclib.f:;speclib.f::JACG
//It returns zero upon success.
int nbb_JACG(nbb_rvector_t o_xjac, unsigned int in_polyOrder,
             double in_alpha, double in_beta);

double nbb_endw1(unsigned int in_n, double in_alpha, double in_beta);
double nbb_endw2(unsigned int in_n, double in_alpha, double in_beta);

//
//     This routine evaluates the derivative based on all points
//     in the stencils.  It is more memory efficient than "fd_weights"
//
//     This set of routines comes from the appendix of
//     A Practical Guide to Pseudospectral Methods, B. Fornberg
//     Cambridge Univ. Press, 1996.   (pff)
//
//     Input parameters:
//       xx -- point at wich the approximations are to be accurate
//       x  -- array of x-ordinates:   x(0:n)
//       n  -- polynomial degree of interpolant (# of points := n+1)
//       m  -- highest order of derivative to be approxxmated at xi
//
//     Output:
//       c  -- set of coefficients c(0:n,0:m).
//             c(j,k) is to be applied at x(j) when
//             the kth derivative is approxxmated by a
//             stencil extending over x(0),x(1),...x(n).
void nbb_fd_weights_full(double in_xx, double * in_x, unsigned int in_n,
                     unsigned int in_m, nbb_matrix2_t * io_c);

double nbb_pnormj(unsigned int in_P, double in_alpha, double in_beta);

//-------------------------------------------------------------------
//
//    Generate NP GAUSS JACOBI points (Z) and weights (W)
//    associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
//    The polynomial degree N=NP-1.
//    Double precision version.
//
//-------------------------------------------------------------------
//It returns zero upon success.
int nbb_ZWGJD(double * io_z, double * io_w, unsigned int in_pDOF, double in_alpha, double in_beta);

//-------------------------------------------------------------------
//
//    Generate NP GAUSS LOBATTO JACOBI points (Z) and weights (W)
//    associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
//    The polynomial degree N=NP-1.
//    Double precision version.
//
//-------------------------------------------------------------------
//It returns zero upon success.
int nbb_ZWGLJD(double * io_z, double * io_w, unsigned int in_pDOF, double in_alpha, double in_beta);


//-------------------------------------------------------------------
//
//    Generate NP GAUSS LOBATTO JACOBI points (Z) and weights (W)
//    associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
//    The polynomial degree N=NP-1.
//    Single precision version.
//
//-------------------------------------------------------------------
//  The arrays: in_work_z, in_work_w, *io_z, *io_w are all zero-based arrays.
//  The arrays in_work_z and in_work_w are going to be used as work horses.
//  The io_z & io_w will recieve the actual results.
//-------------------------------------------------------------------
//It returns zero upon success.
int nbb_ZWGLJ(double * in_work_z, double * in_work_w,
              unsigned int in_pDOF, double in_alpha, double in_beta,
              NBN_REAL * io_z, NBN_REAL * io_w
              );

//-------------------------------------------------------------------
//
//    Generate NP Gauss-Lobatto Legendre points (Z) and weights (W)
//    associated with Jacobi polynomial P(N)(alpha=0,beta=0).
//    The polynomial degree N=NP-1.
//    Z and W are in single precision, but all the arithmetic
//    operations are done in double precision.
//
//-------------------------------------------------------------------
//  The arrays: in_work_z, in_work_w, *io_z, *io_w are all zero-based arrays.
//  The arrays in_work_z and in_work_w are going to be used as work horses.
//  The io_z & io_w will recieve the actual results.
//-------------------------------------------------------------------
//It returns zero upon success.
int nbb_ZWGLL(double * in_work_z, double * in_work_w, unsigned int in_pDOF,
              NBN_REAL * io_z, NBN_REAL * io_w);

//-------------------------------------------------------------------
//  nekbone-2.3.4/src/semhat.f
//-------------------------------------------------------------------
//It returns zero upon success.
int nbb_semhat(nbb_matrix2_t io_a, NBN_REAL * io_b, nbb_matrix2_t  io_c,
               nbb_matrix2_t io_d, NBN_REAL * io_z, NBN_REAL * io_w,
               unsigned int in_Porder, double * io_work_w, double * io_work_z);

void nbb_setup_g(unsigned int in_pDOF, NBN_REAL * in_wxm1,
                NBN_REAL * o_g1, NBN_REAL * o_g4, NBN_REAL * o_g6);

int nbb_proxy_setup(nbb_matrix2_t io_a, NBN_REAL * io_b, nbb_matrix2_t  io_c,
                    nbb_matrix2_t io_d, NBN_REAL * io_z, NBN_REAL * io_w,
                    unsigned int in_pDOF, double * io_work_w, double * io_work_z,
                    NBN_REAL * o_g1, NBN_REAL * o_g4, NBN_REAL * o_g6,
                    nbb_matrix2_t o_dxm1, nbb_matrix2_t o_dxtm1);

#endif // NEKBONE_POLYBASIS_H
