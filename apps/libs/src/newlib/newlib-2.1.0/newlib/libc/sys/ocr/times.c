#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/ocr.h>

clock_t
_DEFUN (_times, (buf),
        struct tms *buf)
{
  clock_t ticks;
  u8 ret = ocrTimes (buf, &ticks);
  if (ret == 0)
    return ticks;
  errno = (int)ret;
  return (int)ret;
}
