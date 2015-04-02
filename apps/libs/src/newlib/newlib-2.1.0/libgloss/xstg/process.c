#include <_ansi.h>
#include <_syslist.h>
#include <ocr.h>

int
_DEFUN (_fork, (),
        _NOARGS)
{
  return ocr_fork ();
}

int
_DEFUN (_execve, (name, argv, env),
        char  *name  _AND
        char **argv  _AND
        char **env)
{
  return ocr_evecve (name, argv, env);
}

int
_DEFUN (_getpid, (),
        _NOARGS)
{
  return ocr_getpid ();
}

int
_DEFUN (_kill, (pid, sig),
        int pid  _AND
        int sig)
{
  return ocr_kill ((s32)pid, (s32)sig);
}

