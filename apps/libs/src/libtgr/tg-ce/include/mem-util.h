#ifndef __MEM_UTIL_H__
#define __MEM_UTIL_H__

//
// Methods to write to TG addresses not directly addressable in the CE
//
uint64_t tg_memcpy( uint64_t dstAddr, uint64_t srcAddr, size_t len );
void   tg_zero( uint64_t dstAddr, size_t len );
//
// Memory related functions to validate an address range
// These return either a CE valid IPM or CR address or 0 (if failure)
//
uint64_t validate_xe_addr( xe_info * xei, uint64_t addr, size_t len );
uint64_t validate_addr( block_info * bi, uint64_t addr, size_t len );

//
// Memory allocation helpers
//
mem_type mem_type_of( uint64_t addr );

mem_seg * xe_alloc_mem( xe_info * xei, mem_type type, size_t len );
mem_seg * xe_alloc_mem_at( xe_info * xei, mem_type type, uint64_t addr, size_t len );

mem_seg * block_alloc_mem( block_info * bi, mem_type type, size_t len, bool private, int agent );
mem_seg * block_alloc_mem_at( block_info * bi, mem_type type, uint64_t addr, size_t len );

mem_seg * global_alloc_mem( ce_info * ci, size_t len, bool private, int agent );
mem_seg * global_alloc_mem_at( ce_info * ci, uint64_t addr, size_t len );

#endif // __MEM_UTIL_H__
