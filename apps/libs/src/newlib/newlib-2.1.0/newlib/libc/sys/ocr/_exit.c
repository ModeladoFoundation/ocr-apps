#include <limits.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/ocr.h>

void
_DEFUN (_exit, (rc),
    int rc)
{
  if (rc == 0)
    ocrShutdown();
  else
    ocrAbort();
}

