/* ANSI C namespace clean utility typedefs */

/* tgr platform type definitions

   This file defines various typedefs needed by the system calls that support
   the C library.  Basically, they're just the POSIX versions with an '_'
   prepended.  This file lives in the `sys' directory so targets can provide
   their own if desired (or they can put target dependant conditionals here).
*/

#ifndef	_SYS__TYPES_H
#define _SYS__TYPES_H

#include <machine/_types.h>

#ifndef __off_t_defined
typedef long _off_t;
#endif

#ifndef __dev_t_defined
typedef unsigned long int __dev_t;
#endif

#ifndef __uid_t_defined
typedef unsigned int __uid_t;
#endif
#ifndef __gid_t_defined
typedef unsigned int __gid_t;
#endif

#ifndef __off64_t_defined
typedef long int _off64_t;
#endif

#ifndef __daddr_t_defined
typedef	int	__daddr_t;
#define __daddr_t_defined
#endif

#ifndef __caddr_t_defined
typedef	char *	__caddr_t;
#define __caddr_t_defined
#endif

typedef long int __clock_t;
#define __clock_t_defined

typedef long int __time_t;
#define __time_t_defined

typedef	unsigned long	__ino_t;
typedef unsigned long int __nlink_t;
typedef __int64_t __pid_t;
typedef unsigned int __mode_t;
typedef	int __key_t;
typedef long int __ssize_t;

typedef int __clockid_t;
/*
 * We need fpos_t for the following, but it doesn't have a leading "_",
 * so we use _fpos_t instead.
 */
#ifndef __fpos_t_defined
typedef long _fpos_t;		/* XXX must match off_t in <sys/types.h> */
				/* (and must be `long' for now) */
#endif

#ifdef __LARGE64_FILES
#ifndef __fpos64_t_defined
typedef _off64_t _fpos64_t;
#endif
#endif

#define __need_wint_t
#include <stddef.h>

#ifndef __mbstate_t_defined
/* Conversion state information.  */
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;		/* Value so far.  */
} _mbstate_t;
#endif

struct __flock_mutex_t_tmp;
typedef struct
{
  int __a;
  int __b;
  struct
  {
    long int __c1;
    int __c2;
  } __c;
  int __d;
  struct __flock_mutex_t_tmp * __e;
} __flock_mutex_t;

typedef struct { __flock_mutex_t mutex; } _flock_t;

#ifndef __iconv_t_defined
/* Iconv descriptor type */
typedef void *_iconv_t;
#endif

#endif	/* _SYS__TYPES_H */
