
#ifndef TASK_DEPENDENCES_DECL_H
#define TASK_DEPENDENCES_DECL_H

#include "allocator/firstfit_allocator.h"

#include <vector>
#include <ocr.h>
#include <nanos6/nanos6_rt_interface.h>

namespace ompss {

// this needs to be reallocated
extern "C" typedef  void (*register_dep_funct_t)(void* handler, void* args_block);

// Keeps lists of all the dependences found so far
//
// Struct of arrays (depv direct mapping) or array of structs
// (reduced memory fragmentation)?
// Hybrid? vector<acquire> + vector<release,satisfy,releaseOrDestroy>
struct TaskDependences {
    template < typename T >
    using vector_allocator = buffered_alloc::firstfit_allocator<T>;

    template < typename T >
    using vector_type = std::vector<T, vector_allocator<T>>;

    register_dep_funct_t   register_dependences;
    vector_type<ocrGuid_t> acquire;
    vector_type<ocrGuid_t> release;
    vector_type<uint8_t>   acq_satisfy;            // using a char avoids the vector overload for bool
    vector_type<uint8_t>   rel_destroy_not_satisfy;

    TaskDependences( nanos_task_info* task );
};

} // namespace ompss

#endif // TASK_DEPENDENCES_DECL_H

