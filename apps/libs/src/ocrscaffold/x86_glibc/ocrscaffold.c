#define _GNU_SOURCE
#include <features.h>
#include <stdint.h>
#include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>

#define _NOARGS void
#include <sys/ocr.h>
//
// Note: This file needs to be compiled with the platform (Linux) includes
//       instead of newlib's since we need to use the platform syscalls.
//       However, this leads to mismatches in newlib vs. platform types,
//       structures, and flags which we need to translate back and forth.

#define ocr_assert(expr)    // for now no realization


///////////////////////////////////////////////////////////////////////////////
// System call shim definition
//
//extern int * __errno(); // we may need to cvt errno defs

static int ocr_errno;   // obviously not reentrant

//
// This is a stub/hack that satisfies a need by syscall()
// to capture errno. We do that in DEF_[V]SYSCALL below
//
//void __syscall_error()
//{
//}

static inline int isNull( ocrGuid_t g ) { return g == NULL_GUID; }

// int * __errno_location() { return & ocr_errno; }
//
// This is a scaffolded OCR interface that provides a Linux implementation
// of the OCR SAL(?) API.
// Linux system calls are implemented via the syscall() interface.
//
//
// define a sys_XXX system call with the same prototype as XXX
// save/restore errno for the caller
//
#define DEF_SYSCALL( rtype, name, decl, ... ) \
    static inline rtype sys_##name decl { \
        unsigned long retval = syscall( SYS_##name, ##__VA_ARGS__ ); \
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

DEF_SYSCALL( char *, getcwd, (char *buf, size_t size), buf, size );

DEF_SYSCALL( int, munmap, (void * addr, size_t len), addr, len );

DEF_SYSCALL( void *, mmap, (void * addr, size_t len, int prot, int flags,
                            int fd, off_t offset),
                           addr, len, prot, flags, fd, offset);
//
// We tag guids as types
//
enum ocr_guid_type {
    GUID_NONE,
    GUID_APP,
    GUID_FD,
    GUID_MEMORY,
    GUID_CONTEXT
};
//
// Access methods
//
#define setGuidType( v, t ) (((uint64_t)(t) << 60) | (v))
#define getGuidType( v )    ((uint64_t)(t) >> 60)
#define getGuidValue( h )   ((h) & (((uint64_t)1 << 60) - 1))
#define isGuidType( h, t )  (getGuidType(h) == (t))

//
// Track GUIDs we hand out so that we can free our resources
// Static allocate to avoid using malloc (could use mmap I guess ...)
//
#define NGUIDS    16
#define INIT_GUID 0xFFFFFFFFFFFFFFFF
#define FREE_GUID 0xFFFFFFFFFFFFFFF0


static
struct guidmap {
    ocrGuid_t guid;
    uint64_t addr;
    uint64_t len;
} Guids[NGUIDS] = {
    { .guid = INIT_GUID, .addr = 0, .len = 0 }
};

static int
add_guid( ocrGuid_t guid, uint64_t addr, uint64_t len )
{
    //
    // init on first use
    //
    if( Guids[0].guid == INIT_GUID ) {
        for( int i = 0 ; i < NGUIDS ; i++ )
            Guids[i].guid = FREE_GUID;
    }
    for( int i = 0 ; i < NGUIDS ; i++ ) {
        if( Guids[i].guid == FREE_GUID ) {
            Guids[i].guid = guid;
            Guids[i].addr = addr;
            Guids[i].len = len;
            return 0;
        }
    }
    return 1;
}

static int
rm_guid( ocrGuid_t guid )
{
    for( int i = 0 ; i < NGUIDS ; i++ ) {
        if( Guids[i].guid == guid ) {
            Guids[i].guid = FREE_GUID;
            return 0;
        }
    }
    return 1;
}

static int
get_guid( ocrGuid_t guid, uint64_t * addr, uint64_t * len )
{
    for( int i = 0 ; i < NGUIDS ; i++ ) {
        if( Guids[i].guid == guid ) {
            if( addr != NULL ) {
                *addr = Guids[i].addr;
                *len = Guids[i].len;
            }
            return 0;
        }
    }
    return 1;
}

//
// Is this a guid we know?
//
static inline int find_guid( ocrGuid_t guid )
{
    return get_guid( guid, NULL, 0 ) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Base OCR methods
//
uint8_t
ocrDbCreate(ocrGuid_t *db, uint64_t* addr, uint64_t len, uint16_t flags,
            ocrGuid_t affinity, ocrInDbAllocator_t allocator)
{
    uint64_t p = (uint64_t) sys_mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if( p == (uint64_t)-1 ) {
        return -1;
    }
    *db = (ocrGuid_t) setGuidType( p, GUID_MEMORY );
    *addr = p;

    if( add_guid( *db, p, len ) ) {
        // printf("Out of mmap GUID entries!\n");
        sys_munmap( (void *) p, len );
        return 1;
    }
    // printf("ocrDbCreate: 0x%08x bytes at 0x%016llx\n", len, p );
    return 0;
}
uint8_t
ocrDbDestroy( ocrGuid_t db )
{
    uint64_t addr;
    uint64_t len;

    ocr_assert( isGuidType(db, GUID_MEMORY) );

    if( get_guid( db, &addr, &len ) ) {
        // printf("Can't find mmap GUID entry!\n");
        return 1;
    }
    (void) rm_guid( db );

    // printf("ocrDbDestroy: 0x%08x bytes at 0x%016llx\n", len, addr );
    return sys_munmap( (void *) addr, len );
}

uint8_t ocrAffinityGetCurrent( ocrGuid_t * affinity )
{
    *affinity = setGuidType( 0, GUID_APP );

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OCR Init/Shutdown methods
//
void ocrInit( ocrGuid_t *legacyContext, ocrConfig_t * ocrConfig )
{
    ocr_assert( legacyContext != NULL );
    ocr_assert( ocrConfig != NULL );

    *legacyContext = setGuidType( 0, GUID_CONTEXT );
}

void ocrParseArgs( int argc, const char* argv[], ocrConfig_t * ocrConfig )
{
    ocr_assert( ocrConfig != NULL );
    ocrConfig->userArgc = argc;
    ocrConfig->userArgv = (char **) argv;
    ocrConfig->iniFile = NULL;
}

void ocrShutdown()
{
    sys_exit( 0 );

    for(;;) // just in case
        ;
}

void ocrAbort()
{
    sys_kill( sys_getpid(), SIGABRT );
}

u8 ocrFinalize(ocrGuid_t legacyContext)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );

    return 0;
}

u8 ocrLegacyContextAssociateMemory( ocrGuid_t legacyContext,
                                    ocrGuid_t db,
                                    ocrLegacyContextMemory_t type )
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( db, GUID_MEMORY ) );

    return 0;
}

u8 ocrLegacyContextRemoveMemory( ocrGuid_t legacyContext,
                                 ocrGuid_t db,
                                 ocrLegacyContextMemory_t type )
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( db, GUID_MEMORY ) );

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OCR SAL methods
//
u8 ocrUSalOpen( ocrGuid_t legacyContext, ocrGuid_t* handle,
                const char * file, s32 flags, s32 mode )
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( handle != NULL );
    ocr_assert( file != NULL );

    int retval = sys_open( file, flags, mode );

    if( retval >= 0 ) {
        * handle = (ocrGuid_t) setGuidType( retval, GUID_FD );
        add_guid( *handle, 0, 0 );
    }

    return retval < 0;
}

u8 ocrUSalClose(ocrGuid_t legacyContext, ocrGuid_t handle)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );

    return sys_close( getGuidValue(handle) ) < 0;
}

u8 ocrUSalRead( ocrGuid_t legacyContext, ocrGuid_t handle,
                s32 *readCount, char * ptr, s32 len)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );

    int nread = sys_read( getGuidValue(handle), (void *) ptr, (size_t) len );

    if( nread >= 0 )
        *readCount = nread;

    return nread < 0;
}

u8 ocrUSalWrite( ocrGuid_t legacyContext, ocrGuid_t handle,
                 s32 *wroteCount, const char * ptr, s32 len)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );

    int nwritten = sys_write( getGuidValue(handle), (const void *) ptr, (size_t) len );

    if( nwritten >= 0 )
        *wroteCount = nwritten;

    return nwritten < 0;
}

//
// debugging hook
//
void do_catch()
{
    ocr_errno = 0;
}

u8 ocrUSalChown(ocrGuid_t legacyContext, const char* path, uid_t owner, gid_t group)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_chown( path, owner, group ) == -1);
}

//
// mode_t mappings seem to match between newlib and Linux. So no translation
// between newlib and Linux needed.
//
u8 ocrUSalChmod(ocrGuid_t legacyContext, const char* path, mode_t mode)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_chmod( path, mode ) == -1);
}
u8 ocrUSalChdir(ocrGuid_t legacyContext, const char* path)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_chdir( path ) == -1);
}
u8 ocrIsAtty (s32 file)
{
    do_catch(); return 0;
}
s64 ocrReadlink (ocrGuid_t legacyContext, const char *path, char *buf, size_t bufsize)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return sys_readlink( path, buf, bufsize );
}
u8 ocrUSalSymlink(ocrGuid_t legacyContext, const char* path1, const char* path2)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_symlink( path1, path2 ) == -1);
}
u8 ocrUSalLink(ocrGuid_t legacyContext, const char* existing, const char* new)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_link( existing, new ) == -1);
}
u8 ocrUSalUnlink(ocrGuid_t legacyContext, const char* name)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_unlink( name ) == -1);
}
//
// This file is compiled with the Linux host includes and it's struct stat
// is a different size and order than newlib's. So we do a manual copy-over
// to avoid trashing our stack, etc.
//
u8 ocrUSalFStat(ocrGuid_t legacyContext, ocrGuid_t handle, struct stat* st)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );
    ocr_assert( st != NULL );
    int ret = sys_fstat( getGuidValue(handle), st );

    return (u8) (ret == -1);
}
u8 ocrUSalStat(ocrGuid_t legacyContext, const char* file, struct stat* st)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return (u8) (sys_stat( file, st ) == -1);
}
s64 ocrUSalLseek(ocrGuid_t legacyContext, ocrGuid_t handle, s64 offset, s32 whence)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );
    return (s64) sys_lseek( getGuidValue(handle), (off_t) offset, (int) whence );
}

u8 ocrFork (_NOARGS)
{
    return -1;
}
u8 ocrEvecve (char *name, char **argv, char **env)
{
    return -1;
}
u8 ocrGetPID (_NOARGS)
{
    return (u8) (sys_getpid( ) == -1);
}
u8 ocrKill (s32 pid, s32 sig)
{
    return (u8) (sys_kill( pid, sig ) == -1);
}
u8 ocrGetTimeofDay (struct timeval  *ptimeval, void *ptimezone)
{
    return -1;
}
