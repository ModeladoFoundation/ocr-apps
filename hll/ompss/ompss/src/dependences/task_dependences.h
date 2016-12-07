
#ifndef TASK_DEPENDENCES_H
#define TASK_DEPENDENCES_H

#include "task_dependences_decl.h"

#include "task/task_scope.h"

#include <vector>
#include <ocr.h>

namespace ompss {

inline TaskDependences::TaskDependences( nanos_task_info* info ) :
    register_dependences(info->register_depinfo),
    acquire( vector_type<ocrGuid_t>::allocator_type( TaskScopeInfo::getLocalScope().scratchMemory ) ),
    release( acquire.get_allocator() ),
    acq_satisfy( acquire.get_allocator() ),
    rel_destroy_not_satisfy( acquire.get_allocator() )
{
}

} // namespace ompss

#endif // TASK_DEPENDENCES_H

