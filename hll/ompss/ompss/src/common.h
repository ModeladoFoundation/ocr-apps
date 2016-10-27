
#ifndef COMMON_H
#define COMMON_H

#include "debug.h"

#include <ocr.h>

static inline ocrGuid_t getBufferDb( void* addr )
{
    char* base = ((char*)addr)-sizeof(ocrGuid_t);
    return *((ocrGuid_t*)base);
}

static inline void* getUserBuffer( void* base )
{
    char* user = ((char*)base)+sizeof(ocrGuid_t);
    return ((void*)user);
}

static inline void* ompss_malloc( u64 size )
{
    PROFILE_BLOCK;
    ocrGuid_t* db_buffer;
    ocrGuid_t datablock;
    u8 err = ocrDbCreate(&datablock, (void**)&db_buffer, sizeof(ocrGuid_t)+size,
                     /*flags=*/0, /*loc=*/NULL_HINT, NO_ALLOC);
    ASSERT( err == 0 );
    db_buffer[0] = datablock;
    return getUserBuffer(db_buffer);
}

static inline void ompss_free( void* addr )
{
    PROFILE_BLOCK;

    u8 err = ocrDbDestroy( getBufferDb(addr) );
    ASSERT( err == 0 );
}

#endif // COMMON_H
