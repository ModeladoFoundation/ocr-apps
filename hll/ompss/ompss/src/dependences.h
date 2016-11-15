
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include "debug/fatal.h"
#include "dependences_decl.h"
#include "task_decl.h"

#include "event.h"
#include "task-local.h"

namespace ompss {

inline TaskDependences::TaskDependences( nanos_task_info* info ) :
    register_dependences(info->register_depinfo),
    acquire( vector_type<ocrGuid_t>::allocator_type( getLocalScope().scratchMemory ) ),
    release( acquire.get_allocator() ),
    acq_satisfy( acquire.get_allocator() ),
    rel_destroy_not_satisfy( acquire.get_allocator() )
{
}

inline void AccessDependence::createReadSection()
{
    // If read section event was not previously created
    if( !readCompleted.initialized() ) {
        // Create latch event
        readCompleted.initialize();
        // Increment latch event pre-slot counter
        // to "open" read-only section
        (*readCompleted)++;
    }
}

inline void AccessDependence::readSectionAddReader( ompss::Task& task )
{
    (*readCompleted)++;

    log::verbose<log::Module::dependences>( "Add read event to satisfy list ", std::hex, readCompleted->handle().guid );
    task.dependences.release.push_back(*readCompleted);
    task.dependences.rel_destroy_not_satisfy.push_back(false);
}

inline void AccessDependence::createWriteSection( ompss::Task& task )
{
    auto& event = writeCompleted;
    // Replace previous write event
    if( event.initialized() ) {
        // Destroy old event after EDT execution
        log::verbose<log::Module::dependences>( "Add write event to destroy list ", std::hex, event->handle().guid );
        task.dependences.release.push_back(event->handle());
        task.dependences.rel_destroy_not_satisfy.push_back(true);
        event.release();
    }
    event.initialize();

    // Add this new event to release events list
    log::verbose<log::Module::dependences>( "Add write event to satisfy list ", std::hex, event->handle().guid );
    task.dependences.release.push_back(event->handle());
    task.dependences.rel_destroy_not_satisfy.push_back(false);
}

inline void AccessDependence::addRAWDependence( ompss::Task& task )
{
    auto& event = writeCompleted;
    if( event.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task ", &task, " depends on ", this,
                 "; event GUID: ", event->handle().guid );

        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(false);
    }
}

inline void AccessDependence::addWAWDependence( ompss::Task& task )
{
    auto& event = writeCompleted;
    if( event.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task ", &task, " depends on ", this,
                 "; event GUID: ", event->handle().guid );

        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(false);
    }
}

inline void AccessDependence::addWARDependence( ompss::Task& task )
{
    auto& event = readCompleted;
    if( event.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task ", &task, " depends on ", this,
                 "; event GUID: ", event->handle().guid );

        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(true);

        // Delete read section from dependency map
        event.erase();
    }
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

#endif // DEPENDENCIES_H

