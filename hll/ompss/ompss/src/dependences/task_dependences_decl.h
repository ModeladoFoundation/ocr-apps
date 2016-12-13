
#ifndef TASK_DEPENDENCES_DECL_H
#define TASK_DEPENDENCES_DECL_H

#include "allocator/firstfit_allocator.h"
#include "task/taskwait.h"

#include <vector>
#include <ocr.h>
#include <nanos6_rt_interface.h>

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
    TaskwaitEvent          newTaskCompleted;
    vector_type<ocrGuid_t> acquire;
    vector_type<ocrGuid_t> acquire_satisfy;
    vector_type<ocrGuid_t> release;

    TaskDependences( nanos_task_info* task );
};

} // namespace ompss

#endif // TASK_DEPENDENCES_DECL_H

