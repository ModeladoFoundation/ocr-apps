#include <stdlib.h>
#include <stdint.h>
#include <reent.h>
#include <errno.h>
#include <sys/ocr.h>
#include <_ansi.h>

#include "ocr_shim.h"

int
_DEFUN (_open, (file, flags, mode),
        char *file  _AND
        int   flags _AND
        int   mode)
{
  ocrGuid_t guid;
  int fd = -1;
  u8 ret = ocrUSalOpen(_REENT->_ocr.legacyContext, &guid, file, (s32)flags, (s32)mode);
  if (ret == 0) {
    fd = ocrLibAddGuid( _REENT, guid );
    if( fd < 0 ) {
      (void) ocrUSalClose(_REENT->_ocr.legacyContext, guid);
      ret = -1;
    }
  } else
      errno = (int)ret;
  return fd;
}

int
_DEFUN (_read, (file, ptr, len),
        int   file  _AND
        char *ptr   _AND
        int   len)
{
  s32 readCount = 0;
  ocrGuid_t guid = ocrLibFdToGuid( _REENT, file );

  u8 ret = ocrUSalRead(_REENT->_ocr.legacyContext, guid, &readCount, ptr, (s32)len);
  if (ret == 0)
	return (int)readCount;
  errno = (int)ret;
  return (int)-1;
}

int
_DEFUN (_write, (file, ptr, len),
        int   file  _AND
        char *ptr   _AND
        int   len)
{
  s32 wroteCount = 0;
  ocrGuid_t guid = ocrLibFdToGuid( _REENT, file );

  u8 ret = ocrUSalWrite(_REENT->_ocr.legacyContext, guid, &wroteCount, ptr, (s32)len);
  if (ret == 0)
	return (int)wroteCount;
  errno = (int)ret;
  return (int)-1;
}

int
_DEFUN (_close, (file),
        int file)
{
  ocrGuid_t guid = ocrLibRmFd( _REENT, file );

  u8 ret = ocrUSalClose(_REENT->_ocr.legacyContext, guid );
  if (ret) {
    errno = (int)ret;
    ret = -1;
  }
  return (int)ret;
}

off_t
_DEFUN (_lseek, (file, ptr, dir),
        int   file  _AND
        int   ptr   _AND
        int   dir)
{
  s64 ret = ocrUSalLseek (_REENT->_ocr.legacyContext, (s32)file, (s32)ptr, (s32)dir);
  if (ret) {
    errno = (int)ret;
    ret = (s64)-1;
  }
  return (off_t)ret;
}

weak_alias( _open, __open )
weak_alias( _close, __close )
weak_alias( _read, __read )
weak_alias( _write, __write )
weak_alias( _lseek, __lseek )
