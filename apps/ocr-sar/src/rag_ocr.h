#ifndef RAG_OCR_H
#define RAG_OCR_H

//
// Roger A Golliver -- A collection of things for helping the port to Open Community Runtime
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "crlibm.h"

#ifndef RAG_SIM
#define GUID_VALUE(guid) ((uint64_t)(guid))
#define SHARED
#else
#define GUID_VALUE(guid) ((uint64_t)(guid.data))
#define SHARED rmdglobal
#endif

#ifndef RAG_SIM
#define xe_printf printf
#define RAG_FLUSH {fflush(stdout);fflush(stderr);}
#else
#define RAG_FLUSH
#endif

#ifndef RAG_SIM
void ocr_exit(void); // RAG ocr_exit() non-public
#define xe_exit(a)  {xe_printf("\nxe_exit(%d)\n",a); RAG_FLUSH; ocrShutdown(); if(a)ocr_exit(); } // RAG ocr_exit() non-public
#else
#define xe_exit(a)  {xe_printf("\nxe_exit(%d)\n",a); RAG_FLUSH; ocrShutdown(); }
#endif

#ifdef RAG_SIM
#define OCR_DB_CREATE(guid,addr,len,affinity) \
do { \
	u8 __retval__; \
	__retval__ = ocrDbCreate(&(guid),(u64 *)&(addr),(len),DB_PROP_NONE,(affinity),NO_ALLOC); \
	if (__retval__ != 0) { \
		xe_printf("ocrDbCreate ERROR ret_val=%d\n", __retval__); RAG_FLUSH; xe_exit(__retval__); \
	} /* else { xe_printf("ocrDbDreate OKAY\n"); RAG_FLUSH; } */ \
} while(0)
#else
#define OCR_DB_CREATE(guid,addr,len,affinity) \
do { \
	u8 __retval__; \
	__retval__ = ocrDbCreate(&(guid),&(addr),(len),DB_PROP_NONE,(affinity),NO_ALLOC); \
	if (__retval__ != 0) { \
		xe_printf("ocrDbCreate ERROR ret_val=%d\n", __retval__); RAG_FLUSH; xe_exit(__retval__); \
	} /* else { xe_printf("ocrDbDreate OKAY\n"); RAG_FLUSH; } */ \
} while(0)
#endif

#define OCR_DB_RELEASE(guid) \
do { \
	u8  __retval__; \
	__retval__ = 0; /*ocrDbRelease(guid);*/ \
	if (__retval__ != 0) { \
		xe_printf("ocrDbRelease ERROR arg=%ld %s:%d\n", GUID_VALUE(guid), __FILE__, __LINE__); \
		xe_exit(__retval__); \
	} \
} while(0)

#define OCR_DB_FREE(addr,guid) \
do { \
	u8 __retval__; \
	__retval__ = ocrDbDestroy((guid)); \
	if (__retval__ != 0) { \
		xe_printf("ocrDbDestroy ERROR arg=%ld %s:%d\n", GUID_VALUE(guid), __FILE__, __LINE__); \
		xe_exit(__retval__); \
	} \
} while(0)

#ifndef RAG_SIM
  #include <assert.h>
#else
  #ifdef assert
    #undef assert
  #endif
  #ifdef NDEBUG
    #define assert(x)
  #else
    #define assert(x) if(!(x)) {xe_printf("ASSERT () failed %s:%d\n",__FILE__, __LINE__); xe_exit(x); }
  #endif
#endif

void RAG_MEMCPY(void *out, SHARED void *in, size_t size);
void rag_memcpy(void *out, void *in, size_t size);

#ifndef RAG_SIM
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
#else
#undef REM_LDX_ADDR
#undef REM_STX_ADDR
#undef REM_LDX_ADDR_SIZE
#undef REM_STX_ADDR_SIZE

#undef REM_LD8_ADDR
#undef REM_ST8_ADDR

#undef REM_LD16_ADDR
#undef REM_ST16_ADDR

#undef REM_LD32_ADDR
#undef REM_ST32_ADDR

#undef REM_LD64_ADDR
#undef REM_ST64_ADDR

#define REM_LDX_ADDR(out_var,in_ptr,type) rag_memcpy(&out_var,in_ptr,sizeof(type))
#define REM_STX_ADDR(out_ptr,in_var,type) rag_memcpy(out_ptr,&in_var,sizeof(type))
#define REM_LDX_ADDR_SIZE(out_var,in_ptr,size) rag_memcpy(&out_var,in_ptr,size)
#define REM_STX_ADDR_SIZE(out_ptr,in_var,size) rag_memcpy(out_ptr,&in_var,size)

#define REM_LD8_ADDR(out_var,in_ptr) rag_memcpy(&out_var,in_ptr,1*sizeof(char))
#define REM_ST8_ADDR(out_ptr,in_var) rag_memcpy(out_ptr,&in_var,1*sizeof(char))

#define REM_LD16_ADDR(out_var,in_ptr) rag_memcpy(&out_var,in_ptr,2*sizeof(char))
#define REM_ST16_ADDR(out_ptr,in_var) rag_memcpy(out_ptr,&in_var,2*sizeof(char))

#define REM_LD32_ADDR(out_var,in_ptr) rag_memcpy(&out_var,in_ptr,4*sizeof(char))
#define REM_ST32_ADDR(out_ptr,in_var) rag_memcpy(out_ptr,&in_var,4*sizeof(char))

#define REM_LD64_ADDR(out_var,in_ptr) rag_memcpy(&out_var,in_ptr,8*sizeof(char))
#define REM_ST64_ADDR(out_ptr,in_var) rag_memcpy(out_ptr,&in_var,8*sizeof(char))
#endif

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

// _clg => edt template guid
// _scg => edt task guid
// _dbg => data block guid
// _dbp => data block pointer
// _lcl => data on stack

void *spad_calloc(ocrGuid_t *dbg, size_t n, size_t size);
void *spad_malloc(ocrGuid_t *dbg, size_t size);
void  spad_free(void *dbp, ocrGuid_t dbg );
void  spad_memset(void *out, int val, size_t size);

void *bsm_calloc(ocrGuid_t *dbg, size_t n, size_t size);
void *bsm_malloc(ocrGuid_t *dbg, size_t size);
void  bsm_free(void *dbp, ocrGuid_t dbg );
void  bsm_memset(void *out, int val, size_t size);

void *dram_calloc(ocrGuid_t *dbg, size_t n, size_t size);
void *dram_malloc(ocrGuid_t *dbg, size_t size);
void  dram_free(void *dbp, ocrGuid_t dbg );
void  dram_memset(void *out, int val, size_t size);

void GlobalPtrToDataBlock(void *out, SHARED void *in, size_t size);
void SPADtoSPAD(void *out, void *in, size_t size);
void SPADtoBSM(void *out, void *in, size_t size);
void BSMtoSPAD(void *out, void *in, size_t size);
void BSMtoBSM(void *out, void *in, size_t size);
void SPADtoDRAM(void *out, void *in, size_t size);
void DRAMtoSPAD(void *out, void *in, size_t size);

#define RAG_DEF_MACRO_SPAD(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_ITW); assert(retval==0);

#define RAG_DEF_MACRO_BSM(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_ITW); assert(retval==0);

#define RAG_DEF_MACRO_PASS(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_ITW); assert(retval==0);

#define RAG_REF_MACRO_SPAD(type,var,ptr_var,lcl_var,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid; \
	type *var, *ptr_var= (void *)depv[slot].ptr, lcl_var; \
	REM_LDX_ADDR(lcl_var, ptr_var, type); \
	var = &lcl_var;

#define RAG_REF_MACRO_BSM(type,var,no_ptr,no_lcl,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid; \
	type var = (void *)depv[slot].ptr;

#define RAG_REF_MACRO_PASS(no_type,no_var,no_ptr,no_lcl,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid;

static int blk_size(int n,int max_blk_size) {
	int ret_val = n;
	for( int i = max_blk_size ; i>1 ; i-- ) {
		if( (n%i) == 0 ) {
			ret_val = i;
			break;
		}
	}
#ifdef DEBUG
	xe_printf("blk_size(%d,%d) returns = %d\n",n,max_blk_size,ret_val);
#endif
	return ret_val;
}

//
// was in math.h, using crlibm and rag_ocr
//
extern float fabsf(float arg);
extern double sqrt_rn(double arg);
extern float sqrtf_rn(float arg);
extern float sinf(float arg);
extern float cosf(float arg);
extern void  sincosf(float arg, float *si, float *rc);
extern float fmodf(float x, float y);
extern float floorf(float arg );
extern float ceilf(float arg);
extern double round(double arg);
#endif // RAG_OCR_H
