#ifndef __TYPE_H__
#define __TYPE_H__
// copied from SNU-NPB NAS benchmark implementation

typedef struct {
    double real;
    double imag;
} dcomplex;

#define dcmplx(r,i)       (dcomplex){r, i}
#define dcmplx_add(a,b)   (dcomplex){(a).real+(b).real, (a).imag+(b).imag}
#define dcmplx_sub(a,b)   (dcomplex){(a).real-(b).real, (a).imag-(b).imag}
#define dcmplx_mul(a,b)   (dcomplex){((a).real*(b).real)-((a).imag*(b).imag),\
                                         ((a).real*(b).imag)+((a).imag*(b).real)}
#define dcmplx_mul2(a,b)  (dcomplex){(a).real*(b), (a).imag*(b)}
static inline dcomplex dcmplx_div(dcomplex z1, dcomplex z2) {
    double a = z1.real;
    double b = z1.imag;
    double c = z2.real;
    double d = z2.imag;

    double divisor = c*c + d*d;
    double real = (a*c + b*d) / divisor;
    double imag = (b*c - a*d) / divisor;
    dcomplex result = (dcomplex){real, imag};
    return result;
}
#define dcmplx_div2(a,b)  (dcomplex){(a).real/(b), (a).imag/(b)}
#define dcmplx_abs(x)     sqrt(((x).real*(x).real) + ((x).imag*(x).imag))

#define dconjg(x)         (dcomplex){(x).real, -1.0*(x).imag}
#endif
