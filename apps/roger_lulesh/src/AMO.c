#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "RAG.h"

#include "AMO.h"

#ifdef CILK
extern "C" {
#endif

/* double */

void AMO__sync_addition_double(SHARED double *memPtr, double addVal) {
	volatile double memVal, newVal;
	volatile uint64_t compVal, exchVal, rtrnVal;
#ifdef UPC
	rtrnVal = bupc_atomicU64_read_strict((shared void *) memPtr);
#else
	rtrnVal = *(uint64_t *)memPtr;
#endif
	do {
                memVal  = *(double *)&rtrnVal;

		newVal  = memVal + addVal;

		compVal = rtrnVal;
		exchVal = *(uint64_t *)&newVal;
#if 0
fprintf(stdout,"memVal = %e, addVal = %e, newVal = %e\n",memVal,addVal,newVal);fflush(stdout);
fprintf(stdout,"compVal = %16.16lx, exchVal = %16.16lx\n",compVal,exchVal);fflush(stdout);
fprintf(stdout,"rtrnVal = %16.16lx, exchVal = %16.16lx\n",rtrnVal,*(uint64_t *)&newVal);fflush(stdout);
#endif
#ifdef UPC
	        rtrnVal = bupc_atomicU64_cswap_strict((shared void *)memPtr,compVal,exchVal);
#else
		rtrnVal = __sync_val_compare_and_swap((uint64_t *)memPtr,compVal,exchVal);
#endif
	} while ( rtrnVal != compVal );
#if 0
  if(*memPtr != newVal) {
    fprintf(stdout,"ERROR %e = AMO__sync_addition_double(%e,%e) != %e ERROR\n",*memPtr,memVal,addVal,newVal);fflush(stdout);
    fprintf(stdout,"ERROR rtrnVal = %e RTRNvAL = %e ERROR\n",rtrnVal,RTRNvAL);fflush(stdout);
    exit(0);
  }
#endif
}

/* uint64_t */

uint64_t AMO__sync_fetch_and_add_uint64_t(SHARED uint64_t *memPtr, uint64_t addVal) {
#ifdef UPC
	uint64_t rtrnVal = bupc_atomicU64_fetchadd_strict((shared void *)memPtr,addVal);
#else
	uint64_t rtrnVal = __sync_fetch_and_add(memPtr,addVal);
#endif
	return rtrnVal;
}

uint64_t AMO__sync_val_compare_and_swap_uint64_t(SHARED uint64_t *memPtr, uint64_t compVal, uint64_t exchVal) {
#ifdef UPC
	uint64_t rtrnVal = bupc_atomicU64_cswap_strict((shared void *)memPtr,compVal,exchVal);
#else
	uint64_t rtrnVal = __sync_val_compare_and_swap(memPtr,compVal,exchVal);
#endif
	return rtrnVal;
}

int AMO__sync_bool_compare_and_swap_uint64_t(SHARED uint64_t *memPtr, uint64_t compVal, uint64_t exchVal) {
#ifdef UPC
	int rtrnVal = (compVal == bupc_atomicU64_cswap_strict((shared void *)memPtr,compVal,exchVal));
#else
	int rtrnVal = __sync_bool_compare_and_swap(memPtr,compVal,exchVal);
#endif
	return rtrnVal;
}

/* int64_t */

int64_t AMO__sync_fetch_and_add_int64_t(SHARED int64_t *memPtr, int64_t addVal) {
#ifdef UPC
	int64_t rtrnVal = bupc_atomicI64_fetchadd_strict((shared void *)memPtr,addVal);
#else
	int64_t rtrnVal = __sync_fetch_and_add(memPtr,addVal);
#endif
	return rtrnVal;
}

int64_t AMO__sync_val_compare_and_swap_int64_t(SHARED int64_t *memPtr, int64_t compVal, int64_t exchVal) {
#ifdef UPC
	int64_t rtrnVal = bupc_atomicI64_cswap_strict((shared void *)memPtr,compVal,exchVal);
#else
	int64_t rtrnVal = __sync_val_compare_and_swap(memPtr,compVal,exchVal);
#endif
	return rtrnVal;
}

int     AMO__sync_bool_compare_and_swap_int64_t(SHARED int64_t *memPtr, int64_t compVal, int64_t exchVal) {
#ifdef UPC
	int rtrnVal = (compVal == bupc_atomicI64_cswap_strict((shared void *)memPtr,compVal,exchVal));
#else
	int rtrnVal = __sync_bool_compare_and_swap(memPtr,compVal,exchVal);
#endif
	return rtrnVal;
}

/* LOCKS */

SHARED uint64_t idamax_lock = 0; // 0 --> UNLOCKED; 1 --> LOCKED
SHARED uint64_t idamin_lock = 0; // 0 --> UNLOCKED; 1 --> LOCKED
SHARED uint64_t *pidamax_lock = &idamax_lock;
SHARED uint64_t *pidamin_lock = &idamin_lock;


void AMO__lock_uint64_t(SHARED uint64_t *memPtr) {
  const uint64_t rag_lock = 1;
  const uint64_t rag_unlock = 0;
  for(;;) {
#ifdef UPC
    if (rag_unlock == bupc_atomicU64_cswap_strict((shared void *)memPtr,rag_unlock,rag_lock))
      break;
#else
    if (__sync_bool_compare_and_swap(memPtr, rag_unlock, rag_lock))
      break;
#endif
    };
  return;
}

void AMO__unlock_uint64_t(SHARED uint64_t *memPtr) {
  const uint64_t rag_lock = 1;
  const uint64_t rag_unlock = 0;
  for(;;) {
#ifdef UPC
    if (rag_lock == bupc_atomicU64_cswap_strict((shared void *)memPtr,rag_lock,rag_unlock))
      break;
#else
    if (__sync_bool_compare_and_swap(memPtr, rag_lock, rag_unlock))
      break;
#endif
  };
  return;
}
#ifdef CILK
} // extern "C"
#endif
