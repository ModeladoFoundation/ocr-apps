
#ifndef TASK_SCOPE_H
#define TASK_SCOPE_H

#ifdef EXACT_MATCH_DEPENDENCES
#  include "dependences/exact_match_map.h"
#else
#  include "dependences/overlap_match_map.h"
#endif

#include "outline.h"
#include "task/task_decl.h"
#include "task/tls.h"

namespace ompss {

struct TaskScopeInfo {
    typedef std::aligned_storage<sizeof(Task),alignof(Task)>::type UninitializedTask;
    typedef buffered_alloc::lifo_allocator<uint8_t,64U,2048U> ParamAllocator;
    typedef buffered_alloc::firstfit_allocator<uint8_t>      ScratchAllocator;

    ParamAllocator::arena_type              paramMemory;
    ScratchAllocator::arena_type<64U,2048U> scratchMemory;
    UninitializedTask                       taskMemory;
    AccessTracker                           accesses;
    ocrGuid_t                               taskOutlineTemplate;
    TaskwaitEvent                           taskwait;

    TaskScopeInfo();
    ~TaskScopeInfo();

    static void setLocalScope( ompss::TaskScopeInfo& scope ) {
        setTLS( 0U, static_cast<void*>(&scope) );
    }

    static void unsetLocalScope() {
        setTLS( 0U, nullptr );
    }

    static ompss::TaskScopeInfo& getLocalScope() {
        return *static_cast<ompss::TaskScopeInfo*>( getTLS( 0U ) );
    }
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
    taskOutlineTemplate(),
    taskwait()
{
    uint8_t err = ocrEdtTemplateCreate( &taskOutlineTemplate, edtOutlineWrapper, EDT_PARAM_UNK, EDT_PARAM_UNK );
    ASSERT( err == 0);

    // Open taskwait region
    taskwait.openRegion();

    // Store local scope in EDT local storage
    setLocalScope( *this );
}

inline TaskScopeInfo::~TaskScopeInfo() {
    uint8_t err = ocrEdtTemplateDestroy( taskOutlineTemplate );
    ASSERT( err == 0 );

    // Close taskwait region
    taskwait.closeRegion();

    unsetLocalScope();
}

} // namespace ompss

#endif // TASK_SCOPE_H

