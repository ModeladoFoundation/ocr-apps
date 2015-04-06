#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/ocr.h>

int
_DEFUN (_fork, (),
        _NOARGS)
{
  u8 ret = ocrFork ();
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_execve, (name, argv, env),
        char  *name  _AND
        char **argv  _AND
        char **env)
{
  u8 ret = ocrEvecve (name, argv, env);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_getpid, (),
        _NOARGS)
{
  u8 ret = ocrGetPID ();
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_kill, (pid, sig),
        int pid  _AND
        int sig)
{
  u8 ret = ocrKill ((s32)pid, (s32)sig);
  if (ret) errno = (int)ret;
  return (int)ret;
}

