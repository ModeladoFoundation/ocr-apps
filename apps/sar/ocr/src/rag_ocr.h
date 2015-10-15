#ifndef RAG_OCR_H
#define RAG_OCR_H

//
// Roger A Golliver -- A collection of things for helping the port to Open Community Runtime
//

#ifndef TG_ARCH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "crlibm.h"

#define GUID_VALUE(guid) ((uint64_t)(guid))

// Used to be rmdglobal for TG but is not necessary anymore, as we rely on the allocator
#define SHARED

#ifdef TG_ARCH
#define RAG_FLUSH
#define rag_printf(...)
#define rag_flush
#else
#define RAG_FLUSH {fflush(stdout);fflush(stderr);}
#define rag_printf(...)
#define rag_flush
#endif

#ifndef TG_ARCH
void ocr_exit(void); // RAG ocr_exit() non-public
#define xe_exit(a)  {if(a)PRINTF("\nxe_exit(%d)\n",a); RAG_FLUSH; ocrShutdown(); if(a)ocr_exit(); } // RAG ocr_exit() non-public
#else
#define xe_exit(a)  {if(a){ PRINTF("\nxe_exit(%d)\n",a); RAG_FLUSH; } ocrShutdown(); }
#endif

#ifdef TG_ARCH
#define OCR_DB_CREATE(guid,addr,len,affinity) \
do { \
	u8 __retval__; \
	__retval__ = ocrDbCreate(&(guid),(void *)&(addr),(len),DB_PROP_NONE,(affinity),NO_ALLOC); \
	rag_printf("OCR_DB_C %16.16lx \n",GUID_VALUE(guid));rag_flush; \
	if (__retval__ != 0) { \
		PRINTF("ocrDbCreate ERROR ret_val=%d\n", __retval__); RAG_FLUSH; xe_exit(__retval__); \
	} /* else { PRINTF("ocrDbDreate OKAY\n"); RAG_FLUSH; } */ \
} while(0)
#else
#define OCR_DB_CREATE(guid,addr,len,affinity) \
do { \
	u8 __retval__; \
	__retval__ = ocrDbCreate(&(guid),&(addr),(len),DB_PROP_NONE,(affinity),NO_ALLOC); \
	rag_printf("OCR_DB_C %16.16lx \n",GUID_VALUE(guid));rag_flush; \
	if (__retval__ != 0) { \
		PRINTF("ocrDbCreate ERROR ret_val=%d\n", __retval__); RAG_FLUSH; xe_exit(__retval__); \
	} /* else { PRINTF("ocrDbDreate OKAY\n"); RAG_FLUSH; } */ \
} while(0)
#endif

#define OCR_DB_RELEASE(guid) \
do { \
	u8  __retval__; \
	rag_printf("OCR_DB_R %16.16lx \n",GUID_VALUE(guid));rag_flush; \
	__retval__ = ocrDbRelease(guid); \
	if (__retval__ != 0) { \
		PRINTF("ocrDbRelease ERROR arg=%ld %s:%d\n", GUID_VALUE(guid), __FILE__, __LINE__); \
		xe_exit(__retval__); \
	} \
} while(0)

#if 0
#define OCR_DB_FREE(addr,guid) \
	do { rag_printf("OCR_DB_D %16.16lx \n",GUID_VALUE(guid));rag_flush; } while (0)
#else
#define OCR_DB_FREE(addr,guid) \
do { \
	u8 __retval__; \
	rag_printf("OCR_DB_D %16.16lx \n",GUID_VALUE(guid));rag_flush; \
	__retval__ = ocrDbDestroy((guid)); \
	if (__retval__ != 0) { \
		PRINTF("ocrDbDestroy ERROR arg=%ld %s:%d\n", GUID_VALUE(guid), __FILE__, __LINE__); \
		xe_exit(__retval__); \
	} \
} while(0)
#endif

#ifndef TG_ARCH
  #include <assert.h>
#else
  #ifdef assert
    #undef assert
  #endif
  #ifdef NDEBUG
    #define assert(x)
  #else
    #define assert(x) if(!(x)) {PRINTF("ASSERT () failed %s:%d\n",__FILE__, __LINE__); xe_exit(x); }
  #endif
#endif

void RAG_MEMCPY(void *out, SHARED void *in, size_t size);
void rag_memcpy(void *out, void *in, size_t size);

#ifndef TG_ARCH
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
void SPADtoBSM( void *out, void *in, size_t size);
void SPADtoDRAM(void *out, void *in, size_t size);

void BSMtoSPAD(void *out, void *in, size_t size);
void BSMtoBSM( void *out, void *in, size_t size);
void BSMtoDRAM(void *out, void *in, size_t size);

void DRAMtoSPAD(void *out, void *in, size_t size);
void DRAMtoBSM( void *out, void *in, size_t size);
void DRAMtoDRAM(void *out, void *in, size_t size);

#define RAG_DEF_MACRO_SPAD(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_RW); assert(retval==0); \
	rag_printf("DEF_SPAD %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#define RAG_DEF_MACRO_BSM(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_RW); assert(retval==0); \
	rag_printf("DEF_BSM  %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#define RAG_DEF_MACRO_PASS(scg,no_type,no_var,no_ptr,no_lcl,dbg,slot) \
 	retval = ocrAddDependence(dbg,scg,slot,DB_MODE_RW); assert(retval==0); \
	rag_printf("DEF_PASS %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#define RAG_REF_MACRO_SPAD(type,var,ptr_var,lcl_var,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid; \
	type *var, *ptr_var= (void *)depv[slot].ptr, lcl_var; \
	REM_LDX_ADDR(lcl_var, ptr_var, type); \
	var = &lcl_var; \
	rag_printf("REF_SPAD %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#define RAG_REF_MACRO_BSM(type,var,no_ptr,no_lcl,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid; \
	type var = (void *)depv[slot].ptr; \
	rag_printf("REF_BSM  %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#define RAG_REF_MACRO_PASS(no_type,no_var,no_ptr,no_lcl,dbg,slot) \
	ocrGuid_t dbg = depv[slot].guid; \
	rag_printf("REF_PASS %16.16lx \n",GUID_VALUE(dbg));rag_flush;

#ifndef RAG_NEW_BLK_SIZE
static int blk_size(int n,int max_blk_size) {
	int ret_val = n;
	for( int i = max_blk_size ; i>1 ; i-- ) {
		if( (n%i) == 0 ) {
			ret_val = i;
			break;
		}
	}
#ifdef TRACE
	PRINTF("blk_size(%d,%d) returns = %d\n",n,max_blk_size,ret_val);
#endif
	return ret_val;
}
#endif

#ifdef RAG_CRLIBM
//
// was in math.h, using crlibm and rag_ocr
//
extern float cosf(float arg);
extern float sinf(float arg);
extern void  sincosf(float arg, float *si, float *rc);
#ifdef TG_ARCH
#if 0
inline float  __attribute__((always_inline)) sqrtf(float x) {
    __asm__ (
        "rcpsqrtF32 %0, %0\n\t"
        : "=r" (x)
        : "r" (x)
        :
    );
    return x;
}
#else
extern float __ieee754_sqrtf(float arg);
#define sqrtf(arg) __ieee754_sqrtf((float)arg)
#endif
#else
extern float sqrtf(float arg);
#endif

extern float fabsf(float arg);
extern float fmodf(float x, float y);
extern float floorf(float arg );
extern float ceilf(float arg);

extern double cos(double arg);
extern double sin(double arg);
extern void   sincos(double arg, double *si, double *rc);
#ifdef TG_ARCH
#if 0
inline double  __attribute__((always_inline)) sqrt(double x) {
    __asm__ (
        "sqrtF64 %0, %0\n\t"
        : "=r" (x)
        : "r" (x)
        :
    );
    return x;
}
#else
extern double __ieee754_sqrt(double arg);
#define sqrt(arg) __ieee754_sqrt((double)arg)
#endif
#else
extern double sqrt(double arg);
#endif

extern double round(double arg);
#else
#include <math.h>
#endif

extern SHARED ocrGuid_t templateList[25];
extern SHARED int templateIndex;

#endif // RAG_OCR_H
