
#ifndef TASKWAIT_H
#define TASKWAIT_H

#include "memory/lazy.h"
#include "event.h"

#include <extensions/ocr-legacy.h>

namespace ompss {

class TaskwaitEvent {
public:
    TaskwaitEvent() :
        _event()
    {
    }

    void registerEdt( const ocrGuid_t& edt ) {
        (*_event)++;
        _event->addDependence(edt);
    }

    void openRegion() {
        _event.reset();
        (*_event)++;
    }

    void closeRegion() {
        (*_event)--;
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

private:
    mem::Lazy<ocr::LatchEvent> _event;
};

} // namespace ompss

#endif // TASKWAIT_H

