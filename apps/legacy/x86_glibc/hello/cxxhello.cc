#include <iostream>

extern struct _reent * _REENT;

using namespace std;

class foo {
    int m1;
    float m2;
    char * m3;
public:
    foo( int a1, float a2, int a3 );
    ~foo();
    float M2() { return m2; }
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

foo * blah()
{
    return new foo( 10, 20, 100 );
}

int mainEdt( int argc, char **argv )
{
    foo * f = blah();

    std::cout << "Hello World!" << std::endl;
    std::cout << "cplus output: " << f->M2() << std::endl;

    return 0;
}
