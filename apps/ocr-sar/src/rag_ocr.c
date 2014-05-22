#ifndef RAG_SIM
#include "ocr.h"
#include "rag_ocr.h"
#else
#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"
#include "rag_ocr.h"
#endif

void RAG_MEMCPY(void *out, SHARED void *in, size_t size) {
	char *__out = (char *)out;
	char *__in  = (char *)in;
	for(int i=0;i<size;i++) {
		*__out++ = *__in++;
	} // for i (size)
}

void rag_memcpy(void *out, void *in, size_t size) {
	char *__out = (char *)out;
	char *__in  = (char *)in;
	for(int i=0;i<size;i++) {
		*__out++ = *__in++;
	} // for i (size)
}

#ifndef RAG_SIM
static ocrGuid_t locate_in_spad = NULL_GUID;
static ocrGuid_t locate_in_bsm  = NULL_GUID;
static ocrGuid_t locate_in_dram = NULL_GUID;
#else
static ocrGuid_t locate_in_spad = NULL_GUID;
static ocrGuid_t locate_in_bsm  = NULL_GUID;
static ocrGuid_t locate_in_dram = NULL_GUID;

static ocrLocation_t locate_in_spad_data = {
  .type = OCR_LOC_TYPE_RELATIVE,
  .data.relative.identifier = 0,
  .data.relative.level = OCR_LOCATION_DRAM, // hack should be CORE
};

static ocrLocation_t locate_in_bsm_data = {
  .type = OCR_LOC_TYPE_RELATIVE,
  .data.relative.identifier = 0,
  .data.relative.level = OCR_LOCATION_DRAM, // hach should be BLOCK
};
static ocrLocation_t locate_in_dram_data = {
  .type = OCR_LOC_TYPE_RELATIVE,
  .data.relative.identifier = 0,
  .data.relative.level = OCR_LOCATION_DRAM,
};

#endif

void *spad_calloc(ocrGuid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_spad,(u64)&locate_in_spad_data);
#endif
#ifdef DEBUG
	xe_printf("SPAD_CALLOC n %ld size %ld (ptr=%16.16lx)\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_spad);
#ifdef DEBUG
	xe_printf("SPAD_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	spad_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *spad_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef DEBUG
	xe_printf("SPAD_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_spad,(u64)&locate_in_spad_data);
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_spad);
#ifdef DEBUG
	xe_printf("SPAD_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void spad_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	xe_printf("SPAD_FREE (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)dbp,GUID_VALUE(dbg));RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void *bsm_calloc(ocrGuid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_bsm,(u64)&locate_in_bsm_data);
#endif
#ifdef DEBUG
	xe_printf("BSM_CALLOC n %ld size %ld\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_bsm);
#ifdef DEBUG
	xe_printf("BSM_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	bsm_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *bsm_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp = NULL;
	ocrGuid_t tmp_dbg = NULL_GUID;
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_bsm,(u64)&locate_in_bsm_data);
#endif
#ifdef DEBUG
	xe_printf("BSM_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_bsm);
#ifdef DEBUG
	xe_printf("BSM_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void bsm_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	xe_printf("BSM_FREE (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)dbp,GUID_VALUE(dbg));RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void spad_memset(void *out, int val, size_t size) {
	char *__out = (char *)out;
	const char tmp = val;
	for(int i=0;i<size;i++) {
		*__out++ = tmp;
	} // for i (size)
}

void bsm_memset(void *out, int val, size_t size) {
	char *__out = (char *)out;
	const char tmp = val;
	for(int i=0;i<size;i++) {
		*__out++ = tmp;
//		REM_ST8_ADDR(__out++,tmp);
	} // for i (size)
}

void *dram_calloc(ocrGuid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_dram,(u64)&locate_in_dram_data);
#endif
#ifdef DEBUG
	xe_printf("DRAM_CALLOC n %ld size %ld\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_dram);
#ifdef DEBUG
	xe_printf("DRAM_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	dram_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *dram_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef RAG_SIM
        ocrGuidSetAddressU64(&locate_in_dram,(u64)&locate_in_dram_data);
#endif
#ifdef DEBUG
	xe_printf("DRAM_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_dram);
#ifdef DEBUG
	xe_printf("DRAM_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void dram_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	xe_printf("DRAM_FREE (dbg=%16.16lx)\n",GUID_VALUE(dbg));RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void dram_memset(void *out, int val, size_t size) {
	char *__out = (char *)out;
	const char tmp = val;
	for(int i=0;i<size;i++) {
		*__out++ = tmp;
//		REM_ST8_ADDR(__out++,tmp);
	} // for i (size)
}

void GlobalPtrToDataBlock(void *out, SHARED void *in, size_t size) {
#ifdef DEBUG
	xe_printf("GblPtrToDB OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	RAG_MEMCPY(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("SPADtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("SPADtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("SPADtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("BSMtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("BSMtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("BSMtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("DRAMtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("DRAMtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("DRAMtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

float fabsf(float arg) {
	union {
		float f;
		uint32_t ui;
	} x;
	x.f = arg;
	if(arg==arg) // if a number
		x.ui &= ~0x80000000;
	return x.f;
}

double sqrt_rn(double arg) {
	extern double __ieee754_sqrt(double arg);
	double ret_val = __ieee754_sqrt(arg);
#ifdef DEBUG_LVL_5
#ifdef RAG_SIM
	xe_printf("//////// sqrt_rn(%16.16lx) == %16.16lx\n",*(uint64_t *)&arg,*(uint64_t *)&ret_val);RAG_FLUSH;
#else
	xe_printf("//////// sqrt_rn(%16.16lx) == %16.16lx\n",*(uint64_t *)&arg,*(uint64_t *)&ret_val);RAG_FLUSH;
	// printf("//////// sqrt_rn(%f) == %f\n",arg,ret_val);RAG_FLUSH;
#endif
#endif
	return ret_val;
}

float sqrtf_rn(float arg) {
	extern float __ieee754_sqrtf(float arg);
	float ret_val = __ieee754_sqrtf(arg);
#ifdef DEBUG_LVL_5
#ifdef RAG_SIM
	xe_printf("//////// sqrtf_rn(%8.8x) == %8.8x\n",*(uint32_t *)&arg,*(uint32_t *)&ret_val);RAG_FLUSH;
#else
	xe_printf("//////// sqrtf_rn(%8.8x) == %8.8x\n",*(uint32_t *)&arg,*(uint32_t *)&ret_val);RAG_FLUSH;
	// printf("//////// sqrtf_rn(%f) == %f\n",arg,ret_val);RAG_FLUSH;
#endif
#endif
	return ret_val;
}

float  sinf(float arg) {
	return (float)sin_rn((double)arg);
}
float  cosf(float arg) {
	return (float)cos_rn((double)arg);
}
void  sincosf(float arg, float *si, float *rc) {
	*si = sinf(arg);
	*rc = cosf(arg);
	return;
}

float fmodf(float x, float y) {
	int n = x/y;
	return x-n*y;
}

float floorf(float arg ) {
	float ret_val;
	if(arg==0)return arg;					// +0 or -0
	if(arg!=arg)return arg;					// not a number
	if(arg==((arg+16777216.0f)-16777216.0f)) return arg;	// integral or inf
	ret_val = ((arg+16777216.0f)-16777216.0f);		// finite non-integral non-zero
	if(ret_val <= arg )return ret_val;			// okay, rounding was down
	if((ret_val-1.0f) <= arg) return ret_val - 1.0f;	// fix up, rounding was up
#if defined(DEBUG) && !defined(RAG_SIM)
	xe_printf("floorf ERROR %f %f\n",arg,(arg+16777216.0f));RAG_FLUSH;
	xe_printf("floorf ERROR %f %f\n",arg,((arg+16777216.0f)-16777216.0f));RAG_FLUSH;
	xe_printf("floorf ERROR %f %f\n",arg,ret_val);RAG_FLUSH;
	xe_exit(1);
#endif
	return 0.0f/0.0f;
}

float ceilf(float arg) {
	float ret_val;
	if(arg==0)return arg;					// +0 or -0
	if(arg!=arg)return arg;					// not a number
	if(arg==((arg+16777216.0f)-16777216.0f)) return arg;	// integral or inf
	ret_val = ((arg+16777216.0f)-16777216.0f);		// finite non-integral non-zero
	if(arg <= ret_val)return ret_val;			// okay, rounding was up
	if(arg <= (ret_val+1.0f)) return ret_val + 1.0f;	// fix up, rounding was down
#if defined(DEBUG) && !defined(RAG_SIM)
	xe_printf("ceilf ERROR %f %f\n",arg,(arg+16777216.0f));RAG_FLUSH;
	xe_printf("ceilf ERROR %f %f\n",arg,((arg+16777216.0f)-16777216.0f));RAG_FLUSH;
	xe_printf("ceilf ERROR %f %f\n",arg,ret_val);RAG_FLUSH;
	xe_exit(1);
#endif
	return 0.0f/0.0f;
}

double round(double arg) {
	if(arg<0.0) {
		return (double)((uint64_t)(arg-0.5));
	} else {
		return (double)((uint64_t)(arg+0.5));
	}
}
