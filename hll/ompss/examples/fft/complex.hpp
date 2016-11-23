
#ifndef COMPLEX_H
#define COMPLEX_H

#include <fstream>

struct Complex {
    typedef float Real;

    Real r; // real part
    Real i; // imaginary part

    Complex() :
        r(0.0), i(0.0)
    {
    }

    Complex( Real real, Real imag ) :
        r(real), i(imag)
    {
    }

    Complex operator* ( const Complex& o ) const {
        return Complex( r*o.r - i*o.i,
                          r*o.i + i*o.r );
    }

    Complex operator+( const Complex& o ) const {
        return Complex( r+o.r, i+o.i );
    }

    Complex operator-( const Complex& o ) const {
        return Complex( r-o.r, i-o.i );
    }

    Complex& operator+=( const Complex& o ) {
        r += o.r;
        i += o.i;
        return *this;
    }
};

inline Complex* read( unsigned N, const char* filename )
{
    Complex* X = new Complex[N];

    std::ifstream input(filename, std::ios_base::in | std::ios_base::binary );
    input.read( reinterpret_cast<char*>(X), N*sizeof(Complex) );

    return X;
}

inline void write( unsigned N, const Complex* X, const char* filename )
{
    std::ofstream output(filename, std::ios_base::out | std::ios_base::binary );
    //output.write( reinterpret_cast<const char*>(X), N*sizeof(Complex) );
}

#endif // COMPLEX_H

