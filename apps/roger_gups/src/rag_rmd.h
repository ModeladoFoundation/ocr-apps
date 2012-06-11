#ifndef RAG_RMD_H
#define RAG_RMD_H
//
//
// Roger A Golliver -- A collection of things for helping the port to codelets
//

#include <string.h>

#ifdef RAG_SIM
#include "xe-codelet.h"
#include "xe_memory.h"
#include "xe_console.h"
#include "xe_global.h"
#include "xe_ase.h"
#endif

#ifdef RAG_AFL
#include "codelet.h"
#include "rmd_afl_all.h"
#define xe_printf printf
#define rmdglobal 
#define rmdblock 
#endif

#if defined(RAG_AFL) || defined(RAG_SIM)
static rmd_guid_t NULL_GUID = { .data = 0 };

static inline rmd_guid_t xe_exit(int a) {rmd_complete();return NULL_GUID;}

#ifndef RMD_DB_MEM
#define RMD_DB_MEM(x,y) \
do { \
	u64 __retval__; \
	int __err__; \
	__retval__ = rmd_db_acquire((y),(0)); \
	__err__ = GET_STATUS(__retval__);        \
	if (__err__ != 0) { \
		xe_printf("db_acquire ERROR arg=%ld (%s) %s:%d\n", (uint64_t)y.data, strerror(__err__), __FILE__, __LINE__); \
		xe_exit(__err__); \
	} \
	*(x) = (void*)GET_ADDRESS(__retval__);  \
} while(0)
#endif

#endif // RAG_AFL || RAG_SIM

#ifdef RAG_AFL
#define REM_LDX_ADDR(out_var,in_ptr,type) memcpy(&out_var,in_ptr,sizeof(type))
#define REM_STX_ADDR(out_ptr,in_var,type) memcpy(out_ptr,&in_var,sizeof(type))
#define REM_LDX_ADDR_SIZE(out_var,in_ptr,size) memcpy(&out_var,in_ptr,size)
#define REM_STX_ADDR_SIZE(out_ptr,in_var,size) memcpy(out_ptr,&in_var,size)

#define REM_LD8_ADDR(out_var,in_ptr) memcpy(&out_var,in_ptr,1*sizeof(char))
#define REM_ST8_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,1*sizeof(char))

#define REM_LD16_ADDR(out_var,in_ptr) memcpy(&out_var,in_ptr,2*sizeof(char))
#define REM_ST16_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,2*sizeof(char))

#define REM_LD32_ADDR(out_var,in_ptr) memcpy(&out_var,in_ptr,4*sizeof(char))
#define REM_ST32_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,4*sizeof(char))

#define REM_LD64_ADDR(out_var,in_ptr) memcpy(&out_var,in_ptr,8*sizeof(char))
#define REM_ST64_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,8*sizeof(char))
#endif

// _clg => codelet guid
// _scg => scheduled codelet guid
// _dbg => data block guid
// _dbp => data block pointer
// _lcl => data on stack

void *rag_spad_malloc(rmd_guid_t *dbg, size_t size);
void  rag_spad_free(void *dbp, rmd_guid_t dbg );

void *rag_bsm_malloc(rmd_guid_t *dbg, size_t size);
void  rag_bsm_free(void *dbp, rmd_guid_t dbg );

void *rag_dram_malloc(rmd_guid_t *dbg, size_t size);
void  rag_dram_free(void *dbp, rmd_guid_t dbg );

void rag_DRAMtoSPAD(void *out, void rmdglobal * restrict in, size_t size);
#endif
