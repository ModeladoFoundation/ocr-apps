
#ifndef PROFILE_H
#define PROFILE_H

#include "extrae-instrumentation.h"
#include "profile-events.h"

#define PROFILE_BLOCK( event ) \
    profile::InstrumentBlock<profile::Event::event> __this_block;

namespace profile {

template< profile::Event event >
struct InstrumentBlock : public extrae::InstrumentBlock<event> {
    InstrumentBlock()
    {
    }

    ~InstrumentBlock()
    {
    }
};

} // namespace profile

#endif // PROFILE_H

