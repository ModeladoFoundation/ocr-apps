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

#include "RAG_edt.h"

void domain_AllocateNodalPersistent_edt_1(Index_t i_out, Index_t i_end, struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
      domain->m_xd[i] = (Real_t)(0.0);
      domain->m_yd[i] = (Real_t)(0.0);
      domain->m_zd[i] = (Real_t)(0.0);

      domain->m_xdd[i] = (Real_t)(0.0);
      domain->m_ydd[i] = (Real_t)(0.0);
      domain->m_zdd[i] = (Real_t)(0.0) ;

      domain->m_nodalMass[i] = (Real_t)(0.0) ;
  } // for i
}

domain_AllocateElemPersistent_edt_1(Index_t i_out, Index_t i_end, struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
            domain->m_e[i] = (Real_t)(0.0);
            domain->m_p[i] = (Real_t)(0.0);
            domain->m_v[i] = (Real_t)(1.0);
  } // for i
}
