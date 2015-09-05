#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include "libswtest.h"
//
// newlib placeholders ...
//
// It's possible that an application includes nothing that references
// errno. However, the ocrscaffold lib and ce_intf.c does. This brings
// in nonewlib.o in a newlib env causing multiple defines for exit and
// a few other things. So, we declare those things 'weak'.
//
__attribute__((weak))
struct _reent * _impure_ptr = (struct _reent *)0L;

int * __errno() { static int real_errno; return & real_errno; }
//
// clang helpfully optimizes a clear loop in crt0.c to use memset()
// but without newlib to provide it we need to do it here
//
__attribute__((weak))
void *memset( void *s, int c, size_t n )
{
    uint8_t *p = s;
    uint8_t *e = p + c * n;

    while( p < e )
        *p++ = 0;
    return s;
}
//
// These functions needed for non-newlib. Should be put in a libswtest.a
//
#define MAX_EXIT_FUNCS  32

typedef void (* fptr_t)( void );

static fptr_t exit_funcs[ MAX_EXIT_FUNCS ];
static int exit_func_cnt = 0;

__attribute__((weak))
int atexit( fptr_t f )
{
    if( exit_func_cnt >= MAX_EXIT_FUNCS )
        return 1;
    else {
        exit_funcs[ exit_func_cnt++ ] = f;
        return 0;
    }
}

void _Exit( int retval )
{
    ce_exit( retval );
    __builtin_unreachable();
}

__attribute__((weak))
void exit( int retval )
{
    for( int i = 0 ; i < exit_func_cnt ; i++ )
        exit_funcs[i]();
    _Exit( retval );
}

