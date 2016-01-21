#include <sys/time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ocr.h>
#include <_ansi.h>

int
_DEFUN (_gettimeofday_r, (reent, ptimeval, ptimezone),
        struct _reent *reent _AND
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  u8 ret = ocrGetTimeofDay (ptimeval, ptimezone);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_gettimeofday, (ptimeval, ptimezone),
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  u8 ret = _gettimeofday_r (_REENT, ptimeval, ptimezone);
}
