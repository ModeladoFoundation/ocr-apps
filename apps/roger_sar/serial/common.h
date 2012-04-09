#define _USE_MATH_DEFINES

#ifdef RAG_AFL
#define _GNU_SOURCE
#endif

#include <math.h>

#ifdef RAG_SIM
void sincosf(float arg_x, float* sin_x, float *cos_x);
#endif

#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#if RAG_PURE_FLOAT
#define M_PI		3.1415926535897932384626433832795029f
#else
#define M_PI		3.1415926535897932384626433832795029l
#endif
#endif

#if RAG_PURE_FLOAT
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
	rmd_guid_t Tp_dbg;	// Timestamp of pulse transmissions
	rmd_guid_t Pt_edge_dbg;	// Transmitter positions at each pulse
	rmd_guid_t Pt_data_dbg;	// Transmitter positions at each pulse
	rmd_guid_t X_edge_dbg;	// Pulse compressed SAR edge vector DB guid
	rmd_guid_t X_data_dbg;	// Pulse compressed SAR data DB guid
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

int  ReadParams(struct RadarParams*, struct ImageParams*, struct AffineParams*,
		struct ThinSplineParams*, int*, struct CfarParams*);
void ReadData(FILE*, FILE*, FILE*, struct Inputs*, struct ImageParams*);

rmd_guid_t FormImage(	
#if RAG_DIG_SPOT_ON
		rmd_guid_t dig_spot_dbg,
#endif
		rmd_guid_t image_params_dbg,
		rmd_guid_t radar_params_dbg,
		rmd_guid_t in_dbg,
		rmd_guid_t curImage_dbg);

rmd_guid_t BackProj(uint64_t arg,
	rmd_guid_t in_dbg,
	rmd_guid_t corners_dbg,
	rmd_guid_t image_params_dbg,
	rmd_guid_t curImage_dbg,
	rmd_guid_t radar_params_dbg);

#if RAG_DIG_SPOT_ON
struct complexData** DigSpot(float, float, struct DigSpotVars*, struct ImageParams*, struct RadarParams*, struct Inputs*);
#endif

rmd_guid_t Affine(
	rmd_guid_t affine_params_dbg,
	rmd_guid_t imageParams_dbg,
	rmd_guid_t curImage_dbg,
	rmd_guid_t refImage_dbg);

#if RAG_THIN_ON
void ThinSpline(struct ThinSplineParams*, struct ImageParams*, struct complexData**, struct complexData**);
#endif

void CCD(int Ncor, struct point**, struct ImageParams*, struct complexData**, struct complexData**);

int CFAR(struct ImageParams*, int Ncor, struct CfarParams*, struct point** corr_map, struct detects* Y);

float sinc(float x);
void  sinc_interp(float *X, struct complexData *Y, struct complexData *YI, int Nz, float B, int M, int lenY);
