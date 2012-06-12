#include "rag_rmd.h"

typedef unsigned long uint64_t; 
typedef   signed long  int64_t;

#include "cacheIsa.h"

/* Random number generator */
static const uint64_t POLY =    0x0000000000000007UL;
static const uint64_t PERIOD = 1317624576693539401UL;

#ifndef CACHE_RUN
#define CACHE_RUN 0
#endif

#if CACHE_RUN == 0
#define TABLE_TYPE rmdglobal
#else
#define TABLE_TYPE rmdcache
#endif

#ifdef CHECK
#define LOG2STABLESIZE ((uint64_t)10)		/* log2(16)   ==  4 */
#define     STABLESIZE (1UL<<LOG2STABLESIZE)	/* 128 B / 8B == 16 */
#define  LOG2TABLESIZE ((uint64_t)12)
#define      TABLESIZE (1UL<< LOG2TABLESIZE)
#else
#if (RAG_CACHE==1) || (CACHE_RUN==1)
// half of 32KB cache (half of the 64KB scratch-pad is used for cache)
#define LOG2STABLESIZE ((uint64_t)10)		/* log2(2048) == 11 */
#define     STABLESIZE (1UL<<LOG2STABLESIZE)	/* 16 KB / 8B == 2K */
#else
// half of 64KB scratch-pad
#define LOG2STABLESIZE ((uint64_t)12)		/* log2(4096) == 12 */
#define     STABLESIZE (1UL<<LOG2STABLESIZE)	/* 32 KB / 8B == 4K */
#endif
#define  LOG2TABLESIZE ((uint64_t)20)		/* log2(1M)   == 20 */
#define      TABLESIZE (1UL<<LOG2TABLESIZE)	/* 8 MB / 8B  == 1M */
#endif

uint64_t TABLE_TYPE  * restrict stable;
uint64_t TABLE_TYPE  * restrict  table;

#define NTHREADS 1
const int64_t nThreads = NTHREADS;

const int64_t  log2stableSize =      LOG2STABLESIZE;
const int64_t      stableSize = 1UL<<LOG2STABLESIZE;
const int64_t   log2tableSize =       LOG2TABLESIZE;
const int64_t       tableSize =  1UL<<LOG2TABLESIZE;
const uint64_t      tableMask = (1UL<<LOG2TABLESIZE)-1;

#ifdef CHECK
const int64_t numberUpdates = 4*TABLESIZE;
#else
const int64_t numberUpdates = 4*NTHREADS*TABLESIZE;
#endif

static uint64_t giantstep(int64_t n)
{
	int64_t  i, j;
	uint64_t m2[64];
	uint64_t temp, ran;

	while (n < 0) n += PERIOD;
	while (n > PERIOD) n -= PERIOD;
	if (n == 0) return 0x1;

	temp = 0x1;
	for (i=0; i<64; i++) {
		m2[i] = temp;
		temp = (temp << 1) ^ ((int64_t) temp < 0 ? POLY : 0);
		temp = (temp << 1) ^ ((int64_t) temp < 0 ? POLY : 0);
	}
    
	for (i=62; i>=0; i--)
		if ((n >> i) & 1)
			break;

	ran = 0x2;
	while (i > 0) {
		temp = 0;
		for (j=0; j<64; j++)
			if ((ran >> j) & 1)
				temp ^= m2[j];
		ran = temp;
		i -= 1;
		if ((n >> i) & 1)
		ran = (ran << ((uint64_t)1)) ^ ((int64_t) ran < 0 ? POLY : 0);
	}
#ifdef DEBUG
	xe_printf("giantstep(%ld) returns %ld\n",n,ran);
#endif  
	return ran;
}

static void myBlock(
	int64_t tid,
	int64_t nThreads,
	int64_t totalSize,
	int64_t *start,
	int64_t *stop,
	int64_t *size)
{
	int64_t div;
	int64_t rem;

	div = totalSize / nThreads;
	rem = totalSize % nThreads;

	if (tid < rem) {
		*start = tid * (div + 1);
		*stop  = *start + div;
		*size  = div + 1;
	} else {
		*start = tid * div + rem;
		*stop  = *start + div - 1;
		*size  = div;
	}
#ifdef DEBUG
	xe_printf("myBlock(%ld,%ld,%ld) returns (%ld,%ld,%ld)\n",
	tid, nThreads, totalSize, *start, *stop, *size);
#endif
	return;
}

void loop64( int64_t tid ) {
#ifdef TRACE
	xe_printf("// Initialize starting value for PRNG (not time critical)\n");
#endif

	int64_t start, stop, size;
	myBlock(tid, nThreads, numberUpdates, &start, &stop, &size);

	uint64_t ran;
	ran = giantstep(start);

#if (RAG_CACHE==0) && (CACHE_RUN==0)
#ifdef TRACE
	xe_printf("// Copy stable from DRAM to SPAD (not time critical)\n");
#endif
	rmd_guid_t spad_stable_dbg;
	uint64_t * restrict spad_stable = rag_spad_malloc(&spad_stable_dbg, STABLESIZE*sizeof(uint64_t));
	if(spad_stable == NULL) {
		xe_printf("spad_stable rag_spad_malloc error\n");
		xe_exit(1);
	}
	rag_DRAMtoSPAD( spad_stable, stable, STABLESIZE*sizeof(uint64_t) );
#ifdef DEBUG
	xe_printf("SPAD COPY OKAY?\n");
	for(int64_t i=0;i<STABLESIZE;i++) {
		if(spad_stable[i] != stable[i]) {
			xe_printf("SPAD %ld != %ld at %ld\n",spad_stable[i],stable[i],i);
		}
	}
	xe_printf("spad copy okay?\n");
#endif
#else
#ifdef TRACE
	xe_printf("// Caching stable and table from from DRAM.\n");
#endif
#endif
#ifdef TRACE
	xe_printf("// Run GUPS Kernel (time critical)\n");
#endif
	for (int64_t i=start; i<=stop; i++) {
#ifdef TRACE_LVL_1
		if( (i&((1UL<<16)-1)) == 0 ) {
			xe_printf("UPDATE i=%ld\n",i);
		}
#endif
#if 1
		ran = (ran + ran)            ^ (((int64_t) ran < 0) ? POLY : 0);
#else
		ran = (ran << ((uint64_t)1)) ^ (((int64_t) ran < 0) ? POLY : 0);
#endif
#if (RAG_CACHE==0) && (RAG_ATOMIC==0) && (CACHE_RUN==0)
		table[ran&tableMask] ^= spad_stable[ran>>(64UL-LOG2STABLESIZE)];
#endif
#if (RAG_CACHE==0) && (RAG_ATOMIC==1) && (CACHE_RUN==0)
#ifdef RAG_SIM
		uint64_t tmp =  spad_stable[ran>>(64UL-LOG2STABLESIZE)];
		REM_XADD64(table[ran&tableMask],tmp);
#else
		(void) __sync_fetch_and_xor(&table[ran&tableMask],spad_stable[ran>>(64UL-LOG2STABLESIZE)]);
#endif
#endif
#if (RAG_CACHE==0) && (RAG_ATOMIC==0) && (CACHE_RUN==1)
		table[ran&tableMask] ^= stable[ran>>(64UL-LOG2STABLESIZE)];
#endif
#if (RAG_CACHE==0) && (RAG_ATOMIC==1) && (CACHE_RUN==1)
#ifdef RAG_SIM
		uint64_t tmp =  stable[ran>>(64UL-LOG2STABLESIZE)];
		REM_XADD64(table[ran&tableMask],tmp);
#else
		(void) __sync_fetch_and_xor(&table[ran&tableMask],stable[ran>>(64UL-LOG2STABLESIZE)]);
#endif
#endif
#if (RAG_CACHE==1) && (RAG_ATOMIC==0)
#ifdef RAG_SIM
		uint64_t TABLE_TYPE * restrict  table_ptr;
		uint64_t TABLE_TYPE * restrict stable_ptr;
		uint64_t old_mem, val, new_new;
		table_ptr  = &table[ran&tableMask];
		stable_ptr = &stable[ran>>(64UL-LOG2STABLESIZE)];
		CAC_LD64_ADDR(old_mem,table_ptr);
		CAC_LD64_ADDR(val,stable_ptr);
		new_new = old_mem ^ val;
		CAC_ST64_ADDR(table_ptr,new_new);
        //CAC_WB_ADDR((&(table[ran&tableMask])));
#else
		table[ran&tableMask] ^= stable[ran>>(64UL-LOG2STABLESIZE)];
#endif
#endif
#if (RAG_CACHE==1) && (RAG_ATOMIC==1)
#ifdef RAG_SIM
		uint64_t TABLE_TYPE * restrict stable_ptr;
		uint64_t tmp;
		stable_ptr = &stable[ran>>(64UL-LOG2STABLESIZE)];
		CAC_LD64_ADDR(tmp,stable_ptr);
		CAC_XADD64(table[ran&tableMask],tmp);
        //CAC_WB_ADDR((&(table[ran&tableMask])));
#else
		(void) __sync_fetch_and_xor(&table[ran&tableMask],stable[ran>>(64UL-LOG2STABLESIZE)]);
#endif
#endif

#ifdef DEBUG_LVL_2
		xe_printf("update %ld @ %ld with %ld ran is %ld\n",
		i, ran&tableMask, ran>>(64UL-LOG2STABLESIZE), ran);
#endif
	}
#if RAG_CACHE==1
//__asm__ volatile ( "cache.wball"  :::"memory");
//__asm__ volatile ( "cache.invall" :::"memory");
#endif
#if (RAG_CACHE==0) && (CACHE_RUN==0)
	rag_spad_free(spad_stable,spad_stable_dbg);
#endif
	return;
}

rmd_guid_t main_codelet(uint64_t arg,int n_db,void *db_ptr[],rmd_guid_t *dbg) {
	rmd_guid_t ret_val = NULL_GUID;
#ifdef TRACE
	xe_printf("// Enter main_codelet RAG_CACHE=%d RAG_ATOMIC=%d CACHE_RUN=%d\n",RAG_CACHE, RAG_ATOMIC,CACHE_RUN);
#endif

#ifdef TRACE
#ifdef CHECK
	xe_printf("// Checking requested, so setting numberUpdates to 4*tableSize\n");
#else
	xe_printf("// Checking not requested, so setting numberUpdates to 4*nThreads*tableSize\n");
#endif
#endif

#ifdef TRACE
	xe_printf("// Initialize stable (not time critical)\n");
#endif
	rmd_guid_t stable_dbg;
	stable = (uint64_t TABLE_TYPE * restrict)rag_dram_malloc(&stable_dbg,STABLESIZE*sizeof(uint64_t));
	if(stable == NULL) {
		xe_printf("stable rag_dram_malloc error\n");
		xe_exit(1);
	}
#ifdef TRACE
	xe_printf("// log2stableSize %ld stableSize %ld\n",
	log2stableSize, stableSize );
#endif
	stable[0] = 0;
#ifdef DEBUG_LVL_2
	xe_printf("set stable[%ld]=%ld\n",(int64_t)0,stable[0]);
#endif
	for (int64_t i=1; i<STABLESIZE; i++) {
		stable[i] = stable[i-1] + 0x0123456789abcdefUL;
#ifdef DEBUG_LVL_2
		xe_printf("set stable[%ld]=%ld\n",i,stable[i]);
#endif
	}

#ifdef DEBUG_LVL_1
	xe_printf("check that stable is initialized correctly\n");
	if (stable[0] !=  0) {
		xe_printf("STABLE[0] is %ld should be 0\n",stable[0]);
	}
	for (int64_t i=1; i< STABLESIZE; i++) {
		if (stable[i] !=  stable[i-1] + 0x0123456789abcdefUL) {
			xe_printf("STABLE[%ld] is %ld should be %ld\n",i,stable[i],i);
		}
	}
#endif


#ifdef TRACE
	xe_printf("// Initialize table (not time critical)\n");
#endif
	rmd_guid_t table_dbg;
	table = (uint64_t TABLE_TYPE * restrict)rag_dram_malloc(&table_dbg,tableSize*sizeof(uint64_t));
	if(table == NULL) {
		xe_printf("table rag_dram_malloc error\n");
		xe_exit(1);
	}

#ifdef TRACE
	xe_printf("// nThreads %ld log2tableSize %ld numberUpdates %ld\n",
	nThreads, log2tableSize, tableSize, numberUpdates );
	xe_printf("// tableSize %ld numberUpdates %ld (%ld M)\n",
	tableSize, numberUpdates,
	((uint64_t)(((double)numberUpdates)/1000./1000.)));
#endif

	int64_t tid = 0;
	int64_t start, stop, size;

	myBlock(tid, nThreads, tableSize, &start, &stop, &size);

    	for (int64_t i = start; i <= stop; i++) {
#ifdef TRACE_LVL_1
		if( (i&((1UL<<16)-1)) == 0 ) {
			xe_printf("INIT i=%ld\n",i);
		}
#endif
		table[i] = i;
#ifdef DEBUG_LVL_2
		xe_printf("set table[%ld]=%ld\n",i,table[i]);
#endif
	}

#ifdef DEBUG_LVL_1
	xe_printf("check that table is initialized correctly\n");
	for (int64_t i = start; i <= stop; i++) {
		if (table[i] != i) {
			xe_printf("TABLE[%ld] is %ld should be %ld\n",i,table[i],i);
		}
	}
#endif

#ifdef TRACE
	xe_printf("// Setup (not time critical) and run GUPS Kernel (time critical)\n");
#endif
	loop64( tid );

#ifdef CHECK
	xe_printf("// Checking ... (not time critical)\n");
	uint64_t ran = 0x1;
	uint64_t cnt = 0;
	for (int64_t i=0; i<numberUpdates; i++) {
#ifdef TRACE_LVL_1
		if( (i&((1UL<<16)-1)) == 0 ) {
			xe_printf("CHECK i=%ld\n",i);
		}
#endif
		ran = (ran << ((uint64_t)1)) ^ (((int64_t) ran < 0) ? POLY : 0);
#ifdef RAG_SIM
#if (RAG_ATOMIC == 1)
		table[ran&tableMask] -= stable[ran>>(64UL-LOG2STABLESIZE)];
#elif (RAG_ATOMIC == 0)
		table[ran&tableMask] ^= stable[ran>>(64UL-LOG2STABLESIZE)];
#else
#error RAG_ATOMIC not zero or one 
#endif
#else
#if (RAG_ATOMIC == 1)
		(void) __sync_fetch_and_xor(&table[ran&tableMask],stable[ran>>(64UL-LOG2STABLESIZE)]);
#elif (RAG_ATOMIC == 0)
		table[ran&tableMask] ^= stable[ran>>(64UL-LOG2STABLESIZE)];
#else
#error RAG_ATOMIC not zero or one 
#endif
#endif
#ifdef DEBUG_LVL_2
		xe_printf("check  %ld @ %ld with %ld ran is %ld\n",
		i, ran&tableMask, ran>>(64UL-LOG2STABLESIZE), ran);
#endif
	}
	for (int64_t i=0; i<tableSize; i++) {
		if (table[i] != i) {
#ifdef TRACE
			xe_printf("table[%ld] is %ld should be %ld\n",i,table[i],i);
#endif
			cnt++;
		}
#ifdef DEBUG_LVL_2
 		else {
			xe_printf("table[%ld] is %ld\n",i,table[i]);
		}
#endif
	}
	xe_printf ("// Found %ld errors in %ld locations (%s).\n",
	cnt, tableSize, (cnt <= 0.01*tableSize) ? "passed" : "failed");
#else
#ifdef TRACE
	xe_printf("// Finished\n");
#endif
#endif
	rag_dram_free((void *) table, table_dbg);
	rag_dram_free((void *)stable,stable_dbg);
#ifdef TRACE
	xe_printf("// Leave main_codelet\n");
#endif
	xe_exit(0);
	return ret_val;
}
