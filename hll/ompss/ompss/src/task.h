
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "event.h"
#include "task-local.h"

#include <cstring>
#include <nanos6_rt_interface.h>

namespace ompss {

inline TaskArguments::TaskArguments( uint32_t args_size ) :
    size(args_size),
    buffer()
{
}

inline TaskArguments::TaskArguments( const TaskArguments& other ) :
    size( other.size ),
    buffer()
{
    std::uninitialized_copy( other.buffer, other.buffer+other.size, buffer );
}

inline TaskDefinition::TaskDefinition( nanos_task_info* info, uint32_t args_size ) :
    run(info->run),
    arguments(args_size)
{
}

inline TaskScopeInfo::TaskScopeInfo() :
    taskwait(),
    accesses(),
    flags(),
    taskMemory(),
    argsMemory()
{
}

inline Task::Task( nanos_task_info* info, uint32_t args_size ) :
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

inline Task* Task::factory::construct( nanos_task_info* info, uint32_t args_size )
{
    void* ptr = static_cast<void*>(&getLocalScope().taskMemory);
    return new (ptr) Task( info, args_size );
}

inline void Task::factory::destroy( Task* task )
{
    task->~Task();
}

inline uint32_t Task::getParamc()
{
    uint64_t size = sizeof(TaskDefinition) + definition->arguments.size;
    return size/sizeof(uint64_t) + 1;
}

inline uint64_t* Task::getParamv()
{
    return static_cast<uint64_t*>(
        static_cast<void*>(definition)
    );
}

inline std::pair<uint32_t,uint64_t*> Task::packParams()
{
    uint32_t taskdef_size = sizeof(TaskDefinition) + definition->arguments.size;
    taskdef_size = taskdef_size/sizeof(uint64_t) + (taskdef_size%sizeof(64)?1 : 0);

    uint32_t deps_size = dependences.release.size() * sizeof(ocrGuid_t);
    deps_size= 1 + deps_size/sizeof(uint64_t) + (deps_size%sizeof(64)?1 : 0);

    uint32_t flags_size = dependences.release.size() * sizeof(uint8_t);
    flags_size= flags_size/sizeof(uint64_t) + (flags_size%sizeof(64)?1 : 0);
    flags_size++;

    uint32_t taskdef_idx = 0;
    uint32_t deps_idx = taskdef_idx + taskdef_size;
    uint32_t flags_idx = deps_idx + deps_size;

    uint32_t paramc = taskdef_size + deps_size + flags_size;
    uint64_t* paramv = new uint64_t[taskdef_size + deps_size + flags_size];

    new(&paramv[taskdef_idx]) TaskDefinition( *definition );

    paramv[deps_idx] = dependences.release.size();

    ocrGuid_t* p_release = static_cast<ocrGuid_t*>( static_cast<void*>(&paramv[deps_idx+1]) );
    std::uninitialized_copy( dependences.release.begin(), dependences.release.end(),
                             (ocrGuid_t*)&paramv[deps_idx+1] );

    uint8_t* p_rflags = static_cast<uint8_t*>( static_cast<void*>(&paramv[flags_idx]) );
    std::uninitialized_copy( dependences.rel_destroy_not_satisfy.begin(), dependences.rel_destroy_not_satisfy.end(),
                             (uint8_t*)&paramv[flags_idx] );

    return std::make_pair( paramc, paramv );
}

inline std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> Task::unpackParams( uint32_t paramc, uint64_t* paramv )
{
    uint32_t taskdef_idx = 0;
    TaskDefinition* def = static_cast<TaskDefinition*>(
                            static_cast<void*>(&paramv[taskdef_idx])
                          );

    uint32_t taskdef_size = sizeof(TaskDefinition) + def->arguments.size;
    taskdef_size = taskdef_size/sizeof(uint64_t) + (taskdef_size%sizeof(64)?1 : 0);

    uint32_t deps_idx = taskdef_idx + taskdef_size;
    uint64_t num_deps = paramv[deps_idx];
    ocrGuid_t* deps = (ocrGuid_t*)&paramv[deps_idx+1];

    uint32_t deps_size = num_deps * sizeof(ocrGuid_t);
    deps_size = 1 + deps_size/sizeof(uint64_t) + (deps_size%sizeof(64)?1 : 0);

    uint32_t flags_size = num_deps * sizeof(uint8_t);
    flags_size= flags_size/sizeof(uint64_t) + (flags_size%sizeof(64)?1 : 0);

    uint32_t flags_idx = deps_idx + deps_size;
    uint8_t* flags = (uint8_t*)&paramv[flags_idx];

    return std::make_tuple(def,num_deps,deps,flags);
}

} // namespace ompss

#endif // TASK_H

