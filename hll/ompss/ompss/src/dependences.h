
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include "dependences_decl.h"
#include "task_decl.h"

#include "event.h"
#include "task-local.h"

namespace ompss {

static inline void createReadSection( ompss::AccessDependence& dep )
{
    // If read section event was not previously created
    if( !dep.readCompleted.initialized() ) {
        // Create latch event
        dep.readCompleted.initialize();
        // Increment latch event pre-slot counter
        // to "open" read-only section
        (*dep.readCompleted).satisfyIncrease();
    }
}

static inline void readSectionAddReader( ompss::Task& task, ompss::AccessDependence& dep )
{
    (*dep.readCompleted)++;

    task.dependences.events.push_back(*dep.readCompleted);
    task.dependences.eventTypes.push_back(ompss::LatchEvent::type());
}

static inline void createWriteSection( ompss::Task& task, ompss::AccessDependence& dep )
{
    // Replace previous write event regardless it existed or not
    dep.writeCompleted = ompss::StickyEvent();

    task.dependences.events.push_back(*dep.readCompleted);
    task.dependences.eventTypes.push_back(ompss::StickyEvent::type());
}

static inline void addDependencyRAW( ompss::Task& task, ompss::AccessDependence& dep )
{
    if( dep.writeCompleted.initialized() ) {
        task.dependences.events.push_back(*dep.writeCompleted);
        task.dependences.eventTypes.push_back(ompss::StickyEvent::type());
    }
}

static inline void addDependencyWAW( ompss::Task& task, ompss::AccessDependence& dep )
{
    if( dep.writeCompleted.initialized() ) {
        task.dependences.events.push_back(*dep.writeCompleted);
        task.dependences.eventTypes.push_back(ompss::StickyEvent::type());
    }
}

static inline void addDependencyWAR( ompss::Task& task, ompss::AccessDependence& dep )
{
    if( dep.readCompleted.initialized() ) {
        task.dependences.events.push_back(*dep.readCompleted);
        task.dependences.eventTypes.push_back(ompss::LatchEvent::type());

        // Delete read section from dependency map
        dep.readCompleted.reset();
    }
}

static inline void acquireDependences( const ocrGuid_t& edt, const ocrGuid_t& task_db, ompss::Task& task )
{
    u8 err;
    err = ocrAddDependence( task_db, edt,
                            OCR_EVENT_LATCH_DECR_SLOT, DB_DEFAULT_MODE );
    ASSERT( err == 0 );

    ompss::GuidVector& events     = task.dependences.events;
    ompss::TypeVector& eventTypes = task.dependences.eventTypes;
    if( !events.empty() ) {
        // Task is using dependences.
        // Parent (running task) shall postpone its clean-up
        getLocalScope().flags.postponeCleanup = true;

        // Register dependences
        auto eventIterator = events.begin();
        auto typeIterator = eventTypes.begin();
        u32 slot = 1;// position of the depv array
        while( eventIterator != events.end() ) {
            err = ocrAddDependence( *eventIterator, edt, slot, DB_MODE_RW/*default mode*/ );
            ASSERT( err == 0 );

            if( *typeIterator == OCR_EVENT_LATCH_T ) {
                err = ocrEventSatisfy( *eventIterator, NULL_GUID );
                ASSERT( err == 0 );
            }

            ++eventIterator;
            ++typeIterator;
        }
    }
}

static inline void releaseDependences( ompss::Task& task )
{
    u8 err;
    ompss::GuidVector& events     = task.dependences.events;
    ompss::TypeVector& eventTypes = task.dependences.eventTypes;

    auto eventIterator = events.begin();
    auto typeIterator = eventTypes.begin();
    while( eventIterator != events.end() ) {
        switch( *typeIterator ) {
            case OCR_EVENT_LATCH_T:
                err = ocrEventSatisfy( *eventIterator, NULL_GUID );
                ASSERT( err == 0 );
                break;
            case OCR_EVENT_STICKY_T:
                err = ocrEventDestroy( *eventIterator );
                ASSERT( err == 0 );
                break;
            default:
                ASSERT( FALSE );
        }
    }
}

} // namespace ompss

#endif // DEPENDENCIES_H

