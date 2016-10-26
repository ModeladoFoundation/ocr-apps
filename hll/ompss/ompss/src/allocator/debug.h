
#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>
#include <memory>

namespace debug {

template< typename FuncName, typename FileName >
struct function {
    function( std::string function, std::string file, int line ) :
        _function(function), _file(file), _line(line)
    {
        std::cout << "[Enter] " << _function
                  << " @ " << _file << ":" << _line
                  << std::endl;
    }

    ~function() {
        std::cout << "[Exit]  " << _function
                  << " @ " << _file << ":" << _line
                  << std::endl;
    }

    std::string _function;
    std::string _file;
    int         _line;
};

template<typename FuncName, typename FileName>
std::unique_ptr<function<FuncName,FileName>> log_function( FuncName func, FileName file, int line )
{
    std::unique_ptr<function<FuncName,FileName>> ptr(
            new function<FuncName,FileName>(func,file,line) );
    return ptr;
}

} // namespace debug

#ifndef NDEBUG
#define LOG_FUNCTION \
    auto function_log = debug::log_function(__PRETTY_FUNCTION__,__FILE__,__LINE__);
#else
#define LOG_FUNCTION
#endif

#endif // DEBUG_H

