
#ifndef EVENT_H
#define EVENT_H

#include <ocr.h>
#include "lazy.h"

namespace ompss {

class EventBase {
protected:
    EventBase() :
        _handle( NULL_GUID )
    {
    }

    virtual ~EventBase()
    {
        _handle = NULL_GUID;
    }

public:
    void addDependence( const ocrGuid_t& source ) {
        addDependenceDecrease( source );
    }

    void addDependenceIncrease( const ocrGuid_t& source ) {
        u8 err = ocrAddDependence( source, _handle,
                    OCR_EVENT_LATCH_INCR_SLOT, DB_DEFAULT_MODE );
    }

    void addDependenceDecrease( const ocrGuid_t& source ) {
        u8 err = ocrAddDependence( source, _handle,
                    OCR_EVENT_LATCH_DECR_SLOT, DB_DEFAULT_MODE );
    }

    void satisfy() {
        satisfyDecrease();
    }

    void satisfyIncrease() {
        u8 err = ocrEventSatisfySlot( _handle,
                    NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
        ASSERT( err == 0 );
    }

    void satisfyDecrease() {
        u8 err = ocrEventSatisfySlot( _handle,
                    NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
        ASSERT( err == 0 );
    }

    operator ocrGuid_t&() {
        return _handle;
    }

    ocrGuid_t& handle() {
        return _handle;
    }

    const ocrGuid_t& handle() const {
        return _handle;
    }

private:
    ocrGuid_t _handle;
};

struct OnceEvent : public EventBase {
    OnceEvent() :
        EventBase()
    {
        reset();
    }

    void reset() {
        u8 err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    static ocrEventTypes_t type()
    {
        return OCR_EVENT_ONCE_T;
    }
};

struct StickyEvent : public EventBase {
    StickyEvent() :
        EventBase()
    {
        u8 err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    virtual ~StickyEvent() {
        u8 err = ocrEventDestroy( handle() );
        ASSERT( err == 0 );
    }

    void reset() {
        u8 err = ocrEventDestroy( handle() );
        ASSERT( err == 0 );

        err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    static ocrEventTypes_t type()
    {
        return OCR_EVENT_STICKY_T;
    }
};

struct LatchEvent : public EventBase {
    LatchEvent() :
        EventBase()
    {
        reset();
    }

    virtual ~LatchEvent() {
    }

    void reset() {
        u8 err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    LatchEvent& operator++(int) {
        satisfyIncrease();
        return *this;
    }

    LatchEvent& operator--(int) {
        satisfyDecrease();
        return *this;
    }

    static ocrEventTypes_t type()
    {
        return OCR_EVENT_LATCH_T;
    }
};

template<>
class IsGuidBased<EventBase> : public std::true_type {};

template<>
class IsGuidBased<OnceEvent> : public std::true_type {};

template<>
class IsGuidBased<StickyEvent> : public std::true_type {};

template<>
class IsGuidBased<LatchEvent> : public std::true_type {};

} // namespace ompss

#endif

