#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Alloc.h"
#include "Debug.h"
//#include "uthash.h"
//
//typedef struct block_ref {
//    void*    ptr;       // HASH KEY
//    uint64_t block_id;  // HASH VALUE
//    UT_hash_handle hh;
//} REF;
//
//static REF *ht = NULL;

void Alloc_init() {
}

/**
 *  Allocate a block and get an unique block id for later reference.
 *  How this block is allocated depends on how PIL handles it. It
 *  is quite possible that there are one copy for each node in a
 *  distributed system.
 *  @param node       An integer number to specify the node where
 *                    allocation happens
 *  @param size       The size of the data block needed.
 *  @param ptr        A placeholder for the pointer to the newly allocated data block.
 *                    If it's NULL it means the user doesn't need the pointer for data accessing immediately.
 *                    The acquired pointer is only valid within the same EDT
 *  @return The allocated block id. 0 means allocation failed.
 */
void *Alloc_acquire_block(size_t size)
{
    // TODO: alloc works with MPI rank?
    void * ptr = malloc(size);
#ifdef DEBUG
    ASSERT(ptr);
#endif
    return ptr;
}

uint64_t Alloc_get_block_id(void *ptr)
{
    return (uint64_t) ptr;
}

/**
 *  Free a data block
 *  @param id         The unique ID is used as a reference to the data block
 */
void Alloc_free_block(void* ptr)
{

#ifdef DEBUG
    ASSERT(ptr);
#endif
    free(ptr);
}

int Alloc_no_object_exists() {
    return 1;
}

int Alloc_count_objects() {
    return 0;
}

