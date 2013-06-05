#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "RAG.h"
#include "MEM.h"
#include "AMO.h"

#if CILK
extern "C" {
#endif

#if defined(FSIM) || defined(OCR)

ocrLocation_t locate_in_dram = {
  .type = OCR_LOC_TYPE_RELATIVE,
  .data.relative.identifier = 0,
  .data.relative.level = OCR_LOCATION_DRAM,
};

ocrLocation_t locate_in_spad = {
  .type = OCR_LOC_TYPE_RELATIVE,
  .data.relative.identifier = 0,
  .data.relative.level = OCR_LOCATION_DRAM, // RAG HACK
};

void domain_create(struct DomainObject_t *domainObject, size_t edgeNodes, size_t edgeElems) { 
  uint16_t flags = 0;
  uint64_t retVal = 0;
  size_t sqrNodes = edgeNodes*edgeNodes;
  size_t numNodes = sqrNodes*edgeNodes;
  size_t numElems = edgeElems*edgeElems*edgeElems;
  size_t   size_in_bytes =      (ONE*sizeof(struct Domain_t))
  //////////////////////////////////////////////////
  ////  domain_AllocateNodalPersistent(numNodes) ////
  //////////////////////////////////////////////////
  /* domain->m_x           */ + (numNodes*sizeof(Real_t))
  /* domain->m_y           */ + (numNodes*sizeof(Real_t))
  /* domain->m_z           */ + (numNodes*sizeof(Real_t))
  /* domain->m_xd          */ + (numNodes*sizeof(Real_t))
  /* domain->m_yd          */ + (numNodes*sizeof(Real_t))
  /* domain->m_zd          */ + (numNodes*sizeof(Real_t))
  /* domain->m_xdd         */ + (numNodes*sizeof(Real_t))
  /* domain->m_ydd         */ + (numNodes*sizeof(Real_t))
  /* domain->m_zdd         */ + (numNodes*sizeof(Real_t))
  /* domain->m_fx          */ + (numNodes*sizeof(Real_t))
  /* domain->m_fy          */ + (numNodes*sizeof(Real_t))
  /* domain->m_fz          */ + (numNodes*sizeof(Real_t))
  /* domain->m_nodalMass   */ + (numNodes*sizeof(Real_t))
  //////////////////////////////////////////////////
  ////  domain_AllocateElemPersistent(numElems) ////
  //////////////////////////////////////////////////
  /* domain->m_matElemlist */ + (numElems*sizeof(Index_t))
  /* domain->m_nodelist    */ + (numElems*EIGHT*sizeof(Index_t))
  /* domain->m_lxim        */ + (numElems*sizeof(Index_t))
  /* domain->m_lxip        */ + (numElems*sizeof(Index_t))
  /* domain->m_letam       */ + (numElems*sizeof(Index_t))
  /* domain->m_letap       */ + (numElems*sizeof(Index_t))
  /* domain->m_lzetam      */ + (numElems*sizeof(Index_t))
  /* domain->m_lzetap      */ + (numElems*sizeof(Index_t))
  /* domain->m_elemBC      */ + (numElems*sizeof(Int_t))
  /* domain->m_e           */ + (numElems*sizeof(Real_t))
  /* domain->m_p           */ + (numElems*sizeof(Real_t))
  /* domain->m_q           */ + (numElems*sizeof(Real_t))
  /* domain->m_ql          */ + (numElems*sizeof(Real_t))
  /* domain->m_qq          */ + (numElems*sizeof(Real_t))
  /* domain->m_v           */ + (numElems*sizeof(Real_t))
  /* domain->m_volo        */ + (numElems*sizeof(Real_t))
  /* domain->m_delv        */ + (numElems*sizeof(Real_t))
  /* domain->m_vdov        */ + (numElems*sizeof(Real_t))
  /* domain->m_arealg      */ + (numElems*sizeof(Real_t))
  /* domain->m_ss          */ + (numElems*sizeof(Real_t))
  /* domain->m_elemMass    */ + (numElems*sizeof(Real_t))
  /////////////////////////////////////////////////
  ////  domain_AllocateElemTemporary(numElems) ////
  /////////////////////////////////////////////////
  /* domain->m_dxx         */ + (numElems*sizeof(Real_t))
  /* domain->m_dyy         */ + (numElems*sizeof(Real_t))
  /* domain->m_dzz         */ + (numElems*sizeof(Real_t))
  /* domain->m_delv_xi     */ + (numElems*sizeof(Real_t))
  /* domain->m_delv_eta    */ + (numElems*sizeof(Real_t))
  /* domain->m_delv_zeta   */ + (numElems*sizeof(Real_t))
  /* domain->m_delx_xi     */ + (numElems*sizeof(Real_t))
  /* domain->m_delx_eta    */ + (numElems*sizeof(Real_t))
  /* domain->m_delx_zeta   */ + (numElems*sizeof(Real_t))
  /* domain->m_vnew        */ + (numElems*sizeof(Real_t))
  /////////////////////////////////////////////////
  ////  domain_AllocateNodesets(edgeNodes*edgeNodes) ////
  /////////////////////////////////////////////////
  /* domain->m_symmX       */ + (sqrNodes*sizeof(Index_t))
  /* domain->m_symmY       */ + (sqrNodes*sizeof(Index_t))
  /* domain->m_symmZ       */ + (sqrNodes*sizeof(Index_t));

  retVal = ocrDbCreate(&(domainObject->guid),(uint64_t *)&(domainObject->base),flags, size_in_bytes, &locate_in_dram, NO_ALLOC);
//xe_printf("rag: ocrDbCreate retval = %16.16lx  guid =%16.16lx  base = %16.16lx\n",retVal,domainObject->guid,domainObject->base);
  if( retVal != 0 ) {
    xe_printf("RAG: domain ocrDbCreate error %d\n",retVal);
    EXIT(1);
  }
//xe_printf("rag: domain_create len = %16.16lx\n",size_in_bytes);
  domainObject->offset  = 0;
//xe_printf("rag: domain_create off = %16.16lx\n",domainObject->offset);
  domainObject->limit   = size_in_bytes;
//xe_printf("rag: domain_create lmt = %16.16lx\n",domainObject->limit);
} // dram_create()

void  domain_destroy(struct DomainObject_t *domainObject) { 
  uint64_t retVal = ocrDbDestroy(domainObject->guid);
  if( retVal != 0 ) {
    xe_printf("RAG: dram ocrDbCreate error %d\n",retVal);
    EXIT(1);
  }
} // dram_destroy()

void *dram_alloc(struct DomainObject_t *domainObject, size_t count, size_t sizeof_type) { 
  size_t len = count * sizeof_type;
  size_t off = AMO__sync_fetch_and_add_uint64_t((uint64_t *)&domainObject->offset,(uint64_t)len);
//xe_printf("rag: dram_alloc len  = %16.16lx\n",len);
//xe_printf("rag: dram_alloc off  = %16.16lx\n",off);
//xe_printf("rag: dram_alloc limit= %16.16lx\n",domainObject->limit);
  void *ptr = domainObject->base + off;
//xe_printf("rag: dram_alloc ptr  = %16.16lx\n",ptr);
  if( domainObject->offset < domainObject->limit ) {
    return ptr;
  } else {
    xe_printf("RAG: dram_alloc, domainObject limit overflow (%d >= %d)\n",off,domainObject->limit);
    EXIT(1);
    return NULL ; // IMPOSSIBLE //
  }
} // dram_alloc()

void  dram_free(void *ptr) {
  return;
} // dram_free()

SHARED ocrGuid_t spad_db_guid_stack[100];
SHARED uint64_t spad_db_guid_stack_top = 0; // emplty

void *spad_alloc(size_t count, size_t sizeof_type) { 
  uint16_t flags = 0;
  uint64_t retVal = 0;
  uint64_t *ptrValue;
  size_t len = count * sizeof_type;
  uint64_t spad_db_guid_stack_index = AMO__sync_fetch_and_add_uint64_t(&spad_db_guid_stack_top,(int64_t)1);
  if ( spad_db_guid_stack_index < 100 ) {
    ocrGuid_t *spad_db_stack_element = &spad_db_guid_stack[spad_db_guid_stack_index];
    retVal = ocrDbCreate(spad_db_stack_element,ptrValue,flags, len, &locate_in_spad, NO_ALLOC);
    if( retVal != 0 ) {
      xe_printf("RAG: spad ocrDbCreate error %d\n",retVal);
      EXIT(1);
    }
    return (void *)ptrValue;  
  } else {
    xe_printf("RAG: spad_alloc stack overflow\n");
    EXIT(1);
    return NULL; // IMPOSSIBLE //
  }
} // spad_alloc()

void  spad_free(void *ptr) {
  uint64_t retVal = 0;
  uint64_t spad_db_guid_stack_index = AMO__sync_fetch_and_add_uint64_t(&spad_db_guid_stack_top,(int64_t)-1);
  if ( 0 < spad_db_guid_stack_index ) {
    ocrGuid_t spad_db_stack_element = spad_db_guid_stack[spad_db_guid_stack_index-1];
    retVal = ocrDbDestroy(spad_db_stack_element);
    if( retVal != 0 ) {
      xe_printf("RAG: spad ocrDbDestroy error %d\n",retVal);
      EXIT(1);
     }
  } else {
    xe_printf("RAG: spad_free stack underflow\n");
    EXIT(1);
  }
  return;
} // spad_free()


#endif // FSIM or OCR

#if CILK
} // extern "C"
#endif
