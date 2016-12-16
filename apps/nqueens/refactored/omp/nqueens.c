// \file nqueens.c
// \author Jorge Bellon <jbellonc@intel.com>
//

#include "timer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

// Computes Hamming-weight for an arbitrary integer
static inline uint32_t NumberOfSetBits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static void find_solutions( uint32_t* found, uint32_t all, uint32_t ldiag, uint32_t cols, uint32_t rdiag )
{
    if( cols != all ) {
        uint32_t available = ~( ldiag | cols | rdiag ) & all;
        uint32_t spot = available & (-available);

        while( spot != 0 ) {
            #pragma omp task final( NumberOfSetBits(cols) > 4 )
            find_solutions( found, all, (ldiag|spot)<<1, cols|spot, (rdiag|spot)>>1 );

            available = available - spot;
            spot = available & (-available);
        }
    } else {
        // A solution was found!
        #pragma omp atomic
        *found += 1;
    }
}

void solve_nqueens( uint32_t n )
{
    timestamp_t start, stop;
    uint32_t result;
    uint32_t found = 0;
    uint32_t all = (1 << n) - 1;

    get_time(&start);
    #pragma omp parallel default(shared)
    #pragma omp single
    {
        find_solutions( &found, all, 0, 0, 0 );
        #pragma omp taskwait
    }
    get_time(&stop);

    #pragma omp atomic read
    result = found;

    printf( "%d-queens; %dx%d; sols: %d\n", n, n, n, result );
    summary_throughput_timer(&start,&stop,1);
}

int main ( int argc, char* argv[] )
{
    assert( argc == 2 );

    uint32_t n = atoi( argv[1] );
    assert( n < 31 );

    solve_nqueens( n );
    return 0;
}
