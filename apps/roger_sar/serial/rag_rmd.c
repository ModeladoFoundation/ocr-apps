#include "rag_rmd.h"

#ifdef RAG_SIM
int exit(int);
#endif

static rmd_location_t    alloc_spad = {
	.type                     = RMD_LOC_TYPE_RELATIVE,
	.data.relative.identifier = 0,
	.data.relative.level      = RMD_LOCATION_CORE
};
static rmd_location_t    alloc_blk = {
	.type                     = RMD_LOC_TYPE_RELATIVE,
	.data.relative.identifier = 0,
	.data.relative.level      = RMD_LOCATION_BLOCK
};

static rmd_location_t    alloc_dram = {
	.type                     = RMD_LOC_TYPE_RELATIVE,
	.data.relative.identifier = 0,
	.data.relative.level      = RMD_LOCATION_DRAM
};

static const int local = 0;

void *spad_calloc(rmd_guid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
#ifdef TRACE
#endif
	RMD_DB_ALLOC(&tmp_dbg,n*size,local,&alloc_spad);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
#ifdef TRACE
	xe_printf("SPAD_CALLOC n %ld size %ld (ptr=%ld) (dbg=%ld)\n",n,size,(uint64_t)tmp_dbp,tmp_dbg.data);RAG_FLUSH;
#endif
	return tmp_dbp;
}

void *spad_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_spad);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
#ifdef TRACE
	xe_printf("SPAD_MALLOC size %ld (ptr=%ld) (dbg=%ld)\n",size,(uint64_t)tmp_dbp,tmp_dbg.data);RAG_FLUSH;
#endif
	return tmp_dbp;
}

void spad_free(void *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef TRACE
	xe_printf("SPAD_FREE (ptr=%ld) (dbg=%ld)\n",(uint64_t)dbp,dbg.data);RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void spad_memset(void *out, int val, size_t size) {
	memset(out,val,size);
}

void *bsm_calloc(rmd_guid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,n*size,local,&alloc_blk);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	bsm_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
#ifdef TRACE
	xe_printf("BSM_CALLOC n %ld size %ld (ptr=%ld) (dbg=%ld)\n",n,size,(uint64_t)tmp_dbp,tmp_dbg.data);RAG_FLUSH;
#endif
	return tmp_dbp;
}

void *bsm_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_blk);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
#ifdef TRACE
	xe_printf("BSM_MALLOC size %ld (ptr=%ld) (dbg=%ld)\n",size,(uint64_t)tmp_dbp,tmp_dbg.data);RAG_FLUSH;
#endif
	return tmp_dbp;
}

void bsm_free(void *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef TRACE
	xe_printf("BSM_FREE (ptr=%ld) (dbg=%ld)\n",(uint64_t)dbp,dbg.data);RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void bsm_memset(void *out, int val, size_t size) {
	char *__out = (char *)out;
	const char tmp = val;
	for(int i=0;i<size;i++) {
		REM_ST8_ADDR(__out++,tmp);
	} // for i (size)
}

void *dram_calloc(rmd_guid_t *dbg, size_t n, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,n*size,local,&alloc_dram);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	dram_memset(tmp_dbp,0,n*size);
	*dbg = tmp_dbg;
#ifdef TRACE
	xe_printf("DRAM_CALLOC n %ld size %ld\n (dbg=%ld)",n,size,tmp_dbg.data);RAG_FLUSH;
#endif
	return tmp_dbp;
}

void *dram_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
#ifdef TRACE
	xe_printf("DRAM_MALLOC size %ld (dbg=%ld)\n",size,tmp_dbg.data);RAG_FLUSH;
#endif
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_dram);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
	return tmp_dbp;
}

void dram_free(void *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef TRACE
	xe_printf("DRAM_FREE (dbg=%ld)\n",dbg.data);RAG_FLUSH;
#endif
	dbp = (void *)NULL;
}

void dram_memset(void *out, int val, size_t size) {
	char *__out = (char *)out;
	const char tmp = val;
	for(int i=0;i<size;i++) {
		REM_ST8_ADDR(__out++,tmp);
	} // for i (size)
}

#if defined(RAG_AFL) || defined(RAG_SIM)
int32_t  RAG_GET_INT(int32_t *addr)
{int32_t tmp_int;REM_LD32_ADDR(tmp_int,addr);return tmp_int;}

void     RAG_PUT_INT(int32_t *addr, int32_t value)
{REM_ST32_ADDR(addr,value);return;}

uint32_t RAG_GET_UINT(uint32_t *addr)
{uint32_t tmp_uint;REM_LD32_ADDR(tmp_uint,addr);return tmp_uint;}

void     RAG_PUT_UINT(uint32_t *addr, uint32_t value)
{REM_ST32_ADDR(addr,value);return;}

int64_t  RAG_GET_LONG(int64_t *addr)
{int64_t tmp_long;REM_LD64_ADDR(tmp_long,addr);return tmp_long;}

void     RAG_PUT_LONG(int64_t *addr, int64_t value)
{REM_ST64_ADDR(addr,value);return;}

uint64_t RAG_GET_ULONG(uint64_t *addr)
{uint64_t tmp_ulong;REM_LD64_ADDR(tmp_ulong,addr);return tmp_ulong;}

void     RAG_PUT_ULONG(uint64_t *addr, uint64_t value)
{REM_ST64_ADDR(addr,value);return;}

float    RAG_GET_FLT(float    *addr)
{float   tmp_flt;REM_LD32_ADDR(tmp_flt,addr);return tmp_flt;}

void     RAG_PUT_FLT(float    *addr, float    value)
{REM_ST32_ADDR(addr,value);return;}

double   RAG_GET_DBL(double   *addr)
{float   tmp_dbl;REM_LD64_ADDR(tmp_dbl,addr);return tmp_dbl;}

void     RAG_PUT_DBL(double   *addr, double   value)
{REM_ST64_ADDR(addr,value);return;}

void *   RAG_GET_PTR(void     *addr)
{void*   tmp_ptr;REM_LD64_ADDR(tmp_ptr,addr);return tmp_ptr;}

void     RAG_PUT_PTR(void     *addr, void    *value)
{REM_ST64_ADDR(addr,value);return;}
#endif

#if 1
void SPADtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("OUT = %lx IN = %lx SIZE = %ld\n",(uint64_t)__out,(uint64_t)__in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	MEMCPY(out,in,size);
//	REM_STX_ADDR_SIZE(out,in,size);
#else
	memcpy(out,in,size);
#endif
}
#else
void SPADtoBSM(void *out, void *in, size_t size) {
	char *__out = (char *)out;
	char *__in  = (char *)in ;
#ifdef DEBUG
	xe_printf("OUT = %lx IN = %lx SIZE = %ld\n",(uint64_t)__out,(uint64_t)__in,(uint64_t)size);RAG_FLUSH;
#endif
	for(int i=0;i<size;i++) {
		REM_ST8_ADDR(__out++,*__in++);
	} // for i (size)
}
#endif

void BSMtoBSM(void *out, void *in, size_t size) {
#ifdef DEBUG
	xe_printf("OUT = %lx IN = %lx SIZE = %ld\n",(uint64_t)__out,(uint64_t)__in,(uint64_t)size);RAG_FLUSH;
#endif
#ifdef RAG_SIM
	MEMCPY(out,in,size);
#else
	memcpy(out,in,size);
#endif
}
