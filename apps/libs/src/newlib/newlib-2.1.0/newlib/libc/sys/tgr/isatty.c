#include <stdlib.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/tgr.h>

int
_DEFUN (_isatty_r, (reent, file),
        struct _reent *reent _AND
        int file)
{
  return tgr_isatty ((s32)file);
}

int
_DEFUN (_isatty, (file),
        int file)
{
  return _isatty_r( _REENT, file );
}
