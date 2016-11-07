
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "allocator/lifo_allocator.h"
#include "common.h"
#include "dependences_decl.h"
#include "event.h"
#include "memory/lazy.h"
#include "taskwait.h"

#include <map>
#include <nanos6_rt_interface.h>

extern "C" typedef void (*run_funct_t)(void* args_block);

namespace ompss {

typedef std::map<
    uintptr_t,
    AccessDependence,
    std::less<uintptr_t>
> DependenceMap;

struct TaskArguments {
    uint64_t size;
    uint8_t  buffer[];

    TaskArguments( uint32_t size );
    TaskArguments( const TaskArguments& other );
};

struct TaskDefinition {
    run_funct_t   run;
    TaskArguments arguments;

    TaskDefinition( nanos_task_info* info, uint32_t args_size );
};

struct TaskFlags {
    bool postponeCleanup;

    TaskFlags() :
        postponeCleanup(false)
    {
    }
};

struct Task {
    TaskDefinition&  definition;
    TaskDependences  dependences;

    Task( nanos_task_info* info, TaskDefinition& def );
    ~Task();

    std::pair<uint32_t,uint64_t*> packParams();
    static std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> unpackParams( uint32_t paramc, uint64_t* paramv );

    struct factory {
        static Task* construct( nanos_task_info* info, uint32_t args_size );
        static void destroy( Task* task );
    };
};

struct TaskScopeInfo {
    typedef std::aligned_storage<sizeof(Task),alignof(Task)>::type UninitializedTask;
    typedef buffered_alloc::lifo_allocator<uint8_t,32U,256U> Allocator;

    Allocator::arena_type      tmpMemory;
    UninitializedTask          taskMemory;
    DependenceMap              accesses;
    TaskwaitEvent              taskwait;
    TaskFlags                  flags;

    TaskScopeInfo();
};

} // namespace ompss

#endif // TASK_DECL_H

