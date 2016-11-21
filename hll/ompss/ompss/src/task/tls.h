
#ifndef TASK_LOCAL_H
#define TASK_LOCAL_H

#include <extensions/ocr-runtime-itf.h>
#include <ocr.h>

namespace ompss {

// Workaround to convert an ELS ocrGuid_t
// into a pointer
union _els_to_ptr {
    void*     ptr;
    ocrGuid_t els;
};

inline void setTLS( uint32_t slot, void* data )
{
    // Workaround to access ELS
    union _els_to_ptr tmp;
    tmp.ptr = data;
    ocrElsUserSet( slot, tmp.els );
}

inline void* getTLS( uint32_t slot )
{
    // Workaround to get a pointer from EDT local storage
    union _els_to_ptr tmp;
    tmp.els = ocrElsUserGet( slot );
    return (void*)tmp.ptr;
}

inline void unsetTLS( uint32_t slot )
{
    // Default initialize TLS value
    ocrElsUserSet( slot, NULL_GUID );
}

} // namespace ompss

#endif // TASK_LOCAL_H

