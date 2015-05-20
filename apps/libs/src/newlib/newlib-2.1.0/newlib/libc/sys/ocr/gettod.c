#include <sys/time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ocr.h>
#include <_ansi.h>

int
_DEFUN (_gettimeofday, (ptimeval, ptimezone),
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  u8 ret = ocrGetTimeofDay (ptimeval, ptimezone);
  if (ret) errno = (int)ret;
  return (int)ret;
}
