
#ifndef TASKWAIT_H
#define TASKWAIT_H

#include "memory/lazy.h"
#include "event.h"

#include <extensions/ocr-legacy.h>

namespace ompss {

class TaskwaitEvent {
public:
    ocr::LatchEvent& getEvent() {
        return _event;
    }

    const ocr::LatchEvent& getEvent() const {
        return _event;
    }

    void registerEdt() {
        _event++;
    }

    void openRegion() {
        _event++;
    }

    void closeRegion() {
        _event--;
    }

    void reset() {
        new (&_event) ocr::LatchEvent();
    }

    void wait() {
        // Prepare sticky event for legazy block progress
        ocr::StickyEvent blockProgressEvent;
        blockProgressEvent.addDependence( _event );

        // Close taskwait region. May
        // satisfy sticky event
        closeRegion();

        // Open next taskwait region
        reset();
        openRegion();

        // Wait for sticky event to be satisfied
        // (all successor tasks has been completed)
        uint8_t err = ocrLegacyBlockProgress( blockProgressEvent, NULL, NULL, NULL,
                                      LEGACY_PROP_NONE );
        ASSERT( err == 0 );
    }

private:
    ocr::LatchEvent _event;
};

} // namespace ompss

#endif // TASKWAIT_H

