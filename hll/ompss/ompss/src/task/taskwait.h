
#ifndef TASKWAIT_H
#define TASKWAIT_H

#include "memory/lazy.h"
#include "event.h"

#include <extensions/ocr-legacy.h>

namespace ompss {

class TaskwaitEvent {
public:
    TaskwaitEvent() :
        _tasks(0),
        _event()
    {
    }

    TaskwaitEvent( TaskwaitEvent&& other ) :
        _tasks(0),
        _event()
    {
        openRegion();
        addDependence( other );
        other.closeRegion();
    }

    TaskwaitEvent& operator=( const TaskwaitEvent& ) = delete;

    TaskwaitEvent& operator=( TaskwaitEvent&& other ) {
        _tasks = other._tasks;
        *_event = std::move( other.getEvent() );
        return *this;
    }

    ocr::LatchEvent& getEvent() {
        return *_event;
    }

    const ocr::LatchEvent& getEvent() const {
        return *_event;
    }

    void addDependence( const TaskwaitEvent& other ) {
        getEvent()++;
        _event->addDependence( other.getEvent() );
    }

    void registerEdt( const ocrGuid_t& edt ) {
        //if( _tasks > 1024  ) {
        //    // Too many dependences added so far
        //    replace();
        //}
        //++_tasks;
        getEvent()++;
        _event->addDependence(edt);
    }

    void openRegion() {
        _event.reset();
        getEvent()++;
    }

    void closeRegion() {
        getEvent()--;
    }

    void wait() {
        // Prepare sticky event for legazy block progress
        ocr::StickyEvent blockProgressEvent;
        blockProgressEvent.addDependence( *_event );

        // Close taskwait region. May
        // satisfy sticky event
        closeRegion();

        // Open next taskwait region
        openRegion();

        // Wait for sticky event to be satisfied
        // (all successor tasks has been completed)
        uint8_t err = ocrLegacyBlockProgress( blockProgressEvent, NULL, NULL, NULL,
                                      LEGACY_PROP_NONE );
        ASSERT( err == 0 );
    }

    void replace() {
        // Create a replacement instance
        TaskwaitEvent replacement;
        // Add old -> new dependence
        replacement.openRegion();
        replacement.addDependence(*this);
        // Close old region
        closeRegion();
        // and replace old event with new one
        *this = std::move(replacement);
    }

private:
    unsigned long              _tasks;
    mem::Lazy<ocr::LatchEvent> _event;
};

} // namespace ompss

#endif // TASKWAIT_H

