#include <_ansi.h>
#include <_syslist.h>
#include <ocr.h>

int
_DEFUN (_wait, (status),
        int  *status)
{
  return ocr_wait ((s32 *)status);
}
