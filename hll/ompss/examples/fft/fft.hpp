
#ifndef FFT_H
#define FFT_H

#include "complex.h"

#include <cmath>

#include <memory>
#include <iostream>

#include <sys/time.h>

const float PI = 3.141592653589793238460f;

inline double take_time() {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    gettimeofday( &tv, NULL );
    return tv.tv_sec*1000.0 + tv.tv_usec/1000.0;
}

// Computes twiddle factor W[k,N]
inline Complex twiddle( unsigned k, unsigned N )
{
    float radix = -2.0 * PI * k / N;
    return Complex( cosf(radix), sinf(radix) );
}

// Compute twiddle factors for N
inline Complex* compute_twiddle_table( unsigned N )
{
    Complex* W = new Complex[N/2];

    float radix = -2.0 * PI / N;
    for( unsigned k = 0; k < N/2; ++k ) {
        W[k] = Complex( cosf(radix*k), sinf(radix*k) );
    }

    return W;
}

// Generates sample input
inline void generate_input(unsigned N, Complex* X )
{
    std::uninitialized_fill( X, X+N/2,   Complex( 1.0f, 0.0f ) );
    std::uninitialized_fill( X+N/2, X+N, Complex( 0.0f, 0.0f ) );
}

void print(unsigned N, Complex* X)
{
    for (unsigned k = 0; k < N; ++k) {
        std::cout
            << std::fixed //<< std::setprecision(6)
            << k << "; "
            << X[k].r << "; "
            << X[k].i << "; "
            << std::endl;
    }
}

#endif // FFT_H

