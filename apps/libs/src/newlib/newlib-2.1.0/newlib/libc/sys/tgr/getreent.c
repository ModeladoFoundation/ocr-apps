#include <reent.h>

/* This is the default function used by _REENT when not using multithreading */
struct _reent * __libc_getreent (void)
{
  return _impure_ptr;
}
weak_alias(__libc_getreent,__getreent)


struct _tgr_reent ** __libc_get_tgr_reent_ptr(void)
{
    return &(_impure_ptr->__tgr_reent);
}
weak_alias(__libc_get_tgr_reent_ptr,__get_tgr_reent_ptr)
