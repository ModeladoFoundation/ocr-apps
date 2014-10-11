/**
 *	Fast Fourier Transform (FFT)
 *	@author: Austin Gibbons - austin.gibbons@intel.com
 *	@date: June 7th, 2011
 *
 *	This is a serial version of the Cooley-Tukey FFT algorthim
 *	Its purpose is to serve as an introductory tool to developing
 *	Synthetic Apeture Radar (SAR) and offer parallelization hints
 *	in regards to both HTA and Codelets paradigms
 *
 *	@Input: a file where each line represents an n-dimensional point
 *	@Output: a file where each line represents an n-dimensional point
 */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#ifndef pi
#define pi 3.14159265358979323846
#endif

#ifndef stdout
#define stdout 1
#endif

typedef struct point
{
	double real;
	double imaginary;
} point;

typedef struct complex_double
{
	double real;
	double imaginary;
} complex_double;

//globals
int numPoints;

/*
 * 	twiddle factor is not actually a point, but rather
 * 	simply a complex number. I will change whenever I change point
 */
complex_double* twiddle;
point* origPointer;

//functions
complex_double* buildTwiddle(int numPoints, int inverse);
//complex_double cexp(double x);
double my_pow(double base, int exp);
double factorial(int x);
point* buildDataPoints(char *file);
char* outfileName(char *file);
void writeData(point *dataPoints, char *fileName);
int countLines(/*FILE *fd*/ int fd, int BUFF_SIZE);
//point mult(const point a, const point b);
point mult(const complex_double a, const point b);
point plus(const point a, const point b);
point minus(const point a, const point b);
point* fastFourierTransform(point* x, int N, int stride, int inverse);
point* FourierTransform(point* x, int N, int inverse);
void freeAll(point* a, point* b, point* c, point* d);
void printPoint(point p);
void printPoints(point* a, int N);
void printTwoPoints(point* a, point* b, int N);
point* copyPoints(point* a, int numPoints, int stride);
int checkPoints(point* a, point* b, int numPoints);
point* hardCoded();
void * memcpy ( void * destination, const void * source, unsigned long num );
