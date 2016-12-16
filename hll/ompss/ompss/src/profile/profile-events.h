
#ifndef PROFILE_EVENTS_H
#define PROFILE_EVENTS_H

namespace profile {

enum class EventType : unsigned {
    api       = 0xa500,
    user_code = 0xa500 + (1U<<4),
    /* new event = 0xa500 + (2U<<4), etc. */
    invalid   = ~0U
};

constexpr unsigned startingValue( EventType type ) {
    return static_cast<unsigned>(type);
}

enum class Event : unsigned {
    nanos_register_read_depinfo       = startingValue( EventType::api ),
    nanos_register_readwrite_depinfo,
    nanos_create_task,
    nanos_submit_task,
    nanos_taskwait,
    mainEdt,
    outlineEdt,
    shutdownEdt,
    ompss_user_code                   = startingValue( EventType::user_code ),
    undefined                         = startingValue( EventType::invalid )
};



} // namespace profile

#endif // PROFILE_EVENTS_H

