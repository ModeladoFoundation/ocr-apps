#include <stdint.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <tg-types.h>
#include <tg-cmdline.h>
#include <tg-console.h>
#include <xstg-map.h>
#include <mmio-table.h>

#include "tgr-ce.h"
#include "tgr-alarms.h"
#include "ce-xe-intf.h"
#include "ce-os-svc.h"
#include "xe-elf64.h"
#include "util.h"

#define __ASM__
#include <fsim-api.h>
#undef __ASM__

//
// halt the CE core to wait for an event
//
#define intr_wait()  do {__asm__ __volatile__("hlt;"); } while(0)

#define MAX_BLOCKS_PER_CLUSTER  (4) // anticipating HW

#define FSIM_CE_CONFIG  // compiled to run on fsim

////////////////////////////////////////////////////////////////////////////////
// Test scaffolding support
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

#define CE_STACK_SIZE   16  // in KB

////////////////////////////////////////////////////////////////////////////////
//
// Main entry from tgkrnl
//  At this point the cmd line info has been parsed and the
//  XE args created (including the XE ELF file name).
//
// We initialize our data structures from tgkrnl parsed values.
// Includes ce_info, block_info(s) and their xe_info array.
// Memory init creates region objects.
// Wire ourselves into the XE alarm handling
//  Register our IRQ handler to get called on all 'unknown' SW alarms
//  This will save the XE/alarm info and return keeping the XEs stopped.
//  We will need to create our own set of alarm values to avoid interfering
//  with the already defined ones.
//
// Run all XEs until passed the xe loader.
//  Wait for all to do an XE_READY alarm (crt0 startup does an XE_READY alarm)
//  After all have checked in (and not been restarted), restart/continue XE0 after
//  initializing all other XEs to UNUSED.
//
// Main loop
//  Check for MSG alarms and process
//  If XE0 has finished
//      exit (whatever that means)
//  HALT (an interrupt such as an alarm will wake us up)
//
// Features TBD:
//  Suspend timer
//      tgkrnl is set up to field a timer interrupt, but I don't know what
//      the clock rate is to know what the interrupt rate is. Anyway, the
//      timer firing should wake up the main loop which could count-down the
//      suspend timers
//
// Multi-block boot
//  Currently loads XE binary on all blocks but we have to mechanism for CEs
//  to do other than run the same program. No inter-CE comm mechanism.
//
//
// Main config for CE to cluster mapping
//
// Scenarios:
//  Mode BLOCK
//      1 CE core managing 1 block, 1 ce_info has own IPM memory slice (FSIM)
//
//  Mode CLUSTER, 4 blocks, 1 slice
//      4 cores (quad-core CE), 1 ce_info referencing all cluster blocks,
//      each core manages a single block. Block interrupts mapped to managing
//      core local APIC.
//      Does each core queue for work? Run a main loop servicing a block?
//      Requires locks on CE, IPM memory allocation, OS svc intf
//
//  Mode CLUSTER, 4 blocks, 4 slices
//      4 cores (quad-core CE) each managing a block (FSIM model), per CE slice
//      tgkrnl image replicated across each slice, each core goes through full init
//      no shared data between cores - how do we know when it's all done?
//      do cmdline args need munging? copy cmdline into each slice?
//      Requires locks on IPM memory allocation
//      XXX Does this model really make sense?
//
// Questions:
// - How do we best determine slice memory addr in IPM? remap vars?
//      How do we tell firmware about it (w/o baking it in) ?
// - How do we tell when we're done ? Who checks? Who do we tell?
// - How do we get per-cluster/block config info?
//      Place cmdline data at start of cluster block 0 L2 ?
//      Provide cfg filename cmdline arg
//
// Possible cfg file contents:
//  - block exe filename to load
//  - block XE arguments
//  - block XE stack size
//  - start mode

//
// Hard code this until we can get the info via cmdline or the like
//
#ifdef FSIM_CE_CONFIG

ce_info     CEs[1];
//
// We statically define block_info structures and then parcel them out
// to the CEs as they're initialized
//
block_info Blocks[ MAX_BLOCKS_PER_CLUSTER ];

ce_config FSIM_config =
{
    .mode = CE_MODE_BLOCK,
    .core_count = 1,
    .slice_count = 1,          // we only support this for now
    .ce_count = 1,
    .ces = CEs,
    .blocks = Blocks,
    .block_start_mode = 1,     // only start 1
    .block_load_mode = 1,      // only load global (text) once
    .xe_stack_size = 4096      // CE allocated XE stack size
};
ce_config * Tgr_config = & FSIM_config;

#endif // FSIM_CE_CONFIG

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

    for( ; xei < bi->xes + bi->xe_count ; xei++ )
        if( xei->use_state == state ) {
            return xei;
        }
    return NULL;
}

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

        printf("TGR-MAIN: terminated xe 0x%lx, running = %d\n",
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
// XXX Since we may be in a scenario where we have a single CE controlling
// multiple blocks, the addressing here probably needs to be promoted to CR
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
    //
    bi->L2 = mem_region_create( Mem_L2, 1, "L2" );

    if( bi->L2 == NULL ||
        mem_region_add( bi->L2,
                        CR_L2_BASE(bi->id.block),
                        config->L2_size * 1024 ) ) {
        printf("ERROR: can't initialize this block's L2 memory region\n");
        return 1;
    }
    //
    // allocate and initialize our xe infos
    //
    bi->xe_count = config->xes_per_block;

    mem_seg * seg = mem_alloc( bi->ce->CE, sizeof(xe_info) * bi->xe_count );
    if( seg == NULL ) {
        printf("ERROR: can't allocate this block's XE objects\n");
        return 1;
    }
    bi->xes = (xe_info *) seg->va;

    memset( bi->xes, 0, sizeof(xe_info) * bi->xe_count );

    for( int xe = 0 ; xe < bi->xe_count ; xe++ ) {
        xe_info * xei = bi->xes + xe;

        xei->block = bi;
        xei->id = bi->id;
        xei->id.agent = xe + 1; // CE is agent 0, XE0 is agent 1

        xei->L1 = mem_region_create( Mem_L1, 1, "L1" );

        if( xei->L1 == NULL ||
            mem_region_add( xei->L1,
                            CR_L1_BASE(bi->id.block, xei->id.agent),
                            config->L1_size * 1024 ) ) {
            printf("ERROR: can't initialize this XE's L1 memory region\n");
            return 1;
        }
    }
    return 0;
}

//
// ce_info init
//  ce          - main data structure for managing a block or cluster, as configured
//  block_ids   - array holding the block ids in a cluster to manage
//  block_count - number of blocks (w/above ids) to manage, size of block_ids
//  xe_count    - number of XEs to manage per-block
// Returns 0 for success, 1 for failure.
//
// Note: This init creates a CE slice memory region for each CE object. If the HW
//       model turns out to be 1 slice shared across all cores, but with
//       each core dedicated to a separate block, then this won't work. A single
//       region needs to be created with each CE object referencing it.
//
int ce_info_init( ce_config * config )
{
    //
    // Create a region for the CE's slice memory and static allocate the text/data
    // and stack segments.
    //
    ce_info * cei = config->ces;

    cei->config = config;

    cei->id = config->cluster_id;
    cei->id.block = 0;

    cei->CE = mem_region_create( Mem_CE, 1, "CE Slice" );

    if( cei->CE == NULL ||
        mem_region_add( cei->CE, CE_PROG_BASE, CE_SLICE_SIZE ) ) {
        printf("ERROR: can't initialize this CE's private memory region\n");
        return 1;
    }
    //
    // Pre-allocate our (tgkrnl) program image and stack
    //
    mem_seg * seg = mem_alloc_at( cei->CE,
                                  CE_PROG_BASE,
                                  (uint64_t) (CE_PROG_END - CE_PROG_BASE) );
    mem_return( seg );

    int ce_stack_size = CE_STACK_SIZE * 1024;
    uint64_t ce_stack_start = CE_PROG_BASE + CE_SLICE_SIZE - ce_stack_size;

    seg = mem_alloc_at( cei->CE, ce_stack_start, ce_stack_size );
    mem_return( seg );
    //
    // populate our block infos
    //
#if 0
    seg = mem_alloc( cei->CE, sizeof(block_info) * block_count );
    if( seg == NULL ) {
        printf("ERROR: can't allocate this CE's block_info memory region\n");
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
            printf("ERROR: can't initialize the CE block object\n");
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
    // init the cluster/tile id from tgkrnl vars
    //
    config->cluster_id.rack = RackNum;
    config->cluster_id.cube = CubeNum;
    config->cluster_id.socket  = SocketNum;
    config->cluster_id.cluster = ClusterNum;
    config->cluster_id.block   = BlockNum;  // keep for fsim config

    config->xes_per_block      = XeCount;

    config->IPM_size = IpmSize * 1024 * 1024;
    config->L2_size  = L2Size * 1024;
    config->L1_size  = XeL1Size * 1024;
    //
    // Some validation
    //
    if( BlockCount > MAX_BLOCKS_PER_CLUSTER ) {
        printf("TGR ERROR: Unsupported block count - %d\n", BlockCount);
        return 1;
    }
    if( config->slice_count != 1 ) {
        printf("TGR ERROR: Unsupported slice count - %d\n", BlockCount);
        return 1;
    }
    //
    // global_info init XXX single block for now
    //
    config->global.IPM = mem_region_create( Mem_IPM, 1, "IPM" );

    if( config->global.IPM == NULL ||
        mem_region_add( config->global.IPM, SR_IPM_BASE, config->IPM_size ) ) {

        printf("TGR ERROR: can't initialize this block's global memory region\n");
        return 1;
    }
    //
    // allocate static dedicated IPM areas - TBD
    //
    // Init our CE
    //
    if( ce_info_init( config ) ) {
        printf("TGR ERROR: can't initialize the tgr CE object\n");
        return 1;
    }
    return 0;
}

//
// Load all the blocks associated with the CE
// XXX Will need to change if we allow different XE exes per block.
//
static int tgr_load_blocks( ce_info * cei, const char * elf_file )
{
    int status = 0;
    //
    // stat the file to see how big it is and then allocate space for it
    //
    struct stat st;

    if( ce_os_filestat( elf_file, &st ) < 0 ) {
        printf("Can't stat XE ELF file '%s'\n", elf_file);
        return 1;
    }
    //
    // allocate space for and copy the file in
    //
    mem_seg * elf = mem_alloc( cei->CE, st.st_size );

    if( elf == NULL ) {
        printf("Can't allocate %d bytes of CE mem for XE ELF\n", st.st_size );
        return 1;
    }

    int fd = ce_os_fileopen( elf_file, O_RDONLY, 0 );

    if( fd < 0 ) {
        printf("Can't open XE Elf for reading\n");
        mem_free( elf );
        return 1;
    }
    ssize_t got = ce_os_fileread( fd, (void *) elf->va, st.st_size );
    if( got != st.st_size ) {
        printf("Read of XE Elf failed, got %d\n", got );
        return 1;
    }
    (void) ce_os_fileclose( fd );

    printf("TGR: XE Elf image '%s' 0x%x bytes at %p\n", elf_file, st.st_size, elf->va );
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
    printf("TGR: multi-core thread start called, core %d\n", core);
    return;
}

//
// We currently only support 1 core (FSIM) or 4 core with single slice
//
int tgr_main( void )
{
    printf("TGR: starting\n");
    //
    // subsystem inits
    //
    mem_init();
    ce_os_svc_init();
    printf("TGR: initialized subsystems\n");
    //
    // Initialize our object tree
    //
    if( tgr_init( Tgr_config ) )
        return 1;

    printf("TGR: initialized datastructures\n");

    ce_info * cei = Tgr_config->ces;
    //
    // We get the XE file name from the args passed to tgkrnl (FSIM)
    //
    if( XeArgc == 0 ) {
        printf("TGR-ERROR: no XE ELF filename available to load\n");
        xe_terminate_all( cei );
        return 1;
    }
    //
    // XXX Reference the tgkrnl generated XE argv in the CfgArea
    //
    Tgr_config->xe_elffile = (char *) XeArgv[0];

    printf("TGR: XE ELF = %p - '%s'\n", XeArgv[0], Tgr_config->xe_elffile );
    //
    // test hook
    //
    TGR_TEST_MAIN;
    //
    // Load our blocks XE image
    //
    if( tgr_load_blocks( cei, Tgr_config->xe_elffile ) ) {
        printf("TGR-ERROR: loading the XE ELF failed\n");
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
    printf("TGR: all blocks done\n");

    return 0;
}
