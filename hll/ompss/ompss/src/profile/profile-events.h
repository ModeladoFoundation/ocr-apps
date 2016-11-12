
#ifndef PROFILE_EVENTS_H
#define PROFILE_EVENTS_H

namespace profile {

enum class EventType : unsigned {
    api = 0xa500,
    invalid
};

constexpr unsigned startingValue( EventType type ) {
    return static_cast<unsigned>(type) << 16;
}

enum class Event : unsigned {
    nanos_register_read_depinfo       = startingValue( EventType::api ),
    nanos_register_readwrite_depinfo,
    nanos_create_task,
    nanos_submit_task,
    nanos_taskwait,
    undefined                         = startingValue( EventType::invalid )
};

} // namespace profile

#endif // PROFILE_EVENTS_H

