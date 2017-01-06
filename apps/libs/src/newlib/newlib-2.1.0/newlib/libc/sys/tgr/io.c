#include <stdlib.h>
#include <stdint.h>
#include <reent.h>
#include <errno.h>
#include <sys/tgr.h>
#include <_ansi.h>
#include <poll.h>

/////////// open ///////////

int
_DEFUN (_open_r, (reent, file, flags, mode),
        struct _reent *reent _AND
        const char *file  _AND
        int   flags _AND
        int   mode)
{
  u64 fd;
  s8 ret = tgr_open(&fd, file, (s32)flags, (s32)mode);
  if (ret < 0) {
      return ret;
  }
  return (int)fd;
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
  ssize_t readCount = 0;

  s8 ret = tgr_read(file, &readCount, ptr, (s32)len);
  if (ret == 0)
	return (ssize_t)readCount;

  return (ssize_t)-1;
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
  ssize_t wroteCount = 0;

  s8 ret = tgr_write(file, &wroteCount, ptr, (s32)len);
  if (ret == 0)
	return (ssize_t)wroteCount;
  return (ssize_t)-1;
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
  return tgr_close( file );
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
  s64 ptr64 = (s64)ptr;
  s8 ret = tgr_lseek (file, &ptr64, (s32)dir);
  if (ret == -1) {
    return (off_t)-1;
  }
  return (off_t)ptr64;
}

off_t
_DEFUN (_lseek, (file, ptr, dir),
        int   file  _AND
        off_t   ptr   _AND
        int   dir)
{
    return _lseek_r( _REENT, file, ptr, dir );
}

weak_alias( _open, __open )
weak_alias( _close, __close )
weak_alias( _read, __read )
weak_alias( _write, __write )
weak_alias( _lseek, __lseek )
