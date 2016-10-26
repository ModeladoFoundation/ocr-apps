
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "common.h"
#include "dependences_decl.h"
#include "event.h"
#include "map.h"
#include "memory/lazy.h"

#include <nanos6_rt_interface.h>

extern "C" typedef void (*run_funct_t)(void* args_block);

namespace ompss {

struct TaskArguments {
    uint32_t size;
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
    TaskDefinition*  definition;
    TaskDependences  dependences;

    Task( nanos_task_info* info, uint32_t args_size );
    ~Task();

    uint32_t  getParamc();
    uint64_t* getParamv();

    std::pair<uint32_t,uint64_t*> packParams();
    static std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> unpackParams( uint32_t paramc, uint64_t* paramv );

    struct factory {
        static Task* construct( nanos_task_info* info, uint32_t args_size );
        static void destroy( Task* task );
    };
};

struct TaskScopeInfo {
    typedef std::aligned_storage<sizeof(Task),alignof(Task)>::type UninitializedTask;
    typedef std::aligned_storage<64U,64U>::type Scratchpad;

    ocr::LatchEvent    taskwaitEvent;
    DependenceMap      accesses;
    TaskFlags          flags;
    UninitializedTask  taskMemory;
    Scratchpad         argsMemory;

    TaskScopeInfo();
};

} // namespace ompss

#endif // TASK_DECL_H

