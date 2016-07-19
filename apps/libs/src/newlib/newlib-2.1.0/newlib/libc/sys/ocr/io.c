#include <stdlib.h>
#include <stdint.h>
#include <reent.h>
#include <errno.h>
#include <sys/ocr.h>
#include <_ansi.h>

#include "ocr_shim.h"

/////////// open ///////////

int
_DEFUN (_open_r, (reent, file, flags, mode),
        struct _reent *reent _AND
        const char *file  _AND
        int   flags _AND
        int   mode)
{
  ocrGuid_t guid;
  int fd = -1;
  u8 ret = ocrUSalOpen(reent->_ocr.legacyContext, &guid, file, (s32)flags, (s32)mode);
  if (ret == 0) {
    fd = ocrLibAddGuid( reent, guid );
    if( fd < 0 ) {
      (void) ocrUSalClose(reent->_ocr.legacyContext, guid);
      ret = -1;
    }
  } else
      reent->_errno = (int)ret;
  return fd;
}

int
_DEFUN (_open, (file, flags, mode),
        const char *file  _AND
        int   flags _AND
        int   mode)
{
    return _open_r( _REENT, file, flags, mode );
}

/////////// read ///////////

ssize_t
_DEFUN (_read_r, (reent, file, ptr, len),
        struct _reent *reent _AND
        int   file  _AND
        void *ptr   _AND
        size_t   len)
{
  s32 readCount = 0;
  ocrGuid_t guid = ocrLibFdToGuid( reent, file );

  u8 ret = ocrUSalRead(reent->_ocr.legacyContext, guid, &readCount, ptr, (s32)len);
  if (ret == 0)
	return (int)readCount;
  reent->_errno = (int)ret;
  return (int)-1;
}

ssize_t
_DEFUN (_read, (file, ptr, len),
        int   file  _AND
        void *ptr   _AND
        size_t   len)
{
    return _read_r( _REENT, file, ptr, len );
}

/////////// write ///////////

ssize_t
_DEFUN (_write_r, (reent, file, ptr, len),
        struct _reent *reent _AND
        int   file  _AND
        const void *ptr   _AND
        size_t   len)
{
  s32 wroteCount = 0;
  ocrGuid_t guid = ocrLibFdToGuid( reent, file );

  u8 ret = ocrUSalWrite(reent->_ocr.legacyContext, guid, &wroteCount, ptr, (s32)len);
  if (ret == 0)
	return (int)wroteCount;
  reent->_errno = (int)ret;
  return (int)-1;
}

ssize_t
_DEFUN (_write, (file, ptr, len),
        int   file  _AND
        const void *ptr   _AND
        size_t   len)
{
    return _write_r( _REENT, file, ptr, len );
}

/////////// close ///////////

int
_DEFUN (_close_r, (reent,file),
        struct _reent *reent _AND
        int file)
{
  ocrGuid_t guid = ocrLibRmFd( reent, file );

  u8 ret = ocrUSalClose(reent->_ocr.legacyContext, guid );
  if (ret) {
    reent->_errno = (int)ret;
    ret = -1;
  }
  return (int)ret;
}

int
_DEFUN (_close, (file),
        int file)
{
    return _close_r( _REENT, file );
}
/////////// lseek ///////////

off_t
_DEFUN (_lseek_r, (reent, file, ptr, dir),
        struct _reent *reent _AND
        int   file  _AND
        off_t   ptr   _AND
        int   dir)
{
  ocrGuid_t guid = ocrLibFdToGuid( reent, file );
  s64 ret = ocrUSalLseek (reent->_ocr.legacyContext, guid, (s32)ptr, (s32)dir);
  if (ret == -1) {
    reent->_errno = (int)ret;
    ret = (s64)-1;
  }
  return (off_t)ret;
}

off_t
_DEFUN (_lseek, (file, ptr, dir),
        int   file  _AND
        off_t   ptr   _AND
        int   dir)
{
    return _lseek_r( _REENT, file, ptr, dir );
}

weak_alias( _getcwd, getcwd )
weak_alias( _open, __open )
weak_alias( _close, __close )
weak_alias( _read, __read )
weak_alias( _write, __write )
weak_alias( _lseek, __lseek )
