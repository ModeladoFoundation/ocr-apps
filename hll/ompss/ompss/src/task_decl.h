
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "common.h"
#include "dependences_decl.h"
#include "event.h"
#include "map.h"

#include <nanos6_rt_interface.h>

extern "C" {
typedef void (*run_funct_t)(void* args_block);
typedef void (*register_dep_funct_t)(void* handler, void* args_block);
} // extern C

namespace ompss {

struct TaskDefinition {
    void*                arguments;
    run_funct_t          run;
    register_dep_funct_t register_dependences;

    TaskDefinition( nanos_task_info* info, void* args ) :
        arguments(args),
        run(info->run),
        register_dependences(info->register_depinfo)
    {
    }
};

struct TaskDependences {
    GuidVector events;
    TypeVector eventTypes;
};

struct TaskFlags {
    bool postponeCleanup;

    TaskFlags() :
        postponeCleanup(false)
    {
    }
};

struct TaskScopeInfo {
    LatchEvent          taskwaitEvent;
    ompss::hash_table_t accesses;
    TaskFlags           flags;

    TaskScopeInfo();
    ~TaskScopeInfo();
};

struct Task {
    TaskDefinition  definition;
    TaskDependences dependences;
    TaskScopeInfo   scope;

    Task( nanos_task_info* info, void* args );

    struct factory {
        static Task* construct( nanos_task_info* info, u64 args_size );
        static void destroy( Task* task );
    };
};

} // namespace ompss

#endif // TASK_DECL_H

