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
char * ce_getcwd( char * buf, size_t size );
int    ce_chdir( const char * path );
int    ce_chmod( const char * path, mode_t mode);
int    ce_chown( const char * path, uid_t owner, gid_t group);
int    ce_link( const char * oldpath, const char * newpath);
int    ce_symlink( const char * oldpath, const char * newpath);
int    ce_unlink( const char * pathname );
int    ce_open( const char * file, int flags, int mode );
int    ce_close( int fd );
int    ce_read( int fd, void * buf, size_t count );
int    ce_write( int fd, const void * buf, size_t count );
off_t  ce_lseek( int fd, off_t offset, int whence );
int    ce_stat( const char * file, struct stat * stbuf );
int    ce_fstat( int fd, struct stat * stbuf );
int    ce_gettimeofday( struct timeval *ptimeval, void *ptimezone );
//
// Cache builtin wrapper
// Flush (Write-back) and/or invalidate lines in the addr range
// 'flags' takes one of the following defines
//
#define CACHE_WB        0x02    // write-back
#define CACHE_INVAL     0x04    // invalidate
#define CACHE_WBINVAL   0x06    // write-back and invalidate

void cache_range( unsigned char flags, void * from, void * to );
//
// XE location in the system.
// Note that XEs have agent numbers 1 - 8
//
typedef struct {
    uint64_t agent_addr: 17,
             agent: 4,
             block_spacer: 2,
             block: 4,
             cluster_spacer: 1,
             cluster: 4,
             socket_spacer: 13,
             socket: 3,
             cube: 6,
             rack: 6;
} XE_Id;

XE_Id get_xe_id( void );

#endif // __LIBSWTEST_H__
