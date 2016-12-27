#ifndef __CE_OS_SVC_H__
#define __CE_OS_SVC_H__
void ce_os_svc_init();

int ce_os_getcwd( char * fname, size_t max_size );
int ce_os_chdir( const char * fname );
int ce_os_chmod( const char * fname, u64 mode );
int ce_os_chown( const char * fname, u64 owner, u64 group );
int ce_os_link( const char * oldpath, const char * newpath );
int ce_os_symlink( const char * oldpath, const char * newpath );
int ce_os_unlink( const char * fname );
int ce_os_fileopen( const char * fname, u64 flags, u64 mode );
int ce_os_fileclose( int fd );
ssize_t ce_os_fileread( int fd, void * buf, size_t count );
ssize_t ce_os_filewrite( int fd, void * buf, size_t count );
int ce_os_filelseek( int fd, off_t * offset, int whence );
int ce_os_filestat( const char * fname, struct stat * stat );
int ce_os_filefstat( int fd, struct stat * stat );
int ce_os_gettimeofday( struct timeval * tv );
int ce_os_gethostname( char * buf, size_t max_size );
int ce_os_mkdir( const char * dirname, u64 mode );
int ce_os_rmdir( const char * dirname );
int ce_os_isatty( int fd );
int ce_os_readlink( const char * linkpath, char * linkvalue, size_t max_size );

#endif // __CE_OS_SVC_H__
