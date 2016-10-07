// \file nqueens.c
// \author Jorge Bellon <jbellonc@intel.com>
//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

static void find_solutions( uint32_t* found, uint32_t all, uint32_t ldiag, uint32_t cols, uint32_t rdiag )
{
    if( cols != all ) {
        uint32_t available = ~( ldiag | cols | rdiag ) & all;
        uint32_t col = available & (-available);

        while( col != 0 ) {
            #pragma omp task
            find_solutions( found, all, (ldiag|col)<<1, cols|col, (rdiag|col)>>1 );

            available = available - col;
            col = available & (-available);
        }
    } else {
        // A solution was found!
        #pragma omp atomic
        *found += 1;
    }
}

void solve_nqueens( uint32_t n )
{
    uint32_t found = 0;

    uint32_t all = (1 << n) - 1;
    #pragma omp parallel default(shared)
    #pragma omp single
    {
        find_solutions( &found, all, 0, 0, 0 );
        #pragma omp taskwait
    }

    printf( "%d-queens; %dx%d; sols: %d\n", n, n, n, found );
}

int main ( int argc, char* argv[] )
{
    assert( argc == 2 );

    uint32_t n = atoi( argv[1] );
    assert( 0 < n && n < 31 );

    solve_nqueens( n );
    return 0;
}
