#ifndef weak_alias

#define weak_alias(name, aliasname) \
    extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

#endif

#include_next <_ansi.h>

