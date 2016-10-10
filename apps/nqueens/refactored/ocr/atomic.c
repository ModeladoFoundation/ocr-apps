
#include <stdatomic.h>

atomic_uint solutions = ATOMIC_VAR_INIT(0U);

void solution_found()
{
    atomic_fetch_add( &solutions, 1 );
}

unsigned get_solution_number()
{
    return atomic_load( &solutions );
}

