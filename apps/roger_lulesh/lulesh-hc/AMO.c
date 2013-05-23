#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "AMO.h"

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
