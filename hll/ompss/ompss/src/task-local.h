
#ifndef TASK_LOCAL_H
#define TASK_LOCAL_H

#include "task_decl.h"

#include <extensions/ocr-runtime-itf.h>
#include <ocr.h>

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
    tmp.els = ocrElsUserGet( slot );
    return (void*)tmp.ptr;
}

static inline void unsetTLS( u32 slot )
{
    // Default initialize TLS value
    ocrElsUserSet( slot, NULL_GUID );
}

static inline ompss::TaskScopeInfo& getLocalScope()
{
    return *reinterpret_cast<ompss::TaskScopeInfo*>( getTLS( 0U ) );
}

static inline void setLocalScope( ompss::TaskScopeInfo& scope )
{
    setTLS( 0U, reinterpret_cast<void*>(&scope) );
}

#endif // TASK_LOCAL_H

