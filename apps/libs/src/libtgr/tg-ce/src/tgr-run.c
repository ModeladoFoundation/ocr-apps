#include <stdint.h>
#include <sys/time.h>
#include <errno.h>

#include <tg-console.h>

#include "tgr-ce.h"
#include "tgr-alarms.h"
#include "ce-xe-intf.h"
#include "util.h"

//
// X86 HW synch primitives
//
#define intr_wait()  do {__asm__ __volatile__("sti;hlt;"); } while(0)
#define intr_block() do {__asm__ __volatile__("cli;"); } while(0)
#define intr_allow() do {__asm__ __volatile__("sti;"); } while(0)

//
// Get the msg arguments from XE's registers and call the CE-XE handler
//
static int route_xe_alarm( xe_info * xei )
{
    uint64_t arg0 = xe_get_reg( xei, XE_ARG0_REG );
    uint64_t arg1 = xe_get_reg( xei, XE_ARG1_REG );
    uint64_t status = 0;

    // printf("TGR-RUN: XE 0x%lx - arg0 0x%lx, arg1 0x%lx\n", xei->id.all, arg0, arg1 );

    (void) ce_xe_msg_handler( xei, arg0, arg1, & status );

    xe_set_reg( xei, XE_ARG0_REG, status );

    if( xei->use_state == XE_RUNNING && ! xei->blocked )
        xe_continue( xei );

    return 0;
}

//
// Start initial XE - in master mode only XE0 gets started this way the other
// XEs get started via 'clone' requests.
// The XE exe has already been loaded by this point so we just need to get a
// stack, set some regs, and let it go.
//
static int xe_start( xe_info * xei )
{
    //
    // create a stack for this XE
    //
    size_t stack_size = xei->block->ce->config->xe_stack_size;

    mem_seg * seg = xe_alloc_mem( xei, Mem_ANY, stack_size );

    if( seg == NULL ) {
        printf("TGR-ERROR: can't get a stack for XE 0x%lx\n", xei->id.all );
        return 1;
    }
    xei->stack = seg;
    xei->stack_top = seg->va + seg->len;
    //
    // set its initial PC and stack register
    // XXX push an initial argc/argv with just the name of its exe
    //
    uint64_t * stack = (uint64_t *) xei->stack_top;
    uint64_t   exe   = (uint64_t) xei->block->ce->config->xe_elffile;
    *--stack = 0UL;  // envp
    *--stack = 0UL;  // argv[1]
    *--stack = exe;  // argv[0]
    *--stack = 1;    // argc

    printf("Starting XE 0x%lx with a 0x%lx byte stack at %p (top %p)\n",
            xei->id.all, xei->stack->len, xei->stack->va, stack );

    xe_set_sp( xei, (uint64_t) stack );
    xe_set_ra( xei, 0UL );
    xe_set_pc( xei, xei->block->entry );

    xei->use_state = XE_RUNNING;

    xe_continue( xei );

    xei->block->running++;

    return 0;
}

//
// See if any suspending XEs have timed out
//
static void block_xe_timeout( block_info * bi )
{
    if( bi->suspending ) {
        bool suspending = false;
        struct timespec ts;

        ce_gettime( & ts );
        //
        // time out any expired suspending XEs while tracking if there
        // are any active suspensions to timeout in the future
        //
        for( xe_info * xei = bi->xes ; xei < bi->xes + bi->xe_count ; xei++ ) {
            if( xei->blocked && xei->suspend_time.tv_sec != (time_t)-1 ) {
                if( ! EARLIER(ts, xei->suspend_time) )
                    xe_resume( xei, ETIMEDOUT );
                else
                    suspending = true;
            }
        }
        bi->suspending = suspending;
    }
}

//
// XXX Block run startup and loop
//
int tgr_block_run( void * arg )
{
    block_info * bi = arg;
    //
    // start our XEs for this block
    //
    if( xe_start( xe_of_block(bi, 0) ) ) {
        printf("TGR-ERROR: Start of block %d XE0 failed\n", bi->id.block );
        return 1;
    }
    //
    // start all ?
    //
    if( bi->ce->config->block_start_mode == 0 ) {
        for( int xe = 1 ; xe < bi->xe_count ; xe++ ) {
            if( xe_start( xe_of_block(bi, xe) ) ) {
                printf("TGR-ERROR: Start of block %d XE%d failed\n",
                        bi->id.block, xe );
                return 1;
            }
        }
    }
    //
    // XXX Atomically increment the CE's running block count
    // In a multi-core config this needs an inter-core synch primitive !
    //
    bi->ce->running++;
    //
    // Our main loop
    // Note: synchronization only works if the interrupt handler and
    //       this thread are running on the same core.
    //
    //int wake_count = 0;

    while( bi->running > 0 ) {
        volatile uint64_t alarms;

        // printf("TGR_RUN: woke %d\n", ++wake_count );

        intr_block();

        if( bi->alarmed == 0UL && ! bi->suspending ) {
            intr_wait();
            continue;
        }
        alarms = bi->alarmed;
        bi->alarmed = 0;

        intr_allow();
        //
        // see if we need to wake up a suspended XE
        //
        if( bi->suspending ) {
            block_xe_timeout( bi );
        }
        //
        // if only a timer tick, start over
        //
        if( alarms == 0UL )
            continue;
        //
        // handle XE msg alarms
        //
        for( int xe = 0 ; xe < bi->xe_count && alarms ; xe++ ) {
            if( alarms & xe_alarmed_bit(xe)) {
                route_xe_alarm( xe_of_block( bi, xe ) );

                alarms &= ~ xe_alarmed_bit(xe);
            }
        }
    }
    //
    // Terminate any unused XEs now
    // they didn't count towards our 'running' count but need to be shutdown
    // before declaring victory.
    //
    for( xe_info *xei = bi->xes ; xei < bi->xes + bi->xe_count ; xei++ ) {
        if( xei->use_state == XE_UNUSED )
            xe_terminate( xei );
    }
    //
    // XXX atomically decrement our count of running blocks
    //
    bi->ce->running--;

    return 0;
}

