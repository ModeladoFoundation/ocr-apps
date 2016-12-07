
#include "task/task_scope.h"
#include <cstdio>

int main( int argc, char* argv[] ) {
#ifdef RELEASE
    std::printf("#define RELEASE\n");
#endif

    std::printf(
"{\n"
"N\n"
"s/@OMPSS_LOCAL_STRUCT_PLACEHOLDER@/typedef struct {\\\n"
"    _Alignas(%lu) unsigned char _mem[%lu];\\\n"
"} ompss_local_t;/\n"
"}", alignof(ompss::TaskScopeInfo), sizeof(ompss::TaskScopeInfo) );
    return 0;
}

