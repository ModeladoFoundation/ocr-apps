// \file nqueens.c
// \author Jorge Bellon <jbellonc@intel.com>
//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void solution_found(void);
int get_solution_number(void);

// Computes Hamming-weight for an arbitrary integer
static inline uint32_t NumberOfSetBits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static void find_solutions( uint32_t all, uint32_t ldiag, uint32_t cols, uint32_t rdiag, char final )
{
    if( cols != all ) {
        uint32_t available = ~( ldiag | cols | rdiag ) & all;
        uint32_t spot = available & (-available);

        while( spot != 0 ) {
            // Final clause is implemented manually
            // (still no compiler support for nanos6 API)
            if( final ) {
                find_solutions( all, (ldiag|spot)<<1, cols|spot, (rdiag|spot)>>1, final );
            } else {
                #pragma omp task
                find_solutions( all, (ldiag|spot)<<1, cols|spot, (rdiag|spot)>>1, /* final condition: */ NumberOfSetBits(cols) > 4 );
            }

            available = available - spot;
            spot = available & (-available);
        }
    } else {
        // A solution was found!
        solution_found();
    }
}

void solve_nqueens( uint32_t n )
{
    uint32_t all = (1 << n) - 1;
    find_solutions( all, 0, 0, 0, 0 );
    #pragma omp taskwait

    printf( "%d-queens; %dx%d; sols: %d\n", n, n, n, get_solution_number() );
}

int ompss_user_main ( int argc, char* argv[] )
{
    assert( argc == 2 );

    uint32_t n = atoi( argv[1] );
    assert( 0 < n && n < 31 );

    solve_nqueens( n );
    return 0;
}
