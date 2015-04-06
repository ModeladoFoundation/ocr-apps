#include <stdlib.h>

void *
_malloc_r (struct _reent *ptr, size_t size)
{
  return malloc (size);
}

void
_free_r (struct _reent *ptr, void *addr)
{
  free (addr);
}

void
_cfree_r (struct _reent *ptr, void *mem)
{
  return cfree (mem);
}

void *
_calloc_r (struct _reent *ptr, size_t size, size_t len)
{
  return calloc (size, len);
}

