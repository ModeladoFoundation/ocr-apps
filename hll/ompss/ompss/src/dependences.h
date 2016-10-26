
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include "dependences_decl.h"
#include "task_decl.h"

#include "event.h"
#include "task-local.h"

namespace ompss {

inline TaskDependences::TaskDependences( nanos_task_info* info ) :
    register_dependences(info->register_depinfo),
    acquire(),
    release(),
    acq_satisfy(),
    rel_destroy_not_satisfy()
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

    task.dependences.release.push_back(*readCompleted);
    task.dependences.rel_destroy_not_satisfy.push_back(false);
}

inline void AccessDependence::createWriteSection( ompss::Task& task )
{
    auto& event = writeCompleted;
    // Replace previous write event
    if( event.initialized() ) {
        // Destroy old event after EDT execution
        task.dependences.release.push_back(event->handle());
        task.dependences.rel_destroy_not_satisfy.push_back(true);
        event->reset();
    } else {
        event.initialize();
    }
    // Add this new event to release events list
    task.dependences.release.push_back(event->handle());
    task.dependences.rel_destroy_not_satisfy.push_back(false);
}

inline void AccessDependence::addRAWDependence( ompss::Task& task )
{
    auto& event = writeCompleted;
    if( event.initialized() ) {
        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(false);
    }
}

inline void AccessDependence::addWAWDependence( ompss::Task& task )
{
    auto& event = writeCompleted;
    if( event.initialized() ) {
        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(false);
    }
}

inline void AccessDependence::addWARDependence( ompss::Task& task )
{
    auto& event = readCompleted;
    if( event.initialized() ) {
        task.dependences.acquire.push_back(event->handle());
        task.dependences.acq_satisfy.push_back(true);

        // Delete read section from dependency map
        event->reset();
    }
}

static inline void acquireDependences( ompss::Task& task )
{
    uint8_t err;

    std::vector<ocrGuid_t>& events = task.dependences.acquire;
    std::vector<unsigned char>& satisfy = task.dependences.acq_satisfy;
    if( !events.empty() ) {
        // Task is using dependences.
        // Parent (running task) shall postpone its clean-up
        getLocalScope().flags.postponeCleanup = true;

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

static inline void releaseDependences( uint32_t num_deps, ocrGuid_t dependences[], uint8_t destroy[] )
{
    uint8_t err;
    for( uint32_t i = 0; i < num_deps; ++i ) {
        if( destroy[i] ) {
            err = ocrEventDestroy( dependences[i] );
            ASSERT( err == 0 );
        } else {
            err = ocrEventSatisfy( dependences[i], NULL_GUID );
            ASSERT( err == 0 );
        }
    }
}

} // namespace ompss

#endif // DEPENDENCIES_H

