
#ifndef TASK_LOCAL_H
#define TASK_LOCAL_H

#include "task_decl.h"

#include <extensions/ocr-runtime-itf.h>
#include <ocr.h>

namespace ompss {

// Workaround to convert an ELS ocrGuid_t
// into a pointer
union _els_to_ptr {
    void*     ptr;
    ocrGuid_t els;
};

static inline void setTLS( uint32_t slot, void* data )
{
    // Workaround to access ELS
    union _els_to_ptr tmp;
    tmp.ptr = data;
    ocrElsUserSet( slot, tmp.els );
}

static inline void* getTLS( uint32_t slot )
{
    // Workaround to get a pointer from EDT local storage
    union _els_to_ptr tmp;
    tmp.els = ocrElsUserGet( slot );
    return (void*)tmp.ptr;
}

static inline void unsetTLS( uint32_t slot )
{
    // Default initialize TLS value
    ocrElsUserSet( slot, NULL_GUID );
}

static inline ompss::TaskScopeInfo& getLocalScope()
{
    return *static_cast<ompss::TaskScopeInfo*>( getTLS( 0U ) );
}

static inline void setLocalScope( ompss::TaskScopeInfo& scope )
{
    setTLS( 0U, static_cast<void*>(&scope) );
}

} // namespace ompss

#endif // TASK_LOCAL_H
