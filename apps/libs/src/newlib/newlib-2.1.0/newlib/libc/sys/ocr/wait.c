#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/ocr.h>

int
_DEFUN (_wait, (status),
        int  *status)
{
  u8 ret = ocrWait ((s32 *)status);
  if (ret) errno = (int)ret;
  return (int)ret;
}
