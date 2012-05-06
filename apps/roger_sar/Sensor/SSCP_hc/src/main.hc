/*****************************************************************
 * Streaming Sensor Challenge Problem (SSCP) reference
 * implementation.
 *
 * USAGE: SSCP.exe <Data input file> <Platform position input
 *        file> <Pulse transmission time input file> <Output
 *		  file containing detects>
 *
 * NOTES: The code expects Parameters.txt to be located in the 
 * working directory. The Parameters.txt file is
 * described below.
 *
 * Fs: Sample frequency, Hz
 * Fc: Carrier frequency, Hz
 * PRF: Pulse Repetition Frequency, Hz
 * F: Oversampling factor
 * Ix: Number of pixels in x dimension of full image, pixels
 * Iy: Number of pixels in y dimension of full image, pixels
 * Sx: Number of pixels in x dimension of subimage, pixels
 * Sy: Number of pixels in y dimension of subimage, pixels
 * P1: Number of pulses per image
 * S1: Number of samples per pulse
 * r0: Range from platform to scene center, m
 * R0: Range of the zeroth range bin, m
 * Nc: Number of affine registration control points
 * Rc: Range of affine registration
 * Sc: Affine registration neighborhood size
 * Tc: Affine registration correlation threshold
 * Nf: Number of thin-spline registration control points
 * Rf: Range of fine registration
 * Scf: Thin-spline neighborhood size
 * Tf: Thin-spline correlation threshold
 * Ncor: Coherent Change Detection (CCD) neighborhood size
 * Ncfar: Constant false alarm rate (CFAR) neighborhood size
 * Tcfar: CFAR threshold
 * Nguard: Number of guard cells for CFAR neighborhood
 * NumberImages: Number of images to process
 *
 * Written by: Brian Mulvaney, <brian.mulvaney@gtri.gatech.edu>
 *             Georgia Tech Research Institute
 ****************************************************************/

#include "common.h"
#include "assert.h"

// #define DEBUG_SSCP

struct Inputs in;					// Inputs
struct detects *Y;					// Detects list
struct point **corr_map;			// Correlation map
struct DigSpotVars dig_spot;		// Digital spotlight variables
struct complexData **curImage;		// Current image
struct complexData **refImage;		// Reference (previous) image
struct CfarParams cfar_params;		// CFAR parameters
struct RadarParams radar_params;	// Radar parameters
struct ImageParams image_params;	// Image parameters
struct AffineParams affine_params;	// Affine registration parameters
struct ThinSplineParams ts_params;	// Thin spline registration parameters

int main(int argc, char *argv[])
{
//	int m, n;
	int Ncor;
	FILE *pInFile, *pInFile2, *pInFile3;
	FILE *pOutFile;
//	extern struct reg_map *regmap;
	
#if defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST)
	pInFile  = NULL;
	pInFile2 = NULL;
	pInFile3 = NULL;
	pOutFile = stdout;
#else
	// SAR data
	if( (pInFile = fopen(argv[1], "rb")) == NULL ) {
		fprintf(stderr,"Error opening %s\n", argv[1]);
		exit(1);
	}

	// Platform positions
	if( (pInFile2 = fopen(argv[2], "rb")) == NULL ) {
		fprintf(stderr,"Error opening %s\n", argv[2]);
		exit(1);
	}

	// Pulse transmission timestamps
	if( (pInFile3 = fopen(argv[3], "rb")) == NULL ) {
		fprintf(stderr,"Error opening %s\n", argv[3]);
		exit(1);
	}

	pOutFile = fopen(argv[4], "wb");
#endif

	switch(ReadParams(&radar_params, &image_params, &affine_params, &ts_params, &Ncor, &cfar_params)) {
	  case 1:
		  fprintf(stderr,"Unable to open Parameters.txt\n");
		  exit(1);
	  case 2:
		  fprintf(stderr,"Parameters.txt does not adhere to expected format.\n");
		  exit(1);
	}

	// Ensure all window sizes are odd
	if( !(affine_params.Sc % 2) ) {
		fprintf(stderr,"Sc must be odd. Exiting.\n");
		exit(-1);
	}

	if( !(ts_params.Scf % 2) ) {
		fprintf(stderr,"Scf must be odd. Exiting.\n");
		exit(-1);
	}

	if( !(Ncor % 2) ) {
		fprintf(stderr,"Ncor must be odd. Exiting.\n");
		exit(-1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		fprintf(stderr,"Ncfar must be odd. Exiting.\n");
		exit(-1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		fprintf(stderr,"Nguard must be odd. Exiting.\n");
		exit(-1);
	}

	// Calculate dependent variables
	image_params.TF = image_params.Ix/image_params.Sx;
#ifdef RAG_PURE_FLOAT
	image_params.dr = c_mks_mps/radar_params.fs/2.0f/((float)image_params.F);
#else
	image_params.dr = c_mks_mps/radar_params.fs/2/image_params.F;
#endif
	image_params.P2 = image_params.P1/image_params.TF;
	image_params.S2 = image_params.S1/image_params.TF;

	// TF > 1 implies digital spotlighting, TF = 1 implies no digital spotlighting
	if(image_params.TF > 1) {
		image_params.P3 = image_params.P2;
		image_params.S3 = image_params.S2;

		// Calculate new zeroth range bin
		radar_params.R0_prime = radar_params.r0 - (radar_params.r0 - radar_params.R0)/image_params.TF;
#ifdef DEBUG_RAG
fprintf(stderr,"r0=%f, R0=%f, R0_prime=%f\n",radar_params.r0, radar_params.R0, radar_params.R0_prime);fflush(stderr);
#endif
		// Allocate memory for variables needed to perform digital spotlighting
		dig_spot.freqVec = (float*)malloc(image_params.S1*sizeof(float));
		if(dig_spot.freqVec == NULL) {
			fprintf(stderr,"Unable to allocate memory for freqVec.\n");
			exit(1);
		}
#ifdef DEBUG_RAG
fprintf(stderr,"dig_spot.freqVec %p %d\n",dig_spot.freqVec,image_params.S1); fflush(stderr);
#endif

		dig_spot.filtOut = (struct complexData*)malloc(image_params.P2*sizeof(struct complexData));
		if(dig_spot.filtOut == NULL) {
			fprintf(stderr,"Unable to allocate memory for filtOut.\n");
			exit(1);
		}

		dig_spot.X2 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X2 == NULL) {
			fprintf(stderr,"Error allocating memory for X2.\n");
			exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X2[n] = (struct complexData*)malloc(image_params.S1*sizeof(struct complexData));
			if (dig_spot.X2[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X2.\n");
				exit(1);
			}
		}
		
		dig_spot.X3 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X3 == NULL) {
			fprintf(stderr,"Error allocating memory for X3.\n");
			exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X3[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X3[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X3.\n");
				exit(1);
			}
		}

		dig_spot.X4 = (struct complexData**)malloc(image_params.P2*sizeof(struct complexData*));
		if(dig_spot.X4 == NULL) {
			fprintf(stderr,"Error allocating memory for X4.\n");
			exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.X4[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X4[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X4.\n");
				exit(1);
			}
		}

		dig_spot.tmpVector = (struct complexData*)malloc(image_params.P1*sizeof(struct complexData));
		if(dig_spot.tmpVector == NULL) {
			fprintf(stderr,"Unable to allocate memory for tmpVector.\n");
			exit(1);
		}

		dig_spot.Pt2 = (float**)malloc(image_params.P2*sizeof(float*));
		if(dig_spot.Pt2 == NULL) {
			fprintf(stderr,"Error allocating memory for Pt2.\n");
			exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.Pt2[n] = (float*)malloc(3*sizeof(float));
			if (dig_spot.Pt2[n] == NULL) {
				fprintf(stderr,"Error allocating memory for Pt2.\n");
				exit(1);
			}
		}

		// Create frequency vector (positive freqs followed by negative freqs)
		dig_spot.freqVec[0] = 0;
		if( !(image_params.S1 % 2) )
		{	// S1 even
			for(int n=1; n<image_params.S1/2; n++)
			{
				dig_spot.freqVec[n] = n*(radar_params.fs/image_params.S1);
				dig_spot.freqVec[image_params.S1-n] = -n*(radar_params.fs/image_params.S1);
			}
			dig_spot.freqVec[image_params.S1/2] = -radar_params.fs/2;
		}
		else
		{	// S1 odd
			for(int n=1; n<=image_params.S1/2; n++)
			{
				dig_spot.freqVec[n] = n*(radar_params.fs/image_params.S1);
				dig_spot.freqVec[image_params.S1-n] = -n*(radar_params.fs/image_params.S1);
			}
		}
	}
	else {
		image_params.P3 = image_params.P1;
		image_params.S3 = image_params.S1;
	}

	image_params.S4 = (int)ceilf(image_params.F*image_params.S3);

	// Allocate memory for axis vectors
	image_params.xr = (float*)malloc(image_params.Ix*sizeof(float));
	image_params.yr = (float*)malloc(image_params.Iy*sizeof(float));
	if(image_params.xr == NULL || image_params.yr == NULL) {
		fprintf(stderr,"Error allocating memory for axis vectors.\n");
		exit(1);
	}

	// Create axis vectors
	for(int n=0; n<image_params.Ix; n++) {
		image_params.xr[n] = (n - floorf((float)image_params.Ix/2))*image_params.dr;
	}
	for(int n=0; n<image_params.Iy; n++) {
		image_params.yr[n] = (n - floorf((float)image_params.Iy/2))*image_params.dr;
	}

	// Allocate memory for pulse compressed SAR data
	in.X = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
	if(in.X == NULL) {
		fprintf(stderr,"Error allocating memory for X.\n");
		exit(1);
	}
	for(int n=0; n<image_params.P1; n++) {
		in.X[n] = (struct complexData*)malloc(image_params.S1*sizeof(struct complexData));
		if (in.X[n] == NULL) {
			fprintf(stderr,"Error allocating memory for X.\n");
			exit(1);
		}
	}

	// Allocate memory for transmitter positions at each pulse
	in.Pt = (float**)malloc(image_params.P1*sizeof(float*));
	if(in.Pt == NULL) {
		fprintf(stderr,"Error allocating memory for Pt.\n");
		exit(1);
	}
	for(int n=0; n<image_params.P1; n++) {
		in.Pt[n] = (float*)malloc(3*sizeof(float));
		if(in.Pt[n] == NULL) {
			fprintf(stderr,"Error allocating memory for Pt.\n");
			exit(1);
		}
	}

	// Allocate memory for timestamp of pulse transmissions
	in.Tp = (float*)malloc(image_params.P1*sizeof(float));
	if(in.Tp == NULL) {
		fprintf(stderr,"Error allocating memory for Tp.\n");
		exit(1);
	}

	// Allocate memory for current image
	curImage = (struct complexData**)malloc(image_params.Iy*sizeof(struct complexData*));
	if(curImage == NULL) {
		fprintf(stderr,"Error allocating memory for curImage.\n");
		exit(1);
	}
	for(int n=0; n<image_params.Iy; n++) {
		curImage[n] = (struct complexData*)malloc(image_params.Ix*sizeof(struct complexData));
		if (curImage[n] == NULL) {
			fprintf(stderr,"Error allocating memory for curImage.\n");
			exit(1);
		}
	}

	// Allocate memory for reference image
	refImage = (struct complexData**)malloc(image_params.Iy*sizeof(struct complexData*));
	if(refImage == NULL) {
		fprintf(stderr,"Error allocating memory for refImage.\n");
		exit(1);
	}
	for(int n=0; n<image_params.Iy; n++) {
		refImage[n] = (struct complexData*)malloc(image_params.Ix*sizeof(struct complexData));
		if (refImage[n] == NULL) {
			fprintf(stderr,"Error allocating memory for refImage.\n");
			exit(1);
		}
	}

	// Allocate memory for correlation map
	corr_map = (struct point**)malloc((image_params.Iy-Ncor+1)*sizeof(struct point*));
	if(corr_map == NULL) {
		fprintf(stderr,"Error allocating memory for correlation map.\n");
		exit(1);
	}
	for(int m=0; m<image_params.Iy-Ncor+1; m++)
	{
		corr_map[m] = (struct point*)malloc((image_params.Ix-Ncor+1)*sizeof(struct point));
		if (corr_map[m] == NULL) {
			fprintf(stderr,"Error allocating memory for correlation map.\n");
			exit(1);
		}
	}

	// Allocate memory for detection list
	Y = (struct detects*)malloc((image_params.Iy-Ncor-cfar_params.Ncfar+2)*(image_params.Ix-Ncor-cfar_params.Ncfar+2)*sizeof(struct detects));
	if(Y == NULL) {
		fprintf(stderr,"Error allocating memory for detection list.\n");
		exit(1);
	}

	// Read input data
	ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);

	// Form first image
finish {FormImage(&dig_spot, &image_params, &in, curImage, &radar_params); }

	while(--image_params.numImages) {

		ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);
		
		for(int m=0; m<image_params.Iy; m++) {
			memcpy(&refImage[m][0], &curImage[m][0], image_params.Ix*sizeof(struct complexData));
		}

		// Form current image
finish {	FormImage(&dig_spot, &image_params, &in, curImage, &radar_params); }
#ifdef RAG_AFFINE_ON
#ifdef TRACE
fprintf(stderr,"Affine registration\n");fflush(stderr);
#endif
		// Affine registration
finish {	Affine(&affine_params, &image_params, curImage, refImage); }
#endif

		// Thin-spline registration
#ifdef RAG_THIN_ON	// RAG WAS COMMENTED OUT
#ifdef TRACE
fprintf(stderr,"Thin-spline registration\n");fflush(stderr);
#endif
		ThinSpline(&ts_params, &image_params, curImage, refImage);
#endif

#ifdef TRACE
fprintf(stderr,"Coherent Change Detection (Ncor = %d)\n",Ncor);fflush(stderr);
#endif
		// Coherent Change Detection
finish {        CCD(Ncor, corr_map, &image_params, curImage, refImage); }

#ifdef TRACE
fprintf(stderr,"Constant False Alarm Rate\n");fflush(stderr);
#endif
		// Constant False Alarm Rate
finish {	CFAR(&image_params, Ncor, &cfar_params, corr_map, Y, pOutFile); }

	} // while images to process

#ifdef DEBUG_SSCP
#ifdef TRACE
fprintf(stderr,"Output Images to .bins\n");fflush(stderr);
#endif
    {
        FILE *pOutImg = fopen("images_debug.bin", "wb");
        FILE *pOutCorr = fopen("corr_debug.bin", "wb");

        assert(pOutImg != NULL);
        assert(pOutCorr != NULL);

        for(int m=0; m<image_params.Iy; m++)
            fwrite(&curImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);
        for(int m=0; m<image_params.Iy; m++)
            fwrite(&refImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);
        for(int m=0; m<image_params.Iy-Ncor+1; m++)
            fwrite(&corr_map[m][0], sizeof(struct point), image_params.Ix-Ncor+1, pOutCorr);
        //for(int m=0; m<affine_params.Nc; m++)
        //    fwrite(&regmap[m], sizeof(struct reg_map), 1, pOutFile);

        fclose(pOutImg);
        fclose(pOutCorr);
    }
#endif

#if !(defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST))
	fclose(pInFile);
	fclose(pInFile2);
	fclose(pInFile3);
	fclose(pOutFile);
#endif

	return 0;
}
