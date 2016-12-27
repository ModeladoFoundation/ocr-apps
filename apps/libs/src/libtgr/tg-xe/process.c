#include "tgr.h"

static void __clone_launch(s64 (*fn)(void *), void* arg)
{
    finish_req req;

    req.ret = fn(arg);

    send_req(CE_REQTYPE_FINISH, &req, sizeof(req));

    __builtin_unreachable();
}

s8 tgr_clone (s64 (*fn)(void *), void* arg, pid_t* pid)
{
    clone_req req;

    req.entry_fn_ptr = (uint64_t)__clone_launch;
    req.arg1 = (uint64_t)fn;
    req.arg2 = (uint64_t)arg;
    req.stack_top = 0UL;            // Let the CE allocate the stack

    int status = send_req(CE_REQTYPE_CLONE, & req, sizeof(req));

    if (pid != NULL)
        *pid = XE_ID_TO_PID(req.pid);

    RETURN(status)
}

static void __tgr_kill_routine (void)
{
    tgr_exit(-1L);
}

s8 tgr_kill (pid_t pid, s32 sig) /* sig is currently unused */
{
    cancel_req req = {
        .pid = PID_TO_XE_ID(pid),
        .cancel_fn_ptr = (uint64_t)__tgr_kill_routine,
        .force_async = 1
    };

    int status = send_req(CE_REQTYPE_CANCEL, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_killall (void)
{
    int status = send_req(CE_REQTYPE_KILLALL, NULL, 0);

    RETURN(status)
}

s8 tgr_cancel_pid(pid_t pid, void (*cancel_fn)(void))
{
    cancel_req req;

    req.pid = PID_TO_XE_ID(pid);
    req.cancel_fn_ptr = (uint64_t)cancel_fn;
    req.force_async = 0;

    int status = send_req(CE_REQTYPE_CANCEL, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_waitpid (pid_t pid, s64 *status, s8 block)
{
    waitpid_req req;

    req.pid = PID_TO_XE_ID(pid);
    req.block = block;

    int req_status = send_req(CE_REQTYPE_WAITPID, & req, sizeof(req));

    if (status != NULL)
        *status = req.ret;

    RETURN(req_status)
}

s8 tgr_detach (pid_t pid)
{
    detach_req req;

    req.pid = (u64) pid;

    int status = send_req(CE_REQTYPE_DETACH, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_waitall (_NOARGS)
{
    int status = send_req(CE_REQTYPE_WAITALL, NULL, 0);
    RETURN(status)
}

pid_t tgr_getpid (_NOARGS)
{
    getpid_req req;

    int status = send_req(CE_REQTYPE_GETPID, & req, sizeof(req));
    return (s64) req.pid;
}

s8 tgr_resume (pid_t pid)
{
    resume_req req;

    req.pid = (u64) pid;

    int status = send_req(CE_REQTYPE_RESUME, &req, sizeof(req));

    RETURN(status)
}

s8 tgr_suspend (const struct timespec* abstime)
{
    suspend_req req;

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
