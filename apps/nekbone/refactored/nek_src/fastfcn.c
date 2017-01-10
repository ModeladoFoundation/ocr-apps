#ifndef FASTFCN_H
#include "fastfcn.h"
#endif

double fastfcn_cos(double x)
{
    const double pi2 = 6.2831853071795864769252867665590057683943387987502116; // 2*pi
    const double pi_2 = 1.5707963267948966192313216916397514420985846996875529; // pi/2
    const double pi3_2 = 4.7123889803846898576939650749192543262957540990626587; // 3*pi/2

    //Normalize x
    if(x<0) x=-x;
    const int k = (int) (x / pi2);
    x = x - k*pi2;

    double a,b,c,d;

    if (x < pi_2){
        a = 1.13863107247982e-001;
        b = -6.06523404609620e-001;
        c = 3.54338955374090e-002;
        d = 9.97308361272056e-001;
    } else if(x > pi3_2){
        a = -1.13868951779392e-001;
        b = 1.53984182673162e+000;
        c = -5.89956842702277e+000;
        d = 5.52008743069717e+000;
    } else {
        a = 6.72024445604427e-006;
        b = 4.17634062301286e-001;
        c = -2.62427828889882e+000;
        d = 3.14215981350052e+000;
    }

    double v;
    v = a*x + b;
    v = v*x + c;
    v = v*x + d;

    return v;
}