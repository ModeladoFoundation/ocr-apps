
#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
#include <iostream>

namespace ompss {

template <
    typename N,
    typename F
>
struct ProfileBlock {
    ProfileBlock( N&& name, F&& file, int line ) :
        _name(name), _file(file), _line(line)
    {
        std::cout << "Starting block; func: " << _name << " @" << _file << ":" << _line << std::endl;
    }

    ~ProfileBlock() {
        std::cout << "End of block; func: " << _name << " @" << _file << ":" << _line << std::endl;
    }

    N _name;
    F _file;
    int _line;
};

template < typename N, typename F >
inline ProfileBlock<N,F> profile_block( N&& name, F&& file, int line )
{
    return ProfileBlock<N,F>(std::forward<N>(name),std::forward<F>(file),line);
}

} // namespace ompss

#if 0
#define PROFILE_BLOCK \
    auto _b_##__LINE__ = ::ompss::profile_block(__func__,__FILE__,__LINE__);
#else
#define PROFILE_BLOCK
#endif

#else  // __cplusplus

#define PROFILE_BLOCK

#endif // __cplusplus
#endif // DEBUG_H

