#ifndef __UTIL_H__
#define __UTIL_H__

#define MIN(x,y) ((x) < (y) ? (x) : (y))
//
// Compare timespecs
//
#define EARLIER(a, b) ((a).tv_sec < (b).tv_sec || \
                       ((a).tv_sec == (b).tv_sec && \
                        (a).tv_nsec < (b).tv_nsec))

void ce_gettime( struct timespec *ts );

//
// printing wrappers
//
extern int ce_verbose;

#define ce_error( subsys, ... )    printf("TGR-ERROR: " subsys " - "  __VA_ARGS__)
#define ce_print( subsys, ... )    printf("TGR-" subsys ": " __VA_ARGS__)
#define ce_vprint( subsys, ... )   if( ce_verbose ) printf("TGR-" subsys ": " __VA_ARGS__)

//
// string.h fns
//
size_t strlen( const char *s );
char *strncat( char *dest, const char *src, size_t n );

void * memcpy( void *d, const void *s, size_t n );
void * memset( void *d, int v, size_t n );

#endif // __UTIL_H__
