
#ifndef TASK_SCOPE_H
#define TASK_SCOPE_H

#include "task/task_decl.h"
#include "task/tls.h"

//#include "dependences/exact_match_map.h"
#include "dependences/overlap_match_map.h"

namespace ompss {

struct TaskScopeInfo {
    typedef std::aligned_storage<sizeof(Task),alignof(Task)>::type UninitializedTask;
    typedef buffered_alloc::lifo_allocator<uint8_t,64U,2048U> ParamAllocator;
    typedef buffered_alloc::firstfit_allocator<uint8_t>      ScratchAllocator;

    ParamAllocator::arena_type              paramMemory;
    ScratchAllocator::arena_type<64U,2048U> scratchMemory;
    UninitializedTask                       taskMemory;
    AccessTracker                           accesses;
    TaskwaitEvent                           taskwait;

    TaskScopeInfo();
};

inline TaskScopeInfo::TaskScopeInfo() :
    paramMemory(),
    scratchMemory(),
    taskMemory(),
#if defined(TREE_TMP_ALLOC)
    accesses(tree_allocator(scratchMemory)),
#else
    accesses(),
#endif
    taskwait()
{
}

inline ompss::TaskScopeInfo& getLocalScope()
{
    return *static_cast<ompss::TaskScopeInfo*>( getTLS( 0U ) );
}

inline void setLocalScope( ompss::TaskScopeInfo& scope )
{
    setTLS( 0U, static_cast<void*>(&scope) );
}

} // namespace ompss

#endif // TASK_SCOPE_H

