#include <_ansi.h>
#include <_syslist.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/ocr.h>

int
_DEFUN (_gettimeofday, (ptimeval, ptimezone),
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  u8 ret = ocrGetTimeofDay (ptimeval, ptimezone);
  if (ret) errno = (int)ret;
  return (int)ret;
}
