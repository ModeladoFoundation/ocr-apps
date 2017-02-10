#ifndef __MEM_UTIL_H__
#define __MEM_UTIL_H__

#include "xstg-map.h"

#define SR_TO_MR(r,u,s,a)  (_MR_LEAD_ONE | \
                            (TO_ULL(r) << MAP_RACK_SHIFT)   | \
                            (TO_ULL(u) << MAP_CUBE_SHIFT)   | \
                            (TO_ULL(s) << MAP_SOCKET_SHIFT) | \
                            ((a) & (_SR_LEAD_ONE-1)))

#define BR_TO_CR(b,a)    (_CR_LEAD_ONE | \
                          (TO_ULL(b) << MAP_BLOCK_SHIFT) | \
                          ((a) & (_BR_LEAD_ONE-1)))

#define SR_TO_CR(a)      (_CR_LEAD_ONE | ((a) & (_CR_LEAD_ONE-1)))

#define AR_TO_CR(b,a,v)  (_CR_LEAD_ONE | \
                          (TO_ULL(b) << MAP_BLOCK_SHIFT) | \
                          (TO_ULL(a) << MAP_AGENT_SHIFT) | \
                          ((v) & (_AR_LEAD_ONE-1)))

#define IS_MR(a)         (((a) >> MAP_MACHINE_SHIFT) == 0b0001)
#define IS_SR(a)         (((a) >> MAP_SOCKET_SHIFT) == 1)
#define IS_CR(a)         (((a) >> MAP_CLUSTER_SHIFT) == 1)
#define IS_BR(a)         (((a) >> MAP_BLOCK_SHIFT) == 1)
#define IS_AR(a)         (((a) >> MAP_AGENT_SHIFT) == 1)

#define IS_IPM(a)    ((((a) >> MAP_IN_SOCKET_IPM_SHIFT) & 0b111) == MAP_IN_SOCKET_IPM_MAGIC)
#define IS_OPM(a)    ((((a) >> MAP_IN_SOCKET_DRAM_SHIFT) & 0b11))
#define IS_L3(a)     ((((a) >> MAP_IN_CLUSTER_L3_SHIFT) & 1) == MAP_IN_CLUSTER_L3_MAGIC)
#define IS_L2(a)     ((((a) >> MAP_IN_BLOCK_L2_SHIFT) & 1) == MAP_IN_BLOCK_L2_MAGIC)
#define IS_L1(a)     ((((a) >> MAP_IN_AGENT_L1_SHIFT) & 1) == MAP_IN_AGENT_L1_MAGIC)

#define IPM_OFFSET(a) ((a) & ((1UL << MAP_IN_SOCKET_IPM_SHIFT) - 1))
#define L2_OFFSET(a) ((a) & ((1UL << MAP_IN_BLOCK_L2_SHIFT) - 1))
#define L1_OFFSET(a) ((a) & ((1UL << MAP_IN_AGENT_L1_SHIFT) - 1))
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
//
// Read in a file to CE memory
//
mem_seg * tgr_load_file( ce_info * cei, const char * file );

#endif // __MEM_UTIL_H__
