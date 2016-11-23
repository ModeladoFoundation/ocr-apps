
#include "complex.h"
#include "fft.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

// Discrete Fourier Transform
// Efficiency is O(N^2)
inline void dft( unsigned N, Complex* X, Complex* R )
{
    for( unsigned k = 0; k < N; ++k ) {
        for( unsigned n = 0; n < N; ++n ) {
            Complex W = twiddle( k*n, N );
            R[k] += X[n] * W;
        }
    }
}

int main(int argc, char* argv[])
{
    if( argc < 2 ) {
        std::cout << "Usage " << argv[0]
            << " size" << std::endl;
        return -1;
    }

    const unsigned log2N = atoi(argv[1]);
    assert( log2N < sizeof(unsigned)*8 );
    const unsigned N = 1UL << log2N;
    const char* filename = argc>2? argv[2]: NULL;

    Complex* X = new Complex[N];
    Complex* R = new Complex[N];

    generate_input(N, X);

    dft( N, X, R );

    if( filename )
        write( N, R, argv[2] );

    delete[] X;
    delete[] R;

    return 0;
}

