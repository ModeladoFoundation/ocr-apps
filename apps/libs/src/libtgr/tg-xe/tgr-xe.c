#include "tgr.h"

// No reent is needed for this tgr implementation.
struct tgr_reent;
struct tgr_reent ** __get_tgr_reent_ptr(void);

// Set to 1 as soon as finialization has happened.
static char __tgr_has_fini = 0;

s8 tgr_init(s32 * argc, char ** argv[], char ** env[])
{
    *__get_tgr_reent_ptr() = NULL; // This implementation needs no reent
    return 0;
}

void tgr_fini(void)
{
    send_req(CE_REQTYPE_KILLALL, NULL, 0);
    __tgr_has_fini = 1;
}

void tgr_exit (s64 retval)
{
    if (!__tgr_has_fini) {
        send_req(CE_REQTYPE_FINISH, &retval, sizeof(u64));
    } else {
        //
        // We have finalized the runtime, so this must be the last XE. Finalize it.
        // If it is returning a non-zero retval (Error) then raise an assert error,
        // otherwise just terminate.
        //
        if (retval) {
            __asm__ __volatile__(
                "alarm %2"
                : /* no outputs */ : "{r2}" (retval), "{r3}" (0), "L" (XE_ASSERT_ERROR)
            );
        } else {
            __asm__ __volatile__(
                "alarm %2"
                : /* no outputs */ : "{r2}" (retval), "{r3}" (0), "L" (XE_TERMINATE)
            );
        }
    }
    __builtin_unreachable();
}

u8 tgr_sizeof_reent(void)
{
    return 0;
}

void ce_print( char *msg )
{
    char * p = msg;
    while( *p++ )    // will count the end '\0'
    ;
    __asm__ __volatile__(
        "alarm %2"
        : /* no outputs */ : "{r2}" (msg), "{r3}" (p - msg), "L" (XE_CONOUT)
        );
}

//
// Cache builtin wrapper
// Flush (Write-back) and/or invalidate lines in the addr range
//
void cache_range( unsigned char flags, void * from, void * to )
{
    switch( flags ) {
        case CACHE_WBINVAL:
        __builtin_xstg_cache_range( CACHE_WBINVAL, from, to ); break;
        // Disable until assembly output issue fixed (I/D instead of 1/0)
        //case CACHE_INVAL:
        //  __builtin_xstg_cache_range( CACHE_INVAL, from, to ); break;
        case CACHE_WB:
        __builtin_xstg_cache_range( CACHE_WB, from, to ); break;
        default:
        break;
    }
}

//
// common method to signal CE with a request.
//
int send_req( uint64_t type, void * buf, uint64_t len )
{
    uint64_t r2 = CE_REQ_MAKE( type, len );
    uint64_t status;

    cache_range( CACHE_WBINVAL, buf, buf + len );

    __asm__ __volatile__(
        "flush B, N\n\t"
        "fence 0xF, B\n\t"
        "alarm %3"
        : /* outs */ "={r2}" (status)
        : /* ins */ "{r2}" (r2), "{r3}" (buf), "L" (XE_MSG_READY)
        );
    return status;
}
