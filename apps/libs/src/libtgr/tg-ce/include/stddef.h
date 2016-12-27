#ifndef __STDDEF_H__
#define __STDDEF_H__
//
// we don't need much in the way of stddef
// compiling with newlib and NOT clang makes this necessary.
//
#ifndef NULL
#define NULL ((void *)0UL)
#endif // NULL

#define true 1
#define false 0

typedef uint8_t bool;

typedef uint64_t size_t;
typedef int64_t  ssize_t;

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __WINT_TYPE__ wint_t;

struct sigevent { int dummy; };

#endif // __STDDEF_H__
