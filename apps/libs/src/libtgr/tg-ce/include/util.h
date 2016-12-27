#ifndef __UTIL_H__
#define __UTIL_H__

#define MIN(x,y) ((x) < (y) ? (x) : (y))
//
// Compare timespecs
//
#define EARLIER(a, b) ((a).tv_sec < (b).tv_sec || \
                       ((a).tv_sec == (b).tv_sec && \
                        (a).tv_nsec < (b).tv_nsec))

void ce_gettime( struct timespec *ts );

size_t strlen( const char *s );
char *strncat( char *dest, const char *src, size_t n );

void * memcpy( void *d, const void *s, size_t n );
void * memset( void *d, int v, size_t n );
//
// in mem_util.c
//
uint64_t tg_memcpy( uint64_t dstAddr, uint64_t srcAddr, size_t len );
void   tg_zero( uint64_t dstAddr, size_t len );
//
// Memory related functions to validate an address range
//
uint64_t validate_xe_addr( xe_info * xei, uint64_t addr, size_t len );
uint64_t validate_addr( block_info * bi, uint64_t addr, size_t len );

mem_type mem_type_of( uint64_t addr );

//
// Memory allocation helpers
//
mem_seg * xe_alloc_mem( xe_info * xei, mem_type type, size_t len );
mem_seg * xe_alloc_mem_at( xe_info * xei, mem_type type, uint64_t addr, size_t len );

mem_seg * block_alloc_mem( block_info * bi, mem_type type, size_t len, bool private, int agent );
mem_seg * block_alloc_mem_at( block_info * bi, mem_type type, uint64_t addr, size_t len );

mem_seg * global_alloc_mem( ce_info * ci, size_t len, bool private, int agent );
mem_seg * global_alloc_mem_at( ce_info * ci, uint64_t addr, size_t len );

#endif // __UTIL_H__
