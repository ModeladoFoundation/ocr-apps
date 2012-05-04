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

#include "rag_rmd.h"

// main codelet for rmd's runtime to start

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval;
#ifdef TRACE_LVL_1
if(n_db) {
xe_printf("// enter main_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld\n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);RAG_FLUSH;
} else {
xe_printf("// enter main_codelet arg %ld n_db %d\n",arg,n_db);RAG_FLUSH;
}
#endif
	FILE *pInFile, *pInFile2, *pInFile3;
	struct Inputs in;			// Inputs
	struct detects *Y;			// Detects list
	struct point **corr_map;		// Correlation map
#ifdef RAG_DIG_SPOT
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
#ifdef RAG_DIG_SPOT
	rmd_guid_t dig_spot_dbg;	// Digital spotlight variables
#endif

	rmd_guid_t cfar_params_dbg;	// CFAR parameters datablock guid
	rmd_guid_t radar_params_dbg;	// Radar parameters datablock guid
	rmd_guid_t image_params_dbg;	// Image parameters datablock guid
	rmd_guid_t affine_params_dbg;	// Affine registration parameters datablock guid
	rmd_guid_t ts_params_dbg;	// Thin spline registration parameters datablock guid

	struct Inputs *in_ptr;			// Inputs
#ifdef RAG_DIG_SPOT
	struct DigSpotVars *dig_spot_ptr;	// Digital spotlight variables
#endif

	struct CfarParams *cfar_params_ptr;	// CFAR parameters
	struct RadarParams *radar_params_ptr;	// Radar parameters
	struct ImageParams *image_params_ptr;	// Image parameters
	struct AffineParams *affine_params_ptr;	// Affine registration parameters
	struct ThinSplineParams *ts_params_ptr;	// Thin spline registration parameters

	in_ptr       =  bsm_malloc(&in_dbg     ,  sizeof(struct Inputs));
	if(in_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for in data block.\n");
		exit(1);
	}
#ifdef RAG_DIG_SPOT
	dig_spot_ptr = bsm_malloc(&dig_spot_dbg,  sizeof(struct DigSpotVars));
	if(dig_spot_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for dig_spot data block.\n");
		exit(1);
	}
#endif

	cfar_params_ptr   = bsm_malloc(&cfar_params_dbg,  sizeof(struct CfarParams));
	radar_params_ptr  = bsm_malloc(&radar_params_dbg, sizeof(struct RadarParams));
	image_params_ptr  = bsm_malloc(&image_params_dbg, sizeof(struct ImageParams));
	affine_params_ptr = bsm_malloc(&affine_params_dbg,sizeof(struct AffineParams));
	ts_params_ptr     = bsm_malloc(&ts_params_dbg,    sizeof(struct ThinSplineParams));

	if(cfar_params_ptr   == NULL
	|| radar_params_ptr  == NULL
	|| image_params_ptr  == NULL
	|| affine_params_ptr == NULL
	|| ts_params_ptr     == NULL ) {
		fprintf(stderr,"Error allocating memory for params blocks.\n");
		exit(1);
	}

#if defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST)
	pInFile  = NULL;
	pInFile2 = NULL;
	pInFile3 = NULL;
#else
#ifdef TRACE_LVL_1
xe_printf("// SAR data\n");RAG_FLUSH;
#endif
	if( (pInFile = fopen(argv[1], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[1]);
		exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Platform positions\n");RAG_FLUSH;
#endif
	if( (pInFile2 = fopen(argv[2], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[2]);
		exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Pulse transmission timestamps\n");RAG_FLUSH;
#endif
	if( (pInFile3 = fopen(argv[3], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[3]);
		exit(1);
	}
#endif

	switch(ReadParams(&radar_params, &image_params, &affine_params, &ts_params, &cfar_params)) {
	  case 0: break;
	  case 1:
		  fprintf(stderr,"Unable to open Parameters.txt\n");
		  exit(1);
	  case 2:
		  fprintf(stderr,"Parameters.txt does not adhere to expected format.\n");
		  exit(1);
	  default: 
		  fprintf(stderr,"Unexpected return value from ReadParams.\n");
		  exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Ensure all window sizes are odd\n");RAG_FLUSH;
#endif
	if( !(affine_params.Sc % 2) ) {
		fprintf(stderr,"Sc must be odd. Exiting.\n");
		exit(1);
	}

	if( !(ts_params.Scf % 2) ) {
		fprintf(stderr,"Scf must be odd. Exiting.\n");
		exit(1);
	}

	if( !(image_params.Ncor % 2) ) {
		fprintf(stderr,"Ncor must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		fprintf(stderr,"Ncfar must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		fprintf(stderr,"Nguard must be odd. Exiting.\n");
		exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Calculate dependent variables\n");RAG_FLUSH;
#endif
	image_params.TF = image_params.Ix/image_params.Sx;
#ifdef RAG_PURE_FLOAT
	image_params.dr = c_mks_mps/radar_params.fs/2.0f/((float)image_params.F);
#else
	image_params.dr = c_mks_mps/radar_params.fs/2/image_params.F;
#endif
	image_params.P2 = image_params.P1/image_params.TF;
	image_params.S2 = image_params.S1/image_params.TF;

#ifdef TRACE_LVL_1
xe_printf("// TF > 1 implies digital spotlighting, TF = 1 implies no digital spotlighting\n");RAG_FLUSH;
#endif
	if(image_params.TF > 1) {
#ifndef RAG_DIG_SPOT
		xe_printf("!!! DIGITAL SPOTLIGHTING NOT YET SUPPORTED !!!\n");RAG_FLUSH;
		exit(1);
#else	// RAG_DIG_SPOT
#ifdef TRACE_LVL_1
xe_printf("// TF > 1 implies digital spotlighting\n");RAG_FLUSH;
#endif
		image_params.P3 = image_params.P2;
		image_params.S3 = image_params.S2;
#ifdef TRACE_LVL_1
xe_printf("// Calculate new zeroth range bin\n");RAG_FLUSH;
#endif
		radar_params.R0_prime = radar_params.r0 - (radar_params.r0 - radar_params.R0)/image_params.TF;
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for variables needed to perform digital spotlighting\n");RAG_FLUSH;
#endif
		dig_spot.freqVec = (float*)malloc(image_params.S1*sizeof(float));
		if(dig_spot.freqVec == NULL) {
			fprintf(stderr,"Unable to allocate memory for freqVec.\n");
			exit(1);
		}

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
#ifdef TRACE_LVL_1
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
#endif // RAG_DIG_SPOT
	} else {
#ifdef TRACE_LVL_1
xe_printf("// TF = 1 implies no digital spotlighting\n");RAG_FLUSH;
#endif
		image_params.P3 = image_params.P1;
		image_params.S3 = image_params.S1;
	}

	image_params.S4 = (int)ceilf(image_params.F*image_params.S3);
#ifdef DEBUG
xe_printf("// check ceilf S4 %d F %d S3 %d\n",image_params.S4,image_params.F,image_params.S3);RAG_FLUSH;
#endif
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for axis vectors\n");RAG_FLUSH;
#endif
        rmd_guid_t image_params_xr_dbg;
        rmd_guid_t image_params_yr_dbg;
	image_params.xr = (float*)bsm_malloc(&image_params_xr_dbg,image_params.Ix*sizeof(float));
	image_params.yr = (float*)bsm_malloc(&image_params_yr_dbg,image_params.Iy*sizeof(float));
	if(image_params.xr == NULL || image_params.yr == NULL) {
		fprintf(stderr,"Error allocating memory for axis vectors.\n");
		exit(1);
	}
#ifdef TRACE_LVL_1
xe_printf("// Create axis vectors\n");RAG_FLUSH;
#endif
	for(int i=0; i<image_params.Ix; i++) {
		RAG_PUT_FLT(&image_params.xr[i], (i - floorf((float)image_params.Ix/2))*image_params.dr);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_FLT(&image_params.yr[i],(i - floorf((float)image_params.Iy/2))*image_params.dr);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for pulse compressed SAR data\n");RAG_FLUSH;
#endif
        rmd_guid_t in_X_dbg;
	in.X = (struct complexData**)bsm_malloc(&in_X_dbg,image_params.P1*sizeof(struct complexData*));
	if(in.X == NULL) {
		fprintf(stderr,"Error allocating memory for X edge vector.\n");
		exit(1);
	}
	in.X_edge_dbg = in_X_dbg;
        struct complexData* in_X_data_ptr = NULL; rmd_guid_t in_X_data_dbg;
#ifdef RAG_AFL
	in_X_data_ptr = (struct complexData*)dram_malloc(&in_X_data_dbg,image_params.P1*image_params.S1*sizeof(struct complexData));
#else
	in_X_data_ptr = (struct complexData*) bsm_malloc(&in_X_data_dbg,image_params.P1*image_params.S1*sizeof(struct complexData));
#endif
	if ( in_X_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for X data.\n");
		exit(1);
	}
	in.X_data_dbg = in_X_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		RAG_PUT_PTR(&in.X[i], in_X_data_ptr + i*image_params.S1);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for transmitter positions at each pulse\n");RAG_FLUSH;
#endif
	rmd_guid_t in_Pt_dbg;
	in.Pt = (float**)bsm_malloc(&in_Pt_dbg,image_params.P1*sizeof(float*));
	if(in.Pt == NULL) {
		fprintf(stderr,"Error allocating memory for Pt edge vector.\n");
		exit(1);
	}
	in.Pt_edge_dbg = in_Pt_dbg;
	float * in_Pt_data_ptr = NULL; rmd_guid_t in_Pt_data_dbg;
#ifdef RAG_AFL
	in_Pt_data_ptr = (float*)dram_malloc(&in_Pt_data_dbg,image_params.P1*3*sizeof(float));
#else
	in_Pt_data_ptr = (float*) bsm_malloc(&in_Pt_data_dbg,image_params.P1*3*sizeof(float));
#endif
	if( in_Pt_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for Pt data.\n");
		exit(1);
	}
	in.Pt_data_dbg = in_Pt_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		RAG_PUT_PTR(&in.Pt[i], in_Pt_data_ptr + i*3);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for timestamp of pulse transmissions\n");RAG_FLUSH;
#endif
	rmd_guid_t in_Tp_dbg;
	in.Tp = (float*)bsm_malloc(&in_Tp_dbg,image_params.P1*sizeof(float));
	if(in.Tp == NULL) {
		fprintf(stderr,"Error allocating memory for Tp.\n");
		exit(1);
	}
	in.Tp_dbg = in_Tp_dbg;
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for current image\n");RAG_FLUSH;
#endif
	rmd_guid_t curImage_dbg;
#ifdef RAG_AFL
	curImage = (struct complexData**) bsm_malloc(&curImage_dbg,image_params.Iy*sizeof(struct complexData*));
#else
	curImage = (struct complexData**) bsm_malloc(&curImage_dbg,image_params.Iy*sizeof(struct complexData*));
#endif
	if( curImage == NULL) {
		fprintf(stderr,"Error allocating memory for curImage edge vector.\n");
		exit(1);
	}
	struct complexData *curImage_data_ptr; rmd_guid_t curImage_data_dbg;
#ifdef RAG_AFL
	curImage_data_ptr = (struct complexData*)dram_malloc(&curImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#else
	curImage_data_ptr = (struct complexData*) bsm_malloc(&curImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#endif
	if (curImage_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for curImage data.\n");
		exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_PTR(&curImage[i], curImage_data_ptr + i*image_params.Ix);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for reference image\n");RAG_FLUSH;
#endif
	rmd_guid_t refImage_dbg;
#ifdef RAG_AFL
	refImage = (struct complexData**) bsm_malloc(&refImage_dbg,image_params.Iy*sizeof(struct complexData*));
#else
	refImage = (struct complexData**) bsm_malloc(&refImage_dbg,image_params.Iy*sizeof(struct complexData*));
#endif
	if(refImage == NULL) {
		fprintf(stderr,"Error allocating memory for refImage edge vector.\n");
		exit(1);
	}
	struct complexData *refImage_data_ptr; rmd_guid_t refImage_data_dbg;
#ifdef RAG_AFL
	refImage_data_ptr = (struct complexData*)dram_malloc(&refImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#else
	refImage_data_ptr = (struct complexData*) bsm_malloc(&refImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#endif
	if (refImage_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for refImage data.\n");
		exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		RAG_PUT_PTR(&refImage[i], refImage_data_ptr + i*image_params.Ix);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for correlation map\n");RAG_FLUSH;
#endif
	rmd_guid_t corr_map_dbg;
#ifdef RAG_AFL
	corr_map = (struct point**) bsm_malloc(&corr_map_dbg,(image_params.Iy-image_params.Ncor+1)*sizeof(struct point*));
#else
	corr_map = (struct point**) bsm_malloc(&corr_map_dbg,(image_params.Iy-image_params.Ncor+1)*sizeof(struct point*));
#endif
	if(corr_map == NULL) {
		fprintf(stderr,"Error allocating memory for correlation map edge vector.\n");
		exit(1);
	}
	struct point *corr_map_data_ptr; rmd_guid_t corr_map_data_dbg;
#ifdef RAG_AFL
	corr_map_data_ptr = (struct point*)dram_malloc(&corr_map_data_dbg,(image_params.Iy-image_params.Ncor+1)*(image_params.Ix-image_params.Ncor+1)*sizeof(struct point));
#else
	corr_map_data_ptr = (struct point*) bsm_malloc(&corr_map_data_dbg,(image_params.Iy-image_params.Ncor+1)*(image_params.Ix-image_params.Ncor+1)*sizeof(struct point));
#endif
	if (corr_map_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for correlation map data.\n");
		exit(1);
	}
	for(int i=0; i<image_params.Iy-image_params.Ncor+1; i++)
	{
		RAG_PUT_PTR(&corr_map[i], corr_map_data_ptr + i*(image_params.Ix-image_params.Ncor+1));
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for detection list\n");RAG_FLUSH;
#endif
	rmd_guid_t Y_dbg;
	Y = (struct detects*)bsm_malloc(&Y_dbg,(image_params.Iy-image_params.Ncor-cfar_params.Ncfar+2)*(image_params.Ix-image_params.Ncor-cfar_params.Ncfar+2)*sizeof(struct detects));
	if(Y == NULL) {
		fprintf(stderr,"Error allocating memory for detection list.\n");
		exit(1);
	}
////////////////////////////////////////////////////////////////////////////
//  DONE WITH INITIALIZATION OF PARAMETER DATA PUT, DATA INTO DATA BLOCKS //
////////////////////////////////////////////////////////////////////////////
	REM_STX_ADDR(in_ptr           , in           , struct Inputs);
#ifdef RAG_DIG_SPOT
	REM_STX_ADDR(dig_spot_ptr     , dig_spot     , struct DigSpotVars);
#endif

	REM_STX_ADDR(cfar_params_ptr  , cfar_params  , struct CfarParams);
	REM_STX_ADDR(radar_params_ptr , radar_params , struct RadarParams);
	REM_STX_ADDR(image_params_ptr , image_params , struct ImageParams);
	REM_STX_ADDR(affine_params_ptr, affine_params, struct AffineParams);
	REM_STX_ADDR(ts_params_ptr    , ts_params    , struct ThinSplineParams);

	struct file_args_t file_args_lcl, *file_args_ptr;
	rmd_guid_t file_args_dbg;
	file_args_lcl.pInFile  = pInFile;
	file_args_lcl.pInFile2 = pInFile2;
	file_args_lcl.pInFile3 = pInFile3;
	file_args_ptr = (struct file_args_t *)bsm_malloc(&file_args_dbg,sizeof(struct file_args_t));
	if(file_args_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for file_args.\n");
		exit(1);
	}
	REM_STX_ADDR(file_args_ptr,file_args_lcl,struct file_args_t);
//////////////////////////////////////////////////////////////////////////
//   Create all the first level codelets                                //
//////////////////////////////////////////////////////////////////////////

#ifdef TRACE_LVL_1
xe_printf("// create a codelet for main_body_codelet function\n");RAG_FLUSH;
#endif
	rmd_guid_t main_body_clg;
	extern rmd_guid_t main_body_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&main_body_clg,		// rmd_guid_t *new_guid
		main_body_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		11,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a codelet for post_main_codelet function\n");RAG_FLUSH;
#endif
	rmd_guid_t post_main_clg;
	extern rmd_guid_t post_main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&post_main_clg,		// rmd_guid_t *new_guid
		post_main_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		21,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);

#ifdef RAG_DIG_SPOT
#error
#endif

#ifdef TRACE_LVL_1
xe_printf("// create an instance for post_main_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t post_main_scg;
	retval = rmd_codelet_sched(
		&post_main_scg,		// rmd_guid_t* scheduled codelet's guid
		0,			// uint64_t arg
		post_main_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an instance for main_body_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t main_body_scg;
	retval = rmd_codelet_sched(
		&main_body_scg,		// rmd_guid_t* scheduled codelet's guid
		post_main_scg.data,	// uint64_t arg
		main_body_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// main_body_scg = %ld\n",main_body_scg.data);RAG_FLUSH;
#endif

#ifdef TRACE_LVL_1
xe_printf("// provide the arguments to main_body_codelet.\n");RAG_FLUSH;
#endif

RAG_DEF_MACRO_BSM( main_body_scg,struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_DEF_MACRO_BSM( main_body_scg,struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_DEF_MACRO_SPAD(main_body_scg,struct ImageParams *,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
RAG_DEF_MACRO_SPAD(main_body_scg,struct RadarParams *,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,3);
RAG_DEF_MACRO_SPAD(main_body_scg,struct AffineParams *,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,4);
RAG_DEF_MACRO_SPAD(main_body_scg,struct ThinSplineParams *,ts_params,ts_params_ptr,ts_params_lcl,ts_params_dbg,5);
RAG_DEF_MACRO_SPAD(main_body_scg,struct Cfar_params *,NULL,NULL,NULL,cfar_params_dbg,6);
RAG_DEF_MACRO_BSM( main_body_scg,struct point **,corr_map,NULL,NULL,corr_map_dbg,7);
RAG_DEF_MACRO_SPAD(main_body_scg,struct Inputs *,in,in_ptr,in_lcl,in_dbg,8);
RAG_DEF_MACRO_PASS(main_body_scg,struct detects *,NULL,NULL,NULL,Y_dbg,9);
RAG_DEF_MACRO_BSM( main_body_scg,struct file_args_t,NULL,NULL,NULL,file_args_dbg,10);

#ifdef TRACE_LVL_1
xe_printf("// provide the arguments to post_main_codelet.\n");RAG_FLUSH;
#endif

//RAG_DEF_MACRO_PASS(post_main_scg,struct detects *,NULL,NULL,NULL,Y_dbg,0);
//coming from main_body_codelet
RAG_DEF_MACRO_SPAD(post_main_scg,struct ImageParams *,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_DEF_MACRO_PASS(post_main_scg,float *,NULL,NULL,NULL,image_params_yr_dbg,2);
RAG_DEF_MACRO_PASS(post_main_scg,float *,NULL,NULL,NULL,image_params_xr_dbg,3);
RAG_DEF_MACRO_SPAD(post_main_scg,struct RadarParams *,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,4);
RAG_DEF_MACRO_SPAD(post_main_scg,struct AffineParams *,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,5);
RAG_DEF_MACRO_SPAD(post_main_scg,struct ThinSplineParams *,ts_params,ts_params_ptr,ts_params_lcl,ts_params_dbg,6);
RAG_DEF_MACRO_SPAD(post_main_scg,struct Cfar_params *,NULL,NULL,NULL,cfar_params_dbg,7);
RAG_DEF_MACRO_BSM( post_main_scg,struct complexData **,curImage,NULL,NULL,curImage_dbg,8);
RAG_DEF_MACRO_PASS(post_main_scg,struct complexData *, NULL,NULL,NULL,curImage_data_dbg,9);
RAG_DEF_MACRO_BSM( post_main_scg,struct complexData **,refImage,NULL,NULL,refImage_dbg,10);
RAG_DEF_MACRO_PASS(post_main_scg,struct complexData *, NULL,NULL,NULL,refImage_data_dbg,11);
RAG_DEF_MACRO_BSM( post_main_scg,struct point **,corr_map,NULL,NULL,corr_map_dbg,12);
RAG_DEF_MACRO_PASS(post_main_scg,struct point *, NULL,NULL,NULL,corr_map_data_dbg,13);
RAG_DEF_MACRO_SPAD(post_main_scg,struct Inputs *,in,in_ptr,in_lcl,in_dbg,14);
RAG_DEF_MACRO_PASS(post_main_scg,struct complexData **,NULL,NULL,NULL,in_X_dbg,15);
RAG_DEF_MACRO_PASS(post_main_scg,struct complex *,NULL,NULL,NULL,in_X_data_dbg,16);
RAG_DEF_MACRO_PASS(post_main_scg,struct point **,NULL,NULL,NULL,in_Pt_dbg,17);
RAG_DEF_MACRO_PASS(post_main_scg,struct point *,NULL,NULL,NULL,in_Pt_data_dbg,18);
RAG_DEF_MACRO_PASS(post_main_scg,struct float *,NULL,NULL,NULL,in_Tp_dbg,19);
RAG_DEF_MACRO_BSM( post_main_scg,struct file_args_t,NULL,NULL,NULL,file_args_dbg,20);

// release DB of post_main arguments
	RMD_DB_RELEASE(file_args_dbg);
	RMD_DB_RELEASE(image_params_dbg);
        RMD_DB_RELEASE(image_params_yr_dbg);
        RMD_DB_RELEASE(image_params_xr_dbg);
	RMD_DB_RELEASE(radar_params_dbg);
	RMD_DB_RELEASE(affine_params_dbg);
	RMD_DB_RELEASE(ts_params_dbg);
	RMD_DB_RELEASE(cfar_params_dbg);
	RMD_DB_RELEASE(curImage_dbg);
	RMD_DB_RELEASE(curImage_data_dbg);
	RMD_DB_RELEASE(refImage_dbg);
	RMD_DB_RELEASE(refImage_data_dbg);
	RMD_DB_RELEASE(corr_map_dbg);
	RMD_DB_RELEASE(corr_map_data_dbg);
        RMD_DB_RELEASE(in_dbg);
        RMD_DB_RELEASE(in_X_dbg);
        RMD_DB_RELEASE(in_X_data_dbg);
	RMD_DB_RELEASE(in_Pt_dbg);
	RMD_DB_RELEASE(in_Pt_data_dbg);
	RMD_DB_RELEASE(in_Tp_dbg);
	RMD_DB_RELEASE(Y_dbg);
#ifdef TRACE_LVL_1
xe_printf("// leave main_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
//////////////////////////////////////////////////////////////////////////
rmd_guid_t main_body_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval;
	assert(n_db == 11);
#ifdef TRACE_LVL_1
xe_printf("// enter main_body_codelet\n");RAG_FLUSH;
#endif

RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,3);
RAG_REF_MACRO_SPAD(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,4);
RAG_REF_MACRO_SPAD(struct ThinSplineParams,ts_params,ts_params_ptr,ts_params_lcl,ts_params_dbg,5);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_params_ptr,cfar_params_lcl,cfar_params_dbg,6);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,7);
RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,8);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,9);
RAG_REF_MACRO_SPAD(struct file_args_t,file_args,file_args_ptr,file_args_lcl,file_args_dbg,10);

	rmd_guid_t post_main_scg;
	post_main_scg.data = arg;
#ifdef TRACE_LVL_1
xe_printf("// create a codelet for ReadData_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t ReadData_clg;
	extern rmd_guid_t ReadData_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&ReadData_clg,		// rmd_guid_t *new_guid
		ReadData_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		3,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create a codelet for FormImage_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t FormImage_clg;
	extern rmd_guid_t FormImage_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&FormImage_clg,		// rmd_guid_t *new_guid
		FormImage_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
#ifdef RAG_DIG_SPOT
		6,			// int n_dep
#else
		5,			// int n_dep
#endif
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create a codelet for Affine_codelet function\n");RAG_FLUSH;
#endif
	rmd_guid_t Affine_clg;
	extern rmd_guid_t Affine_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&Affine_clg,		// rmd_guid_t *new_guid
		Affine_codelet,		// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		4,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);

#ifdef RAG_THIN
#error
#endif
#ifdef TRACE_LVL_1
xe_printf("// create a codelet for CCD_codelet function\n");RAG_FLUSH;
#endif
	rmd_guid_t CCD_clg;
	extern rmd_guid_t CCD_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&CCD_clg,		// rmd_guid_t *new_guid
		CCD_codelet,		// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		4,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create a codelet for CFAR_codelet function\n");RAG_FLUSH;
#endif
	rmd_guid_t CFAR_clg;
	extern rmd_guid_t CFAR_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);
	retval = rmd_codelet_create(
		&CFAR_clg,		// rmd_guid_t *new_guid
		CFAR_codelet,		// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		4,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for CFAR_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t CFAR_scg;
	retval = rmd_codelet_sched(
		&CFAR_scg,		// rmd_guid_t* scheduled codelet's guid
		post_main_scg.data,	// uint64_t arg
		CFAR_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an instance for curImage/refImage CDD_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t CCD_scg;
	retval = rmd_codelet_sched(
		&CCD_scg,		// rmd_guid_t* scheduled codelet's guid
		CFAR_scg.data,		// uint64_t arg
		CCD_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for curImage/refImage Affine_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t Affine_scg;
	retval = rmd_codelet_sched(
		&Affine_scg,		// rmd_guid_t* scheduled codelet's guid
		CCD_scg.data,		// uint64_t arg
		Affine_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for curImage FormImage_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t FormImage_scg;
	retval = rmd_codelet_sched(
		&FormImage_scg,		// rmd_guid_t* scheduled codelet's guid
		Affine_scg.data,	// uint64_t arg
		FormImage_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for curImage ReadData_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t ReadData_scg;
	retval = rmd_codelet_sched(
		&ReadData_scg,		// rmd_guid_t* scheduled codelet's guid
		FormImage_scg.data,	// uint64_t arg
		ReadData_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for refImage FormImage_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t refFormImage_scg;
	retval = rmd_codelet_sched(
		&refFormImage_scg,	// rmd_guid_t* scheduled codelet's guid
		ReadData_scg.data,	// uint64_t arg
		FormImage_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create an instance for refImage ReadData_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t refReadData_scg;
	retval = rmd_codelet_sched(
		&refReadData_scg,	// rmd_guid_t* scheduled codelet's guid
		refFormImage_scg.data,	// uint64_t arg
		ReadData_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// Read first set of input data\n");RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(refReadData_scg,NULL,NULL,NULL,NULL,in_dbg,0);
RAG_DEF_MACRO_SPAD(refReadData_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(refReadData_scg,NULL,NULL,NULL,NULL,file_args_dbg,2);
//	ReadData(in, image_params, pInFile, pInFile2, pInFile3);
#ifdef TRACE_LVL_1
xe_printf("// Form first image\n");RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,in_dbg,0);
// done by refReadInputs_codelet
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,NULL_GUID,4);
#ifdef RAG_DIG_SPOT
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif
#ifdef RAG_DIG_SPOT
//	FormImage(in_dbg,image_params_dbg,radar_params_dbg,
//		curImage_dbg,NULL_GUID,dig_spot_dgb);
#else
//	FormImage(in_dbg,image_params_dbg,radar_params_dbg,
//		curImage_dbg,NULL_GUID);
#endif
	assert(image_params->numImages == 2);
	for(int image=1;image<image_params->numImages;image++) {
#ifdef TRACE_LVL_1
xe_printf("// Read second set of input data\n");RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(ReadData_scg,struct Inputs *,in,in_ptr,in_lcl,in_dbg,0);
//comming from refFormImage
RAG_DEF_MACRO_SPAD(ReadData_scg,struct ImageParams *,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(ReadData_scg,struct file_args_t,file_args,file_arg_ptr,file_args_lcl,file_args_dbg,2);
//		ReadData(in, image_params, pInFile, pInFile2, pInFile3);
		
#ifdef TRACE_LVL_1
xe_printf("// Form second image (image_id=%d)\n",image);RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,in_dbg,0);
// done by ReadInputs_codelet
RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_DEF_MACRO_SPAD(FormImage_scg,NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif
#ifdef RAG_DIG_SPOT
//		FormImage(in_dbg,image_params_dbg,radar_params_dbg,
//			curImage_dbg,refImage_dbg,dig_spot_dbg);
#else
//		FormImage(in_dbg,image_params_dbg,radar_params_dbg,
//			curImage_dbg,refImage_dbg);
#endif

#ifdef RAG_AFFINE
#ifdef TRACE_LVL_1
xe_printf("// Affine registration\n");RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(Affine_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);
// done by FormInputs_codelet
RAG_DEF_MACRO_SPAD(Affine_scg,NULL,NULL,NULL,NULL,refImage_dbg,1);
RAG_DEF_MACRO_SPAD(Affine_scg,NULL,NULL,NULL,NULL,affine_params_dbg,2);
RAG_DEF_MACRO_SPAD(Affine_scg,NULL,NULL,NULL,NULL,image_params_dbg,3);
//		Affine(curImage_dbg,refImage_dbg,affine_params_dbg, image_params_dbg);
#endif

#ifdef RAG_THIN	// RAG WAS COMMENTED OUT
#ifdef TRACE_LVL_1
xe_printf("// Thin-spline registration\n");RAG_FLUSH;
#endif
		ThinSpline(ts_params, image_params, curImage, refImage);
#endif

#ifdef TRACE_LVL_1
xe_printf("// Coherent Change Detection (Ncor = %d)\n",image_params->Ncor);RAG_FLUSH;
#endif
		// Coherent Change Detection
//RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);
// done by Affine_codelet
//RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,refImage_dbg,1);
// done by Affine_codelet
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,corr_map_dbg,2);
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,image_params_dbg,3);
//	        CCD(curImage, refImage, corr_map, image_params);

#ifdef TRACE_LVL_1
xe_printf("// Constant False Alarm Rate\n");RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,corr_map_dbg,0);
// done by CCD_codelet
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,cfar_params_dbg,2);
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,Y_dbg,3);
//		CFAR(corr_map, image_params, cfar_params, Y);

	} // while
#ifdef TRACE_LVL_1
xe_printf("// leave main_body_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
//////////////////////////////////////////////////////////////////////////
rmd_guid_t post_main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval;
	assert(n_db == 21);
	FILE *pInFile, *pInFile2, *pInFile3;
#ifdef TRACE_LVL_1
xe_printf("// enter post_main_codelet\n");RAG_FLUSH;
#endif
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_BSM( float *,image_params_yr_ptr,NULL,NULL,image_params_yr_dbg,2);
RAG_REF_MACRO_BSM( float *,image_params_xr_ptr,NULL,NULL,image_params_xr_dbg,3);
RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,4);
RAG_REF_MACRO_SPAD(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,5);
RAG_REF_MACRO_SPAD(struct ThinSplineParams,ts_params,ts_params_ptr,ts_params_lcl,ts_params_dbg,6);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_params_ptr,cfar_params_lcl,cfar_params_dbg,7);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,8);
RAG_REF_MACRO_BSM( struct complexData *, curImage_data_ptr,NULL,NULL,curImage_data_dbg,9);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,10);
RAG_REF_MACRO_BSM( struct complexData *,refImage_data_ptr,NULL,NULL,refImage_data_dbg,11);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,12);
RAG_REF_MACRO_BSM( struct point *, corr_map_data_ptr,NULL,NULL,corr_map_data_dbg,13);
RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,14);
RAG_REF_MACRO_BSM( struct complexData **,in_X_ptr,NULL,NULL,in_X_dbg,15);
RAG_REF_MACRO_BSM( struct complexData *,in_X_data_ptr,NULL,NULL,in_X_data_dbg,16);
RAG_REF_MACRO_BSM( struct point **,in_Pt_ptr,NULL,NULL,in_Pt_dbg,17);
RAG_REF_MACRO_BSM( struct point *,in_Pt_data_ptr,NULL,NULL,in_Pt_data_dbg,18);
RAG_REF_MACRO_BSM( float *,in_Tp_ptr,NULL,NULL,in_Tp_dbg,19);
RAG_REF_MACRO_SPAD(struct file_args_t,file_args,file_args_ptr,file_args_lcl,file_args_dbg,20);

	pInFile  = file_args_lcl.pInFile;
	pInFile2 = file_args_lcl.pInFile2;
	pInFile3 = file_args_lcl.pInFile3;

#ifdef DEBUG_SSCP
#ifdef TRACE_LVL_1
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
        for(int m=0; m<image_params->Iy; m++)
            fwrite(&curImage[m][0], sizeof(struct complexData), image_params->Ix, pOutImg);
        for(int m=0; m<image_params->Iy; m++)
            fwrite(&refImage[m][0], sizeof(struct complexData), image_params->Ix, pOutImg);
        for(int m=0; m<image_params->Iy-image_params->Ncor+1; m++)
            fwrite(&corr_map[m][0], sizeof(struct point), image_params->Ix-image_params->Ncor+1, pOutCorr);
#else
        for(int m=0; m<image_params->Iy; m++) {
            struct complexData *cur_m;
            cur_m = (struct complexData *)RAG_GET_PTR(curImage+m);
            for(int n=0; n<image_params->Ix; n++) {
	        struct complexData cur_m_n;
	        REM_LDX_ADDR(cur_m_n,cur_m+n,struct complexData);
		xe_printf("cur 0x%x 0x%x\n",*(uint32_t *)&cur_m_n.real, *(uint32_t *)&cur_m_n.imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params->Iy; m++) {
            struct complexData *ref_m;
            ref_m = (struct complexData *)RAG_GET_PTR(refImage+m);
            for(int n=0; n<image_params->Ix; n++) {
	        struct complexData ref_m_n;
	        REM_LDX_ADDR(ref_m_n,ref_m+n,struct complexData);
		xe_printf("ref 0x%x 0x%x\n",*(uint32_t *)&ref_m_n.real, *(uint32_t *)&ref_m_n.imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params->Iy-image_params->Ncor+1; m++) {
            struct point *corr_m;
            corr_m = (struct point *)RAG_GET_PTR(corr_map+m);
            for(int n=0; n<image_params->Ix-image_params->Ncor+1; n++) {
	        struct point corr_m_n;
	        REM_LDX_ADDR(corr_m_n,corr_m+n,struct point);
		xe_printf("corr 0x%x 0x%x 0x%x\n",*(uint32_t *)&corr_m_n.x, *(uint32_t *)&corr_m_n.y,*(uint32_t *)&corr_m_n.p);
	    } // for n
	} // for m
#endif

#ifndef RAG_SIM
        fclose(pOutImg);
        fclose(pOutCorr);
#endif
    }
#endif // DEBUG_SSCP

#if !(defined(RAG_IMPLICIT_INPUTS) && !defined(RAG_IMPLICIT_INPUTS_TEST))
	fclose(pInFile);
	fclose(pInFile2);
	fclose(pInFile3);
#endif

	bsm_free(Y,Y_dbg);
#ifdef RAG_AFL
	dram_free(corr_map_data_ptr,corr_map_data_dbg); // corr_map[]
#else
	 bsm_free(corr_map_data_ptr,corr_map_data_dbg); // corr_map[]
#endif
	bsm_free(corr_map,corr_map_dbg);
#ifdef RAG_AFL
	dram_free(refImage_data_ptr,refImage_data_dbg); //refImage[]
#else
	 bsm_free(refImage_data_ptr,refImage_data_dbg); //refImage[]
#endif
	bsm_free(refImage,refImage_dbg);
#ifdef RAG_AFL
	dram_free(curImage_data_ptr,curImage_data_dbg); // curImage[]
#else
	 bsm_free(curImage_data_ptr,curImage_data_dbg); // curImage[]
#endif
	bsm_free(curImage,curImage_dbg);
	bsm_free(in_Tp_ptr,in_Tp_dbg);
#ifdef RAG_AFL
	dram_free(in_Pt_data_ptr,in_Pt_data_dbg); // in.Pt[]
#else
	 bsm_free(in_Pt_data_ptr,in_Pt_data_dbg); // in.Pt[]
#endif
	bsm_free(in_Pt_ptr,in_Pt_dbg);
#ifdef RAG_AFL
        dram_free(in_X_data_ptr,in_X_data_dbg); // in.X[]
#else
         bsm_free(in_X_data_ptr,in_X_data_dbg); // in.X[]
#endif
        bsm_free(in_X_ptr,in_X_dbg);
        bsm_free(in,in_dbg);

        bsm_free(image_params_yr_ptr,image_params_yr_dbg);
        bsm_free(image_params_xr_ptr,image_params_xr_dbg);

	bsm_free(ts_params_ptr    ,ts_params_dbg);
	bsm_free(affine_params_ptr,affine_params_dbg);
	bsm_free(image_params_ptr ,image_params_dbg);
	bsm_free(radar_params_ptr ,radar_params_dbg);
	bsm_free(cfar_params_ptr  ,cfar_params_dbg);

	bsm_free(file_args_ptr,file_args_dbg);

#ifdef RAG_DIG_SPOT
        bsm_free(dig_spot_ptr,dig_spot_dbg);
#endif

#ifdef TRACE_LVL_1
xe_printf("// leave post_main_codelet\n");RAG_FLUSH;
#endif
	xe_exit(0);
	return NULL_GUID;
}
