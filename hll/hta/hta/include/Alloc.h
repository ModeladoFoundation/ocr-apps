#ifndef __ALLOC_H__
#define __ALLOC_H__
#include <inttypes.h>
#include <stddef.h>
#include "pil.h"

/*
 *  Alloc functions are wrappers of backend memory allocation APIs
 *
 */
void Alloc_init();

void *Alloc_acquire_block(size_t size);

uint64_t Alloc_get_block_id(void *ptr);

void Alloc_free_block(void *ptr);

gpp_t Alloc_recover_gpp(void *ptr);

#endif
