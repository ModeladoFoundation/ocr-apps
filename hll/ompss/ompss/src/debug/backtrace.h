
#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <array>
#include <ostream>
#include <execinfo.h>

namespace debug {

struct BacktraceDump {
    BacktraceDump( std::ostream& os )
    {
        std::array<void*,32> buffer;
        backtrace( buffer.data(), buffer.size() );
        char** trace_frames = backtrace_symbols( buffer.data(), buffer.size() );
        for( unsigned f = 0; f < buffer.size(); ++f ) {
            os  << "[" << f << "] "
                << trace_frames[f]
                << std::endl;
        }
        free( trace_frames );
    }
};

} // namespace debug

#endif // BACKTRACE_H

