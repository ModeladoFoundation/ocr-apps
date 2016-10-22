
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "common.h"
#include "dependences_decl.h"
#include "event.h"
#include "map.h"
#include "scratchpad.h"

#include <nanos6_rt_interface.h>

extern "C" {
typedef void (*run_funct_t)(void* args_block);
typedef void (*register_dep_funct_t)(void* handler, void* args_block);
} // extern C

namespace ompss {

struct TaskArguments {
    u32   size;
    char  buffer[];

    TaskArguments( u32 size );
};

struct TaskDefinition {
    run_funct_t   run;
    TaskArguments arguments;

    TaskDefinition( nanos_task_info* info, u32 args_size ) :
        run(info->run),
        arguments(args_size)
    {
    }
};

struct TaskDependences {
    register_dep_funct_t register_dependences;
    GuidVector events;
    TypeVector eventTypes;

    TaskDependences( nanos_task_info* task );
};

struct TaskFlags {
    bool postponeCleanup;

    TaskFlags() :
        postponeCleanup(false)
    {
    }
};

struct Task {
    TaskDefinition*  definition;
    TaskDependences  dependences;

    Task( nanos_task_info* info, u32 args_size );
    ~Task();

    u32  getParamc();
    u64* getParamv();

    struct factory {
        static Task* construct( nanos_task_info* info, u32 args_size );
        static void destroy( Task* task );
    };
};

struct TaskScopeInfo {
    typedef std::aligned_storage<64U,64U>::type Scratchpad;

    LatchEvent              taskwaitEvent;
    DependenceMap           accesses;
    TaskFlags               flags;
    mem::scratchpad<Task,1> taskMemory;
    Scratchpad              argsMemory;

    TaskScopeInfo();
};

} // namespace ompss

#endif // TASK_DECL_H

