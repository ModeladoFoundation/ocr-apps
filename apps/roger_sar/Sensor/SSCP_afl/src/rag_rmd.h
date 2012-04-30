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
#define RAG_FLUSH
#endif

#ifdef RAG_AFL
#include "codelet.h"
#include "rmd_afl_all.h"

#define xe_printf printf
#define RAG_FLUSH {fflush(stdout);fflush(stderr);}
#endif

#if defined(RAG_AFL) || defined(RAG_SIM)
#define xe_exit(a) {rmd_complete();return NULL_GUID;}

#ifndef RMD_DB_MEM
#define RMD_DB_MEM(x,y) \
do { \
	u64 __retval__; \
	int __err__; \
	__retval__ = rmd_db_acquire((y),(0)); \
	__err__ = GET_STATUS(__retval__);        \
	if (__err__ != 0) { \
		xe_printf("db_acquire ERROR arg=%ld (%s) %s:%d\n", (uint64_t)y.data, strerror(__err__), __FILE__, __LINE__); \
		exit(__err__); \
	} \
	*(x) = (void*)GET_ADDRESS(__retval__);  \
} while(0)
#endif

static rmd_guid_t NULL_GUID = { .data = 0 };
#endif // RAG_AFL || RAG_SIM

#ifdef RAG_SIM
#ifdef assert
#undef assert
#endif
#ifdef NDEBUG
#define assert(x)
#else
#define assert(x) if(!(x)) {xe_printf("ASSERT () failed %s:%d\n",__FILE__, __LINE__); exit(x); }
#endif
#endif

#ifdef RAG_AFL
#include <assert.h>
#endif

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

#if defined(RAG_AFL) || defined(RAG_SIM)
int32_t  RAG_GET_INT(int32_t    *addr);
void     RAG_PUT_INT(int32_t    *addr, int32_t  value);

uint32_t RAG_GET_UINT(uint32_t  *addr);
void     RAG_PUT_UINT(uint32_t  *addr, uint32_t value);

int64_t  RAG_GET_LONG(int64_t   *addr);
void     RAG_PUT_LONG(int64_t   *addr, int64_t  value);

uint64_t RAG_GET_ULONG(uint64_t *addr);
void     RAG_PUT_ULONG(uint64_t *addr, uint64_t value);

float    RAG_GET_FLT(float      *addr);
void     RAG_PUT_FLT(float      *addr, float    value);

double   RAG_GET_DBL(double     *addr);
void     RAG_PUT_DBL(double     *addr, double   value);

void *   RAG_GET_PTR(void       *addr);
void     RAG_PUT_PTR(void       *addr, void *   value);

// _clg => codelet guid
// _scg => scheduled codelet guid
// _dbg => data block guid
// _dbp => data block pointer
// _lcl => data on stack

void *spad_calloc(rmd_guid_t *dbg, size_t n, size_t size);
void *spad_malloc(rmd_guid_t *dbg, size_t size);
void  spad_free(void *dbp, rmd_guid_t dbg );
void  spad_memset(void *out, int val, size_t size);

void *bsm_calloc(rmd_guid_t *dbg, size_t n, size_t size);
void *bsm_malloc(rmd_guid_t *dbg, size_t size);
void  bsm_free(void *dbp, rmd_guid_t dbg );
void  bsm_memset(void *out, int val, size_t size);

void *dram_calloc(rmd_guid_t *dbg, size_t n, size_t size);
void *dram_malloc(rmd_guid_t *dbg, size_t size);
void  dramblk_free(void *dbp, rmd_guid_t dbg );
void  dram_memset(void *out, int val, size_t size);

void SPADtoBSM(void *out, void *in, size_t size);
void BSMtoBSM(void *out, void *in, size_t size);

#define RAG_DEF_MACRO_SPAD(scg,type,var,ptr,lcl,dbg,slot) \
 	retval = rmd_codelet_satisfy(scg,dbg,slot); assert(retval==0);

#define RAG_DEF_MACRO_BSM(scg,type,var,ptr,lcl,dbg,slot) \
 	retval = rmd_codelet_satisfy(scg,dbg,slot); assert(retval==0);

#define RAG_DEF_MACRO_PASS(scg,type,var,ptr,lcl,dbg,slot) \
 	retval = rmd_codelet_satisfy(scg,dbg,slot); assert(retval==0);

#define RAG_REF_MACRO_SPAD(type,var,ptr,lcl,dbg,slot) \
	rmd_guid_t dbg = db[slot]; \
	type *var, *ptr=db_ptr[slot], lcl; \
	REM_LDX_ADDR(lcl, ptr, type); \
	var = &lcl;

#define RAG_REF_MACRO_BSM(type,var,ptr,lcl,dbg,slot) \
	rmd_guid_t dbg = db[slot]; \
	type var = db_ptr[slot];

#define RAG_REF_MACRO_PASS(type,var,ptr,lcl,dbg,slot) \
	rmd_guid_t dbg = db[slot];

#endif // RAG_AFL || RAG_SIM
#endif
