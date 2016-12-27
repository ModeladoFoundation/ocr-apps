#ifndef __MEMORY_H__
#define __MEMORY_H__
///////////////////////////// Memory Use Tracking /////////////////////////////
//
// Linked list of free blocks
// Core methods are region (L1,L2,...) agnostic.
// Per region alloc/free methods provided.
//
typedef enum mem_type {
    Mem_None,
    Mem_L1,
    Mem_L2,
    Mem_IPM,
    Mem_DRAM,
    Mem_ANY,
    Mem_CE      // special - meaning from CE private space
} mem_type;

typedef enum {
    CE = 0, XE = 1  // add the XE number to XE to get the actual agent #
} mem_agent;
//
// Memory region descriptor (opaque)
// This holds the free list for an arbitrary memory region
//
typedef struct mem_region mem_region;
//
// Memory allocation element
//
#define MAX_FREE_MEM    64  // maximum number of elements available

#define NO_FREE_MEM ((uint64_t)-1L)

typedef struct mem_seg mem_seg;

struct mem_seg {
    uint64_t va;            // MR format segment start address
    uint64_t len:56,        // allocation size
             private:1,     // XE private - reclaim on finish
             unused:2,      // flag bits in reserve
             agent:5;       // which agent allocated (CE = 0, XE0 = 1,...)
    mem_region * region;    // associated region
    mem_seg * next;         // chain pointer for usage or free lists
};

//
// subsystem init
//
void mem_init( void );
//
// Create a new memory region
//
mem_region * mem_region_create( mem_type type, bool track, const char *name );
//
// Reap resources from memory region
//
void mem_region_destroy( mem_region * mr );
//
// get the region attributes
//
const char * mem_region_name( mem_region * mr );
mem_type mem_region_type( mem_region * mr );
//
// Add a free memory range of len bytes to the region
//
int mem_region_add( mem_region * mr, uint64_t va, uint64_t len );
//
// Remove a seg from a list of same
//
int mem_remove( mem_seg ** head, mem_seg * seg );
//
// Free a previously allocated segment
//
int mem_free( mem_seg * ms );
//
// Release a mem_seg structure without de-allocating the memory segment
// Used for fixed allocations never going to be freed
//
void mem_return( mem_seg * ms );
//
// Allocate len bytes at address off from the region
//
mem_seg * mem_alloc_at( mem_region * mr, uint64_t va, uint64_t len );
//
// Allocate len bytes from the region
//
mem_seg * mem_alloc( mem_region * mr, uint64_t len );

//
// Not a validator, so much as a categorizer. May lie for non-MR addrs,
// but will never return a wrong type for a good address (MR format).
//
mem_type  mem_type_of( uint64_t addr );

void mem_region_dump( mem_region * mr );

#endif // __MEMORY_H__
