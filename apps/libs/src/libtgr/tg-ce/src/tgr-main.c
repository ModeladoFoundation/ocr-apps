#include <stdint.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <tg-types.h>
#include <tg-cmdline.h>
#include <tg-console.h>
#include <tg-remap.h>
#include <xstg-map.h>
#include <mmio-table.h>

#include "tgr-ce.h"
#include "tgr-alarms.h"
#include "ce-xe-intf.h"
#include "ce-os-svc.h"
#include "xe-elf64.h"
#include "util.h"
#include "mem-util.h"

#define __ASM__
#include <fsim-api.h>
#undef __ASM__

//
// halt the CE core to wait for an event
//
#define intr_wait()  do {__asm__ __volatile__("hlt;"); } while(0)

#define MAX_BLOCKS_PER_CLUSTER  (4) // anticipating HW

//
////////////////////////////////////////////////////////////////////////////////
//
// Main entry from tgkrnl
//  At the point we enter tgr_main(), tgkrnl has parsed the cmd line info and
//  the XE args created (including the XE ELF file name).
//
// We initialize our data structures from tgkrnl parsed values.
// Includes ce_info, block_info(s) and their xe_info array.
// Memory init creates region objects.
// Wire ourselves into the XE alarm handling
//  Register our IRQ handler to get called on all SW alarms
//  If a handled alarm, this will save the XE/alarm info and return keeping
//  the XEs stopped. We may want to create our own set of alarm values to avoid
//  interfering with the already defined ones.
// Load the XE ELF exe (read in from the exe file)
// We start all the cores running their 'main loop' and when the core 0 loop
// returns it waits for the rest of the cores to finish.
//
// Main loop
//  This is implemented in tgr-run.c:tgr_block_run()
//  - Before entering the run loop it starts the indicated XEs (1 or all)
//  The loop waits (halted) for XE message arrival or clock ticks (for XE
//  suspend timer use).
//  - Check for MSG alarms and processes them
//  - Check for expired XE suspend timers and resumes those that are.
//  - Loop terminates when no XEs in the block are running
//
// Multi-block boot
//  Currently we only support loading that same XE binary on all blocks in the
//  cluster/tile as we don't have a mechanism to get per-block XE exe filenames.
//  There's an ini file parser implemented for tgr-ce, but no define config file
//  format.
//
// We currently support only 2 configuration scenarios:
//  Mode BLOCK
//      1 CE core managing 1 block, 1 ce_info has own IPM memory slice (FSIM)
//
//  Mode CLUSTER, 4 blocks, 1 slice
//      4 cores (quad-core CE), 1 ce_info referencing all cluster blocks,
//      each core manages a single block. The core's managed block's interrupts
//      mapped to the core's local APIC.
//
// Questions:
// - How do we get per-cluster/block config info?
//      Place cmdline/config data at start of cluster block 0 L2 ?
//      Provide cfg file name on via cmdline arg?
//
// Possible cfg file contents:
//  - per block exe filename to load
//  - per block XE arguments
//  - per block XE stack size
//  - start mode


////////////////////////////////////////////////////////////////////////////////
// Test scaffolding support
// XXX The scaffolded and FSIM tests haven't been brought over so the following
// test support could probably be removed
//
#ifndef SCAFFOLD_TEST
extern u64 _end;    // linker added symbol marking the last byte of the tgkrnl image.
#define CE_PROG_END    ((uint64_t) (&_end))

#else  // SCAFFOLD_TEST

#include "tgr-test-cfg.h"

//
// for test scaffolding purposes, the test env defines a memory
// region we can allocate from, spoofing the CE mem_area
// We pick a random size to 'reserve' for our code
//
extern test_cfg * Test_cfg;

#undef CE_PROG_BASE
#undef CE_SLICE_SIZE

#define CE_PROG_BASE   ((uint64_t) Test_cfg->slice)
#define CE_SLICE_SIZE  Test_cfg->slice_size
#define CE_PROG_END    ((uint64_t) (Test_cfg->slice + 0x1000))
//
// override awkward XE and CE register addresses
//
#undef BR_PRF_BASE
#define BR_PRF_BASE(n)    (Test_cfg->regs)

#undef BR_MSR_BASE
#define BR_MSR_BASE(n)    (Test_cfg->regs + 512)

#undef BR_XE_CONTROL
#define BR_XE_CONTROL(n)  (((uint8_t *)& Test_cfg->xe_ctrl) + (n))

#endif // SCAFFOLD_TEST
//
// For tests under FSIM
//
#ifdef FSIM_TEST
extern int tgr_test_main();

#define TGR_TEST_MAIN { return tgr_test_main(); }

#else  // !FSIM_TEST

#define TGR_TEST_MAIN

#endif // FSIM_TEST

/////////////////////////////// XE utility methods /////////////////////////////
//
// We map ids to XE index, validate it's this block and a valid XE
//
xe_info * xe_get_info_by_id( block_info * bi, id_tuple id )
{
    id_tuple blkid = id;
    blkid.agent = 0;

    if( blkid.all == bi->id.all && id.agent >= 0 && id.agent < bi->xe_count )
        return bi->xes + (id.agent - 1);  // cvt agent to XE index
    else
        return NULL;
}

xe_info * xe_get_info_by_index( block_info * bi, int xe )
{
    return (xe >= 0 && xe < bi->xe_count) ? bi->xes + xe : NULL;
}

//
// General iterator over a block's XE infos
// xei == NULL returns the first one
//
xe_info * xe_get_next_info( block_info * bi, xe_info * xei )
{
    //
    // Find (the next) XE
    //
    if( xei == NULL )
        return bi->xes;

    else if( ++xei - bi->xes < bi->xe_count  )
        return xei;
    else
        return NULL;
}

//
// xe_info iterator by state
// provide either NULL (starting state) or the previously returned xe_info
// to continue
//
xe_info * xe_get_info_by_state( block_info * bi, xe_info * xei, xe_state state )
{
    //
    // Find (the next) XE with a particular state
    //
    xei = xei ? xei + 1 : bi->xes;

    for( ; xei < bi->xes + bi->xe_count ; xei++ ) {
        if( xei->use_state == state ) {
            return xei;
        }
    }
    return NULL;
}

/////////////////////////////// XE state mgmt methods //////////////////////////
//
// Restart a suspended XE
//
void xe_resume( xe_info * xei, int status )
{
    xei->blocked = false;
    xei->resumes = 0;
    xe_set_reg( xei, XE_ARG0_REG, status );
    xe_continue( xei );
}

//
// An event occurred that terminated an XE
// mark the appropriate states
//
void xe_terminated( xe_info * xei )
{
    if( xei->use_state != XE_TERMINATED ) {
        if( xei->use_state == XE_RUNNING )
            xei->block->running--;

        xei->use_state = XE_TERMINATED;

        ce_vprint("MAIN", "terminated xe 0x%lx, running = %d\n",
                xei->id.all, xei->block->running);

        xe_terminate( xei );
    }
}
//
// When we have a fatal failure, we could end more gracefully if
// we were to terminate all the XEs before bailing. This walks
// all the blocks and forcefully terminates their XEs
//
void xe_terminate_all( ce_info * cei )
{
    for( block_info *bi = cei->blocks ; bi < cei->blocks + cei->block_count ; bi++ ) {
        for( xe_info *xei = bi->xes ; xei < bi->xes + bi->xe_count ; xei++ ) {
            xe_terminate( xei );
        }
    }
}
//
// Power gate an XE which will terminate it's operation
// In Fsim this will set core done
//
// Since we may be in a scenario where we have a single CE controlling
// multiple blocks, the addressing here probably needs to be CR
//
void xe_terminate( xe_info * xei )
{
    volatile u8 * xe_control = (volatile u8 *) CR_XE_CONTROL(xei->block->id.block,
                                                             xei->id.agent-1);

    *xe_control = XE_CTL_PWR_GATE;
}

void xe_stop( xe_info * xei )
{
    volatile u8 * xe_control = (volatile u8 *) CR_XE_CONTROL(xei->block->id.block,
                                                             xei->id.agent-1);

    if( (*xe_control & XE_CTL_CLK_GATE) == 0 )
        *xe_control = XE_CTL_CLK_GATE;
}

void xe_continue( xe_info * xei )
{
    volatile u8 * xe_control = (volatile u8 *) CR_XE_CONTROL(xei->block->id.block,
                                                             xei->id.agent-1);

    *xe_control = 0x00;
}

uint64_t xe_get_reg( xe_info * xei, int reg )
{
    uint64_t * regp = (uint64_t *) CR_PRF_BASE(xei->block->id.block, xei->id.agent) + reg;
    return *regp;
}

void xe_set_reg( xe_info * xei, int reg, uint64_t value )
{
    uint64_t * regp = (uint64_t *) CR_PRF_BASE(xei->block->id.block, xei->id.agent) + reg;
    *regp = value;
}

void xe_set_pc( xe_info * xei, uint64_t value )
{
    xe_set_msr( xei, CURRENT_PC, value );
}

void xe_set_msr( xe_info * xei, int reg, uint64_t value )
{
    uint64_t * regp = (uint64_t *) CR_MSR_BASE(xei->block->id.block, xei->id.agent) + reg;
    *regp = value;
}

/////////////////////////// tgr-ce initialization methods //////////////////////
//
// block_info init
//
int block_info_init( block_info * bi, int block, ce_config *config )
{
    bi->id = bi->ce->id;
    bi->id.block = block;
    bi->running = 0;
    bi->suspending = false;
    bi->alarmed = 0;
    bi->doing_waitall = false;
    bi->load_text = true;  // default to setting this ...
    //
    // Create a region to manage the block's L2
    // We have no statically allocated L2 memory
    // Track allocations and don't bother with locking
    //
    bi->L2 = mem_region_create( Mem_L2, true, false, "L2" );

    if( bi->L2 == NULL ||
        mem_region_add( bi->L2, CR_L2_BASE(bi->id.block), config->L2_size ) ) {
        ce_error("MAIN", "Can't initialize block %d's L2 memory region\n", block);
        return 1;
    }
    //
    // allocate and initialize our xe infos
    //
    bi->xe_count = config->xes_per_block;

    mem_seg * seg = mem_alloc( bi->ce->CE, sizeof(xe_info) * bi->xe_count );
    if( seg == NULL ) {
        ce_error("MAIN", "Can't allocate block %d's XE objects\n", block);
        return 1;
    }
    bi->xes = (xe_info *) seg->va;

    memset( bi->xes, 0, sizeof(xe_info) * bi->xe_count );

    for( int xe = 0 ; xe < bi->xe_count ; xe++ ) {
        xe_info * xei = bi->xes + xe;

        xei->block = bi;
        xei->id = bi->id;
        xei->id.agent = xe + 1; // CE is agent 0, XE0 is agent 1
        //
        // Create a region to manage this XE's L1
        // Track allocations and don't bother with locking
        //
        xei->L1 = mem_region_create( Mem_L1, true, false, "L1" );

        if( xei->L1 == NULL ||
            mem_region_add( xei->L1,
                            CR_L1_BASE(bi->id.block, xei->id.agent),
                            config->L1_size ) ) {
            ce_error("MAIN", "Can't initialize XE %x's L1 memory region\n", xei->id.all);
            return 1;
        }
    }
    return 0;
}

//
// ce_info init
// This method initializes the ce_info structure in the provided config object.
// It calls block init for each block managed by the CE
//
// Returns 0 for success, 1 for failure.
//
// Note: This init creates a single contiguous CE memory region (slice) for the
//       CE object independent of how many cores the CE has.
//
int ce_info_init( ce_config * config )
{
    //
    // Create a region for the CE's private memory and static allocate the text/data
    // and stack segments. We track allocations and lock it during changes.
    //
    // XXX We might want to move the mem region init to a config init method in case
    // CE memory assignments become complicated in the real HW, and to more easily
    // reserve the CE's memory in the overall IPM memory region
    //
    ce_info * cei = & config->ce;

    cei->config = config;

    cei->id = config->cluster_id;
    cei->id.block = 0;

    cei->CE = mem_region_create( Mem_CE, true, true, "CE Memory" );

    if( cei->CE == NULL ||
        mem_region_add( cei->CE, config->ce_mem_base, config->ce_mem_size ) ) {
        ce_error("MAIN", "Can't initialize CE's private memory region\n");
        return 1;
    }
    //
    // Pre-allocate our (tgkrnl) program image and stack
    //
    mem_seg * seg = mem_alloc_at( cei->CE,
                                  config->ce_mem_base,
                                  (uint64_t) (CE_PROG_END - config->ce_mem_base) );
    mem_return( seg );

    if( tgr_reserve_ce_memory( cei ) ) {
        ce_error("MAIN", "Can't reserve CE memory\n");
        return 1;
    }
    //
    // populate our block infos
    //
#if 0
    seg = mem_alloc( cei->CE, sizeof(block_info) * block_count );
    if( seg == NULL ) {
        ce_error("MAIN", "Can't allocate this CE's block_info memory region\n");
        return 1;
    }
    cei->blocks = (block_info *) seg->va;
#endif // 0
    cei->blocks = config->blocks;
    cei->block_count = config->core_count;

    for( int i = 0 ; i < cei->block_count ; i++ ) {
        block_info * bi = cei->blocks + i;
        int block = (config->mode == CE_MODE_BLOCK) ? config->cluster_id.block : i;

        bi->ce = cei;

        if( block_info_init( bi, block, config ) ) {
            ce_error("MAIN", "Can't initialize the CE block %d object\n", block);
            return 1;
        }
    }
    return 0;
}

//
// Initialize our object tree - CE, block(s), XEs
// We assume that the following has already been set:
// - mode
// - core_count
// - slice_count
// - block_start_mode
// - block_load_mode
// - xe_stack_size
//
int tgr_init( ce_config * config )
{
    //
    // Some validation
    //
    if( BlockCount > MAX_BLOCKS_PER_CLUSTER ) {
        ce_error("MAIN", "Unsupported block count - %d\n", BlockCount);
        return 1;
    }
    //
    // global_info init
    // Track allocations and lock
    // XXX We may need to have configuration specific 'global memory' init
    //
    config->global.IPM = mem_region_create( Mem_IPM, true, true, "IPM" );

    if( config->global.IPM == NULL ||
        mem_region_add( config->global.IPM, config->IPM_base, config->IPM_size ) ) {

        ce_error("MAIN", "Can't initialize this CE's global memory region\n");
        return 1;
    }
    //
    // XXX allocate static dedicated IPM areas - TBD
    //
    // Init our CE
    //
    if( ce_info_init( config ) ) {
        ce_error("MAIN", "Can't initialize the tgr CE object\n");
        return 1;
    }
    return 0;
}

//
// Load all the blocks associated with the CE
// This method reads the XE ELF exe image into a region allocated in CE private
// memory and then passes a pointer to the image to the XE loader.
//
// XXX Will need to change if we allow different XE exes per block.
//
static int tgr_load_blocks( ce_info * cei, const char * elf_file )
{
    int status = 0;

    mem_seg * elf = tgr_load_file( cei, elf_file );

    if( elf == NULL ) {
        ce_error("MAIN", "Can't get XE ELF file\n");
        return 1;
    }

    ce_print("MAIN", "XE Elf image '%s' 0x%lx bytes at %p, seg %p\n",
                elf_file, elf->len, elf->va, elf );
    //
    // load it, allocating memory for the various segments
    //
    for( int i = 0 ; i < cei->block_count ; i++ )
        status = tgr_load_xe_elf_image( cei->blocks + i, elf->va );

    mem_free( elf );

    return status;
}

//
// Place holder for tgkrnl method to start a core at an entry point
// in a multi-core configuration.
//
void tg_thread_start( int core, int (*entry)( void *), void * arg )
{
    block_info * bi = arg;
    ce_print("MAIN", "multi-core thread start called, core %d, block 0x%lx\n", core, bi);
    return;
}

/////////////////////////////// tgr-ce entry point /////////////////////////////
//
// We currently only support 1 core (FSIM) or 4 core with single slice
//
int tgr_main( void )
{
    ce_print("MAIN", "starting\n");
    //
    // subsystem inits
    //
    mem_init();
    ce_os_svc_init();
    ce_print("MAIN", "initialized subsystems\n");
    //
    // Initialize our object tree
    //
    ce_config * config = tgr_config_init();

    if( config == NULL || tgr_init( config ) )
        return 1;

    ce_print("MAIN", "initialized datastructures\n");

    ce_info * cei = & config->ce;

    ce_vprint("MAIN", "XE ELF = %p - '%s'\n", config->xe_elffile, config->xe_elffile );
    //
    // test hook
    //
    TGR_TEST_MAIN;
    //
    // Load our blocks XE image
    //
    if( tgr_load_blocks( cei, config->xe_elffile ) ) {
        ce_error("MAIN", "loading the XE ELF failed\n");
        xe_terminate_all( cei );
        return 1;
    }
    //
    // In a quad-core model (either), we need to spawn each core
    // on a block (guaranteed 1 block / core). Depending on model
    // the blocks may be associated with the same or different ce_infos.
    // This initial thread/core will handle the first block.
    //
    // start our CE's blocks running after first hooking the alarm handling
    //
    tgr_alarms_init( cei );

    for( int i = 1 ; i < cei->config->core_count ; i++ )
        tg_thread_start( i, tgr_block_run, cei->blocks + i );
    //
    // do our main event driven loop for block 0 (if multicore, else BlockNum)
    //
    tgr_block_run( cei->blocks );
    //
    // Since that block finished, synchronize with the other blocks/cores
    // Our timer interrupt will periodically wake us to check.
    //
    while( cei->running ) {
        intr_wait();
    }
    //
    // All blocks stopped, one way or another
    //
    ce_print("MAIN", "all blocks done\n");

    return 0;
}
