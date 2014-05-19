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

#ifndef RAG_SIM
#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"
#else
#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"
#include "rag_ocr.h"
#include "common.h"
#endif

#if defined(RAG_DIG_SPOT) || defined(RAG_THIN)
#error
#endif

// main Edt for ocr's runtime to start

ocrGuid_t mainEdt(
	uint32_t paramc, uint64_t *paramv,
	uint32_t depc, ocrEdtDep_t *depv)
{
	int retval;
#ifdef TRACE
xe_printf("enter mainEdt paramc %d depc %d\n",paramc,depc);RAG_FLUSH;
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

	ocrGuid_t in_dbg;      			// Inputs
#ifdef RAG_DIG_SPOT
	ocrGuid_t dig_spot_dbg;			// Digital spotlight variables
#endif

	ocrGuid_t cfar_params_dbg;		// CFAR parameters datablock guid
	ocrGuid_t radar_params_dbg;		// Radar parameters datablock guid
	ocrGuid_t image_params_dbg;		// Image parameters datablock guid
	ocrGuid_t affine_params_dbg;		// Affine registration parameters datablock guid
	ocrGuid_t ts_params_dbg;		// Thin spline registration parameters datablock guid

	struct Inputs *in_ptr;			// Inputs
#ifdef RAG_DIG_SPOT
	struct DigSpotVars *dig_spot_ptr;	// Digital spotlight variables
#endif

	struct CfarParams *cfar_params_ptr;	// CFAR parameters
	struct RadarParams *radar_params_ptr;	// Radar parameters
	struct ImageParams *image_params_ptr;	// Image parameters
	struct AffineParams *affine_params_ptr;	// Affine registration parameters
	struct ThinSplineParams *ts_params_ptr;	// Thin spline registration parameters

#ifdef TRACE
xe_printf("allocate Inputs\n");RAG_FLUSH;
#endif
	in_ptr       =  bsm_malloc(&in_dbg     ,  sizeof(struct Inputs));
	if(in_ptr == NULL) {
		xe_printf("Error allocating memory for in data block.\n");RAG_FLUSH;
		xe_exit(1);
	}
#ifdef RAG_DIG_SPOT
#ifdef TRACE
xe_printf("allocate DigSpotVars\n");RAG_FLUSH;
#endif
	dig_spot_ptr = bsm_malloc(&dig_spot_dbg,  sizeof(struct DigSpotVars));
	if(dig_spot_ptr == NULL) {
		xe_printf("Error allocating memory for dig_spot data block.\n");RAG_FLUSH;
		xe_exit(1);
	}
#endif

#ifdef TRACE
xe_printf("allocate CfarParams\n");RAG_FLUSH;
#endif
	cfar_params_ptr   = bsm_malloc(&cfar_params_dbg,  sizeof(struct CfarParams));
#ifdef TRACE
xe_printf("allocate RadarParams\n");RAG_FLUSH;
#endif
	radar_params_ptr  = bsm_malloc(&radar_params_dbg, sizeof(struct RadarParams));
#ifdef TRACE
xe_printf("allocate ImageParams\n");RAG_FLUSH;
#endif
	image_params_ptr  = bsm_malloc(&image_params_dbg, sizeof(struct ImageParams));
#ifdef TRACE
xe_printf("allocate AffineParams\n");RAG_FLUSH;
#endif
	affine_params_ptr = bsm_malloc(&affine_params_dbg,sizeof(struct AffineParams));
#ifdef TRACE
xe_printf("allocate ThinSplineParamsr\n");RAG_FLUSH;
#endif
	ts_params_ptr     = bsm_malloc(&ts_params_dbg,    sizeof(struct ThinSplineParams));

	if(cfar_params_ptr   == NULL
	|| radar_params_ptr  == NULL
	|| image_params_ptr  == NULL
	|| affine_params_ptr == NULL
	|| ts_params_ptr     == NULL ) {
		xe_printf("Error allocating memory for params blocks.\n");RAG_FLUSH;
		xe_exit(1);
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
		xe_exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Platform positions\n");RAG_FLUSH;
#endif
	if( (pInFile2 = fopen(argv[2], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[2]);
		xe_exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Pulse transmission timestamps\n");RAG_FLUSH;
#endif
	if( (pInFile3 = fopen(argv[3], "rb")) == NULL ) {
		xe_printf("Error opening %s\n", argv[3]);
		xe_exit(1);
	}
#endif

	switch(ReadParams(&radar_params, &image_params, &affine_params, &ts_params, &cfar_params)) {
	  case 0: break;
	  case 1:
		  xe_printf("Unable to open Parameters.txt\n");RAG_FLUSH;
		  xe_exit(1);
	  case 2:
		  xe_printf("Parameters.txt does not adhere to expected format.\n");RAG_FLUSH;
		  xe_exit(1);
	  default:
		  xe_printf("Unexpected return value from ReadParams.\n");RAG_FLUSH;
		  xe_exit(1);
	}

#ifdef TRACE_LVL_1
xe_printf("// Ensure all window sizes are odd\n");RAG_FLUSH;
#endif
	if( !(affine_params.Sc % 2) ) {
		xe_printf("Sc must be odd. Exiting.\n");RAG_FLUSH;
		xe_exit(1);
	}

	if( !(ts_params.Scf % 2) ) {
		xe_printf("Scf must be odd. Exiting.\n");RAG_FLUSH;
		xe_exit(1);
	}

	if( !(image_params.Ncor % 2) ) {
		xe_printf("Ncor must be odd. Exiting.\n");RAG_FLUSH;
		xe_exit(1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		xe_printf("Ncfar must be odd. Exiting.\n");RAG_FLUSH;
		xe_exit(1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		xe_printf("Nguard must be odd. Exiting.\n");RAG_FLUSH;
		xe_exit(1);
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
		xe_exit(1);
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
			xe_printf("Unable to allocate memory for freqVec.\n");RAG_FLUSH;
			xe_exit(1);
		}

		dig_spot.filtOut = (struct complexData*)malloc(image_params.P2*sizeof(struct complexData));
		if(dig_spot.filtOut == NULL) {
			xe_printf("Unable to allocate memory for filtOut.\n");RAG_FLUSH;
			xe_exit(1);
		}

		dig_spot.X2 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X2 == NULL) {
			xe_printf("Error allocating memory for X2.\n");RAG_FLUSH;
			xe_exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X2[n] = (struct complexData*)malloc(image_params.S1*sizeof(struct complexData));
			if (dig_spot.X2[n] == NULL) {
				xe_printf("Error allocating memory for X2.\n");RAG_FLUSH;
				xe_exit(1);
			}
		}

		dig_spot.X3 = (struct complexData**)malloc(image_params.P1*sizeof(struct complexData*));
		if(dig_spot.X3 == NULL) {
			xe_printf("Error allocating memory for X3.\n");RAG_FLUSH;
			xe_exit(1);
		}
		for(int n=0; n<image_params.P1; n++) {
			dig_spot.X3[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X3[n] == NULL) {
				xe_printf("Error allocating memory for X3.\n");RAG_FLUSH;
				xe_exit(1);
			}
		}

		dig_spot.X4 = (struct complexData**)malloc(image_params.P2*sizeof(struct complexData*));
		if(dig_spot.X4 == NULL) {
			xe_printf("Error allocating memory for X4.\n");RAG_FLUSH;
			xe_exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.X4[n] = (struct complexData*)malloc(image_params.S2*sizeof(struct complexData));
			if (dig_spot.X4[n] == NULL) {
				xe_printf("Error allocating memory for X4.\n");RAG_FLUSH;
				xe_exit(1);
			}
		}

		dig_spot.tmpVector = (struct complexData*)malloc(image_params.P1*sizeof(struct complexData));
		if(dig_spot.tmpVector == NULL) {
			xe_printf("Unable to allocate memory for tmpVector.\n");RAG_FLUSH;
			xe_exit(1);
		}

		dig_spot.Pt2 = (float**)malloc(image_params.P2*sizeof(float*));
		if(dig_spot.Pt2 == NULL) {
			xe_printf("Error allocating memory for Pt2.\n");RAG_FLUSH;
			xe_exit(1);
		}
		for(int n=0; n<image_params.P2; n++) {
			dig_spot.Pt2[n] = (float*)malloc(3*sizeof(float));
			if (dig_spot.Pt2[n] == NULL) {
				xe_printf("Error allocating memory for Pt2.\n");RAG_FLUSH;
				xe_exit(1);
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
        ocrGuid_t image_params_xr_dbg;
        ocrGuid_t image_params_yr_dbg;
	image_params.xr = (float*)bsm_malloc(&image_params_xr_dbg,image_params.Ix*sizeof(float));
	image_params.yr = (float*)bsm_malloc(&image_params_yr_dbg,image_params.Iy*sizeof(float));
	if(image_params.xr == NULL || image_params.yr == NULL) {
		xe_printf("Error allocating memory for axis vectors.\n");RAG_FLUSH;
		xe_exit(1);
	}
#ifdef TRACE_LVL_1
xe_printf("// Create axis vectors\n");RAG_FLUSH;
#endif
	for(int i=0; i<image_params.Ix; i++) {
		image_params.xr[i] = (i - floorf((float)image_params.Ix/2))*image_params.dr;
	}
	for(int i=0; i<image_params.Iy; i++) {
		image_params.yr[i] = (i - floorf((float)image_params.Iy/2))*image_params.dr;
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for pulse compressed SAR data\n");RAG_FLUSH;
#endif
        ocrGuid_t in_X_dbg;
	in.X = (struct complexData**)bsm_malloc(&in_X_dbg,image_params.P1*sizeof(struct complexData*));
	if(in.X == NULL) {
		xe_printf("Error allocating memory for X edge vector.\n");RAG_FLUSH;
		xe_exit(1);
	}
	in.X_edge_dbg = in_X_dbg;
        struct complexData* in_X_data_ptr = NULL; ocrGuid_t in_X_data_dbg;
#ifdef RAG_DRAM
	in_X_data_ptr = (struct complexData*)dram_malloc(&in_X_data_dbg,image_params.P1*image_params.S1*sizeof(struct complexData));
#else
	in_X_data_ptr = (struct complexData*) bsm_malloc(&in_X_data_dbg,image_params.P1*image_params.S1*sizeof(struct complexData));
#endif
	if ( in_X_data_ptr == NULL) {
		xe_printf("Error allocating memory for X data.\n");RAG_FLUSH;
		xe_exit(1);
	}
	in.X_data_dbg = in_X_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		in.X[i] = in_X_data_ptr + i*image_params.S1;
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for transmitter positions at each pulse\n");RAG_FLUSH;
#endif
	ocrGuid_t in_Pt_dbg;
	in.Pt = (float**)bsm_malloc(&in_Pt_dbg,image_params.P1*sizeof(float*));
	if(in.Pt == NULL) {
		xe_printf("Error allocating memory for Pt edge vector.\n");RAG_FLUSH;
		xe_exit(1);
	}
	in.Pt_edge_dbg = in_Pt_dbg;
	float * in_Pt_data_ptr = NULL; ocrGuid_t in_Pt_data_dbg;
#ifdef RAG_DRAM
	in_Pt_data_ptr = (float*)dram_malloc(&in_Pt_data_dbg,image_params.P1*3*sizeof(float));
#else
	in_Pt_data_ptr = (float*) bsm_malloc(&in_Pt_data_dbg,image_params.P1*3*sizeof(float));
#endif
	if( in_Pt_data_ptr == NULL) {
		xe_printf("Error allocating memory for Pt data.\n");RAG_FLUSH;
		xe_exit(1);
	}
	in.Pt_data_dbg = in_Pt_data_dbg;
	for(int i=0; i<image_params.P1; i++) {
		in.Pt[i] = in_Pt_data_ptr + i*3;
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for timestamp of pulse transmissions\n");RAG_FLUSH;
#endif
	ocrGuid_t in_Tp_dbg;
	in.Tp = (float*)bsm_malloc(&in_Tp_dbg,image_params.P1*sizeof(float));
	if(in.Tp == NULL) {
		xe_printf("Error allocating memory for Tp.\n");RAG_FLUSH;
		xe_exit(1);
	}
	in.Tp_dbg = in_Tp_dbg;
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for current image\n");RAG_FLUSH;
#endif
	ocrGuid_t curImage_dbg;
	curImage = (struct complexData**) bsm_malloc(&curImage_dbg,image_params.Iy*sizeof(struct complexData*));
	if( curImage == NULL) {
		xe_printf("Error allocating memory for curImage edge vector.\n");RAG_FLUSH;
		xe_exit(1);
	}
	struct complexData *curImage_data_ptr; ocrGuid_t curImage_data_dbg;
#ifdef RAG_DRAM
	curImage_data_ptr = (struct complexData*)dram_malloc(&curImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#else
	curImage_data_ptr = (struct complexData*) bsm_malloc(&curImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#endif
	if (curImage_data_ptr == NULL) {
		xe_printf("Error allocating memory for curImage data.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		curImage[i] = curImage_data_ptr + i*image_params.Ix;
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for reference image\n");RAG_FLUSH;
#endif
	ocrGuid_t refImage_dbg;
	refImage = (struct complexData**) bsm_malloc(&refImage_dbg,image_params.Iy*sizeof(struct complexData*));
	if(refImage == NULL) {
		xe_printf("Error allocating memory for refImage edge vector.\n");RAG_FLUSH;
		xe_exit(1);
	}
	struct complexData *refImage_data_ptr; ocrGuid_t refImage_data_dbg;
#ifdef RAG_DRAM
	refImage_data_ptr = (struct complexData*)dram_malloc(&refImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#else
	refImage_data_ptr = (struct complexData*) bsm_malloc(&refImage_data_dbg,image_params.Iy*image_params.Ix*sizeof(struct complexData));
#endif
	if (refImage_data_ptr == NULL) {
		xe_printf("Error allocating memory for refImage data.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy; i++) {
		refImage[i] = refImage_data_ptr + i*image_params.Ix;
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for correlation map\n");RAG_FLUSH;
#endif
	ocrGuid_t corr_map_dbg;
	corr_map = (struct point**) bsm_malloc(&corr_map_dbg,(image_params.Iy-image_params.Ncor+1)*sizeof(struct point*));
	if(corr_map == NULL) {
		xe_printf("Error allocating memory for correlation map edge vector.\n");RAG_FLUSH;
		xe_exit(1);
	}
	struct point *corr_map_data_ptr; ocrGuid_t corr_map_data_dbg;
#ifdef RAG_DRAM
	corr_map_data_ptr = (struct point*)dram_malloc(&corr_map_data_dbg,(image_params.Iy-image_params.Ncor+1)*(image_params.Ix-image_params.Ncor+1)*sizeof(struct point));
#else
	corr_map_data_ptr = (struct point*) bsm_malloc(&corr_map_data_dbg,(image_params.Iy-image_params.Ncor+1)*(image_params.Ix-image_params.Ncor+1)*sizeof(struct point));
#endif
	if (corr_map_data_ptr == NULL) {
		xe_printf("Error allocating memory for correlation map data.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int i=0; i<image_params.Iy-image_params.Ncor+1; i++)
	{
		corr_map[i] = corr_map_data_ptr + i*(image_params.Ix-image_params.Ncor+1);
	}
#ifdef TRACE_LVL_1
xe_printf("// Allocate memory for detection list\n");RAG_FLUSH;
#endif
	ocrGuid_t Y_dbg;
	Y = (struct detects*)bsm_malloc(&Y_dbg,(image_params.Iy-image_params.Ncor-cfar_params.Ncfar+2)*(image_params.Ix-image_params.Ncor-cfar_params.Ncfar+2)*sizeof(struct detects));
	if(Y == NULL) {
		xe_printf("Error allocating memory for detection list.\n");RAG_FLUSH;
		xe_exit(1);
	}
////////////////////////////////////////////////////////////////////////////
//  DONE WITH INITIALIZATION OF PARAMETER DATA, PUT DATA INTO DATA BLOCKS //
////////////////////////////////////////////////////////////////////////////
	SPADtoBSM(in_ptr           , &in           , sizeof(struct Inputs));
#ifdef RAG_DIG_SPOT
	SPADtoBSM(dig_spot_ptr     , &dig_spot     , sizeof(struct DigSpotVars));
#endif

	SPADtoBSM(cfar_params_ptr  , &cfar_params  , sizeof(struct CfarParams));
	SPADtoBSM(radar_params_ptr , &radar_params , sizeof(struct RadarParams));
	SPADtoBSM(image_params_ptr , &image_params , sizeof(struct ImageParams));
	SPADtoBSM(affine_params_ptr, &affine_params, sizeof(struct AffineParams));
	SPADtoBSM(ts_params_ptr    , &ts_params    , sizeof(struct ThinSplineParams));

	struct file_args_t file_args_lcl, *file_args_ptr;
	ocrGuid_t file_args_dbg;
	file_args_lcl.pInFile  = pInFile;
	file_args_lcl.pInFile2 = pInFile2;
	file_args_lcl.pInFile3 = pInFile3;
	file_args_ptr = (struct file_args_t *)bsm_malloc(&file_args_dbg,sizeof(struct file_args_t));
	if(file_args_ptr == NULL) {
		xe_printf("Error allocating memory for file_args.\n");RAG_FLUSH;
		xe_exit(1);
	}
	SPADtoBSM(file_args_ptr    , &file_args_lcl, sizeof(struct file_args_t));

//////////////////////////////////////////////////////////////////////////
//   Create all the first level edts                                    //
//////////////////////////////////////////////////////////////////////////
#ifdef TRACE_LVL_1
xe_printf("// create a template for post_main_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_main_clg;
	extern ocrGuid_t post_main_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&post_main_clg,		// ocrGuid_t *new_guid
			 post_main_edt,		// ocr_edt_ptr func_ptr
			0,			// paramc
			22);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for post_main_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t post_main_scg;
	retval = ocrEdtCreate(
			&post_main_scg,		// *created_edt_guid
			 post_main_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// provide the arguments to post_main_edt.\n");RAG_FLUSH;
#endif

RAG_DEF_MACRO_PASS(post_main_scg,struct detects *,NULL,NULL,NULL,Y_dbg,0);
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

#ifdef TRACE_LVL_1
xe_printf("// create a template for main_body_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t main_body_clg;
	extern ocrGuid_t main_body_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&main_body_clg,		// ocrGuid_t *new_guid
			 main_body_edt,		// ocr_edt_ptr func_ptr
			1,			// paramc
			11);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for main_body_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t main_body_scg, main_body_evg;
{	uint64_t paramv[1] = { GUID_VALUE(post_main_scg) };
	retval = ocrEdtCreate(
			&main_body_scg,		// *created_edt_guid
			 main_body_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&main_body_evg);	// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// main_body_scg = %ld\n",main_body_scg);RAG_FLUSH;
#endif

#ifdef TRACE_LVL_1
xe_printf("// provide the arguments to main_body_edt.\n");RAG_FLUSH;
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

RAG_DEF_MACRO_BSM( post_main_scg,NULL,NULL,NULL,NULL,main_body_evg,21);

// release DB of post_main arguments
	OCR_DB_RELEASE(file_args_dbg);
	OCR_DB_RELEASE(image_params_dbg);
        OCR_DB_RELEASE(image_params_yr_dbg);
        OCR_DB_RELEASE(image_params_xr_dbg);
	OCR_DB_RELEASE(radar_params_dbg);
	OCR_DB_RELEASE(affine_params_dbg);
	OCR_DB_RELEASE(ts_params_dbg);
	OCR_DB_RELEASE(cfar_params_dbg);
	OCR_DB_RELEASE(curImage_dbg);
	OCR_DB_RELEASE(curImage_data_dbg);
	OCR_DB_RELEASE(refImage_dbg);
	OCR_DB_RELEASE(refImage_data_dbg);
	OCR_DB_RELEASE(corr_map_dbg);
	OCR_DB_RELEASE(corr_map_data_dbg);
        OCR_DB_RELEASE(in_dbg);
        OCR_DB_RELEASE(in_X_dbg);
        OCR_DB_RELEASE(in_X_data_dbg);
	OCR_DB_RELEASE(in_Pt_dbg);
	OCR_DB_RELEASE(in_Pt_data_dbg);
	OCR_DB_RELEASE(in_Tp_dbg);
	OCR_DB_RELEASE(Y_dbg);
#ifdef TRACE_LVL_1
xe_printf("// leave mainEdt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
//////////////////////////////////////////////////////////////////////////
ocrGuid_t main_body_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv)
{
	int retval;
#ifdef TRACE_LVL_1
xe_printf("// enter main_body_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	ocrGuid_t post_main_scg = (ocrGuid_t)paramv[0];

	assert(depc==11);
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

#ifdef TRACE_LVL_1
xe_printf("// create a template for ReadData_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t ReadData_clg;
	extern ocrGuid_t ReadData_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&ReadData_clg,		// ocrGuid_t *new_guid
			 ReadData_edt,		// ocr_edt_ptr func_ptr
			1,			// paramc
			3);			// depc
	assert(retval==0);
#ifdef TRACE_LVL_1
xe_printf("// create a template for FormImage_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t FormImage_clg;
	extern ocrGuid_t FormImage_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&FormImage_clg,		// ocrGuid_t *new_guid
			 FormImage_edt,		// ocr_edt_ptr func_ptr
			1,			// paramc
#ifdef RAG_DIG_SPOT
			6			// depc
#else
			5			// depc
#endif
		);
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for Affine_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t Affine_clg;
	extern ocrGuid_t Affine_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&Affine_clg,		// ocrGuid_t *new_guid
			 Affine_edt,		// ocr_edt_ptr func_ptr
			2,			// paramc
			4);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for post_Affine_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_Affine_clg;
	extern ocrGuid_t post_Affine_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&post_Affine_clg,	// ocrGuid_t *new_guid
			 post_Affine_edt,	// ocr_edt_ptr func_ptr
			0,			// paramc
			6);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for post_affine_async_1_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_affine_async_1_clg;
	extern ocrGuid_t post_affine_async_1_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&post_affine_async_1_clg,	// ocrGuid_t *new_guid
			 post_affine_async_1_edt,	// ocr_edt_ptr func_ptr
			1,				// paramc
			10);				// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for post_affine_async_2_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_affine_async_2_clg;
	extern ocrGuid_t post_affine_async_2_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&post_affine_async_2_clg,	// ocrGuid_t *new_guid
			 post_affine_async_2_edt,	// ocr_edt_ptr func_ptr
			0,				// paramc
			10);				// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for CCD_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t CCD_clg;
	extern ocrGuid_t CCD_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&CCD_clg,		// ocrGuid_t *new_guid
			 CCD_edt,		// ocr_edt_ptr func_ptr
			0,			// paramc
			5);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for post_CFAR function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_CFAR_clg;
	retval = ocrEdtTemplateCreate(
			&post_CFAR_clg,		// ocrGuid_t *new_guid
			 post_CFAR_edt,		// ocr_edt_ptr func_ptr
			0,			// paramc
			3);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for post_CFAR\n");RAG_FLUSH;
#endif
	ocrGuid_t post_CFAR_scg, post_CFAR_evg;
	retval = ocrEdtCreate(
			&post_CFAR_scg,		// *created_edt_guid
			 post_CFAR_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&post_CFAR_evg);	// *outputEvent
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create a template for CFAR_edt function\n");RAG_FLUSH;
#endif
	ocrGuid_t CFAR_clg;
	extern ocrGuid_t CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	retval = ocrEdtTemplateCreate(
			&CFAR_clg,		// ocrGuid_t *new_guid
			 CFAR_edt,		// ocr_edt_ptr func_ptr
			1,			// paramc
			5			// depc
			);
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for CFAR_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t CFAR_scg, CFAR_evg;
{	uint64_t paramv[1] = { GUID_VALUE(post_CFAR_scg) };
	retval = ocrEdtCreate(
			&CFAR_scg,		// *created_edt_guid
			 CFAR_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&CFAR_evg);		// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for curImage/refImage CDD_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t CCD_scg, CCD_evg;
	retval = ocrEdtCreate(
			&CCD_scg,		// *created_edt_guid
			 CCD_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&CCD_evg);		// *outputEven
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for post_Affine\n");RAG_FLUSH;
#endif
	ocrGuid_t post_Affine_scg, post_Affine_evg;
	retval = ocrEdtCreate(
			&post_Affine_scg,	// *created_edt_guid
			 post_Affine_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&post_Affine_evg);	// *outputEvent
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for post_affine_async_2_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t post_affine_async_2_scg, post_affine_async_2_evg;
	retval = ocrEdtCreate(
			&post_affine_async_2_scg,	// *created_edt_guid
			 post_affine_async_2_clg, 	// edt_template_guid
			EDT_PARAM_DEF,			// paramc
			NULL,				// *paramv
			EDT_PARAM_DEF,			// depc
			NULL,				// *depv
			EDT_PROP_FINISH,		// properties
			NULL_GUID,			// affinity
			&post_affine_async_2_evg);	// *outputEvent
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt post_affine_async_1_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t post_affine_async_1_scg, post_affine_async_1_evg;
{	uint64_t paramv[1] = { GUID_VALUE(post_affine_async_2_scg) };
	retval = ocrEdtCreate(
			&post_affine_async_1_scg,	// *created_edt_guid
			 post_affine_async_1_clg, 	// edt_template_guid
			EDT_PARAM_DEF,			// paramc
			paramv,				// *paramv
			EDT_PARAM_DEF,			// depc
			NULL,				// *depv
			EDT_PROP_FINISH,		// properties
			NULL_GUID,			// affinity
			&post_affine_async_1_evg);	// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for curImage/refImage Affine_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t Affine_scg, Affine_evg;
{	uint64_t paramv[2] = { GUID_VALUE(post_Affine_scg), GUID_VALUE(post_affine_async_1_scg) };
	retval = ocrEdtCreate(
			&Affine_scg,		// *created_edt_guid
			 Affine_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&Affine_evg);		// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for curImage FormImage_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t FormImage_scg;
{	uint64_t paramv[1] = { GUID_VALUE(Affine_scg) };
	retval = ocrEdtCreate(
			&FormImage_scg,		// *created_edt_guid
			 FormImage_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for curImage ReadData_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t ReadData_scg;
{	uint64_t paramv[1] = { GUID_VALUE(FormImage_scg) };
	retval = ocrEdtCreate(
			&ReadData_scg,		// *created_edt_guid
			 ReadData_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for refImage FormImage_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t refFormImage_scg;
{	uint64_t paramv[1] = { GUID_VALUE(ReadData_scg) };
	retval = ocrEdtCreate(
			&refFormImage_scg,	// *created_edt_guid
			    FormImage_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
	assert(retval==0);

#ifdef TRACE_LVL_1
xe_printf("// create an edt for refImage ReadData_edt\n");RAG_FLUSH;
#endif
	ocrGuid_t refReadData_scg;
{	uint64_t paramv[1] = { GUID_VALUE(refFormImage_scg) };
	retval = ocrEdtCreate(
			&refReadData_scg,	// *created_edt_guid
			    ReadData_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
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
// done by refReadInputs_edt
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
#ifdef RAG_SIM_NULL_GUID_WORKAROUND
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,4);
#else
RAG_DEF_MACRO_SPAD(refFormImage_scg,NULL,NULL,NULL,NULL,NULL_GUID,4);
#endif
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
// done by ReadInputs_edt
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

#if RAG_AFFINE_ON
#ifdef TRACE_LVL_1
xe_printf("// Affine registration\n");RAG_FLUSH;
#endif
//RAG_DEF_MACRO_SPAD(Affine_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);
// done by FormInputs_edt
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
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,refImage_dbg,1);
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,corr_map_dbg,2);
RAG_DEF_MACRO_SPAD(CCD_scg,NULL,NULL,NULL,NULL,image_params_dbg,3);
//	        CCD(curImage, refImage, corr_map, image_params);

#ifdef TRACE_LVL_1
xe_printf("// Constant False Alarm Rate\n");RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,corr_map_dbg,0);
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,cfar_params_dbg,2);
RAG_DEF_MACRO_SPAD(CFAR_scg,NULL,NULL,NULL,NULL,Y_dbg,3);
//		CFAR(corr_map, image_params, cfar_params, Y);

RAG_DEF_MACRO_SPAD(post_affine_async_1_scg,NULL,NULL,NULL,NULL,Affine_evg,9);
RAG_DEF_MACRO_SPAD(post_affine_async_2_scg,NULL,NULL,NULL,NULL,post_affine_async_1_evg,9);
RAG_DEF_MACRO_SPAD(post_Affine_scg,        NULL,NULL,NULL,NULL,post_affine_async_2_evg,5);
RAG_DEF_MACRO_SPAD(CCD_scg,                NULL,NULL,NULL,NULL,post_Affine_evg,4);
RAG_DEF_MACRO_SPAD(CFAR_scg,               NULL,NULL,NULL,NULL,CCD_evg,4);
RAG_DEF_MACRO_SPAD(post_CFAR_scg,          NULL,NULL,NULL,NULL,CFAR_evg,2);

	} // while
#ifdef TRACE_LVL_1
xe_printf("// leave main_body_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
//////////////////////////////////////////////////////////////////////////
ocrGuid_t post_main_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv)
{
	int retval;
	assert(paramc==0);
	assert(depc==22);
	FILE *pInFile, *pInFile2, *pInFile3;
#ifdef TRACE_LVL_1
xe_printf("// enter post_main_edt\n");RAG_FLUSH;
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
xe_printf("// Output Images to .bin files\n");RAG_FLUSH;
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
            for(int n=0; n<image_params->Ix; n++) {
		xe_printf("cur 0x%x 0x%x\n",*(uint32_t *)&curImage[m][n].real, *(uint32_t *)&curImage[m][n].imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params->Iy; m++) {
            for(int n=0; n<image_params->Ix; n++) {
		xe_printf("ref 0x%x 0x%x\n",*(uint32_t *)&refImage[m][n].real, *(uint32_t *)&refImage[m][n].imag);
	    } // for n
	} // for m

        for(int m=0; m<image_params->Iy-image_params->Ncor+1; m++) {
            for(int n=0; n<image_params->Ix-image_params->Ncor+1; n++) {
		xe_printf("corr 0x%x 0x%x 0x%x\n",*(uint32_t *)&corr_map[m][n].x, *(uint32_t *)&corr_map[m][n].y,*(uint32_t *)&corr_map[m][n].p);
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
#ifdef RAG_DRAM
	dram_free(corr_map_data_ptr,corr_map_data_dbg); // corr_map[]
#else
	 bsm_free(corr_map_data_ptr,corr_map_data_dbg); // corr_map[]
#endif
	bsm_free(corr_map,corr_map_dbg);
#ifdef RAG_DRAM
	dram_free(refImage_data_ptr,refImage_data_dbg); //refImage[]
#else
	 bsm_free(refImage_data_ptr,refImage_data_dbg); //refImage[]
#endif
	bsm_free(refImage,refImage_dbg);
#ifdef RAG_DRAM
	dram_free(curImage_data_ptr,curImage_data_dbg); // curImage[]
#else
	 bsm_free(curImage_data_ptr,curImage_data_dbg); // curImage[]
#endif
	bsm_free(curImage,curImage_dbg);
	bsm_free(in_Tp_ptr,in_Tp_dbg);
#ifdef RAG_DRAM
	dram_free(in_Pt_data_ptr,in_Pt_data_dbg); // in.Pt[]
#else
	 bsm_free(in_Pt_data_ptr,in_Pt_data_dbg); // in.Pt[]
#endif
	bsm_free(in_Pt_ptr,in_Pt_dbg);
#ifdef RAG_DRAM
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
xe_printf("// leave post_main_edt\n");RAG_FLUSH;
#endif
	xe_exit(0);
	return NULL_GUID;
}
