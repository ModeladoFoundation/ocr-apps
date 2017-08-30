#include <stdint.h>
#include <sys/time.h>

#include <tg-types.h>
#include <tg-cmdline.h>
#include <tg-console.h>

#include <xstg-map.h>

#include "tgr-ce.h"
#include "util.h"

/////////////////////////////// tgr FSIM config /////////////////////////////
#ifdef TGR_FSIM
//
// tgkrnl stack is set up funny under fsim
// The firmware sets the stack to the top of the slice but the tgkrnl
// startup resets it to AR_L1_BASE + 0xDF00 which doesn't exist in the
// 'real world' since the CE has no L1 - probably a QEMUism.
// We don't model/manage CE L1 so we just reserve 16k at the top of slice
// as a place holder
//
#define CE_STACK_TOP   (CE_PROG_BASE + CE_SLICE_SIZE)
#define CE_STACK_SIZE  16  // in KB
//
// We statically define block_info structures and then parcel them out
// to the CEs as they're initialized
//
block_info Blocks[ 1 ];

static ce_config FSIM_config =
{
    .mode = CE_MODE_BLOCK,
    .core_count = 1,           // block count is 1-1 with core count
	.ce_mem_base = CE_PROG_BASE,
	.ce_mem_size = CE_SLICE_SIZE,
    .blocks = Blocks,
    .block_start_mode = 1,     // only start 1 XE in a block (pthreads mode)
    .block_load_mode = 1,      // only load global (text) once
    .xe_stack_size = 0x8000    // default CE allocated XE stack size
};

ce_config * tgr_config_init( void )
{
	ce_config * config = & FSIM_config;
    //
    // init the cluster/tile id from tgkrnl vars
    //
    config->cluster_id.rack    = RackNum;
    config->cluster_id.cube    = CubeNum;
    config->cluster_id.socket  = SocketNum;
    config->cluster_id.cluster = ClusterNum;
    config->cluster_id.block   = BlockNum;  // keep for fsim config

    config->xes_per_block = XeCount;

	config->IPM_base = SR_IPM_BASE;
    config->IPM_size = IpmSize * 1024 * 1024;
    config->L2_size  = L2Size * 1024;
    config->L1_size  = XeL1Size * 1024;

	ce_vprint("CFG", "L1_size = 0x%lx, L2_size = 0x%lx, IPM_size = 0x%lx\n",
			config->L1_size, config->L2_size, config->IPM_size );

    //
    // We get the XE file name from the args passed to tgkrnl (FSIM)
    //
    if( XeArgc == 0 ) {
        ce_error("CONFIG", "no XE ELF filename available to load\n");
        // xe_terminate_all( cei );
        return NULL;
    }
    //
    // Reference the tgkrnl generated XE argv in the CfgArea
    // We will need to reserve the argv area in IPM
    //
    config->xe_elffile = (char *) XeArgv[0];

	return config;
}

//
// Configuration specific memory reservations happen here.
// Reserve the CE's stack and other locations in the CE memory region
//
int tgr_reserve_ce_memory( ce_info * cei )
{
    //
    // Assume the stack is allocated at the top of the CE slice
    //
    int ce_stack_size = CE_STACK_SIZE * 1024;
	uint64_t ce_mem_end     = cei->config->ce_mem_base + cei->config->ce_mem_size;
    uint64_t ce_stack_start = ce_mem_end - ce_stack_size;

    mem_seg * seg = mem_alloc_at( cei->CE, ce_stack_start, ce_stack_size );

	mem_return( seg );

	if( seg == NULL )
		return 1;
	//
	// Now the XE argv, CfgArea in IPM
	//
    seg = mem_alloc_at( cei->config->global.IPM, (uint64_t) CfgArea, CfgAreaSize );
	mem_return( seg );

	return seg == NULL;
}

#endif // TGR_FSIM

/////////////////////////////// tgr Multi-core config //////////////////////////

#ifdef TGR_MULTI_CORE

#define CE_STACK_SIZE  16  // in KB
//
// This configuration is meant to be a placeholder for the real HW
// config with a quad-core CE managing 4 blocks
//
block_info Blocks[ 4 ];

static ce_config FSIM_config =
{
    .mode = CE_MODE_CLUSTER,
    .core_count = 4,           // block count is 1-1 with core count
    .ce_count = 1,
    .ce = & CE,
	.ce_mem_base = CE_PROG_BASE,
	.ce_mem_size = CE_SLICE_SIZE,
    .blocks = Blocks,
    .block_start_mode = 1,     // only start 1 XE in a block (pthreads mode)
    .block_load_mode = 1,      // only load global (text) once
    .xe_stack_size = 0x8000    // default CE allocated XE stack size
};

ce_config * tgr_config_init( void )
{
	ce_config * config = & FSIM_config;
    //
    // init the cluster/tile id from tgkrnl vars
    //
    config->cluster_id.rack = RackNum;
    config->cluster_id.cube = CubeNum;
    config->cluster_id.socket  = SocketNum;
    config->cluster_id.cluster = ClusterNum;
    config->cluster_id.block   = BlockNum;  // keep for fsim config

    config->xes_per_block      = XeCount;
	//
	// IPM use may just cover a segment of HW IPM
	//
	config->IPM_base = SR_IPM_BASE;
    config->IPM_size = IpmSize * 1024 * 1024;

    config->L2_size  = L2Size * 1024;
    config->L1_size  = XeL1Size * 1024;

	ce_vprint("CFG", "L1_size = 0x%lx, L2_size = 0x%lx, IPM_size = 0x%lx\n",
			config->L1_size, config->L2_size, config->IPM_size );

	return config;
}

//
// Allocate the CE's stack in the CE memory region
//
int tgr_reserve_ce_memory( ce_info * cei )
{
    //
    // This is a simplistic model of having an array of stacks at the top of
	// the CE's memory - 1 per core.
    //
	for( int core = 0 ; core < cei->config->core_count ; core++ ) {
		int ce_stack_size = CE_STACK_SIZE * 1024;
		uint64_t ce_stack_start = CE_STACK_TOP - (core + 1) * ce_stack_size;

		mem_seg * seg = mem_alloc_at( cei->CE, ce_stack_start, ce_stack_size );

		if( seg != NULL )
			mem_return( seg );
		return seg == NULL;
	}
}
#endif // TGR_MULTI_CORE
