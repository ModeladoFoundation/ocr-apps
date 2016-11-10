
#ifndef FATAL_H
#define FATAL_H

#include "backtrace.h"

#include <exception>
#include <iostream>

namespace debug {

template< typename ErrorType >
struct FatalError : public BacktraceDump {
    FatalError( const char* message, const char* function,
                const char* file, unsigned line ) :
        BacktraceDump( std::cerr )
    {
        std::cerr
            << "Fatal error: "
            << message
            << " in " << function
            << " @ " << file
            << ":" << line
            << std::endl;

        std::terminate();
    }
};

struct AssertionFailed : public FatalError<AssertionFailed> {
    AssertionFailed( const char* condition, const char* function,
                     const char* file, unsigned line ) :
        FatalError( condition, function, file, line )
    {
    }
};

template< typename FunctionType, typename FileType >
inline void check_condition( bool passed, const char* condition,
        const FunctionType& function, const FileType& file, unsigned line ) {
    if( !passed ) {
        AssertionFailed( condition, function, file, line );
    }
}

#ifndef NDEBUG

#   define dbg_check(cond) \
       debug::check_condition( cond, #cond " failed ", __func__, __FILE__, __LINE__ )

#else

#  define dbg_check(cond)

#endif

} // namespace debug

#endif // FATAL_H

