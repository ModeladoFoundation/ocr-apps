
#ifndef EVENT_H
#define EVENT_H

#include "memory/lazy.h"

#include <ocr.h>

namespace ocr {

class EventBase {
protected:
    EventBase() :
        _handle( NULL_GUID )
    {
    }

    ~EventBase() {
        _handle = NULL_GUID;
    }

    // Moving is just copying the GUID
    // and reset the origin GUID to NULL_GUID
    EventBase( EventBase&& other ) :
        _handle( other._handle )
    {
        other._handle = NULL_GUID;
    }

    // Moving is just copying the GUID
    // and reset the origin GUID to NULL_GUID
    EventBase& operator=( EventBase&& other ) {
        _handle = other._handle;
        other._handle = NULL_GUID;
        return *this;
    }

public:
    void addDependence( const EventBase& source ) {
        addDependence( source._handle );
    }

    void addDependence( const ocrGuid_t& source ) {
        addDependenceDecrease( source );
    }

    void addDependenceIncrease( const EventBase& source ) {
        addDependenceIncrease( source._handle );
    }

    void addDependenceIncrease( const ocrGuid_t& source ) {
        uint8_t err = ocrAddDependence( source, _handle,
                    OCR_EVENT_LATCH_INCR_SLOT, DB_DEFAULT_MODE );
        ASSERT( err == 0 );
    }

    void addDependenceDecrease( const EventBase& source ) {
        addDependenceDecrease( source._handle );
    }

    void addDependenceDecrease( const ocrGuid_t& source ) {
        uint8_t err = ocrAddDependence( source, _handle,
                    OCR_EVENT_LATCH_DECR_SLOT, DB_DEFAULT_MODE );
        ASSERT( err == 0 );
    }

    void satisfy() {
        satisfyDecrease();
    }

    void satisfyIncrease() {
        uint8_t err = ocrEventSatisfySlot( _handle,
                    NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
        ASSERT( err == 0 );
    }

    void satisfyDecrease() {
        uint8_t err = ocrEventSatisfySlot( _handle,
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
        uint8_t err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    static ocrEventTypes_t type() {
        return OCR_EVENT_ONCE_T;
    }
};

struct StickyEvent : public EventBase {
    StickyEvent() :
        EventBase()
    {
        uint8_t err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    StickyEvent( const EventBase& ) = delete;

    StickyEvent( StickyEvent&& event ) :
        EventBase( std::move(event) )
    {
    }

    StickyEvent& operator=( const EventBase& ) = delete;

    StickyEvent& operator=( StickyEvent&& ) = default;

    ~StickyEvent() {
        uint8_t err = ocrEventDestroy( handle() );
        ASSERT( err == 0 );
    }

    static ocrEventTypes_t type() {
        return OCR_EVENT_STICKY_T;
    }
};

struct LatchEvent : public EventBase {
    LatchEvent() :
        EventBase()
    {
        uint8_t err = ocrEventCreate( &handle(),
                    type(), EVT_PROP_NONE );
        ASSERT( err == 0 );
    }

    LatchEvent( LatchEvent&& event ) :
        EventBase( std::move(event) )
    {
    }

    // Latch events cant be copied
    LatchEvent( const EventBase& event ) = delete;

    LatchEvent& operator=( LatchEvent&& ) = default;

    // Latch event cant be copy asigned
    LatchEvent& operator=( const EventBase& ) = delete;

    LatchEvent& operator++(int) {
        satisfyIncrease();
        return *this;
    }

    LatchEvent& operator--(int) {
        satisfyDecrease();
        return *this;
    }

    static ocrEventTypes_t type() {
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

