#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <reent.h>
#include <_ansi.h>
#include <sys/ocr.h>

#include "ocr_shim.h"

#if 0
#include <dirent.h>
int
_DEFUN (getdents, (fd, dirp, count),
        unsigned int fd _AND
        struct dirent *dirp _AND
        unsigned int count)
{
  return ocr_getdents (fd, dirp, count);
}
#endif

int
_DEFUN (_link, (existing, new),
        char *existing _AND
        char *new)
{
  u8 ret = ocrUSalLink (_REENT->_ocr.legacyContext, existing, new);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_unlink, (name),
        char *name)
{
  u8 ret = ocrUSalUnlink (_REENT->_ocr.legacyContext, name);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_stat, (file, st),
        const char  *file _AND
        struct stat *st)
{
  u8 ret = ocrUSalStat (_REENT->_ocr.legacyContext, file, st);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_fstat, (fildes, st),
        int    fildes _AND
        struct stat *st)
{
  ocrGuid_t guid = ocrLibFdToGuid( _REENT, fildes );
  u8 ret = ocrUSalFStat (_REENT->_ocr.legacyContext, guid, st);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chdir, (path),
        const char *path)
{
  u8 ret = ocrUSalChdir (_REENT->_ocr.legacyContext, path);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chmod, (path, mode),
        const char *path _AND
        mode_t mode)
{
  u8 ret = ocrUSalChmod (_REENT->_ocr.legacyContext, path, mode);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chown, (path, owner, group),
        const char *path  _AND
        uid_t owner _AND
        gid_t group)
{
  u8 ret = ocrUSalChown (_REENT->_ocr.legacyContext, path, owner, group);
  if (ret) errno = (int)ret;
  return (int)ret;
}

ssize_t
_DEFUN (_readlink, (path, buf, bufsize),
        const char *path _AND
        char *buf _AND
        size_t bufsize)
{
  s64 ret = ocrReadlink (_REENT->_ocr.legacyContext, path, buf, bufsize);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_symlink, (path1, path2),
        const char *path1 _AND
        const char *path2)
{
  u8 ret = ocrUSalSymlink (_REENT->_ocr.legacyContext, path1, path2);
  if (ret) errno = (int)ret;
  return (int)ret;
}

weak_alias( _link, link )
weak_alias( _unlink, unlink )
weak_alias( _stat, stat )
weak_alias( _fstat, fstat )
weak_alias( _chdir, chdir )
weak_alias( _chmod, chmod )
weak_alias( _chown, chown )
weak_alias( _readlink, readlink )
weak_alias( _symlink, symlink )
