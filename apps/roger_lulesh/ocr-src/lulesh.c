//#define USE_CALL
//#define USE_EDT_CALL
#define USE_EDT

#if defined(USE_CALL)

#define EDT_CALL(me,arg,next) \
me(arg)

#elif defined(USE_EDT_CALL)

#define DEF_EDT_CALL(me,arg,next) \
ocrGuid_t me ## _edt_0(u32 paramc, u64 *paramv u32 depc, void **depv) { \
SHARED struct Domain_t *arg = (SHARED struct Domain_t *)(*depv);

#define MAKE_EDT_CALL(me,arg,next) \
ocrGuid_t me ## _edt_0(u32 paramc, u64 *paramv, u32 depc, void **depv) ; \
(void) me ## _edt_0((u32)0,(u64 *)NULL,(void *)NULL,(u32)1,(void *)&arg)

#elif defined(USE_EDT)

#define DEF_EDT_CALL(me,arg,next) \
ocrGuid_t me ## _edt_0(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) { \
SHARED struct Domain_t *arg = (SHARED struct Domain_t *)depv[0].ptr;

#endif

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
#if !defined(FSIM) && !defined(OCR)
#include <string.h>
#endif

#include "RAG.h"
#include "AMO.h"
#include "MEM.h"
#include "FP.h"

// RAG would like to remove all golbal accesses to domainObject and domain, to better model passing data blocks
#if defined(FSIM)
SHARED struct DomainObject_t domainObject = { .guid.data = (uint64_t)NULL, .base = NULL, .offset = 0, .limit = 0,
                                              .edgeElems = 0, .edgeNodes = 0, };
//SHARED             ocrGuid_t NULL_GUID    = { .data = 0, };
#elif defined(OCR)
SHARED struct DomainObject_t domainObject = { .guid      = (uint64_t)NULL, .base = NULL, .offset = 0, .limit = 0,
                                              .edgeElems = 0, .edgeNodes = 0,};
#else
#define NULL_GUID ((int)0)
#endif // FSIM or OCR

SHARED struct Domain_t     *SHARED domain = NULL;

static INLINE
SHARED Real_t *Allocate_Real_t( size_t hcSize ) {
  return  (SHARED Real_t *)SPAD_MALLOC(hcSize,sizeof(Real_t)); // RAG using SPAD_MALLOC but really getting a DRAM datablock
} // Allocate_Real_t()

static INLINE
void     Release_Real_t( SHARED Real_t *ptr ) {
  if(ptr != NULL) SPAD_FREE(ptr);                       // RAG usein SPAD_FREE but really freeing a DRAM datablock
} // Release_Real_t()

/* RAG -- prototypes for the edt functions */
#include "RAG_edt.h"
/* RAG -- moved serial/scalar functions to here */
#include "kernels.h"
/* RAG -- moved initialization code here */
#include "initialize.h"

#if defined(OCR) || defined(FSIM)
ocrGuid_t  beginEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t middleEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t    endEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
#if defined(USE_EDT)
SHARED ocrGuid_t LagrangeLeapFrogEdtGuid, LagrangeLeapFrogEdtTempGuid;
ocrGuid_t LagrangeLeapFrog_edt_0(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
SHARED ocrGuid_t ShowProgressEdtGuid,     ShowProgressEdtTempGuid;
ocrGuid_t ShowProgress_edt_0(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
#endif
#endif

static INLINE
void InitStressTermsForElems(Index_t numElem, 
                             SHARED Real_t *sigxx, SHARED Real_t *sigyy, SHARED Real_t *sigzz) {
  // pull in the stresses appropriate to the hydro integration
  FINISH
    EDT_PAR_FOR_0xNx1(i,numElem,InitStressTermsForElems_edt_1,domain,sigxx,sigyy,sigzz);
  END_FINISH
} // InitStressTermsForElems()

static INLINE
void IntegrateStressForElems( Index_t numElem,
                              SHARED Real_t *sigxx, SHARED Real_t *sigyy, SHARED Real_t *sigzz,
                              SHARED Real_t *determ) {
  // loop over all elements
  FINISH
    EDT_PAR_FOR_0xNx1(i,numElem,IntegrateStressForElems_edt_1,domain,sigxx,sigyy,sigzz,determ);
  END_FINISH
} // IntegrateStressForElems()

static INLINE
void CalcFBHourglassForceForElems( SHARED Real_t *determ,
            SHARED Real_t *x8n,      SHARED Real_t *y8n,      SHARED Real_t *z8n,
            SHARED Real_t *dvdx,     SHARED Real_t *dvdy,     SHARED Real_t *dvdz,
            Real_t hourg) {
  /*************************************************
   *
   *     FUNCTION: Calculates the Flanagan-Belytschko anti-hourglass
   *               force.
   *
   *************************************************/
  Index_t numElem = domain->m_numElem ;
// compute the hourglass modes
  FINISH
    EDT_PAR_FOR_0xNx1(i2,numElem,CalcFBHourglassForceForElems_edt_1,domain,determ,x8n,y8n,z8n,dvdx,dvdy,dvdz,hourg)
  END_FINISH
} // CalcFBHourglassForceForElems

static INLINE
void CalcHourglassControlForElems( SHARED Real_t *determ, Real_t hgcoef ) {
TRACE6("CalcHourglassControlForElems entry");
  Index_t numElem = domain->m_numElem ;
  Index_t numElem8 = numElem * EIGHT ;

TRACE6("Allocate dvdx,dvdy,dvdz,x8n,y8n,z8n");
  SHARED Real_t *dvdx = Allocate_Real_t(numElem8) ;
  SHARED Real_t *dvdy = Allocate_Real_t(numElem8) ;
  SHARED Real_t *dvdz = Allocate_Real_t(numElem8) ;
  SHARED Real_t *x8n  = Allocate_Real_t(numElem8) ;
  SHARED Real_t *y8n  = Allocate_Real_t(numElem8) ;
  SHARED Real_t *z8n  = Allocate_Real_t(numElem8) ;

TRACE6("/* start loop over elements */");
  FINISH
    EDT_PAR_FOR_0xNx1(i,numElem,CalcHourglassControlForElems_edt_1,domain,determ,x8n,y8n,z8n,dvdx,dvdy,dvdz)
  END_FINISH

  if ( hgcoef > cast_Real_t(0.) ) {
TRACE6("Call CalcFBHourglassForceForElems()");
    CalcFBHourglassForceForElems(determ,x8n,y8n,z8n,dvdx,dvdy,dvdz,hgcoef) ;
  }

TRACE6("Release  z8n,y8n,x8n,dvdz,dvdy,dvdx");
  Release_Real_t(z8n) ;
  Release_Real_t(y8n) ;
  Release_Real_t(x8n) ;
  Release_Real_t(dvdz) ;
  Release_Real_t(dvdy) ;
  Release_Real_t(dvdx) ;

TRACE6("CalcHourglassControlForElems return");
  return ;
} // CalcHourglassControlForElems

static INLINE
void CalcVolumeForceForElems( SHARED struct Domain_t *domain ) {
TRACE5("CalcVolueForceForElems() entry");
  Index_t numElem = domain->m_numElem ;

  if (numElem != 0) {
    Real_t  hgcoef = domain->m_hgcoef ;

TRACE5("Allocate sigxx,sigyy,sigzz,determ");
    SHARED Real_t *sigxx  = Allocate_Real_t(numElem) ;
    SHARED Real_t *sigyy  = Allocate_Real_t(numElem) ;
    SHARED Real_t *sigzz  = Allocate_Real_t(numElem) ;
    SHARED Real_t *determ = Allocate_Real_t(numElem) ;

TRACE5("/* Sum contributions to total stress tensor */");
    InitStressTermsForElems( numElem , sigxx , sigyy , sigzz );

TRACE5("// call elemlib stress integration loop to produce nodal forces from");
TRACE5("// material stresses.");
    IntegrateStressForElems( numElem , sigxx , sigyy , sigzz , determ) ;

TRACE5("// check for negative element volume");
    FINISH
      EDT_PAR_FOR_0xNx1(i,numElem,CalcVolumeForceForElems_edt_1,determ)
    END_FINISH

TRACE5("// Hourglass Control for Elems");
    CalcHourglassControlForElems(determ, hgcoef) ;

TRACE5("Release  determ,sigzz,sigyy,sigxx");
    Release_Real_t(determ) ;
    Release_Real_t(sigzz) ;
    Release_Real_t(sigyy) ;
    Release_Real_t(sigxx) ;
  } // if numElem

TRACE5("CalcVolueForceForElems() entry");
} // CalcVolumeForceForElems()

static INLINE void CalcForceForNodes( SHARED struct Domain_t *domain ) {
  Index_t numNode = domain->m_numNode ;
  FINISH
    EDT_PAR_FOR_0xNx1(i,numNode,CalcForceForNodes_edt_1,domain)
  END_FINISH

TRACE4("/* Calcforce calls partial, force, hourq */");

  CalcVolumeForceForElems(domain) ;

TRACE4("/* Calculate Nodal Forces at domain boundaries */");
TRACE4("/* problem->commSBN->Transfer(CommSBN::forces); */");

} // CalcForceForNodes()

static INLINE
void CalcAccelerationForNodes() {
  Index_t numNode = domain->m_numNode ;
  FINISH
    EDT_PAR_FOR_0xNx1(i,numNode,CalcAccelerationForNodes_edt_1,domain)
  END_FINISH
} // CalcAccelerationForNodes()

static INLINE
void ApplyAccelerationBoundaryConditionsForNodes() {
  Index_t numNodeBC = (domain->m_sizeX+1)*(domain->m_sizeX+1) ;
  FINISH
    EDT_PAR_FOR_0xNx1(i,numNodeBC,ApplyAccelerationBoundaryConditionsForNodes_edt_1,domain);
  END_FINISH
} // ApplyAccelerationBoundaryConditionsForNodes()

static INLINE
void CalcVelocityForNodes(const Real_t dt, const Real_t u_cut) {
  Index_t numNode = domain->m_numNode ;
  FINISH
    EDT_PAR_FOR_0xNx1(i,numNode,CalcVelocityForNodes_edt_1,domain,dt,u_cut)
  END_FINISH
} // CalcVelocityForNodes()

static INLINE
void CalcPositionForNodes(const Real_t dt) {
  Index_t numNode = domain->m_numNode ;
//DEBUG fprintf(stdout,"CPFN:dt= %e\n",dt);
  FINISH
    EDT_PAR_FOR_0xNx1(i,numNode,CalcPositionForNodes_edt_1,domain,dt)
  END_FINISH
} // CalcPositionForNodes()

static INLINE
void LagrangeNodal(SHARED struct Domain_t *domain) {
  HAB_CONST Real_t delt = domain->m_deltatime ;
  Real_t u_cut = domain->m_u_cut ;

TRACE2("/* time of boundary condition evaluation is beginning of step for force and");
TRACE2(" * acceleration boundary conditions. */");

TRACE2(" /* Call CalcForceForNodes() */");

  CalcForceForNodes(domain);

TRACE2(" /* Call AccelerationForNodes() */");

  CalcAccelerationForNodes();

TRACE2(" /* Call ApplyAccelerationBoundaryConditionsForNodes() */");

  ApplyAccelerationBoundaryConditionsForNodes();

TRACE2(" /* Call CalcVelocityForNOdes() */");

  CalcVelocityForNodes( delt, u_cut ) ;

TRACE2(" /* Call CalcPositionForNodes() */");

  CalcPositionForNodes( delt );

  return;
} // LagrangeNodal()

static INLINE
void CalcKinematicsForElems( Index_t numElem, Real_t dt ) {
  FINISH
    // loop over all elements
    EDT_PAR_FOR_0xNx1(i,numElem,CalcKinematicsForElems_edt_1,domain,dt);
  END_FINISH
} // CalcKinematicsForElems()

static INLINE
void CalcLagrangeElements(Real_t deltatime) {
   Index_t numElem = domain->m_numElem ;
   if (numElem > 0) {
      CalcKinematicsForElems(numElem, deltatime) ;

      // element loop to do some stuff not included in the elemlib function.
      FINISH
        EDT_PAR_FOR_0xNx1(i,numElem,CalcLagrangeElements_edt_1,domain);
      END_FINISH
   } // if numElem
} // CalcLagrangeElements()

static INLINE
void CalcMonotonicQGradientsForElems() {
  Index_t numElem = domain->m_numElem ;
  FINISH
    EDT_PAR_FOR_0xNx1(i,numElem,CalcMonotonicQGradientsForElems_edt_1,domain)
  END_FINISH
} // CalcMonotonicQGradientsForElems()

static INLINE
void CalcMonotonicQRegionForElems(
                          Real_t qlc_monoq,            // parameters
                          Real_t qqc_monoq,
                          Real_t monoq_limiter_mult,
                          Real_t monoq_max_slope,
                          Real_t ptiny,
                          Index_t elength ) {          // the elementset length
  FINISH
    EDT_PAR_FOR_0xNx1(ielem,elength,CalcMonotonicQRegionForElems_edt_1,domain,qlc_monoq,qqc_monoq,monoq_limiter_mult,monoq_max_slope,ptiny);
  END_FINISH
} // CalcMonotonicQRegionForElems()

static INLINE
void CalcMonotonicQForElems() {
   //
   // initialize parameters
   // 
   HAB_CONST Real_t ptiny    = cast_Real_t(1.e-36) ;
   Real_t monoq_max_slope    = domain->m_monoq_max_slope ;
   Real_t monoq_limiter_mult = domain->m_monoq_limiter_mult ;

   //
   // calculate the monotonic q for pure regions
   //
   Index_t elength = domain->m_numElem ;
   if (elength > 0) {
      Real_t qlc_monoq = domain->m_qlc_monoq;
      Real_t qqc_monoq = domain->m_qqc_monoq;
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

static SHARED uint64_t index_AMO;
static SHARED uint64_t *pIndex_AMO = &index_AMO;

static INLINE
void CalcQForElems() {
   Real_t qstop = domain->m_qstop ;
   Index_t numElem = domain->m_numElem ;

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
#ifdef    UPC
      bupc_atomicU64_set_strict(pIndex_AMO,(uint64_t)0);
#else  // NOT UPC
      *pIndex_AMO = 0;
#endif // UPC
      FINISH
        EDT_PAR_FOR_0xNx1(i,numElem,CalcQForElems_edt_1,domain,qstop,pIndex_AMO);
      END_FINISH

#ifdef    UPC
      if ( bupc_atomicU64_read_strict(pIndex_AMO) > 0 ) {
         EXIT(QStopError) ;
      } // if pIndex_AMO
#else  // NOT UPC
      if( *pIndex_AMO > 0 ) {
         EXIT(QStopError) ;
      } // if pIndex_AMO
#endif // UPC
   } // if numElem
} // CalcQForElems()

#if 0  // RAG -- inlined CalcPressureForElems_edt_1 into CalcEnergyForElems()
static INLINE
void CalcPressureForElems(Real_t* p_new, Real_t* bvc,
                          Real_t* pbvc, Real_t* e_old,
                          Real_t* compression, Real_t *vnewc,
                          Real_t pmin,
                          Real_t p_cut, Real_t eosvmax,
                          Index_t length) {
  FINISH
    EDT_PAR_FOR_0xNx1(i,length,CalcPressureForElems_edt_1,p_new,bvc,pbvc,e_old,compression,vnewc,pmin,p_cut,eosvmax)
  END_FINISH
} // CalcPressureForElems()
#endif

static INLINE
void CalcEnergyForElems( SHARED Real_t *p_new, SHARED Real_t *e_new, SHARED Real_t *q_new,
                         SHARED Real_t *bvc, SHARED Real_t *pbvc,
                         SHARED Real_t *p_old, SHARED Real_t *e_old, SHARED Real_t *q_old,
                         SHARED Real_t *compression, SHARED Real_t *compHalfStep,
                         SHARED Real_t *vnewc, SHARED Real_t *work, SHARED Real_t *delvc,
                         Real_t pmin, Real_t p_cut, Real_t  e_cut, Real_t q_cut, Real_t emin,
                         SHARED Real_t *qq, SHARED Real_t *ql,
                         Real_t rho0, Real_t eosvmax, Index_t length ) {
  SHARED Real_t *pHalfStep = Allocate_Real_t(length) ;
  FINISH // RAG STRIDE ONE                                OUT
    EDT_PAR_FOR_0xNx1(i,length,CalcEnergyForElems_edt_1,  e_new,e_old,delvc,p_old,q_old,work,
                                                          emin);
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                OUT       OUT OUT  *IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcPressureForElems_edt_1,pHalfStep,bvc,pbvc,e_new,compHalfStep,vnewc,
                                                          pmin,p_cut,eosvmax);
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                OUT   IN/OUT                  *IN* *IN* *IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcEnergyForElems_edt_2,  q_new,e_new,compHalfStep,qq,ql,pbvc,bvc,pHalfStep,delvc,p_old,q_old,work,
                                                          rho0,e_cut,emin);
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                OUT   OUT OUT  *IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcPressureForElems_edt_1,p_new,bvc,pbvc,e_new,compression,vnewc,
                                                          pmin,p_cut,eosvmax)
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                IN/OUT     *IN*       *IN**IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcEnergyForElems_edt_3,  e_new,delvc,pbvc,vnewc,bvc,p_new,ql,qq,p_old,q_old,pHalfStep,q_new,
                                                          rho0,e_cut,emin);
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                OUT   OUT OUT  *IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcPressureForElems_edt_1,p_new,bvc,pbvc,e_new,compression,vnewc,
                                                          pmin,p_cut,eosvmax);
  END_FINISH // RAG STRIDE ONE
  FINISH // RAG STRIDE ONE                                OUT        *IN*             *IN**IN*
    EDT_PAR_FOR_0xNx1(i,length,CalcEnergyForElems_edt_4,  q_new,delvc,pbvc,e_new,vnewc,bvc,p_new,ql,qq,
                                                          rho0,q_cut);
  END_FINISH // RAG STRIDE ONE
  Release_Real_t(pHalfStep) ;
  return ;
} // CalcEnergyForElems()

#if 0
static INLINE
void CalcSoundSpeedForElems(Real_t *vnewc, Real_t rho0, Real_t *enewc,
                            Real_t *pnewc, Real_t *pbvc,
                            Real_t *bvc,   Index_t nz) {
  FINISH // RAG SCATTERS
    EDT_PAR_FOR_0xNx1(i,nz,CalcSoundSpeedForElems_edt_1,domain,vnewc,enewc,pnewc,pbvc,bvc,rho0)
  END_FINISH // RAG SCATTERs
} // CalcSoundSpeedForElems()
#endif

static INLINE
void EvalEOSForElems( SHARED Real_t *vnewc, Index_t length ) {
  Real_t  e_cut = domain->m_e_cut;
  Real_t  p_cut = domain->m_p_cut;
  Real_t  q_cut = domain->m_q_cut;

  Real_t eosvmax = domain->m_eosvmax ;
  Real_t eosvmin = domain->m_eosvmin ;
  Real_t pmin    = domain->m_pmin ;
  Real_t emin    = domain->m_emin ;
  Real_t rho0    = domain->m_refdens ;

  SHARED Real_t *e_old = Allocate_Real_t(length) ;
  SHARED Real_t *delvc = Allocate_Real_t(length) ;
  SHARED Real_t *p_old = Allocate_Real_t(length) ;
  SHARED Real_t *q_old = Allocate_Real_t(length) ;
  SHARED Real_t *compression = Allocate_Real_t(length) ;
  SHARED Real_t *compHalfStep = Allocate_Real_t(length) ;
  SHARED Real_t *qq = Allocate_Real_t(length) ;
  SHARED Real_t *ql = Allocate_Real_t(length) ;
  SHARED Real_t *work = Allocate_Real_t(length) ;
  SHARED Real_t *p_new = Allocate_Real_t(length) ;
  SHARED Real_t *e_new = Allocate_Real_t(length) ;
  SHARED Real_t *q_new = Allocate_Real_t(length) ;
  SHARED Real_t *bvc = Allocate_Real_t(length) ;
  SHARED Real_t *pbvc = Allocate_Real_t(length) ;

TRACE1("/* compress data, minimal set */");

  FINISH // RAG GATHERS
#if 1
    EDT_PAR_FOR_0xNx1(i,length,EvalEOSForElems_edt_1,domain,delvc,e_old,p_old,q_old,qq,ql);
#else
    PAR_FOR_0xNx1(i,length,domain,delvc,e_old,p_old,q_old,qq,ql)
      Index_t zidx = domain->m_matElemlist[i] ;
      e_old[i] = domain->m_e[zidx] ;
      delvc[i] = domain->m_delv[zidx] ;
      p_old[i] = domain->m_p[zidx] ;
      q_old[i] = domain->m_q[zidx] ;
      qq[i]    = domain->m_qq[zidx] ;
      ql[i]    = domain->m_ql[zidx] ;
    END_PAR_FOR(i)
#endif
  END_FINISH // RAG GATHERS

TRACE1("/* Check for v > eosvmax or v < eosvmin */");

  FINISH // RAG STRIDE ONE
#if 1
    EDT_PAR_FOR_0xNx1(i,length,EvalEOSForElems_edt_2,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax);
#else
    PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
      Real_t vchalf ;
      compression[i] = cast_Real_t(1.) / vnewc[i] - cast_Real_t(1.);
      vchalf = vnewc[i] - delvc[i] * cast_Real_t(.5);
      compHalfStep[i] = cast_Real_t(1.) / vchalf - cast_Real_t(1.);
      work[i] = cast_Real_t(0.) ;
//  END_PAR_FOR(i)

      if ( eosvmin != cast_Real_t(0.) ) {
//    PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
        if (vnewc[i] <= eosvmin) { /* impossible due to calling func? */
          compHalfStep[i] = compression[i] ;
        } // if vnewc
//    END_PAR_FOR(i)
      } // if eosvmin

      if ( eosvmax != cast_Real_t(0.) ) {
//    PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
        if (vnewc[i] >= eosvmax) { /* impossible due to calling func? */
          p_old[i]        = cast_Real_t(0.) ;
          compression[i]  = cast_Real_t(0.) ;
          compHalfStep[i] = cast_Real_t(0.) ;
        } // if vnewc
//    END_PAR_FOR(i)
      } // if eosvmax

    END_PAR_FOR(i)
#endif
  END_FINISH // RAG STRIDE ONE

  CalcEnergyForElems(p_new, e_new, q_new, bvc, pbvc,
                     p_old, e_old, q_old, compression, compHalfStep,
                     vnewc, work,  delvc, pmin,
                     p_cut, e_cut, q_cut, emin,
                     qq, ql, rho0, eosvmax, length);


  FINISH     // RAG SCATTERS
#if 1
    EDT_PAR_FOR_0xNx1(i,length,EvalEOSForElems_edt_3,domain,p_new,e_new,q_new);
#else
    PAR_FOR_0xNx1(i,length,domain,p_new,e_new,q_new)
      Index_t zidx = domain->m_matElemlist[i] ;
      domain->m_p[zidx] = p_new[i] ;
      domain->m_e[zidx] = e_new[i] ;
      domain->m_q[zidx] = q_new[i] ;
    END_PAR_FOR(i)
#endif
  END_FINISH // RAG SCATTERS

#if 1
  FINISH     // RAG SCATTERS
    EDT_PAR_FOR_0xNx1(i,length,CalcSoundSpeedForElems_edt_1,domain,vnewc,e_new,p_new,pbvc,bvc,rho0)
  END_FINISH // RAG SCATTERs
#else
  CalcSoundSpeedForElems(vnewc, rho0, e_new, p_new,
                         pbvc, bvc, length) ;
#endif

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

static INLINE
void ApplyMaterialPropertiesForElems() {
  Index_t length = domain->m_numElem ;

  if (length != 0) {
TRACE1("/* Expose all of the variables needed for material evaluation */");
    Real_t eosvmin = domain->m_eosvmin ;
    Real_t eosvmax = domain->m_eosvmax ;
    SHARED Real_t *vnewc  = Allocate_Real_t(length) ;

    FINISH // RAG GATHERS
      EDT_PAR_FOR_0xNx1(i,length,ApplyMaterialPropertiesForElems_edt_1,domain,vnewc) ;
    END_FINISH // RAG GATHERS

    FINISH // RAG STIDE ONES
      EDT_PAR_FOR_0xNx1(i,length,ApplyMaterialPropertiesForElems_edt_2,vnewc,eosvmin,eosvmax) ;
    END_FINISH // RAG STIDE ONES

    FINISH // RAG GATHER with ERROR CHECK
      EDT_PAR_FOR_0xNx1(i,length,ApplyMaterialPropertiesForElems_edt_3,domain,eosvmin,eosvmax) ;
    END_FINISH // RAG GATHER with ERROR CHECK

    EvalEOSForElems(vnewc, length);

    Release_Real_t(vnewc) ;

  } // if length

} // ApplyMaterialPropertiesForElems()

static INLINE
void UpdateVolumesForElems() {
  Index_t numElem = domain->m_numElem;
  if (numElem != 0) {
    Real_t v_cut = domain->m_v_cut;
    FINISH
      EDT_PAR_FOR_0xNx1(i,numElem,UpdateVolumesForElems_edt_1,domain,v_cut)
    END_FINISH
  } // if numElem
  return ;
} // UpdateVolumesForElems()

static INLINE
void LagrangeElements(SHARED struct Domain_t *domain) {
  HAB_CONST Real_t deltatime = domain->m_deltatime ;

TRACE2("/* Call CalcLagrangeElements() */");

  CalcLagrangeElements(deltatime) ;

TRACE2("/* Call CalcQForElems() -- Calculate Q.  (Monotonic q option requires communication) */");

  CalcQForElems() ;

TRACE2("/* Call ApplyMaterialPropertiesForElems() */");

  ApplyMaterialPropertiesForElems() ;

TRACE2("/* Call UpdateVolumesForElems() */");

  UpdateVolumesForElems() ;

} // LagrangeElements()

static SHARED Real_t    DtCourant;
static SHARED Index_t   Courant_elem;
static SHARED Real_t  *pDtCourant    = &DtCourant;
static SHARED Index_t *pCourant_elem = &Courant_elem;

static INLINE
void CalcCourantConstraintForElems(SHARED struct Domain_t *domain) {
  *pDtCourant = cast_Real_t(1.0e+20) ;
  *pCourant_elem = -1;

  FINISH
    Real_t      qqc = domain->m_qqc ;
    Real_t  qqc2 = cast_Real_t(64.0) * qqc * qqc ;
    Index_t length = domain->m_numElem ;
    PAR_FOR_0xNx1(i,length,domain,qqc2,pDtCourant,pCourant_elem,pidamin_lock)
      Index_t indx = domain->m_matElemlist[i] ;

      Real_t dtf = domain->m_ss[indx] * domain->m_ss[indx] ;

      if ( domain->m_vdov[indx] < cast_Real_t(0.) ) {

        dtf = dtf
            + qqc2 * domain->m_arealg[indx] * domain->m_arealg[indx]
            * domain->m_vdov[indx] * domain->m_vdov[indx] ;
      } // if domain->m_vdov

      dtf = SQRT(dtf) ;

      dtf = domain->m_arealg[indx] / dtf ;

      /* determine minimum timestep with its corresponding elem */
      if (domain->m_vdov[indx] != cast_Real_t(0.)) {
        if ( dtf < *pDtCourant ) {
AMO__lock_uint64_t(pidamin_lock);          // LOCK
          if ( dtf < *pDtCourant ) {
            *pDtCourant    = dtf ;
            *pCourant_elem = indx ;
          } // if *pDtCourant
AMO__unlock_uint64_t(pidamin_lock);        // UNLOCK
        } // if *pDtCourant
      } // if domain->m_vdov

    END_PAR_FOR(i)
  END_FINISH

  /* Don't try to register a time constraint if none of the elements
   * were active */
  if ( *pCourant_elem != -1) {
//DEBUG fprintf(stdout,"dtcourant %e\n",domain->m_dtcourant);
     domain->m_dtcourant = *pDtCourant ;
  } // if *pCourant_elem

  return ;
} // CalcCourantConstraintForElems()

static SHARED Real_t    DtHydro;
static SHARED Index_t   Hydro_elem;
static SHARED Real_t  *pDtHydro    = &DtHydro;
static SHARED Index_t *pHydro_elem = &Hydro_elem;

static INLINE
void CalcHydroConstraintForElems(SHARED struct Domain_t *domain) {
  *pDtHydro = cast_Real_t(1.0e+20) ;
  *pHydro_elem = -1 ;

  FINISH // IDAMIN via AMO
    Real_t dvovmax = domain->m_dvovmax ;
    Index_t length = domain->m_numElem ;
#if 1
    EDT_PAR_FOR_0xNx1(i,length,CalcHydroConstraintForElems_edt_1,domain,pDtHydro,pHydro_elem,pidamin_lock,dvovmax);
#else
    PAR_FOR_0xNx1(i,length,domain,dvovmax,pidamin_lock,pDtHydro,pHydro_elem)
      Index_t indx = domain->m_matElemlist[i] ;

      if (domain->m_vdov[indx] != cast_Real_t(0.)) {
        Real_t dtdvov = dvovmax / (FABS(domain->m_vdov[indx])+cast_Real_t(1.e-20)) ;
        if ( *pDtHydro > dtdvov ) {
AMO__lock_uint64_t(pidamin_lock);          // LOCK
          if ( *pDtHydro > dtdvov ) {
            *pDtHydro    = dtdvov ;
            *pHydro_elem = indx ;
        } // if *pDtHydro
AMO__unlock_uint64_t(pidamin_lock);        // UNLOCK
        } // if *pDtHydro
      } // if domain->m_vdov

    END_PAR_FOR(i)
#endif
  END_FINISH // IDAMIN via AMO

  if (*pHydro_elem != -1) {
     domain->m_dthydro = *pDtHydro ;
  } // if *pHydro_elem

  return ;
} // CalcHydroConstraintForElems()

static INLINE
void CalcTimeConstraintsForElems(SHARED struct Domain_t *domain) {
TRACE3("CalcTimeConstrantsForElems() entry");

TRACE3("/* evaluate time constraint */");
  CalcCourantConstraintForElems(domain) ;

TRACE3("/* check hydro constraint */");
  CalcHydroConstraintForElems(domain) ;

TRACE3("CalcTimeConstrantsForElems() return");
} // CalcTimeConstraintsForElems()

#if defined(USE_EDT_CALL) || defined(USE_EDT)
DEF_EDT_CALL(LagrangeLeapFrog,domain,ShowProgress)
#else
static INLINE
void LagrangeLeapFrog(SHARED struct Domain_t *domain) {
#endif
TRACE1("LagrangeLeapFrog() entry");

TRACE1("/* calculate nodal forces, accelerations, velocities, positions, with */");
TRACE1(" * applied boundary conditions and slide surface considerations       */");

  LagrangeNodal(domain);

TRACE1("/* calculate element quantities (i.e. velocity gradient & q), and update */");
TRACE1(" * material states                                                       */");

  LagrangeElements(domain);

TRACE1("/* calculate time constraints for elems */");

  CalcTimeConstraintsForElems(domain);

#if defined(USE_EDT)
  { uint8_t retVal = 0;
TRACE1("call ocrAddDependence(ShowProgressEdtGuid)");
    retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,(ocrGuid_t)ShowProgressEdtGuid,0,DB_MODE_ITW);
    if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);
  }
#endif

TRACE1("LagrangeLeapFrog() return");
#if defined(USE_EDT)
  return NULL_GUID;
#endif
} // LagangeLeapFrog()

#if defined(USE_EDT_CALL) || defined(USE_EDT)
DEF_EDT_CALL(ShowProgress,domain,middle)
#else
static INLINE
void ShowProgress(SHARED struct Domain_t *domain) {
#endif
#if       LULESH_SHOW_PROGRESS
#ifdef      FSIM
  xe_printf("time = %16.16lx, dt=%16.16lx, e(0)=%16.16lx\n",
          *(uint64_t *)&(domain->m_time),
          *(uint64_t *)&(domain->m_deltatime),
          *(uint64_t *)&(domain->m_e[0])) ;
#else    // NOT FSIM
#ifdef        HEX
     printf("time = %16.16lx, dt=%16.16lx, e(0)=%16.16lx\n",
          *(uint64_t *)&(domain->m_time),
          *(uint64_t *)&(domain->m_deltatime),
          *(uint64_t *)&(domain->m_e[0])) ;
#else      // NOT HEX
    printf("time = %e, dt=%e, e(0)=%e\n",
          ((double)domain->m_time),
          ((double)domain->m_deltatime),
          ((double)domain->m_e[0])) ;
#endif     // HEX
    fflush(stdout);
#endif   // FSIM
#endif // LULESH_SHOW_PROGRESS

#if defined(USE_EDT)
{ uint8_t retVal = 0;
  ocrGuid_t middleEdtGuid, middleEdtTempGuid;

TRACE1("call ocrEdtTemplateCreate(middleEdtTempGuid)");
  retVal = ocrEdtTemplateCreate(&middleEdtTempGuid,middleEdt,0,1);
  if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
TRACE1("call ocrEdtCreate(middleEdtGuid)");
  retVal = ocrEdtCreate(&middleEdtGuid, middleEdtTempGuid,
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
  if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
TRACE1("call ocrEdtTemplateDestroy(middleEdtTempGuid)");
//retVal = ocrEdtTemplateDestroy(middleEdtTempGuid);
//if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);

TRACE1("call ocrAddDependence(middleEdtGuid)");
  retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,middleEdtGuid,0,DB_MODE_ITW);
  if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);

}
#endif

#if defined(USE_EDT_CALL) || defined(USE_EDT)
  return NULL_GUID;
#else
  return;
#endif
} // ShowProgress()


#if    defined(OCR)
ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
#elif defined(FSIM)
int mainEdt() {
TRACE1("mainEdt entry");
#endif // FSIM or OCR
  uint8_t retVal = 0;
#if     defined(FSIM) || defined(OCR)
// tiny problem size
  size_t  edgeElems =  5 ;
// ran to completion quickly with 5, many cycles for 10, 15, 30 and 45
#else   // FSIM
  size_t  edgeElems = 45 ; // standard problem size
#endif // FSIM

  size_t  edgeNodes = edgeElems+1 ;

TRACE0("/* ALLOCATE DOMAIN DATA STRUCTURE */");

#if defined(FSIM) || defined(OCR)
  DOMAIN_CREATE(&domainObject,edgeElems,edgeNodes);
//xe_printf("rag: domainObject at        %16.16lx\n",&domainObject);
#if    defined(FSIM)
//xe_printf("rag: domainObject.guid      %16.16lx\n",domainObject.guid.data);
#elif  defined(OCR)
//xe_printf("rag: domainObject.guid      %16.16lx\n",domainObject.guid);
#endif // FSIM or OCR
//xe_printf("rag: domainObject.base      %16.16lx\n",domainObject.base);
//xe_printf("rag: domainObject.offset    %16.16lx\n",domainObject.offset);
//xe_printf("rag: domainObject.limit     %16.16lx\n",domainObject.limit);
//xe_printf("rag: domainObject.edgeElems %16.16lx\n",domainObject.edgeElems);
//xe_printf("rag: domainObject.edgeNodes %16.16lx\n",domainObject.edgeNodes);
#endif // FSIM or OCR

  domain = (SHARED struct Domain_t *)DRAM_MALLOC(ONE,sizeof(struct Domain_t));

#if defined(FSIM)
  xe_printf("rag: domain %16.16lx\n",(uint64_t)domain);
#endif
#if defined(FSIM) || defined(OCR) || defined(UPC)
  for( size_t i = 0; i < ONE*sizeof(struct Domain_t) ; ++i ) {
     char *ptr = (char *)domain;
     ptr[i] = (char)0;
  } // for i
#else // DEFAULT all the others
  memset(domain,0,sizeof(ONE*sizeof(struct Domain_t)));
#endif // FSIM or OCR

TRACE0("/* INITIALIZE SEDOV MESH  */");

ocrGuid_t beginEdtGuid, beginEdtTempGuid;
TRACE1("call ocrEdtTemplateCreate(beginEdtTempGuid)");
  retVal = ocrEdtTemplateCreate(&beginEdtTempGuid,beginEdt,0,1);
  if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
TRACE1("call ocrEdtCreate(beginEdtGuid)");
  retVal = ocrEdtCreate(&beginEdtGuid, beginEdtTempGuid,
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
  if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
TRACE1("call ocrEdtTemplateDestroy(beginEdtTempGuid)");
//retVal = ocrEdtTemplateDestroy(beginEdtTempGuid);
//if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);
TRACE1("call ocrAddDependence(beginEdtGuid)");
  retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,beginEdtGuid,0,DB_MODE_ITW);
  if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);

TRACE1("mainEdt return");
#if   defined(FSIM)
  return 0;
#elif defined(OCR)
  return NULL_GUID;
#endif // FSIM or OCR
} // mainEdt()

////////////////////////////////////
//////////// beginEDT //////////////
////////////////////////////////////
ocrGuid_t beginEdt (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
TRACE1("beginEdt entry");
uint8_t retVal = 0;
SHARED struct Domain_t *domain = (SHARED struct Domain_t *)depv[0].ptr;
//xe_printf("rag: domain %16.16lx\n",domain);
//xe_printf("rag: domainObject at %16.16lx\n",&domainObject);
Index_t edgeElems = (&domainObject)->edgeElems;
//xe_printf("rag: edgeElems %d\n",edgeElems);
Index_t edgeNodes = (&domainObject)->edgeNodes;
//xe_printf("rag: edgeNodes %d\n",edgeNodes);
//xe_printf("rag: call InitializeProblem\n");
  InitializeProblem( domain, edgeElems , edgeNodes );
//xe_printf("rag: back InitializeProblem\n");

  ocrGuid_t middleEdtGuid, middleEdtTempGuid;

TRACE1("call ocrEdtTemplateCreate(middleEdtTempGuid)");
  retVal = ocrEdtTemplateCreate(&middleEdtTempGuid,middleEdt,0,1);
  if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
TRACE1("call ocrEdtCreate(middleEdtGuid)");
  retVal = ocrEdtCreate(&middleEdtGuid, middleEdtTempGuid,
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
  if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
TRACE1("call ocrEdtTemplateDestroy(middleEdtTempGuid)");
//retVal = ocrEdtTemplateDestroy(middleEdtTempGuid);
//if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);

TRACE1("call ocrAddDependence(middleEdtGuid)");
  retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,middleEdtGuid,0,DB_MODE_ITW);
  if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);

TRACE1("beginEdt return");
  return NULL_GUID;
}// beginEdt()
/////////////////////////////////////
//////////// middleEDT //////////////
/////////////////////////////////////
ocrGuid_t middleEdt (u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
uint8_t retVal = 0;
TRACE1("middleEdt entry");
SHARED struct Domain_t *domain = (SHARED struct Domain_t *)depv[0].ptr;
//xe_printf("rag: domain %16.16lx\n",domain);
#if defined(USE_EDT)
  if(domain->m_time < domain->m_stoptime ) {
#else
  while(domain->m_time < domain->m_stoptime ) {
#endif

TRACE0("/* TimeIncrement() */");

    TimeIncrement(domain) ;

TRACE0("/* LagrangeLeapFrog() */");

#if defined(USE_CALL)
    EDT_CALL(LagrangeLeapFrog,domain,ShowProgress) ;
#elif defined(USE_EDT_CALL)
    MAKE_EDT_CALL(LagrangeLeapFrog,domain,ShowProgress) ;
#elif defined(USE_EDT)

TRACE1("call ocrEdtTemplateCreate(LagrangeLeapFrogEdtTempGuid)");
    ocrGuid_t NSLagrangeLeapFrogEdtTempGuid; // RAG FIXME WORKAROUND TO NOT ALLOWING SHARED
    retVal = ocrEdtTemplateCreate(&NSLagrangeLeapFrogEdtTempGuid,LagrangeLeapFrog_edt_0,0,1);
    if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
    LagrangeLeapFrogEdtTempGuid = NSLagrangeLeapFrogEdtTempGuid;
TRACE1("call ocrEdtCreate(LagrangeLeapFrogEdtGuid)");
    ocrGuid_t NSLagrangeLeapFrogEdtGuid; // RAG FIXME WORKAROUND TO NOT ALLOWING SHARED
    retVal = ocrEdtCreate(&NSLagrangeLeapFrogEdtGuid, NSLagrangeLeapFrogEdtTempGuid,
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
    if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
    LagrangeLeapFrogEdtGuid = NSLagrangeLeapFrogEdtGuid;
TRACE1("call ocrEdtTeimplateDestroy(LagrangeLeapFrogEdtTempGuid)");
//  retVal = ocrEdtTemplateDestroy(LagrangeLeapFrogEdtTempGuid);
//  if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);

#else
    LagrangeLeapFrog(domain) ;
#endif

#if defined(USE_CALL)
    EDT_CALL(ShowProgress,domain,middle);
#elif defined(USE_EDT_CALL)
    MAKE_EDT_CALL(ShowProgress,domain,middle) ;
#elif defined(USE_EDT)

TRACE1("call ocrEdtTemplateCreate(ShowProgressEdtTempGuid)");
    ocrGuid_t NSShowProgressEdtTempGuid; // RAG FIXME WORKAROUND TO NOT ALLOWING SHARED
    retVal = ocrEdtTemplateCreate(&NSShowProgressEdtTempGuid,ShowProgress_edt_0,0,1);
    if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
    ShowProgressEdtTempGuid = NSShowProgressEdtTempGuid;
TRACE1("call ocrEdtCreate(ShowProgressEdtGuid)");
    ocrGuid_t NSShowProgressEdtGuid; // RAG FIXME WORKAROUND TO NOT ALLOWING SHARED
    retVal = ocrEdtCreate(&NSShowProgressEdtGuid, NSShowProgressEdtTempGuid,
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
    if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
    ShowProgressEdtGuid = NSShowProgressEdtGuid;
TRACE1("call ocrEdtTemplateDestroy(ShowProgressEdtTempGuid)");
//  retVal = ocrEdtTemplateDestroy(ShowProgressEdtTempGuid);
//  if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);

#else
    ShowProgress(domain);
#endif

#ifdef USE_EDT
TRACE1("call ocrAddDependence(LagrangeLeapFrogEdtGuid)");
    retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,(ocrGuid_t)LagrangeLeapFrogEdtGuid,0,DB_MODE_ITW);
    if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);
#endif

#if defined(USE_EDT)
  } else { // if time
#else
  } // while time
#endif

  ocrGuid_t endEdtGuid, endEdtTempGuid;

TRACE1("call ocrEdtTemplateCreate(endEdtTempGuid)");
  retVal = ocrEdtTemplateCreate(&endEdtTempGuid,endEdt,0,1);
  if(retVal != 0)xe_printf("ocrEdtTemplateCreate retVal %d\n",retVal);
TRACE1("call ocrEdtCreate(endEdtGuid)");
  retVal = ocrEdtCreate(&endEdtGuid, endEdtTempGuid, 
                       (u32) EDT_PARAM_DEF, (u64 *) NULL,
                       (u32) EDT_PARAM_DEF, (ocrGuid_t *) NULL,
                       (u16) EDT_PROP_NONE, NULL_GUID, (ocrGuid_t *) NULL);
  if(retVal != 0)xe_printf("ocrEdtCreate retVal %d\n",retVal);
TRACE1("call ocrEdtTemplateDestroy(endEdtTempGuid)");
//retVal = ocrEdtTemplateDestroy(endEdtTempGuid);
//if(retVal != 0)xe_printf("ocrEdtTemplateDestroy retVal %d\n",retVal);

TRACE1("call ocrAddDependence(endEdtGuid)");
  retVal = ocrAddDependence((ocrGuid_t)domainObject.guid,endEdtGuid,0,DB_MODE_ITW);
  if(retVal != 0)xe_printf("ocrAddDependence retVal %d\n",retVal);

#if defined(USE_EDT)
  } // if time
#endif
TRACE1("middleEdt return");
  return NULL_GUID;
} // middleEdt()
//////////////////////////////////
//////////// endEDT //////////////
//////////////////////////////////
ocrGuid_t endEdt (u32 paramc, u64 *params, u32 depc, ocrEdtDep_t depv[]) {
TRACE1("endEdt entry");
SHARED struct Domain_t *domain = (SHARED struct Domain_t *)depv[0].ptr;
//xe_printf("domain %16.16lx\n",domain);
#ifdef    FSIM
  xe_printf("   Final Origin Energy = %16.16lx \n", *(SHARED uint64_t *)&domain->m_e[0]) ;
#else //  NOT FSIM
#ifdef      HEX
  printf("   Final Origin Energy = %16.16lx \n", *(SHARED uint64_t *)&domain->m_e[0]) ;
#else    // NOT HEX
  printf("   Final Origin Energy = %12.6e \n", (double)domain->m_e[0]) ;
#endif //   HEX
  fflush(stdout);
#endif // FSIM

#if defined(FSIM) || defined(OCR)
  DOMAIN_DESTROY(&domainObject);
#else
TRACE1("/* Deallocate field memory */");
  domain_DeallocateNodesets() ;
  domain_DeallocateNodalPersistent() ;

  domain_DeallocateElemTemporary () ;
  domain_DeallocateElemPersistent() ;

  DRAM_FREE(domain);
#endif // FSIM or OCR

TRACE1("endEdt exit");
  EXIT(0);
TRACE1("endEdt return");
  return NULL_GUID;
} // endEdt()
