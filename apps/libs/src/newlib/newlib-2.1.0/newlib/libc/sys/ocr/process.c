#include <_ansi.h>
#include <_syslist.h>
#include <reent.h>
#include <errno.h>
#include <sys/ocr.h>

/////////// fork ///////////
int
_DEFUN (_fork_r, (reent),
        struct _reent *reent)
{
  u8 ret = ocrFork ();
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_fork, (),
        _NOARGS)
{
    return _fork_r( _REENT );
}

/////////// execve ///////////
int
_DEFUN (_execve_r, (reent, name, argv, env),
        struct _reent *reent _AND
        const char  *name  _AND
        char * const *argv  _AND
        char * const *env)
{
  u8 ret = ocrExecve (name, argv, env);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_execve, (name, argv, env),
        const char  *name  _AND
        char * const *argv  _AND
        char * const *env)
{
    return _execve_r( _REENT, name, argv, env );
}

/////////// getpid ///////////
int
_DEFUN (_getpid_r, (reent),
        struct _reent *reent)
{
  u8 ret = ocrGetPID ();
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_getpid, (),
        _NOARGS)
{
    return _getpid_r( _REENT );
}

/////////// kill ///////////
int
_DEFUN (_kill_r, (reent, pid, sig),
        struct _reent *reent _AND
        int pid  _AND
        int sig)
{
  u8 ret = ocrKill ((s32)pid, (s32)sig);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
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
