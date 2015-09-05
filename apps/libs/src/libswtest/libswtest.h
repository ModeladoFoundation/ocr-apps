#ifndef __LIBSWTEST_H__
#define __LIBSWTEST_H__

//
// basic interfaces (not system related)
//
void ce_exit( int retval ) __attribute__((noreturn));
void ce_abort( void ) __attribute__((noreturn));
void ce_print( char *msg );
//
// ce sw alarm intf
//
void * ce_memalloc( uint64_t len );
void   ce_memfree( void * va );
//
// These map directly to Linux system calls and have the
// same prototypes.
//
int    ce_open( const char * file, int flags, int mode );
int    ce_close( int fd );
int    ce_read( int fd, void * buf, size_t count );
int    ce_write( int fd, const void * buf, size_t count );
off_t  ce_lseek( int fd, off_t offset, int whence );
int    ce_stat( const char * file, struct stat * stbuf );
int    ce_fstat( int fd, struct stat * stbuf );
//
// Cache builtin wrapper
// Flush (Write-back) and/or invalidate lines in the addr range
// 'flags' takes one of the following defines
//
#define CACHE_WB        0x02	// write-back
#define CACHE_INVAL     0x04	// invalidate
#define CACHE_WBINVAL   0x06	// write-back and invalidate

void cache_range( unsigned char flags, void * from, void * to );

#endif // __LIBSWTEST_H__
