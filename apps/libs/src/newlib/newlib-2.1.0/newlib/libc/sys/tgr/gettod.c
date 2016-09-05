#include <sys/time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/tgr.h>
#include <_ansi.h>

int
_DEFUN (_gettimeofday_r, (reent, ptimeval, ptimezone),
        struct _reent *reent _AND
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  return tgr_gettimeofday (ptimeval, ptimezone);
}

int
_DEFUN (_gettimeofday, (ptimeval, ptimezone),
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  return _gettimeofday_r (_REENT, ptimeval, ptimezone);
}
