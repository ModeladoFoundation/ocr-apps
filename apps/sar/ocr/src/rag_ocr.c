#include "ocr.h"
#include "rag_ocr.h"

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

#ifndef TG_ARCH
static ocrGuid_t locate_in_spad = NULL_GUID;
static ocrGuid_t locate_in_bsm  = NULL_GUID;
static ocrGuid_t locate_in_dram = NULL_GUID;
#else
static ocrGuid_t locate_in_spad = NULL_GUID;
static ocrGuid_t locate_in_bsm  = NULL_GUID;
static ocrGuid_t locate_in_dram = NULL_GUID;

#if 0
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
#endif

void *spad_calloc(ocrGuid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_spad,(u64)&locate_in_spad_data);
#endif
#endif
#ifdef DEBUG
	PRINTF("SPAD_CALLOC n %ld size %ld (ptr=%16.16lx)\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_spad);
#ifdef DEBUG
	PRINTF("SPAD_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	spad_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *spad_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef DEBUG
	PRINTF("SPAD_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_spad,(u64)&locate_in_spad_data);
#endif
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_spad);
#ifdef DEBUG
	PRINTF("SPAD_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void spad_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	PRINTF("SPAD_FREE (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)dbp,GUID_VALUE(dbg));RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void *bsm_calloc(ocrGuid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_bsm,(u64)&locate_in_bsm_data);
#endif
#endif
#ifdef DEBUG
	PRINTF("BSM_CALLOC n %ld size %ld\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_bsm);
#ifdef DEBUG
	PRINTF("BSM_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	bsm_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *bsm_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp = NULL;
	ocrGuid_t tmp_dbg = NULL_GUID;
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_bsm,(u64)&locate_in_bsm_data);
#endif
#endif
#ifdef DEBUG
	PRINTF("BSM_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_bsm);
#ifdef DEBUG
	PRINTF("BSM_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void bsm_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	PRINTF("BSM_FREE (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)dbp,GUID_VALUE(dbg));RAG_FLUSH;
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
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_dram,(u64)&locate_in_dram_data);
#endif
#endif
#ifdef DEBUG
	PRINTF("DRAM_CALLOC n %ld size %ld\n",n,size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,n*size,locate_in_dram);
#ifdef DEBUG
	PRINTF("DRAM_CALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	dram_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void *dram_malloc(ocrGuid_t *dbg, size_t size) {
	void *tmp_dbp;
	ocrGuid_t tmp_dbg = *dbg;
#ifdef TG_ARCH
#if 0
        ocrGuidSetAddressU64(&locate_in_dram,(u64)&locate_in_dram_data);
#endif
#endif
#ifdef DEBUG
	PRINTF("DRAM_MALLOC size %ld\n",size);RAG_FLUSH;
#endif
	OCR_DB_CREATE(tmp_dbg,tmp_dbp,size,locate_in_dram);
#ifdef DEBUG
	PRINTF("DRAM_MALLOC (ptr=%16.16lx) (dbg=%16.16lx)\n",(uint64_t)tmp_dbp,GUID_VALUE(tmp_dbg));RAG_FLUSH;
#endif
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void dram_free(void *dbp, ocrGuid_t dbg) {
	OCR_DB_RELEASE(dbg);
	OCR_DB_FREE(dbp,dbg);
#ifdef DEBUG
	PRINTF("DRAM_FREE (dbg=%16.16lx)\n",GUID_VALUE(dbg));RAG_FLUSH;
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
	PRINTF("GblPtrToDB OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	RAG_MEMCPY(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("SPADtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("SPADtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void SPADtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("SPADtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("BSMtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("BSMtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void BSMtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("BSMtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoSPAD(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("DRAMtoSPAD OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("DRAMtoBSM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void DRAMtoDRAM(void *out, void *in, size_t size) {
#ifdef DEBUG
	PRINTF("DRAMtoDRAM OUT = %16.16lx IN = %16.16lx SIZE = %ld\n",(uint64_t)out,(uint64_t)in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef TG_ARCH
	rag_memcpy(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}

#ifdef RAG_CRLIBM
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

float  sinf(float arg) {
	return (float)sin_rn((double)arg);
}
float  cosf(float arg) {
	return (float)cos_rn((double)arg);
}
void  sincosf(float arg, float *si, float *rc) {
	*si = (float)sin_rn((double)arg);
	*rc = (float)cos_rn((double)arg);
	return;
}
#if 0
float  sqrtf(float arg) {
	return __ieee754_sqrtf(arg);
}
#endif
double  sin(double arg) {
	return sin_rn(arg);
}
double  cos(double arg) {
	return cos_rn(arg);
}
void  sincos(double arg, double *si, double *rc) {
	*si = sin_rn(arg);
	*rc = cos_rn(arg);
	return;
}
#if 0
double  sqrt(double arg) {
	return __ieee754_sqrt(arg);
}
#endif

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
#if defined(DEBUG) && !defined(TG_ARCH)
	PRINTF("floorf ERROR %f %f\n",arg,(arg+16777216.0f));RAG_FLUSH;
	PRINTF("floorf ERROR %f %f\n",arg,((arg+16777216.0f)-16777216.0f));RAG_FLUSH;
	PRINTF("floorf ERROR %f %f\n",arg,ret_val);RAG_FLUSH;
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
#if defined(DEBUG) && !defined(TG_ARCH)
	PRINTF("ceilf ERROR %f %f\n",arg,(arg+16777216.0f));RAG_FLUSH;
	PRINTF("ceilf ERROR %f %f\n",arg,((arg+16777216.0f)-16777216.0f));RAG_FLUSH;
	PRINTF("ceilf ERROR %f %f\n",arg,ret_val);RAG_FLUSH;
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
#endif
