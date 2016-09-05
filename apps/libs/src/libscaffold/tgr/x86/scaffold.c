#define _GNU_SOURCE
#include <_ansi.h>
#include <features.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/user.h>

#define _NOARGS void
#include <sys/tgr.h>

//
// Note: This file needs to be compiled with the platform (Linux) includes
//       instead of newlib's since we need to use the platform syscalls.
//       However, this leads to mismatches in newlib vs. platform types,
//       structures, and flags which we need to translate back and forth.


///////////////////////////////////////////////////////////////////////////////
// Newlib to/from Linux platform data translations
//
// Convert from Linux struct stat to newlib's
// Newlib system type sizes have been matched to Linux x86_64 std.
//
struct	newlib_stat
{
  dev_t		nst_dev;
  ino_t		nst_ino;
  mode_t	nst_mode;
  nlink_t	nst_nlink;
  uid_t		nst_uid;
  gid_t		nst_gid;
  dev_t		nst_rdev;
  off_t		nst_size;
  time_t	nst_atime;
  long		nst_spare1;
  time_t	nst_mtime;
  long		nst_spare2;
  time_t	nst_ctime;
  long		nst_spare3;
  long		nst_blksize;
  long		nst_blocks;
  long	    nst_spare4[2];
};
//
// Do a manual field by field copy from the system stat to the newlib one
//
void stat_copy( struct stat * st, struct stat * lst )
{
    struct newlib_stat *nst = (struct newlib_stat *) st;

    nst->nst_dev = lst->st_dev;
    nst->nst_ino = lst->st_ino;
    nst->nst_mode = lst->st_mode;
    nst->nst_nlink = lst->st_nlink;
    nst->nst_uid = lst->st_uid;
    nst->nst_gid = lst->st_gid;
    nst->nst_rdev = lst->st_rdev;
    nst->nst_size = lst->st_size;
    nst->nst_atime = lst->st_atime;
    nst->nst_mtime = lst->st_mtime;
    nst->nst_ctime = lst->st_ctime;
    nst->nst_blksize = lst->st_blksize;
    nst->nst_blocks = lst->st_blocks;
}

//
// translate newlib open flags to Linux
//
#define NL_O_RDONLY 0
#define NL_O_WRONLY 1
#define NL_O_RDWR   2
#define NL_O_APPEND 0x0008
#define NL_O_CREAT  0x0200
#define NL_O_TRUNC  0x0400
#define NL_O_EXCL   0x0800

#define NL_O_FLAGS  (NL_O_WRONLY|NL_O_RDWR|NL_O_APPEND| \
                     NL_O_CREAT|NL_O_TRUNC|NL_O_EXCL)

static int nlflags_to_linux( int nlflags )
{
    int lflags = 0;
    if( nlflags & NL_O_WRONLY ) lflags |= O_WRONLY;
    if( nlflags & NL_O_RDWR   ) lflags |= O_RDWR;
    if( nlflags & NL_O_APPEND ) lflags |= O_APPEND;
    if( nlflags & NL_O_CREAT  ) lflags |= O_CREAT;
    if( nlflags & NL_O_TRUNC  ) lflags |= O_TRUNC;
    if( nlflags & NL_O_EXCL   ) lflags |= O_EXCL;

    return lflags;
}


///////////////////////////////////////////////////////////////////////////////
// System call shim definition
//
int * __errno(); // we may need to cvt errno defs

// This is a scaffolded TG runtime interface that provides a Linux
// implementation of the TG runtime API.
// Linux system calls are implemented via the syscall() interface.
//
//
// define a sys_XXX system call with the same prototype as XXX
// save/restore errno for the caller
//
#define DEF_SYSCALL( rtype, name, decl, ... ) \
    static inline rtype sys_##name decl { \
        unsigned long retval = syscall( SYS_##name, ##__VA_ARGS__ ); \
        if( retval >= 0xfffffffffffff001 ) { \
            *__errno() = -(int)retval; retval = ~0L; \
        } \
        return (rtype) retval; \
    }
//
// same but for syscalls with no (void) return
//
#define DEF_VSYSCALL( name, decl, ... ) \
    static inline void sys_##name decl { \
        (void) syscall( SYS_##name, __VA_ARGS__ ); \
    }
DEF_VSYSCALL( exit, (int status), status ); // doesn't return

DEF_SYSCALL( int, kill, (int pid, int signal), pid, signal );

DEF_SYSCALL( int, getpid, (void) );

DEF_SYSCALL( int, open, (const char *pathname, int flags, mode_t mode),
                        pathname, flags, mode);
DEF_SYSCALL( int, close, (int fd), fd );

DEF_SYSCALL( ssize_t, read, (int fd, void * addr, size_t len), fd, addr, len );

DEF_SYSCALL( ssize_t, write, (int fd, const void * addr, size_t len), fd, addr, len );

DEF_SYSCALL( off_t, lseek, (int fd, off_t offset, int whence), fd, offset, whence );

DEF_SYSCALL( int, stat, (const char *path, struct stat *buf), path, buf );

DEF_SYSCALL( int, fstat, (int fd, struct stat *buf), fd, buf );

DEF_SYSCALL( ssize_t, readlink, (const char *path, char *buf, size_t bufsiz),
                                path, buf, bufsiz );
DEF_SYSCALL( int, symlink, (const char *oldpath, const char *newpath), oldpath, newpath );

DEF_SYSCALL( int, link, (const char *oldpath, const char *newpath), oldpath, newpath );

DEF_SYSCALL( int, unlink, (const char *pathname), pathname );

DEF_SYSCALL( int, chown, (const char *path, uid_t owner, gid_t group),
                         path, owner, group );
DEF_SYSCALL( int, chmod, (const char *path, mode_t mode), path, mode );

DEF_SYSCALL( int, chdir, (const char *path), path );

DEF_SYSCALL( int, mkdir, (const char *path, mode_t mode), path, mode );

DEF_SYSCALL( int, getcwd, (char *buf, size_t size), buf, size );

DEF_SYSCALL( int, munmap, (void * addr, size_t len), addr, len );

DEF_SYSCALL( void *, mmap, (void * addr, size_t len, int prot, int flags,
                            int fd, off_t offset),
                           addr, len, prot, flags, fd, offset);

DEF_SYSCALL( int, gettimeofday, (struct timeval  *ptimeval, void *ptimezone), ptimeval, ptimezone );

//
// tgr scaffold reent
//
// This implementation does not currently use a reent (it only supports signal threadedness)
//
struct tgr_reent;

// This is implemented in newlib to get a pointer to the tgr_reent pointer in the current thread's struct
struct tgr_reent ** __get_tgr_reent_ptr(_NOARGS);

// A macro which will get a pointer to the tgr_reent for the current
// thread.
#define _TGR_REENT (*__get_tgr_reent_ptr())

u8 tgr_sizeof_reent(_NOARGS) {
    return 0; // Not used
}

///////////////////////////////////////////////////////////////////////////////
// init & fini TGR methods
//

s8 tgr_init(int * argc, char ** argv[], char ** envp[])
{
    _TGR_REENT = NULL;
    return 0;
}

void tgr_fini(_NOARGS)
{
    return;
}

void tgr_exit(s64 retval)
{
    sys_exit(retval);
    for(;;) // just in case
        ;
}

///////////////////////////////////////////////////////////////////////////////
// memory TGR methods
//
s64 tgr_getpagesize(_NOARGS)
{
    return PAGE_SIZE;
}
s8 tgr_mmap(u64* addr, u64 len)
{
    uint64_t p = (uint64_t) sys_mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if( p == (uint64_t)-1 ) {
        return -1;
    }

    *addr = p;
    return 0;
}
s8 tgr_munmap(u64* addr, u64 len)
{
    return sys_munmap( (void *) addr, len );
}

///////////////////////////////////////////////////////////////////////////////
// TGR API methods
//
s8 tgr_getcwd(char* buf, u64 bufSize)
{
    return sys_getcwd( buf, bufSize );
}

s8 tgr_open(u64* fd, const char* file, s32 flags, s32 mode)
{
    flags = nlflags_to_linux( flags );

    int retval = sys_open( file, flags, mode );

    if( retval >= 0 ) {
        * fd = retval;
    }

    return retval;
}

s8 tgr_close(u64 fd)
{
    return sys_close( fd );
}

s8 tgr_read(u64 fd, s32 *readCount, char* ptr, u32 len)
{
    int nread = sys_read( fd, (void *) ptr, (size_t) len );

    if( nread >= 0 )
        *readCount = nread;
        return 0;

    return -1;
}

s8 tgr_write(u64 fd, s32 *wroteCount, const char* ptr, s32 len)
{
    int nwritten = sys_write( fd, (const void *) ptr, (size_t) len );

    if( nwritten >= 0 )
        *wroteCount = nwritten;

    return nwritten < 0;
}

//
// debugging hook
//
void do_catch()
{
    *__errno() = 0;
}

s8 tgr_chown(const char* path, uid_t owner, gid_t group)
{
    return sys_chown( path, owner, group );
}

//
// mode_t mappings seem to match between newlib and Linux. So no translation
// between newlib and Linux needed.
//
s8 tgr_chmod(const char* path, mode_t mode)
{
    return sys_chmod( path, mode );
}
s8 tgr_chdir(const char* path)
{
    return sys_chdir( path );
}
s8 tgr_mkdir(const char* path, s32 mode)
{
    return sys_mkdir( path, (mode_t)mode );
}
s8 tgr_isatty (u64 file)
{
    do_catch(); return 0;
}
s8 tgr_readlink (const char *path, char *buf, size_t bufsize)
{
    return sys_readlink( path, buf, bufsize );
}
s8 tgr_symlink(const char* path1, const char* path2)
{
    return sys_symlink(path1, path2);
}
s8 tgr_link(const char* existing, const char* link)
{
    return sys_link(existing, link);
}
s8 tgr_unlink(const char* name)
{
    return sys_unlink(name);
}
//
// This file is compiled with the Linux host includes and it's struct stat
// is a different size and order than newlib's. So we do a manual copy-over
// to avoid trashing our stack, etc.
//
s8 tgr_fstat(u64 fd, struct stat* st)
{
    struct stat lst;
    int ret = sys_fstat( fd, & lst );

    if( ret == 0 )
        stat_copy( st, & lst );

    return ret;
}
s8 tgr_stat(const char* file, struct stat* st)
{
    return sys_stat(file, st);
}
s8 tgr_lseek(u64 fd, s64 * offset, s32 whence)
{
    off_t ret = sys_lseek(fd, (off_t) offset, (int) whence);
    if (ret == -1) {
        return -1;
    }
    *offset = ret;
    return 0;
}

s8 tgr_clone (s64 (*fn)(void *), void *arg, s64 * pid) {
    *__errno() = ENOSYS;
    return -1;
}

s8 tgr_cleanpid (s64 pid, s64 *status, s8 block)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_detach(s64 pid)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_waitall(_NOARGS)
{
    *__errno() = ENOSYS;
    return -1;
}
s64 tgr_getpid (_NOARGS)
{
     return sys_getpid();
}
s8 tgr_kill (s64 pid, s32 sig)
{
    return sys_kill( pid, sig );
}
s8 tgr_killall (_NOARGS)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_cancel_pid (s64 pid, void(* cancel_fn)(void))
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_resume(s64 pid)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_suspend(const struct timespec *abstime)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_gettimeofday (struct timeval  *ptimeval, void *ptimezone)
{
    return sys_gettimeofday( ptimeval, ptimezone );
}
