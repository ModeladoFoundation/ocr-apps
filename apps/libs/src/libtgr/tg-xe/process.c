#include <errno.h>

#include "tgr.h"

static void __clone_launch(s64 (*fn)(void *), void *arg)
{
    s64 ret = fn(arg);
    send_req(CE_REQTYPE_FINISH, &ret, sizeof(u64));
    __builtin_unreachable();
}

s8 tgr_clone (s64 (*fn)(void *), void *arg, s64* pid)
{
    struct {
        uint64_t fn_ptr;            // in
        uint64_t arg;               // in
        uint64_t trampoline_fn_ptr; // in
        uint64_t xe_id;             // out
    } req;

    req.fn_ptr = (uint64_t)fn;
    req.arg = (uint64_t)arg;
    req.trampoline_fn_ptr = (uint64_t)__clone_launch;

    int status = send_req(CE_REQTYPE_CLONE, & req, sizeof(req));

    if (pid != NULL) *pid = XE_ID_TO_PID(req.xe_id);

    RETURN(status)
}

static void __tgr_kill_routine (void)
{
    tgr_exit(-1L);
}

s8 tgr_kill (s64 pid, s32 sig) /* sig is currently unused */
{
    struct {
        uint64_t xe_id;         // in
        uint64_t cancel_fn_ptr; // in
        uint64_t force_async;   // in
    } req = {
        PID_TO_XE_ID(pid),
        (uint64_t)__tgr_kill_routine,
        1
    };

    int status = send_req(CE_REQTYPE_CANCEL, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_killall (void)
{
    int status = send_req(CE_REQTYPE_KILLALL, NULL, 0);

    RETURN(status)
}

s8 tgr_cancel_pid(s64 pid, void (*cancel_fn)(void))
{
    struct {
        uint64_t xe_id;         // in
        uint64_t cancel_fn_ptr; // in
        uint64_t force_async; // in
    } req;

    req.xe_id = PID_TO_XE_ID(pid);
    req.cancel_fn_ptr = (uint64_t)cancel_fn;
    req.force_async = 0;

    int status = send_req(CE_REQTYPE_CANCEL, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_cleanpid (s64 pid, s64 *status, s8 block)
{
    struct {
        uint64_t xe_id;      // in
        uint64_t return_val; // out
        uint64_t block;      // out
    } req;

    req.xe_id = PID_TO_XE_ID(pid);
    req.block = block;

    int req_status;
    req_status = send_req(CE_REQTYPE_CLEANPID, & req, sizeof(req));

    if (status != NULL) *status = req.return_val;

    RETURN(req_status)
}

s8 tgr_detach (s64 pid)
{
    u64 req = (u64) pid; // No need for a struct
    int status = send_req(CE_REQTYPE_DETACH, & req, sizeof(req));
    RETURN(status)
}

s8 tgr_waitall (_NOARGS)
{
    int status = send_req(CE_REQTYPE_WAITALL, NULL, 0);
    RETURN(status)
}

s64 tgr_getpid (_NOARGS)
{
    //
    // Return the XE identification as the pid.
    //
    // Since we are not using any sort of scheduler, there is guaranteed only
    // one process/thread per XE.
    //

    u64 core_loc;

    __asm__ __volatile__(
        "loadmsr %0, %1, 64"
        : /* outs */ "={r1}" (core_loc)
        : /* ins  */  "L" (CORE_LOCATION_NUM)
        );

    return (s64)core_loc;
}

s8 tgr_resume (s64 pid)
{
    int status = send_req(CE_REQTYPE_RESUME, &pid, sizeof(pid));
    RETURN(status)
}

s8 tgr_suspend (const struct timespec* abstime)
{
    struct {
        uint64_t timeout; // in
        uint64_t sec;     // in
        uint64_t nsec;    // in
    } req;

    if (abstime) {
        req.timeout = 1;
        req.sec = abstime->tv_sec;
        req.nsec = abstime->tv_nsec;
    } else {
        req.timeout = 0;
    }

    int status = send_req(CE_REQTYPE_SUSPEND, & req, sizeof(req));

    RETURN(status)
}
