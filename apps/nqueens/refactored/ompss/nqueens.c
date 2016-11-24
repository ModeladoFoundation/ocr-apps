// \file nqueens.c
// \author Jorge Bellon <jbellonc@intel.com>
//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned solutions = 0U;

static inline void solution_found(void) {
    __atomic_fetch_add( &solutions, 1u, __ATOMIC_RELAXED );
}

static inline unsigned get_solution_number(void) {
    unsigned value;
    __atomic_load( &solutions, &value, __ATOMIC_SEQ_CST );
    return value;
}

// Computes Hamming-weight for an arbitrary integer
static inline uint32_t NumberOfSetBits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void find_solutions( uint32_t maxBitsSet, uint32_t all, uint32_t ldiag, uint32_t cols, uint32_t rdiag, char final )
{
    if( cols != all ) {
        uint32_t available = ~( ldiag | cols | rdiag ) & all;
        uint32_t spot = available & (-available);

        while( spot != 0 ) {
            // Final clause is implemented manually
            // (still no compiler support for nanos6 API)
            final = NumberOfSetBits(cols) > maxBitsSet;

            if( final ) {
                find_solutions( maxBitsSet, all, (ldiag|spot)<<1, cols|spot, (rdiag|spot)>>1, final );
            } else {
                #pragma omp task
                find_solutions( maxBitsSet, all, (ldiag|spot)<<1, cols|spot, (rdiag|spot)>>1, final );
            }

            available = available - spot;
            spot = available & (-available);
        }
    } else {
        // A solution was found!
        solution_found();
    }
}

void solve_nqueens( uint32_t n, uint32_t cutoff )
{
    uint32_t all = (1 << n) - 1;
    find_solutions( n-cutoff, all, 0, 0, 0, 0 );
    #pragma omp taskwait

    printf( "%d-queens; %dx%d; sols: %d\n", n, n, n, get_solution_number() );
}

int ompss_user_main ( int argc, char* argv[] )
{
    if( argc != 3 ) {
        printf("Usage %s size cutoff", argv[0] );
        return EXIT_FAILURE;
    }

    uint32_t n = atoi( argv[1] );
    uint32_t cutoff = atoi( argv[2] );
    assert( 0 < n && n < 31 );
    assert( cutoff < n );

    solve_nqueens( n, cutoff );
    return 0;
}
