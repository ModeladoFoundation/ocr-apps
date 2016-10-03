
#ifndef TASK_LOCAL_H
#define TASK_LOCAL_H

#include <extensions/ocr-runtime-itf.h>

#include "task_decl.h"

typedef struct {
    hash_table_t local_dependences;
    ocrGuid_t    taskwait_evt;
} task_local_storage_t;

// Workaround to convert an ELS ocrGuid_t
// into a pointer
union _els_to_ptr {
    void*     ptr;
    ocrGuid_t els;
};

static inline void setTLS( u32 slot, void* data )
{
    // Store hash table pointer into EDT local storage
    // Workaround to access ELS
    union _els_to_ptr tmp;
    tmp.ptr = data;
    ocrElsUserSet( slot, tmp.els );
}

static inline void* getTLS( u32 slot )
{
    // Workaround to get a pointer from EDT local storage
    union _els_to_ptr tmp;
    tmp.els = ocrElsUserGet( 0U );
    return (void*)tmp.ptr;
}

static inline void unsetTLS( u32 slot )
{
    // Default initialize TLS value
    ocrElsUserSet( slot, NULL_GUID );
}

static inline void setTaskLocalStorage( task_local_storage_t* tls )
{
    setTLS( 0, (void*)tls );
}

static inline hash_table_t* getLocalDepMap()
{
    return (hash_table_t*)getTLS( 0 );
}

#endif // TASK_LOCAL_H

