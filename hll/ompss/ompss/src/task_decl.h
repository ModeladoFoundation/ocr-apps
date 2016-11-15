
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "allocator/firstfit_allocator.h"
#include "allocator/lifo_allocator.h"
#include "allocator/proxy_allocator.h"
#include "common.h"
#include "dependences_decl.h"
#include "event.h"
#include "memory/lazy.h"
#include "taskwait.h"

#include <map>
#include <nanos6_rt_interface.h>

extern "C" typedef void (*run_funct_t)(void* args_block);

namespace ompss {

typedef std::pair<uintptr_t,AccessDependence> dependence_pair;

//#define TREE_TMP_ALLOC
#define TREE_DB_ALLOC

#if defined(TREE_TMP_ALLOC)
typedef buffered_alloc::firstfit_allocator<dependence_pair> tree_allocator;
#elif defined(TREE_DB_ALLOC)
typedef db_allocator<dependence_pair> tree_allocator;
#else
typedef std::allocator<dependence_pair> tree_allocator;
#endif

typedef std::map<
    uintptr_t,
    AccessDependence,
    std::less<uintptr_t>,
    //proxy_allocator<tree_allocator>
    tree_allocator
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
    TaskFlags()
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
    typedef buffered_alloc::lifo_allocator<uint8_t,64U,128U> ParamAllocator;
    typedef buffered_alloc::firstfit_allocator<uint8_t>      ScratchAllocator;

    ParamAllocator::arena_type             paramMemory;
    ScratchAllocator::arena_type<64U,512U> scratchMemory;
    UninitializedTask                      taskMemory;
    DependenceMap                          accesses;
    TaskwaitEvent                          taskwait;
    TaskFlags                              flags;

    TaskScopeInfo();
};

} // namespace ompss

#endif // TASK_DECL_H

