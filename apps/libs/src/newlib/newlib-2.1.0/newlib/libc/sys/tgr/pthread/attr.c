/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Handling of thread attributes */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include "pthread.h"
#include "internals.h"
#include <shlib-compat.h>

#include <sys/tgr.h>

int __pthread_attr_init_2_1(pthread_attr_t *attr)
{
  size_t ps = tgr_getpagesize();

  attr->__detachstate = PTHREAD_CREATE_JOINABLE;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;
  attr->__guardsize = ps;
  attr->__stackaddr = NULL;
  attr->__stackaddr_set = 0;
  attr->__stacksize = STACK_SIZE - ps;
  return 0;
}

versioned_symbol (libpthread, __pthread_attr_init_2_1, pthread_attr_init,
		  GLIBC_2_1);

#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
int __pthread_attr_init_2_0(pthread_attr_t *attr)
{
  attr->__detachstate = PTHREAD_CREATE_JOINABLE;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;
  return 0;
}
compat_symbol (libpthread, __pthread_attr_init_2_0, pthread_attr_init,
	       GLIBC_2_0);
#endif

int pthread_attr_destroy(pthread_attr_t *attr)
{
  return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate < PTHREAD_CREATE_JOINABLE ||
      detachstate > PTHREAD_CREATE_DETACHED)
    return EINVAL;
  attr->__detachstate = detachstate;
  return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
  *detachstate = attr->__detachstate;
  return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
  switch (scope) {
  case PTHREAD_SCOPE_SYSTEM:
    attr->__scope = scope;
    return 0;
  case PTHREAD_SCOPE_PROCESS:
    return ENOTSUP;
  default:
    return EINVAL;
  }
}

int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
  *scope = attr->__scope;
  return 0;
}

int __pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
#ifdef FLOATING_STACKS
  /* We have to check against the maximum allowed stack size.  This is no
     problem if the manager is already started and we determined it.  If
     this hasn't happened, we have to find the limit outself.  */
  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();

  if (stacksize > __pthread_max_stacksize)
    return EINVAL;
#else
  /* We have a fixed size limit.  */
  if (stacksize > STACK_SIZE)
    return EINVAL;
#endif

  /* We don't accept value smaller than PTHREAD_STACK_MIN.  */
  if (stacksize < PTHREAD_STACK_MIN)
    return EINVAL;

  attr->__stacksize = stacksize;
  return 0;
}
weak_alias (__pthread_attr_setstacksize, pthread_attr_setstacksize)

int __pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
  *stacksize = attr->__stacksize;
  return 0;
}
weak_alias (__pthread_attr_getstacksize, pthread_attr_getstacksize)
