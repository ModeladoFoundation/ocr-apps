
#ifndef OUTPUT_H
#define OUTPUT_H

#include "debug/loglevel.h"

#ifndef RELEASE
#include <atomic>
#include <fstream>
#include <iostream>
#include <cstdio>
#endif

#include <type_traits>
#include <utility>

namespace log {

#ifdef RELEASE

// In release, log is disabled
template< typename... Args >
inline void join( Args... args ) {
}

#else

template< bool B, typename... Args >
void join( const std::integral_constant<bool,B>&, Args... args );

template< typename... Args >
inline void join( const std::false_type&, Args... args ) {
}

inline unsigned tid() {
    static std::atomic<unsigned> tid = ATOMIC_VAR_INIT(0);
    static thread_local unsigned self = ~0U;
    if( self == ~0U )
        self = tid++;
    return self;
}

struct Stream : public std::fstream {
    Stream() :
        std::fstream( getFileName(), std::ios_base::out|std::ios_base::trunc )
    {
    }

    static Stream& getLocalStream() {
        static thread_local Stream privateStream;
        return privateStream;
    }

private:
    static const char* getFileName() {
        static thread_local bool initialized = false;
        static thread_local char name[10];
        if( !initialized ) {
            std::snprintf( name, 10, "out_%u", tid() );
        }
        return name;
    }
};

template< typename First, typename... Args >
inline void join( const std::true_type& enabled, First first, Args... args ) {
    if( tid() == 0 ) {
        //std::ostream& os = Stream::getLocalStream();
        std::ostream& os = std::cout;
        os << std::forward<First>(first);
        join( enabled, std::forward<Args>(args)... );
    }
}

template<>
inline void join( const std::true_type& ) {
    if( tid() == 0 ) {
        //std::ostream& os = Stream::getLocalStream();
        std::ostream& os = std::cout;
        os << std::endl;
    }
}

#endif // RELEASE

// Desagregation by log level
template< Module module, typename... Args >
inline void message( Args... args ) {
    join( log_enabled<module,Verbosity::message>(),
          std::forward<Args>(args)... );
}

template< Module module, typename... Args >
inline void warning( Args... args ) {
    join( log_enabled<module,Verbosity::warning>(),
          std::forward<Args>(args)... );
}

template< Module module, typename... Args >
inline void log( Args... args ) {
    join( log_enabled<module,Verbosity::log>(),
          std::forward<Args>(args)... );
}

template< Module module, typename... Args >
inline void verbose( Args... args ) {
    join( log_enabled<module,Verbosity::verbose>(),
          std::forward<Args>(args)... );
}

} // namespace log

#endif // OUTPUT_H

