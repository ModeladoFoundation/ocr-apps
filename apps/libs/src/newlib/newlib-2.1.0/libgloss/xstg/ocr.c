#define _GNU_SOURCE
#include <sys/features.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ocr.h>

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
get_guid( ocrGuid_t guid, uint64_t * addr, uint64_t *len )
{
    for( int i = 0 ; i < NGUIDS ; i++ ) {
        if( Guids[i].guid == guid ) {
            *addr = Guids[i].addr;
            *len = Guids[i].len;
            return 0;
        }
    }
    return 1;
}

#if 0
uint8_t
ocrDbCreate(ocrGuid_t *db, uint64_t* addr, uint64_t len, uint16_t flags,
            ocrGuid_t affinity, ocrInDbAllocator_t allocator)
{
    uint64_t p = (uint64_t) mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if( p == (uint64_t)-1 ) {
        return -1;
    }
    *db = (ocrGuid_t) p;  // guarantee unique and provide to destroy
    *addr = p;

    if( add_guid( (ocrGuid_t) p, p, len ) ) {
        printf("Out of mmap GUID entries!\n");
        munmap( (void *) p, len );
        return 1;
    }
    printf("ocrDbCreate: 0x%08x bytes at 0x%016llx\n", len, p );
    return 0;
}
#else
u8 ocrDbCreate(ocrGuid_t *db, u64* addr, u64 len, u16 flags, ocrGuid_t affinity, ocrInDbAllocator_t allocator)
{
  return 0;
}
#endif

#if 0
uint8_t
ocrDbDestroy(ocrGuid_t db)
{
    uint64_t addr;
    uint64_t len;

    if( get_guid( db, &addr, &len ) ) {
        printf("Can't find mmap GUID entry!\n");
        return 1;
    }
    printf("ocrDbDestroy: 0x%08x bytes at 0x%016llx\n", len, addr );
    return munmap( (void *) addr, len );
}
#else
u8 ocrDbDestroy(ocrGuid_t db)
{
  return 0;
}
#endif

void ocr_exit (u32 rc)
{
  return;
}

u8 ocrAffinityGetCurrent(ocrGuid_t * affinity)
{
  return 0;
}


