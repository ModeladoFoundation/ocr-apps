#include "rag_rmd.h"

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

void *rag_spad_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_spad);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
#ifdef DEBUG
	xe_printf("SPAD_MALLOC size %ld (ptr=%ld) (dbg=%ld)\n",size,(uint64_t)tmp_dbp,tmp_dbg.data);
#endif
	return tmp_dbp;
}

void rag_spad_free(void *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef DEBUG
	xe_printf("SPAD_FREE (ptr=%ld) (dbg=%ld)\n",(uint64_t)dbp,dbg.data);
#endif
	dbp = (void *)NULL;
}

void *rag_bsm_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_blk);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
#ifdef DEBUG
	xe_printf("BSM_MALLOC size %ld (ptr=%ld) (dbg=%ld)\n",size,(uint64_t)tmp_dbp,tmp_dbg.data);
#endif
	return tmp_dbp;
}

void rag_bsm_free(void *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef DEBUG
	xe_printf("BSM_FREE (ptr=%ld) (dbg=%ld)\n",(uint64_t)dbp,dbg.data);
#endif
	dbp = (void *)NULL;
}

void *rag_dram_malloc(rmd_guid_t *dbg, size_t size) {
	void *tmp_dbp;
	rmd_guid_t tmp_dbg = *dbg;
	RMD_DB_ALLOC(&tmp_dbg,size,local,&alloc_dram);
	RMD_DB_MEM(&tmp_dbp,tmp_dbg);
	*dbg = tmp_dbg;
#ifdef DEBUG
	xe_printf("DRAM_MALLOC size %ld (ptr=%ld) (dbg=%ld)\n",size,(uint64_t)tmp_dbp,tmp_dbg.data);
#endif
	return tmp_dbp;
}

void rag_dram_free(void rmdglobal *dbp, rmd_guid_t dbg) {
	RMD_DB_RELEASE(dbg);
	RMD_DB_FREE(dbg);
#ifdef DEBUG
	xe_printf("DRAM_FREE (dbg=%ld)\n",(uint64_t)dbg.data);
#endif
	dbp = (void rmdglobal *)NULL;
}

void rag_DRAMtoSPAD(void *out, void rmdglobal * restrict in, size_t size) {
#ifdef RAG_SIM
	REM_LDX_ADDR_SIZE(*out,in,size);
#else
	memcpy(out,in,size);
#endif
}

void rag_BSMtoSPAD(void *out, void rmdblock * restrict in, size_t size) {
#ifdef RAG_SIM
	REM_LDX_ADDR_SIZE(*out,in,size);
#else
	memcpy(out,in,size);
#endif
}
