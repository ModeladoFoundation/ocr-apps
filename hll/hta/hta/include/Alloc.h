#ifndef __ALLOC_H__
#define __ALLOC_H__
#include <inttypes.h>
#include <stddef.h>

/* 
 *  Alloc functions are wrappers of backend memory allocation APIs
 *
 */
void Alloc_init();

void *Alloc_acquire_block(int node, size_t size);

uint64_t Alloc_get_block_id(void *ptr);

void Alloc_free_block(void *ptr);

/// Check if all dynamically allocated objects are freed
int Alloc_no_object_exists();
int Alloc_count_objects();

// TODO: add unhashed Alloc functions for allocation in parallel executions
//void *Alloc_get_block_ptr(REF r);
//void Alloc_invalidate_ref(REF *r);
//void Alloc_free_and_invalidate(REF *r);

#endif
