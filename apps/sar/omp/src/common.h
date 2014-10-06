#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef RAG_CRLIBM
#include "crlibm.h"
extern float ceilf(float arg);
extern float cosf(float arg);
extern float fabsf(float arg);
extern float fmodf(float x, float y);
extern float floorf(float arg );
extern float sinf(float arg);
extern void  sincosf(float arg, float *si, float *rc);
#if 1
extern float sqrtf(float arg); // RAG -- Use hardware sqrtf, it is correctly rounded
#else
#define      sqrtf(arg) __ieee754_sqrtf(arg)
extern float __ieee754_sqrtf(float arg);
#endif

extern double cos(double arg);
extern double sin(double arg);
extern void  sincos(double arg, double *si, double *rc);
extern double round(double arg);
#if 1
extern double sqrt(double arg); // RAG -- Use hardware sqrt, it is correctly rounded
#else
#define       sqrt( arg) __ieee754_sqrt( arg)
extern double __ieee754_sqrt(double arg);
#endif

#else // RAG_CRLIBM
#define _USE_MATH_DEFINES
#include <math.h>
#endif // RAG_CRLIBM
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#ifdef RAG_PURE_FLOAT
#define M_PI		3.1415926535897932384626433832795029f
#else
#define M_PI		3.1415926535897932384626433832795029
#endif
#endif

#ifdef RAG_PURE_FLOAT
#define SPEED_OF_LIGHT 300000000.0f
#else
#define SPEED_OF_LIGHT 300000000.0
#endif

#define MALLOC_CHECKED(size) malloc_checked(size, __FILE__, __LINE__)
void *malloc_checked(size_t size, const char *file, int line);

#define CALLOC_CHECKED(nelem, elemsize) calloc_checked(nelem, elemsize, __FILE__, __LINE__)
void *calloc_checked(size_t nelem, size_t elemsize, const char *file, int line);

#define FFTWF_MALLOC_CHECKED(size) fftwf_malloc_checked(size, __FILE__, __LINE__)
void *fftwf_malloc_checked(size_t size, const char *file, int line);

#define FREE_AND_NULL(x) \
    do { if (x) { free(x); x = NULL; } } while(0);

struct complexData {
    float real;
    float imag;
};

struct point {
    int x;      // x coordinate (pixel)
    int y;      // y coordinate (pixel)
    float p;    // correlation value
};

struct detects {
    float x;    // x coordinate (m)
    float y;    // y coordinate (m)
    float p;    // correlation value
};

struct Inputs {
    float *Tp;  // Timestamp of pulse transmissions
    float **Pt; // Transmitter positions at each pulse
    struct complexData **X; // Pulse compressed SAR data
};

struct RadarParams {
    float fc;       // Carrier frequency (Hz)
    float fs;       // Sampling frequency (Sa/s)
    float r0;       // Range from platform to scene center (m)
    float R0;       // Range of the zeroth range bin (m)
    float PRF;      // Pulse repetition frequency (Hz)
    float R0_prime; // Range of the zeroth range bin if digital spotlighting is used (m)
};

struct RegistrationParams {
    int Nc;         // Number of registration control points
    int Sc;         // Registration neighborhood size
    int Rc;         // Range of registration
    float Tc;       // Registration correlation threshold
};

struct CfarParams {
    int Ncfar;      // Constant false alarm rate (CFAR) neighborhood size
    int Nguard;     // Number of guard cells for CFAR neighborhood
    float Tcfar;    // CFAR threshold
};

struct ImageParams {
    int F;          // Oversampling factor
    int TF;         // Tiling factor
    int Ix;         // Number of pixels in x direction
    int Iy;         // Number of pixels in y direction
    int Ix2;        // Number of pixels in x direction after upsampling by Fbp
    int Iy2;        // Number of pixels in y direction after upsampling by Fbp
    int Sx;         // Number of pixels in x direction of digital spotlighting subimage
    int Sy;         // Number of pixels in y direction of digital spotlighting subimage
    int Sx2;        // Number of pixels in x direction of digital spotlighting subimage after upsampling by Fbp
    int Sy2;        // Number of pixels in y direction of digital spotlighting subimage after upsampling by Fbp
    int P1;         // Number of pulses used to form images
    int S1;         // Number of complex return samples per pulse
    int Fbp;        // Image space oversampling factor during backprojection
    int P2;         // Number of reduced synthetic pulses after digital spotlighting
    int S2;         // Intermediate number of samples per simulated pulse during digital spotlighting
    int P3;         // Number of pulses after backprojection
    int S3;         // Number of samples per simulated pulse after digital spotlighting
    int S4;         // Length of oversampled IDFT
    int Ncor;       // Coherent Change Detection (CCD) neighborhood size
    int numImages;  // Number of images to process
    float dr;       // Range bin resolution (m) after upsampling by F
    float dx;       // Pixel edge size in x (m)
    float dy;       // Pixel edge size in y (m)
    float dx2;      // Pixel edge size in x (m) after upsampling by Fbp
    float dy2;      // Pixel edge size in y (m) after upsampling by Fbp
    float *xr;      // x axis vector
    float *yr;      // y-axis vector
    float *xr2;     // x axis vector after upsampling by Fbp
    float *yr2;     // y-axis vector after upsampling by Fbp
};

struct DigSpotVars {
    float **Pt2;
    float *freqVec;
    struct complexData **X2;
    struct complexData **X3;
    struct complexData **X4;
    struct complexData *filtOut;
    struct complexData *tmpVector;
};

/*
struct reg_map {
    int x;
    int y;
    int u;
    int v;
    float p;
};
*/

int CFAR(struct ImageParams*, int, struct CfarParams*, struct point**, struct detects*);
int ReadParams(struct RadarParams*, struct ImageParams*, struct RegistrationParams*, struct CfarParams*);
void ReadData(FILE*, FILE*, FILE*, struct Inputs*, struct ImageParams*);
void Registration(struct RegistrationParams*, struct ImageParams*, struct complexData**, struct complexData**);
void CCD(int Ncor, struct point**, struct ImageParams*, struct complexData**, struct complexData**);
void sinc_interp(float *X, struct complexData *Y, struct complexData *YI, int Nz, float B, int M, int lenY);
void FormImage(struct DigSpotVars*, struct ImageParams*, struct Inputs*, struct complexData**, struct RadarParams*);
void BackProj(struct complexData*[], float**, int, int, int, int, struct ImageParams*, struct complexData**, struct RadarParams*);
float sinc(float x);
struct complexData** DigSpot(float, float, struct DigSpotVars*, struct ImageParams*, struct RadarParams*, struct Inputs*);
void Downsample(
    const struct ImageParams *image_params,
    struct complexData **curImage,
    struct complexData **refImage,
    struct complexData ** curImage2,
    struct complexData ** refImage2);

#endif // _COMMON_H_
