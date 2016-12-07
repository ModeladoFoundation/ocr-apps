
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences/task_dependences.h"

#include "common.h"
#include "event.h"
#include "memory/serializer.h"

#include "task/task_scope.h"

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

inline Task::Task( nanos_task_info* info, TaskDefinition& def ) :
    definition(def),
    dependences(info)
{
}

inline Task* Task::factory::construct( nanos_task_info* info, uint32_t args_size )
{
    // We want to allocate TaskDefinition and args_size together, so it
    // is not possible to use Allocator<TaskDefinition>. This is because
    // requested size would be multiplied by sizeof(TaskDefinition), and
    // args_size could not be a multiplier of this value.
    // Default allocation type is uint8_t (unsigned char).
    TaskScopeInfo::ParamAllocator alloc(TaskScopeInfo::getLocalScope().paramMemory);
    // Allocate space for TaskDefinition and arguments buffer.
    void* def_ptr  = alloc.allocate( sizeof(TaskDefinition) + args_size, alignof(TaskDefinition) );
    void* task_ptr = static_cast<void*>(&TaskScopeInfo::getLocalScope().taskMemory);

    // Construct in-place both Task definition and Task instances
    TaskDefinition* def = new (def_ptr) TaskDefinition( info, args_size );
    return new (task_ptr) Task( info, *def );
}

inline void Task::factory::destroy( Task* task )
{
    // Explicitly call destructors because
    // inplace-new was used to construct.
    TaskDefinition* def = &task->definition;
    def->~TaskDefinition();
    task->~Task();

    // Free all temporary storage
    // allocated for this task
    TaskScopeInfo::getLocalScope().paramMemory.clear();
}

inline std::pair<uint32_t,uint64_t*> Task::packParams()
{
    uint64_t* base = reinterpret_cast<uint64_t*>(&definition);
    mem::Serializer serializer( base );
    serializer.advance( sizeof(TaskDefinition) + definition.arguments.size );

    serializer.write<size_t>( dependences.release.size() );

    serializer.write( dependences.release.begin(), dependences.release.end() );

    serializer.write( dependences.rel_destroy_not_satisfy.begin(), dependences.rel_destroy_not_satisfy.end() );

    uint32_t distance = mem::distance<uint64_t>( serializer.position(), base );
    return { distance, base };
}

inline std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> Task::unpackParams( uint32_t paramc, uint64_t* paramv )
{
    mem::Deserializer deserializer( paramv );
    TaskDefinition* definition = deserializer.read<TaskDefinition>();
    deserializer.advance( definition->arguments.size );

    size_t numReleaseDependences = *deserializer.read<size_t>();

    ocrGuid_t* releaseDependences = deserializer.read<ocrGuid_t>(numReleaseDependences);

    uint8_t* destroyFlags = deserializer.read<uint8_t>(numReleaseDependences);

    dbg_check( paramc == mem::distance<uint64_t>( deserializer.position(), paramv ) );

    return { definition, numReleaseDependences, releaseDependences, destroyFlags };
}

inline void acquireDependences( ompss::Task& task )
{
    uint8_t err;

    auto& events  = task.dependences.acquire;
    auto& satisfy = task.dependences.acq_satisfy;
    if( !events.empty() ) {
        // Dependences already registered in EDT creation
        // Just satisfy flagged events
        for( uint32_t slot = 0; slot < events.size(); ++slot ) {
            if( satisfy[slot] ) {
                err = ocrEventSatisfy( events[slot], NULL_GUID );
                ASSERT( err == 0 );
            }
        }
    }
}

inline void releaseDependences( uint32_t num_deps, ocrGuid_t dependences[], uint8_t destroy[] )
{
    log::verbose<log::Module::dependences>( "Releasing dependences" );
    uint8_t err;
    for( uint32_t i = 0; i < num_deps; ++i ) {
        if( destroy[i] ) {
            log::verbose<log::Module::dependences>( "Release: destroy GUID ", std::hex, dependences[i].guid );
            err = ocrEventDestroy( dependences[i] );
            ASSERT( err == 0 );
        } else {
            log::verbose<log::Module::dependences>( "Release: satisfy GUID ", std::hex, dependences[i].guid );
            err = ocrEventSatisfy( dependences[i], NULL_GUID );
            ASSERT( err == 0 );
        }
    }
}


} // namespace ompss

#endif // TASK_H

