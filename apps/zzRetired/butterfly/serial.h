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

#ifndef pi
#define pi 3.14159265358979323846
#endif

typedef struct point
{
	double real;
	double imaginary;
} point;

//globals
int numPoints;

complex double* twiddle;
point* origPointer;

//functions
complex double* buildTwiddle(int numPoints, int inverse);
point* buildDataPoints(char *file);
char* outfileName(char *file);
void writeData(point *dataPoints, char *fileName);
int countLines(FILE *fd, int BUFF_SIZE);
point mult(const complex double a, const point b);
point plus(const point a, const point b);
point minus(const point a, const point b);
point* FourierTransform(point* x, int numPoints, int inverse);
void freeAll(point* a, point* b, point* c, point* d);
void printPoint(point p);
void printPoints(point* a, int N);
void printTwoPoints(point* a, point* b, int N);
point* copyPoints(point* a, int numPoints, int stride);
int checkPoints(point* a, point* b, int numPoints);
