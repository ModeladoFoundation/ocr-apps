/*

                 Copyright (c) 2010.
      Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.
                  LLNL-CODE-461231
                All rights reserved.

This file is part of LULESH, Version 1.0.0
Please also read this link -- http://www.opensource.org/licenses/index.php

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted below)
     in the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the LLNS/LLNL nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Additional BSD Notice

1. This notice is required to be provided under our contract with the U.S.
   Department of Energy (DOE). This work was produced at Lawrence Livermore
   National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.

2. Neither the United States Government nor Lawrence Livermore National
   Security, LLC nor any of their employees, makes any warranty, express
   or implied, or assumes any liability or responsibility for the accuracy,
   completeness, or usefulness of any information, apparatus, product, or
   process disclosed, or represents that its use would not infringe
   privately-owned rights.

3. Also, reference herein to any specific commercial products, process, or
   services by trade name, trademark, manufacturer or otherwise does not
   necessarily constitute or imply its endorsement, recommendation, or
   favoring by the United States Government or Lawrence Livermore National
   Security, LLC. The views and opinions of authors expressed herein do not
   necessarily state or reflect those of the United States Government or
   Lawrence Livermore National Security, LLC, and shall not be used for
   advertising or product endorsement purposes.

*/

#include <stdio.h>
#include <stdlib.h>

#ifdef HAB_C
#include "hc.h"
#include <math.h>
#else
#include <tgmath.h>
#endif

#define LULESH_SHOW_PROGRESS 1

enum { VolumeError = -1, QStopError = -2 } ;

/****************************************************/
/* Allow flexibility for arithmetic representations */
/****************************************************/

/* Could also support fixed point and interval arithmetic types */
typedef float        real4 ;
typedef double       real8 ;
typedef long double  real10 ;  /* 10 bytes on x86 */

typedef int    Index_t ; /* array subscript and loop index */
typedef real8  Real_t ;  /* floating point representation */
typedef int    Int_t ;   /* integer representation */

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

#include "domain.h"


/* Stuff needed for boundary conditions */
/* 2 BCs on each of 6 hexahedral faces (12 bits) */
#define XI_M        0x003
#define XI_M_SYMM   0x001
#define XI_M_FREE   0x002

#define XI_P        0x00c
#define XI_P_SYMM   0x004
#define XI_P_FREE   0x008

#define ETA_M       0x030
#define ETA_M_SYMM  0x010
#define ETA_M_FREE  0x020

#define ETA_P       0x0c0
#define ETA_P_SYMM  0x040
#define ETA_P_FREE  0x080

#define ZETA_M      0x300
#define ZETA_M_SYMM 0x100
#define ZETA_M_FREE 0x200

#define ZETA_P      0xc00
#define ZETA_P_SYMM 0x400
#define ZETA_P_FREE 0x800


static inline
void TimeIncrement()
{
   Real_t targetdt = domain.m_stoptime - domain.m_time ;

   if ((domain.m_dtfixed <= cast_Real_t(0.0)) && (domain.m_cycle != cast_Int_t(0))) {
      Real_t ratio ;
      Real_t olddt = domain.m_deltatime ;

      /* This will require a reduction in parallel */
      Real_t newdt = cast_Real_t(1.0e+20) ;
      if (domain.m_dtcourant < newdt) {
         newdt = domain.m_dtcourant / cast_Real_t(2.0) ;
      }
      if (domain.m_dthydro < newdt) {
         newdt = domain.m_dthydro * cast_Real_t(2.0) / cast_Real_t(3.0) ;
      }

      ratio = newdt / olddt ;
      if (ratio >= cast_Real_t(1.0)) {
         if (ratio < domain.m_deltatimemultlb) {
            newdt = olddt ;
         }
         else if (ratio > domain.m_deltatimemultub) {
            newdt = olddt*domain.m_deltatimemultub ;
         }
      }

      if (newdt > domain.m_dtmax) {
         newdt = domain.m_dtmax ;
      }
      domain.m_deltatime = newdt ;
   }

   /* TRY TO PREVENT VERY SMALL SCALING ON THE NEXT CYCLE */
   if ((targetdt > domain.m_deltatime) &&
       (targetdt < (cast_Real_t(4.0) * domain.m_deltatime / cast_Real_t(3.0))) ) {
      targetdt = cast_Real_t(2.0) * domain.m_deltatime / cast_Real_t(3.0) ;
   }

   if (targetdt < domain.m_deltatime) {
      domain.m_deltatime = targetdt ;
   }

   domain.m_time += domain.m_deltatime ;

   ++domain.m_cycle ;
}

static inline
void InitStressTermsForElems(Index_t numElem, 
                             Real_t *sigxx, Real_t *sigyy, Real_t *sigzz)
{
   //
   // pull in the stresses appropriate to the hydro integration
   //
   for (Index_t i = 0 ; i < numElem ; ++i){
      sigxx[i] =  sigyy[i] = sigzz[i] =  - domain.m_p[i] - domain.m_q[i] ;
   }
}

static inline
void CalcElemShapeFunctionDerivatives( const Real_t* const x,
                                       const Real_t* const y,
                                       const Real_t* const z,
                                       Real_t b[][EIGHT],
                                       Real_t* const volume )
{
  const Real_t x0 = x[0] ;   const Real_t x1 = x[1] ;
  const Real_t x2 = x[2] ;   const Real_t x3 = x[3] ;
  const Real_t x4 = x[4] ;   const Real_t x5 = x[5] ;
  const Real_t x6 = x[6] ;   const Real_t x7 = x[7] ;

  const Real_t y0 = y[0] ;   const Real_t y1 = y[1] ;
  const Real_t y2 = y[2] ;   const Real_t y3 = y[3] ;
  const Real_t y4 = y[4] ;   const Real_t y5 = y[5] ;
  const Real_t y6 = y[6] ;   const Real_t y7 = y[7] ;

  const Real_t z0 = z[0] ;   const Real_t z1 = z[1] ;
  const Real_t z2 = z[2] ;   const Real_t z3 = z[3] ;
  const Real_t z4 = z[4] ;   const Real_t z5 = z[5] ;
  const Real_t z6 = z[6] ;   const Real_t z7 = z[7] ;

  Real_t fjxxi, fjxet, fjxze;
  Real_t fjyxi, fjyet, fjyze;
  Real_t fjzxi, fjzet, fjzze;
  Real_t cjxxi, cjxet, cjxze;
  Real_t cjyxi, cjyet, cjyze;
  Real_t cjzxi, cjzet, cjzze;

  fjxxi = .125 * ( (x6-x0) + (x5-x3) - (x7-x1) - (x4-x2) );
  fjxet = .125 * ( (x6-x0) - (x5-x3) + (x7-x1) - (x4-x2) );
  fjxze = .125 * ( (x6-x0) + (x5-x3) + (x7-x1) + (x4-x2) );

  fjyxi = .125 * ( (y6-y0) + (y5-y3) - (y7-y1) - (y4-y2) );
  fjyet = .125 * ( (y6-y0) - (y5-y3) + (y7-y1) - (y4-y2) );
  fjyze = .125 * ( (y6-y0) + (y5-y3) + (y7-y1) + (y4-y2) );

  fjzxi = .125 * ( (z6-z0) + (z5-z3) - (z7-z1) - (z4-z2) );
  fjzet = .125 * ( (z6-z0) - (z5-z3) + (z7-z1) - (z4-z2) );
  fjzze = .125 * ( (z6-z0) + (z5-z3) + (z7-z1) + (z4-z2) );

  /* compute cofactors */
  cjxxi =    (fjyet * fjzze) - (fjzet * fjyze);
  cjxet =  - (fjyxi * fjzze) + (fjzxi * fjyze);
  cjxze =    (fjyxi * fjzet) - (fjzxi * fjyet);

  cjyxi =  - (fjxet * fjzze) + (fjzet * fjxze);
  cjyet =    (fjxxi * fjzze) - (fjzxi * fjxze);
  cjyze =  - (fjxxi * fjzet) + (fjzxi * fjxet);

  cjzxi =    (fjxet * fjyze) - (fjyet * fjxze);
  cjzet =  - (fjxxi * fjyze) + (fjyxi * fjxze);
  cjzze =    (fjxxi * fjyet) - (fjyxi * fjxet);

  /* calculate partials :
     this need only be done for l = 0,1,2,3   since , by symmetry ,
     (6,7,4,5) = - (0,1,2,3) .
  */
  b[0][0] =   -  cjxxi  -  cjxet  -  cjxze;
  b[0][1] =      cjxxi  -  cjxet  -  cjxze;
  b[0][2] =      cjxxi  +  cjxet  -  cjxze;
  b[0][3] =   -  cjxxi  +  cjxet  -  cjxze;
  b[0][4] = -b[0][2];
  b[0][5] = -b[0][3];
  b[0][6] = -b[0][0];
  b[0][7] = -b[0][1];

  b[1][0] =   -  cjyxi  -  cjyet  -  cjyze;
  b[1][1] =      cjyxi  -  cjyet  -  cjyze;
  b[1][2] =      cjyxi  +  cjyet  -  cjyze;
  b[1][3] =   -  cjyxi  +  cjyet  -  cjyze;
  b[1][4] = -b[1][2];
  b[1][5] = -b[1][3];
  b[1][6] = -b[1][0];
  b[1][7] = -b[1][1];

  b[2][0] =   -  cjzxi  -  cjzet  -  cjzze;
  b[2][1] =      cjzxi  -  cjzet  -  cjzze;
  b[2][2] =      cjzxi  +  cjzet  -  cjzze;
  b[2][3] =   -  cjzxi  +  cjzet  -  cjzze;
  b[2][4] = -b[2][2];
  b[2][5] = -b[2][3];
  b[2][6] = -b[2][0];
  b[2][7] = -b[2][1];

  /* calculate jacobian determinant (volume) */
  *volume = cast_Real_t(8.) * ( fjxet * cjxet + fjyet * cjyet + fjzet * cjzet);
}

static inline
void SumElemFaceNormal(Real_t *normalX0, Real_t *normalY0, Real_t *normalZ0,
                       Real_t *normalX1, Real_t *normalY1, Real_t *normalZ1,
                       Real_t *normalX2, Real_t *normalY2, Real_t *normalZ2,
                       Real_t *normalX3, Real_t *normalY3, Real_t *normalZ3,
                       const Real_t x0, const Real_t y0, const Real_t z0,
                       const Real_t x1, const Real_t y1, const Real_t z1,
                       const Real_t x2, const Real_t y2, const Real_t z2,
                       const Real_t x3, const Real_t y3, const Real_t z3)
{
   Real_t bisectX0 = cast_Real_t(0.5) * (x3 + x2 - x1 - x0);
   Real_t bisectY0 = cast_Real_t(0.5) * (y3 + y2 - y1 - y0);
   Real_t bisectZ0 = cast_Real_t(0.5) * (z3 + z2 - z1 - z0);
   Real_t bisectX1 = cast_Real_t(0.5) * (x2 + x1 - x3 - x0);
   Real_t bisectY1 = cast_Real_t(0.5) * (y2 + y1 - y3 - y0);
   Real_t bisectZ1 = cast_Real_t(0.5) * (z2 + z1 - z3 - z0);
   Real_t areaX = cast_Real_t(0.25) * (bisectY0 * bisectZ1 - bisectZ0 * bisectY1);
   Real_t areaY = cast_Real_t(0.25) * (bisectZ0 * bisectX1 - bisectX0 * bisectZ1);
   Real_t areaZ = cast_Real_t(0.25) * (bisectX0 * bisectY1 - bisectY0 * bisectX1);

   *normalX0 += areaX;
   *normalX1 += areaX;
   *normalX2 += areaX;
   *normalX3 += areaX;

   *normalY0 += areaY;
   *normalY1 += areaY;
   *normalY2 += areaY;
   *normalY3 += areaY;

   *normalZ0 += areaZ;
   *normalZ1 += areaZ;
   *normalZ2 += areaZ;
   *normalZ3 += areaZ;
}

static inline
void CalcElemNodeNormals(Real_t pfx[EIGHT],
                         Real_t pfy[EIGHT],
                         Real_t pfz[EIGHT],
                         const Real_t x[EIGHT],
                         const Real_t y[EIGHT],
                         const Real_t z[EIGHT])
{
   for (Index_t i = 0 ; i < EIGHT ; ++i) {
      pfx[i] = cast_Real_t(0.0);
      pfy[i] = cast_Real_t(0.0);
      pfz[i] = cast_Real_t(0.0);
   }
   /* evaluate face one: nodes 0, 1, 2, 3 */
   SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
                  &pfx[1], &pfy[1], &pfz[1],
                  &pfx[2], &pfy[2], &pfz[2],
                  &pfx[3], &pfy[3], &pfz[3],
                  x[0], y[0], z[0], x[1], y[1], z[1],
                  x[2], y[2], z[2], x[3], y[3], z[3]);
   /* evaluate face two: nodes 0, 4, 5, 1 */
   SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
                  &pfx[4], &pfy[4], &pfz[4],
                  &pfx[5], &pfy[5], &pfz[5],
                  &pfx[1], &pfy[1], &pfz[1],
                  x[0], y[0], z[0], x[4], y[4], z[4],
                  x[5], y[5], z[5], x[1], y[1], z[1]);
   /* evaluate face three: nodes 1, 5, 6, 2 */
   SumElemFaceNormal(&pfx[1], &pfy[1], &pfz[1],
                  &pfx[5], &pfy[5], &pfz[5],
                  &pfx[6], &pfy[6], &pfz[6],
                  &pfx[2], &pfy[2], &pfz[2],
                  x[1], y[1], z[1], x[5], y[5], z[5],
                  x[6], y[6], z[6], x[2], y[2], z[2]);
   /* evaluate face four: nodes 2, 6, 7, 3 */
   SumElemFaceNormal(&pfx[2], &pfy[2], &pfz[2],
                  &pfx[6], &pfy[6], &pfz[6],
                  &pfx[7], &pfy[7], &pfz[7],
                  &pfx[3], &pfy[3], &pfz[3],
                  x[2], y[2], z[2], x[6], y[6], z[6],
                  x[7], y[7], z[7], x[3], y[3], z[3]);
   /* evaluate face five: nodes 3, 7, 4, 0 */
   SumElemFaceNormal(&pfx[3], &pfy[3], &pfz[3],
                  &pfx[7], &pfy[7], &pfz[7],
                  &pfx[4], &pfy[4], &pfz[4],
                  &pfx[0], &pfy[0], &pfz[0],
                  x[3], y[3], z[3], x[7], y[7], z[7],
                  x[4], y[4], z[4], x[0], y[0], z[0]);
   /* evaluate face six: nodes 4, 7, 6, 5 */
   SumElemFaceNormal(&pfx[4], &pfy[4], &pfz[4],
                  &pfx[7], &pfy[7], &pfz[7],
                  &pfx[6], &pfy[6], &pfz[6],
                  &pfx[5], &pfy[5], &pfz[5],
                  x[4], y[4], z[4], x[7], y[7], z[7],
                  x[6], y[6], z[6], x[5], y[5], z[5]);
}

static inline
void SumElemStressesToNodeForces( hcConst Real_t B[][EIGHT],
                                  const Real_t stress_xx,
                                  const Real_t stress_yy,
                                  const Real_t stress_zz,
                                  Real_t* const fx,
                                  Real_t* const fy,
                                  Real_t* const fz )
{
  Real_t pfx0 = B[0][0] ;   Real_t pfx1 = B[0][1] ;
  Real_t pfx2 = B[0][2] ;   Real_t pfx3 = B[0][3] ;
  Real_t pfx4 = B[0][4] ;   Real_t pfx5 = B[0][5] ;
  Real_t pfx6 = B[0][6] ;   Real_t pfx7 = B[0][7] ;

  Real_t pfy0 = B[1][0] ;   Real_t pfy1 = B[1][1] ;
  Real_t pfy2 = B[1][2] ;   Real_t pfy3 = B[1][3] ;
  Real_t pfy4 = B[1][4] ;   Real_t pfy5 = B[1][5] ;
  Real_t pfy6 = B[1][6] ;   Real_t pfy7 = B[1][7] ;

  Real_t pfz0 = B[2][0] ;   Real_t pfz1 = B[2][1] ;
  Real_t pfz2 = B[2][2] ;   Real_t pfz3 = B[2][3] ;
  Real_t pfz4 = B[2][4] ;   Real_t pfz5 = B[2][5] ;
  Real_t pfz6 = B[2][6] ;   Real_t pfz7 = B[2][7] ;

  fx[0] = -( stress_xx * pfx0 );
  fx[1] = -( stress_xx * pfx1 );
  fx[2] = -( stress_xx * pfx2 );
  fx[3] = -( stress_xx * pfx3 );
  fx[4] = -( stress_xx * pfx4 );
  fx[5] = -( stress_xx * pfx5 );
  fx[6] = -( stress_xx * pfx6 );
  fx[7] = -( stress_xx * pfx7 );

  fy[0] = -( stress_yy * pfy0  );
  fy[1] = -( stress_yy * pfy1  );
  fy[2] = -( stress_yy * pfy2  );
  fy[3] = -( stress_yy * pfy3  );
  fy[4] = -( stress_yy * pfy4  );
  fy[5] = -( stress_yy * pfy5  );
  fy[6] = -( stress_yy * pfy6  );
  fy[7] = -( stress_yy * pfy7  );

  fz[0] = -( stress_zz * pfz0 );
  fz[1] = -( stress_zz * pfz1 );
  fz[2] = -( stress_zz * pfz2 );
  fz[3] = -( stress_zz * pfz3 );
  fz[4] = -( stress_zz * pfz4 );
  fz[5] = -( stress_zz * pfz5 );
  fz[6] = -( stress_zz * pfz6 );
  fz[7] = -( stress_zz * pfz7 );
}

static inline
void IntegrateStressForElems( Index_t numElem,
                              Real_t *sigxx, Real_t *sigyy, Real_t *sigzz,
                              Real_t *determ)
{
  Real_t B[3][EIGHT] ;// shape function derivatives
  Real_t x_local[EIGHT] ;
  Real_t y_local[EIGHT] ;
  Real_t z_local[EIGHT] ;
  Real_t fx_local[EIGHT] ;
  Real_t fy_local[EIGHT] ;
  Real_t fz_local[EIGHT] ;

  // loop over all elements
  for( Index_t k=0 ; k<numElem ; ++k )
  {
    const Index_t* const elemNodes = (Index_t *)&domain.m_nodelist[EIGHT*k];

    // get nodal coordinates from global arrays and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode )
    {
      Index_t gnode = elemNodes[lnode];
      x_local[lnode] = domain.m_x[gnode];
      y_local[lnode] = domain.m_y[gnode];
      z_local[lnode] = domain.m_z[gnode];
    }

    /* Volume calculation involves extra work for numerical consistency. */
    CalcElemShapeFunctionDerivatives(x_local, y_local, z_local,
                                         B, &determ[k]);

    CalcElemNodeNormals( B[0] , B[1], B[2],
                          x_local, y_local, z_local );

    SumElemStressesToNodeForces( B, sigxx[k], sigyy[k], sigzz[k],
                                         fx_local, fy_local, fz_local ) ;

    // copy nodal force contributions to global force arrray.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode )
    {
      Index_t gnode = elemNodes[lnode];
      domain.m_fx[gnode] += fx_local[lnode];
      domain.m_fy[gnode] += fy_local[lnode];
      domain.m_fz[gnode] += fz_local[lnode];
    }
  }
}

static inline
void CollectDomainNodesToElemNodes(const Index_t* elemToNode,
                                   Real_t elemX[EIGHT],
                                   Real_t elemY[EIGHT],
                                   Real_t elemZ[EIGHT])
{
   Index_t nd0i = elemToNode[0] ;
   Index_t nd1i = elemToNode[1] ;
   Index_t nd2i = elemToNode[2] ;
   Index_t nd3i = elemToNode[3] ;
   Index_t nd4i = elemToNode[4] ;
   Index_t nd5i = elemToNode[5] ;
   Index_t nd6i = elemToNode[6] ;
   Index_t nd7i = elemToNode[7] ;

   elemX[0] = domain.m_x[nd0i];
   elemX[1] = domain.m_x[nd1i];
   elemX[2] = domain.m_x[nd2i];
   elemX[3] = domain.m_x[nd3i];
   elemX[4] = domain.m_x[nd4i];
   elemX[5] = domain.m_x[nd5i];
   elemX[6] = domain.m_x[nd6i];
   elemX[7] = domain.m_x[nd7i];

   elemY[0] = domain.m_y[nd0i];
   elemY[1] = domain.m_y[nd1i];
   elemY[2] = domain.m_y[nd2i];
   elemY[3] = domain.m_y[nd3i];
   elemY[4] = domain.m_y[nd4i];
   elemY[5] = domain.m_y[nd5i];
   elemY[6] = domain.m_y[nd6i];
   elemY[7] = domain.m_y[nd7i];

   elemZ[0] = domain.m_z[nd0i];
   elemZ[1] = domain.m_z[nd1i];
   elemZ[2] = domain.m_z[nd2i];
   elemZ[3] = domain.m_z[nd3i];
   elemZ[4] = domain.m_z[nd4i];
   elemZ[5] = domain.m_z[nd5i];
   elemZ[6] = domain.m_z[nd6i];
   elemZ[7] = domain.m_z[nd7i];

}

static inline
void VoluDer(const Real_t x0, const Real_t x1, const Real_t x2,
             const Real_t x3, const Real_t x4, const Real_t x5,
             const Real_t y0, const Real_t y1, const Real_t y2,
             const Real_t y3, const Real_t y4, const Real_t y5,
             const Real_t z0, const Real_t z1, const Real_t z2,
             const Real_t z3, const Real_t z4, const Real_t z5,
             Real_t* dvdx, Real_t* dvdy, Real_t* dvdz)
{
   const Real_t twelfth = cast_Real_t(1.0) / cast_Real_t(12.0) ;

   *dvdx =
      (y1 + y2) * (z0 + z1) - (y0 + y1) * (z1 + z2) +
      (y0 + y4) * (z3 + z4) - (y3 + y4) * (z0 + z4) -
      (y2 + y5) * (z3 + z5) + (y3 + y5) * (z2 + z5);
   *dvdy =
      - (x1 + x2) * (z0 + z1) + (x0 + x1) * (z1 + z2) -
      (x0 + x4) * (z3 + z4) + (x3 + x4) * (z0 + z4) +
      (x2 + x5) * (z3 + z5) - (x3 + x5) * (z2 + z5);

   *dvdz =
      - (y1 + y2) * (x0 + x1) + (y0 + y1) * (x1 + x2) -
      (y0 + y4) * (x3 + x4) + (y3 + y4) * (x0 + x4) +
      (y2 + y5) * (x3 + x5) - (y3 + y5) * (x2 + x5);

   *dvdx *= twelfth;
   *dvdy *= twelfth;
   *dvdz *= twelfth;
}

static inline
void CalcElemVolumeDerivative(Real_t dvdx[EIGHT],
                              Real_t dvdy[EIGHT],
                              Real_t dvdz[EIGHT],
                              const Real_t x[EIGHT],
                              const Real_t y[EIGHT],
                              const Real_t z[EIGHT])
{
   VoluDer(x[1], x[2], x[3], x[4], x[5], x[7],
           y[1], y[2], y[3], y[4], y[5], y[7],
           z[1], z[2], z[3], z[4], z[5], z[7],
           &dvdx[0], &dvdy[0], &dvdz[0]);
   VoluDer(x[0], x[1], x[2], x[7], x[4], x[6],
           y[0], y[1], y[2], y[7], y[4], y[6],
           z[0], z[1], z[2], z[7], z[4], z[6],
           &dvdx[3], &dvdy[3], &dvdz[3]);
   VoluDer(x[3], x[0], x[1], x[6], x[7], x[5],
           y[3], y[0], y[1], y[6], y[7], y[5],
           z[3], z[0], z[1], z[6], z[7], z[5],
           &dvdx[2], &dvdy[2], &dvdz[2]);
   VoluDer(x[2], x[3], x[0], x[5], x[6], x[4],
           y[2], y[3], y[0], y[5], y[6], y[4],
           z[2], z[3], z[0], z[5], z[6], z[4],
           &dvdx[1], &dvdy[1], &dvdz[1]);
   VoluDer(x[7], x[6], x[5], x[0], x[3], x[1],
           y[7], y[6], y[5], y[0], y[3], y[1],
           z[7], z[6], z[5], z[0], z[3], z[1],
           &dvdx[4], &dvdy[4], &dvdz[4]);
   VoluDer(x[4], x[7], x[6], x[1], x[0], x[2],
           y[4], y[7], y[6], y[1], y[0], y[2],
           z[4], z[7], z[6], z[1], z[0], z[2],
           &dvdx[5], &dvdy[5], &dvdz[5]);
   VoluDer(x[5], x[4], x[7], x[2], x[1], x[3],
           y[5], y[4], y[7], y[2], y[1], y[3],
           z[5], z[4], z[7], z[2], z[1], z[3],
           &dvdx[6], &dvdy[6], &dvdz[6]);
   VoluDer(x[6], x[5], x[4], x[3], x[2], x[0],
           y[6], y[5], y[4], y[3], y[2], y[0],
           z[6], z[5], z[4], z[3], z[2], z[0],
           &dvdx[7], &dvdy[7], &dvdz[7]);
}

static inline
void CalcElemFBHourglassForce(Real_t *xd, Real_t *yd, Real_t *zd,  Real_t *hourgam0,
                              Real_t *hourgam1, Real_t *hourgam2, Real_t *hourgam3,
                              Real_t *hourgam4, Real_t *hourgam5, Real_t *hourgam6,
                              Real_t *hourgam7, Real_t coefficient,
                              Real_t *hgfx, Real_t *hgfy, Real_t *hgfz )
{
   Index_t i00=0;
   Index_t i01=1;
   Index_t i02=2;
   Index_t i03=3;

   Real_t h00 =
      hourgam0[i00] * xd[0] + hourgam1[i00] * xd[1] +
      hourgam2[i00] * xd[2] + hourgam3[i00] * xd[3] +
      hourgam4[i00] * xd[4] + hourgam5[i00] * xd[5] +
      hourgam6[i00] * xd[6] + hourgam7[i00] * xd[7];

   Real_t h01 =
      hourgam0[i01] * xd[0] + hourgam1[i01] * xd[1] +
      hourgam2[i01] * xd[2] + hourgam3[i01] * xd[3] +
      hourgam4[i01] * xd[4] + hourgam5[i01] * xd[5] +
      hourgam6[i01] * xd[6] + hourgam7[i01] * xd[7];

   Real_t h02 =
      hourgam0[i02] * xd[0] + hourgam1[i02] * xd[1]+
      hourgam2[i02] * xd[2] + hourgam3[i02] * xd[3]+
      hourgam4[i02] * xd[4] + hourgam5[i02] * xd[5]+
      hourgam6[i02] * xd[6] + hourgam7[i02] * xd[7];

   Real_t h03 =
      hourgam0[i03] * xd[0] + hourgam1[i03] * xd[1] +
      hourgam2[i03] * xd[2] + hourgam3[i03] * xd[3] +
      hourgam4[i03] * xd[4] + hourgam5[i03] * xd[5] +
      hourgam6[i03] * xd[6] + hourgam7[i03] * xd[7];

   hgfx[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

   hgfx[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

   hgfx[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

   hgfx[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

   hgfx[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

   hgfx[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

   hgfx[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

   hgfx[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);

   h00 =
      hourgam0[i00] * yd[0] + hourgam1[i00] * yd[1] +
      hourgam2[i00] * yd[2] + hourgam3[i00] * yd[3] +
      hourgam4[i00] * yd[4] + hourgam5[i00] * yd[5] +
      hourgam6[i00] * yd[6] + hourgam7[i00] * yd[7];

   h01 =
      hourgam0[i01] * yd[0] + hourgam1[i01] * yd[1] +
      hourgam2[i01] * yd[2] + hourgam3[i01] * yd[3] +
      hourgam4[i01] * yd[4] + hourgam5[i01] * yd[5] +
      hourgam6[i01] * yd[6] + hourgam7[i01] * yd[7];

   h02 =
      hourgam0[i02] * yd[0] + hourgam1[i02] * yd[1]+
      hourgam2[i02] * yd[2] + hourgam3[i02] * yd[3]+
      hourgam4[i02] * yd[4] + hourgam5[i02] * yd[5]+
      hourgam6[i02] * yd[6] + hourgam7[i02] * yd[7];

   h03 =
      hourgam0[i03] * yd[0] + hourgam1[i03] * yd[1] +
      hourgam2[i03] * yd[2] + hourgam3[i03] * yd[3] +
      hourgam4[i03] * yd[4] + hourgam5[i03] * yd[5] +
      hourgam6[i03] * yd[6] + hourgam7[i03] * yd[7];


   hgfy[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

   hgfy[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

   hgfy[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

   hgfy[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

   hgfy[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

   hgfy[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

   hgfy[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

   hgfy[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);

   h00 =
      hourgam0[i00] * zd[0] + hourgam1[i00] * zd[1] +
      hourgam2[i00] * zd[2] + hourgam3[i00] * zd[3] +
      hourgam4[i00] * zd[4] + hourgam5[i00] * zd[5] +
      hourgam6[i00] * zd[6] + hourgam7[i00] * zd[7];

   h01 =
      hourgam0[i01] * zd[0] + hourgam1[i01] * zd[1] +
      hourgam2[i01] * zd[2] + hourgam3[i01] * zd[3] +
      hourgam4[i01] * zd[4] + hourgam5[i01] * zd[5] +
      hourgam6[i01] * zd[6] + hourgam7[i01] * zd[7];

   h02 =
      hourgam0[i02] * zd[0] + hourgam1[i02] * zd[1]+
      hourgam2[i02] * zd[2] + hourgam3[i02] * zd[3]+
      hourgam4[i02] * zd[4] + hourgam5[i02] * zd[5]+
      hourgam6[i02] * zd[6] + hourgam7[i02] * zd[7];

   h03 =
      hourgam0[i03] * zd[0] + hourgam1[i03] * zd[1] +
      hourgam2[i03] * zd[2] + hourgam3[i03] * zd[3] +
      hourgam4[i03] * zd[4] + hourgam5[i03] * zd[5] +
      hourgam6[i03] * zd[6] + hourgam7[i03] * zd[7];


   hgfz[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

   hgfz[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

   hgfz[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

   hgfz[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

   hgfz[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

   hgfz[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

   hgfz[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

   hgfz[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);
}

static inline
void CalcFBHourglassForceForElems(Real_t *determ,
            Real_t *x8n,      Real_t *y8n,      Real_t *z8n,
            Real_t *dvdx,     Real_t *dvdy,     Real_t *dvdz,
            Real_t hourg)
{
   /*************************************************
    *
    *     FUNCTION: Calculates the Flanagan-Belytschko anti-hourglass
    *               force.
    *
    *************************************************/

   Index_t numElem = domain.m_numElem ;

   Real_t hgfx[EIGHT], hgfy[EIGHT], hgfz[EIGHT] ;

   Real_t coefficient;

   Real_t  gamma[4][EIGHT];
   Real_t hourgam0[4], hourgam1[4], hourgam2[4], hourgam3[4] ;
   Real_t hourgam4[4], hourgam5[4], hourgam6[4], hourgam7[4];
   Real_t xd1[EIGHT], yd1[EIGHT], zd1[EIGHT] ;

   gamma[0][0] = cast_Real_t( 1.);
   gamma[0][1] = cast_Real_t( 1.);
   gamma[0][2] = cast_Real_t(-1.);
   gamma[0][3] = cast_Real_t(-1.);
   gamma[0][4] = cast_Real_t(-1.);
   gamma[0][5] = cast_Real_t(-1.);
   gamma[0][6] = cast_Real_t( 1.);
   gamma[0][7] = cast_Real_t( 1.);
   gamma[1][0] = cast_Real_t( 1.);
   gamma[1][1] = cast_Real_t(-1.);
   gamma[1][2] = cast_Real_t(-1.);
   gamma[1][3] = cast_Real_t( 1.);
   gamma[1][4] = cast_Real_t(-1.);
   gamma[1][5] = cast_Real_t( 1.);
   gamma[1][6] = cast_Real_t( 1.);
   gamma[1][7] = cast_Real_t(-1.);
   gamma[2][0] = cast_Real_t( 1.);
   gamma[2][1] = cast_Real_t(-1.);
   gamma[2][2] = cast_Real_t( 1.);
   gamma[2][3] = cast_Real_t(-1.);
   gamma[2][4] = cast_Real_t( 1.);
   gamma[2][5] = cast_Real_t(-1.);
   gamma[2][6] = cast_Real_t( 1.);
   gamma[2][7] = cast_Real_t(-1.);
   gamma[3][0] = cast_Real_t(-1.);
   gamma[3][1] = cast_Real_t( 1.);
   gamma[3][2] = cast_Real_t(-1.);
   gamma[3][3] = cast_Real_t( 1.);
   gamma[3][4] = cast_Real_t( 1.);
   gamma[3][5] = cast_Real_t(-1.);
   gamma[3][6] = cast_Real_t( 1.);
   gamma[3][7] = cast_Real_t(-1.);

/*************************************************/
/*    compute the hourglass modes */


   for(Index_t i2=0;i2<numElem;++i2){
      const Index_t *elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i2];
      Index_t i3=EIGHT*i2;
      Real_t volinv=cast_Real_t(1.0)/determ[i2];
      Real_t ss1, mass1, volume13 ;
      for(Index_t i1=0;i1<4;++i1){

         Real_t hourmodx =
            x8n[i3]   * gamma[i1][0] + x8n[i3+1] * gamma[i1][1] +
            x8n[i3+2] * gamma[i1][2] + x8n[i3+3] * gamma[i1][3] +
            x8n[i3+4] * gamma[i1][4] + x8n[i3+5] * gamma[i1][5] +
            x8n[i3+6] * gamma[i1][6] + x8n[i3+7] * gamma[i1][7];

         Real_t hourmody =
            y8n[i3]   * gamma[i1][0] + y8n[i3+1] * gamma[i1][1] +
            y8n[i3+2] * gamma[i1][2] + y8n[i3+3] * gamma[i1][3] +
            y8n[i3+4] * gamma[i1][4] + y8n[i3+5] * gamma[i1][5] +
            y8n[i3+6] * gamma[i1][6] + y8n[i3+7] * gamma[i1][7];

         Real_t hourmodz =
            z8n[i3]   * gamma[i1][0] + z8n[i3+1] * gamma[i1][1] +
            z8n[i3+2] * gamma[i1][2] + z8n[i3+3] * gamma[i1][3] +
            z8n[i3+4] * gamma[i1][4] + z8n[i3+5] * gamma[i1][5] +
            z8n[i3+6] * gamma[i1][6] + z8n[i3+7] * gamma[i1][7];

         hourgam0[i1] = gamma[i1][0] -  volinv*(dvdx[i3  ] * hourmodx +
                                                  dvdy[i3  ] * hourmody +
                                                  dvdz[i3  ] * hourmodz );

         hourgam1[i1] = gamma[i1][1] -  volinv*(dvdx[i3+1] * hourmodx +
                                                  dvdy[i3+1] * hourmody +
                                                  dvdz[i3+1] * hourmodz );

         hourgam2[i1] = gamma[i1][2] -  volinv*(dvdx[i3+2] * hourmodx +
                                                  dvdy[i3+2] * hourmody +
                                                  dvdz[i3+2] * hourmodz );

         hourgam3[i1] = gamma[i1][3] -  volinv*(dvdx[i3+3] * hourmodx +
                                                  dvdy[i3+3] * hourmody +
                                                  dvdz[i3+3] * hourmodz );

         hourgam4[i1] = gamma[i1][4] -  volinv*(dvdx[i3+4] * hourmodx +
                                                  dvdy[i3+4] * hourmody +
                                                  dvdz[i3+4] * hourmodz );

         hourgam5[i1] = gamma[i1][5] -  volinv*(dvdx[i3+5] * hourmodx +
                                                  dvdy[i3+5] * hourmody +
                                                  dvdz[i3+5] * hourmodz );

         hourgam6[i1] = gamma[i1][6] -  volinv*(dvdx[i3+6] * hourmodx +
                                                  dvdy[i3+6] * hourmody +
                                                  dvdz[i3+6] * hourmodz );

         hourgam7[i1] = gamma[i1][7] -  volinv*(dvdx[i3+7] * hourmodx +
                                                  dvdy[i3+7] * hourmody +
                                                  dvdz[i3+7] * hourmodz );

      }

      /* compute forces */
      /* store forces into h arrays (force arrays) */

      ss1=domain.m_ss[i2];
      mass1=domain.m_elemMass[i2];
      volume13=cbrt(determ[i2]);

      Index_t n0si2 = elemToNode[0];
      Index_t n1si2 = elemToNode[1];
      Index_t n2si2 = elemToNode[2];
      Index_t n3si2 = elemToNode[3];
      Index_t n4si2 = elemToNode[4];
      Index_t n5si2 = elemToNode[5];
      Index_t n6si2 = elemToNode[6];
      Index_t n7si2 = elemToNode[7];

      xd1[0] = domain.m_xd[n0si2];
      xd1[1] = domain.m_xd[n1si2];
      xd1[2] = domain.m_xd[n2si2];
      xd1[3] = domain.m_xd[n3si2];
      xd1[4] = domain.m_xd[n4si2];
      xd1[5] = domain.m_xd[n5si2];
      xd1[6] = domain.m_xd[n6si2];
      xd1[7] = domain.m_xd[n7si2];

      yd1[0] = domain.m_yd[n0si2];
      yd1[1] = domain.m_yd[n1si2];
      yd1[2] = domain.m_yd[n2si2];
      yd1[3] = domain.m_yd[n3si2];
      yd1[4] = domain.m_yd[n4si2];
      yd1[5] = domain.m_yd[n5si2];
      yd1[6] = domain.m_yd[n6si2];
      yd1[7] = domain.m_yd[n7si2];

      zd1[0] = domain.m_zd[n0si2];
      zd1[1] = domain.m_zd[n1si2];
      zd1[2] = domain.m_zd[n2si2];
      zd1[3] = domain.m_zd[n3si2];
      zd1[4] = domain.m_zd[n4si2];
      zd1[5] = domain.m_zd[n5si2];
      zd1[6] = domain.m_zd[n6si2];
      zd1[7] = domain.m_zd[n7si2];

      coefficient = - hourg * cast_Real_t(0.01) * ss1 * mass1 / volume13;

      CalcElemFBHourglassForce(xd1,yd1,zd1,
                      hourgam0,hourgam1,hourgam2,hourgam3,
                      hourgam4,hourgam5,hourgam6,hourgam7,
                      coefficient, hgfx, hgfy, hgfz);

      domain.m_fx[n0si2] += hgfx[0];
      domain.m_fy[n0si2] += hgfy[0];
      domain.m_fz[n0si2] += hgfz[0];

      domain.m_fx[n1si2] += hgfx[1];
      domain.m_fy[n1si2] += hgfy[1];
      domain.m_fz[n1si2] += hgfz[1];

      domain.m_fx[n2si2] += hgfx[2];
      domain.m_fy[n2si2] += hgfy[2];
      domain.m_fz[n2si2] += hgfz[2];

      domain.m_fx[n3si2] += hgfx[3];
      domain.m_fy[n3si2] += hgfy[3];
      domain.m_fz[n3si2] += hgfz[3];

      domain.m_fx[n4si2] += hgfx[4];
      domain.m_fy[n4si2] += hgfy[4];
      domain.m_fz[n4si2] += hgfz[4];

      domain.m_fx[n5si2] += hgfx[5];
      domain.m_fy[n5si2] += hgfy[5];
      domain.m_fz[n5si2] += hgfz[5];

      domain.m_fx[n6si2] += hgfx[6];
      domain.m_fy[n6si2] += hgfy[6];
      domain.m_fz[n6si2] += hgfz[6];

      domain.m_fx[n7si2] += hgfx[7];
      domain.m_fy[n7si2] += hgfy[7];
      domain.m_fz[n7si2] += hgfz[7];
   }
}

static inline
void CalcHourglassControlForElems(Real_t determ[], Real_t hgcoef)
{
   Index_t i, ii, jj ;
   Real_t  x1[EIGHT],  y1[EIGHT],  z1[EIGHT] ;
   Real_t pfx[EIGHT], pfy[EIGHT], pfz[EIGHT] ;
   Index_t numElem = domain.m_numElem ;
   Index_t numElem8 = numElem * EIGHT ;
   Real_t *dvdx = Allocate_Real_t(numElem8) ;
   Real_t *dvdy = Allocate_Real_t(numElem8) ;
   Real_t *dvdz = Allocate_Real_t(numElem8) ;
   Real_t *x8n  = Allocate_Real_t(numElem8) ;
   Real_t *y8n  = Allocate_Real_t(numElem8) ;
   Real_t *z8n  = Allocate_Real_t(numElem8) ;

   /* start loop over elements */
   for (i=0 ; i<numElem ; ++i){

      Index_t* elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i];
      CollectDomainNodesToElemNodes(elemToNode, x1, y1, z1);

      CalcElemVolumeDerivative(pfx, pfy, pfz, x1, y1, z1);

      /* load into temporary storage for FB Hour Glass control */
      for(ii=0;ii<EIGHT;++ii){
         jj=EIGHT*i+ii;

         dvdx[jj] = pfx[ii];
         dvdy[jj] = pfy[ii];
         dvdz[jj] = pfz[ii];
         x8n[jj]  = x1[ii];
         y8n[jj]  = y1[ii];
         z8n[jj]  = z1[ii];
      }

      determ[i] = domain.m_volo[i] * domain.m_v[i];

      /* Do a check for negative volumes */
      if ( domain.m_v[i] <= cast_Real_t(0.0) ) {
         exit(VolumeError) ;
      }
   }

   if ( hgcoef > cast_Real_t(0.) ) {
      CalcFBHourglassForceForElems(determ,x8n,y8n,z8n,dvdx,dvdy,dvdz,hgcoef) ;
   }

   Release_Real_t(z8n) ;
   Release_Real_t(y8n) ;
   Release_Real_t(x8n) ;
   Release_Real_t(dvdz) ;
   Release_Real_t(dvdy) ;
   Release_Real_t(dvdx) ;

   return ;
}

static inline
void CalcVolumeForceForElems()
{
   Index_t numElem = domain.m_numElem ;
   if (numElem != 0) {
      Real_t  hgcoef = domain.m_hgcoef ;
      Real_t *sigxx  = Allocate_Real_t(numElem) ;
      Real_t *sigyy  = Allocate_Real_t(numElem) ;
      Real_t *sigzz  = Allocate_Real_t(numElem) ;
      Real_t *determ = Allocate_Real_t(numElem) ;

      /* Sum contributions to total stress tensor */
      InitStressTermsForElems(numElem, sigxx, sigyy, sigzz);

      // call elemlib stress integration loop to produce nodal forces from
      // material stresses.
      IntegrateStressForElems( numElem, sigxx, sigyy, sigzz, determ) ;

      // check for negative element volume
      for ( Index_t k=0 ; k<numElem ; ++k ) {
         if (determ[k] <= cast_Real_t(0.0)) {
            exit(VolumeError) ;
         }
      }

      CalcHourglassControlForElems(determ, hgcoef) ;

      Release_Real_t(determ) ;
      Release_Real_t(sigzz) ;
      Release_Real_t(sigyy) ;
      Release_Real_t(sigxx) ;
   }
}

static inline void CalcForceForNodes()
{
  Index_t numNode = domain.m_numNode ;
  for (Index_t i=0; i<numNode; ++i) {
     domain.m_fx[i] = cast_Real_t(0.0) ;
     domain.m_fy[i] = cast_Real_t(0.0) ;
     domain.m_fz[i] = cast_Real_t(0.0) ;
  }

  /* Calcforce calls partial, force, hourq */
  CalcVolumeForceForElems() ;

  /* Calculate Nodal Forces at domain boundaries */
  /* problem->commSBN->Transfer(CommSBN::forces); */

}

static inline
void CalcAccelerationForNodes()
{
   Index_t numNode = domain.m_numNode ;
   for (Index_t i = 0; i < numNode; ++i) {
      domain.m_xdd[i] = domain.m_fx[i] / domain.m_nodalMass[i];
      domain.m_ydd[i] = domain.m_fy[i] / domain.m_nodalMass[i];
      domain.m_zdd[i] = domain.m_fz[i] / domain.m_nodalMass[i];
   }
}

static inline
void ApplyAccelerationBoundaryConditionsForNodes()
{
  Index_t numNodeBC = (domain.m_sizeX+1)*(domain.m_sizeX+1) ;
  for(Index_t i=0 ; i<numNodeBC ; ++i)
     domain.m_xdd[domain.m_symmX[i]] = cast_Real_t(0.0) ;

  for(Index_t i=0 ; i<numNodeBC ; ++i)
     domain.m_ydd[domain.m_symmY[i]] = cast_Real_t(0.0) ;

  for(Index_t i=0 ; i<numNodeBC ; ++i)
     domain.m_zdd[domain.m_symmZ[i]] = cast_Real_t(0.0) ;
}

static inline
void CalcVelocityForNodes(const Real_t dt, const Real_t u_cut)
{
   Index_t numNode = domain.m_numNode ;

   for ( Index_t i = 0 ; i < numNode ; ++i )
   {
     Real_t xdtmp, ydtmp, zdtmp ;

     xdtmp = domain.m_xd[i] + domain.m_xdd[i] * dt ;
     if( fabs(xdtmp) < u_cut ) xdtmp = cast_Real_t(0.0);
     domain.m_xd[i] = xdtmp ;

     ydtmp = domain.m_yd[i] + domain.m_ydd[i] * dt ;
     if( fabs(ydtmp) < u_cut ) ydtmp = cast_Real_t(0.0);
     domain.m_yd[i] = ydtmp ;

     zdtmp = domain.m_zd[i] + domain.m_zdd[i] * dt ;
     if( fabs(zdtmp) < u_cut ) zdtmp = cast_Real_t(0.0);
     domain.m_zd[i] = zdtmp ;
   }
}

static inline
void CalcPositionForNodes(const Real_t dt)
{
   Index_t numNode = domain.m_numNode ;

   for ( Index_t i = 0 ; i < numNode ; ++i )
   {
     domain.m_x[i] += domain.m_xd[i] * dt ;
     domain.m_y[i] += domain.m_yd[i] * dt ;
     domain.m_z[i] += domain.m_zd[i] * dt ;
   }
}

static inline
void LagrangeNodal()
{
  hcConst Real_t delt = domain.m_deltatime ;
  Real_t u_cut = domain.m_u_cut ;

  /* time of boundary condition evaluation is beginning of step for force and
   * acceleration boundary conditions. */
  CalcForceForNodes();

  CalcAccelerationForNodes();

  ApplyAccelerationBoundaryConditionsForNodes();

  CalcVelocityForNodes( delt, u_cut ) ;

  CalcPositionForNodes( delt );

  return;
}

static inline
Real_t CalcElemVolume_scalars( const Real_t x0, const Real_t x1,
               const Real_t x2, const Real_t x3,
               const Real_t x4, const Real_t x5,
               const Real_t x6, const Real_t x7,
               const Real_t y0, const Real_t y1,
               const Real_t y2, const Real_t y3,
               const Real_t y4, const Real_t y5,
               const Real_t y6, const Real_t y7,
               const Real_t z0, const Real_t z1,
               const Real_t z2, const Real_t z3,
               const Real_t z4, const Real_t z5,
               const Real_t z6, const Real_t z7 )
{
  Real_t twelveth = cast_Real_t(1.0)/cast_Real_t(12.0);

  Real_t dx61 = x6 - x1;
  Real_t dy61 = y6 - y1;
  Real_t dz61 = z6 - z1;

  Real_t dx70 = x7 - x0;
  Real_t dy70 = y7 - y0;
  Real_t dz70 = z7 - z0;

  Real_t dx63 = x6 - x3;
  Real_t dy63 = y6 - y3;
  Real_t dz63 = z6 - z3;

  Real_t dx20 = x2 - x0;
  Real_t dy20 = y2 - y0;
  Real_t dz20 = z2 - z0;

  Real_t dx50 = x5 - x0;
  Real_t dy50 = y5 - y0;
  Real_t dz50 = z5 - z0;

  Real_t dx64 = x6 - x4;
  Real_t dy64 = y6 - y4;
  Real_t dz64 = z6 - z4;

  Real_t dx31 = x3 - x1;
  Real_t dy31 = y3 - y1;
  Real_t dz31 = z3 - z1;

  Real_t dx72 = x7 - x2;
  Real_t dy72 = y7 - y2;
  Real_t dz72 = z7 - z2;

  Real_t dx43 = x4 - x3;
  Real_t dy43 = y4 - y3;
  Real_t dz43 = z4 - z3;

  Real_t dx57 = x5 - x7;
  Real_t dy57 = y5 - y7;
  Real_t dz57 = z5 - z7;

  Real_t dx14 = x1 - x4;
  Real_t dy14 = y1 - y4;
  Real_t dz14 = z1 - z4;

  Real_t dx25 = x2 - x5;
  Real_t dy25 = y2 - y5;
  Real_t dz25 = z2 - z5;

#define TRIPLE_PRODUCT(x1, y1, z1, x2, y2, z2, x3, y3, z3) \
   ((x1)*((y2)*(z3) - (z2)*(y3)) + (x2)*((z1)*(y3) - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)))

  Real_t volume =
    TRIPLE_PRODUCT(dx31 + dx72, dx63, dx20,
       dy31 + dy72, dy63, dy20,
       dz31 + dz72, dz63, dz20) +
    TRIPLE_PRODUCT(dx43 + dx57, dx64, dx70,
       dy43 + dy57, dy64, dy70,
       dz43 + dz57, dz64, dz70) +
    TRIPLE_PRODUCT(dx14 + dx25, dx61, dx50,
       dy14 + dy25, dy61, dy50,
       dz14 + dz25, dz61, dz50);

#undef TRIPLE_PRODUCT

  volume *= twelveth;

  return volume ;
}

static inline
Real_t CalcElemVolume( const Real_t x[EIGHT], const Real_t y[EIGHT], const Real_t z[EIGHT] )
{
return CalcElemVolume_scalars( x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7],
                       y[0], y[1], y[2], y[3], y[4], y[5], y[6], y[7],
                       z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
}

static inline
Real_t AreaFace( const Real_t x0, const Real_t x1,
                 const Real_t x2, const Real_t x3,
                 const Real_t y0, const Real_t y1,
                 const Real_t y2, const Real_t y3,
                 const Real_t z0, const Real_t z1,
                 const Real_t z2, const Real_t z3)
{
   Real_t fx = (x2 - x0) - (x3 - x1);
   Real_t fy = (y2 - y0) - (y3 - y1);
   Real_t fz = (z2 - z0) - (z3 - z1);
   Real_t gx = (x2 - x0) + (x3 - x1);
   Real_t gy = (y2 - y0) + (y3 - y1);
   Real_t gz = (z2 - z0) + (z3 - z1);
   Real_t area =
      (fx * fx + fy * fy + fz * fz) *
      (gx * gx + gy * gy + gz * gz) -
      (fx * gx + fy * gy + fz * gz) *
      (fx * gx + fy * gy + fz * gz);
   return area ;
}

static inline
Real_t CalcElemCharacteristicLength( const Real_t x[EIGHT],
                                     const Real_t y[EIGHT],
                                     const Real_t z[EIGHT],
                                     const Real_t volume)
{
   Real_t a, charLength = cast_Real_t(0.0);

   a = AreaFace(x[0],x[1],x[2],x[3],
                y[0],y[1],y[2],y[3],
                z[0],z[1],z[2],z[3]) ;
   charLength = fmax(a,charLength) ;

   a = AreaFace(x[4],x[5],x[6],x[7],
                y[4],y[5],y[6],y[7],
                z[4],z[5],z[6],z[7]) ;
   charLength = fmax(a,charLength) ;

   a = AreaFace(x[0],x[1],x[5],x[4],
                y[0],y[1],y[5],y[4],
                z[0],z[1],z[5],z[4]) ;
   charLength = fmax(a,charLength) ;

   a = AreaFace(x[1],x[2],x[6],x[5],
                y[1],y[2],y[6],y[5],
                z[1],z[2],z[6],z[5]) ;
   charLength = fmax(a,charLength) ;

   a = AreaFace(x[2],x[3],x[7],x[6],
                y[2],y[3],y[7],y[6],
                z[2],z[3],z[7],z[6]) ;
   charLength = fmax(a,charLength) ;

   a = AreaFace(x[3],x[0],x[4],x[7],
                y[3],y[0],y[4],y[7],
                z[3],z[0],z[4],z[7]) ;
   charLength = fmax(a,charLength) ;

   charLength = cast_Real_t(4.0) * volume / sqrt(charLength);

   return charLength;
}

static inline
void CalcElemVelocityGrandient( const Real_t* const xvel,
                                const Real_t* const yvel,
                                const Real_t* const zvel,
                                hcConst Real_t b[][EIGHT],
                                const Real_t detJ,
                                Real_t* const d )
{
  const Real_t inv_detJ = cast_Real_t(1.0) / detJ ;
  Real_t dyddx, dxddy, dzddx, dxddz, dzddy, dyddz;
  const Real_t* const pfx = b[0];
  const Real_t* const pfy = b[1];
  const Real_t* const pfz = b[2];

  d[0] = inv_detJ * ( pfx[0] * (xvel[0]-xvel[6])
                     + pfx[1] * (xvel[1]-xvel[7])
                     + pfx[2] * (xvel[2]-xvel[4])
                     + pfx[3] * (xvel[3]-xvel[5]) );

  d[1] = inv_detJ * ( pfy[0] * (yvel[0]-yvel[6])
                     + pfy[1] * (yvel[1]-yvel[7])
                     + pfy[2] * (yvel[2]-yvel[4])
                     + pfy[3] * (yvel[3]-yvel[5]) );

  d[2] = inv_detJ * ( pfz[0] * (zvel[0]-zvel[6])
                     + pfz[1] * (zvel[1]-zvel[7])
                     + pfz[2] * (zvel[2]-zvel[4])
                     + pfz[3] * (zvel[3]-zvel[5]) );

  dyddx  = inv_detJ * ( pfx[0] * (yvel[0]-yvel[6])
                      + pfx[1] * (yvel[1]-yvel[7])
                      + pfx[2] * (yvel[2]-yvel[4])
                      + pfx[3] * (yvel[3]-yvel[5]) );

  dxddy  = inv_detJ * ( pfy[0] * (xvel[0]-xvel[6])
                      + pfy[1] * (xvel[1]-xvel[7])
                      + pfy[2] * (xvel[2]-xvel[4])
                      + pfy[3] * (xvel[3]-xvel[5]) );

  dzddx  = inv_detJ * ( pfx[0] * (zvel[0]-zvel[6])
                      + pfx[1] * (zvel[1]-zvel[7])
                      + pfx[2] * (zvel[2]-zvel[4])
                      + pfx[3] * (zvel[3]-zvel[5]) );

  dxddz  = inv_detJ * ( pfz[0] * (xvel[0]-xvel[6])
                      + pfz[1] * (xvel[1]-xvel[7])
                      + pfz[2] * (xvel[2]-xvel[4])
                      + pfz[3] * (xvel[3]-xvel[5]) );

  dzddy  = inv_detJ * ( pfy[0] * (zvel[0]-zvel[6])
                      + pfy[1] * (zvel[1]-zvel[7])
                      + pfy[2] * (zvel[2]-zvel[4])
                      + pfy[3] * (zvel[3]-zvel[5]) );

  dyddz  = inv_detJ * ( pfz[0] * (yvel[0]-yvel[6])
                      + pfz[1] * (yvel[1]-yvel[7])
                      + pfz[2] * (yvel[2]-yvel[4])
                      + pfz[3] * (yvel[3]-yvel[5]) );
  d[5]  = cast_Real_t( .5) * ( dxddy + dyddx );
  d[4]  = cast_Real_t( .5) * ( dxddz + dzddx );
  d[3]  = cast_Real_t( .5) * ( dzddy + dyddz );
}

static inline
void CalcKinematicsForElems( Index_t numElem, Real_t dt )
{
  Real_t B[3][EIGHT] ; /** shape function derivatives */
  Real_t D[6] ;
  Real_t x_local[EIGHT] ;
  Real_t y_local[EIGHT] ;
  Real_t z_local[EIGHT] ;
  Real_t xd_local[EIGHT] ;
  Real_t yd_local[EIGHT] ;
  Real_t zd_local[EIGHT] ;
  Real_t detJ = cast_Real_t(0.0) ;

  // loop over all elements
  for( Index_t k=0 ; k<numElem ; ++k )
  {
    Real_t volume ;
    Real_t relativeVolume ;
    const Index_t* const elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*k] ;

    // get nodal coordinates from global arrays and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode )
    {
      Index_t gnode = elemToNode[lnode];
      x_local[lnode] = domain.m_x[gnode];
      y_local[lnode] = domain.m_y[gnode];
      z_local[lnode] = domain.m_z[gnode];
    }

    // volume calculations
    volume = CalcElemVolume(x_local, y_local, z_local );
    relativeVolume = volume / domain.m_volo[k] ;
    domain.m_vnew[k] = relativeVolume ;
    domain.m_delv[k] = relativeVolume - domain.m_v[k] ;

    // set characteristic length
    domain.m_arealg[k] = CalcElemCharacteristicLength(x_local,
                                                  y_local,
                                                  z_local,
                                                  volume);

    // get nodal velocities from global array and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode )
    {
      Index_t gnode = elemToNode[lnode];
      xd_local[lnode] = domain.m_xd[gnode];
      yd_local[lnode] = domain.m_yd[gnode];
      zd_local[lnode] = domain.m_zd[gnode];
    }

    Real_t dt2 = cast_Real_t(0.5) * dt;
    for ( Index_t j=0 ; j<EIGHT ; ++j )
    {
       x_local[j] -= dt2 * xd_local[j];
       y_local[j] -= dt2 * yd_local[j];
       z_local[j] -= dt2 * zd_local[j];
    }

    CalcElemShapeFunctionDerivatives( x_local,
                                          y_local,
                                          z_local,
                                          B, &detJ );

    CalcElemVelocityGrandient( xd_local,
                               yd_local,
                               zd_local,
                               B, detJ, D );

    // put velocity gradient quantities into their global arrays.
    domain.m_dxx[k] = D[0];
    domain.m_dyy[k] = D[1];
    domain.m_dzz[k] = D[2];
  }
}

static inline
void CalcLagrangeElements(Real_t deltatime)
{
   Index_t numElem = domain.m_numElem ;
   if (numElem > 0) {
      CalcKinematicsForElems(numElem, deltatime) ;

      // element loop to do some stuff not included in the elemlib function.
      for ( Index_t k=0 ; k<numElem ; ++k )
      {
        // calc strain rate and apply as constraint (only done in FB element)
        Real_t vdov = domain.m_dxx[k] + domain.m_dyy[k] + domain.m_dzz[k] ;
        Real_t vdovthird = vdov/cast_Real_t(3.0) ;
        
        // make the rate of deformation tensor deviatoric
        domain.m_vdov[k] = vdov ;
        domain.m_dxx[k] -= vdovthird ;
        domain.m_dyy[k] -= vdovthird ;
        domain.m_dzz[k] -= vdovthird ;

        // See if any volumes are negative, and take appropriate action.
        if (domain.m_vnew[k] <= cast_Real_t(0.0))
        {
           exit(VolumeError) ;
        }
      }
   }
}

static inline
void CalcMonotonicQGradientsForElems()
{
#define SUM4(a,b,c,d) (a + b + c + d)
   Index_t numElem = domain.m_numElem ;
   const Real_t ptiny = cast_Real_t(1.e-36) ;

   for (Index_t i = 0 ; i < numElem ; ++i ) {
      Real_t ax,ay,az ;
      Real_t dxv,dyv,dzv ;

      const Index_t *elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i];
      Index_t n0 = elemToNode[0] ;
      Index_t n1 = elemToNode[1] ;
      Index_t n2 = elemToNode[2] ;
      Index_t n3 = elemToNode[3] ;
      Index_t n4 = elemToNode[4] ;
      Index_t n5 = elemToNode[5] ;
      Index_t n6 = elemToNode[6] ;
      Index_t n7 = elemToNode[7] ;

      Real_t x0 = domain.m_x[n0] ;
      Real_t x1 = domain.m_x[n1] ;
      Real_t x2 = domain.m_x[n2] ;
      Real_t x3 = domain.m_x[n3] ;
      Real_t x4 = domain.m_x[n4] ;
      Real_t x5 = domain.m_x[n5] ;
      Real_t x6 = domain.m_x[n6] ;
      Real_t x7 = domain.m_x[n7] ;

      Real_t y0 = domain.m_y[n0] ;
      Real_t y1 = domain.m_y[n1] ;
      Real_t y2 = domain.m_y[n2] ;
      Real_t y3 = domain.m_y[n3] ;
      Real_t y4 = domain.m_y[n4] ;
      Real_t y5 = domain.m_y[n5] ;
      Real_t y6 = domain.m_y[n6] ;
      Real_t y7 = domain.m_y[n7] ;

      Real_t z0 = domain.m_z[n0] ;
      Real_t z1 = domain.m_z[n1] ;
      Real_t z2 = domain.m_z[n2] ;
      Real_t z3 = domain.m_z[n3] ;
      Real_t z4 = domain.m_z[n4] ;
      Real_t z5 = domain.m_z[n5] ;
      Real_t z6 = domain.m_z[n6] ;
      Real_t z7 = domain.m_z[n7] ;

      Real_t xv0 = domain.m_xd[n0] ;
      Real_t xv1 = domain.m_xd[n1] ;
      Real_t xv2 = domain.m_xd[n2] ;
      Real_t xv3 = domain.m_xd[n3] ;
      Real_t xv4 = domain.m_xd[n4] ;
      Real_t xv5 = domain.m_xd[n5] ;
      Real_t xv6 = domain.m_xd[n6] ;
      Real_t xv7 = domain.m_xd[n7] ;

      Real_t yv0 = domain.m_yd[n0] ;
      Real_t yv1 = domain.m_yd[n1] ;
      Real_t yv2 = domain.m_yd[n2] ;
      Real_t yv3 = domain.m_yd[n3] ;
      Real_t yv4 = domain.m_yd[n4] ;
      Real_t yv5 = domain.m_yd[n5] ;
      Real_t yv6 = domain.m_yd[n6] ;
      Real_t yv7 = domain.m_yd[n7] ;

      Real_t zv0 = domain.m_zd[n0] ;
      Real_t zv1 = domain.m_zd[n1] ;
      Real_t zv2 = domain.m_zd[n2] ;
      Real_t zv3 = domain.m_zd[n3] ;
      Real_t zv4 = domain.m_zd[n4] ;
      Real_t zv5 = domain.m_zd[n5] ;
      Real_t zv6 = domain.m_zd[n6] ;
      Real_t zv7 = domain.m_zd[n7] ;

      Real_t vol = domain.m_volo[i]*domain.m_vnew[i] ;
      Real_t norm = cast_Real_t(1.0) / ( vol + ptiny ) ;

      Real_t dxj = cast_Real_t(-0.25)*(SUM4(x0,x1,x5,x4) - SUM4(x3,x2,x6,x7)) ;
      Real_t dyj = cast_Real_t(-0.25)*(SUM4(y0,y1,y5,y4) - SUM4(y3,y2,y6,y7)) ;
      Real_t dzj = cast_Real_t(-0.25)*(SUM4(z0,z1,z5,z4) - SUM4(z3,z2,z6,z7)) ;

      Real_t dxi = cast_Real_t( 0.25)*(SUM4(x1,x2,x6,x5) - SUM4(x0,x3,x7,x4)) ;
      Real_t dyi = cast_Real_t( 0.25)*(SUM4(y1,y2,y6,y5) - SUM4(y0,y3,y7,y4)) ;
      Real_t dzi = cast_Real_t( 0.25)*(SUM4(z1,z2,z6,z5) - SUM4(z0,z3,z7,z4)) ;

      Real_t dxk = cast_Real_t( 0.25)*(SUM4(x4,x5,x6,x7) - SUM4(x0,x1,x2,x3)) ;
      Real_t dyk = cast_Real_t( 0.25)*(SUM4(y4,y5,y6,y7) - SUM4(y0,y1,y2,y3)) ;
      Real_t dzk = cast_Real_t( 0.25)*(SUM4(z4,z5,z6,z7) - SUM4(z0,z1,z2,z3)) ;

      /* find delvk and delxk ( i cross j ) */

      ax = dyi*dzj - dzi*dyj ;
      ay = dzi*dxj - dxi*dzj ;
      az = dxi*dyj - dyi*dxj ;

      domain.m_delx_zeta[i] = vol / sqrt(ax*ax + ay*ay + az*az + ptiny) ;

      ax *= norm ;
      ay *= norm ;
      az *= norm ;

      dxv = cast_Real_t(0.25)*(SUM4(xv4,xv5,xv6,xv7) - SUM4(xv0,xv1,xv2,xv3)) ;
      dyv = cast_Real_t(0.25)*(SUM4(yv4,yv5,yv6,yv7) - SUM4(yv0,yv1,yv2,yv3)) ;
      dzv = cast_Real_t(0.25)*(SUM4(zv4,zv5,zv6,zv7) - SUM4(zv0,zv1,zv2,zv3)) ;

      domain.m_delv_zeta[i] = ax*dxv + ay*dyv + az*dzv ;

      /* find delxi and delvi ( j cross k ) */

      ax = dyj*dzk - dzj*dyk ;
      ay = dzj*dxk - dxj*dzk ;
      az = dxj*dyk - dyj*dxk ;

      domain.m_delx_xi[i] = vol / sqrt(ax*ax + ay*ay + az*az + ptiny) ;

      ax *= norm ;
      ay *= norm ;
      az *= norm ;

      dxv = cast_Real_t(0.25)*(SUM4(xv1,xv2,xv6,xv5) - SUM4(xv0,xv3,xv7,xv4)) ;
      dyv = cast_Real_t(0.25)*(SUM4(yv1,yv2,yv6,yv5) - SUM4(yv0,yv3,yv7,yv4)) ;
      dzv = cast_Real_t(0.25)*(SUM4(zv1,zv2,zv6,zv5) - SUM4(zv0,zv3,zv7,zv4)) ;

      domain.m_delv_xi[i] = ax*dxv + ay*dyv + az*dzv ;

      /* find delxj and delvj ( k cross i ) */

      ax = dyk*dzi - dzk*dyi ;
      ay = dzk*dxi - dxk*dzi ;
      az = dxk*dyi - dyk*dxi ;

      domain.m_delx_eta[i] = vol / sqrt(ax*ax + ay*ay + az*az + ptiny) ;

      ax *= norm ;
      ay *= norm ;
      az *= norm ;

      dxv = cast_Real_t(-0.25)*(SUM4(xv0,xv1,xv5,xv4) - SUM4(xv3,xv2,xv6,xv7)) ;
      dyv = cast_Real_t(-0.25)*(SUM4(yv0,yv1,yv5,yv4) - SUM4(yv3,yv2,yv6,yv7)) ;
      dzv = cast_Real_t(-0.25)*(SUM4(zv0,zv1,zv5,zv4) - SUM4(zv3,zv2,zv6,zv7)) ;

      domain.m_delv_eta[i] = ax*dxv + ay*dyv + az*dzv ;
   }
#undef SUM4
}

static inline
void CalcMonotonicQRegionForElems(// parameters
                          Real_t qlc_monoq,
                          Real_t qqc_monoq,
                          Real_t monoq_limiter_mult,
                          Real_t monoq_max_slope,
                          Real_t ptiny,

                          // the elementset length
                          Index_t elength )
{
   for ( Index_t ielem = 0 ; ielem < elength; ++ielem ) {
      Real_t qlin, qquad ;
      Real_t phixi, phieta, phizeta ;
      Index_t i = domain.m_matElemlist[ielem];
      Int_t bcMask = domain.m_elemBC[i] ;
      Real_t delvm, delvp ;

      /*  phixi     */
      Real_t norm = cast_Real_t(1.) / ( domain.m_delv_xi[i] + ptiny ) ;

      switch (bcMask & XI_M) {
         case 0:         delvm = domain.m_delv_xi[domain.m_lxim[i]] ; break ;
         case XI_M_SYMM: delvm = domain.m_delv_xi[i] ;                break ;
         case XI_M_FREE: delvm = cast_Real_t(0.0) ;                   break ;
         default:        /* ERROR */ ;                                break ;
      }
      switch (bcMask & XI_P) {
         case 0:         delvp = domain.m_delv_xi[domain.m_lxip[i]] ; break ;
         case XI_P_SYMM: delvp = domain.m_delv_xi[i] ;                break ;
         case XI_P_FREE: delvp = cast_Real_t(0.0) ;                   break ;
         default:        /* ERROR */ ;                                break ;
      }

      delvm = delvm * norm ;
      delvp = delvp * norm ;

      phixi = cast_Real_t(.5) * ( delvm + delvp ) ;

      delvm *= monoq_limiter_mult ;
      delvp *= monoq_limiter_mult ;

      if ( delvm < phixi ) phixi = delvm ;
      if ( delvp < phixi ) phixi = delvp ;
      if ( phixi < cast_Real_t(0.)) phixi = cast_Real_t(0.) ;
      if ( phixi > monoq_max_slope) phixi = monoq_max_slope;


      /*  phieta     */
      norm = cast_Real_t(1.) / ( domain.m_delv_eta[i] + ptiny ) ;

      switch (bcMask & ETA_M) {
         case 0:          delvm = domain.m_delv_eta[domain.m_letam[i]] ; break ;
         case ETA_M_SYMM: delvm = domain.m_delv_eta[i] ;                 break ;
         case ETA_M_FREE: delvm = cast_Real_t(0.0) ;                     break ;
         default:         /* ERROR */ ;                                  break ;
      }
      switch (bcMask & ETA_P) {
         case 0:          delvp = domain.m_delv_eta[domain.m_letap[i]] ; break ;
         case ETA_P_SYMM: delvp = domain.m_delv_eta[i] ;                 break ;
         case ETA_P_FREE: delvp = cast_Real_t(0.0) ;                     break ;
         default:         /* ERROR */ ;                                  break ;
      }

      delvm = delvm * norm ;
      delvp = delvp * norm ;

      phieta = cast_Real_t(.5) * ( delvm + delvp ) ;

      delvm *= monoq_limiter_mult ;
      delvp *= monoq_limiter_mult ;

      if ( delvm  < phieta ) phieta = delvm ;
      if ( delvp  < phieta ) phieta = delvp ;
      if ( phieta < cast_Real_t(0.)) phieta = cast_Real_t(0.) ;
      if ( phieta > monoq_max_slope)  phieta = monoq_max_slope;

      /*  phizeta     */
      norm = cast_Real_t(1.) / ( domain.m_delv_zeta[i] + ptiny ) ;

      switch (bcMask & ZETA_M) {
         case 0:           delvm = domain.m_delv_zeta[domain.m_lzetam[i]] ; break ;
         case ZETA_M_SYMM: delvm = domain.m_delv_zeta[i] ;                  break ;
         case ZETA_M_FREE: delvm = cast_Real_t(0.0) ;                       break ;
         default:          /* ERROR */ ;                                    break ;
      }
      switch (bcMask & ZETA_P) {
         case 0:           delvp = domain.m_delv_zeta[domain.m_lzetap[i]] ; break ;
         case ZETA_P_SYMM: delvp = domain.m_delv_zeta[i] ;                  break ;
         case ZETA_P_FREE: delvp = cast_Real_t(0.0) ;                       break ;
         default:          /* ERROR */ ;                                    break ;
      }

      delvm = delvm * norm ;
      delvp = delvp * norm ;

      phizeta = cast_Real_t(.5) * ( delvm + delvp ) ;

      delvm *= monoq_limiter_mult ;
      delvp *= monoq_limiter_mult ;

      if ( delvm   < phizeta ) phizeta = delvm ;
      if ( delvp   < phizeta ) phizeta = delvp ;
      if ( phizeta < cast_Real_t(0.)) phizeta = cast_Real_t(0.);
      if ( phizeta > monoq_max_slope  ) phizeta = monoq_max_slope;

      /* Remove length scale */

      if ( domain.m_vdov[i] > cast_Real_t(0.) )  {
         qlin  = cast_Real_t(0.) ;
         qquad = cast_Real_t(0.) ;
      }
      else {
         Real_t delvxxi   = domain.m_delv_xi[i]   * domain.m_delx_xi[i]   ;
         Real_t delvxeta  = domain.m_delv_eta[i]  * domain.m_delx_eta[i]  ;
         Real_t delvxzeta = domain.m_delv_zeta[i] * domain.m_delx_zeta[i] ;

         if ( delvxxi   > cast_Real_t(0.) ) delvxxi   = cast_Real_t(0.) ;
         if ( delvxeta  > cast_Real_t(0.) ) delvxeta  = cast_Real_t(0.) ;
         if ( delvxzeta > cast_Real_t(0.) ) delvxzeta = cast_Real_t(0.) ;

         Real_t rho = domain.m_elemMass[i] / (domain.m_volo[i] * domain.m_vnew[i]) ;

         qlin = -qlc_monoq * rho *
            (  delvxxi   * (cast_Real_t(1.) - phixi) +
               delvxeta  * (cast_Real_t(1.) - phieta) +
               delvxzeta * (cast_Real_t(1.) - phizeta)  ) ;

         qquad = qqc_monoq * rho *
            (  delvxxi*delvxxi     * (cast_Real_t(1.) - phixi*phixi) +
               delvxeta*delvxeta   * (cast_Real_t(1.) - phieta*phieta) +
               delvxzeta*delvxzeta * (cast_Real_t(1.) - phizeta*phizeta)  ) ;
      }

      domain.m_qq[i] = qquad ;
      domain.m_ql[i] = qlin  ;
   }
}

static inline
void CalcMonotonicQForElems()
{  
   //
   // initialize parameters
   // 
   const Real_t ptiny        = cast_Real_t(1.e-36) ;
   Real_t monoq_max_slope    = domain.m_monoq_max_slope ;
   Real_t monoq_limiter_mult = domain.m_monoq_limiter_mult ;

   //
   // calculate the monotonic q for pure regions
   //
   Index_t elength = domain.m_numElem ;
   if (elength > 0) {
      Real_t qlc_monoq = domain.m_qlc_monoq;
      Real_t qqc_monoq = domain.m_qqc_monoq;
      CalcMonotonicQRegionForElems(// parameters
                           qlc_monoq,
                           qqc_monoq,
                           monoq_limiter_mult,
                           monoq_max_slope,
                           ptiny,

                           // the elemset length
                           elength );
   }
}

static inline
void CalcQForElems()
{
   Real_t qstop = domain.m_qstop ;
   Index_t numElem = domain.m_numElem ;

   //
   // MONOTONIC Q option
   //

   /* Calculate velocity gradients */
   CalcMonotonicQGradientsForElems() ;

   /* Transfer veloctiy gradients in the first order elements */
   /* problem->commElements->Transfer(CommElements::monoQ) ; */
   CalcMonotonicQForElems() ;

   /* Don't allow excessive artificial viscosity */
   if (numElem != 0) {
      Index_t idx = -1; 
      for (Index_t i=0; i<numElem; ++i) {
         if ( domain.m_q[i] > qstop ) {
            idx = i ;
            break ;
         }
      }

      if(idx >= 0) {
         exit(QStopError) ;
      }
   }
}

static inline
void CalcPressureForElems(Real_t* p_new, Real_t* bvc,
                          Real_t* pbvc, Real_t* e_old,
                          Real_t* compression, Real_t *vnewc,
                          Real_t pmin,
                          Real_t p_cut, Real_t eosvmax,
                          Index_t length)
{
   Real_t c1s = cast_Real_t(2.0)/cast_Real_t(3.0) ;
   for (Index_t i = 0; i < length ; ++i) {
      bvc[i] = c1s * (compression[i] + cast_Real_t(1.));
      pbvc[i] = c1s;
   }

   for (Index_t i = 0 ; i < length ; ++i){
      p_new[i] = bvc[i] * e_old[i] ;

      if    (fabs(p_new[i]) <  p_cut   )
         p_new[i] = cast_Real_t(0.0) ;

      if    ( vnewc[i] >= eosvmax ) /* impossible condition here? */
         p_new[i] = cast_Real_t(0.0) ;

      if    (p_new[i]       <  pmin)
         p_new[i]   = pmin ;
   }
}

static inline
void CalcEnergyForElems(Real_t* p_new, Real_t* e_new, Real_t* q_new,
                        Real_t* bvc, Real_t* pbvc,
                        Real_t* p_old, Real_t* e_old, Real_t* q_old,
                        Real_t* compression, Real_t* compHalfStep,
                        Real_t* vnewc, Real_t* work, Real_t* delvc, Real_t pmin,
                        Real_t p_cut, Real_t  e_cut, Real_t q_cut, Real_t emin,
                        Real_t* qq, Real_t* ql,
                        Real_t rho0,
                        Real_t eosvmax,
                        Index_t length)
{
   const Real_t sixth = cast_Real_t(1.0) / cast_Real_t(6.0) ;
   Real_t *pHalfStep = Allocate_Real_t(length) ;

   for (Index_t i = 0 ; i < length ; ++i) {
      e_new[i] = e_old[i] - cast_Real_t(0.5) * delvc[i] * (p_old[i] + q_old[i])
         + cast_Real_t(0.5) * work[i];

      if (e_new[i]  < emin ) {
         e_new[i] = emin ;
      }
   }

   CalcPressureForElems(pHalfStep, bvc, pbvc, e_new, compHalfStep, vnewc,
                   pmin, p_cut, eosvmax, length);

   for (Index_t i = 0 ; i < length ; ++i) {
      Real_t vhalf = cast_Real_t(1.) / (cast_Real_t(1.) + compHalfStep[i]) ;

      if ( delvc[i] > cast_Real_t(0.) ) {
         q_new[i] /* = qq[i] = ql[i] */ = cast_Real_t(0.) ;
      }
      else {
         Real_t ssc = ( pbvc[i] * e_new[i]
                 + vhalf * vhalf * bvc[i] * pHalfStep[i] ) / rho0 ;

         if ( ssc <= cast_Real_t(0.) ) {
            ssc =cast_Real_t(.333333e-36) ;
         } else {
            ssc = sqrt(ssc) ;
         }

         q_new[i] = (ssc*ql[i] + qq[i]) ;
      }

      e_new[i] = e_new[i] + cast_Real_t(0.5) * delvc[i]
         * (  cast_Real_t(3.0)*(p_old[i]     + q_old[i])
              - cast_Real_t(4.0)*(pHalfStep[i] + q_new[i])) ;
   }

   for (Index_t i = 0 ; i < length ; ++i) {

      e_new[i] += cast_Real_t(0.5) * work[i];

      if (fabs(e_new[i]) < e_cut) {
         e_new[i] = cast_Real_t(0.)  ;
      }
      if (     e_new[i]  < emin ) {
         e_new[i] = emin ;
      }
   }

   CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                   pmin, p_cut, eosvmax, length);

   for (Index_t i = 0 ; i < length ; ++i){
      Real_t q_tilde ;

      if (delvc[i] > cast_Real_t(0.)) {
         q_tilde = cast_Real_t(0.) ;
      }
      else {
         Real_t ssc = ( pbvc[i] * e_new[i]
                 + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

         if ( ssc <= cast_Real_t(0.) ) {
            ssc = cast_Real_t(.333333e-36) ;
         } else {
            ssc = sqrt(ssc) ;
         }

         q_tilde = (ssc*ql[i] + qq[i]) ;
      }

      e_new[i] = e_new[i] - (  cast_Real_t(7.0)*(p_old[i]     + q_old[i])
                               - cast_Real_t(8.0)*(pHalfStep[i] + q_new[i])
                               + (p_new[i] + q_tilde)) * delvc[i]*sixth ;

      if (fabs(e_new[i]) < e_cut) {
         e_new[i] = cast_Real_t(0.)  ;
      }
      if (     e_new[i]  < emin ) {
         e_new[i] = emin ;
      }
   }

   CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                   pmin, p_cut, eosvmax, length);

   for (Index_t i = 0 ; i < length ; ++i){

      if ( delvc[i] <= cast_Real_t(0.) ) {
         Real_t ssc = ( pbvc[i] * e_new[i]
                 + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

         if ( ssc <= cast_Real_t(0.) ) {
            ssc = cast_Real_t(.333333e-36) ;
         } else {
            ssc = sqrt(ssc) ;
         }

         q_new[i] = (ssc*ql[i] + qq[i]) ;

         if (fabs(q_new[i]) < q_cut) q_new[i] = cast_Real_t(0.) ;
      }
   }

   Release_Real_t(pHalfStep) ;

   return ;
}

static inline
void CalcSoundSpeedForElems(Real_t *vnewc, Real_t rho0, Real_t *enewc,
                            Real_t *pnewc, Real_t *pbvc,
                            Real_t *bvc, Real_t ss4o3, Index_t nz)
{
   for (Index_t i = 0; i < nz ; ++i) {
      Index_t iz = domain.m_matElemlist[i];
      Real_t ssTmp = (pbvc[i] * enewc[i] + vnewc[i] * vnewc[i] *
                 bvc[i] * pnewc[i]) / rho0;
      if (ssTmp <= cast_Real_t(1.111111e-36)) {
         ssTmp = cast_Real_t(1.111111e-36);
      }
      domain.m_ss[iz] = sqrt(ssTmp);
   }
}

static inline
void EvalEOSForElems(Real_t *vnewc, Index_t length)
{
   Real_t  e_cut = domain.m_e_cut;
   Real_t  p_cut = domain.m_p_cut;
   Real_t  ss4o3 = domain.m_ss4o3;
   Real_t  q_cut = domain.m_q_cut;

   Real_t eosvmax = domain.m_eosvmax ;
   Real_t eosvmin = domain.m_eosvmin ;
   Real_t pmin    = domain.m_pmin ;
   Real_t emin    = domain.m_emin ;
   Real_t rho0    = domain.m_refdens ;

   Real_t *e_old = Allocate_Real_t(length) ;
   Real_t *delvc = Allocate_Real_t(length) ;
   Real_t *p_old = Allocate_Real_t(length) ;
   Real_t *q_old = Allocate_Real_t(length) ;
   Real_t *compression = Allocate_Real_t(length) ;
   Real_t *compHalfStep = Allocate_Real_t(length) ;
   Real_t *qq = Allocate_Real_t(length) ;
   Real_t *ql = Allocate_Real_t(length) ;
   Real_t *work = Allocate_Real_t(length) ;
   Real_t *p_new = Allocate_Real_t(length) ;
   Real_t *e_new = Allocate_Real_t(length) ;
   Real_t *q_new = Allocate_Real_t(length) ;
   Real_t *bvc = Allocate_Real_t(length) ;
   Real_t *pbvc = Allocate_Real_t(length) ;

   /* compress data, minimal set */
   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      e_old[i] = domain.m_e[zidx] ;
   }

   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      delvc[i] = domain.m_delv[zidx] ;
   }

   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      p_old[i] = domain.m_p[zidx] ;
   }

   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      q_old[i] = domain.m_q[zidx] ;
   }

   for (Index_t i = 0; i < length ; ++i) {
      Real_t vchalf ;
      compression[i] = cast_Real_t(1.) / vnewc[i] - cast_Real_t(1.);
      vchalf = vnewc[i] - delvc[i] * cast_Real_t(.5);
      compHalfStep[i] = cast_Real_t(1.) / vchalf - cast_Real_t(1.);
   }

   /* Check for v > eosvmax or v < eosvmin */
   if ( eosvmin != cast_Real_t(0.) ) {
      for(Index_t i=0 ; i<length ; ++i) {
         if (vnewc[i] <= eosvmin) { /* impossible due to calling func? */
            compHalfStep[i] = compression[i] ;
         }
      }
   }
   if ( eosvmax != cast_Real_t(0.) ) {
      for(Index_t i=0 ; i<length ; ++i) {
         if (vnewc[i] >= eosvmax) { /* impossible due to calling func? */
            p_old[i]        = cast_Real_t(0.) ;
            compression[i]  = cast_Real_t(0.) ;
            compHalfStep[i] = cast_Real_t(0.) ;
         }
      }
   }

   for (Index_t i = 0 ; i < length ; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      qq[i] = domain.m_qq[zidx] ;
      ql[i] = domain.m_ql[zidx] ;
      work[i] = cast_Real_t(0.) ; 
   }

   CalcEnergyForElems(p_new, e_new, q_new, bvc, pbvc,
                 p_old, e_old,  q_old, compression, compHalfStep,
                 vnewc, work,  delvc, pmin,
                 p_cut, e_cut, q_cut, emin,
                 qq, ql, rho0, eosvmax, length);


   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      domain.m_p[zidx] = p_new[i] ;
   }

   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      domain.m_e[zidx] = e_new[i] ;
   }

   for (Index_t i=0; i<length; ++i) {
      Index_t zidx = domain.m_matElemlist[i] ;
      domain.m_q[zidx] = q_new[i] ;
   }

   CalcSoundSpeedForElems(vnewc, rho0, e_new, p_new,
             pbvc, bvc, ss4o3, length) ;

   Release_Real_t(pbvc) ;
   Release_Real_t(bvc) ;
   Release_Real_t(q_new) ;
   Release_Real_t(e_new) ;
   Release_Real_t(p_new) ;
   Release_Real_t(work) ;
   Release_Real_t(ql) ;
   Release_Real_t(qq) ;
   Release_Real_t(compHalfStep) ;
   Release_Real_t(compression) ;
   Release_Real_t(q_old) ;
   Release_Real_t(p_old) ;
   Release_Real_t(delvc) ;
   Release_Real_t(e_old) ;
}

static inline
void ApplyMaterialPropertiesForElems()
{
  Index_t length = domain.m_numElem ;

  if (length != 0) {
    /* Expose all of the variables needed for material evaluation */
    Real_t eosvmin = domain.m_eosvmin ;
    Real_t eosvmax = domain.m_eosvmax ;
    Real_t *vnewc = Allocate_Real_t(length) ;

    for (Index_t i=0 ; i<length ; ++i) {
       Index_t zn = domain.m_matElemlist[i] ;
       vnewc[i] = domain.m_vnew[zn] ;
    }

    if (eosvmin != cast_Real_t(0.)) {
       for(Index_t i=0 ; i<length ; ++i) {
          if (vnewc[i] < eosvmin)
             vnewc[i] = eosvmin ;
       }
    }

    if (eosvmax != cast_Real_t(0.)) {
       for(Index_t i=0 ; i<length ; ++i) {
          if (vnewc[i] > eosvmax)
             vnewc[i] = eosvmax ;
       }
    }

    for (Index_t i=0; i<length; ++i) {
       Index_t zn = domain.m_matElemlist[i] ;
       Real_t vc = domain.m_v[zn] ;
       if (eosvmin != cast_Real_t(0.)) {
          if (vc < eosvmin)
             vc = eosvmin ;
       }
       if (eosvmax != cast_Real_t(0.)) {
          if (vc > eosvmax)
             vc = eosvmax ;
       }
       if (vc <= 0.) {
          exit(VolumeError) ;
       }
    }

    EvalEOSForElems(vnewc, length);

    Release_Real_t(vnewc) ;

  }
}

static inline
void UpdateVolumesForElems()
{
   Index_t numElem = domain.m_numElem;
   if (numElem != 0) {
      Real_t v_cut = domain.m_v_cut;

      for(Index_t i=0 ; i<numElem ; ++i) {
         Real_t tmpV ;
         tmpV = domain.m_vnew[i] ;

         if ( fabs(tmpV - cast_Real_t(1.0)) < v_cut )
            tmpV = cast_Real_t(1.0) ;
         domain.m_v[i] = tmpV ;
      }
   }

   return ;
}

static inline
void LagrangeElements()
{
  const Real_t deltatime = domain.m_deltatime ;

  CalcLagrangeElements(deltatime) ;

  /* Calculate Q.  (Monotonic q option requires communication) */
  CalcQForElems() ;

  ApplyMaterialPropertiesForElems() ;

  UpdateVolumesForElems() ;
}

static inline
void CalcCourantConstraintForElems()
{
   Real_t dtcourant = cast_Real_t(1.0e+20) ;
   Index_t   courant_elem = -1 ;
   Real_t      qqc = domain.m_qqc ;
   Index_t length = domain.m_numElem ;

   Real_t  qqc2 = cast_Real_t(64.0) * qqc * qqc ;

   for (Index_t i = 0 ; i < length ; ++i) {
      Index_t indx = domain.m_matElemlist[i] ;

      Real_t dtf = domain.m_ss[indx] * domain.m_ss[indx] ;

      if ( domain.m_vdov[indx] < cast_Real_t(0.) ) {

         dtf = dtf
            + qqc2 * domain.m_arealg[indx] * domain.m_arealg[indx]
            * domain.m_vdov[indx] * domain.m_vdov[indx] ;
      }

      dtf = sqrt(dtf) ;

      dtf = domain.m_arealg[indx] / dtf ;

   /* determine minimum timestep with its corresponding elem */
      if (domain.m_vdov[indx] != cast_Real_t(0.)) {
         if ( dtf < dtcourant ) {
            dtcourant = dtf ;
            courant_elem = indx ;
         }
      }
   }

   /* Don't try to register a time constraint if none of the elements
    * were active */
   if (courant_elem != -1) {
      domain.m_dtcourant = dtcourant ;
   }

   return ;
}

static inline
void CalcHydroConstraintForElems()
{
   Real_t dthydro = cast_Real_t(1.0e+20) ;
   Index_t hydro_elem = -1 ;
   Real_t dvovmax = domain.m_dvovmax ;
   Index_t length = domain.m_numElem ;

   for (Index_t i = 0 ; i < length ; ++i) {
      Index_t indx = domain.m_matElemlist[i] ;

      if (domain.m_vdov[indx] != cast_Real_t(0.)) {
         Real_t dtdvov = dvovmax / (fabs(domain.m_vdov[indx])+cast_Real_t(1.e-20)) ;
         if ( dthydro > dtdvov ) {
            dthydro = dtdvov ;
            hydro_elem = indx ;
         }
      }
   }

   if (hydro_elem != -1) {
      domain.m_dthydro = dthydro ;
   }

   return ;
}

static inline
void CalcTimeConstraintsForElems() {
   /* evaluate time constraint */
   CalcCourantConstraintForElems() ;

   /* check hydro constraint */
   CalcHydroConstraintForElems() ;
}

static inline
void LagrangeLeapFrog()
{
   /* calculate nodal forces, accelerations, velocities, positions, with
    * applied boundary conditions and slide surface considerations */
   LagrangeNodal();

   /* calculate element quantities (i.e. velocity gradient & q), and update
    * material states */
   LagrangeElements();

   CalcTimeConstraintsForElems();

   // LagrangeRelease() ;  Creation/destruction of temps may be important to capture 
}

#ifdef HAB_C
static Real_t x_local[EIGHT], y_local[EIGHT], z_local[EIGHT] ;
#endif

int main(int argc, char *argv[])
{
   Index_t edgeElems = 45 ;
   Index_t edgeNodes = edgeElems+1 ;
   // Real_t ds = cast_Real_t(1.125)/cast_Real_t(edgeElems) ; /* may accumulate roundoff */
   Real_t tx, ty, tz ;
   Index_t nidx, zidx ;
   Index_t domElems ;

   /* get run options to measure various metrics */

   /* ... */

   /****************************/
   /*   Initialize Sedov Mesh  */
   /****************************/

   /* construct a uniform box for this processor */

   domain.m_sizeX   = edgeElems ;
   domain.m_sizeY   = edgeElems ;
   domain.m_sizeZ   = edgeElems ;
   domain.m_numElem = edgeElems*edgeElems*edgeElems ;
   domain.m_numNode = edgeNodes*edgeNodes*edgeNodes ;

   domElems = domain.m_numElem ;


   /* allocate field memory */

   domain_AllocateElemPersistent(domain.m_numElem) ;
   domain_AllocateElemTemporary (domain.m_numElem) ;

   domain_AllocateNodalPersistent(domain.m_numNode) ;
   domain_AllocateNodesets(edgeNodes*edgeNodes) ;


   /* initialize nodal coordinates */

   nidx = 0 ;
   tz  = cast_Real_t(0.) ;
   for (Index_t plane=0; plane<edgeNodes; ++plane) {
      ty = cast_Real_t(0.) ;
      for (Index_t row=0; row<edgeNodes; ++row) {
         tx = cast_Real_t(0.) ;
         for (Index_t col=0; col<edgeNodes; ++col) {
            domain.m_x[nidx] = tx ;
            domain.m_y[nidx] = ty ;
            domain.m_z[nidx] = tz ;
            ++nidx ;
            // tx += ds ; /* may accumulate roundoff... */
            tx = cast_Real_t(1.125)*cast_Real_t(col+1)/cast_Real_t(edgeElems) ;
         }
         // ty += ds ;  /* may accumulate roundoff... */
         ty = cast_Real_t(1.125)*cast_Real_t(row+1)/cast_Real_t(edgeElems) ;
      }
      // tz += ds ;  /* may accumulate roundoff... */
      tz = cast_Real_t(1.125)*cast_Real_t(plane+1)/cast_Real_t(edgeElems) ;
   }


   /* embed hexehedral elements in nodal point lattice */

   nidx = 0 ;
   zidx = 0 ;
   for (Index_t plane=0; plane<edgeElems; ++plane) {
      for (Index_t row=0; row<edgeElems; ++row) {
         for (Index_t col=0; col<edgeElems; ++col) {
            Index_t *localNode = (Index_t *)&domain.m_nodelist[EIGHT*zidx] ;
            localNode[0] = nidx                                       ;
            localNode[1] = nidx                                   + 1 ;
            localNode[2] = nidx                       + edgeNodes + 1 ;
            localNode[3] = nidx                       + edgeNodes     ;
            localNode[4] = nidx + edgeNodes*edgeNodes                 ;
            localNode[5] = nidx + edgeNodes*edgeNodes             + 1 ;
            localNode[6] = nidx + edgeNodes*edgeNodes + edgeNodes + 1 ;
            localNode[7] = nidx + edgeNodes*edgeNodes + edgeNodes     ;
            ++zidx ;
            ++nidx ;
         }
         ++nidx ;
      }
      nidx += edgeNodes ;
   }

   /* Create a material IndexSet (entire domain same material for now) */
   for (Index_t i=0; i<domElems; ++i) {
      domain.m_matElemlist[i] = i ;
   }
   
   /* initialize material parameters */
   domain.m_dtfixed            = cast_Real_t(-1.0e-7) ;
   domain.m_deltatime          = cast_Real_t(1.0e-7) ;
   domain.m_deltatimemultlb    = cast_Real_t(1.1) ;
   domain.m_deltatimemultub    = cast_Real_t(1.2) ;
   domain.m_stoptime           = cast_Real_t(1.0e-2) ;
   domain.m_dtcourant          = cast_Real_t(1.0e+20) ;
   domain.m_dthydro            = cast_Real_t(1.0e+20) ;
   domain.m_dtmax              = cast_Real_t(1.0e-2) ;
   domain.m_time               = cast_Real_t(0.) ;
   domain.m_cycle              = 0 ;

   domain.m_e_cut              = cast_Real_t(1.0e-7) ;
   domain.m_p_cut              = cast_Real_t(1.0e-7) ;
   domain.m_q_cut              = cast_Real_t(1.0e-7) ;
   domain.m_u_cut              = cast_Real_t(1.0e-7) ;
   domain.m_v_cut              = cast_Real_t(1.0e-10) ;

   domain.m_hgcoef             = cast_Real_t(3.0) ;
   domain.m_ss4o3              = cast_Real_t(4.0)/cast_Real_t(3.0) ;

   domain.m_qstop              =  cast_Real_t(1.0e+12) ;
   domain.m_monoq_max_slope    =  cast_Real_t(1.0) ;
   domain.m_monoq_limiter_mult =  cast_Real_t(2.0) ;
   domain.m_qlc_monoq          = cast_Real_t(0.5) ;
   domain.m_qqc_monoq          = cast_Real_t(2.0)/cast_Real_t(3.0) ;
   domain.m_qqc                = cast_Real_t(2.0) ;

   domain.m_pmin               =  cast_Real_t(0.) ;
   domain.m_emin               = cast_Real_t(-1.0e+15) ;

   domain.m_dvovmax            =  cast_Real_t(0.1) ;

   domain.m_eosvmax            =  cast_Real_t(1.0e+9) ;
   domain.m_eosvmin            =  cast_Real_t(1.0e-9) ;

   domain.m_refdens            =  cast_Real_t(1.0) ;

   /* initialize field data */
   for (Index_t i=0; i<domElems; ++i) {
#ifndef HAB_C
      Real_t x_local[EIGHT], y_local[EIGHT], z_local[EIGHT] ;
#endif
      Index_t *elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i] ;
      for( Index_t lnode=0 ; lnode < EIGHT ; ++lnode )
      {
        Index_t gnode = elemToNode[lnode];
        x_local[lnode] = domain.m_x[gnode];
        y_local[lnode] = domain.m_y[gnode];
        z_local[lnode] = domain.m_z[gnode];
      }

      // volume calculations
      Real_t volume = CalcElemVolume(x_local, y_local, z_local );
      domain.m_volo[i] = volume ;
      domain.m_elemMass[i] = volume ;
      for (Index_t j=0; j<EIGHT; ++j) {
         Index_t idx = elemToNode[j] ;
         domain.m_nodalMass[idx] += volume / cast_Real_t(8.0) ;
      }
   }

   /* deposit energy */
   domain.m_e[0] = cast_Real_t(3.948746e+7) ;

   /* set up symmetry nodesets */
   nidx = 0 ;
   for (Index_t i=0; i<edgeNodes; ++i) {
      Index_t planeInc = i*edgeNodes*edgeNodes ;
      Index_t rowInc   = i*edgeNodes ;
      for (Index_t j=0; j<edgeNodes; ++j) {
         domain.m_symmX[nidx] = planeInc + j*edgeNodes ;
         domain.m_symmY[nidx] = planeInc + j ;
         domain.m_symmZ[nidx] = rowInc   + j ;
         ++nidx ;
      }
   }

   /* set up elemement connectivity information */
   domain.m_lxim[0] = 0 ;
   for (Index_t i=1; i<domElems; ++i) {
      domain.m_lxim[i]   = i-1 ;
      domain.m_lxip[i-1] = i ;
   }
   domain.m_lxip[domElems-1] = domElems-1 ;

   for (Index_t i=0; i<edgeElems; ++i) {
      domain.m_letam[i] = i ; 
      domain.m_letap[domElems-edgeElems+i] = domElems-edgeElems+i ;
   }
   for (Index_t i=edgeElems; i<domElems; ++i) {
      domain.m_letam[i] = i-edgeElems ;
      domain.m_letap[i-edgeElems] = i ;
   }

   for (Index_t i=0; i<edgeElems*edgeElems; ++i) {
      domain.m_lzetam[i] = i ;
      domain.m_lzetap[domElems-edgeElems*edgeElems+i] = domElems-edgeElems*edgeElems+i ;
   }
   for (Index_t i=edgeElems*edgeElems; i<domElems; ++i) {
      domain.m_lzetam[i] = i - edgeElems*edgeElems ;
      domain.m_lzetap[i-edgeElems*edgeElems] = i ;
   }

   /* set up boundary condition information */
   for (Index_t i=0; i<domElems; ++i) {
      domain.m_elemBC[i] = 0 ;  /* clear BCs by default */
   }

   /* faces on "external" boundaries will be */
   /* symmetry plane or free surface BCs */
   for (Index_t i=0; i<edgeElems; ++i) {
      Index_t planeInc = i*edgeElems*edgeElems ;
      Index_t rowInc   = i*edgeElems ;
      for (Index_t j=0; j<edgeElems; ++j) {
         domain.m_elemBC[planeInc+j*edgeElems] |= XI_M_SYMM ;
         domain.m_elemBC[planeInc+j*edgeElems+edgeElems-1] |= XI_P_FREE ;
         domain.m_elemBC[planeInc+j] |= ETA_M_SYMM ;
         domain.m_elemBC[planeInc+j+edgeElems*edgeElems-edgeElems] |= ETA_P_FREE ;
         domain.m_elemBC[rowInc+j] |= ZETA_M_SYMM ;
         domain.m_elemBC[rowInc+j+domElems-edgeElems*edgeElems] |= ZETA_P_FREE ;
      }
   }

   /* timestep to solution */
   while(domain.m_time < domain.m_stoptime ) {
      TimeIncrement() ;
      LagrangeLeapFrog() ;
      /* problem->commNodes->Transfer(CommNodes::syncposvel) ; */
#if LULESH_SHOW_PROGRESS
      printf("time = %e, dt=%e, e(0) = %e\n",
             ((double)domain.m_time),((double)domain.m_deltatime), (double)domain.m_e[0] ) ;
#endif
   }

   printf("   Final Origin Energy = %12.6e \n", (double)domain.m_e[0]);

   return 0 ;
}

