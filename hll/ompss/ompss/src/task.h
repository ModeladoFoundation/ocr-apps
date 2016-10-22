
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "event.h"
#include "task-local.h"

#include <nanos6_rt_interface.h>

namespace ompss {

inline TaskArguments::TaskArguments( u32 args_size ) :
    size(args_size),
    buffer()
{
}

inline TaskDependences::TaskDependences( nanos_task_info* info ) :
    register_dependences(info->register_depinfo)
{
}

inline TaskScopeInfo::TaskScopeInfo() :
    taskwaitEvent(),
    accesses(),
    flags(),
    taskMemory(),
    argsMemory()
{
}

inline Task::Task( nanos_task_info* info, u32 args_size ) :
    definition(nullptr),
    dependences(info)
{
    size_t size = sizeof(TaskDefinition) + args_size;
    ASSERT( size < sizeof(getLocalScope().argsMemory) );

    void* def_buffer = static_cast<void*>(&getLocalScope().argsMemory);

    definition = static_cast<TaskDefinition*>(def_buffer);
    new(definition) TaskDefinition( info, args_size );
}

inline Task::~Task()
{
    definition->~TaskDefinition();
}

inline Task* Task::factory::construct( nanos_task_info* info, u32 args_size )
{
    //void* buffer = ompss_malloc(sizeof(Task));
    Task* task = getLocalScope().taskMemory.get_chunk();
    new (task) Task( info, args_size );
    return task;
}

inline void Task::factory::destroy( Task* task )
{
    task->~Task();
    //ompss_free( task );
    getLocalScope().taskMemory.return_chunk(task);
}

inline u32 Task::getParamc()
{
    u64 size = sizeof(TaskDefinition) + definition->arguments.size;
    return size/sizeof(u64) + 1;
}

inline u64* Task::getParamv()
{
    return static_cast<u64*>(
        static_cast<void*>(definition)
    );
}

} // namespace ompss

#endif // TASK_H

