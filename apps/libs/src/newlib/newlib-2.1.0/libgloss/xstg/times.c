#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <ocr.h>

clock_t
_DEFUN (_times, (buf),
        struct tms *buf)
{
  return ocr_times (buf);
}
