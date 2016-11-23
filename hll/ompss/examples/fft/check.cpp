
#include "complex.h"
#include "fft.h"

#include <fstream>
#include <cassert>
#include <cstdlib>

bool matches( const Complex& lhs, const Complex& rhs )
{
    const float tolerance = 1e-3f;
    const Complex delta = lhs - rhs;
    return -tolerance < delta.r && delta.r < tolerance
        && -tolerance < delta.i && delta.i < tolerance;
}

int main( int argc, char* argv[] )
{
    if( argc < 4 ) {
        std::cout << "Usage: " << argv[0]
            << " log2(size) input.1 input.2"
            << std::endl;
        return -1;
    }

    const unsigned log2N = atoi(argv[1]);
    assert( log2N < sizeof(unsigned)*8 );
    const unsigned N = 1UL << log2N;

    Complex* X = read( N, argv[2] );
    Complex* Y = read( N, argv[3] );

    bool passed = true;
    unsigned k = 0;
    while( passed && k < N ) {
        passed = matches( X[k], Y[k] );
        ++k;
    }

    if( passed )
        std::cout << "Check succeeded" << std::endl;
    else {
        --k;
        printf("Check failed. X[%d] (%f,%f) != Y[%d] (%f,%f)\n",
            k, X[k].r, X[k].i, k, Y[k].r, Y[k].i );
    }

    delete[] X;
    delete[] Y;
}
