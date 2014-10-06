#ifndef TG_ARCH
#define _GNU_SOURCE
#endif

#include <fftw3.h>
#ifndef TG_ARCH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifndef M_PI
#define M_PI		(3.1415926535897932384626433832795029)
#endif

#ifdef RAG_PURE_FLOAT
#define c_mks_mps (300000000.0f)
#else
#define c_mks_mps (300000000.0)
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
	float *Tp;		// Timestamp of pulse transmissions
	float **Pt;		// Transmitter positions at each pulse
	struct complexData **X;	// Pulse compressed SAR data
	ocrGuid_t Tp_dbg;	// Timestamp of pulse transmissions
	ocrGuid_t Pt_dbg;	// Transmitter positions at each pulse
	ocrGuid_t X_dbg;	// Pulse compressed SAR data DB guid
};

struct RadarParams {
	float fc;		// Carrier frequency (Hz)
	float fs;		// Sampling frequency (Sa/s)
	float r0;		// Range from platform to scene center (m)
	float R0;		// Range of the zeroth range bin (m)
	float PRF;		// Pulse repetition frequency (Hz)
	float R0_prime;		// Range of the zeroth range bin if digital spotlighting is used (m)
};

struct AffineParams {
	int Nc;			// Number of affine registration control points
	int Sc;			// Affine registration neighborhood size
	int Rc;			// Range of affine registration
	float Tc;		// Affine registration correlation threshold
};

struct CfarParams {
	int Ncfar;		// Constant false alarm rate (CFAR) neighborhood size
	int Nguard;		// Number of guard cells for CFAR neighborhood
	float Tcfar;		// CFAR threshold
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
	int Fbp;		// Image space oversampling during backprojection
	int P2;			// Number of reduced synthetic pulses after digital spotlighting
	int S2;			// Intermediate number of samples per simulated pulse during digital spotlighting
	int P3;			// Number of pulses after backprojection
	int S3;			// Number of samples per simulated pulse after digital spotlighting
	int S4;			// Length of oversampled IDFT
	int numImages;		// Number of images to process
	float dr;		// Pixel edge size (m)
	float *xr;		// x axis vector
	float *yr;		// y-axis vectors
	int   Ncor;             // CCD/CFAR correlation neighborhood size
	int imageNumber;	// Image number that is being processed RAG
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

struct file_args_t {
#ifndef TG_ARCH
	FILE *pInFile, *pInFile2, *pInFile3, *pOutFile;
#else
	void *pInFile, *pInFile2, *pInFile3, *pOutFile;
#endif
};

struct corners_t {
	int m1; int m2; int n1; int n2;
};

int  ReadParams(struct RadarParams*,  struct ImageParams*, \
                struct AffineParams*, struct CfarParams*);

ocrGuid_t ReadData_edt (uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t FormImage_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t BackProj_edt (uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t Affine_edt   (uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t CCD_edt      (uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t CFAR_edt     (uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
ocrGuid_t post_CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);

#if RAG_DIG_SPOT_ON
struct complexData** DigSpot(float, float, struct DigSpotVars*, struct ImageParams*, struct RadarParams*, struct Inputs*);
#endif

float sinc(float x);
void  sinc_interp(float *X, struct complexData *Y, struct complexData *YI, int Nz, float B, int M, int lenY);
