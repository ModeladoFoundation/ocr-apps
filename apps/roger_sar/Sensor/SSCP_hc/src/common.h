#define _USE_MATH_DEFINES

#define _GNU_SOURCE

#include <math.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#ifdef RAG_PURE_FLOAT
#define M_PI		3.1415926535897932384626433832795029f
#else
#define M_PI		3.1415926535897932384626433832795029l
#endif
#endif

#ifdef RAG_PURE_FLOAT
#define c_mks_mps 300000000.0f
#else
#define c_mks_mps 3e8
#endif

struct complexData {
	float real;
	float imag;
};

struct point {
	int x;		// x coordinate (pixel)
	int y;		// y coordinate (pixel)
	float p;	// correlation value
};

struct detects {
	float x;	// x coordinate (m)
	float y;	// y coordinate (m)
	float p;	// correlation value
};

struct Inputs {
	float *Tp;	// Timestamp of pulse transmissions
	float **Pt;	// Transmitter positions at each pulse
	struct complexData **X;	// Pulse compressed SAR data
};

struct RadarParams {
	float fc;		// Carrier frequency (Hz)
	float fs;		// Sampling frequency (Sa/s)
	float r0;		// Range from platform to scene center (m) 
	float R0;		// Range of the zeroth range bin (m)
	float PRF;		// Pulse repetition frequency (Hz)
	float R0_prime;	// Range of the zeroth range bin if digital spotlighting is used (m)
};

struct AffineParams {
	int Nc;			// Number of affine registration control points
	int Sc;			// Affine registration neighborhood size
	int Rc;			// Range of affine registration
	float Tc;		// Affine registration correlation threshold
};

struct ThinSplineParams {
	int Nf;			// Number of thin-spline registration control points
	int Rf;			// Range of fine registration
	int Scf;		// Thin-spline neighborhood size
	float Tf;		// Thin-spline registration correlation threshold
};

struct CfarParams {
	int Ncfar;		// Constant false alarm rate (CFAR) neighborhood size
	int Nguard;		// Number of guard cells for CFAR neighborhood
	float Tcfar;	// CFAR threshold
};

struct ImageParams {
	int F;			// Oversampling factor
	int TF;			// Tiling factor
	int Ix;			// Number of pixels in x direction
	int Iy;			// Number of pixels in y direction
	int Sx;			// Number of pixels in x direction of digital spotlighting subimage
	int Sy;			// Number of pixels in y direction of digital spotlighting subimage
	int P1;			// Number of pulses used to form images
	int S1;			// Number of complex return samples per pulse
	int P2;			// Number of reduced synthetic pulses after digital spotlighting
	int S2;			// Intermediate number of samples per simulated pulse during digital spotlighting
	int P3;			// Number of pulses after backprojection
	int S3;			// Number of samples per simulated pulse after digital spotlighting
	int S4;			// Length of oversampled IDFT
	int numImages;	// Number of images to process
	float dr;		// Pixel edge size (m)
	float *xr;		// x axis vector
	float *yr;		// y-axis vectors
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

int ReadParams(struct RadarParams*, struct ImageParams*, struct AffineParams*, struct ThinSplineParams*, int*, struct CfarParams*);

void ReadData(FILE*, FILE*, FILE*, struct Inputs*, struct ImageParams*);

#pragma hc suspendable
void FormImage(struct DigSpotVars*, struct ImageParams*, struct Inputs*, struct complexData**, struct RadarParams*);

#if RAG_DIG_SPOT_ON
struct complexData** DigSpot(float, float, struct DigSpotVars*, struct ImageParams*, struct RadarParams*, struct Inputs*);
#endif

#pragma hc suspendable
void BackProj(struct complexData*[], float**, int, int, int, int, struct ImageParams*, struct complexData**, struct RadarParams*);

#pragma hc suspendable
void Affine(struct AffineParams*, struct ImageParams*, struct complexData**, struct complexData**);

#if RAG_THIN_ON
void ThinSpline(struct ThinSplineParams*, struct ImageParams*, struct complexData**, struct complexData**);
#endif

#pragma hc suspendable
void CCD(int Ncor, struct point**, struct ImageParams*, struct complexData**, struct complexData**);

#pragma hc suspendable
void CFAR(struct ImageParams*, int, struct CfarParams*, struct point**, struct detects*,FILE *pOutFile);

void sinc_interp(float *X, struct complexData *Y, struct complexData *YI, int Nz, float B, int M, int lenY);
float sinc(float x);

static int blk_size(int n,int max_blk_size) {
	int ret_val = n;
	for(int i = max_blk_size; i>1; i--) {
		if( (n%i) == 0 ) {
			ret_val = i;	
			break;
		}
	}
#ifdef DEBUG
	fprintf(stderr,"blk_size(%d,%d) returns %d\n",n,max_blk_size,ret_val);
#endif
	return ret_val;
}

int RAG_fetch_and_add(int *ptr, int add_val);

struct Corners_t {
	int x1; int x2; int y1; int y2;
};
struct corners_t {
	int m1; int m2; int n1; int n2;
};
