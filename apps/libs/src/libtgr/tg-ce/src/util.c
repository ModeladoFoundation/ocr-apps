#include <stdint.h>
#include <sys/time.h>

#include <tg-console.h>

#include "tgr-ce.h"
#include "util.h"

int ce_verbose = CE_VERBOSE;

///////////////////////////// CE Time functions ////////////////////////////////
//
// Get the CEs notion of time, ticks is from tgkrnl
//
#define TICKS_PER_SEC   1000000000 // tune
#define NSEC_PER_TICK   1          // tune

void ce_gettime( struct timespec *ts )
{
    ts->tv_sec = ticks / TICKS_PER_SEC;
    ts->tv_nsec = (ticks - ts->tv_sec * TICKS_PER_SEC) * NSEC_PER_TICK;
}

///////////////////////////// Utility functions ////////////////////////////////
//
// 4 commonly used string fns
//
size_t strlen( const char *s )
{
    const char *t;

    for( t = s; *t ; t++ )
        ;
    return t - s;
}

char *strncat( char *dest, const char *src, size_t n )
{
    char *d = dest;

    while( *d )
        d++;
    while( *src && n-- > 0 )
        *d++ = *src++;
    *d = '\0';

    return dest;
}

void * memcpy( void *d, const void *s, size_t n )
{
    void *r = d;

    while(n-- > 0)
        *(char *)d++ = *(const char *)s++;
    return r;
}

void * memset( void *d, int v, size_t n )
{
    void *r = d;

    while(n-- > 0)
        *(char *)d++ = v;
    return r;
}
