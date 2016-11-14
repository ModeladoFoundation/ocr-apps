#include <_ansi.h>
#include <_syslist.h>
#include <reent.h>
#include <sys/tgr.h>

/////////// getpid ///////////
int
_DEFUN (_getpid_r, (),
        struct _reent *reent)
{
    return tgr_getpid ();
}
int
_DEFUN (_getpid, (),
        _NOARGS)
{
    return tgr_getpid ();
}
weak_alias(_getpid, __getpid);
weak_alias(_getpid, getpid);

/////////// kill ///////////
int
_DEFUN (_kill_r, (reent, pid, sig),
        struct _reent *reent _AND
        int pid  _AND
        int sig)
{
  return tgr_kill ((s64)pid, (s32)sig);
}

int
_DEFUN (_kill, (pid, sig),
        int pid  _AND
        int sig)
{
    return _kill_r( _REENT, pid, sig );
}

/////////// sched_yield ///////////
int
_DEFUN (sched_yield, (),
        _NOARGS)
{
    return 0;
}
