#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Alloc.h"
#include "Debug.h"
#include "pil.h"
#include "pil_mem.h"

void Alloc_init() {
}

/**
 *  Allocate a block and get an unique block id for later reference.
 *  How this block is allocated depends on how PIL handles it. It
 *  is quite possible that there are one copy for each node in a
 *  distributed system.
 *  @param size       The size of the data block needed.
 *  @return The allocated block id. 0 means allocation failed.
 */
void *Alloc_acquire_block(size_t size)
{
    gpp_t g;
    pil_alloc(&g, sizeof(gpp_t) + size); // allocate extra space to store a pointer to the gpp so it won't be lost
    *(gpp_t *)g.ptr = g; // store the pointer to new gpp
    //printf("allocated block %p\n", ((void*)g->ptr) + sizeof(gpp_t *));
    return ((void*)g.ptr) + sizeof(gpp_t); // skip the part that stores the gpp pointer so that it is hidden
}

uint64_t Alloc_get_block_id(void *ptr)
{
    gpp_t g = *(gpp_t*)(ptr - sizeof(gpp_t));
    return (uint64_t) (g.guid);
}

gpp_t Alloc_recover_gpp(void *ptr)
{
    return *(gpp_t*)(ptr - sizeof(gpp_t));
}

/**
 *  Free a data block
 *  @param id         The unique ID is used as a reference to the data block
 */
void Alloc_free_block(void* ptr)
{
    //printf("freed block %p\n", ptr);
    gpp_t g = *(gpp_t*)(ptr - sizeof(gpp_t));
    pil_free(g);
}

