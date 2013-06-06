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

static INLINE uint64_t align_address(arg) {
 return(((arg)+(0x0000000000000007ll)) 
              &(0xFFFFFFFFFFFFFFF8ll));
} // align_address()

void domain_create(struct DomainObject_t *domainObject, size_t edgeNodes, size_t edgeElems) { 
  uint16_t flags  = 0;
  uint8_t  retVal = 0;
  size_t sqrNodes = edgeNodes*edgeNodes;
  size_t numNodes = sqrNodes*edgeNodes;
  size_t numElems = edgeElems*edgeElems*edgeElems;
  size_t   size_in_bytes =      0;
  //////////////////////
  // size of Domain_t //
  //////////////////////
  size_in_bytes +=              align_address(ONE*sizeof(struct Domain_t));
  //////////////////////////////////////////////////
  ////  domain_AllocateNodalPersistent(numNodes) ////
  //////////////////////////////////////////////////
  size_in_bytes +=            (
  /* domain->m_x           */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_y           */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_z           */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_xd          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_yd          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_zd          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_xdd         */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_ydd         */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_zdd         */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_fx          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_fy          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_fz          */ + align_address(numNodes*sizeof(Real_t))
  /* domain->m_nodalMass   */ + align_address(numNodes*sizeof(Real_t))
                              );
  //////////////////////////////////////////////////
  ////  domain_AllocateElemPersistent(numElems) ////
  //////////////////////////////////////////////////
  size_in_bytes +=            (
  /* domain->m_matElemlist */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_nodelist    */ + align_address(numElems*EIGHT*sizeof(Index_t))
  /* domain->m_lxim        */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_lxip        */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_letam       */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_letap       */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_lzetam      */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_lzetap      */ + align_address(numElems*sizeof(Index_t))
  /* domain->m_elemBC      */ + align_address(numElems*sizeof(Int_t))
  /* domain->m_e           */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_p           */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_q           */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_ql          */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_qq          */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_v           */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_volo        */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delv        */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_vdov        */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_arealg      */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_ss          */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_elemMass    */ + align_address(numElems*sizeof(Real_t))
                              );
  /////////////////////////////////////////////////
  ////  domain_AllocateElemTemporary(numElems) ////
  /////////////////////////////////////////////////
  size_in_bytes +=            (
  /* domain->m_dxx         */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_dyy         */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_dzz         */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delv_xi     */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delv_eta    */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delv_zeta   */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delx_xi     */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delx_eta    */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_delx_zeta   */ + align_address(numElems*sizeof(Real_t))
  /* domain->m_vnew        */ + align_address(numElems*sizeof(Real_t))
                              );
  /////////////////////////////////////////////////
  ////  domain_AllocateNodesets(edgeNodes*edgeNodes) ////
  /////////////////////////////////////////////////
  size_in_bytes +=            (
  /* domain->m_symmX       */ + align_address(sqrNodes*sizeof(Index_t))
  /* domain->m_symmY       */ + align_address(sqrNodes*sizeof(Index_t))
  /* domain->m_symmZ       */ + align_address(sqrNodes*sizeof(Index_t))
                              );

  retVal = ocrDbCreate(&(domainObject->guid),(uint64_t *)&(domainObject->base),size_in_bytes, flags, &locate_in_dram, NO_ALLOC);
//xe_printf("rag: ocrDbCreate retval = %16.16lx  GUID =%16.16lx  BASE = %16.16lx\n",(uint64_t)retVal,domainObject->guid.data,domainObject->base);
  if( retVal != 0 ) {
    xe_printf("RAG: domain ocrDbCreate error %d\n",(uint64_t)retVal);
    EXIT(1);
  }
//xe_printf("rag: domain_create LEN = %16.16lx\n",size_in_bytes);
  domainObject->offset  = 0;
//xe_printf("rag: domain_create OFF = %16.16lx\n",domainObject->offset);
  domainObject->limit   = size_in_bytes;
//xe_printf("rag: domain_create LMT = %16.16lx\n",domainObject->limit);
} // dram_create()

void  domain_destroy(struct DomainObject_t *domainObject) { 
  uint8_t retVal = ocrDbDestroy(domainObject->guid);
  if( retVal != 0 ) {
    xe_printf("RAG: dram ocrDbCreate error %d\n",(uint64_t)retVal);
    EXIT(1);
  }
} // dram_destroy()

void *dram_alloc(struct DomainObject_t *domainObject, size_t count, size_t sizeof_type) { 
  size_t len = align_address(count*sizeof_type);
  size_t off = AMO__sync_fetch_and_add_uint64_t((uint64_t *)&domainObject->offset,(uint64_t)len);
  void *ptr = (void *)(((uint8_t *)domainObject->base) + off);
#ifdef FSIM
//xe_printf("rag: dram_alloc len  = %16.16lx\n",len);
//xe_printf("rag: dram_alloc off  = %16.16lx\n",off);
//xe_printf("rag: dram_alloc lmt  = %16.16lx\n",domainObject->limit);
//xe_printf("rag: dram_alloc base = %16.16lx\n",(uint64_t)domainObject->base);
//xe_printf("rag: dram_alloc PTR  = %16.16lx\n",ptr);
#endif // FSIM
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

SHARED ocrGuid_t spad_dbGuid_stack[100];
SHARED uint64_t spad_dbGuid_stack_top = 0; // empty

void *spad_alloc(size_t len) { 
  uint16_t flags = 0;
  uint8_t retVal = 0;
  uint64_t *ptrValue = NULL;
  uint64_t spad_dbGuid_stack_index = AMO__sync_fetch_and_add_uint64_t(&spad_dbGuid_stack_top,(int64_t)1);
//xe_printf("rag: spad_alloc index = %16.16lx\n",spad_dbGuid_stack_index);
//xe_printf("rag: spad_alloc top   = %16.16lx\n",spad_dbGuid_stack_top);
  if ( spad_dbGuid_stack_index < 100 ) {
//  xe_printf("rag: spad_alloc  len   = %16.16lx\n",len);
//  xe_printf("rag: spad_alloc  flags = %16.16lx\n",(uint64_t)flags);
    retVal = ocrDbCreate(&spad_dbGuid_stack[spad_dbGuid_stack_index], (uint64_t *)&ptrValue, len, flags, &locate_in_spad, NO_ALLOC);
//  xe_printf("rag: ocrDbCreate retval = %16.16lx  guid =%16.16lx  base = %16.16lx\n",(uint64_t)retVal,spad_dbGuid_stack[spad_dbGuid_stack_index].data,ptrValue);
    if( retVal != 0 ) {
      xe_printf("RAG: spad ocrDbCreate error %d\n",(uint64_t)retVal);
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
  uint64_t spad_dbGuid_stack_index = AMO__sync_fetch_and_add_uint64_t(&spad_dbGuid_stack_top,(int64_t)-1);
//xe_printf("rag: spad_free index = %16.16lx\n",(spad_dbGuid_stack_index-1));
//xe_printf("rag: spad_free  dbGuid   = %16.16lx\n",spad_dbGuid_stack[spad_dbGuid_stack_index-1].data);
  if ( 0 < spad_dbGuid_stack_index ) {
    retVal = ocrDbDestroy(spad_dbGuid_stack[spad_dbGuid_stack_index-1]);
    if( retVal != 0 ) { 
      xe_printf("RAG: spad ocrDbDestroy error %d\n",(uint64_t)retVal);
      EXIT(1);
    } else {
      spad_dbGuid_stack[spad_dbGuid_stack_index-1].data = (uint64_t)NULL;
      return;
    }
  } else {
    xe_printf("RAG: spad_free stack underflow\n");
    EXIT(1);
  }
  return; // IMPOSSIBLE
} // spad_free()


#endif // FSIM or OCR

#if CILK
} // extern "C"
#endif
