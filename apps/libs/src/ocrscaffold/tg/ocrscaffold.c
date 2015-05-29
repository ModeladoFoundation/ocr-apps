#define _GNU_SOURCE
#include <sys/features.h>
#include <stdint.h>
#include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
// #include <sys/syscall.h>
#include <signal.h>

#define _NOARGS void
#include <sys/ocr.h>

#define ocr_assert(expr)    // for now no realization

static int ocr_errno;   // obviously not reentrant

static inline int isNull( ocrGuid_t g ) { return g == NULL_GUID; }

//
// This is a scaffolded OCR interface for xstg that provides error
// returns for all of the OCRSal API.
//

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
    uint64_t p = -1;

    if( p == (uint64_t)-1 ) {
        return -1;
    }
    *db = (ocrGuid_t) setGuidType( p, GUID_MEMORY );
    *addr = p;

    if( add_guid( *db, p, len ) ) {
        // printf("Out of mmap GUID entries!\n");
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
    return -1;
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
    for(;;) // just in case
        ;
}

void ocrAbort()
{
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

    int retval = -1;

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

    return 1;
}

u8 ocrUSalRead( ocrGuid_t legacyContext, ocrGuid_t handle,
                s32 *readCount, char * ptr, s32 len)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );

    int nread = -1;

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

    int nwritten = -1;

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
    return 1;
}

//
// mode_t mappings seem to match between newlib and Linux. So no translation
// between newlib and Linux needed.
//
u8 ocrUSalChmod(ocrGuid_t legacyContext, const char* path, mode_t mode)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
u8 ocrUSalChdir(ocrGuid_t legacyContext, const char* path)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
u8 ocrIsAtty (s32 file)
{
    do_catch(); return 0;
}
s64 ocrReadlink (ocrGuid_t legacyContext, const char *path, char *buf, size_t bufsize)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return -1;
}
u8 ocrUSalSymlink(ocrGuid_t legacyContext, const char* path1, const char* path2)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
u8 ocrUSalLink(ocrGuid_t legacyContext, const char* existing, const char* new)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
u8 ocrUSalUnlink(ocrGuid_t legacyContext, const char* name)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
u8 ocrUSalFStat(ocrGuid_t legacyContext, ocrGuid_t handle, struct stat* st)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );
    ocr_assert( st != NULL );
    int ret = -1;

    return (u8) (ret == -1);
}
u8 ocrUSalStat(ocrGuid_t legacyContext, const char* file, struct stat* st)
{
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    return 1;
}
s64 ocrUSalLseek(ocrGuid_t legacyContext, ocrGuid_t handle, s64 offset, s32 whence)
{
    if( isNull(handle) )
        return -1;
    ocr_assert( isGuidType( legacyContext, GUID_CONTEXT ) );
    ocr_assert( isGuidType( handle, GUID_FD ) );
    return -1;
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
    return 1;
}
u8 ocrKill (s32 pid, s32 sig)
{
    return 1;
}
u8 ocrGetTimeofDay (struct timeval  *ptimeval, void *ptimezone)
{
    return 1;
}
