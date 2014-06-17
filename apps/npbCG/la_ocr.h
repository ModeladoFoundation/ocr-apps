#include <math.h>

static inline void _copy(u32 n, double* x, double* y)
{
    u32 i;
    for(i = 0; i < n; ++i)
        x[i] = y[i];
}
static inline void __scale(u32 n, double* r, double a, double* x)
{
    u32 i;
    for(i = 0; i < n; ++i)
        r[i] = a*x[i];
}

static inline void __dot(u32 n, double* r, double* x, double* y)
{
    u32 i;
    for(i = 0; i < n; ++i)
        *r += x[i]*y[i];
}

static inline double _dot(u32 n, double* x, double* y)
{
    double r = 0;
    __dot(n,&r,x,y);
    return r;
}

static inline void __dotg(u32 n, double* r, double* x, u32* g, double* y)
{
    u32 i;
    for(i = 0; i < n; ++i)
        *r += x[i]*y[g[i]];
}

static inline double _dotg(u32 n, double* x, u32* g, double* y)
{
    double r = 0;
    __dotg(n,&r,x,g,y);
    return r;
}

static inline void __daxpy(u32 n, double* r, double a, double* x, double* y)
{
    u32 i;
    for(i = 0; i < n; ++i)
        r[i] = a*x[i] + y[i];
}

static inline double _dist(u32 n, double* x, double* y)
{
    double r = 0;
    u32 i;
    for(i = 0; i < n; ++i)
        r += (x[i]-y[i])*(x[i]-y[i]);
    return sqrt(r);
}

