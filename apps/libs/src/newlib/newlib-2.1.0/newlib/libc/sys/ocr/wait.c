#include <_ansi.h>
#include <_syslist.h>
#include <reent.h>
#include <errno.h>
#include <sys/ocr.h>

int
_DEFUN (_wait_r, (reent, status),
        struct _reent *reent _AND
        int  *status)
{
  u8 ret = ocrWait ((s32 *)status);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_wait, (status),
        int  *status)
{
  return _wait_r( _REENT, status );
}
