
#ifndef TRACEBLOCK_H
#define TRACEBLOCK_H

#include <iostream>

namespace debug {

template <
    typename N,
    typename F
>
struct TraceBlock {
    TraceBlock( N&& name, F&& file, int line ) :
        _name(name), _file(file), _line(line)
    {
        std::cout << "Starting block; func: " << _name << " @" << _file << ":" << _line << std::endl;
    }

    ~TraceBlock() {
        std::cout << "End of block; func: " << _name << " @" << _file << ":" << _line << std::endl;
    }

    N _name;
    F _file;
    int _line;
};

template < typename N, typename F >
inline TraceBlock<N,F> trace_block( N&& name, F&& file, int line )
{
    return TraceBlock<N,F>(std::forward<N>(name),std::forward<F>(file),line);
}

} // namespace debug

#if 0
#define PROFILE_BLOCK \
    auto _b_##__LINE__ = ::ompss::trace_block(__func__,__FILE__,__LINE__);
#else
#define PROFILE_BLOCK
#endif


#endif // TRACEBLOCK_H
