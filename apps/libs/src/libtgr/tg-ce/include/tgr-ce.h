#ifndef __TGR_CE_H__
#define __TGR_CE_H__

#include <stdint.h>
#include <stddef.h>

#include "memory.h"
//
// This file contains the main types and objects of the TG baremetal run-time
//
// A simplified TG entity identifier (borrowed from fsim)
// Used for XE Pids, block ids, cluster id, ...
// Uses anonymous unions
//
typedef union
{
    struct {
      int64_t agent:8,    // agent index in block
              block:8,    // block index in cluster
              cluster:8,  // cluster index on die
              socket:8,   // socket index of 8-sockets within hypercube
              cube:8,     // subset hypercube index within rack
              rack:8,     // physical rack in machine room
              unused:16;  // placeholder, maybe used for seq# in pids?
    };
    uint64_t all;
} id_tuple;

//
// Memory layout:
//
// Forwards
//
typedef struct xe_info xe_info;
typedef struct block_info block_info;
typedef struct ce_info ce_info;

typedef struct ce_config ce_config;

#define MAX_XE_COUNT    8

//
// The run-time's concept of XE run state
//
typedef enum xe_state {
    XE_UNUSED = 0,  // Ready to be (re)used to run a program.
    XE_RUNNING,     // Currently running a program (could be suspended).
    XE_FINISHED,    // Finished running program, but not yet cleaned up.
    XE_TERMINATED,  // Permanently terminated.
} xe_state;
//
// The block info structure contains an xe_info structure for each block XE
// It holds the XE's identity, run state, and resource allocations.
//
struct xe_info
{
    block_info * block;      // block we belong to
    id_tuple  id;            // XE identifier
    uint64_t  stack_top;     // VA for this XE's stack top
    mem_seg * stack;         // CE allocated stack segment (or NULL)
    uint32_t  allocations;   // count of memory allocations made
    uint32_t  frees;         // count of memory frees
    //
    // Run state and other state mgmt variables
    //
    xe_state  use_state;     // whether this XE is being used to run something.
    char      detached;      // whether this XE will be reaped on finish.
    char      blocked;       // whether this XE is in a blocked request (alarm)
                             // an XE can block if:
                             // - it does a suspend
                             // - it waits on another XE
                             // - it waits on all running XEs
                             // - it's finished, but hasn't been reaped yet
    int       resumes;       // Number of queued resumes,
                             // -1 => blocked and suspended
    //
    // An XE can wait for another XE to finish and the waiter and waitee
    // maintain pointers to each other.
    //
    xe_info * waiting_xei;    // Set if someone waiting for me
    xe_info * waiting_for;    // Set to victim if a waiting
    void *    wait_req_addr;  // address of waiter's req to complete
    uint64_t  exit_code;      // Finished XE's exit code returned to waiter
    //
    // (Abs time) when to stop suspending. Only valid if resumes == -1.
    // If tv_sec is ((time_t)-1), then never stop.
    //
    struct timespec suspend_time;
    //
    // memory allocation support
    //
    mem_region * L1;
};

#define XE_NUM(xei)  ((xei)->id.agent - 1)   // XE index from agent number

//
// Each CE maintains a block_info structure describing the resources for its
// block, both static and dynamic.
//

struct block_info
{
    id_tuple     id;        // block in cluster
    ce_info *    ce;        // who manages us
    mem_region * L2;        // manages the block L2
    bool         load_text; // when loading the ELF into memory, load text as well
    uint64_t     entry;     // ELF defined entry point address
    //
    // XE info
    //
    xe_info * xes;           // state info on our XEs
    int       xe_count;      // configured XEs / block
    int       running;       // count of running XEs (?)
    bool      doing_waitall; // the primary XE is doing a waitall
    bool      suspending;    // set if any XE is suspending
    volatile uint64_t alarmed;  // XE has unsatisfied alarm, XE num = byte offset
};
//
// Per XE alarm occurred bit in 'alarmed'
//
#define xe_alarmed_bit(xe) (1UL << (xe))

__attribute__((always_inline))
inline xe_info * xe_of_block( block_info * bi, int xe )
{
    return bi->xes + xe;
}

//
// This represents the memory of the CE that manages one or more blocks
//
struct ce_info
{
    ce_config *  config;      // overall configuration
    id_tuple     id;          // cluster
    mem_region * CE;          // manages the CE's slice memory
    block_info * blocks;      // block_info array for this CE
    int          block_count;
    volatile uint64_t running;  // number of blocks still running
};

__attribute__((always_inline))
inline block_info * block_of_ce( ce_info * ci, int block )
{
    return ci->blocks + block;
}

__attribute__((always_inline))
inline xe_info * xe_of_ce( ce_info * ci, int block, int xe )
{
    return ci->blocks[block].xes + xe;
}
//
// Currently each CE has a statically assigned region of IPM that is represented
// by this structure. This should be expanded to a scheme where the region(s) are
// requested from a central agent that manages IPM usage.
//
typedef struct
{
    mem_region * IPM;
} global_info;

//
//
// Main config for CE to cluster mapping
//
typedef enum {
    CE_MODE_BLOCK,      // single core, single block - FSIM default
    CE_MODE_CLUSTER     // multi-core, cluster multi-block
} ce_mode;

struct ce_config {
    ce_mode  mode;        // How we map onto the HW
    int      core_count;  // how many cores the CE has
    id_tuple cluster_id;
    uint64_t IPM_base;    // Start of managed IPM
    uint64_t IPM_size;    // size in bytes
    uint64_t L2_size;     // size in bytes
    uint64_t L1_size;     // XE only, CE has no L1, in bytes
    int xes_per_block;
    int block_start_mode; // 0 start all XEs in a block, 1 start master (XE0) only
    int block_load_mode;  // 0 load each block independently, 1 load global once
    int xe_stack_size;    // size for CE allocated XE stacks
    uint64_t ce_mem_base; // start x86 address of CE memory
    uint64_t ce_mem_size; // size of CE memory
    ce_info     ce;       // allocated based on mode, slice, and core counts
    block_info *blocks;   // allocated based on mode
    global_info global;   // manages IPM region
    char * xe_elffile;    // Pointer to the XE ELF filename in the configarea
};

//
// Method provided by tgr-config to return a configuration specific
// initialized ce_config object
//
ce_config * tgr_config_init( void );
//
// Method provided by tgr-config to reserve the CE core stack space
// and other memory.
//
int tgr_reserve_ce_memory( ce_info * cei );

//
// Allocate/free memory from block memory - CE private or L2
//
void block_free( mem_seg * seg );

//
// Allocate/free memory from global memory - IPM
//
void global_free( mem_seg * seg );

//
// get the xe_info with id 'pid'
//
xe_info * xe_get_info_by_id( block_info *bi, id_tuple id );
//
// Get the xe'th xe info
//
xe_info * xe_get_info_by_index( block_info * bi, int xe );
//
// iterate over xe_infos
//
xe_info * xe_get_next_info( block_info * bi, xe_info * xei );
//
// iterate over xe_infos by state
//
xe_info * xe_get_info_by_state( block_info * bi, xe_info * xei, xe_state state );

//
// XE low level utility functions
//
void xe_set_msr( xe_info * xei, int regno, uint64_t value );
void xe_set_pc( xe_info * xei, uint64_t value );

uint64_t xe_get_reg( xe_info * xei, int regno );
void     xe_set_reg( xe_info * xei, int regno, uint64_t value );

#define xe_set_ra( xei, v ) xe_set_reg( xei, 511, v )
#define xe_set_sp( xei, v ) xe_set_reg( xei, 509, v )

void xe_stop( xe_info * xei );
void xe_continue( xe_info * xei );
void xe_terminate( xe_info * xei );

//
// Restart a suspended XE
//
void xe_resume( xe_info * xei, int status );
//
// Truly terminate an XE (not return to the thread pool)
//
void xe_terminated( xe_info * xei );

// int tgr_init( ce_config * config, ce_info *cei );
//
// In tgr-run.c
//
int tgr_block_run( void * arg );

#endif // __TGR_CE_H__
