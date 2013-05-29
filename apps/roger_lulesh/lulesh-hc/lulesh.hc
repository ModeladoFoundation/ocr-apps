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
#include <stdint.h>

#ifdef HAB_C
#include "hc.h"
#include <math.h>
// ATOMIC MEMORY OPERATIONS
#include "AMO.h"
// MACROS FOR STRIPMINING, TILING and PARALLELIZATION
#include "RAG_habanaro.h"

#else // NOT HAB_C
#include <tgmath.h>
#endif // HAB_C

#define LULESH_SHOW_PROGRESS 1

enum { VolumeError = -1, QStopError = -2 } ;

/****************************************************/
/* Allow flexibility for arithmetic representations */
/****************************************************/

/* Could also support fixed point and interval arithmetic types */
typedef float        real4 ;
typedef double       real8 ;
typedef long double  real10 ;  /* 10 bytes on x86 */

typedef int32_t  Index_t ; /* array subscript and loop index */
typedef real8    Real_t ;  /* floating point representation */
typedef uint32_t Int_t ;   /* integer representation */

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

// The NumberOfDimensions
#define THREE ((Index_t)3)
// Either 2^3/2 or 2^2 not sure which
#define FOUR ((Index_t)4)
// Either 6 or 2*3
#define SIX ((Index_t)6)
// 2^NumberOfDimensions
#define EIGHT ((Index_t)8)
// small BLK_SIZE for small problem
#define BLK_SIZE (4)

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

struct Domain {

   /******************/
   /* Implementation */
   /******************/

   /* Node-centered */

   Real_t  *m_x ;          /* coordinates */
   Real_t  *m_y ; 
   Real_t  *m_z ;

   Real_t  *m_xd ;         /* velocities */
   Real_t  *m_yd ; 
   Real_t  *m_zd ; 

   Real_t  *m_xdd ;        /* accelerations */
   Real_t  *m_ydd ;
   Real_t  *m_zdd ;

   Real_t  *m_fx ;         /* forces */
   Real_t  *m_fy ;
   Real_t  *m_fz ;

   Real_t  *m_nodalMass ;  /* mass */

   Index_t *m_symmX ;      /* symmetry plane nodesets */
   Index_t *m_symmY ;
   Index_t *m_symmZ ;

   /* Element-centered */

   Index_t *m_matElemlist ;  /* material indexset */
   Index_t *m_nodelist ;     /* elemToNode connectivity */

   Index_t *m_lxim ;         /* element connectivity across each face */
   Index_t *m_lxip ;
   Index_t *m_letam ;
   Index_t *m_letap ;
   Index_t *m_lzetam ;
   Index_t *m_lzetap ;

   Int_t   *m_elemBC ;       /* symmetry/free-surface flags for each elem face */

   Real_t  *m_dxx ;          /* principal strains -- temporary */
   Real_t  *m_dyy ;
   Real_t  *m_dzz ;

   Real_t  *m_delv_xi ;      /* velocity gradient -- temporary */
   Real_t  *m_delv_eta ;
   Real_t  *m_delv_zeta ;

   Real_t  *m_delx_xi ;      /* coordinate gradient -- temporary */
   Real_t  *m_delx_eta ;
   Real_t  *m_delx_zeta ;
   
   Real_t  *m_e ;            /* energy */

   Real_t  *m_p ;            /* pressure */
   Real_t  *m_q ;            /* q */
   Real_t  *m_ql ;           /* linear term for q */
   Real_t  *m_qq ;           /* quadratic term for q */

   Real_t  *m_v ;            /* relative volume */
   Real_t  *m_volo ;         /* reference volume */
   Real_t  *m_vnew ;         /* new relative volume -- temporary */
   Real_t  *m_delv ;         /* m_vnew - m_v */
   Real_t  *m_vdov ;         /* volume derivative over volume */

   Real_t  *m_arealg ;       /* characteristic length of an element */
   
   Real_t  *m_ss ;           /* "sound speed" */

   Real_t  *m_elemMass ;     /* mass */

   /* Parameters */

   Real_t  m_dtfixed ;           /* fixed time increment */
   Real_t  m_time ;              /* current time */
   Real_t  m_deltatime ;         /* variable time increment */
   Real_t  m_deltatimemultlb ;
   Real_t  m_deltatimemultub ;
   Real_t  m_stoptime ;          /* end time for simulation */

   Real_t  m_u_cut ;             /* velocity tolerance */
   Real_t  m_hgcoef ;            /* hourglass control */
   Real_t  m_qstop ;             /* excessive q indicator */
   Real_t  m_monoq_max_slope ;
   Real_t  m_monoq_limiter_mult ;
   Real_t  m_e_cut ;             /* energy tolerance */
   Real_t  m_p_cut ;             /* pressure tolerance */
   Real_t  m_ss4o3 ;
   Real_t  m_q_cut ;             /* q tolerance */
   Real_t  m_v_cut ;             /* relative volume tolerance */
   Real_t  m_qlc_monoq ;         /* linear term coef for q */
   Real_t  m_qqc_monoq ;         /* quadratic term coef for q */
   Real_t  m_qqc ;
   Real_t  m_eosvmax ;
   Real_t  m_eosvmin ;
   Real_t  m_pmin ;              /* pressure floor */
   Real_t  m_emin ;              /* energy floor */
   Real_t  m_dvovmax ;           /* maximum allowable volume change */
   Real_t  m_refdens ;           /* reference density */

   Real_t  m_dtcourant ;         /* courant constraint */
   Real_t  m_dthydro ;           /* volume change constraint */
   Real_t  m_dtmax ;             /* maximum allowable time increment */

   Int_t   m_cycle ;             /* iteration count for simulation */

   Index_t m_sizeX ;           /* X,Y,Z extent of this block */
   Index_t m_sizeY ;
   Index_t m_sizeZ ;

   Index_t m_numElem ;         /* Elements/Nodes in this domain */
   Index_t m_numNode ;
} domain ;

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

   /**********/
   /* Access */
   /**********/

   /* Node-centered */

#define domain_x(idx)           domain.m_x[(Index_t)(idx)]
#define domain_y(idx)           domain.m_y[(Index_t)(idx)]
#define domain_z(idx)           domain.m_z[(Index_t)(idx)]

#define domain_xd(idx)          domain.m_xd[(Index_t)(idx)]
#define domain_yd(idx)          domain.m_yd[(Index_t)(idx)]
#define domain_zd(idx)          domain.m_zd[(Index_t)(idx)]

#define domain_xdd(idx)         domain.m_xdd[(Index_t)(idx)]
#define domain_ydd(idx)         domain.m_ydd[(Index_t)(idx)]
#define domain_zdd(idx)         domain.m_zdd[(Index_t)(idx)]

#define domain_fx(idx)          domain.m_fx[(Index_t)(idx)]
#define domain_fy(idx)          domain.m_fy[(Index_t)(idx)]
#define domain_fz(idx)          domain.m_fz[(Index_t)(idx)]

#define domain_nodalMass(idx)   domain.m_nodalMass[(Index_t)(idx)]

#define domain_symmX(idx)       domain.m_symmX[(Index_t)(idx)]
#define domain_symmY(idx)       domain.m_symmY[(Index_t)(idx)]
#define domain_symmZ(idx)       domain.m_symmZ[(Index_t)(idx)]

   /* Element-centered */

#define domain_matElemlist(idx) domain.m_matElemlist[(Index_t)(idx)]
#define domain_nodelist(idx)    domain.m_nodelist[EIGHT*(Index_t)(idx)]
#define domain_nodelist_ref(idx) ((Index_t *)&domain.m_nodelist[EIGHT*(Index_t)(idx)])

#define domain_lxim(idx)        domain.m_lxim[(Index_t)(idx)]
#define domain_lxip(idx)        domain.m_lxip[(Index_t)(idx)]
#define domain_letam(idx)       domain.m_letam[(Index_t)(idx)]
#define domain_letap(idx)       domain.m_letap[(Index_t)(idx)]
#define domain_lzetam(idx)      domain.m_lzetam[(Index_t)(idx)]
#define domain_lzetap(idx)      domain.m_lzetap[(Index_t)(idx)]
 
#define domain_elemBC(idx)      domain.m_elemBC[(Index_t)(idx)]

#define domain_dxx(idx)         domain.m_dxx[idx]
#define domain_dyy(idx)         domain.m_dyy[idx]
#define domain_dzz(idx)         domain.m_dzz[idx]

#define domain_delv_xi(idx)     domain.m_delv_xi[(Index_t)(idx)]
#define domain_delv_eta(idx)    domain.m_delv_eta[(Index_t)(idx)]
#define domain_delv_zeta(idx)   domain.m_delv_zeta[(Index_t)(idx)]

#define domain_delx_xi(idx)     domain.m_delx_xi[(Index_t)(idx)]
#define domain_delx_eta(idx)    domain.m_delx_eta[(Index_t)(idx)]
#define domain_delx_zeta(idx)   domain.m_delx_zeta[(Index_t)(idx)]

#define domain_e(idx)           domain.m_e[(Index_t)(idx)]

#define domain_p(idx)           domain.m_p[(Index_t)(idx)]
#define domain_q(idx)           domain.m_q[(Index_t)(idx)]
#define domain_ql(idx)          domain.m_ql[(Index_t)(idx)]
#define domain_qq(idx)          domain.m_qq[(Index_t)(idx)]

#define domain_v(idx)           domain.m_v[(Index_t)(idx)]
#define domain_volo(idx)        domain.m_volo[(Index_t)(idx)]
#define domain_vnew(idx)        domain.m_vnew[(Index_t)(idx)]
#define domain_delv(idx)        domain.m_delv[(Index_t)(idx)]
#define domain_vdov(idx)        domain.m_vdov[(Index_t)(idx)]

#define domain_arealg(idx)      domain.m_arealg[(Index_t)(idx)]

#define domain_ss(idx)          domain.m_ss[(Index_t)(idx)]

#define domain_elemMass(idx)    domain.m_elemMass[(Index_t)(idx)]

   /* Params */

#define domain_dtfixed()              domain.m_dtfixed
#define domain_time()                 domain.m_time
#define domain_deltatime()            domain.m_deltatime
#define domain_deltatimemultlb()      domain.m_deltatimemultlb
#define domain_deltatimemultub()      domain.m_deltatimemultub
#define domain_stoptime()             domain.m_stoptime

#define domain_u_cut()                domain.m_u_cut
#define domain_hgcoef()               domain.m_hgcoef
#define domain_qstop()                domain.m_qstop
#define domain_monoq_max_slope()      domain.m_monoq_max_slope
#define domain_monoq_limiter_mult()   domain.m_monoq_limiter_mult
#define domain_e_cut()                domain.m_e_cut
#define domain_p_cut()                domain.m_p_cut
#define domain_ss4o3()                domain.m_ss4o3
#define domain_q_cut()                domain.m_q_cut
#define domain_v_cut()                domain.m_v_cut
#define domain_qlc_monoq()            domain.m_qlc_monoq
#define domain_qqc_monoq()            domain.m_qqc_monoq
#define domain_qqc()                  domain.m_qqc
#define domain_eosvmax()              domain.m_eosvmax
#define domain_eosvmin()              domain.m_eosvmin
#define domain_pmin()                 domain.m_pmin
#define domain_emin()                 domain.m_emin
#define domain_dvovmax()              domain.m_dvovmax
#define domain_refdens()              domain.m_refdens

#define domain_dtcourant()            domain.m_dtcourant
#define domain_dthydro()              domain.m_dthydro
#define domain_dtmax()                domain.m_dtmax

#define domain_cycle()                domain.m_cycle

#define domain_sizeX()                domain.m_sizeX
#define domain_sizeY()                domain.m_sizeY
#define domain_sizeZ()                domain.m_sizeZ
#define domain_numElem()              domain.m_numElem
#define domain_numNode()              domain.m_numNode

// static inline   Int_t   *Allocate_Int_t(size_t hcSize) { return   (Int_t *)malloc(hcSize*  sizeof(Int_t)); }
static inline  Real_t  *Allocate_Real_t(size_t hcSize) { return  (Real_t *)malloc(hcSize* sizeof(Real_t)); }
// static inline Index_t *Allocate_Index_t(size_t hcSize) { return (Index_t *)malloc(hcSize*sizeof(Index_t)); }

void   Release_Int_t(  Int_t *ptr) { if(ptr != NULL) { free(ptr); } }
void  Release_Real_t( Real_t *ptr) { if(ptr != NULL) { free(ptr); } }
void Release_Index_t(Index_t *ptr) { if(ptr != NULL) { free(ptr); } }

#define   cast_Int_t(arg) (  (Int_t)(arg))
#define  cast_Real_t(arg) ( (Real_t)(arg))
#define cast_Index_t(arg) ((Index_t)(arg))

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

   /**************/
   /* Allocation */
   /**************/

void
domain_AllocateNodalPersistent(size_t hcSize) {
   if(domain.m_x != NULL)free(domain.m_x); domain.m_x = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_y != NULL)free(domain.m_y); domain.m_y = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_z != NULL)free(domain.m_z); domain.m_z = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_xd != NULL)free(domain.m_xd); domain.m_xd = malloc(hcSize*sizeof(Real_t));
   if(domain.m_yd != NULL)free(domain.m_yd); domain.m_yd = malloc(hcSize*sizeof(Real_t));
   if(domain.m_zd != NULL)free(domain.m_zd); domain.m_zd = malloc(hcSize*sizeof(Real_t));

   if(domain.m_xdd != NULL)free(domain.m_xdd); domain.m_xdd = malloc(hcSize*sizeof(Real_t));
   if(domain.m_ydd != NULL)free(domain.m_ydd); domain.m_ydd = malloc(hcSize*sizeof(Real_t));
   if(domain.m_zdd != NULL)free(domain.m_zdd); domain.m_zdd = malloc(hcSize*sizeof(Real_t));

   if(domain.m_fx != NULL)free(domain.m_fx); domain.m_fx = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_fy != NULL)free(domain.m_fy); domain.m_fy = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_fz != NULL)free(domain.m_fz); domain.m_fz = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_nodalMass != NULL)free(domain.m_nodalMass); domain.m_nodalMass = malloc(hcSize*sizeof(Real_t));

   FINISH
      FOR_OUTER_0xNx1(i,hcSize)
        ASYNC_IN_1(i,domain) // INDEX and GLOBAL
          FOR_INNER(i)
            domain.m_xd[i] = (Real_t)(0.0);
            domain.m_yd[i] = (Real_t)(0.0);
            domain.m_zd[i] = (Real_t)(0.0);

            domain.m_xdd[i] = (Real_t)(0.0);
            domain.m_ydd[i] = (Real_t)(0.0);
            domain.m_zdd[i] = (Real_t)(0.0) ;

            domain.m_nodalMass[i] = (Real_t)(0.0) ;
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
   END_FINISH
}

void
domain_AllocateElemPersistent(size_t hcSize) {
   if(domain.m_matElemlist != NULL)free(domain.m_matElemlist); domain.m_matElemlist = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_nodelist != NULL)free(domain.m_nodelist); domain.m_nodelist= malloc(EIGHT*hcSize*sizeof(Index_t)) ;

   if(domain.m_lxim != NULL)free(domain.m_lxim); domain.m_lxim = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_lxip != NULL)free(domain.m_lxip); domain.m_lxip = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_letam != NULL)free(domain.m_letam); domain.m_letam = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_letap != NULL)free(domain.m_letap); domain.m_letap = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_lzetam != NULL)free(domain.m_lzetam); domain.m_lzetam = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_lzetap != NULL)free(domain.m_lzetap); domain.m_lzetap = malloc(hcSize*sizeof(Index_t)) ;

   if(domain.m_elemBC != NULL)free(domain.m_elemBC); domain.m_elemBC = malloc(hcSize*sizeof(Int_t)) ;

   if(domain.m_e != NULL)free(domain.m_e); domain.m_e = malloc(hcSize*sizeof(Real_t));

   if(domain.m_p != NULL)free(domain.m_p); domain.m_p = malloc(hcSize*sizeof(Real_t));

   if(domain.m_q != NULL)free(domain.m_q); domain.m_q = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_ql != NULL)free(domain.m_ql); domain.m_ql = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_qq != NULL)free(domain.m_qq); domain.m_qq = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_v != NULL)free(domain.m_v); domain.m_v = malloc(hcSize*sizeof(Real_t));
   if(domain.m_volo != NULL)free(domain.m_volo); domain.m_volo = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_delv != NULL)free(domain.m_delv); domain.m_delv = malloc(hcSize*sizeof(Real_t)) ;
   if(domain.m_vdov != NULL)free(domain.m_vdov); domain.m_vdov = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_arealg != NULL)free(domain.m_arealg); domain.m_arealg = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_ss != NULL)free(domain.m_ss); domain.m_ss = malloc(hcSize*sizeof(Real_t)) ;

   if(domain.m_elemMass != NULL)free(domain.m_elemMass); domain.m_elemMass = malloc(hcSize*sizeof(Real_t)) ;

   FINISH 
      FOR_OUTER_0xNx1(i,hcSize)
        ASYNC_IN_1(i,domain) // INDEX and GLOBAL
          FOR_INNER(i)
            domain.m_e[i] = (Real_t)(0.0);
            domain.m_p[i] = (Real_t)(0.0);
            domain.m_v[i] = (Real_t)(1.0);
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
   END_FINISH
}

   /* Temporaries should not be initialized in bulk but */
   /* this is a runnable placeholder for now */
void
domain_AllocateElemTemporary(size_t hcSize) {
  if(domain.m_dxx != NULL)free(domain.m_dxx); domain.m_dxx = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_dyy != NULL)free(domain.m_dyy); domain.m_dyy = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_dzz != NULL)free(domain.m_dzz); domain.m_dzz = malloc(hcSize*sizeof(Real_t)) ;

  if(domain.m_delv_xi != NULL)free(domain.m_delv_xi); domain.m_delv_xi = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_delv_eta != NULL)free(domain.m_delv_eta); domain.m_delv_eta = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_delv_zeta != NULL)free(domain.m_delv_zeta); domain.m_delv_zeta = malloc(hcSize*sizeof(Real_t)) ;

  if(domain.m_delx_xi != NULL)free(domain.m_delx_xi); domain.m_delx_xi = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_delx_eta != NULL)free(domain.m_delx_eta); domain.m_delx_eta = malloc(hcSize*sizeof(Real_t)) ;
  if(domain.m_delx_zeta != NULL)free(domain.m_delx_zeta); domain.m_delx_zeta = malloc(hcSize*sizeof(Real_t)) ;

  if(domain.m_vnew != NULL)free(domain.m_vnew); domain.m_vnew = malloc(hcSize*sizeof(Real_t)) ;
}

void
domain_AllocateNodesets(size_t hcSize) {
   if(domain.m_symmX != NULL)free(domain.m_symmX); domain.m_symmX = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_symmY != NULL)free(domain.m_symmY); domain.m_symmY = malloc(hcSize*sizeof(Index_t)) ;
   if(domain.m_symmZ != NULL)free(domain.m_symmZ); domain.m_symmZ = malloc(hcSize*sizeof(Index_t)) ;
}

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
   FINISH 
      FOR_OUTER_0xNx1(i,numElem)
        ASYNC_IN_1(i,sigxx,sigyy,sigzz,domain) // INDEX, PARAMSs and GLOBAL
          FOR_INNER(i)
            sigxx[i] =  sigyy[i] = sigzz[i] =  - domain.m_p[i] - domain.m_q[i] ;
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
   END_FINISH
}

static inline
void CalcElemShapeFunctionDerivatives( const Real_t* const x, /* IN */
                                       const Real_t* const y, /* IN */
                                       const Real_t* const z, /* IN */
                                       Real_t b[][EIGHT],     /* OUT */
                                       Real_t* const volume ) /* OUT */
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
void CalcElemNodeNormals(Real_t pfx[EIGHT],     /* OUT */
                         Real_t pfy[EIGHT],     /* OUT */
                         Real_t pfz[EIGHT],     /* OUT */
                         const Real_t x[EIGHT], /* IN */
                         const Real_t y[EIGHT], /* IN */
                         const Real_t z[EIGHT]) /* IN */
{
   for( Index_t i = 0 ; i < EIGHT ; ++i ) {
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
void SumElemStressesToNodeForces( hcConst Real_t B[][EIGHT], /* IN */
                                  const Real_t stress_xx,    /* IN */
                                  const Real_t stress_yy,    /* IN */
                                  const Real_t stress_zz,    /* IN */
                                  Real_t* const fx,          /* OUT */
                                  Real_t* const fy,          /* OUT */
                                  Real_t* const fz )         /* OUT */
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
  // loop over all elements
  FINISH
    FOR_OUTER_0xNx1(k,numElem) 
      ASYNC_IN_1(k, numElem, sigxx, sigyy, sigzz, determ, domain ) // INDEX, PARAMs and GLOBAL
#ifdef HAB_C
        Real_t *B        = malloc(THREE*EIGHT*sizeof(Real_t)) ;
        Real_t *x_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *y_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *z_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *fx_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *fy_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *fz_local = malloc(EIGHT*sizeof(Real_t)) ;
#else // NOT HAB_C
        Real_t B[THREE][EIGHT] -- shape function derivatives
        Real_t  x_local[EIGHT],  y_local[EIGHT],  z_local[EIGHT];
        Real_t fx_local[EIGHT], fy_local[EIGHT], fz_local[EIGHT],
#endif // HAB_C
        FOR_INNER(k) 
          hcConst Index_t* hcConst elemNodes = (Index_t *)&domain.m_nodelist[EIGHT*k];

          // get nodal coordinates from global arrays and copy into local arrays.
          for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
            Index_t gnode = elemNodes[lnode];
            x_local[lnode] = domain.m_x[gnode];
            y_local[lnode] = domain.m_y[gnode];
            z_local[lnode] = domain.m_z[gnode];
          } // for lnode
          /* Volume calculation involves extra work for numerical consistency. */
          CalcElemShapeFunctionDerivatives(x_local, y_local, z_local,
                                           B, &determ[k]);

          CalcElemNodeNormals( &B[0*EIGHT], &B[1*EIGHT], &B[2*EIGHT],
                               x_local, y_local, z_local );

          SumElemStressesToNodeForces( B, sigxx[k], sigyy[k], sigzz[k],
                                          fx_local, fy_local, fz_local ) ;
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
          // accumulate nodal force contributions to global force arrray.
          for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
            Index_t gnode = elemNodes[lnode];
            AMO__sync_addition_double(&domain.m_fx[gnode], fx_local[lnode]);
            AMO__sync_addition_double(&domain.m_fy[gnode], fy_local[lnode]);
            AMO__sync_addition_double(&domain.m_fz[gnode], fz_local[lnode]);
          } // for lnode
        END_FOR_INNER(k)
#ifdef HAB_C
        free(fz_local) ; free(fy_local) ; free(fx_local) ;
        free(z_local)  ; free(y_local)  ; free(x_local) ;
        free(B) ;
#endif // HAB_C
      END_ASYNC_IN_1(k)
    END_FOR_OUTER(k)
  END_FINISH
} // IntegrateStressForElems()

static inline
void CollectDomainNodesToElemNodes(const Index_t* elemToNode,
                                   Real_t elemX[EIGHT],
                                   Real_t elemY[EIGHT],
                                   Real_t elemZ[EIGHT])
{
// RAG -- nd(0..7)i = elemToNode[0..7]
   Index_t nd0i = elemToNode[0] ;
   Index_t nd1i = elemToNode[1] ;
   Index_t nd2i = elemToNode[2] ;
   Index_t nd3i = elemToNode[3] ;
   Index_t nd4i = elemToNode[4] ;
   Index_t nd5i = elemToNode[5] ;
   Index_t nd6i = elemToNode[6] ;
   Index_t nd7i = elemToNode[7] ;
// RAG -- GATHER elemX[0..7] = domain.m_x[elemToNode[0..7]]
   elemX[0] = domain.m_x[nd0i];
   elemX[1] = domain.m_x[nd1i];
   elemX[2] = domain.m_x[nd2i];
   elemX[3] = domain.m_x[nd3i];
   elemX[4] = domain.m_x[nd4i];
   elemX[5] = domain.m_x[nd5i];
   elemX[6] = domain.m_x[nd6i];
   elemX[7] = domain.m_x[nd7i];
// RAG -- GATHER elemY[0..7] = domain.m_y[elemToNode[0..7]]
   elemY[0] = domain.m_y[nd0i];
   elemY[1] = domain.m_y[nd1i];
   elemY[2] = domain.m_y[nd2i];
   elemY[3] = domain.m_y[nd3i];
   elemY[4] = domain.m_y[nd4i];
   elemY[5] = domain.m_y[nd5i];
   elemY[6] = domain.m_y[nd6i];
   elemY[7] = domain.m_y[nd7i];
// RAG -- GATHER elemZ[0..7] = domain.m_z[elemToNode[0..7]]
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
void VoluDer(const Real_t x0, const Real_t x1, const Real_t x2, /* IN */
             const Real_t x3, const Real_t x4, const Real_t x5, /* IN */
             const Real_t y0, const Real_t y1, const Real_t y2, /* IN */
             const Real_t y3, const Real_t y4, const Real_t y5, /* IN */
             const Real_t z0, const Real_t z1, const Real_t z2, /* IN */
             const Real_t z3, const Real_t z4, const Real_t z5, /* IN */
             Real_t* dvdx, Real_t* dvdy, Real_t* dvdz)          /* OUT */
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
void CalcElemVolumeDerivative(Real_t dvdx[EIGHT],    /* OUT */
                              Real_t dvdy[EIGHT],    /* OUT */
                              Real_t dvdz[EIGHT],    /* OUT */
                              const Real_t x[EIGHT], /* IN */
                              const Real_t y[EIGHT], /* IN */
                              const Real_t z[EIGHT]) /* IN */
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
void CalcElemFBHourglassForce(Real_t *xd, Real_t *yd, Real_t *zd,        /* IN */
                              Real_t *hourgam0, Real_t *hourgam1,        /* IN */
                              Real_t *hourgam2, Real_t *hourgam3,        /* IN */
                              Real_t *hourgam4, Real_t *hourgam5,        /* IN */
                              Real_t *hourgam6, Real_t *hourgam7,        /* IN */
                              Real_t coefficient,                        /* IN */
                              Real_t *hgfx, Real_t *hgfy, Real_t *hgfz ) /* OUT */
{
// RAG --   h(x|y|z)[0][0..3] =        MatMul( d(x|y|z)[0][0..7] , hourgam [0..7][0..3] )
// RAG -- hgf(x|y|x)[0][0..7] = coef * MatMul( h(x|y|z)[0][0..3] , hourgamT[0..7][0..3] )
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

// RAG -- GAMMA now compile time constants in rodata
static const Real_t GAMMA[FOUR*EIGHT] = {
( 1.), ( 1.), (-1.), (-1.), (-1.), (-1.), ( 1.), ( 1.),
( 1.), (-1.), (-1.), ( 1.), (-1.), ( 1.), ( 1.), (-1.),
( 1.), (-1.), ( 1.), (-1.), ( 1.), (-1.), ( 1.), (-1.),
(-1.), ( 1.), (-1.), ( 1.), ( 1.), (-1.), ( 1.), (-1.),
};

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


/*************************************************/
/*    compute the hourglass modes */

  FINISH
    FOR_OUTER_0xNx1(i2,numElem)
      ASYNC_IN_1( i2, domain, GAMMA,                      // INDEX and GLOBALs
             determ, x8n,y8n,z8n, dvdx,dvdy,dvdz, hourg ) // PARAMs
#ifdef HAB_C
        Real_t *hgfx = malloc(EIGHT*sizeof(Real_t));
        Real_t *hgfy = malloc(EIGHT*sizeof(Real_t));
        Real_t *hgfz = malloc(EIGHT*sizeof(Real_t));

        Real_t *hourgam0 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam1 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam2 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam3 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam4 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam5 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam6 = malloc(FOUR*sizeof(Real_t));
        Real_t *hourgam7 = malloc(FOUR*sizeof(Real_t));

        Real_t *xd1 = malloc(EIGHT*sizeof(Real_t));
        Real_t *yd1 = malloc(EIGHT*sizeof(Real_t));
        Real_t *zd1 = malloc(EIGHT*sizeof(Real_t));

        Index_t *elemToNode  = malloc(EIGHT*sizeof(Index_t)) ;
#else // NOT HAB_C
        Real_t hgfx[EIGHT], hgfy[EIGHT], hgfz[EIGHT];

        Real_t hourgam0[FOUR], hourgam1[FOUR], hourgam2[FOUR], hourgam3[FOUR];
        Real_t hourgam4[FOUR], hourgam5[FOUR], hourgam6[FOUR], hourgam7[FOUR];

        Real_t  xd1[EIGHT],  yd1[EIGHT],  zd1[EIGHT];

        Index_t *elemToNode  = malloc(EIGHT*sizeof(Index_t)) ;
#endif // HAB_C
        FOR_INNER(i2) 

          Real_t coefficient;

          Index_t i3=EIGHT*i2;

          Real_t volinv=cast_Real_t(1.0)/determ[i2];
          Real_t ss1, mass1, volume13 ;


          // RAG -- GATHER/SCATTER Index Values
          for( Index_t i=0 ; i<EIGHT ; ++i ) {
            elemToNode[i] = *(Index_t *)&domain.m_nodelist[EIGHT*i2+i];
          } // for i

          for( Index_t i1=0 ; i1<FOUR ; ++i1 ){
            Real_t hourmodx =
              x8n[i3+0] * GAMMA[i1*EIGHT+0] + x8n[i3+1] * GAMMA[i1*EIGHT+1] +
              x8n[i3+2] * GAMMA[i1*EIGHT+2] + x8n[i3+3] * GAMMA[i1*EIGHT+3] +
              x8n[i3+4] * GAMMA[i1*EIGHT+4] + x8n[i3+5] * GAMMA[i1*EIGHT+5] +
              x8n[i3+6] * GAMMA[i1*EIGHT+6] + x8n[i3+7] * GAMMA[i1*EIGHT+7];

            Real_t hourmody =
              y8n[i3+0] * GAMMA[i1*EIGHT+0] + y8n[i3+1] * GAMMA[i1*EIGHT+1] +
              y8n[i3+2] * GAMMA[i1*EIGHT+2] + y8n[i3+3] * GAMMA[i1*EIGHT+3] +
              y8n[i3+4] * GAMMA[i1*EIGHT+4] + y8n[i3+5] * GAMMA[i1*EIGHT+5] +
              y8n[i3+6] * GAMMA[i1*EIGHT+6] + y8n[i3+7] * GAMMA[i1*EIGHT+7];

            Real_t hourmodz =
              z8n[i3+0] * GAMMA[i1*EIGHT+0] + z8n[i3+1] * GAMMA[i1*EIGHT+1] +
              z8n[i3+2] * GAMMA[i1*EIGHT+2] + z8n[i3+3] * GAMMA[i1*EIGHT+3] +
              z8n[i3+4] * GAMMA[i1*EIGHT+4] + z8n[i3+5] * GAMMA[i1*EIGHT+5] +
              z8n[i3+6] * GAMMA[i1*EIGHT+6] + z8n[i3+7] * GAMMA[i1*EIGHT+7];

            hourgam0[i1] = GAMMA[i1*EIGHT+0] -  volinv*(dvdx[i3  ] * hourmodx +
                                                        dvdy[i3  ] * hourmody +
                                                        dvdz[i3  ] * hourmodz );

            hourgam1[i1] = GAMMA[i1*EIGHT+1] -  volinv*(dvdx[i3+1] * hourmodx +
                                                        dvdy[i3+1] * hourmody +
                                                        dvdz[i3+1] * hourmodz );

            hourgam2[i1] = GAMMA[i1*EIGHT+2] -  volinv*(dvdx[i3+2] * hourmodx +
                                                        dvdy[i3+2] * hourmody +
                                                        dvdz[i3+2] * hourmodz );

            hourgam3[i1] = GAMMA[i1*EIGHT+3] -  volinv*(dvdx[i3+3] * hourmodx +
                                                        dvdy[i3+3] * hourmody +
                                                        dvdz[i3+3] * hourmodz );

            hourgam4[i1] = GAMMA[i1*EIGHT+4] -  volinv*(dvdx[i3+4] * hourmodx +
                                                        dvdy[i3+4] * hourmody +
                                                        dvdz[i3+4] * hourmodz );

            hourgam5[i1] = GAMMA[i1*EIGHT+5] -  volinv*(dvdx[i3+5] * hourmodx +
                                                        dvdy[i3+5] * hourmody +
                                                        dvdz[i3+5] * hourmodz );

            hourgam6[i1] = GAMMA[i1*EIGHT+6] -  volinv*(dvdx[i3+6] * hourmodx +
                                                        dvdy[i3+6] * hourmody +
                                                        dvdz[i3+6] * hourmodz );

            hourgam7[i1] = GAMMA[i1*EIGHT+7] -  volinv*(dvdx[i3+7] * hourmodx +
                                                        dvdy[i3+7] * hourmody +
                                                        dvdz[i3+7] * hourmodz );

          } // for i1

          /* compute forces */
          /* store forces into h arrays (force arrays) */

          ss1=domain.m_ss[i2];
          mass1=domain.m_elemMass[i2];
          volume13=cbrt(determ[i2]);

// RAG ///////////////////////////////////////////////////////////// RAG //
// RAG  GATHER (x|y|z)d1[0,,7] = domain.m_(x|y|z)[elemToNode[0..7]]  RAG //
// RAG ///////////////////////////////////////////////////////////// RAG //
          for( Index_t i=0 ; i<EIGHT ; ++i ) {
            Index_t gnode = elemToNode[i];
            xd1[i] = domain.m_xd[gnode];
            yd1[i] = domain.m_yd[gnode];
            zd1[i] = domain.m_zd[gnode];
          } // for i

          coefficient = - hourg * cast_Real_t(0.01) * ss1 * mass1 / volume13;

          CalcElemFBHourglassForce(xd1,yd1,zd1,
                                   hourgam0,hourgam1,hourgam2,hourgam3,
                                   hourgam4,hourgam5,hourgam6,hourgam7,
                                   coefficient, hgfx, hgfy, hgfz);

// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
          for( Index_t i=0 ; i<EIGHT ; ++i ) {
            Index_t gnode = elemToNode[i];
            AMO__sync_addition_double(&domain.m_fx[gnode], hgfx[i]);
            AMO__sync_addition_double(&domain.m_fy[gnode], hgfy[i]);
            AMO__sync_addition_double(&domain.m_fz[gnode], hgfz[i]);
          } // for i
        END_FOR_INNER(i2)
#ifdef HAB_C
        free(elemToNode);
        free(hgfz); free(hgfy); free(hgfx);
        free(hourgam7); free(hourgam6); free(hourgam5); free(hourgam4);
        free(hourgam3); free(hourgam2); free(hourgam1); free(hourgam0);
        free(zd1); free(yd1); free(xd1);
#endif // HAB_C
      END_ASYNC_IN_1(i2)
    END_FOR_OUTER(i2)
  END_FINISH
} // CalcFBHourglassForceForElems

static inline
void CalcHourglassControlForElems(Real_t determ[], Real_t hgcoef)
{
   Index_t numElem = domain.m_numElem ;
   Index_t numElem8 = numElem * EIGHT ;
   Real_t *dvdx = Allocate_Real_t(numElem8) ;
   Real_t *dvdy = Allocate_Real_t(numElem8) ;
   Real_t *dvdz = Allocate_Real_t(numElem8) ;
   Real_t *x8n  = Allocate_Real_t(numElem8) ;
   Real_t *y8n  = Allocate_Real_t(numElem8) ;
   Real_t *z8n  = Allocate_Real_t(numElem8) ;

   /* start loop over elements */
   FINISH
     FOR_OUTER_0xNx1(i,numElem)
       ASYNC_IN_1(i,domain,determ,           // INDEX, GLOBAL and PARAM 
               dvdx,dvdy,dvdz,x8n,y8n,z8n)   // LOCALs
#ifdef HAB_C
         Real_t  *x1  = malloc(EIGHT*sizeof(Real_t));
         Real_t  *y1  = malloc(EIGHT*sizeof(Real_t));
         Real_t  *z1  = malloc(EIGHT*sizeof(Real_t));
         Real_t  *pfx = malloc(EIGHT*sizeof(Real_t));
         Real_t  *pfy = malloc(EIGHT*sizeof(Real_t));
         Real_t  *pfz = malloc(EIGHT*sizeof(Real_t));
#else // NOT HAB_C
         Real_t  x1[EIGHT],  y1[EIGHT],  z1[EIGHT] ;
         Real_t pfx[EIGHT], pfy[EIGHT], pfz[EIGHT] ;
#endif // HAB_C
         FOR_INNER(i)

           Index_t* elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i];
// RAG -- GATHER (x|y|z)1[0..7] = domain.m_(x|y|z)[domain.m_nodelist[0..7]]
           CollectDomainNodesToElemNodes(elemToNode, x1, y1, z1);

           CalcElemVolumeDerivative(pfx, pfy, pfz, x1, y1, z1);

           /* load into temporary storage for FB Hour Glass control */
           for( Index_t ii=0 ; ii<EIGHT ; ++ii ){
             Index_t jj=EIGHT*i+ii;

             dvdx[jj] = pfx[ii];
             dvdy[jj] = pfy[ii];
             dvdz[jj] = pfz[ii];
             x8n[jj]  = x1[ii];
             y8n[jj]  = y1[ii];
             z8n[jj]  = z1[ii];
           } // for ii

           determ[i] = domain.m_volo[i] * domain.m_v[i];

           /* Do a check for negative volumes */
           if ( domain.m_v[i] <= cast_Real_t(0.0) ) {
             exit(VolumeError) ;
           }
         END_FOR_INNER(i)
#ifdef HAB_C
         free(pfz); free(pfy); free(pfx);
         free(z1) ; free(y1) ; free(x1);
#endif // HAB_C
       END_ASYNC_IN_1(i)
     END_FOR_OUTER(i)
  END_FINISH

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
} // CalcHourglassControlForElems

static inline
void CalcVolumeForceForElems() {
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
    FINISH
      FOR_OUTER_0xNx1(k,numElem)
        ASYNC_IN_1(k, determ )// INDEX and LOCAL
          FOR_INNER(k)
            if (determ[k] <= cast_Real_t(0.0)) {
              exit(VolumeError) ;
            }
          END_FOR_INNER(k)
        END_ASYNC_IN_1(k)
      END_FOR_OUTER(k)
    END_FINISH

    CalcHourglassControlForElems(determ, hgcoef) ;

    Release_Real_t(determ) ;
    Release_Real_t(sigzz) ;
    Release_Real_t(sigyy) ;
    Release_Real_t(sigxx) ;
  }
} // CalcVolumeForceForElems()

static inline void CalcForceForNodes() {
  Index_t numNode = domain.m_numNode ;
  FINISH 
    FOR_OUTER_0xNx1(i,numNode)
      ASYNC_IN_1(i,domain)   // INDEX and GLOBAL
        FOR_INNER(i)
          domain.m_fx[i] = cast_Real_t(0.0) ;
          domain.m_fy[i] = cast_Real_t(0.0) ;
          domain.m_fz[i] = cast_Real_t(0.0) ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  /* Calcforce calls partial, force, hourq */
  CalcVolumeForceForElems() ;

  /* Calculate Nodal Forces at domain boundaries */
  /* problem->commSBN->Transfer(CommSBN::forces); */

} // CalcForceForNodes()

static inline
void CalcAccelerationForNodes() {
  Index_t numNode = domain.m_numNode ;
  FINISH 
    FOR_OUTER_0xNx1(i,numNode)
      ASYNC_IN_1(i,domain)   // INDEX and GLOBAL
        FOR_INNER(i)
          domain.m_xdd[i] = domain.m_fx[i] / domain.m_nodalMass[i];
          domain.m_ydd[i] = domain.m_fy[i] / domain.m_nodalMass[i];
          domain.m_zdd[i] = domain.m_fz[i] / domain.m_nodalMass[i];
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // CalcAccelerationForNodes()

static inline
void ApplyAccelerationBoundaryConditionsForNodes() {
  Index_t numNodeBC = (domain.m_sizeX+1)*(domain.m_sizeX+1) ;
  FINISH
    FOR_OUTER_0xNx1(i,numNodeBC)
      ASYNC_IN_1(i,domain)   // INDEX and GLOBAL
// RAG -- SCATTER domain.m_(x|y|z)dd[domain.m_symm(X|Y|Z)[i]] = 0.0
        FOR_INNER(i)
          domain.m_xdd[domain.m_symmX[i]] = cast_Real_t(0.0) ;
        END_FOR_INNER(i)
        FOR_INNER(i)
          domain.m_ydd[domain.m_symmY[i]] = cast_Real_t(0.0) ;
        END_FOR_INNER(i)
        FOR_INNER(i)
          domain.m_zdd[domain.m_symmZ[i]] = cast_Real_t(0.0) ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // ApplyAccelerationBoundaryConditionsForNodes()

static inline
void CalcVelocityForNodes(const Real_t dt, const Real_t u_cut) {
  Index_t numNode = domain.m_numNode ;
  FINISH
    FOR_OUTER_0xNx1(i,numNode)
      ASYNC_IN_1(i, domain, dt,u_cut) // INDEX, GLOBAL and PARAMs
        FOR_INNER(i)
          Real_t xdtmp, ydtmp, zdtmp ;

// RAG -- DAXPY       -- (x|y|x)d += dt * (x|y|z)dd
// RAG -- DFLUSH .i.e -- -u_cut < (x|y|z)d <u_cut to 0.0

          xdtmp = domain.m_xd[i] + domain.m_xdd[i] * dt ;
          if( fabs(xdtmp) < u_cut ) xdtmp = cast_Real_t(0.0);
          domain.m_xd[i] = xdtmp ;

          ydtmp = domain.m_yd[i] + domain.m_ydd[i] * dt ;
          if( fabs(ydtmp) < u_cut ) ydtmp = cast_Real_t(0.0);
          domain.m_yd[i] = ydtmp ;

          zdtmp = domain.m_zd[i] + domain.m_zdd[i] * dt ;
          if( fabs(zdtmp) < u_cut ) zdtmp = cast_Real_t(0.0);
          domain.m_zd[i] = zdtmp ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // CalcVelocityForNodes()

static inline
void CalcPositionForNodes(const Real_t dt) {
  Index_t numNode = domain.m_numNode ;
  FINISH
    FOR_OUTER_0xNx1(i,numNode)
      ASYNC_IN_1(i, domain, dt) // INDEX, GLOBAL and PARAM
        FOR_INNER(i)
// RAG -- DAXPY       -- (x|y|x) += dt * (x|y|z)d
          domain.m_x[i] += domain.m_xd[i] * dt ;
          domain.m_y[i] += domain.m_yd[i] * dt ;
          domain.m_z[i] += domain.m_zd[i] * dt ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // CalcPositionForNodes()

static inline
void LagrangeNodal() {
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
} // LagrangeNodal()

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
               const Real_t z6, const Real_t z7 ) {
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
} // CalcElemVolume_scalars()

static inline
Real_t CalcElemVolume( const Real_t x[EIGHT],
                       const Real_t y[EIGHT],
                       const Real_t z[EIGHT] ) {
  return CalcElemVolume_scalars( x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7],
                                 y[0], y[1], y[2], y[3], y[4], y[5], y[6], y[7],
                                 z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
} // CalcElemVolume()

static inline
Real_t AreaFace( const Real_t x0, const Real_t x1,
                 const Real_t x2, const Real_t x3,
                 const Real_t y0, const Real_t y1,
                 const Real_t y2, const Real_t y3,
                 const Real_t z0, const Real_t z1,
                 const Real_t z2, const Real_t z3) {
  Real_t fx = (x2 - x0) - (x3 - x1);
  Real_t fy = (y2 - y0) - (y3 - y1);
  Real_t fz = (z2 - z0) - (z3 - z1);
  Real_t gx = (x2 - x0) + (x3 - x1);
  Real_t gy = (y2 - y0) + (y3 - y1);
  Real_t gz = (z2 - z0) + (z3 - z1);
  Real_t area = (fx * fx + fy * fy + fz * fz)
              * (gx * gx + gy * gy + gz * gz)
              - (fx * gx + fy * gy + fz * gz)
              * (fx * gx + fy * gy + fz * gz) ;
  return area ;
} // AreaFace

static inline
Real_t CalcElemCharacteristicLength( const Real_t x[EIGHT],
                                     const Real_t y[EIGHT],
                                     const Real_t z[EIGHT],
                                     const Real_t volume) {
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
} // CalcElemCharacteristicLength()

static inline
void CalcElemVelocityGrandient( const Real_t* const xvel,
                                const Real_t* const yvel,
                                const Real_t* const zvel,
                                hcConst Real_t b[][EIGHT],
                                const Real_t detJ,
                                Real_t* const d ) {
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
} // CalcElemVelocityGrandient()

static inline
void CalcKinematicsForElems( Index_t numElem, Real_t dt ) {
  FINISH
    // loop over all elements
    FOR_OUTER_0xNx1(k,numElem)
      ASYNC_IN_1(k,domain,dt)   // INDEX, GLOBAL and PARAM
#ifdef HAB_C
        Real_t *B        = malloc(THREE*EIGHT*sizeof(Real_t)) ;
        Real_t *D        = malloc(THREE*EIGHT*sizeof(Real_t)) ;
        Real_t *x_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *y_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *z_local  = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *xd_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *yd_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *zd_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *detJ     = malloc(sizeof(Real_t)) ;
        *detJ     = cast_Real_t(0.0) ;
#else // NOT HAB_C
        Real_t B[THREE][EIGHT] ; /** shape function derivatives */
        Real_t D[SIX] ;
        Real_t x_local[EIGHT] ;
        Real_t y_local[EIGHT] ;
        Real_t z_local[EIGHT] ;
        Real_t xd_local[EIGHT] ;
        Real_t yd_local[EIGHT] ;
        Real_t zd_local[EIGHT] ;
        Real_t detJ = cast_Real_t(0.0) ;
#endif // HAB_C
        FOR_INNER(k)
          Real_t volume ;
          Real_t relativeVolume ;
          hcConst Index_t* hcConst elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*k] ;

          // get nodal coordinates from global arrays and copy into local arrays.
          for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
            Index_t gnode = elemToNode[lnode];
            x_local[lnode] = domain.m_x[gnode];
            y_local[lnode] = domain.m_y[gnode];
            z_local[lnode] = domain.m_z[gnode];
          } // for lnode

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
          for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
            Index_t gnode = elemToNode[lnode];
            xd_local[lnode] = domain.m_xd[gnode];
            yd_local[lnode] = domain.m_yd[gnode];
            zd_local[lnode] = domain.m_zd[gnode];
          } // for lnode

          Real_t dt2 = cast_Real_t(0.5) * dt;
          for( Index_t j=0 ; j<EIGHT ; ++j ) {
             x_local[j] -= dt2 * xd_local[j];
             y_local[j] -= dt2 * yd_local[j];
             z_local[j] -= dt2 * zd_local[j];
          } // for j

#ifdef HAB_C
          CalcElemShapeFunctionDerivatives( x_local,
                                            y_local,
                                            z_local,
                                            B, detJ );

          CalcElemVelocityGrandient( xd_local,
                                     yd_local,
                                     zd_local,
                                     B, *detJ, D );
#else
          CalcElemShapeFunctionDerivatives( x_local,
                                            y_local,
                                            z_local,
                                            B, &detJ );

          CalcElemVelocityGrandient( xd_local,
                                     yd_local,
                                     zd_local,
                                     B, detJ, D );
#endif

          // put velocity gradient quantities into their global arrays.
          domain.m_dxx[k] = D[0];
          domain.m_dyy[k] = D[1];
          domain.m_dzz[k] = D[2];
        END_FOR_INNER(k) 
#ifdef HAB_C
        free(zd_local); free(yd_local); free(xd_local);
        free(z_local); free(y_local); free(x_local);
        free(D); free(B);
#endif // HAB_C
      END_ASYNC_IN_1(k) 
    END_FOR_OUTER(k) 
  END_FINISH
} // CalcKinematicsForElems()

static inline
void CalcLagrangeElements(Real_t deltatime) {
   Index_t numElem = domain.m_numElem ;
   if (numElem > 0) {
      CalcKinematicsForElems(numElem, deltatime) ;

      // element loop to do some stuff not included in the elemlib function.
      FINISH
        FOR_OUTER_0xNx1(k,numElem)
          ASYNC_IN_1(k,domain) // INDEX and GLOBAL
            FOR_INNER(k)
              // calc strain rate and apply as constraint (only done in FB element)
              Real_t vdov = domain.m_dxx[k] + domain.m_dyy[k] + domain.m_dzz[k] ;
              Real_t vdovthird = vdov/cast_Real_t(3.0) ;
        
              // make the rate of deformation tensor deviatoric
              domain.m_vdov[k] = vdov ;
              domain.m_dxx[k] -= vdovthird ;
              domain.m_dyy[k] -= vdovthird ;
              domain.m_dzz[k] -= vdovthird ;

              // See if any volumes are negative, and take appropriate action.
              if (domain.m_vnew[k] <= cast_Real_t(0.0)) {
                exit(VolumeError) ;
              } // if domain.m_vnew
            END_FOR_INNER(k)
          END_ASYNC_IN_1(k)
        END_FOR_OUTER(k)
      END_FINISH
   } // if numElem
} // CalcLagrangeElements()

static inline
void CalcMonotonicQGradientsForElems() {
#define SUM4(a,b,c,d) (a + b + c + d)
  Index_t numElem = domain.m_numElem ;
  FINISH
    FOR_OUTER_0xNx1(i,numElem)
      ASYNC_IN_1(i,domain) // INDEX and GLOBAL
        hcConst Real_t ptiny = cast_Real_t(1.e-36) ;
        FOR_INNER(i)
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
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
#undef SUM4
} // CalcMonotonicQGradientsForElems()

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
  FINISH
    FOR_OUTER_0xNx1(ielem,elength)
      ASYNC_IN_1(ielem,domain,                  // INDEX, GLOBAL and PARAMs
                 qlc_monoq, qqc_monoq, monoq_limiter_mult,
                 monoq_max_slope, ptiny )
        FOR_INNER(ielem)
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
          } // switch XI_M

          switch (bcMask & XI_P) {
            case 0:         delvp = domain.m_delv_xi[domain.m_lxip[i]] ; break ;
            case XI_P_SYMM: delvp = domain.m_delv_xi[i] ;                break ;
            case XI_P_FREE: delvp = cast_Real_t(0.0) ;                   break ;
            default:        /* ERROR */ ;                                break ;
          } // switch XI_P

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
          } // switch ETA_M

          switch (bcMask & ETA_P) {
            case 0:          delvp = domain.m_delv_eta[domain.m_letap[i]] ; break ;
            case ETA_P_SYMM: delvp = domain.m_delv_eta[i] ;                 break ;
            case ETA_P_FREE: delvp = cast_Real_t(0.0) ;                     break ;
            default:         /* ERROR */ ;                                  break ;
          } // switch ETA_P

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
          } // switch ZETA_M
          switch (bcMask & ZETA_P) {
            case 0:           delvp = domain.m_delv_zeta[domain.m_lzetap[i]] ; break ;
            case ZETA_P_SYMM: delvp = domain.m_delv_zeta[i] ;                  break ;
            case ZETA_P_FREE: delvp = cast_Real_t(0.0) ;                       break ;
            default:          /* ERROR */ ;                                    break ;
          } // switch ZETA_P

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
          } else {
            Real_t delvxxi   = domain.m_delv_xi[i]   * domain.m_delx_xi[i]   ;
            Real_t delvxeta  = domain.m_delv_eta[i]  * domain.m_delx_eta[i]  ;
            Real_t delvxzeta = domain.m_delv_zeta[i] * domain.m_delx_zeta[i] ;

            if ( delvxxi   > cast_Real_t(0.) ) delvxxi   = cast_Real_t(0.) ;
            if ( delvxeta  > cast_Real_t(0.) ) delvxeta  = cast_Real_t(0.) ;
            if ( delvxzeta > cast_Real_t(0.) ) delvxzeta = cast_Real_t(0.) ;

            Real_t rho = domain.m_elemMass[i] / (domain.m_volo[i] * domain.m_vnew[i]) ;

            qlin = -qlc_monoq * rho *
                 ( delvxxi   * (cast_Real_t(1.) - phixi  ) +
                   delvxeta  * (cast_Real_t(1.) - phieta ) +
                   delvxzeta * (cast_Real_t(1.) - phizeta) ) ;

            qquad = qqc_monoq * rho *
                  (  delvxxi*delvxxi     * (cast_Real_t(1.) - phixi*phixi    ) +
                     delvxeta*delvxeta   * (cast_Real_t(1.) - phieta*phieta  ) +
                     delvxzeta*delvxzeta * (cast_Real_t(1.) - phizeta*phizeta) ) ;
          } // if domain.m_vdov

          domain.m_qq[i] = qquad ;
          domain.m_ql[i] = qlin  ;
        END_FOR_INNER(ielem)
      END_ASYNC_IN_1(ielem)
    END_FOR_OUTER(ielem)
  END_FINISH
} // CalcMonotonicQRegionForElems()

static inline
void CalcMonotonicQForElems() {  
   //
   // initialize parameters
   // 
   hcConst Real_t ptiny      = cast_Real_t(1.e-36) ;
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
   } // if elength
} // CalcMonotonicQForElems()

static inline
void CalcQForElems() {
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
      int64_t *pidx = malloc(sizeof(Index_t)); 
      *pidx = -1; 
      FINISH
      FOR_OUTER_0xNx1(i,numElem)
        ASYNC_IN_1(i,domain,qstop,pidx)
          FOR_INNER(i)
            if ( domain.m_q[i] > qstop ) {
               AMO__sync_fetch_and_add_int64_t(pidx,1);
               break ;
            } // if domain.m_q
            END_FOR_INNER(i)
          END_ASYNC_IN_1(i)
        END_FOR_OUTER(i)
      END_FINISH

      if(*pidx >= 0) {
         exit(QStopError) ;
      } // if idx
      free(pidx);
   } // if numElem
} // CalcQForElems()

static inline
void CalcPressureForElems(Real_t* p_new, Real_t* bvc,
                          Real_t* pbvc, Real_t* e_old,
                          Real_t* compression, Real_t *vnewc,
                          Real_t pmin,
                          Real_t p_cut, Real_t eosvmax,
                          Index_t length)
{
  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,bvc,pbvc,compression) // INDEX and PARAMs
        Real_t c1s = cast_Real_t(2.0)/cast_Real_t(3.0) ;
        FOR_INNER(i)
          bvc[i] = c1s * (compression[i] + cast_Real_t(1.));
          pbvc[i] = c1s;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,p_new,bvc,e_old,vnewc,pmin,p_cut,eosvmax) // INDEX and PARAMs
        FOR_INNER(i)
          p_new[i] = bvc[i] * e_old[i] ;

          if    (fabs(p_new[i]) <  p_cut   ) 
             p_new[i] = cast_Real_t(0.0) ;

          if    ( vnewc[i] >= eosvmax ) /* impossible condition here? */
             p_new[i] = cast_Real_t(0.0) ;

          if    (p_new[i]       <  pmin)
             p_new[i]   = pmin ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // CalcPressureForElems()

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
                        Index_t length) {

  Real_t *pHalfStep = Allocate_Real_t(length) ;

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,e_new,e_old,delvc,p_old,q_old,work,emin) // INDEX and PARAMs
        FOR_INNER(i)
          e_new[i] = e_old[i] - cast_Real_t(0.5) * delvc[i] * (p_old[i] + q_old[i])
                   + cast_Real_t(0.5) * work[i];

          if (e_new[i]  < emin ) {
            e_new[i] = emin ;
          }
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  CalcPressureForElems(pHalfStep, bvc, pbvc, e_new, compHalfStep, vnewc,
                       pmin, p_cut, eosvmax, length);
  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,compHalfStep,q_new,qq,ql,pbvc,e_new,bvc,pHalfStep,rho0,delvc,p_old,q_old,work,e_cut,emin) // INDEX and PARAMs
        FOR_INNER(i)
          Real_t vhalf = cast_Real_t(1.) / (cast_Real_t(1.) + compHalfStep[i]) ;

          if ( delvc[i] > cast_Real_t(0.) ) {
            q_new[i] /* = qq[i] = ql[i] */ = cast_Real_t(0.) ;
          } else {
            Real_t ssc = ( pbvc[i] * e_new[i]
                       + vhalf * vhalf * bvc[i] * pHalfStep[i] ) / rho0 ;

            if ( ssc <= cast_Real_t(0.) ) {
              ssc =cast_Real_t(.333333e-36) ;
            } else {
              ssc = sqrt(ssc) ;
            } // if ssc

            q_new[i] = (ssc*ql[i] + qq[i]) ;
          } // if delvc

          e_new[i] +=  cast_Real_t(0.5) * delvc[i]
                   * ( cast_Real_t(3.0) * (p_old[i]     + q_old[i])
                     - cast_Real_t(4.0) * (pHalfStep[i] + q_new[i])) ;

          e_new[i] += cast_Real_t(0.5) * work[i];

          if (fabs(e_new[i]) < e_cut) {
            e_new[i] = cast_Real_t(0.)  ;
          } // e_cut
          if (     e_new[i]  < emin ) {
            e_new[i] = emin ;
          } // if emin
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

   CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                   pmin, p_cut, eosvmax, length);

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,delvc,pbvc,e_new,vnewc,bvc,p_new,rho0,ql,qq,p_old,q_old,pHalfStep,q_new,e_cut,emin) // INDEX and PARAMs
        hcConst Real_t sixth = cast_Real_t(1.0) / cast_Real_t(6.0) ;
        FOR_INNER(i)
          Real_t q_tilde ;

          if (delvc[i] > cast_Real_t(0.)) {
            q_tilde = cast_Real_t(0.) ;
          } else {
            Real_t ssc = ( pbvc[i] * e_new[i]
                       + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

            if ( ssc <= cast_Real_t(0.) ) {
              ssc = cast_Real_t(.333333e-36) ;
            } else {
              ssc = sqrt(ssc) ;
            } // if ssc

            q_tilde = (ssc*ql[i] + qq[i]) ;
          } // if delvc

          e_new[i] += - ( cast_Real_t(7.0)*(p_old[i]     + q_old[i])
                        - cast_Real_t(8.0)*(pHalfStep[i] + q_new[i])
                        + (p_new[i] + q_tilde)) * delvc[i]*sixth ;

          if (fabs(e_new[i]) < e_cut) {
            e_new[i] = cast_Real_t(0.)  ;
          } // if e_cut
          if (     e_new[i]  < emin ) {
            e_new[i] = emin ;
          } // if emin
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                   pmin, p_cut, eosvmax, length);

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,delvc,pbvc,e_new,vnewc,bvc,p_new,rho0,ql,qq,q_new,q_cut) // INDEX and PARAMs
        FOR_INNER(i)

          if ( delvc[i] <= cast_Real_t(0.) ) {
            Real_t ssc = (  pbvc[i] * e_new[i]
                         + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

            if ( ssc <= cast_Real_t(0.) ) {
              ssc = cast_Real_t(.333333e-36) ;
            } else {
              ssc = sqrt(ssc) ;
            } // if ssc

            q_new[i] = (ssc*ql[i] + qq[i]) ;

            if (fabs(q_new[i]) < q_cut) q_new[i] = cast_Real_t(0.) ;
          } // if delvc
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

   Release_Real_t(pHalfStep) ;

   return ;
} // CalcEnergyForElems()

static inline
void CalcSoundSpeedForElems(Real_t *vnewc, Real_t rho0, Real_t *enewc,
                            Real_t *pnewc, Real_t *pbvc,
                            Real_t *bvc, Real_t ss4o3, Index_t nz) {
  FINISH
    FOR_OUTER_0xNx1(i,nz)
      ASYNC_IN_1(i,domain,pbvc,enewc,vnewc,bvc,pnewc,rho0) // INDEX, GLOBAL and PARAMs
        FOR_INNER(i)
          Index_t iz = domain.m_matElemlist[i];
          Real_t ssTmp = (pbvc[i] * enewc[i] + vnewc[i] * vnewc[i] *
                           bvc[i] * pnewc[i]) / rho0;
          if (ssTmp <= cast_Real_t(1.111111e-36)) {
            ssTmp = cast_Real_t(1.111111e-36);
          } // if ssTmp
          domain.m_ss[iz] = sqrt(ssTmp);
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH
} // CalcSoundSpeedForElems()

static inline
void EvalEOSForElems(Real_t *vnewc, Index_t length) {
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

// RAG GATHERS

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,domain,delvc,e_old,p_old,q_old,qq,ql)
        FOR_INNER(i) 
          Index_t zidx = domain.m_matElemlist[i] ;
          delvc[i] = domain.m_delv[zidx] ;
          e_old[i] = domain.m_e[zidx] ;
          p_old[i] = domain.m_p[zidx] ;
          q_old[i] = domain.m_q[zidx] ;
          qq[i]    = domain.m_qq[zidx] ;
          ql[i]    = domain.m_ql[zidx] ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH 


// RAG STRIDE ONE

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,compression,vnewc,delvc,compHalfStep,work,eosvmin,eosvmax,p_old)

        FOR_INNER(i) 
           Real_t vchalf ;
           compression[i] = cast_Real_t(1.) / vnewc[i] - cast_Real_t(1.);
           vchalf = vnewc[i] - delvc[i] * cast_Real_t(.5);
           compHalfStep[i] = cast_Real_t(1.) / vchalf - cast_Real_t(1.);
           work[i] = cast_Real_t(0.) ; 
        END_FOR_INNER(i)


        /* Check for v > eosvmax or v < eosvmin */

        if ( eosvmin != cast_Real_t(0.) ) {
          FOR_INNER(i)
            if (vnewc[i] <= eosvmin) { /* impossible due to calling func? */
              compHalfStep[i] = compression[i] ;
            } // if vnewc
          END_FOR_INNER(i)
        } // if eosvmin
        if ( eosvmax != cast_Real_t(0.) ) {
          FOR_INNER(i)
            if (vnewc[i] >= eosvmax) { /* impossible due to calling func? */
              p_old[i]        = cast_Real_t(0.) ;
              compression[i]  = cast_Real_t(0.) ;
              compHalfStep[i] = cast_Real_t(0.) ;
            } // if vnewc
          END_FOR_INNER(i)
        } // if eosvmax

      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH 

  CalcEnergyForElems(p_new, e_new, q_new, bvc, pbvc,
                     p_old, e_old, q_old, compression, compHalfStep,
                     vnewc, work,  delvc, pmin,
                     p_cut, e_cut, q_cut, emin,
                     qq, ql, rho0, eosvmax, length);

// RAG SCATTERS

  FINISH
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,domain,p_new,e_new,q_new)
        FOR_INNER(i) 
          Index_t zidx = domain.m_matElemlist[i] ;
          domain.m_p[zidx] = p_new[i] ;
          domain.m_e[zidx] = e_new[i] ;
          domain.m_q[zidx] = q_new[i] ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH 

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
} // EvalEOSForElems()

static inline
void ApplyMaterialPropertiesForElems() {
  Index_t length = domain.m_numElem ;

  if (length != 0) {
    /* Expose all of the variables needed for material evaluation */
    Real_t eosvmin = domain.m_eosvmin ;
    Real_t eosvmax = domain.m_eosvmax ;
    Real_t *vnewc  = Allocate_Real_t(length) ;

// RAG GATHERS

    FINISH
      FOR_OUTER_0xNx1(i,length)
        ASYNC_IN_1(i,domain,vnewc) // INDEX, GLOBAL and PARAMS
          FOR_INNER(i) 
            Index_t zn = domain.m_matElemlist[i] ;
            vnewc[i] = domain.m_vnew[zn] ;
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
    END_FINISH 

// RAG STIDE ONES

    FINISH
      FOR_OUTER_0xNx1(i,length)
        ASYNC_IN_1(i,vnewc,eosvmin,eosvmax) // INDEX and PARAMs

          if (eosvmin != cast_Real_t(0.)) {
            FOR_INNER(i) 
              if (vnewc[i] < eosvmin) {
                vnewc[i] = eosvmin ;
              } // if vnewc
            END_FOR_INNER(i)
          } // if eosvmin

          if (eosvmax != cast_Real_t(0.)) {
            FOR_INNER(i) 
              if (vnewc[i] > eosvmax) {
                vnewc[i] = eosvmax ;
              } // if vnewc
            END_FOR_INNER(i)
          } // if eosvmax

        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
    END_FINISH 

// RAG GATHER ERROR CHECK

    FINISH
      FOR_OUTER_0xNx1(i,length)
        ASYNC_IN_1(i,domain,vnewc,eosvmin,eosvmax) // INDEX, GLOBAL and PARAMs
          FOR_INNER(i) 
            Index_t zn = domain.m_matElemlist[i] ;
            Real_t  vc = domain.m_v[zn] ;
            if (eosvmin != cast_Real_t(0.)) {
              if (vc < eosvmin) { 
                vc = eosvmin ;
              } // if domain.m_v
            } // if eosvmin
            if (eosvmax != cast_Real_t(0.)) {
              if (vc > eosvmax) {
                vc = eosvmax ;
              } // if domain.m_v
            } // if eosvmax
            if (vc <= 0.) {
              exit(VolumeError) ;
            } // if domain.m_v
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
    END_FINISH 

    EvalEOSForElems(vnewc, length);

    Release_Real_t(vnewc) ;

  } // if length

} // ApplyMaterialPropertiesForElems()

static inline
void UpdateVolumesForElems() {
  Index_t numElem = domain.m_numElem;
  if (numElem != 0) {
    Real_t v_cut = domain.m_v_cut;
    FINISH
      FOR_OUTER_0xNx1(i,numElem)
        ASYNC_IN_1(i,domain,v_cut) // INDEX, GLOBAL and LOCAL
          FOR_INNER(i) 
            Real_t tmpV ;
            tmpV = domain.m_vnew[i] ;

            if ( fabs(tmpV - cast_Real_t(1.0)) < v_cut ) {
              tmpV = cast_Real_t(1.0) ;
            } // tmpV
            domain.m_v[i] = tmpV ;
          END_FOR_INNER(i)
        END_ASYNC_IN_1(i)
      END_FOR_OUTER(i)
    END_FINISH 
  } // if numElem
  return ;
} // UpdateVolumesForElems()

static inline
void LagrangeElements() {
  hcConst Real_t deltatime = domain.m_deltatime ;

  CalcLagrangeElements(deltatime) ;

  /* Calculate Q.  (Monotonic q option requires communication) */
  CalcQForElems() ;

  ApplyMaterialPropertiesForElems() ;

  UpdateVolumesForElems() ;
} // LagrangeElements()

static inline
void CalcCourantConstraintForElems() {
  Real_t  *pDtCourant    = malloc(sizeof(Real_t)) ;
  Index_t *pCourant_elem = malloc(sizeof(Index_t));
  *pDtCourant = cast_Real_t(1.0e+20) ;
  *pCourant_elem = -1;

  FINISH
    Real_t      qqc = domain.m_qqc ;
    Real_t  qqc2 = cast_Real_t(64.0) * qqc * qqc ;
    Index_t length = domain.m_numElem ;
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,domain,qqc2,pDtCourant,pCourant_elem,pidamin_lock)
        FOR_INNER(i)
          Index_t indx = domain.m_matElemlist[i] ;

          Real_t dtf = domain.m_ss[indx] * domain.m_ss[indx] ;

          if ( domain.m_vdov[indx] < cast_Real_t(0.) ) {

            dtf = dtf
                + qqc2 * domain.m_arealg[indx] * domain.m_arealg[indx]
                * domain.m_vdov[indx] * domain.m_vdov[indx] ;
          } // if domain.m_vdov

          dtf = sqrt(dtf) ;

          dtf = domain.m_arealg[indx] / dtf ;

          /* determine minimum timestep with its corresponding elem */
          if (domain.m_vdov[indx] != cast_Real_t(0.)) {
            if ( dtf < *pDtCourant ) {
AMO__lock_uint64_t(pidamin_lock);          // LOCK
              *pDtCourant    = dtf ;
              *pCourant_elem = indx ;
AMO__unlock_uint64_t(pidamin_lock);        // UNLOCK
            } // if *pDtCourant
          } // if domain.m_vdov

        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  /* Don't try to register a time constraint if none of the elements
   * were active */
  if ( *pCourant_elem != -1) {
     domain.m_dtcourant = *pDtCourant ;
  } // if *pCourant_elem

  free(pCourant_elem);
  free(pDtCourant);
  return ;
} // CalcCourantConstraintForElems()

static inline
void CalcHydroConstraintForElems() {
  Real_t  *pDtHydro    = malloc(sizeof(Real_t)) ;
  Index_t *pHydro_elem = malloc(sizeof(Index_t));
  *pDtHydro = cast_Real_t(1.0e+20) ;
  *pHydro_elem = -1 ;

  FINISH
    Real_t dvovmax = domain.m_dvovmax ;
    Index_t length = domain.m_numElem ;
    FOR_OUTER_0xNx1(i,length)
      ASYNC_IN_1(i,domain,dvovmax,pidamin_lock,pDtHydro,pHydro_elem)
        FOR_INNER(i)

          Index_t indx = domain.m_matElemlist[i] ;

          if (domain.m_vdov[indx] != cast_Real_t(0.)) {
            Real_t dtdvov = dvovmax / (fabs(domain.m_vdov[indx])+cast_Real_t(1.e-20)) ;
            if ( *pDtHydro > dtdvov ) {
AMO__lock_uint64_t(pidamin_lock);          // LOCK
              *pDtHydro    = dtdvov ;
              *pHydro_elem = indx ;
AMO__unlock_uint64_t(pidamin_lock);        // UNLOCK
            } // if *pDtHydro
          } // if domain.m_vdov

        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  if (*pHydro_elem != -1) {
     domain.m_dthydro = *pDtHydro ;
  } // if *pHydro_elem

  free(pHydro_elem);
  free(pDtHydro);
  return ;
} // CalcHydroConstraintForElems()

static inline
void CalcTimeConstraintsForElems() {

  /* evaluate time constraint */
  CalcCourantConstraintForElems() ;

  /* check hydro constraint */
  CalcHydroConstraintForElems() ;
} // CalcTimeConstraintsForElems()

static inline
void LagrangeLeapFrog() {
  /* calculate nodal forces, accelerations, velocities, positions, with
   * applied boundary conditions and slide surface considerations */

  LagrangeNodal();

  /* calculate element quantities (i.e. velocity gradient & q), and update
   * material states */

  LagrangeElements();

  CalcTimeConstraintsForElems();

} // LagangeLeapFrog()

int main(int argc, char *argv[]) {
  Index_t edgeElems = 45 ;
  Index_t edgeNodes = edgeElems+1 ;
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

  FINISH
    Real_t sf = cast_Real_t(1.125)/cast_Real_t(edgeElems);
    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;

    FOR_OUTER_0xNx1(pln,edgeNodes)
      FOR_OUTER_0xNx1(col,edgeNodes)
        FOR_OUTER_0xNx1(row,edgeNodes)

          ASYNC_IN_3(pln,col,row,domain,       // INDEXs and GLOBAL
                   sf,dimN,dimNdimN)           // LOCALs
            FOR_INNER(pln)
              Real_t tz = cast_Real_t(pln)*sf;
              Index_t pln_nidx = pln*dimNdimN;
              FOR_INNER(row)
                Real_t ty = cast_Real_t(row)*sf;
                Index_t pln_row_nidx = pln_nidx + row*dimN;
                FOR_INNER(col)
                  Real_t tx = cast_Real_t(col)*sf;
                  Index_t nidx = pln_row_nidx+col;
                  domain.m_x[nidx] = tx;
                  domain.m_y[nidx] = ty;
                  domain.m_z[nidx] = tz;
                END_FOR_INNER(col)
              END_FOR_INNER(row)
            END_FOR_INNER(pln)
          END_ASYNC_IN_3(pln,row,col)
        END_FOR_OUTER(col)
      END_FOR_OUTER(row)
    END_FOR_OUTER(pln)
  END_FINISH

  /* embed hexehedral elements in nodal point lattice */

  FINISH
    Index_t dimE = edgeElems, dimEdimE = edgeElems*edgeElems;
    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;

    FOR_OUTER_0xNx1(pln,edgeElems)
      FOR_OUTER_0xNx1(row,edgeElems)
        FOR_OUTER_0xNx1(col,edgeElems)
          ASYNC_IN_3(pln,row,col,domain,             // INDEXs and GLOBAL
                   dimE,dimEdimE,dimN,dimNdimN)      // LOCALs
            FOR_INNER(pln)
              Index_t pln_nidx = pln*dimNdimN;
              Index_t pln_zidx = pln*dimEdimE;
              FOR_INNER(row)
                Index_t pln_row_nidx = pln_nidx + row*dimN;
                Index_t pln_row_zidx = pln_zidx + row*dimE;
                FOR_INNER(col)
                  Index_t nidx = pln_row_nidx+col;
                  Index_t zidx = pln_row_zidx+col;
                  Index_t *localNode = (Index_t *)&domain.m_nodelist[EIGHT*zidx] ;
                  localNode[0] = nidx                       ;
                  localNode[1] = nidx                   + 1 ;
                  localNode[2] = nidx            + dimN + 1 ;
                  localNode[3] = nidx            + dimN     ;
                  localNode[4] = nidx + dimNdimN            ;
                  localNode[5] = nidx + dimNdimN        + 1 ;
                  localNode[6] = nidx + dimNdimN + dimN + 1 ;
                  localNode[7] = nidx + dimNdimN + dimN     ;
                END_FOR_INNER(col)
              END_FOR_INNER(row)
            END_FOR_INNER(pln)
          END_ASYNC_IN_3(pln,row,col)
        END_FOR_OUTER(col)
      END_FOR_OUTER(row)
    END_FOR_OUTER(pln)
  END_FINISH

  /* Create a material IndexSet (entire domain same material for now) */
  FINISH
    FOR_OUTER_0xNx1(i,domElems)
      ASYNC_IN_1(i,domain)
        FOR_INNER(i)
          domain.m_matElemlist[i] = i ;
        END_FOR_OUTER(col)
      END_FOR_OUTER(row)
    END_FOR_OUTER(pln)
  END_FINISH
   
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

  domain.m_qstop              = cast_Real_t(1.0e+12) ;
  domain.m_monoq_max_slope    = cast_Real_t(1.0) ;
  domain.m_monoq_limiter_mult = cast_Real_t(2.0) ;
  domain.m_qlc_monoq          = cast_Real_t(0.5) ;
  domain.m_qqc_monoq          = cast_Real_t(2.0)/cast_Real_t(3.0) ;
  domain.m_qqc                = cast_Real_t(2.0) ;

  domain.m_pmin               = cast_Real_t(0.) ;
  domain.m_emin               = cast_Real_t(-1.0e+15) ;

  domain.m_dvovmax            = cast_Real_t(0.1) ;

  domain.m_eosvmax            = cast_Real_t(1.0e+9) ;
  domain.m_eosvmin            = cast_Real_t(1.0e-9) ;

  domain.m_refdens            = cast_Real_t(1.0) ;

  FINISH
    FOR_OUTER_0xNx1(i,domElems)
      ASYNC_IN_1(i,domain) // INDEX and GLOBAL
#ifdef HAB_C
        Real_t *x_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *y_local = malloc(EIGHT*sizeof(Real_t)) ;
        Real_t *z_local = malloc(EIGHT*sizeof(Real_t)) ;
#else // NOT HAB_C
        Real_t x_local[EIGHT], y_local[EIGHT], z_local[EIGHT] ;
#endif // HAB_C
        FOR_INNER(i)
          Index_t *elemToNode = (Index_t *)&domain.m_nodelist[EIGHT*i] ;
          for( Index_t lnode=0 ; lnode < EIGHT ; ++lnode ) {
            Index_t gnode = elemToNode[lnode];
            x_local[lnode] = domain.m_x[gnode];
            y_local[lnode] = domain.m_y[gnode];
            z_local[lnode] = domain.m_z[gnode];
          } // for lnode

          // volume calculations
          Real_t volume = CalcElemVolume(x_local, y_local, z_local );
          domain.m_volo[i] = volume ;
          domain.m_elemMass[i] = volume ;
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
          for( Index_t j=0 ; j<EIGHT ; ++j ) {
            Index_t idx = elemToNode[j] ;
            Real_t value = volume / cast_Real_t(8.0);
            AMO__sync_addition_double(&domain.m_nodalMass[idx], value);
          } // for j
        END_FOR_INNER(i)
#ifdef HAB_C
        free(z_local) ;
        free(y_local) ;
        free(x_local) ;
#endif // HAB_C
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  /* deposit energy */
  domain.m_e[0] = cast_Real_t(3.948746e+7) ;

  /* set up symmetry nodesets */
  FINISH
    Index_t dimN = edgeNodes, dimNdimN = dimN*dimN;
    
    FOR_OUTER_0xNx1(i,edgeNodes)
      FOR_OUTER_0xNx1(j,edgeNodes)
        ASYNC_IN_2(i,j,domain,dimN,dimNdimN)
          FOR_INNER(i)
            Index_t planeInc = i*dimNdimN ;
            Index_t rowInc   = i*dimN ;
            FOR_INNER(j)
              Index_t nidx = rowInc + j;
              domain.m_symmX[nidx] = planeInc + j*dimN ;
              domain.m_symmY[nidx] = planeInc + j ;
              domain.m_symmZ[nidx] = rowInc   + j ;
            END_FOR_INNER(j)
          END_FOR_INNER(i)
        END_ASYNC_IN_2(i,j)
      END_FOR_OUTER(j)
    END_FOR_OUTER(i)
  END_FINISH

  /* set up elemement connectivity information */
  FINISH
    domain.m_lxip[0] = 1 ;
    FOR_OUTER_0xNx1(i,domElems)
      ASYNC_IN_1(i,domain,domElems)
        FOR_INNER(i)
          domain.m_lxim[i]   = i-1 ;
          domain.m_lxip[i-1] = i ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
    domain.m_lxim[domElems-1] = domElems-2 ;
  END_FINISH

  FINISH
    FOR_OUTER_0xNx1(i,edgeElems)
      ASYNC_IN_1(i,domain,domElems,edgeElems)
        FOR_INNER(i)
          domain.m_letam[i] = i ; 
          domain.m_letap[domElems-edgeElems+i] = domElems-edgeElems+i ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  FINISH
    FOR_OUTER_0xNx1(i,(domElems-edgeElems))
      ASYNC_IN_1(i,domain,edgeElems)
        FOR_INNER(i)
          domain.m_letam[i+edgeElems] = i ;
          domain.m_letap[i          ] = i+edgeElems ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  FINISH
    FOR_OUTER_0xNx1(i,(edgeElems*edgeElems))
      ASYNC_IN_1(i,domain,domElems,edgeElems)
        FOR_INNER(i)
          domain.m_lzetam[i] = i ;
          domain.m_lzetap[domElems-edgeElems*edgeElems+i] = domElems-edgeElems*edgeElems+i ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  FINISH
    Index_t dimE = edgeElems, dimEdimE = dimE*dimE;
    FOR_OUTER_0xNx1(i,(domElems-dimEdimE))
      ASYNC_IN_1(i,domain,dimEdimE,domElems)
        FOR_INNER(i)
          domain.m_lzetam[i+dimEdimE] = i ;
          domain.m_lzetap[i]          = i+dimEdimE ;
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  /* set up boundary condition information */
  FINISH
    FOR_OUTER_0xNx1(i,domElems)
      ASYNC_IN_1(i,domain,domElems)
        FOR_INNER(i)
          domain.m_elemBC[i] = 0 ;  /* clear BCs by default */
        END_FOR_INNER(i)
      END_ASYNC_IN_1(i)
    END_FOR_OUTER(i)
  END_FINISH

  /* faces on "external" boundaries will be */
  /* symmetry plane or free surface BCs */
  FINISH
    Index_t dimE = edgeElems, dimEdimE = dimE*dimE;
    FOR_OUTER_0xNx1(i,edgeElems)
      FOR_OUTER_0xNx1(j,edgeElems)
        ASYNC_IN_2(i,j,domain,domElems,dimE,dimEdimE)
          FOR_INNER(i)
            Index_t planeInc = i*dimEdimE ;
            Index_t rowInc   = i*dimE ;
            FOR_INNER(j)
              domain.m_elemBC[planeInc+j*dimE           ] |= XI_M_SYMM ;
              domain.m_elemBC[planeInc+j*dimE+1*dimE-1  ] |= XI_P_FREE ;
              domain.m_elemBC[planeInc+j                ] |= ETA_M_SYMM ;
              domain.m_elemBC[planeInc+j+dimEdimE-dimE  ] |= ETA_P_FREE ;
              domain.m_elemBC[rowInc+j                  ] |= ZETA_M_SYMM ;
              domain.m_elemBC[rowInc+j+domElems-dimEdimE] |= ZETA_P_FREE ;
            END_FOR_INNER(j)
          END_FOR_INNER(i)
        END_ASYNC_IN_2(i,j)
      END_FOR_OUTER(j)
    END_FOR_OUTER(i)
  END_FINISH

  /* TIMESTEP TO SOLUTION */

  while(domain.m_time < domain.m_stoptime ) {

    TimeIncrement() ;

    LagrangeLeapFrog() ;

#if LULESH_SHOW_PROGRESS
    printf("time = %e, dt=%e, e(0)=%e\n",
          ((double)domain.m_time),
          ((double)domain.m_deltatime),
          ((double)domain.m_e[0])) ;
    fflush(stdout);
#endif
  } // while time

  printf("   Final Origin Energy = %12.6e \n", (double)domain.m_e[0]) ;

  return 0 ;
} // main()
