
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "allocator/firstfit_allocator.h"
#include "allocator/lifo_allocator.h"
#include "allocator/proxy_allocator.h"

#include "dependences/task_dependences_decl.h"

#include "common.h"
#include "event.h"
#include "memory/lazy.h"

#include "task/taskwait.h"

#include <nanos6/nanos6_rt_interface.h>

extern "C" typedef void (*run_funct_t)(void* args_block);

namespace ompss {

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

struct Task {
    TaskDefinition&  definition;
    TaskDependences  dependences;

    Task( nanos_task_info* info, TaskDefinition& def );

    std::pair<uint32_t,uint64_t*> packParams();
    static std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> unpackParams( uint32_t paramc, uint64_t* paramv );

    void acquireDependences();
    void releaseDependences();

    struct factory {
        static Task* construct( nanos_task_info* info, uint32_t args_size );
        static void destroy( Task* task );
    };
};

} // namespace ompss

#endif // TASK_DECL_H

