#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "ocr.h"
#include "common.h"

struct Inputs in;			// Inputs
struct point **corr_map;		// Correlation map
struct DigSpotVars dig_spot;		// Digital spotlight variables
struct complexData **curImage;		// Current image
struct complexData **refImage;		// Reference (previous) image
struct CfarParams cfar_params;		// CFAR parameters
struct RadarParams radar_params;	// Radar parameters
struct ImageParams image_params;	// Image parameters
struct AffineParams affine_params;	// Affine registration parameters

int main(int argc, char **argv) {

typedef struct complexData FreqDomain_t;
	int wantFileType = 0;

	if (argc == 2) {
		wantFileType = atoi(argv[1]);
	} else {
		fprintf(stderr,"Usage: ./dataImage <file type #>\nwhere 1 >= Data, 2=> Position, 3 => PulseTime\n ASCII output written to stdout\n");fflush(stderr);
		exit(0);
	}

fprintf(stderr,"process FileType number %d\n",wantFileType);fflush(stderr);

	switch(ReadParams(&radar_params, &image_params, &affine_params, &cfar_params)) {
	  case 1:
		  printf("Unable to open Parameters.txt\n");
		  exit(1);
	  case 2:
		  printf("Parameters.txt does not adhere to expected format.\n");
		  exit(1);
	}

	// Ensure all window sizes are odd
	if( !(affine_params.Sc % 2) ) {
		printf("Sc must be odd. Exiting.\n");
		exit(1);
	}

	if( !(image_params.Ncor % 2) ) {
		printf("Ncor must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		printf("Ncfar must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		printf("Nguard must be odd. Exiting.\n");
		exit(1);
	}

	// Calculate dependent variables
	image_params.TF = image_params.Ix/image_params.Sx;
	image_params.dr = c_mks_mps/radar_params.fs/2/image_params.F;
	image_params.P2 = image_params.P1/image_params.TF;
	image_params.S2 = image_params.S1/image_params.TF;

        if( wantFileType == 1 ) { // Data.bin -> Data.c
	    for(int image = 0; image < image_params.numImages; image++) {
		FreqDomain_t **FreqDomain;
fprintf(stderr,"process image number %d\n",image);fflush(stderr);

fprintf(stderr,"malloc FreqDomain\n");fflush(stderr);
		FreqDomain = (FreqDomain_t **)malloc(image_params.P1*sizeof(FreqDomain_t *));
		if(!FreqDomain) {perror("malloc of FreqDomain[]");exit(1);}
		for(int iy=0;iy<image_params.P1;iy++) {
			FreqDomain[iy] = (FreqDomain_t *)malloc(image_params.S1*sizeof(FreqDomain_t));
			if(!FreqDomain[iy]) {perror("malloc of FreqDomain[][]");exit(1);}
		}

fprintf(stderr,"read FreqDomain (binary)\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			fread(&FreqDomain[iy][0],sizeof(FreqDomain_t),image_params.S1,stdin);
		}

fprintf(stderr,"write FreqDomain (ascii)\n");fflush(stderr);
		printf("#include <stdint.h>\n");
		printf("struct {int32_t real;int32_t imag;} image_%d[%d][%d] = {\n",image,image_params.P1,image_params.S1);
		for(int iy=0;iy<image_params.P1;iy++) {
			printf("{");
			for(int ix=0;ix<image_params.S1;ix++) {
	        		printf("{0x%8.8x,0x%8.8x},", *(int32_t *)&FreqDomain[iy][ix].real, *(int32_t *)&FreqDomain[iy][ix].imag);
		        }
			printf("},\n");
		}
		printf("};\n");

	fprintf(stderr,"free FreqDomain\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			free(FreqDomain[iy]);
		}
		free(FreqDomain);
	    } // end for image
	} // end wantFileType == 1
	else if ( wantFileType == 2 ) {
	    for(int image = 0; image < image_params.numImages; image++) {
typedef struct {int32_t x; int32_t y; int32_t z; } PlatformPosition_t;
            PlatformPosition_t **PlatformPosition;
fprintf(stderr,"process image number %d\n",image);fflush(stderr);

fprintf(stderr,"malloc PlatformPosition\n");fflush(stderr);
		PlatformPosition = (PlatformPosition_t **)malloc(image_params.P1*sizeof(PlatformPosition_t *));
		if(!PlatformPosition) {perror("malloc of PlatformPosition[]");exit(1);}
		for(int iy=0;iy<image_params.P1;iy++) {
			PlatformPosition[iy] = (PlatformPosition_t *)malloc(sizeof(PlatformPosition_t));
			if(!PlatformPosition[iy]) {perror("malloc of PlatformPosition[][]");exit(1);}
		}

fprintf(stderr,"read PlatformPosition (binary)\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			fread(&PlatformPosition[iy][0],sizeof(PlatformPosition_t),1,stdin);
		}

fprintf(stderr,"write PlatformPosition (ascii)\n");fflush(stderr);
		printf("#include <stdint.h>\n");
		printf("struct {int32_t x;int32_t y; int32_t z;} platform_%d[%d][%d] = {\n",image,image_params.P1,1);
		for(int iy=0;iy<image_params.P1;iy++) {
			printf("{");
			for(int ix=0;ix<1;ix++) {
	        		printf("{0x%8.8x,0x%8.8x,0x%8.8x},",
				*(int32_t *)&PlatformPosition[iy][ix].x,
				*(int32_t *)&PlatformPosition[iy][ix].y,
				*(int32_t *)&PlatformPosition[iy][ix].z);
		        }
			printf("},\n");
		}
		printf("};\n");

	fprintf(stderr,"free PlatformPosition\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			free(PlatformPosition[iy]);
		}
		free(PlatformPosition);
	    } // end for image
	} // end wantFileType == 2
        else if( wantFileType == 3 ) {
	    for(int image = 0; image < image_params.numImages; image++) {
typedef struct {int32_t t; } PulseTransmission_t;
            PulseTransmission_t **PulseTransmission;
fprintf(stderr,"process image number %d\n",image);fflush(stderr);

fprintf(stderr,"malloc PulseTransmission\n");fflush(stderr);
		PulseTransmission = (PulseTransmission_t **)malloc(image_params.P1*sizeof(PulseTransmission_t *));
		if(!PulseTransmission) {perror("malloc of PulseTransmission[]");exit(1);}
		for(int iy=0;iy<image_params.P1;iy++) {
			PulseTransmission[iy] = (PulseTransmission_t *)malloc(sizeof(PulseTransmission_t));
			if(!PulseTransmission[iy]) {perror("malloc of PulseTransmission[][]");exit(1);}
		}

fprintf(stderr,"read PulseTransmission (binary)\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			fread(&PulseTransmission[iy][0],sizeof(PulseTransmission_t),1,stdin);
		}

fprintf(stderr,"write PulseTransmission (ascii)\n");fflush(stderr);
		printf("#include <stdint.h>\n");
		printf("struct {int32_t t;} pulse_%d[%d][%d] = {\n",image,image_params.P1,1);
		for(int iy=0;iy<image_params.P1;iy++) {
			printf("{");
			for(int ix=0;ix<1;ix++) {
	        		printf("{0x%8.8x},",
				*(int32_t *)&PulseTransmission[iy][ix].t);
		        }
			printf("},\n");
		}
		printf("};\n");

	fprintf(stderr,"free PulseTransmission\n");fflush(stderr);
		for(int iy=0;iy<image_params.P1;iy++) {
			free(PulseTransmission[iy]);
		}
		free(PulseTransmission);
	    } // end for image
	} // end wantFileType == 3

fprintf(stderr,"done\n");fflush(stderr);
	exit(0);
}
