
#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

#include <type_traits>

namespace log {

enum class Module {
    dependences,
    memory,
    any
};

enum class Verbosity {
    message = 0,
    warning,
    log,
    verbose
};

template< Module module>
struct module_verbosity {
    static constexpr Verbosity max = Verbosity::message;
};

template< Module mod, Verbosity lv >
struct log_enabled : public std::integral_constant<bool, (module_verbosity<mod>::max >= lv) >
{
};

#ifndef NDEBUG

#define ENABLE_LOG( module, level )             \
    template<>                                  \
    struct module_verbosity<module> {     \
        static constexpr Verbosity max = level; \
    };

// Here follows enabled modules and their verbosity level
ENABLE_LOG( Module::dependences, Verbosity::log );

#endif // NDEBUG

} // namespace log

#endif // LOG_LEVEL_H

