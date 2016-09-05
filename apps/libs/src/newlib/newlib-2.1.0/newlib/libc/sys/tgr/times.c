#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <reent.h>
#include <errno.h>
#include <sys/tgr.h>

clock_t
_DEFUN (_times_r, (reent, buf),
        struct _reent *reent _AND
        struct tms *buf)
{
  clock_t ticks;
  s8 ret = tgr_times (buf, &ticks);
  if (ret == 0)
    return ticks;
  return ret;
}

clock_t
_DEFUN (_times, (buf),
        struct tms *buf)
{
    return _times_r( _REENT, buf );
}
