
#ifndef PROFILE_H
#define PROFILE_H

#if ENABLE_PROFILE

#ifdef _ENABLE_EXTRAE
#  include "extrae-instrumentation.h"
#endif

#include "profile-events.h"

#define PROFILE_BLOCK( event ) \
    profile::InstrumentBlock<profile::Event::event> __this_block;

namespace profile {

template< profile::Event event >
struct InstrumentBlock
#ifdef _ENABLE_EXTRAE
         : public extrae::InstrumentBlock<event>
#endif
{
    InstrumentBlock()
    {
    }

    ~InstrumentBlock()
    {
    }
};

} // namespace profile

#endif // ENABLE_PROFILE

#endif // PROFILE_H

