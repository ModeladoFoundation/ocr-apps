
#ifndef EXTRAE_INSTRUMENTATION_H
#define EXTRAE_INSTRUMENTATION_H

#include "profile-events.h"

#include <extrae.h>

namespace profile {
namespace extrae {

template< Event >
struct EventTraits;

#define DEFINE_EXTRAE_EVENT( _event, _type )                                                 \
    template<>                                                                               \
    struct EventTraits<profile::Event::_event> {                                             \
        static constexpr extrae_type_t type  = static_cast<extrae_type_t>(EventType::_type); \
        static constexpr extrae_value_t value = static_cast<extrae_value_t>(Event::_event);  \
        static constexpr char description[]  = #_event;                                      \
    };

DEFINE_EXTRAE_EVENT( nanos_register_read_depinfo,      api );
DEFINE_EXTRAE_EVENT( nanos_register_readwrite_depinfo, api );
DEFINE_EXTRAE_EVENT( nanos_create_task,                api );
DEFINE_EXTRAE_EVENT( nanos_submit_task,                api );
DEFINE_EXTRAE_EVENT( nanos_taskwait,                   api );

template< profile::Event event >
struct InstrumentBlock {
    InstrumentBlock()
    {
        Extrae_event( EventTraits<event>::type, EventTraits<event>::value );
    }

    ~InstrumentBlock()
    {
        Extrae_event( EventTraits<event>::type, 0 );
    }

private:
    extrae_type_t  _eventType;
};

struct InstrumentPoint {
    InstrumentPoint( extrae_type_t type, extrae_value_t value )
    {
        Extrae_event( type, value );
    }
};

} // namespace extrae
} // namespace profile

#endif

