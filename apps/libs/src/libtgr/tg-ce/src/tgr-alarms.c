#include <stdint.h>
#include <sys/time.h>

#include <tg-console.h>

#include "tgr-ce.h"
#include "tgr-alarms.h"
#include "ce-xe-intf.h"
#include "util.h"

#include "xe-abi.h"
//
// Process the specific SW request
// Return 1 on XE termination or fatal error, else 0
//
static bool tgr_alarm_cb( int xe, u64 sw, void * closure )
{
    bool handled = true;
    bool restart = false;
    ce_info * cei = closure;
    int block = xe / MAX_XE_COUNT;

    xe = xe % MAX_XE_COUNT;

    if( xe >= cei->config->xes_per_block ) {
        printf("TGR-ERROR: invalid xe (%d) for alarm\n", xe);
        return false;
    }
    block_info * bi = block_of_ce( cei, block );
    xe_info * xei = xe_of_ce( cei, block, xe );

    switch( sw ) {
      //
      // mark this XE as having alarmed and the main loop will
      // wake up (does after all interrupts) and see the state
      // This relies on a block only being managed by a single core.
      //
      case XE_MSG_READY:
      {
        //uint64_t arg0 = xe_get_reg( xei, XE_ARG0_REG );
        //uint64_t arg1 = xe_get_reg( xei, XE_ARG1_REG );

        // printf("TGR-ALARMS: XE 0x%lx - arg0 0x%lx, arg1 0x%lx\n",
        //    xei->id.all, arg0, arg1 );
        bi->alarmed |= xe_alarmed_bit(xe);
        break;
      }

      case XE_CONOUT_ALARM:
      {
        //
        // arg0 points to the msg, arg1 holds the len (includes null)
        // support a max of 128 bytes
        //
        uint64_t msg_ptr = xe_get_reg( xei, XE_ARG0_REG );
        uint64_t msg_len = xe_get_reg( xei, XE_ARG1_REG );

        if( msg_len > 128 ) {
            printf( "TGR-ERROR: XE%lx msg len truncated\n", xei->id.all );
            msg_len = 128;
        }
        char * msg = (char *) validate_xe_addr( xei, msg_ptr, msg_len );

        if( msg == NULL ) {
            printf( "TGR-ERROR: XE%lx msg address invalid\n", xei->id.all );
        } else {
            msg[msg_len-1] = '\0';   // paranoid
            printf( "TGR-CONSOLE (XE%lx): %s\n", xei->id.all, msg );
        }
        restart = true;
        break;
      }

      case XE_READY:
        printf("TGR-ALARMS: XE%lx ready alarm\n", xei->id.all );
        restart = true;
        break;

      case XE_TERMINATE_ALARM:
        // XXX what to do for terminations ?
        printf( "TGR-ALARMS: XE 0x%lx terminate alarm, terminating xe\n", xei->id.all );
        xei->blocked = true;
        xe_terminated( xei );    // power gate it
        break;

      case XE_ASSERT_ERROR:
        // XXX what to do for asserts ?
        printf( "TGR-ALARMS: XE 0x%lx assert alarm, terminating xe\n", xei->id.all );
        xei->blocked = true;
        xe_terminated( xei );    // power gate it
        break;

      default:
        handled = false;
        printf( "TGR-ALARMS: XE%lx unknown alarm %d\n", xei->id.all, sw );
        break;
    }
    if( restart )
        xe_continue( xei );

    return handled;
}

//
// Register our alarm handler with tgkrnl
//
void tgr_alarms_init( ce_info * cei )
{
    register_sw_alarm_hook( tgr_alarm_cb, cei );
}
