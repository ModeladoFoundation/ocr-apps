#ifndef INCLUSION_HELLOWORLD_H
#include "helloworld.h"
#endif

#include "ocr.h" //PRINTF

#include <math.h> //sqrt

const double nota_root = -1e300;

static int equal_double( double x, double y)
{
    double diff = x - y;
    if(diff<0) diff = - diff;
    if(diff > 1e-14) return 0; //Zero for false
    return 1; //1 for true
}

Err_t setupTheParabola(int * io_parabola)
{
    Err_t erri = 0;
    //a -> 2, b -> 1, c -> 0  and y = a*x^2 + b*x + c
    io_parabola[0] = 1;
    io_parabola[1] = 2;
    io_parabola[2] = 1;

    PRINTF("TESTIO> Setup used the parabola (x^2,x,1) = (%d, %d, %d).\n",
            io_parabola[2],
            io_parabola[1],
            io_parabola[0]
            );

    return erri;
}
Err_t solveForRoots(int * in_parabola, double * o_roots)
{
    Err_t erri = 0;
    //a -> 2, b -> 1, c -> 0  and y = a*x^2 + b*x + c
    const double c = in_parabola[0];
    const double b = in_parabola[1];
    const double a = in_parabola[2];

    //See the book "Numerical recipes in C" for more detail.
    const double signb = ((b<0)?(-1):(1));
    const double q = (b + signb*sqrt(b*b - 4*a*c))/2;
    o_roots[0] = q/a;
    o_roots[1] = c/q;

    PRINTF("TESTIO> The Solver for roots %21.14e and %21.14e.\n", o_roots[0], o_roots[0]);

    return erri;
}
Err_t deduplicateRoots(double * io_roots)
{
    PRINTF("TESTIO> Checking for duplicate roots.\n");
    Err_t erri = 0;

    if( equal_double(io_roots[0],io_roots[1]) ){
        io_roots[1] = nota_root;
    }

    return erri;
}
Err_t printRoots(double * io_roots)
{
    Err_t erri = 0;

    if( equal_double(io_roots[1], nota_root)){
        PRINTF("TESTIO> The double root is %21.14e\n", io_roots[0]);
    } else {
        PRINTF("TESTIO> The first and second roots are %21.14e and %21.14e\n", io_roots[0], io_roots[1]);
    }

    return erri;
}