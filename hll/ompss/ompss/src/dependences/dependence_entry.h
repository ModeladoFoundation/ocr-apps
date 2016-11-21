
#ifndef DEPENDENCE_ENTRY_H
#define DEPENDENCE_ENTRY_H

#include "task/task_fwd.h"

#include "dependences/task_dependences_decl.h"
#include "event.h"
#include "memory/lazy.h"

namespace ompss {

// Value of the Map
struct DependenceEntry {
    /*! Latch event that triggers when all read-only accesses are completed
     *  AND the read-only section is over (a write access
     *  has been registered). */
    mem::Lazy<ocr::LatchEvent>  readCompleted;

    /*! Sticky event that triggers when a write-only or read-write access is
     *  completed. */
    mem::Lazy<ocr::StickyEvent> writeCompleted;

    template <AccessTypes type>
    void openSection( TaskDependences& dependences );

    template <AccessTypes type>
    void addDependence( TaskDependences& dependences );
};

template <>
inline void DependenceEntry::openSection<AccessTypes::read_only>( TaskDependences& dependences ) {
    // If read section event was not previously created
    if( !readCompleted.initialized() ) {
        // Create latch event
        readCompleted.initialize();
        // Increment latch event pre-slot counter
        // to "open" read-only section
        (*readCompleted)++;
    }
}

template <>
inline void DependenceEntry::openSection<AccessTypes::read_write>( TaskDependences& dependences ) {
    // Replace previous write event
    if( writeCompleted.initialized() ) {
        // Destroy old event after EDT execution
        log::verbose<log::Module::dependences>( "Add write event to destroy list ", std::hex, writeCompleted->handle().guid );
        dependences.release.push_back(writeCompleted->handle());
        dependences.rel_destroy_not_satisfy.push_back(true);
        writeCompleted.release();
    }
    writeCompleted.initialize();

    // Add this new event to release events list
    log::verbose<log::Module::dependences>( "Add write event to satisfy list ", std::hex, writeCompleted->handle().guid );
    dependences.release.push_back(writeCompleted->handle());
    dependences.rel_destroy_not_satisfy.push_back(false);
}

template <>
inline void DependenceEntry::addDependence<AccessTypes::read_only>( TaskDependences& dependences )
{
    if( writeCompleted.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task depends on ", this,
                 "; event GUID: ", writeCompleted->handle().guid );

        dependences.acquire.push_back(writeCompleted->handle());
        dependences.acq_satisfy.push_back(false);
    }

    // Increase the reader section count
    if( !readCompleted.initialized() ) {
        openSection<AccessTypes::read_only>(dependences);
    }
    (*readCompleted)++;

    log::verbose<log::Module::dependences>( "Add read event to satisfy list ", std::hex, readCompleted->handle().guid );
    dependences.release.push_back(*readCompleted);
    dependences.rel_destroy_not_satisfy.push_back(false);
}

template <>
inline void DependenceEntry::addDependence<AccessTypes::read_write>( TaskDependences& dependences ) {
    // Write after write dependences
    if( writeCompleted.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task depends on ", this,
                 "; event GUID: ", writeCompleted->handle().guid );

        dependences.acquire.push_back(writeCompleted->handle());
        dependences.acq_satisfy.push_back(false);
    }

    // Open a new write section (replaces precedent if exists)
    openSection<AccessTypes::read_write>(dependences);

    // Write after read dependences
    if( readCompleted.initialized() ) {
        log::verbose<log::Module::dependences>( " = Task depends on ", this,
                 "; event GUID: ", readCompleted->handle().guid );

        dependences.acquire.push_back(readCompleted->handle());
        dependences.acq_satisfy.push_back(true);

        // Delete read section from dependency map
        readCompleted.erase();
    }
}

} // namespace ompss

#endif // DEPENDENCE_ENTRY_H

