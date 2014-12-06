#ifndef _MATH_H
#define _MATH_H

#include <ocr.h>

#define M_PI           3.14159265358979323846
#define PRECISION      1e-6
#define MAXITER        10

#define UNAN 0xFFFFFFFFFFFFFFFF
#define PINF 0x7FF0000000000000
#define NINF 0xFFF0000000000000
#define MEXP 0x7FE0000000000000

#define LOG2  (0.69314718055994530941)
#define LOG2E (1.44269504088896340737)

#define f2u64(f) (*(u64*)&f)
#define getMant(u) ((u)&0xFFFFFFFFFFFFF)
#define getExp(u) (((u)>>52)&0x7FF)
#define getSign(u) ((u)>>63)
#define isNaNu(u) ((getExp(u)==0x7FF)&&(getMant(u)!=0))
#define isInfu(u) ((getExp(u)==0x7FF)&&(getMant(u)==0))

inline int  __attribute__((always_inline)) isNaN(double x) {
    u64 xx = f2u64(x);
    return isNaNu(xx);
}

inline int  __attribute__((always_inline)) isInf(double x) {
    u64 xx = f2u64(x);
    return isInfu(xx);
}

#define NaN(x)  do{f2u64(x)=UNAN;}while(0)
#define pINF(x) do{f2u64(x)=PINF;}while(0)
#define nINF(x) do{f2u64(x)=NINF;}while(0)

inline void  __attribute__((always_inline)) sincosf(float x, float* sinx, float* cosx) {
    __asm__ (
        "sincosF32 %0, %2\n\t"
        "or32 %1, %0, 0x0\n\t" /* cosine takes the lower 32 bits */
        "slr64  %0, %0, 32\n\t"
        : "=r" (*sinx), "=r" (*cosx)
        : "r" (x)
        :
        );
}

inline double __attribute__((always_inline)) floor(double x) {
    u64 xx = *(u64*)&x;
    int exp = getExp(xx);
    char sign = getSign(xx);
    u64 m = getMant(xx);
    if(x==0 || exp == 0x7FF) return x;
    exp -= 1023;
    if(exp < 0) return sign ? -1 : 0;
    u64 mask = 0xFFFFFFFFFFFFF>>exp;
    if(exp > 51 || (xx&mask)==0) return x;
    xx &= ~mask;
    return *(double*)&xx + (sign ? -1 : 0);
}

inline double __attribute__((always_inline)) ceil(double x) {
    double y = floor(x);
    if(y==x) return x;
    return y+1;
}

inline double  __attribute__((always_inline)) sin(double x) {
    float sinx, cosx;
    sincosf(x, &sinx, &cosx);
    return sinx;
}

inline double  __attribute__((always_inline)) cos(double x) {
    float sinx, cosx;
    sincosf(x, &sinx, &cosx);
    return cosx;
}

inline int __attribute__((always_inline)) abs(int x) {
    if (x<0) return -x;
    return x;
}

inline double __attribute__((always_inline)) fabs(double x) {
    u64 xx = (*(u64*)&x)&0x7FFFFFFFFFFFFFFF;
    return *(double*)&xx;
}

inline double __attribute__((always_inline)) log(double x) {
#if 1
    if(x==1) return 0;
    if(x<=0 || isNaN(x)) {
        u64 nan; NaN(nan);
        return *(double*)&nan;
    }
    if(isInf(x))
        return x;

    double y,z;
    f2u64(y) = f2u64(x)&0x7FF0000000000000;
    if(y==0) z = x;
    else {
        z = x/y;
        y = (double)getExp(f2u64(x))-1023;
        y *= LOG2;
    }

    int i;
    double r = (z-1)/(z+1);
    double ans = r*2, rr = r*r; r *= 2;
    for(i=1; fabs(r) >= PRECISION && i<MAXITER; ++i) {
        r *= rr;
        double f = 1.0/((i<<1)+1);
        ans += f*r;
    }
    return ans+y;
#else
    //TODO when 4.1.0 is available
    __asm__ (
        "log2F %0, %1\n\t"
        "divF %0, $log2Fe\n\t"
        : "=r" (x)
        : "r" (x)
        );
    return x;
#endif
}

inline double __attribute__((always_inline)) sqrt(double x) {
#if 1
    __asm__ (
        "rcpsqrtF64 %0, %1\n\t"
        "rcpF64 %0, %0\n\t"
        : "=r" (x)
        : "r" (x)
        );
    return x;
#elif 0
    //TODO when 4.1.0 is available
    __asm__ (
        "sqrtF %0, %0\n\t"
        : "=r" (x)
        : "r" (x)
        );
    return x;
#else
    double ans = 1, old = 0;
    while(fabs(old-ans) >= PRECISION)
    {
        old = ans;
        ans = ((x/ans) + ans) / 2;
    }
    return ans;
#endif
}

#define MAXROOTS 16
static double roots[MAXROOTS] = {0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125, 0.00390625,
                                 0.001953125, 0.0009765625, 0.00048828125, 0.000244140625,
                                 0.0001220703125, 0.00006103515625, 0.000030517578125, 0.0000152587890625};
static double rvalue[MAXROOTS] = {1.4142135624, 1.1892071150, 1.0905077327, 1.0442737824,
                                  1.0218971486, 1.0108892860, 1.0054299011, 1.0027112750,
                                  1.0013547198, 1.0006771306, 1.0003385080, 1.0001692397,
                                  1.0000846162, 1.0000423072, 1.0000211533, 1.0000105766};

static inline double __attribute__((always_inline)) pow(double x, double y) {
#if 1
    if(x==1) return 1;
    if(y==0) return 1;
    if(x==0) return x;
    if(x==-1 && isInf(y)) return 1;
    if(isInf(y)) {
        double res;
        if(y<0) {
            if(fabs(x)<1) pINF(res);
            else res = 0;
        }
        else {
            if(fabs(x)<1) res=0;
            else pINF(res);
        }
        return res;
    }
    if(x<0 && isInf(x)) {
        double res;
        if(floor(y)==y && floor(y/2)==y/2) {
            if(y<0) res=-0.0;
            else nINF(res);
        }
        else {
            if(y<0) res=0.0;
            else pINF(res);
        }
        return res;
    }
    if(isInf(x)) {
        double res;
        if(y<0) res = 0;
        else pINF(res);
        return res;
    }

    if(fabs(y)==floor(fabs(y))) {
        double ans = x;
        int e;
        for(e=1; e<fabs(y); ++e)
            ans *= x;
        if(y<0)
            return 1/ans;
        return ans;
    }

    //checked all conditions
    double z = log(x)*y*LOG2E;
    double zi = floor(fabs(z));
    double ans;
    u64 p=1023+zi;
    if(p>2046)
        *(u64*)&ans=PINF;
    else
        *(u64*)&ans=p<<52;

    if(fabs(z)!=zi) {
        double zd = fabs(z)-zi;
        int e;
        for(e=0; e<MAXROOTS && zd!=0; ++e) {
            if(zd>=roots[e]) {
                zd -= roots[e];
                ans *= rvalue[e];
            }
        }
    }

    if(z<0)
        return 1/ans;
    return ans;
#else
    //TODO when 4.1.0 is available
    __asm__ (
        "log2F %0, %1\n\t"
        "divF %0, $log2Fe\n\t"
        "mulF %0, %2\n\t"
        "expF %0, %0\n\t"
        : "=r" (x)
        : "r" (x), "r" (y)
        );
    return x;
#endif
}

#endif //_MATH_H
