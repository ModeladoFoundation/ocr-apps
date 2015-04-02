#include <_ansi.h>
#include <_syslist.h>
#include <sys/ocr.h>

int
_DEFUN (_isatty, (file),
        int file)
{
  return ocrIsAtty ((s32)file);
}
