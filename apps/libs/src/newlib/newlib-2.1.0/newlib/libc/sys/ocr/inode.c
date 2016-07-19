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

/////////// getcwd ///////////

char *
_DEFUN (_getcwd_r, (reent, buf, size),
        struct _reent *reent _AND
        char *buf _AND
        size_t size)
{
    if (size == 0 && buf != NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    if (ocrUSalGetcwd(reent->_ocr.legacyContext, buf, size) != 0) {
        return NULL;
    }

    return buf;
}

char *
_DEFUN (_getcwd, (buf, size),
        char * buf _AND
        size_t size)
{
    return _getcwd_r( _REENT, buf, size);
}

/////////// link ///////////

int
_DEFUN (_link_r, (reent, existing, new),
        struct _reent *reent _AND
        const char *existing _AND
        const char *new)
{
  u8 ret = ocrUSalLink (reent->_ocr.legacyContext, existing, new);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_link, (existing, new),
        const char *existing _AND
        const char *new)
{
    return _link_r( _REENT, existing, new );
}

/////////// unlink ///////////
int
_DEFUN (_unlink_r, (reent, name),
        struct _reent *reent _AND
        const char *name)
{
  u8 ret = ocrUSalUnlink (reent->_ocr.legacyContext, name);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_unlink, (name),
        char *name)
{
    return _unlink_r( _REENT, name );
}
/////////// stat ///////////
int
_DEFUN (_stat_r, (reent, file, st),
        struct _reent *reent _AND
        const char  *file _AND
        struct stat *st)
{
  u8 ret = ocrUSalStat (reent->_ocr.legacyContext, file, st);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_stat, (file, st),
        const char  *file _AND
        struct stat *st)
{
    return _stat_r( _REENT, file, st );
}

/////////// fstat ///////////
int
_DEFUN (_fstat_r, (reent, fildes, st),
        struct _reent *reent _AND
        int    fildes _AND
        struct stat *st)
{
  ocrGuid_t guid = ocrLibFdToGuid( reent, fildes );
  u8 ret = ocrUSalFStat (reent->_ocr.legacyContext, guid, st);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_fstat, (fildes, st),
        int    fildes _AND
        struct stat *st)
{
    return _fstat_r( _REENT, fildes, st );
}

/////////// chdir ///////////
int
_DEFUN (_chdir_r, (reent, path),
        struct _reent *reent _AND
        const char *path)
{
  u8 ret = ocrUSalChdir (_REENT->_ocr.legacyContext, path);
  if (ret) errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chdir, (path),
        const char *path)
{
    return _chdir_r( _REENT, path );
}

/////////// chmod ///////////
int
_DEFUN (_chmod_r, (reent, path, mode),
        struct _reent *reent _AND
        const char *path _AND
        mode_t mode)
{
  u8 ret = ocrUSalChmod (reent->_ocr.legacyContext, path, mode);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chmod, (path, mode),
        const char *path _AND
        mode_t mode)
{
    return _chmod_r( _REENT, path, mode );
}

/////////// chown ///////////
int
_DEFUN (_chown_r, (reent, path, owner, group),
        struct _reent *reent _AND
        const char *path  _AND
        uid_t owner _AND
        gid_t group)
{
  u8 ret = ocrUSalChown (reent->_ocr.legacyContext, path, owner, group);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_chown, (path, owner, group),
        const char *path  _AND
        uid_t owner _AND
        gid_t group)
{
    return _chown_r( _REENT, path, owner, group );
}

/////////// readlink ///////////
ssize_t
_DEFUN (_readlink_r, (reent, path, buf, bufsize),
        struct _reent *reent _AND
        const char *path _AND
        char *buf _AND
        size_t bufsize)
{
  s64 ret = ocrReadlink (reent->_ocr.legacyContext, path, buf, bufsize);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

ssize_t
_DEFUN (_readlink, (path, buf, bufsize),
        const char *path _AND
        char *buf _AND
        size_t bufsize)
{
    return _readlink_r( _REENT, path, buf, bufsize );
}
/////////// symlink ///////////
int
_DEFUN (_symlink_r, (reent, path1, path2),
        struct _reent *reent _AND
        const char *path1 _AND
        const char *path2)
{
  u8 ret = ocrUSalSymlink (reent->_ocr.legacyContext, path1, path2);
  if (ret) reent->_errno = (int)ret;
  return (int)ret;
}

int
_DEFUN (_symlink, (path1, path2),
        const char *path1 _AND
        const char *path2)
{
    return _symlink_r( _REENT, path1, path2 );
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
