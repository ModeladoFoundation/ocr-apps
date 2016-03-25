#include <iostream>

struct error {
    int err;
    error(int i ) { err = i; }
};

int may_throw( int v )
{
    if( v > 5 ) {
        std::cout << "may_throw: Throwing error = " << v << std::endl;
        throw( error(v) );
    }
    return v + 1;
}

class foo {
    int m1;
    float m2;
    char * m3;
public:
    foo( int a1, float a2, int a3 );
    ~foo();
    int M1() { return m1; }
    float M2() { return m2; }
    int blah(int in);
};

foo static_foo( 1, 1.0, 1 );

foo::foo( int a1, float a2, int a3 ) : m1(a1), m2(a2)
{
    m3 = new char[a3];
}

foo::~foo( )
{
    delete m3;
}

int foo::blah( int in )
{
    try {
        may_throw( in );
        return 0;

    } catch( error e ) {
        std::cout << "foo::blah: Caught throw (error = " << e.err << ")" << std::endl;
        throw( error(10) );
    }
}

foo * blah()
{
    return new foo( 10, 20, 100 );
}

int main( int argc, char ** argv, char ** envp )
{
    foo * b = blah();

    std::cout << "Hello World!" << std::endl;
    std::cout << "cplus output: " << b->M2() << std::endl;

    try {
        int v = 7;
        std::cout << "main: Trying blah(" << v << ")" << std::endl;
        b->blah( v );

    } catch( error e ) {
        std::cout << "main: Caught throw (error = " << e.err << ")" << std::endl;
        return e.err;
    }

    return b->M1() * (int) b->M2();
}
