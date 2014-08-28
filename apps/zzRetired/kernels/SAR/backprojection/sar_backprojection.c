// May 27th 2011 Reservoir Labs Inc proprietary

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#ifdef NO_RSTREAM
#include <omp.h>
#endif

typedef float real_t;

// We use the terminology from Chapter 2 "Streaming Sensor Signal Processing"
// of the UHPC document by Dan Campbell.

// X coordinate of the image (i.e. the ground patch we want to map)
// (divide by 10 to keep manageable in the first version)
#ifndef Ix
# define Ix 3200//0
#endif

// Y coordinate of the image (i.e. the ground patch we want to map)
// (divide by 10 to keep manageable in the first version)
#ifndef Iy
# define Iy 3200//0
#endif

// Rows in the P1 x S1 matrix of complex return samples
// (divide by 10 to keep manageable in the first version)
#ifndef PULSES
# define PULSES 3840//0
#endif

// Columns in the P1 x S1 matrix of complex return samples
// (divide by 10 to keep manageable in the first version)
#ifndef RET_PER_PULSE
# define RET_PER_PULSE 3200//0
#endif

// Height around which the plane flies
// (divide by 10 to keep manageable in the first version)
#ifndef HEIGHT
# define HEIGHT 1000//0
#endif

// Resolution
#ifndef dR
# define dR 3
#endif

//
// For this study, we assume a perfect circular flight path centered around
// the center of the image at position (Ix/2, Iy/2) and at height HEIGHT.
// Later on we can add perturbations to this flight path to simulate a real
// flight path.
// The important thing is we do not use the knowledge that the path is a
// circle to change the algorithm.
// Note: considering a perfect circle may be doable using "motion
// compensation" which would require additional FFT/IFFT steps. This is an
// algorithmic change that should be left for later.
//
// To comply with the requirements of the benchmark, we generate the flight
// path in a 3D array that we read to find the locations.
// We assume the radius of the flight path to be 20% wider than the image for
// a radius of 1.2 * sqrt(Ix*Ix + Iy*Iy)/2 (i.e. 0.6 * sqrt(Ix*Ix + Iy*Iy)).
// The maximal distance (i.e. index in PSR) is therefore smaller than:
// sqrt(0.36 * (Ix*Ix + Iy*Iy) + HEIGHT*HEIGHT)
//
// For a height of 10k and Ix, Iy and RET_PER_PULSE of 32k (Dan Campbell's
// numbers page 7), the maximal index in a PSR column is <= 29k which will not
// overflow.
//
// We also assume a dR range resolution of 3 which will further reduce the
// spread.
//
// Reference "position 0" is defined as (Ix/2 + r, Iy/2, HEIGHT)
//
// This version is a reasonable base that passes through the RStream
// transformation tool as of May 27th 2011.
//
// The goal of the "image functions" below is to convey the following
// information:
// - given a variation along x or y (the coordinates of the image on the
// ground), the distance  from the plane to the ground varies almost
// linearly. Hence, the column index into the array of responses (PSR) should be
// properly approximated by a linear function.
// - the row index in the PSR array changes linearly with the induction
// variable along dimension of the plane position.
//
// RStream's goal is to utilize this information and properly optimize for locality.

#ifndef PI
#define PI 3.14159265
#endif

int PARAMS[16];
int Xiter, Yiter, Iiter;

real_t *Im, *Imcheck, *tIm, *PSRsRe, *PSRsIm, *positions;

/*
 * The functions fill_re_int_bin and fill_im_int_bin are "blackboxes".
 * Blackboxes are defined as pure C functions and usually encapsulate
 * code that RStream should not modify. They can arise for instance when:
 * - we want to integrate library code
 * - we want to guarantee RStream will not modify a given piece of code
 * - we want to hide constructs that are not supported by RStream
 * For the requirements of blackboxing and image functions, please refer to
 * the RStream doc.
 */
void fill_re_int_bin(real_t (*aPSRsRe)[RET_PER_PULSE],
		     int x, int y, int i, int int_bin,
		     real_t *re_int_bin, real_t *re_int_bin_p1) {
  *re_int_bin    = aPSRsRe[i][int_bin];
  *re_int_bin_p1 = aPSRsRe[i][int_bin + 1];
}

void fill_im_int_bin(real_t (*aPSRsIm)[RET_PER_PULSE],
		     int x, int y, int i, int int_bin,
		     real_t *im_int_bin, real_t *im_int_bin_p1) {
  *im_int_bin    = aPSRsIm[i][int_bin];
  *im_int_bin_p1 = aPSRsIm[i][int_bin + 1];
}

/*
 * The functions fill_re_int_bin_img and fill_im_int_bin_img represent "image
 * functions" passed to RStream to specify the behavior of the
 * "blackboxes". These can be viewed as user defined annotations that will be
 * used by the RStream mapper.
 * In this case, we specify that there is "some" locality across x and y.
 */
#pragma rstream map image_of:fill_re_int_bin
void fill_re_int_bin_img(real_t (*aPSRsRe)[RET_PER_PULSE],
			 int x, int y, int i, int int_bin,
			 real_t *re_int_bin, real_t *re_int_bin_p1) {

  *re_int_bin    = aPSRsRe[i][x + y] + int_bin;
  *re_int_bin_p1 = aPSRsRe[i][x + y + 1] + int_bin;
}

#pragma rstream map image_of:fill_im_int_bin
void fill_im_int_bin_img(real_t (*aPSRsIm)[RET_PER_PULSE],
			 int x, int y, int i, int int_bin,
			 real_t *im_int_bin, real_t *im_int_bin_p1) {

  *im_int_bin    = aPSRsIm[i][x + y] + int_bin;
  *im_int_bin_p1 = aPSRsIm[i][x + y + 1] + int_bin;
}

// Initializing the data
void initialize() {
}

void initialize_once() {
  int i, j;
  real_t r;

  Xiter = PARAMS[0];
  Yiter = PARAMS[1];
  Iiter = PARAMS[2];

  Im = (real_t*) malloc (Xiter * Iy * sizeof(real_t));
  Imcheck = (real_t*) malloc (Xiter * Iy * sizeof(real_t));
  PSRsRe = (real_t*) malloc (Iiter * PULSES * sizeof(real_t));
  PSRsIm = (real_t*) malloc (Iiter * PULSES * sizeof(real_t));
  positions = (real_t*) malloc (Iiter * 3 * sizeof(real_t));

  memset(Im, 0, Xiter * Iy * sizeof(real_t));
  memset(Imcheck, 0, Xiter * Iy * sizeof(real_t));

  // Generate a circular flight path of radius
  // R = 0.6 * sqrt(Ix*Ix + Iy*Iy))
  // In a second phase introduce some error around that path.
  r = 0.6 * sqrt(Xiter * Xiter + Yiter * Yiter);
  for (i=0; i<Iiter; i++) {
    real_t angle = (2*PI*i)/Iiter;
    ((real_t (*)[3])positions)[i][0] = Xiter/2 + r * cos(angle) ;
    ((real_t (*)[3])positions)[i][1] = Yiter/2 + r * sin(angle) ;
    ((real_t (*)[3])positions)[i][2] = HEIGHT ;
  }

  // Real initialization follows
  //#pragma omp parallel for private(i,j)
  for (i = 0; i < Xiter; i++) {
    for (j = 0; j < Yiter; j++) {
      ((real_t (*)[Iy])Im)[i][j] = 0.0;
      ((real_t (*)[Iy])Imcheck)[i][j] = 0.0;
    }
  }

  //#pragma omp parallel for private(i,j)
  for (i = 0; i < Iiter; i++) {
    for (j = 0; j < RET_PER_PULSE; j++) {
      ((real_t (*)[RET_PER_PULSE])PSRsRe)[i][j] = (real_t)(2*i+3*j)/(real_t)(i+j+1);
      ((real_t (*)[RET_PER_PULSE])PSRsIm)[i][j] = (real_t)(2*i-3*j)/(real_t)(i+j+1);;
    }
  }
}

#pragma rstream inline
void sar_backprojection(real_t (*aIm)[Iy],
			real_t (*aPSRsRe)[RET_PER_PULSE],
			real_t (*aPSRsIm)[RET_PER_PULSE],
			real_t (*positions)[3],
			int pY, int pX, int pI) {
  int x, y, i;

  // Cross-range position of pixel in the image
  for (x=0; x<pX; x++) {
    // Down-range position of pixel in the image
    for (y=0; y<pY; y++) {
      real_t acc = 0.0;

      real_t a0 = (x - positions[0][0]);
      real_t b0 = (y - positions[0][1]);
      real_t c0 = (    positions[0][2]);
      real_t R0 = (real_t)sqrt(a0*a0 + b0*b0 + c0*c0);

      // Pulse number
      for (i=1; i<pI; i++) {
	real_t a = (x - positions[i][0]);
	real_t b = (y - positions[i][1]);
	real_t c = (    positions[i][2]);
	real_t sqr = (real_t)sqrt(a*a + b*b + c*c);
	real_t R = 2.0 * sqr;
	real_t  bin = (R - R0) / dR;
	int int_bin = (int)bin;
	real_t w = bin - int_bin;

	real_t re_int_bin, re_int_bin_p1, im_int_bin, im_int_bin_p1,
	  sample_re, sample_im;

	// Hide the following non-affine accesses into a blackbox whose
	// behavior we describe in an image function.
	// For backprojection, the specification of the algorithm is that the
	// PSR are readonly so whatever behavior we specify for the accesses
	// will be correct and used as locality hints only.
	// real_t sample_re = (1-w) * aPSRsRe[i][int_bin] +
	//   w * aPSRsRe[i][int_bin + 1];
	// real_t sample_im = (1-w) * aPSRsIm[i][int_bin] +
	//   w * aPSRsIm[i][int_bin + 1];

	fill_re_int_bin(aPSRsRe, x, y, i, int_bin, &re_int_bin, &re_int_bin_p1);
	fill_im_int_bin(aPSRsIm, x, y, i, int_bin, &im_int_bin, &im_int_bin_p1);

	sample_re = (1-w) * re_int_bin + w * re_int_bin_p1;
	sample_im = (1-w) * im_int_bin + w * im_int_bin_p1;

	acc = acc +
	  (real_t)(sample_re * cos((2*PI*R)/Iiter) -
		   sample_im * sin((2*PI*R)/Iiter));
      }

      aIm[x][y] = acc;
    }
  }
}


// Here we specify that the problem size is at least 16^3 and at most 2000^3.
// This helps RStream prune some trivial uninteresting cases.
#pragma rstream map split_read_write_references "context_hidden:pY>=16,pX>=16,pI>=16,pY<=2000,pX<=2000,pI<=2000"
void sar_backprojection_outer(real_t (*aIm)[Iy],
			      real_t (*aPSRsRe)[RET_PER_PULSE],
			      real_t (*aPSRsIm)[RET_PER_PULSE],
			      real_t (*positions)[3],
			      int pY, int pX, int pI) {
  sar_backprojection(aIm, aPSRsRe, aPSRsIm, positions, pY, pX, pI);
}

// The kernel
void kernel()
{
  sar_backprojection_outer((real_t (*)[Iy])Im,
			   (real_t (*)[RET_PER_PULSE])PSRsRe,
			   (real_t (*)[RET_PER_PULSE])PSRsIm,
			   (real_t (*)[3])positions,
			   Yiter, Xiter, Iiter);
}


// Checks the data
int check() {
  int i, j;
  sar_backprojection((real_t (*)[Iy])Imcheck,
		     (real_t (*)[RET_PER_PULSE])PSRsRe,
		     (real_t (*)[RET_PER_PULSE])PSRsIm,
		     (real_t (*)[3])positions,
		     Yiter, Xiter, Iiter);

  return check_matrices_portions("Im", "Imcheck", Im, Imcheck, Xiter, Yiter, Iy);
}

// Number of flops in the kernel
double flops_per_trial() {
  double n = (double) Yiter * (double) Xiter * (double) Iiter;
  // 7 adds, 6 mul, 1 sqrt, 1 cos, 1 sin
  return (double) (n*7 + n*6 + n*1 + n*1 + n*1);
}
int nb_samples = 0;
char const * function_name = "sar_backprojection";


// Extraneous machinery to run the code, verify the output and show the performance
#include "support.h"

int main(int argc, char **argv) {
  double t0, t1, total_time, flops_per_second;
  int i, rc;

  process_arguments(argc, argv);

  initialize_once();
  total_time = 0;

  for (i = 0; i < max_trials; i++) {
    initialize();
    coldify_caches();

    t0 = get_time_in_seconds();

    kernel();

    t1 = get_time_in_seconds();

    total_time += t1 - t0;

    free(junk);
  }

  if (check_results) {
    rc = check();
  } else {
    rc = 0;
  }

  flops_per_second = flops_per_trial() * max_trials / total_time;

  double flops_per_measure = flops_per_second / 1e9;

  printf("%s: trials=%d time=%gs %s=%g %s\n",
	 argv[0],
	 max_trials,
	 total_time,
	 "gflop/s",
	 flops_per_measure,
	 (check_results ? (rc ? "(FAILED)" : "(PASSED)") : ""));
  fprintf(stderr, "%s , %d, %g , %g, %d\n", function_name,
	  nb_samples, total_time/max_trials, flops_per_trial(), rc);

  return rc;
}
