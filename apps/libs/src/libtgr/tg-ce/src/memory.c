#include <stdint.h>
#include <sys/time.h>

#include <tg-console.h>

#include "memory.h"
#include "util.h"

///////////////////////////// Memory Use Tracking /////////////////////////////
//
// Usage:
//  At start-up, memory regions are allocated (mem_region_create()) and held by
//  the using module(s)
//  The regions are populated with appropriate memory (va, len) segments
//  using mem_region_add().
//
//  As memory needs to be allocated for use (e.g., by XEs or the CE for stacks, etc)
//  A module will allocate it using the appropriate region with mem_alloc(). It should
//  remember the mem_seg structure passed back to use when/if freeing the memory.
//  Once a memory segment is done with, it can be freed with mem_free()
//
// memory data structures spin-lock - needed in a multi-core configuration
//
// To avoid deadlocks, we ALWAYS lock in this order (and release in the opposite):
//  Region
//  Region Pool
//  Segment Pool
//
typedef uint32_t mem_lock;

#ifdef TGR_MULTI_CORE
//
// Implement a simple spin-lock for a multi-core CE
//
static inline void mem_init_lock( mem_lock * lock )
{
    *lock = 0;
}

static inline void mem_get_lock( mem_lock * lock )
{
    while( __sync_lock_test_and_set( lock, 1 ) != 0 )
        ;
}

static inline void mem_put_lock( mem_lock * lock )
{
    __sync_lock_release( lock );
}
#else // ! TGR_MULTI_CORE
//
// Single threaded CE don't need no steenking locks
//
#define mem_init_lock( lock )
#define mem_get_lock( lock )
#define mem_put_lock( lock )

#endif  // ! TGR_MULTI_CORE

//
// Linked list of free blocks
// Core methods are region (L1,L2,...) agnostic.
// Per region alloc/free methods provided.
//
#define NUM_FREE_REGIONS  16
#define NUM_FREE_SEGS     100
//
// Pool of mem_seg structures to draw from
//
static struct {
    mem_seg * free;
    mem_seg   pool[NUM_FREE_SEGS];
    mem_lock  lock;
} Mem_segs_pool = {
    .free = NULL
};

//
// Private memory region structure
//
struct mem_region {
    const char * name;         // for debugging
    uint32_t     type: 8,      // mem_type for this region
                 track: 1,     // keep the tracking list
                 do_lock: 1,   // needs locking
                 unused: 22;   // unused flag space
    mem_lock     lock;         // region lock
    uint32_t     allocations;  // stats - count of allocations made
    uint32_t     frees;        // stats - count of frees
    u64          base_addr;    // lowest address in the range (not really used)
    mem_seg *    free;         // available memory area
    mem_seg *    alloc;        // allocation tracking list
    mem_region * next;         // link on region free list
};
//
// Pool of regions structures to allocate from
//
static struct {
    mem_region * free;
    mem_region   pool[NUM_FREE_REGIONS];
    mem_lock     lock;
} Mem_regions_pool = {
    .free = NULL
};
//
// Region lock support
//
static inline void mem_lock_region( mem_region * mr )
{
    if( mr->do_lock )
        mem_get_lock( & mr->lock );
}

static inline void mem_unlock_region( mem_region * mr )
{
    if( mr->do_lock )
        mem_put_lock( & mr->lock );
}

//
// Utility functions
//
static mem_seg * get_mem_seg( mem_region * mr, u64 va, u64 len )
{
    mem_get_lock( & Mem_segs_pool.lock );

    mem_seg * ms = Mem_segs_pool.free;

    if( ms ) {
        Mem_segs_pool.free = ms->next;
        ms->region = mr;
        ms->va  = va;
        ms->len = len;
        ms->private = 0;
        ms->agent = 0xF;
        ms->next = NULL;
    }
    mem_put_lock( & Mem_segs_pool.lock );

    return ms;
}

static void put_mem_seg( mem_seg * ms )
{
    if( ms ) {
        mem_get_lock( & Mem_segs_pool.lock );

        ms->next = Mem_segs_pool.free;
        Mem_segs_pool.free = ms;

        mem_put_lock( & Mem_segs_pool.lock );
    }
}

//
// Initialize the subsystem
//
void mem_init( void )
{
    //
    // Memory entries first
    //
    mem_seg * ms = Mem_segs_pool.pool;

    Mem_segs_pool.free = ms;
    mem_init_lock( & Mem_segs_pool.lock );

    for( ; ms < Mem_segs_pool.pool + (NUM_FREE_SEGS - 1)  ; ms++ ) {
        ms->next = ms + 1;
    }
    ms->next = NULL;
    //
    // now regions
    //
    mem_region * mr = Mem_regions_pool.pool;

    Mem_regions_pool.free = mr;
    mem_init_lock( & Mem_regions_pool.lock );

    for( ; mr < Mem_regions_pool.pool + (NUM_FREE_REGIONS - 1)  ; mr++ ) {
        mr->next = mr + 1;
    }
    mr->next = NULL;
}

mem_region * mem_region_create( mem_type type, bool track, bool lock, const char * name )
{
    mem_get_lock( & Mem_regions_pool.lock );

    mem_region * mr = Mem_regions_pool.free;

    if( mr ) {
        Mem_regions_pool.free = mr->next;
        mr->name = name;
        mr->type = type;
        mr->track = track;
        mr->do_lock = lock;
        mr->free  = NULL;
        mr->alloc = NULL;
        mr->next  = NULL;
        mr->base_addr = 0UL;

        mem_init_lock( & mr->lock );
    }
    mem_put_lock( & Mem_regions_pool.lock );

    return mr;
}

void mem_region_destroy( mem_region *mr )
{
    mem_lock_region( mr );
    mem_get_lock( & Mem_regions_pool.lock );

    while( mr->free ) {
        mem_seg * ms = mr->free;
        mr->free = ms->next;
        put_mem_seg(ms);
    }
    while( mr->alloc ) {
        mem_seg * ms = mr->alloc;
        mr->free = ms->next;
        put_mem_seg(ms);
    }
    mr->next = Mem_regions_pool.free;
    Mem_regions_pool.free = mr;

    mem_put_lock( & Mem_regions_pool.lock );
    mem_unlock_region( mr );
}

const char * mem_region_name( mem_region * mr )
{
    return mr->name;
}

mem_type mem_region_type( mem_region * mr )
{
    return mr->type;
}

//
// Add a segment 'len' long at 'va'
// Returns 1 on error
//
int mem_region_add( mem_region * mr, u64 va, u64 len )
{
    mem_lock_region( mr );

    bool track = mr->track;
    mr->track = 0;
    //
    // we don't track 'adds'
    //
    int ret = mem_free( get_mem_seg( mr, va, len ) );

    mr->track = track;
    mem_unlock_region( mr );

    return ret;
}

//
// Remove a seg from a list of same
//
int mem_remove( mem_seg ** head, mem_seg * seg )
{
    mem_seg ** pp = head;

    for( ; *pp ; pp = & (*pp)->next ) {
        if( *pp == seg ) {
            *pp = seg->next;
            seg->next = NULL;
            return 1;
        }
    }
    return 0;   // not found
}

//
// Return a segment 'ms' to the region
//
int mem_free( mem_seg * ms )
{
    int status = 0;

    if( ms == NULL )
        return 1;

    mem_region * mr = ms->region;
    mem_lock_region( mr );
    //
    // if enabled, remove the seg from our tracking list
    //
    if( mr->track && mem_remove( & mr->alloc, ms ) != 1 )
        ce_error("MEM", "mem_free: segment not found in %s alloc list\n", mr->name );
    //
    // We iterate through the va sorted free region, looking for either
    // a mem_seg to merge with (if adjacent) or a 'slot' to insert into.
    // Using 'blind man's cane' list traversal.
    //
    mem_seg ** pp = & mr->free;

    u64 va = ms->va;
    u64 len = ms->len;

    for( ; *pp ; pp = & (*pp)->next ) {
        mem_seg * p = *pp;
        //
        // check for violations first - do we intersect/cover this entry?
        //
        if( (va >= p->va && va < p->va + p->len) ||            // starts in middle?
            (va + len > p->va && va + len < p->va + p->len) || // ends in middle?
            (va < p->va && va + len >= p->va + p->len) ) {     // covers?
            ce_error("MEM", "mem_free: range violation\n");
            status = 1;
            goto unlock;
        }
        //
        // determine relationship
        //
        if( va + len == p->va ) {            // comes before, adjacent
            p->va = va;                      // grow the existing
            p->len += len;
            put_mem_seg(ms);                 // free the incoming
            ms->region->frees++;
            status = 0;
            goto unlock;

        } else if( va + len < p->va ) {      // comes before, insert new
            break;

        } else if( va == p->va + p->len ) {  // comes after, adjacent
            p->len += len;                   // grow the existing
            put_mem_seg(ms);                 // free the incoming
            ms->region->frees++;
            status = 0;
            goto unlock;
        }
        // comes after, move to next
    }
    //
    // either we need to create one before the current or we're at the end
    // of the chain (or it's empty). Same difference - insert the incoming
    //
    ms->next = *pp;
    *pp = ms;
    ms->region->frees++;

unlock:
    mem_unlock_region( mr );
    return status;
}

//
// Return a mem_seg to the free pool without freeing the associated segment
// Used when permanently allocating a segment
//
void mem_return( mem_seg * ms )
{
    if( ms == NULL )
        return;
    mem_region * mr = ms->region;

    mem_lock_region( mr );
    //
    // if enabled, remove the seg from our tracking list
    //
    if( mr->track && mem_remove( & mr->alloc, ms ) != 1 )
        ce_error("MEM", "mem_free: segment not found in %s alloc list\n", mr->name );

    mem_unlock_region( mr );

    put_mem_seg(ms);
}

//
// Try to allocate a segment starting at 'va', 'len' long
// We currently assume that these are fixed allocations and not tracked.
//
mem_seg * mem_alloc_at( mem_region * mr, u64 va, u64 len )
{
    mem_lock_region( mr );

    mem_seg ** pp = & mr->free;
    mem_seg *  ms = NULL;
    //
    // Iterate through the (sorted) free list looking for the containing segment
    //
    for( ; *pp ; pp = & (*pp)->next ) {
        mem_seg * p = *pp;

        if( va >= p->va && va < p->va + p->len ) {  // p contains 'va'
            if( va + len > p->va + p->len )         // p doesn't span 'len'
                break;
            if( len == p->len ) {                   // consumes - remove, return
                *pp = p->next;
                ms = p;
                break;

            } else if( va == p->va ) {              // at front, trim
                ms = get_mem_seg( mr, va, len );
                if( ms ) {
                    p->va += len;
                    p->len -= len;
                    mr->allocations++;
                }
                break;

            } else if( va + len == p->va + p->len ) { // at end, trim
                ms = get_mem_seg( mr, va, len );
                if( ms ) {
                    p->len -= len;
                    mr->allocations++;
                }
                break;

            } else {                                  // ugh, in the middle - split
                ms = get_mem_seg( mr, va, len );
                mem_seg * rest = get_mem_seg( mr, va + len,
                                              (p->va + p->len) - (va + len) );
                if( ms && rest ) {
                    p->len = va - p->va;
                    rest->next = p->next;
                    p->next = rest;
                    mr->allocations++;
                } else {
                    put_mem_seg(ms);
                    put_mem_seg(rest);
                }
                break;
            }
        }
    }
    //
    // if tracking, add to alloc list
    //
    if( ms != NULL && mr->track ) {
        ms->next = mr->alloc;
        mr->alloc = ms;
    }
    mem_unlock_region( mr );
    //
    // if NULL - can't find the space (or out of mem_segs)
    //
    return ms;
}

//
// Try to allocate a 'len' long segment
// Returns an offset into the memory or NO_FREE_MEM
//
mem_seg * mem_alloc( mem_region * mr, u64 len )
{
    mem_lock_region( mr );

    mem_seg ** pp = & mr->free;
    mem_seg *  ms = NULL;
    //
    // round len up to 8 bytes
    //
    len = (len + 7) & ~7UL;

    for( ; *pp ; pp = & (*pp)->next ) {
        mem_seg * p = *pp;
        //
        // first fit, clip off head
        //
        if( p->len > len ) {                     // contains request
            ms = get_mem_seg( mr, p->va, len );  // allocate a seg to return
            if( ms ) {
                p->len -= len;
                p->va += len;
                mr->allocations++;
            }
            break;

        } else if( p->len == len ) {    // just contains request
            *pp = p->next;
            p->next = NULL;
            mr->allocations++;
            ms = p;
            break;
        }
    }
    //
    // if tracking, add to alloc list
    //
    if( ms != NULL && mr->track ) {
        ms->next = mr->alloc;
        mr->alloc = ms;
    }
    mem_unlock_region( mr );
    //
    // if NULL - can't find the space (or out of mem_segs)
    //
    return ms;
}

void mem_seg_list_dump( mem_seg * s )
{
    for( ; s ; s = s->next ) {
        ce_print( "MEM", "  0x%016llx, len 0x%lx, %sagent %d\n",
                s->va, s->len, s->private ? "(private) ":"", s->agent );
    }
}

void mem_region_dump( mem_region * mr )
{
    mem_lock_region( mr );

    ce_print("MEM", "Memory region %s at %p\n", mr->name, mr );
    ce_print("MEM", "  %d allocations, %d frees\n", mr->allocations, mr->frees );
    ce_print("MEM", " Free list:\n" );
    mem_seg_list_dump( mr->free );
    if( mr->track ) {
        ce_print("MEM", " Allocated list:\n" );
        mem_seg_list_dump( mr->alloc );
    }
    mem_unlock_region( mr );
}
