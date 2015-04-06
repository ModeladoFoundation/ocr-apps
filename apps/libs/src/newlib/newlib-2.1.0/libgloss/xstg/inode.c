#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <dirent.h>
#include <ocr.h>

#if 0
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
  return ocr_link (existing, new);
}

int
_DEFUN (_unlink, (name),
        char *name)
{
  return ocr_unlink (name);
}

int
_DEFUN (_stat, (file, st),
        const char  *file _AND
        struct stat *st)
{
  return ocr_stat ((u64)file, st);
}

int
_DEFUN (_fstat, (fildes, st),
        int    fildes _AND
        struct stat *st)
{
  return ocr_fstat ((s32)fildes, st);
}

int
_DEFUN (chdir, (path),
        const char *path)
{
  return ocr_chdir (path);
}

int
_DEFUN (chmod, (path, mode),
        const char *path _AND
        mode_t mode)
{
  return ocr_chmod (path, mode);
}

int
_DEFUN (_chown, (path, owner, group),
        const char *path  _AND
        uid_t owner _AND
        gid_t group)
{
  return ocr_chown (path, owner, group);
}

int
_DEFUN (_readlink, (path, buf, bufsize),
        const char *path _AND
        char *buf _AND
        size_t bufsize)
{
  return ocr_readlink (path, buf, bufsize);
}

int
_DEFUN (_symlink, (path1, path2),
        const char *path1 _AND
        const char *path2)
{
  return ocr_symlink (path1, path2);
}

