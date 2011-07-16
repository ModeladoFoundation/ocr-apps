
/*
 *  *      prints all the points in a list
 *   */
void printPoints(point* a, int N)
{/*
        int k;
        for(k = 0; k < N; k++)
        {
                printPoint(a[k]);
        }
*/}

/*
 *  *      prints all the points in a list
 *   */
void printTwoPoints(point* a, point* b, int N)
{
/*
 *         int k;
 *                 for(k = 0; k < N; k++)
 *                         {
 *                                         printf("k: %d\n", k);
 *                                                         printPoint(a[k]);
 *                                                                         printPoint(b[k]);
 *                                                                                 }
 *
 *                                                                                 */}

/*
 *  *      System.out.println(point.toString());
 *   */
void printPoint(point p)
{
        //printf("%f + %fi\n", p.real, p.imaginary);
        //}
        //
        //
}
/*
 *  *      Implements an approximation to e^(x*i) = cos(x) + i*sin(x)
 *   *      error within x^10/10! of true value. maybe.
 *    */
complex_double cexp(double x);
double factorial(int x);
double my_pow(double base, int exp);
complex_double cexp(double x)
{
        double cos = 1, sin = x;
        int k = 2;
        double sign = -1.0;
        for(; k < 10; k++)
        {
                if(!(k & 0x1)) //even - cos
                {
                        cos += sign*my_pow(x, k)/factorial(k);
                }
                else //odd - sin
                {
                        sin += sign*my_pow(x, k)/factorial(k);
                }

                sign *= -1;
        }

        complex_double ret;
        ret.real = cos;
        ret.imaginary = sin;

        return ret;
}

/*
 *  *      Helpers to my cexp
 *   */
double factorial(int x)
{
        if(x < 1) return 1.0;
        if(x == 2) return 2.0;
        if(x == 3) return 6.0;
        if(x == 4) return 24.0;
        if(x == 5) return 120.0;
        if(x == 6) return 720.0;
        if(x == 7) return 5040.0;
        if(x == 8) return 40320.0;
        if(x == 9) return 362880.0;
        if(x == 10) return 3628800.0;
        if(x == 11) return 39916800.0;

        return -1.0;
}

/*
 *  *      Helpers to my cexp
 *   */
double my_pow(double base, int exp)
{
        if(exp < 0) return -1;
        if(exp == 0) return 1;
        int ret = 1;
        while(exp > 0)
        {
                if (exp & 0x1) ret *= base;
                exp >>= 1;
                base *= base;
        }

        return ret;
}

void * memcpy ( void * destination, const void * source, unsigned long num )
{
        char* dst8 = (char*)destination;
        char* src8 = (char*)source;

        while (num--) {
            *dst8++ = *src8++;
        }
        return destination;
}


/*
 *  *      add two points (operators are not overloadable)
 *   */
point plus(const point a, const point b)
{
        point x;
        x.real = a.real + b.real;
        x.imaginary = a.imaginary + b.imaginary;
        return x;
}

/*
 *  *      subtract two points (operators are not overloadable)
 *   */
point minus(const point a, const point b)
{
        point x;
        x.real = a.real - b.real;
        x.imaginary = a.imaginary - b.imaginary;
        return x;
}

/**
 *  *      Multiplies two points
 *   */
/*
 * point mult(const point a, const point b)
 * {
 *         point x;
 *                 x.real = (b.real * a.real) - (b.imaginary * a.imaginary);
 *                         x.imaginary = (b.imaginary * a.real) + (b.real * a.imaginary);
 *                                 return x;
 *                                 }
 *                                 */
point mult(const complex_double a, const point b)
{
        point x;
        x.real = (b.real * a.real) - (b.imaginary * a.imaginary);
        x.imaginary = (b.imaginary * a.real) + (b.real * a.imaginary);
        return x;
}

/**
 *  *      Precomputes the twiddle factors
 *   */
complex_double* buildTwiddle(int numPoints, int inverse)
{
        twiddle = (complex_double *) malloc(numPoints*sizeof(complex_double)/2);

        int k;
        for(k = 0; k < numPoints/2; k++)
        {
                twiddle[k] = cexp(((double) inverse)*-2.0*pi*((double) k)/((double) numPoints));
        }

        return twiddle;
}

/*
 *  *      Memory clean up. Any extraneous memory is free'd here
 *   */
void freeAll(point* a, point* b, point* c, point* d)
{
        free(a);
        free(b);
        free(c);
        free(d);
        free(twiddle);
}

#include "dataset.h"

