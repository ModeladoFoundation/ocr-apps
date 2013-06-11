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
                                          struct Domain_t *domain) {
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
                                         struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
            domain->m_e[i] = (Real_t)(0.0);
            domain->m_p[i] = (Real_t)(0.0);
            domain->m_v[i] = (Real_t)(1.0);
  } // for i
} // domain_AllocateElemPersistent_edt_1()
 
void InitStressTermsForElems_edt_1(Index_t i_out, Index_t i_end, 
                                   Real_t *sigxx, Real_t *sigyy, Real_t *sigzz, 
                                   struct Domain_t *domain) {
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
