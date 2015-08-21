#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include "libswtest.h"
//
// newlib placeholders ...
//
struct _reent * _impure_ptr = (struct _reent *)0L;
int * __errno() { static int real_errno; return & real_errno; }
//
// clang helpfully optimizes a clear loop in crt0.c to use memset()
// but without newlib to provide it we need to do it here
//
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

void exit( int retval )
{
    for( int i = 0 ; i < exit_func_cnt ; i++ )
        exit_funcs[i]();
    _Exit( retval );
}

