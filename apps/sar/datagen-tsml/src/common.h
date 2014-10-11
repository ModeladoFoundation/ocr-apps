#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define c_mks_mps 3e8			// Speed of light (m/s)
#define Nh 33				// Number of FIR filter coefficients

// Function prototypes
int ReadParams();
int ReadTargets();
void fir_filter(float *x, float *h, float *y, int nh, int nr);
void gen_rand(float *x, float max, float min, float scale, int nx);

extern int rngBinDev;
extern int numImages;
extern int numTargets;
extern int *target_last;
extern int *target_first;
extern float v;
extern float r0;
extern float cr;
extern float fs;
extern float fc;
extern float prf;
extern float theta_el;
extern float edgeSize;
extern float *target_x;
extern float *target_y;
extern float *target_z;
extern float *target_b;
