#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <pthread.h>
#include "Alloc.h"
#include "Debug.h"
#include "pil.h"
#include "pil_mem.h"
#include "uthash.h"

typedef struct block_ref {
    void*    ptr;       // HASH KEY
    uint64_t block_id;  // HASH VALUE
    UT_hash_handle hh;
} REF;

static REF *ht = NULL;
//static pthread_mutex_t lock;

void Alloc_init() {
    //assert(pthread_mutex_init(&lock, NULL) == 0);
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
void *Alloc_acquire_block(int node, size_t size)
{
    gpp_t g;
    pil_alloc(&g, size);

    REF *ref = malloc(sizeof(REF));
    ref->block_id = (uint64_t) g.guid;
    ref->ptr = g.ptr;
#ifdef DEBUG
    // test non-null
    ASSERT(g.ptr);
    // Test uniqueness
    REF *exists;
    //ASSERT(pthread_mutex_lock(&lock) == 0);
        HASH_FIND_PTR(ht, &g.ptr, exists);
    //pthread_mutex_unlock(&lock);
    ASSERT(exists == NULL); 
#endif
    //ASSERT(pthread_mutex_lock(&lock) == 0);
        HASH_ADD_PTR(ht, ptr, ref); 
    //pthread_mutex_unlock(&lock);
    return ref->ptr;
}

uint64_t Alloc_get_block_id(void *ptr)
{
    REF* ref;
    //ASSERT(pthread_mutex_lock(&lock) == 0);
        HASH_FIND_PTR(ht, &ptr, ref);
    //pthread_mutex_unlock(&lock);
    ASSERT(ref != NULL);
    return ref->block_id;
}

/**
 *  Free a data block
 *  @param id         The unique ID is used as a reference to the data block
 */
void Alloc_free_block(void* ptr)
{
    gpp_t g;
    REF* ref;
    //ASSERT(pthread_mutex_lock(&lock) == 0);
        HASH_FIND_PTR(ht, &ptr, ref);
    //pthread_mutex_unlock(&lock);
    ASSERT(ref != NULL);
    //ASSERT(pthread_mutex_lock(&lock) == 0);
        HASH_DEL(ht, ref);
    //pthread_mutex_unlock(&lock);

    g.guid = (guid_t) ref->block_id;
    g.ptr = ref->ptr;
    pil_free(g);
    free(ref);
}

int Alloc_no_object_exists() {
    return ((ht == NULL) ? 1:0);
}

int Alloc_count_objects() {
    return HASH_COUNT(ht);
}

