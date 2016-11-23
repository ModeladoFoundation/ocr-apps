
#include "complex.h"
#include "fft.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

// Computes reverse addressing of k
// Must specify most significant bit possition
// to avoid buffer overflow
//
// Could be optimized for different values of msb, where
// other kinds of register rotation and bitwise shift
// could be applied.
//
// Based on D. E. Knuth bit reversal algorithm
inline unsigned fft_reverse( unsigned a, unsigned msb )
{
    unsigned t;
    a = (a << 15) | (a >> 17);
    t = (a ^ (a >> 10)) & 0x003f801f;
    a = (t + (t << 10)) ^ a;
    t = (a ^ (a >>  4)) & 0x0e038421;
    a = (t + (t <<  4)) ^ a;
    t = (a ^ (a >>  2)) & 0x22488842;
    a = (t + (t <<  2)) ^ a;
    return a >> (32-msb);
}

// Shuffles an array
// Swaps positions using reverse addressing
inline void fft_shuffle( unsigned N, Complex* X )
{
    const unsigned log2N = log2(N);
    for( unsigned k = 0; k < N; ++k ) {
        unsigned j = fft_reverse(k,log2N);
        if( k < j ) {
            std::swap( X[k], X[j] );
        }
    }
}

// Fast Fourier Transform
// Divide and conquer recursive version using DIF
// (decimation in frequency)
// Output data is not ordered. It must be shuffled
// afterwards.
inline void fft_stage( unsigned stage, unsigned N, Complex* X, const Complex* W )
{
    if( N > 1 ) {
        for( unsigned k = 0; k < N/2; ++k ) {
            const Complex X0 = X[k];
            const Complex X1 = X[k+N/2];
            X[k]     =  X0 + X1;
            X[k+N/2] = (X0 - X1) * W[k*stage];
        }

        #pragma omp task
        fft_stage( stage*2, N/2, X, W );
        #pragma omp task
        fft_stage( stage*2, N/2, X+N/2, W );
    }
}

inline void fft( unsigned N, Complex* X )
{
    const Complex* W = compute_twiddle_table(N);

    double start = take_time();

    fft_stage( 1, N, X, W );
    #pragma omp taskwait

    double stop = take_time();
    std::cout << "Time: " << stop - start << " ms" << std::endl;

    fft_shuffle( N, X );
}

extern "C" int ompss_user_main(int argc, char* argv[])
{
    if( argc < 2 ) {
        std::cout
            << "Usage " << argv[0]
            << " log2(size)" << std::endl;
        return -1;
    }
    const unsigned log2N = atoi(argv[1]);
    assert( log2N < sizeof(unsigned)*8 );
    const unsigned N = 1UL << log2N;
    const char* filename = argc>2? argv[2] : NULL;

    Complex* X = new Complex[N];
    generate_input(N, X);

    fft( N, X );

    if( filename )
        write( N, X, filename );

    //delete[] X;

    return 0;
}

