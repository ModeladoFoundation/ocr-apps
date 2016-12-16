
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
        static const char description[];                                                     \
    };

DEFINE_EXTRAE_EVENT( nanos_register_read_depinfo,      api );
DEFINE_EXTRAE_EVENT( nanos_register_readwrite_depinfo, api );
DEFINE_EXTRAE_EVENT( nanos_create_task,                api );
DEFINE_EXTRAE_EVENT( nanos_submit_task,                api );
DEFINE_EXTRAE_EVENT( nanos_taskwait,                   api );
DEFINE_EXTRAE_EVENT( mainEdt,                          api );
DEFINE_EXTRAE_EVENT( outlineEdt,                       api );
DEFINE_EXTRAE_EVENT( shutdownEdt,                      api );
DEFINE_EXTRAE_EVENT( ompss_user_code,                  user_code );

static inline void register_events()
{
    {
    extrae_type_t type = static_cast<extrae_type_t>(profile::EventType::api);
    const char description[] = "Nanos6 Runtime API";
    const unsigned size = 9;
    extrae_value_t values[] = {
        EventTraits<Event::nanos_register_read_depinfo>::value,
        EventTraits<Event::nanos_register_readwrite_depinfo>::value,
        EventTraits<Event::nanos_create_task>::value,
        EventTraits<Event::nanos_submit_task>::value,
        EventTraits<Event::nanos_taskwait>::value,
        EventTraits<Event::mainEdt>::value,
        EventTraits<Event::outlineEdt>::value,
        EventTraits<Event::shutdownEdt>::value,
        static_cast<extrae_value_t>(0U)/*END*/ };
    const char* description_values[] = {
        EventTraits<Event::nanos_register_read_depinfo>::description,
        EventTraits<Event::nanos_register_readwrite_depinfo>::description,
        EventTraits<Event::nanos_create_task>::description,
        EventTraits<Event::nanos_submit_task>::description,
        EventTraits<Event::nanos_taskwait>::description,
        EventTraits<Event::mainEdt>::description,
        EventTraits<Event::outlineEdt>::description,
        EventTraits<Event::shutdownEdt>::description,
        "END" };
    Extrae_define_event_type( &type, const_cast<char*>(description),
                              const_cast<unsigned*>(&size), values, const_cast<char**>(description_values) );
    }
    {
    extrae_type_t type = EventTraits<Event::ompss_user_code>::type;
    const char description[] = "OmpSs user code";
    const unsigned size = 2;
    extrae_value_t values[] = {
        EventTraits<Event::ompss_user_code>::value,
        static_cast<extrae_value_t>(0U)/*END*/ };
    const char* description_values[] = {
        EventTraits<Event::ompss_user_code>::description,
        "END" };
    Extrae_define_event_type( &type, const_cast<char*>(description),
                              const_cast<unsigned*>(&size), values, const_cast<char**>(description_values) );
    }
}

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
};

} // namespace extrae
} // namespace profile

#endif

