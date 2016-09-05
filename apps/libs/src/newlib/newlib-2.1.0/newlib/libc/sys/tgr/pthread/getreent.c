/* get thread-specific reentrant pointer */

#include <internals.h>

#ifndef SHARED
/* We need a hook to force this file to be linked in when static
   libpthread is used.  */
const int __pthread_provide_getreent = 0;
#endif

struct _reent *
__getreent (void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_reentp);
}

struct _tgr_reent ** __get_tgr_reent_ptr(void)
{
  pthread_descr self = thread_self();
  return &(THREAD_GETMEM(self, p_reentp)->__tgr_reent);
}

