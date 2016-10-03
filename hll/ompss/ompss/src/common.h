
#ifndef COMMON_H
#define COMMON_H

#include <ocr.h>

typedef void (*run_funct_t)(void* args_block);
typedef void (*register_dep_funct_t)(void* handler, void* args_block);

static inline void* malloc( u64 size )
{
    ocrGuid_t* buffer;
    ocrGuid_t datablock;
    u8 err = ocrDbCreate(&datablock, (void*)&buffer, sizeof(ocrGuid_t)+size,
                     /*flags=*/0, /*loc=*/NULL_HINT, NO_ALLOC);
    buffer[0] = datablock;
    return &buffer[1];
}

static inline void free( void* addr )
{
    ocrGuid_t* buffer = (ocrGuid_t*)addr;
    ocrGuid_t datablock = buffer[-1];

    ocrDbDestroy( datablock );
}

#endif // COMMON_H

