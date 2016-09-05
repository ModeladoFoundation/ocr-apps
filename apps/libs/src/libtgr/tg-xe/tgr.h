#ifndef __TGR_H__
#define __TGR_H__

#include <sys/tgr.h>

// Not needed or used.
struct tgr_reent;
struct tgr_reent **__get_tgr_reent_ptr(void);

int *__errno(void);

#define ERRNO (*__errno())

//
// Set errno if needed and return -1 on error else return 0.
//
#define RETURN(retval) \
    if (retval) { \
        ERRNO = retval; \
        return -1; \
    } else \
        return 0;

//
// These are defined in xe-sim/include/xe-abi.h
//
#define XE_ASSERT_ERROR 0xC1    // application failure indication
#define XE_READY        0xF0    // provided by crt0 to indicate 'alive-ness'
#define XE_MSG_READY    0xF1    // service request
#define XE_CONOUT       0xFE    // simple string out to fsim 'console'
#define XE_TERMINATE    0xFF    // provided by exit() to indicate done

//
// XE MSRs, defined in intel/tg/common/include/mmio-table.(def|h)
// TODO: use mmio-table.h
//

#define MSR_6   6
#define CURRENT_PC 11
#define CORE_LOCATION_NUM   12
#define POWER_GATE_RESET 31

#define MSR_6_BIT_CLOCK_GATE        0x1000000000000000  // XE weak clock gate


//
// Covert pid's to/from xe_id's
//
// Right now, we are just having these be the same.
// pid must be signed and xe_id must be an agent number
//
#define PID_TO_XE_ID(pid) pid
#define XE_ID_TO_PID(id) id


//
// from fsim ce-handle-alarm.c
//
typedef enum {
    CE_REQTYPE_FIRST    = 0,
    CE_REQTYPE_MEMALLOC = CE_REQTYPE_FIRST,
    CE_REQTYPE_MEMFREE,
    CE_REQTYPE_GETCWD,
    CE_REQTYPE_CHDIR,
    CE_REQTYPE_CHMOD,
    CE_REQTYPE_CHOWN,
    CE_REQTYPE_LINK,
    CE_REQTYPE_SYMLINK,
    CE_REQTYPE_UNLINK,
    CE_REQTYPE_FILEOPEN,
    CE_REQTYPE_FILECLOSE,
    CE_REQTYPE_FILEREAD,
    CE_REQTYPE_FILEWRITE,
    CE_REQTYPE_FILELSEEK,
    CE_REQTYPE_FILESTAT,
    CE_REQTYPE_FILEFSTAT,
    CE_REQTYPE_GETTIMEOFDAY,
    CE_REQTYPE_ISATTY,
    CE_REQTYPE_MKDIR,
    CE_REQTYPE_READLINK,
    CE_REQTYPE_PIPE,
    CE_REQTYPE_CLONE,
    CE_REQTYPE_CLEANPID,
    CE_REQTYPE_FINISH,
    CE_REQTYPE_KILLALL,
    CE_REQTYPE_CANCEL,
    CE_REQTYPE_SUSPEND,
    CE_REQTYPE_RESUME,
    CE_REQTYPE_DETACH,
    CE_REQTYPE_WAITALL,
    CE_REQTYPE_LAST
} ce_request_type;

#define CE_REQ_MAKE( t, l )    (((t) << 32) | ((l) & ((1L << 32) - 1)))

//
// XE location in the system.
// Note that XEs have agent numbers 1 - 8
//
typedef struct {
    uint64_t agent_addr: 17,
             agent: 4,
             block_spacer: 2,
             block: 4,
             cluster_spacer: 1,
             cluster: 4,
             socket_spacer: 13,
             socket: 3,
             cube: 6,
             rack: 6;
} XE_Id;

//
// Send a debug message to the ce for printing. Note, this should not be used
// to print to stdout. For that, use ce_write to fd 1.
//
void ce_print( char *msg );

//
// Cache builtin wrapper
//
// Flush (Write-back) and/or invalidate lines in the addr range
// 'flag' takes one of the following defines
//
#define CACHE_WB        0x02    // write-back
#define CACHE_INVAL     0x04    // invalidate
#define CACHE_WBINVAL   0x06    // write-back and invalidate

//
// Flush (Write-back) and/or invalidate lines in the addr range
//
void cache_range( unsigned char flag, void * from, void * to );

//
// common method to signal CE with a request.
//
// type - a value from ce_request_type
// buf  - a pointer to the request struct
// len  - the sizeof the request struct
//
int send_req( uint64_t type, void * buf, uint64_t len );

#endif
