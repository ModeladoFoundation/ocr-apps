
#ifndef TASK_DEPENDENCES_H
#define TASK_DEPENDENCES_H

#include "task_dependences_decl.h"

#include "task/task_scope.h"

#include <vector>
#include <ocr.h>

namespace ompss {

inline TaskDependences::TaskDependences( nanos_task_info* info ) :
    register_dependences(info->register_depinfo),
    newTaskCompleted(),
    acquire( vector_type<ocrGuid_t>::allocator_type( TaskScopeInfo::getLocalScope().scratchMemory ) ),
    acquire_satisfy( acquire.get_allocator() ),
    release( acquire.get_allocator() )
{
    // Open Region to prevent premature triggering
    newTaskCompleted.openRegion();
}

} // namespace ompss

#endif // TASK_DEPENDENCES_H

