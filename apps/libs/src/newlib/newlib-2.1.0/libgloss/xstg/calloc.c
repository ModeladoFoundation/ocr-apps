#include <string.h>
#include <stdlib.h>

#ifndef _REENT_ONLY
_PTR
_DEFUN (calloc, (n, size),
    size_t n _AND
    size_t size)
{
  return _calloc_r (_REENT, n, size);
}
#endif

