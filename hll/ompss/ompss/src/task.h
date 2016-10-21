
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "event.h"
#include "task-local.h"

#include <nanos6_rt_interface.h>

namespace ompss {

inline TaskScopeInfo::TaskScopeInfo() :
    taskwaitEvent(),
    accesses(),
    flags()
{
}

inline TaskScopeInfo::~TaskScopeInfo()
{
}

inline Task::Task( nanos_task_info* info, void* args ) :
    definition(info,args),
    dependences()
{
}

inline Task* Task::factory::construct( nanos_task_info* info, u64 args_size )
{
    char* buffer = (char*)ompss_malloc( sizeof(Task) + args_size );
    Task* task = new (buffer) Task( info, buffer+sizeof(Task) );
    return task;
}

inline void Task::factory::destroy( Task* task )
{
    task->~Task();
    ompss_free( task );
}

} // namespace ompss

#endif // TASK_H

