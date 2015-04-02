#include <_ansi.h>
#include <_syslist.h>
#include <ocr.h>

int
_DEFUN (_isatty, (file),
        int file)
{
  return ocr_isatty ((s32)file);
}
