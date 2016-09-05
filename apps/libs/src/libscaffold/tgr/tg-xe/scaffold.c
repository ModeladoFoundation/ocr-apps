#define _GNU_SOURCE
#include <sys/features.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define _NOARGS void
#include <sys/tgr.h>

#include "libswtest.h"

//
// This is a scaffolded TGR interface for xstg that interfaces to the
// CE interfaces provided by libtest.a when run on fsim-swtest
// Non-mappable calls just return error.
//

int * __errno();

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
//    _TGR_REENT = NULL;
    return 0;
}

void tgr_fini(_NOARGS)
{
    return;
}

void tgr_exit(s64 retval)
{
    ce_exit(retval);
    for(;;) // just in case
        ;
}

///////////////////////////////////////////////////////////////////////////////
// memory TGR methods
//
#define PAGE_SIZE 4096 // Hardcoded to 4K for now.
s64 tgr_getpagesize(_NOARGS)
{
    return PAGE_SIZE;
}
s8 tgr_mmap(u64* addr, u64 len)
{
    *addr = (u64)ce_memalloc( len );

    if (addr == NULL) {
        *__errno() = ENOMEM;
        return -1;
    }

    return 0;
}
s8 tgr_munmap(u64* addr, u64 len)
{
    if (addr == NULL) {
        *__errno() = EINVAL;
        return -1;
    }
    ce_memfree( (void *) addr );
    return 0; // We are rather optimistic about freeing memory...
}

///////////////////////////////////////////////////////////////////////////////
// TGR API methods
//
s8 tgr_getcwd(char* buf, u64 bufSize)
{
    char * ret = ce_getcwd( (void *) buf, bufSize );
    if (ret) {
        return 0;
    } else {
        return -1;
    }
}

s8 tgr_open(u64* fd, const char* file, s32 flags, s32 mode)
{
    s64 retval = ce_open( file, flags, mode );
    if (retval >= 0) {
        *fd = (u64)retval;
        return 0;
    }
    return -1;
}

s8 tgr_close(u64 fd)
{
    return ce_close( fd );
}

s8 tgr_read(u64 fd, s32 *readCount, char* ptr, u32 len)
{
    int nread = ce_read( fd, (void *) ptr, (size_t) len );

    if( nread >= 0 )
        *readCount = nread;
        return 0;

    return -1;
}

s8 tgr_write(u64 fd, s32 *wroteCount, const char* ptr, s32 len)
{
    int nwritten = ce_write( fd, (const void *) ptr, (size_t) len );

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
    return ce_chown( path, owner, group );
}

//
// mode_t mappings seem to match between newlib and Linux. So no translation
// between newlib and Linux needed.
//
s8 tgr_chmod(const char* path, mode_t mode)
{
    return ce_chmod( path, mode );
}
s8 tgr_chdir(const char* path)
{
    return ce_chdir( path );
}
s8 tgr_mkdir(const char* path, s32 mode)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_isatty (u64 file)
{
    do_catch(); return 0;
}
s8 tgr_readlink (const char *path, char *buf, size_t bufsize)
{
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_symlink(const char* path1, const char* path2)
{
    return ce_symlink(path1, path2);
}
s8 tgr_link(const char* existing, const char* link)
{
    return ce_link(existing, link);
}
s8 tgr_unlink(const char* name)
{
    return ce_unlink(name);
}
s8 tgr_fstat(u64 fd, struct stat* st)
{
    return ce_fstat( fd, st );
}
s8 tgr_stat(const char* file, struct stat* st)
{
    return ce_stat(file, st);
}
s8 tgr_lseek(u64 fd, s64 * offset, s32 whence)
{
    off_t ret = ce_lseek(fd, (off_t) *offset, (int) whence);
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
    *__errno() = ENOSYS;
    return -1;
}
s8 tgr_kill (s64 pid, s32 sig)
{
    *__errno() = ENOSYS;
    return -1;
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
    return ce_gettimeofday( ptimeval, ptimezone );
}
