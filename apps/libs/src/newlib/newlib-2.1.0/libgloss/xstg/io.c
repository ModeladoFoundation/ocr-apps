#include <_ansi.h>
#include <_syslist.h>
#include <ocr.h>

int
_DEFUN (_open, (file, flags, mode),
        char *file  _AND
        int   flags _AND
        int   mode)
{
  return ocr_open((u64)file, (s32)flags, (s32)mode);
}

int
_DEFUN (_read, (file, ptr, len),
        int   file  _AND
        char *ptr   _AND
        int   len)
{
  return ocr_read((s32)file, (u64)ptr, (s32)len);
}

int
_DEFUN (_write, (file, ptr, len),
        int   file  _AND
        char *ptr   _AND
        int   len) {

  return ocr_write((s32)file, (u64)ptr, (s32)len);
}

int
_DEFUN (_close, (fildes),
        int fildes)
{
  return ocr_close((s32)fildes);
}

int
_DEFUN (_lseek, (file, ptr, dir),
        int   file  _AND
        int   ptr   _AND
        int   dir)
{
  return ocr_lseek ((s32)file, (s32)ptr, (s32)dir);
}

