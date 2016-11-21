
#ifndef OUTLINE_H
#define OUTLINE_H

#include "task/task.h"

#include <type_traits>

namespace ompss {

struct MainStorage {
    int exit_code;

    MainStorage() :
        exit_code(0)
    {
    }
};

// Check that MainStorage can be safely passed as an EDT parameter
static_assert( std::is_trivially_copyable<MainStorage>::value == true,
    "This class must fulfill all requirements to support plain copies (memcpy)" );

extern ocrGuid_t taskOutlineTemplate;

ocrGuid_t edtOutlineWrapper( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] );

ocrGuid_t edtShutdown( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] );

} // namespace ompss

#endif // OUTLINE_H

