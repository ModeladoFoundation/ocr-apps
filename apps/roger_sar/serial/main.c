#include "rag_rmd.h"

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

#if defined(RAG_AFL) || defined(RAG_SIM)

// main codelet for rmd's runtime to start

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
#ifdef TRACE
if(n_db) {
xe_printf("enter main_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld\n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);RAG_FLUSH;
} else {
xe_printf("enter main_codelet arg %ld n_db %d\n",arg,n_db);RAG_FLUSH;
}
#endif
#else
int main(int argc, char *argv[])
{
#endif
	int Nd, Ncor;
	FILE *pInFile, *pInFile2, *pInFile3;
#ifndef RAG_SIM
	FILE *pOutFile;
#endif
	struct Inputs in;			// Inputs
	struct detects *Y;			// Detects list
	struct point **corr_map;		// Correlation map
#if RAG_DIG_SPOT_ON
	struct DigSpotVars dig_spot;		// Digital spotlight variables
#endif
	struct complexData **curImage;		// Current image
	struct complexData **refImage;		// Reference (previous) image

	struct CfarParams cfar_params;		// CFAR parameters
	struct RadarParams radar_params;	// Radar parameters
	struct ImageParams image_params;	// Image parameters
	struct AffineParams affine_params;	// Affine registration parameters
	struct ThinSplineParams ts_params;	// Thin spline registration parameters

	rmd_guid_t in_dbg;      	// Inputs
#if RAG_DIG_SPOT_ON
	rmd_guid_t dig_spot_dbg;	// Digital spotlight variables
#endif

	rmd_guid_t cfar_params_dbg;	// CFAR parameters datablock guid
	rmd_guid_t radar_params_dbg;	// Radar parameters datablock guid
	rmd_guid_t image_params_dbg;	// Image parameters datablock guid
	rmd_guid_t affine_params_dbg;	// Affine registration parameters datablock guid
	rmd_guid_t ts_params_dbg;	// Thin spline registration parameters datablock guid

	struct Inputs *in_ptr;			// Inputs
#if RAG_DIG_SPOT_ON
	struct DigSpotVars *dig_spot_ptr;	// Digital spotlight variables
#endif

	struct CfarParams *cfar_params_ptr;	// CFAR parameters
	struct RadarParams *radar_params_ptr;	// Radar parameters
	struct ImageParams *image_params_ptr;	// Image parameters
	struct AffineParams *affine_params_ptr;	// Affine registration parameters
	struct ThinSplineParams *ts_params_ptr;	// Thin spline registration parameters

	in_ptr       = bsm_malloc(&in_dbg      ,  sizeof(struct Inputs));
	if(in_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for in data block.\n");
		xe_exit(1);
	}
#if RAG_DIG_SPOT_ON
	dig_spot_ptr = bsm_malloc(&dig_spot_dbg,  sizeof(struct DigSpotVars));
	if(dig_spot_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for dig_spot data block.\n");
		xe_exit(1);
	}
#endif

	cfar_params_ptr   = bsm_malloc(&cfar_params_dbg,  sizeof(struct CfarParams));
	radar_params_ptr  = bsm_malloc(&radar_params_dbg, sizeof(struct RadarParams));
	image_params_ptr  = bsm_malloc(&image_params_dbg, sizeof(struct ImageParams));
	affine_params_ptr = bsm_malloc(&affine_params_dbg,sizeof(struct AffineParams));
	ts_params_ptr     = bsm_malloc(&ts_params_dbg,    sizeof(struct ThinSplineParams));

	if(cfar_params_ptr == NULL
	|| radar_params_ptr == NULL
	|| image_params_ptr == NULL
	|| affine_params_ptr == NULL
	|| ts_params_ptr == NULL ) {
		fprintf(stderr,"Error allocating memory for params blocks.\n");
		xe_exit(1);
	}

#if defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST)
	pInFile  = NULL;
	pInFile2 = NULL;
	pInFile3 = NULL;
#ifndef RAG_SIM
	pOutFile = stdout;
#endif
#else
#ifdef TRACE
xe_printf("// SAR data\n");RAG_FLUSH;
#endif
	if( (pInFile = fopen(argv[1], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[1]);
		xe_exit(1);
	}

#ifdef TRACE
xe_printf("// Platform positions\n");RAG_FLUSH;
#endif
	if( (pInFile2 = fopen(argv[2], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[2]);
		xe_exit(1);
	}

#ifdef TRACE
xe_printf("// Pulse transmission timestamps\n");RAG_FLUSH;
#endif
	if( (pInFile3 = fopen(argv[3], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[3]);
		xe_exit(1);
	}

	pOutFile = fopen(argv[4], "wb");
#endif

	switch(ReadParams(&radar_params, &image_params, &affine_params, &ts_params, &Ncor, &cfar_params)) {
	  case 0: break;
	  case 1:
		  fprintf(stderr,"Unable to open Parameters.txt\n");
		  xe_exit(1);
	  case 2:
		  fprintf(stderr,"Parameters.txt does not adhere to expected format.\n");
		  xe_exit(1);
	  default: 
		  fprintf(stderr,"Unexpected return value from ReadParams.\n");
		  xe_exit(1);
	}

#ifdef TRACE
xe_printf("// Ensure all window sizes are odd\n");RAG_FLUSH;
#endif
	if( !(affine_params.Sc % 2) ) {
		fprintf(stderr,"Sc must be odd. Exiting.\n");
		xe_exit(-1);
	}

	if( !(ts_params.Scf % 2) ) {
		fprintf(stderr,"Scf must be odd. Exiting.\n");
		xe_exit(-1);
	}

	if( !(Ncor % 2) ) {
		fprintf(stderr,"Ncor must be odd. Exiting.\n");
		xe_exit(-1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		fprintf(stderr,"Ncfar must be odd. Exiting.\n");
		xe_exit(-1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		fprintf(stderr,"Nguard must be odd. Exiting.\n");
		xe_exit(-1);
	}

#ifdef TRACE
xe_printf("// Calculate dependent variables\n");RAG_FLUSH;
#endif
	image_params.TF = image_params.Ix/image_params.Sx;
#if RAG_PURE_FLOAT
	image_params.dr = c_mks_mps/radar_params.fs/2.0f/((float)image_params.F);
#else
	image_params.dr = c_mks_mps/radar_params.fs/2/image_params.F;
#endif
	image_params.P2 = image_params.P1/image_params.TF;
	image_params.S2 = image_params.S1/image_params.TF;

#ifdef TRACE
xe_printf("// TF > 1 implies digital spotlighting, TF = 1 implies no digital spotlighting\n");RAG_FLUSH;
#endif
	if(image_params.TF > 1) {
#if !RAG_DIG_SPOT_ON
		xe_printf("!!! DIGITAL SPOTLIGHTING NOT YET SUPPORTED !!!\n");RAG_FLUSH;
		xe_exit(1);
#else	// RAG_DIG_SPOT_ON
#ifdef TRACE
xe_printf("// TF > 1 implies digital spotlighting\n");RAG_FLUSH;
#endif
		image_params.P3 = image_params.P2;
		image_params.S3 = image_params.S2;

#ifdef TRACE
xe_printf("// Calculate new zeroth range bin\n");RAG_FLUSH;
#endif
		radar_params.R0_prime = radar_params.r0 - (radar_params.r0 - radar_params.R0)/image_params.TF;
#ifdef TRACE
xe_printf("// Allocate memory for variables needed to perform digital spotlighting\n");RAG_FLUSH;
#endif
		dig_spot.freqVec = (float*)malloc(image_params.S1*sizeof(float));
		if(dig_spot.freqVec == NULL) {
			fprintf(stderr,"Unable to allocate memory for freqVec.\n");
			xe_exit(1);
		}

		dig_spot.filtOut = (struct complexData*)malloc(image_params.P2*sizeof(struct complexData));
		if(dig_spot.filtOut == NULL) {
			fprintf(stderr,"Unable to allocate memory for filtOut.\n");
			xe_exit(1);
		}

		dig_spot.X2 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X2 == NULL) {
			fprintf(stderr,"Error allocating memory for X2.\n");
			xe_exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X2[n] = (struct complexData*)malloc(image_params.S1*sizeof(struct complexData));
			if (dig_spot.X2[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X2.\n");
				xe_exit(1);
			}
		}
		
		dig_spot.X3 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X3 == NULL) {
			fprintf(stderr,"Error allocating memory for X3.\n");
			xe_exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X3[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X3[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X3.\n");
				xe_exit(1);
			}
		}

		dig_spot.X4 = (struct complexData**)malloc(image_params.P2*sizeof(struct complexData*));
		if(dig_spot.X4 == NULL) {
			fprintf(stderr,"Error allocating memory for X4.\n");
			xe_exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.X4[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X4[n] == NULL) {
				fprintf(stderr,"Error allocating memory for X4.\n");
				xe_exit(1);
			}
		}

		dig_spot.tmpVector = (struct complexData*)malloc(image_params.P1*sizeof(struct complexData));
		if(dig_spot.tmpVector == NULL) {
			fprintf(stderr,"Unable to allocate memory for tmpVector.\n");
			xe_exit(1);
		}

		dig_spot.Pt2 = (float**)malloc(image_params.P2*sizeof(float*));
		if(dig_spot.Pt2 == NULL) {
			fprintf(stderr,"Error allocating memory for Pt2.\n");
			xe_exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.Pt2[n] = (float*)malloc(3*sizeof(float));
			if (dig_spot.Pt2[n] == NULL) {
				fprintf(stderr,"Error allocating memory for Pt2.\n");
				xe_exit(1);
			}
		}

#ifdef TRACE
xe_printf("// Create frequency vector (positive freqs followed by negative freqs)\n");RAG_FLUSH;
#endif
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
#endif // RAG_DIG_SPOT_ON
	} else {
#ifdef TRACE
xe_printf("// TF = 1 implies no digital spotlighting\n");RAG_FLUSH;
#endif
		image_params.P3 = image_params.P1;
		image_params.S3 = image_params.S1;
	}

	image_params.S4 = (int)ceilf(image_params.F*image_params.S3);
#ifdef TRACE
xe_printf(" S4 %d F %d S3 %d\n",image_params.S4,image_params.F,image_params.S3);RAG_FLUSH;
#endif

#ifdef TRACE
xe_printf("// Allocate memory for axis vectors\n");RAG_FLUSH;
#endif
        rmd_guid_t image_params_xr_dbg;
        rmd_guid_t image_params_yr_dbg;
	image_params.xr = (float*)bsm_malloc(&image_params_xr_dbg,image_params.Ix*sizeof(float));
	image_params.yr = (float*)bsm_malloc(&image_params_yr_dbg,image_params.Iy*sizeof(float));
	if(image_params.xr == NULL || image_params.yr == NULL) {
		fprintf(stderr,"Error allocating memory for axis vectors.\n");
		xe_exit(1);
	}

#ifdef TRACE
xe_printf("// Create axis vectors\n");RAG_FLUSH;
#endif
	for(int i=0; i<image_params.Ix; i++) {
		RAG_PUT_FLT(&image_params.xr[i], (i - floorf((float)image_params.Ix/2))*image_params.dr);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_FLT(&image_params.yr[i],(i - floorf((float)image_params.Iy/2))*image_params.dr);
	}

#ifdef TRACE
xe_printf("// Allocate memory for pulse compressed SAR data\n");RAG_FLUSH;
#endif
        rmd_guid_t in_X_dbg;
	in.X = (struct complexData**)bsm_malloc(&in_X_dbg,image_params.P1*sizeof(struct complexData*));
	if(in.X == NULL) {
		fprintf(stderr,"Error allocating memory for X edge vector.\n");
		xe_exit(1);
	}
	in.X_edge_dbg = in_X_dbg;
        struct complexData* in_X_data_ptr = NULL; rmd_guid_t in_X_data_dbg;
	in_X_data_ptr = (struct complexData*)bsm_malloc(&in_X_data_dbg,image_params.P1*image_params.S1*sizeof(struct complexData));
	if ( in_X_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for X data.\n");
		xe_exit(1);
	}
	in.X_data_dbg = in_X_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		RAG_PUT_PTR(&in.X[i], in_X_data_ptr + i*image_params.S1);
	}

#ifdef TRACE
xe_printf("// Allocate memory for transmitter positions at each pulse\n");RAG_FLUSH;
#endif
	rmd_guid_t in_Pt_dbg;
	in.Pt = (float**)bsm_malloc(&in_Pt_dbg,image_params.P1*sizeof(float*));
	if(in.Pt == NULL) {
		fprintf(stderr,"Error allocating memory for Pt edge vector.\n");
		xe_exit(1);
	}
	in.Pt_edge_dbg = in_Pt_dbg;
	float * in_Pt_data_ptr = NULL; rmd_guid_t in_Pt_data_dbg;
	in_Pt_data_ptr = (float*)bsm_malloc(&in_Pt_data_dbg,image_params.P1*3*sizeof(float));
	if( in_Pt_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for Pt data.\n");
		xe_exit(1);
	}
	in.Pt_data_dbg = in_Pt_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		RAG_PUT_PTR(&in.Pt[i], in_Pt_data_ptr + i*3);
	}

#ifdef TRACE
xe_printf("// Allocate memory for timestamp of pulse transmissions\n");RAG_FLUSH;
#endif
	rmd_guid_t in_Tp_dbg;
	in.Tp = (float*)bsm_malloc(&in_Tp_dbg,image_params.P1*sizeof(float));
	if(in.Tp == NULL) {
		fprintf(stderr,"Error allocating memory for Tp.\n");
		xe_exit(1);
	}
	in.Tp_dbg = in_Tp_dbg;

#ifdef TRACE
xe_printf("// Allocate memory for current image\n");RAG_FLUSH;
#endif
	rmd_guid_t curImage_dbg;
	curImage = (struct complexData**)bsm_malloc(&curImage_dbg,image_params.Iy*sizeof(struct complexData*));
	if( curImage == NULL) {
		fprintf(stderr,"Error allocating memory for curImage edge vector.\n");
		xe_exit(1);
	}
	struct complexData *curImage_data_ptr; rmd_guid_t curImage_data_dbg;
	curImage_data_ptr = (struct complexData*)bsm_malloc(&curImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
	if (curImage_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for curImage data.\n");
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_PTR(&curImage[i], curImage_data_ptr + i*image_params.Ix);
	}

#ifdef TRACE
xe_printf("// Allocate memory for reference image\n");RAG_FLUSH;
#endif
	rmd_guid_t refImage_dbg;
	refImage = (struct complexData**)bsm_malloc(&refImage_dbg,image_params.Iy*sizeof(struct complexData*));
	if(refImage == NULL) {
		fprintf(stderr,"Error allocating memory for refImage edge vector.\n");
		xe_exit(1);
	}
	struct complexData *refImage_data_ptr; rmd_guid_t refImage_data_dbg;
	refImage_data_ptr = (struct complexData*)bsm_malloc(&refImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
	if (refImage_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for refImage data.\n");
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_PTR(&refImage[i], refImage_data_ptr + i*image_params.Ix);
	}

#ifdef TRACE
xe_printf("// Allocate memory for correlation map\n");RAG_FLUSH;
#endif
	rmd_guid_t corr_map_dbg;
	corr_map = (struct point**)bsm_malloc(&corr_map_dbg,(image_params.Iy-Ncor+1)*sizeof(struct point*));
	if(corr_map == NULL) {
		fprintf(stderr,"Error allocating memory for correlation map edge vector.\n");
		xe_exit(1);
	}
	struct point *corr_map_data_ptr; rmd_guid_t corr_map_data_dbg;
	corr_map_data_ptr = (struct point*)bsm_malloc(&corr_map_data_dbg,(image_params.Iy-Ncor+1)*(image_params.Ix-Ncor+1)*sizeof(struct point));
	if (corr_map_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for correlation map data.\n");
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy-Ncor+1; i++)
	{
		RAG_PUT_PTR(&corr_map[i], corr_map_data_ptr + i*(image_params.Ix-Ncor+1));
	}

#ifdef TRACE
xe_printf("// Allocate memory for detection list\n");RAG_FLUSH;
#endif
	rmd_guid_t Y_dbg;
	Y = (struct detects*)bsm_malloc(&Y_dbg,(image_params.Iy-Ncor-cfar_params.Ncfar+2)*(image_params.Ix-Ncor-cfar_params.Ncfar+2)*sizeof(struct detects));
	if(Y == NULL) {
		fprintf(stderr,"Error allocating memory for detection list.\n");
		xe_exit(1);
	}
////////////////////////////////////////////////////////////////////////////
//  DONE WITH INITIALIZATION OF PARAMETER DATA PUT, DATA INTO DATA BLOCKS //
////////////////////////////////////////////////////////////////////////////
	REM_STX_ADDR(in_ptr           , in           , struct Inputs);
#if RAG_DIG_SPOT_ON
	REM_STX_ADDR(dig_spot_ptr     , dig_spot     , struct DigSpotVars);
#endif

	REM_STX_ADDR(cfar_params_ptr  , cfar_params  , struct CfarParams);
	REM_STX_ADDR(radar_params_ptr , radar_params , struct RadarParams);
	REM_STX_ADDR(image_params_ptr , image_params , struct ImageParams);
	REM_STX_ADDR(affine_params_ptr, affine_params, struct AffineParams);
	REM_STX_ADDR(ts_params_ptr    , ts_params    , struct ThinSplineParams);

#ifdef TRACE
xe_printf("// Read first set of input data\n");RAG_FLUSH;
#endif
	ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);

#ifdef TRACE
xe_printf("// Form first image\n");RAG_FLUSH;
#endif
	FormImage(
#if RAG_DIG_SPOT_ON
		dig_spot_dbg,
#endif
		image_params_dbg, radar_params_dbg,
		in_dbg, curImage_dbg);

	while(--image_params.numImages) {
#ifdef TRACE
xe_printf("// Read second set of input data\n");RAG_FLUSH;
#endif
		ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);
		
#ifdef TRACE
xe_printf("// Copy curImage to refImage\n");RAG_FLUSH;
#endif
		for(int m=0; m<image_params.Iy; m++) {
			struct complexData *ref_m;
			ref_m = (struct complexData *)RAG_GET_PTR(refImage+m);
			struct complexData *cur_m;
			cur_m = (struct complexData *)RAG_GET_PTR(curImage+m);
// RAG refImage should be in DRAM
			BSMtoBSM(ref_m, cur_m, image_params.Ix*sizeof(struct complexData));
		}

#ifdef TRACE
xe_printf("// Form second image\n");RAG_FLUSH;
#endif
		FormImage(
#if RAG_DIG_SPOT_ON
			dig_spot_dbg,
#endif
			image_params_dbg, radar_params_dbg,
			in_dbg, curImage_dbg);
#if RAG_AFFINE_ON
#ifdef TRACE
xe_printf("// Affine registration\n");RAG_FLUSH;
#endif
		Affine(affine_params_dbg, image_params_dbg, curImage_dbg, refImage_dbg);
#endif

#if RAG_THIN_ON	// RAG WAS COMMENTED OUT
#ifdef TRACE
xe_printf("// Thin-spline registration\n");RAG_FLUSH;
#endif
		ThinSpline(&ts_params, &image_params, curImage, refImage);
#endif

#ifdef TRACE
xe_printf("// Coherent Change Detection (Ncor = %d)\n",Ncor);RAG_FLUSH;
#endif
		// Coherent Change Detection
	        CCD(Ncor, corr_map, &image_params, curImage, refImage);

#ifdef TRACE
xe_printf("// Constant False Alarm Rate\n");RAG_FLUSH;
#endif
		Nd = CFAR(&image_params, Ncor, &cfar_params, corr_map, Y);

#ifdef TRACE
xe_printf("// Output to file %d detects\n",Nd);RAG_FLUSH;
#endif
		for(int m=0; m<Nd; m++) {
#ifndef RAG_SIM
			fprintf(pOutFile, "x=%7.2fm y=%7.2fm p=%4.2f\n", Y[m].x, Y[m].y, Y[m].p);
#else
			struct detects Y_m;
			REM_LDX_ADDR(Y_m,&Y[m],struct detects);
			xe_printf("x=0x%x m y=0x%x m p=0x%x\n",
				 *(uint32_t *)&Y_m.x, *(uint32_t *)&Y_m.y, *(uint32_t *)&Y_m.p);
#endif
		}
	}
	
#ifdef DEBUG_SSCP
#ifdef TRACE
xe_printf("// Output Images to .bins\n");RAG_FLUSH;
#endif
    {
#ifndef RAG_SIM
        FILE *pOutImg = fopen("images_debug.bin", "wb");
        FILE *pOutCorr = fopen("corr_debug.bin", "wb");

        assert(pOutImg != NULL);
        assert(pOutCorr != NULL);
#endif

#ifndef RAG_SIM
        for(int m=0; m<image_params.Iy; m++)
            fwrite(&curImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);
        for(int m=0; m<image_params.Iy; m++)
            fwrite(&refImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);
        for(int m=0; m<image_params.Iy-Ncor+1; m++)
            fwrite(&corr_map[m][0], sizeof(struct point), image_params.Ix-Ncor+1, pOutCorr);
#else
        for(int m=0; m<image_params.Iy; m++) {
            struct complexData *cur_m;
            cur_m = (struct complexData *)RAG_GET_PTR(curImage+m);
            for(int n=0; n<image_params.Ix; n++) {
	        struct complexData cur_m_n;
	        REM_LDX_ADDR(cur_m_n,cur_m+n,struct complexData);
		xe_printf("cur 0x%x 0x%x\n",*(uint32_t *)&cur_m_n.real, *(uint32_t *)&cur_m_n.imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params.Iy; m++) {
            struct complexData *ref_m;
            ref_m = (struct complexData *)RAG_GET_PTR(refImage+m);
            for(int n=0; n<image_params.Ix; n++) {
	        struct complexData ref_m_n;
	        REM_LDX_ADDR(ref_m_n,ref_m+n,struct complexData);
		xe_printf("ref 0x%x 0x%x\n",*(uint32_t *)&ref_m_n.real, *(uint32_t *)&ref_m_n.imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params.Iy-Ncor+1; m++) {
            struct point *corr_m;
            corr_m = (struct point *)RAG_GET_PTR(corr_map+m);
            for(int n=0; n<image_params.Ix-Ncor+1; n++) {
	        struct point corr_m_n;
	        REM_LDX_ADDR(corr_m_n,corr_m+n,struct point);
		xe_printf("corr 0x%x 0x%x 0x%x\n",*(uint32_t *)&corr_m_n.x, *(uint32_t *)&corr_m_n.y,*(uint32_t *)&corr_m_n.p);
	    } // for n
	} // for m
#endif
        //for(int m=0; m<affine_params.Nc; m++)
        //    fwrite(&regmap[m], sizeof(struct reg_map), 1, pOutFile);

#ifndef RAG_SIM
        fclose(pOutImg);
        fclose(pOutCorr);
#endif
    }
#endif

#if !(defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST))
	fclose(pInFile);
	fclose(pInFile2);
	fclose(pInFile3);
	fclose(pOutFile);
#endif

	bsm_free(Y,Y_dbg);
	bsm_free(corr_map_data_ptr,corr_map_data_dbg); // corr_map[]
	bsm_free(corr_map,corr_map_dbg);
	bsm_free(refImage_data_ptr,refImage_data_dbg); //refImage[]
	bsm_free(refImage,refImage_dbg);
	bsm_free(curImage_data_ptr,curImage_data_dbg); // curImage[]
	bsm_free(curImage,curImage_dbg);
	bsm_free(in.Tp,in_Tp_dbg);
	bsm_free(in_Pt_data_ptr,in_Pt_data_dbg); // in.Pt[]
	bsm_free(in.Pt,in_Pt_dbg);
        bsm_free(in_X_data_ptr,in_X_data_dbg); // in.X[]
        bsm_free(in.X,in_X_dbg);

        bsm_free(image_params.yr,image_params_yr_dbg);
        bsm_free(image_params.xr,image_params_xr_dbg);

	bsm_free(ts_params_ptr    ,ts_params_dbg);
	bsm_free(affine_params_ptr,affine_params_dbg);
	bsm_free(image_params_ptr ,image_params_dbg);
	bsm_free(radar_params_ptr ,radar_params_dbg);
	bsm_free(cfar_params_ptr  ,cfar_params_dbg);

#if RAG_DIG_SPOT_ON
        bsm_free(dig_spot_ptr,dig_spot_dbg);
#endif
        bsm_free(in_ptr,in_dbg);

#if defined(RAG_AFL) || defined(RAG_SIM)
#ifdef TRACE
xe_printf("leave main_codelet\n");RAG_FLUSH;
#endif
	xe_exit(0);
#else
	return 0;
#endif
}
