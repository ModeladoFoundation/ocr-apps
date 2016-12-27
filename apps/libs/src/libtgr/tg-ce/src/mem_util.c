#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

#include "tg-types.h"
#include "tg-cmdline.h"
#include "tg-console.h"
#include "tg-mmio.h"

#include "memory.h"
#include "tgr-ce.h"
#include "util.h"

#include "xstg-map.h"

#define IS_MR(a)     (((a) >> MAP_MACHINE_SHIFT) == 0b0001)
#define IS_SR(a)     (((a) >> MAP_SOCKET_SHIFT) == 1)
#define IS_CR(a)     (((a) >> MAP_CLUSTER_SHIFT) == 1)
#define BR_TO_CR(b,a) (_CR_LEAD_ONE | \
                        (TO_ULL(b) << MAP_BLOCK_SHIFT) | \
                        ((a) & (_BR_LEAD_ONE-1)))
#define SR_TO_CR(a)  (_CR_LEAD_ONE | ((a) & (_CR_LEAD_ONE-1)))
#define IS_BR(a)     (((a) >> MAP_BLOCK_SHIFT) == 1)
#define IS_AR(a)     (((a) >> MAP_AGENT_SHIFT) == 1)
#define AR_TO_CR(b,a,v) (_CR_LEAD_ONE | \
                        (TO_ULL(b) << MAP_BLOCK_SHIFT) | \
                        (TO_ULL(a) << MAP_AGENT_SHIFT) | \
                        ((v) & (_AR_LEAD_ONE-1)))

#define IS_IPM(a)    ((((a) >> MAP_IN_SOCKET_IPM_SHIFT) & 0b111) == MAP_IN_SOCKET_IPM_MAGIC)
#define IS_OPM(a)    ((((a) >> MAP_IN_SOCKET_DRAM_SHIFT) & 0b11))
#define IPM_OFFSET(a) ((a) & ((1UL << MAP_IN_SOCKET_IPM_SHIFT) - 1))
#define IS_L3(a)     ((((a) >> MAP_IN_CLUSTER_L3_SHIFT) & 1) == MAP_IN_CLUSTER_L3_MAGIC)
#define IS_L2(a)     ((((a) >> MAP_IN_BLOCK_L2_SHIFT) & 1) == MAP_IN_BLOCK_L2_MAGIC)
#define L2_OFFSET(a) ((a) & ((1UL << MAP_IN_BLOCK_L2_SHIFT) - 1))
#define IS_L1(a)     ((((a) >> MAP_IN_AGENT_L1_SHIFT) & 1) == MAP_IN_AGENT_L1_MAGIC)
#define L1_OFFSET(a) ((a) & ((1UL << MAP_IN_AGENT_L1_SHIFT) - 1))

#define NO_ADDR (0UL)
//
// This is used to validate addresses from XEs since they may be using
// agent relative addressing
//
uint64_t validate_xe_addr( xe_info * xei, uint64_t addr, size_t len )
{
    addr = (uint64_t) validate_addr( xei->block, addr, len );

    if( addr != 0UL && IS_AR(addr) )
        return AR_TO_CR(xei->block->id.block, xei->id.agent, addr);

    return addr;
}
//
// Validate that the address is in MR format and the whole range is legal
// i.e., wholely contained in a defined memory
// Returns an SR version of the address if IPM, a CR version otherwise,
// except in the case where addr is an AR address where we return AR
// because we can't cvt to CR as we don't know the agent number to use.
//
uint64_t validate_addr( block_info * bi, uint64_t addr, size_t len )
{
    uint64_t offset;
    ce_config * config = bi->ce->config;
    //
    // Our direct address map is only this socket, but, be pedantic ...
    //
    if( IS_MR(addr) &&
        ( RACK_FROM_ID(addr) >= bi->id.rack ||
          CUBE_FROM_ID(addr) >= bi->id.cube ||
          SOCKET_FROM_ID(addr) >= bi->id.socket ) ) {
            printf("TGR-VA: bad MR addr\n");
            return NO_ADDR;
    }
    //
    // convert to socket (for IPM) or cluster relative (everything else)
    //
    if( IS_MR(addr) || IS_SR(addr) ) {
        if( IS_IPM(addr) ) {
            offset = IPM_OFFSET(addr);
            if( offset > config->IPM_size || offset + len > config->IPM_size ){
                printf("TGR-VA: bad IPM addr or len\n");
                return NO_ADDR;
            }
            //
            // Convert to SR IPM address and return
            //
            return (SR_IPM_BASE + IPM_OFFSET(addr));
        //
        // Other Socket memory are illegal.
        // XXX Should we force our cluster only ??
        //
        } else if( IS_OPM(addr) ) {
            printf("TGR-VA: illegal OPM addr\n");
            return NO_ADDR;
        //
        // Handled all socket resources above, must be cluster resource
        //
        }
        if( CLUSTER_FROM_ID(addr) != bi->id.cluster ) {
            printf("TGR-VA: illegal cluster number\n");
            return NO_ADDR;
        }
        addr = SR_TO_CR(addr);
    //
    // convert a block relative to cluster relative
    //
    } else if( IS_BR(addr) ) {
        addr = BR_TO_CR(bi->id.block, addr);
    //
    // agent relative is an issue in that we don't know the agent here
    // and so can't validate some stuff, or translate to CR.
    // Just check for L1 and return it if legal
    //
    } else if( IS_AR(addr) ) {
        if( IS_L1(addr) ) {
            offset = L1_OFFSET(addr);
            if( offset > config->L1_size || offset + len > config->L1_size ) {
                printf("TGR-VA: bad L1 addr or len\n");
                return NO_ADDR;
            }
            return addr;
        }
    }
    //
    // We now only have CR addresses
    //
    if( IS_L3(addr) || BLOCK_FROM_ID(addr) >= BlockCount ) {
        printf("TGR-VA: bad cluster/block/L3\n");
        return NO_ADDR;

    } else if( IS_L2(addr) ) {
        offset = L2_OFFSET(addr);
        if( offset > config->L2_size || offset + len > config->L2_size ) {
            printf("TGR-VA: bad L2 addr or len\n");
            return NO_ADDR;
        }

    //
    // No CE L1 access, so only agents >= 1
    //
    } else if( AGENT_FROM_ID(addr) < 1 || AGENT_FROM_ID(addr) - 1 >= XeCount ) {
        printf("TGR-VA: bad CR XE agent number\n");
        return NO_ADDR;
    //
    // Only allow L1 (no regs)
    //
    } else if( IS_L1(addr) ) {
        offset = L1_OFFSET(addr);
        if( offset > Tgr_config->L1_size || offset + len > Tgr_config->L1_size ) {
            printf("TGR-VA: bad CR L1 addr or len\n");
            return NO_ADDR;
        }
    }
    return addr;
}

mem_type mem_type_of( uint64_t addr )
{
    if( IS_IPM(addr) )
        return Mem_IPM;
    else if( IS_L2(addr) )
        return Mem_L2;
    else if( IS_L1(addr) )
        return Mem_L1;
    return Mem_None;
}


//
// Copy from srcAddr to (u64 aligned) dstAddr, maintaining the srcAddr
// alignment. Returns pointer to first copied byte
// Full address space capable
//
uint64_t tg_memcpy( uint64_t dstAddr, uint64_t srcAddr, size_t len )
{
    u64 pre = srcAddr & (sizeof(srcAddr) - 1);

    len = (len + pre + (sizeof(u64) - 1)) & ~ (s64)(sizeof(u64) - 1);

    tg_dma_copyregion_async( (void *)dstAddr, (void *)(srcAddr - pre), len );
    tg_fence_fbm();

    return dstAddr + pre;
}

//
// Zero memory - full address space capable
//
void tg_zero( uint64_t dstAddr, size_t len )
{
    //
    // get to a 64bit aligned addr
    //
    if(len > 0 && dstAddr & 1 ) {
        tg_st8( (u8 *) dstAddr, 0x0);
        dstAddr += 1;
        len -= 1;
    }
    if(len > 0 && dstAddr & 2 ) {
        tg_st16( (u16 *) dstAddr, 0x0);
        dstAddr += 2;
        len -= 2;
    }
    if(len > 0 && dstAddr & 4 ) {
        tg_st32( (u32 *) dstAddr, 0x0);
        dstAddr += 4;
        len -= 4;
    }
    //
    // Complete the process
    //
    while( len > 0 ) {
        int inc;
        if(len >= 8) {
            tg_st64( (u64 *) dstAddr, 0x0);
            inc = 8;

        } else if(len >= 4) {
            tg_st32( (u32 *) dstAddr, 0x0);
            inc = 4;

        } else if(len >= 2) {
            tg_st16( (u16 *) dstAddr, 0x0);
            inc = 2;

        } else {
            tg_st8( (u8 *) dstAddr, 0x0);
            inc = 1;
        }
        dstAddr += inc;
        len -= inc;
    }
}

///////////////////////////// Memory allocation helpers ////////////////////////
//
//
mem_seg * xe_alloc_mem( xe_info * xei, mem_type type, size_t len )
{
    // XXX
    mem_seg * seg = NULL;

    switch( type ) {
      case Mem_ANY:
        if( (seg = mem_alloc( xei->L1, len )) ||
            (seg = block_alloc_mem( xei->block, Mem_L2, len, 0, xei->id.agent )) ||
            (seg = block_alloc_mem( xei->block, Mem_IPM, len, 0, xei->id.agent )) ) {
            seg->agent = xei->id.agent;
            return seg;
        } else
            return NULL;

      case Mem_L1:
        seg = mem_alloc( xei->L1, len );
        break;

      case Mem_L2:
        seg = block_alloc_mem( xei->block, type, len, 0, xei->id.agent );
        break;

      case Mem_IPM:
        seg = block_alloc_mem( xei->block, type, len, 0, xei->id.agent );
        break;

      default:
        break;
    }
    //
    // if private, stash in the xe_info
    //
    if( seg )
        seg->agent = xei->id.agent;
    return seg;
}

mem_seg * xe_alloc_mem_at( xe_info * xei, mem_type type, uint64_t addr, size_t len )
{
    switch( type ) {
      case Mem_L1:
        return mem_alloc_at( xei->L1, addr, len );

      case Mem_L2:
      case Mem_IPM:
        return block_alloc_mem_at( xei->block, type, addr, len );

      default:
        break;
    }
    return NULL;
}

//
// The name of this is kind of a misnomer as it's used to allocate
// block, CE, and global memory
//
mem_seg * block_alloc_mem( block_info * bi, mem_type type, size_t len, bool private, int agent )
{
    mem_region * mr = NULL;
    mem_seg *    seg = NULL;

    switch( type ) {
      case Mem_L2 :
          mr = bi->L2;
          break;
      case Mem_CE :
          mr = bi->ce->CE;
          break;
      case Mem_IPM :
          return global_alloc_mem( bi->ce, len, private, agent );
          break;
      default:
          printf("block_alloc: unsupported memory type %d\n", type );
          break;
    }
    if( mr ) {
        seg = mem_alloc( mr, len );
        if( seg != NULL ) {
            seg->private = private;
            seg->agent = agent;
        }
    }
    return seg;
}

mem_seg * block_alloc_mem_at( block_info * bi, mem_type type, uint64_t addr, size_t len )
{
    switch( type ) {
      case Mem_L2:
        return mem_alloc_at( bi->L2, addr, len );

      case Mem_IPM:
        return global_alloc_mem_at( bi->ce, addr, len );

      default:
        break;
    }
    return NULL;
}

mem_seg * global_alloc_mem( ce_info * cei, size_t len, bool private, int agent )
{
    mem_seg * seg = mem_alloc( cei->config->global.IPM, len );

    if( seg ) {
        seg->private = private;
        seg->agent = agent;
    }
    return seg;
}

mem_seg * global_alloc_mem_at( ce_info * cei, uint64_t addr, size_t len )
{
    return mem_alloc_at( cei->config->global.IPM, addr, len );
}

//
// An XE is giving back an allocation. Search for it in the private
// stash and then in the per region lists.
//
int return_mem( xe_info * xei, uint64_t va )
{
    // mem_seg * seg;
    //
    // XXX
    // look first in the XE's private stash
    //
    //
    // Now by type
    //
    switch( mem_type_of(va) ) {
      case Mem_L1:
      case Mem_L2:
      case Mem_IPM:
      default:
        break;
    }
    return 0;
}
