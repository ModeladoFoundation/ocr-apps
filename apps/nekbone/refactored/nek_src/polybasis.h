#ifndef NEKBONE_POLYBASIS_H
#define NEKBONE_POLYBASIS_H

#include "blas1.h"

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

#endif // NEKBONE_POLYBASIS_H
