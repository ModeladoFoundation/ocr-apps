#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "AMO.h"

#ifdef CILK
extern "C" {
#endif

/* double */

void AMO__sync_addition_double(double *memPtr, double addVal) {
	double memVal, newVal;
	uint64_t compVal, exchVal, rtrnVal;
	do {
		memVal    = *memPtr;
		newVal    = memVal + addVal;
		compVal = *(uint64_t *)&memVal;
		exchVal = *(uint64_t *)&newVal;
		rtrnVal =__sync_val_compare_and_swap((uint64_t *)memPtr,compVal,exchVal);
	} while ( rtrnVal != compVal );
}

/* uint64_t */

uint64_t AMO__sync_fetch_and_add_uint64_t(uint64_t *memPtr, uint64_t addVal) {
	uint64_t rtrnVal =__sync_fetch_and_add(memPtr,addVal);
	return rtrnVal;
}

uint64_t AMO__sync_val_compare_and_swap_uint64_t(uint64_t *memPtr, uint64_t compVal, uint64_t exchVal) {
	uint64_t rtrnVal =__sync_val_compare_and_swap(memPtr,compVal,exchVal);
	return rtrnVal;
}

int AMO__sync_bool_compare_and_swap_uint64_t(uint64_t *memPtr, uint64_t compVal, uint64_t exchVal) {
	int rtrnVal =__sync_bool_compare_and_swap(memPtr,compVal,exchVal);
	return rtrnVal;
}

/* int64_t */

int64_t AMO__sync_fetch_and_add_int64_t(int64_t *memPtr, int64_t addVal) {
	int64_t rtrnVal =__sync_fetch_and_add(memPtr,addVal);
	return rtrnVal;
}

int64_t AMO__sync_val_compare_and_swap_int64_t(int64_t *memPtr, int64_t compVal, int64_t exchVal) {
	int64_t rtrnVal =__sync_val_compare_and_swap(memPtr,compVal,exchVal);
	return rtrnVal;
}

int     AMO__sync_bool_compare_and_swap_int64_t(int64_t *memPtr, int64_t compVal, int64_t exchVal) {
	int rtrnVal =__sync_bool_compare_and_swap(memPtr,compVal,exchVal);
	return rtrnVal;
}

/* LOCKS */

static uint64_t idamax_lock = 0; // 0 --> UNLOCKED; 1 --> LOCKED
static uint64_t idamin_lock = 0; // 0 --> UNLOCKED; 1 --> LOCKED
uint64_t *pidamax_lock = &idamax_lock;
uint64_t *pidamin_lock = &idamin_lock;


void AMO__lock_uint64_t(uint64_t *memPtr) {
  const uint64_t rag_lock = 1;
  const uint64_t rag_unlock = 0;
  for(;;) {
    if (__sync_bool_compare_and_swap(memPtr, rag_unlock, rag_lock))
      break;
    };
  return;
}

void AMO__unlock_uint64_t(uint64_t *memPtr) {
  const uint64_t rag_lock = 1;
  const uint64_t rag_unlock = 0;
  for(;;) {
    if (__sync_bool_compare_and_swap(memPtr, rag_lock, rag_unlock))
      break;
  };
  return;
}
#ifdef CILK
} // extern "C"
#endif
