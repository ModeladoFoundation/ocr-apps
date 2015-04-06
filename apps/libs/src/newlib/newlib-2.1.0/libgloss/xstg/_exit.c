#include <limits.h>
#include <_ansi.h>
#include <_syslist.h>
#include <ocr.h>

void
_DEFUN (_exit, (rc),
    int rc)
{
  ocr_exit ((s32)rc);

  for (;;)
    ;
}

