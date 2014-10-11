#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

// needed if using -std=c99
#ifndef M_PI
#define M_PI 3.141592653589793223846f
#endif

#include "common.h"

#define RGB(r,g,b) ( ( (r) << 16 ) | ( (g) << 8 ) | ( (b) << 0 ) )
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

// Radar parameters
	float prf;		// Pulse repetition frequency (Hz)
const	float BW = 600e6;	// Bandwidth (Hz)
	float fs = 600e6;	// Sampling Frequency (Sa/s)
	float fc = 10e9;	// Carrier frequency (Hz)

// Simulation parameters
	int rngBinDev;		// Range bin deviation (determines the amount of perturbation in platform trajectory)
	int numImages;		// Number of images to simulate
	float v;		// Platform velocity (m/s)
	float r0;		// Range from platform to scene center (m)
	float cr;		// Cross-Range resolution (m)
	float theta_el;		// Elevation angle (degrees)
	float edgeSize;		// Ground square edge size (m)

// Target info
	int numTargets;		// Number of targets in scene
	int *target_last;	// Last image target appears in
	int *target_first;	// First image target appears in
	float *target_x;	// X location of target relative to origin (m)
	float *target_y;	// Y location of target relative to origin (m)
	float *target_z;	// Y location of target relative to origin (m)
	float *target_b;	// Y location of target relative to origin (m)

// Misc. values
	int Nx;			// Number of range samples
	int numPulses;		// Number of pulses per image
	int pulse_counter; 	// Number of pulses started in the current image
	float R;		// Range from platform to target (m)
	float w;		// Platform angular velocity (rad/s)
	float dr;		// Down-Range resolution (m)
	float Tp;		// Pulse transmission timestamp
	float el;		// Platform eleveation (m)
	float pri;		// Pulse rate interval (s)
	float Rmin;		// Distance from platform to start of range swath (m)
	float Rswath;		// Range swath (m)
	float theta, phi;	// Azimuth angle, Polar angle

int main(int argc, char **argv) {
	struct bmpfile_magic {
		unsigned char magic[2];
	} bmp_magic;

	struct bmpfile_header {
		uint32_t filesz;
		uint16_t creator1;
		uint16_t creator2;
		uint32_t bmp_offset;
	} bmp_header;

	struct bmpfile_info_header {
		uint32_t header_sz;
		int32_t  width;
		int32_t  height;
		uint16_t nplanes;
		uint16_t bitspp;
		uint32_t compress_type;
		uint32_t bmp_bytesz;
		int32_t  hres;
		uint32_t vres;
		uint32_t ncolors;
		uint32_t nimpcolors;
	} bmp_info_header;

	int P1; // Y dim is speed of sound
	int S1; // X dim is speed of light

	typedef uint32_t      Pixel_t;
	typedef float complex FreqDomain_t;

	Pixel_t      **PixelArray;
	FreqDomain_t **FreqDomain;
	int image = 0;
	float gain = 0.0f;

	// look at argv for which image to process

	if (argc >= 2)
		image = atoi(argv[1]);
	else
		image = 1;
	if (argc >= 3)
		gain  = atof(argv[2]);
	else
		gain = 1;
	if ( (argc != 2) && (argc != 3) ) {
		fprintf(stderr,"Usage: ./viewData <image #> <gain>\n<image #>: 1 => ref and 2 => curr\n<gain>: (default 1.0) used to scale the detected data before clamping between 0 and 255\nBMP output written to stdout\n",image, gain);fflush(stderr);
		exit(0);
	}

	// Read parameters

	switch(ReadParams()) {
	  case 1:
		printf("Parameters.txt not found. Exiting.\n");
		return 1;
	  case 2:
		printf("Parameters.txt does not adhere to expected format. Exiting.\n");
		return 1;
	  default:
		break;
	}

	switch(ReadTargets()) {
	  case 1:
		printf("Targets.txt not found. Exiting.\n");
		return 1;
	  default:
		break;
	}

	// Calculate dependent variables

	w = v/r0;
	dr = c_mks_mps/2/fs;
	pri = 1/prf;
	Rswath = sqrtf(2.0)*edgeSize*cosf(theta_el*M_PI/180.0);
	S1 = (int)(fs*2*Rswath/c_mks_mps+ 0.5);
	P1 = (int)ceilf(r0*c_mks_mps*prf/(2*v*cr*fc));

fprintf(stderr,"S1 = %d, P1 = %d, edgeSize = %f\n",S1,P1,edgeSize);fflush(stderr);

fprintf(stderr,"malloc PixalArray\n");fflush(stderr);
	PixelArray = (Pixel_t **)malloc(P1*sizeof(Pixel_t*));
	if(!PixelArray) {perror("malloc of PixelArray[]");exit(1);}
	for(int iy=0;iy<P1;iy++) {
		PixelArray[iy] = (Pixel_t *)malloc(S1*sizeof(Pixel_t));
		if(!PixelArray[iy]) {perror("malloc of PixelArray[][]");exit(1);}
	}

fprintf(stderr,"malloc FreqDomain\n");fflush(stderr);
	FreqDomain = (FreqDomain_t **)malloc(P1*sizeof(FreqDomain_t *));
	if(!FreqDomain) {perror("malloc of FreqDomain[]");exit(1);}
	for(int iy=0;iy<P1;iy++) {
		FreqDomain[iy] = (FreqDomain_t *)malloc(S1*sizeof(FreqDomain_t));
		if(!FreqDomain[iy]) {perror("malloc of FreqDomain[][]");exit(1);}
	}

fprintf(stderr,"init bmp header\n");fflush(stderr);
	bmp_magic.magic[0] = 'B';
	bmp_magic.magic[1] = 'M';

	bmp_header.filesz     = sizeof(bmp_magic)
                              + sizeof(bmp_header)
                              + sizeof(bmp_info_header)
                              + P1*S1*4;
	bmp_header.creator1   = -1;
	bmp_header.creator2   = -1;
	bmp_header.bmp_offset = sizeof(bmp_magic)
                              + sizeof(bmp_header)
                              + sizeof(bmp_info_header);

	bmp_info_header.header_sz     = sizeof(bmp_info_header);
	bmp_info_header.width         = S1;
	bmp_info_header.height        = P1;
	bmp_info_header.nplanes       = 1;
	bmp_info_header.bitspp        = 32;
	bmp_info_header.compress_type = 0;
	bmp_info_header.bmp_bytesz    = 0;
	bmp_info_header.hres          = 1;
	bmp_info_header.vres          = 1;
	bmp_info_header.ncolors       = 0;
	bmp_info_header.nimpcolors    = 0;

	for(int iy=0;iy<P1;iy++) {
		for(int ix=0;ix<S1;ix++) {
			PixelArray[iy][ix] = RGB(    0,    0,    0);
		}
	}

	for(int iImage=1;iImage<=2;iImage++) {
fprintf(stderr,"read FreqDomain image %d\n",iImage);fflush(stderr);
		for(int iy=0;iy<P1;iy++) {
			fread(&FreqDomain[iy][0],sizeof(FreqDomain_t),S1,stdin);
		}
		if( iImage == image) break;
	} // for iImage

fprintf(stderr,"detect FreqDomain\n");fflush(stderr);
	double total = 0.0;
	for(int iy=0;iy<P1;iy++) {
		for(int ix=0;ix<S1;ix++) {
			const float re = crealf(FreqDomain[iy][ix]);
			const float im = cimagf(FreqDomain[iy][ix]);
			const float mag = sqrtf(re*re+im*im);
			FreqDomain[iy][ix] = mag + I*0.0f;
			total += mag;
		}
	}

fprintf(stderr,"max FreqDomain\n");fflush(stderr);
	float max_mag =       0.0f;
	float min_mag = 1000000.0f;
	int   imax_mag =        -1;
	int   imin_mag =        -1;
	for(int iy=0;iy<P1;iy++) {
		for(int ix=0;ix<S1;ix++) {
			if(max_mag < crealf(FreqDomain[iy][ix]) ) {
				max_mag  = crealf(FreqDomain[iy][ix]);
				imax_mag = iy<<16+ix;
			}
			if(min_mag > crealf(FreqDomain[iy][ix]) ) {
				min_mag  = crealf(FreqDomain[iy][ix]);
				imin_mag = iy<<16+ix;
			}
		}
	}
fprintf(stderr,"max is %f\nmin is %f\n",max_mag,min_mag);fflush(stderr);
fprintf(stderr,"imax is %8.8x\nimin is %8.8x\n",imax_mag,imin_mag);fflush(stderr);
fprintf(stderr,"tot is %f\navg is %f, gain = %f\n",total,total/(P1*S1),gain);fflush(stderr);

	uint64_t hist[256];
	for(int i=0;i<256;i++) hist[i] = 0;

fprintf(stderr,"scale FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<P1;iy++) {
		for(int ix=0;ix<S1;ix++) {
			const float re = crealf(FreqDomain[iy][ix]);
			const uint8_t umag = fminf(re*gain,255.0f);
			PixelArray[iy][ix] |= RGB( umag,    0,    0);
			hist[umag]++;
		}
	}

#if 0
for(int i=0;i<256;i++)
fprintf(stderr,"hist[%3d] = %8lld %e = %f\n",
i,hist[i],(double)hist[i]*(double)i,(double)hist[i]*(double)i/total);
fflush(stderr);
#endif

	for(int iTarget=0;iTarget<numTargets;iTarget++) {
		if( (target_first[iTarget] <= image) && (image <= target_last[iTarget]) ) {
			int iy = (int)fmaxf(0.0f,fminf((float)(P1-1),target_y[iTarget]*(P1/edgeSize)+(P1/2)));
			int ix = (int)fmaxf(0.0f,fminf((float)(S1-1),target_x[iTarget]*(S1/edgeSize)+(S1/2)));
fprintf(stderr,"t <%3d,%3d> <%f,%f>\n",iy,ix,target_y[iTarget],target_x[iTarget]);fflush(stderr);
			for(int iiy=iy-2;iiy<=iy+2;iiy++) {
			  for(int iix=ix-2;iix<=ix+2;iix++) {
			    if( target_first[iTarget] != target_last[iTarget] ) {
			    	PixelArray[MAX(0,MIN(iiy,P1-1))][MAX(0,MIN(iix,S1-1))] |= RGB(    0,    0,  255);
			    } // targets in all images
			    else {
				PixelArray[MAX(0,MIN(iiy,P1-1))][MAX(0,MIN(iix,S1-1))] |= RGB(    0,  255,    0);
			    } // target only in last image
			  } // for iix
			} // for iiy
		} // if first and last
	} // for iTarget

fprintf(stderr,"write bmp file\n");fflush(stderr);
	fwrite(&bmp_magic,       sizeof(bmp_magic), 1, stdout);
	fwrite(&bmp_header,      sizeof(bmp_header), 1, stdout);
	fwrite(&bmp_info_header, sizeof(bmp_info_header), 1, stdout);
	for(int iy=0;iy<P1;iy++) {
		fwrite(&PixelArray[iy][0], sizeof(Pixel_t), S1, stdout);
	}

fprintf(stderr,"free FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<P1;iy++) {
		free(FreqDomain[iy]);
	}
	free(FreqDomain);

fprintf(stderr,"free PixelArray\n");fflush(stderr);
	for(int iy=0;iy<P1;iy++) {
		free(PixelArray[iy]);
	}
	free(PixelArray);

fprintf(stderr,"done\n");fflush(stderr);
	exit(0);
}
