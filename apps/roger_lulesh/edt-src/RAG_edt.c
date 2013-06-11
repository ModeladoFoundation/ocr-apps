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

extern struct Domain_t *domain;

#include "RAG_edt.h"

#include "kernels.h"

void domain_AllocateNodalPersistent_edt_1(Index_t i_out, Index_t i_end, 
                                          SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
      domain->m_xd[i] = (Real_t)(0.0);
      domain->m_yd[i] = (Real_t)(0.0);
      domain->m_zd[i] = (Real_t)(0.0);

      domain->m_xdd[i] = (Real_t)(0.0);
      domain->m_ydd[i] = (Real_t)(0.0);
      domain->m_zdd[i] = (Real_t)(0.0) ;

      domain->m_nodalMass[i] = (Real_t)(0.0) ;
  } // for i
} // domain_AllocateNodalPersistent_edt_1()

void domain_AllocateElemPersistent_edt_1(Index_t i_out, Index_t i_end, 
                                         SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
            domain->m_e[i] = (Real_t)(0.0);
            domain->m_p[i] = (Real_t)(0.0);
            domain->m_v[i] = (Real_t)(1.0);
  } // for i
} // domain_AllocateElemPersistent_edt_1()
 
void InitStressTermsForElems_edt_1(Index_t i_out, Index_t i_end, 
                                   Real_t *sigxx, Real_t *sigyy, Real_t *sigzz, 
                                   SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    sigxx[i] = sigyy[i] = sigzz[i] =  - domain->m_p[i] - domain->m_q[i] ;
  } // for i
} // InitStressTermsForElems_edt_1()

void IntegrateStressForElems_edt_1(Index_t i_out, Index_t i_end, Real_t *sigxx, Real_t *sigyy, Real_t *sigzz, Real_t *determ, struct Domain_t *domain) {
#if  defined(HAB_C)
  Real_t *B        = (Real_t *)malloc(THREE*EIGHT*sizeof(Real_t)) ;
  Real_t *x_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *y_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *z_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fx_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fy_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fz_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
#else // NOT HAB_C
  Real_t B[THREE*EIGHT]; // -- shape function derivatives B[THREE][EIGHT]
  Real_t  x_local[EIGHT],  y_local[EIGHT],  z_local[EIGHT];
  Real_t fx_local[EIGHT], fy_local[EIGHT], fz_local[EIGHT];
#endif //    HAB_C
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    SHARED HAB_CONST Index_t* HAB_CONST elemNodes = (SHARED Index_t *)&domain->m_nodelist[EIGHT*i];
    // get nodal coordinates from global arrays and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemNodes[lnode];
      x_local[lnode] = domain->m_x[gnode];
      y_local[lnode] = domain->m_y[gnode];
      z_local[lnode] = domain->m_z[gnode];
    } // for lnode
    /* Volume calculation involves extra work for numerical consistency. */
    CalcElemShapeFunctionDerivatives(x_local, y_local, z_local,
                                     B, &determ[i]);
    CalcElemNodeNormals( &B[0*EIGHT], &B[1*EIGHT], &B[2*EIGHT],
                         x_local, y_local, z_local );
    SumElemStressesToNodeForces( B, sigxx[i], sigyy[i], sigzz[i],
                                 fx_local, fy_local, fz_local ) ;
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
    // accumulate nodal force contributions to global force arrray.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemNodes[lnode];
      AMO__sync_addition_double(&domain->m_fx[gnode], fx_local[lnode]);
      AMO__sync_addition_double(&domain->m_fy[gnode], fy_local[lnode]);
      AMO__sync_addition_double(&domain->m_fz[gnode], fz_local[lnode]);
    } // for lnode
#if  defined(HAB_C)
    free(fz_local) ; free(fy_local) ; free(fx_local) ;
    free(z_local)  ; free(y_local)  ; free(x_local) ;
    free(B) ;
#endif //    HAB_C
  } // for i
} // IntegrateStressForElems_edt_1()


void CalcFBHourglassForceForElems_edt_1( Index_t i2_out , Index_t i2_end,
                                         Real_t hourg ,
                                         HAB_CONST Real_t *GAMMA , Real_t *determ ,
                                         Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                         Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ,
                                         SHARED struct Domain_t *domain) {
  for(Index_t i2 = i2_out ; i2 < i2_end ; ++i2 ) {
#if  defined(HAB_C)
    Real_t *hgfx = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *hgfy = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *hgfz = (Real_t *)malloc(EIGHT*sizeof(Real_t));

    Real_t *hourgam0 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam1 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam2 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam3 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam4 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam5 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam6 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam7 = (Real_t *)malloc(FOUR*sizeof(Real_t));

    Real_t *xd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *yd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *zd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));

    Index_t *elemToNode  = (Index_t *)malloc(EIGHT*sizeof(Index_t)) ;
#else // NOT HAB_C
    Real_t hgfx[EIGHT], hgfy[EIGHT], hgfz[EIGHT];

    Real_t hourgam0[FOUR], hourgam1[FOUR], hourgam2[FOUR], hourgam3[FOUR];
    Real_t hourgam4[FOUR], hourgam5[FOUR], hourgam6[FOUR], hourgam7[FOUR];

    Real_t  xd1[EIGHT],  yd1[EIGHT],  zd1[EIGHT];

    Index_t elemToNode[EIGHT];
#endif //    HAB_C


    Real_t coefficient;

    Index_t i3=EIGHT*i2;

    Real_t volinv=cast_Real_t(1.0)/determ[i2];
    Real_t ss1, mass1, volume13 ;

// RAG -- GATHER/SCATTER Index Values

    for( Index_t i=0 ; i<EIGHT ; ++i ) {
      elemToNode[i] = *(Index_t *)&domain->m_nodelist[EIGHT*i2+i];
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

    ss1=domain->m_ss[i2];
    mass1=domain->m_elemMass[i2];
    volume13=CBRT(determ[i2]);

// RAG ///////////////////////////////////////////////////////////// RAG //
// RAG  GATHER (x|y|z)d1[0,,7] = domain->m_(x|y|z)[elemToNode[0..7]]  RAG //
// RAG ///////////////////////////////////////////////////////////// RAG //

    for( Index_t i=0 ; i<EIGHT ; ++i ) {
      Index_t gnode = elemToNode[i];
      xd1[i] = domain->m_xd[gnode];
      yd1[i] = domain->m_yd[gnode];
      zd1[i] = domain->m_zd[gnode];
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
      AMO__sync_addition_double(&domain->m_fx[gnode], hgfx[i]);
      AMO__sync_addition_double(&domain->m_fy[gnode], hgfy[i]);
      AMO__sync_addition_double(&domain->m_fz[gnode], hgfz[i]);
    } // for i

#if  defined(HAB_C)
    free(elemToNode);
    free(hgfz); free(hgfy); free(hgfx);
    free(hourgam7); free(hourgam6); free(hourgam5); free(hourgam4);
    free(hourgam3); free(hourgam2); free(hourgam1); free(hourgam0);
    free(zd1); free(yd1); free(xd1);
#endif //    HAB_C
  } // for i2
} // CalcFBHourglassForceForElems()

void CalcHourglassControlForElems_edt_1( Index_t i_out , Index_t i_end,
                                         Real_t *determ ,
                                         Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                         Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ,
                                         SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
#if  defined(HAB_C)
     Real_t  *x1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *y1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *z1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfx = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfy = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfz = (Real_t *)malloc(EIGHT*sizeof(Real_t));
#else // NOT HAB_C
     Real_t  x1[EIGHT],  y1[EIGHT],  z1[EIGHT] ;
     Real_t pfx[EIGHT], pfy[EIGHT], pfz[EIGHT] ;
#endif //    HAB_C

     SHARED Index_t* elemToNode = (SHARED Index_t *) &domain->m_nodelist[EIGHT*i];
// RAG -- GATHER (x|y|z)1[0..7] = domain->m_(x|y|z)[domain->m_nodelist[0..7]]

     CollectDomainNodesToElemNodes(elemToNode, x1, y1, z1, domain);

     CalcElemVolumeDerivative(pfx, pfy, pfz, x1, y1, z1);

//TRACE6("/* load into temporary storage for FB Hour Glass control */");
     for( Index_t ii=0 ; ii<EIGHT ; ++ii ){
       Index_t jj=EIGHT*i+ii;

       dvdx[jj] = pfx[ii];
       dvdy[jj] = pfy[ii];
       dvdz[jj] = pfz[ii];
       x8n[jj]  = x1[ii];
       y8n[jj]  = y1[ii];
       z8n[jj]  = z1[ii];
     } // for ii

     determ[i] = domain->m_volo[i] * domain->m_v[i];

//TRACE6("/* Do a check for negative volumes */");
     if ( domain->m_v[i] <= cast_Real_t(0.0) ) {
       EXIT(VolumeError) ;
     }
#if  defined(HAB_C)
     free(pfz); free(pfy); free(pfx);
     free(z1) ; free(y1) ; free(x1);
#endif //    HAB_C
  } // for i
} // CalcHourglassControlForElems_edt_1()


void CalcVolumeForceForElems_edt_1( Index_t i_out , Index_t i_end ,
                                    Real_t *determ ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
     if ( determ[i] <= cast_Real_t(0.0) ) {
       EXIT(VolumeError) ;
     }
  } // for i
} // CalcVolueForceForElems_edt_1()

void CalcForceForNodes_edt_1( Index_t i_out , Index_t i_end ,
                                    SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    domain->m_fx[i] = cast_Real_t(0.0) ;
    domain->m_fy[i] = cast_Real_t(0.0) ;
    domain->m_fz[i] = cast_Real_t(0.0) ;
  } // for i
} // CalcForceForNodes_edt_1()
