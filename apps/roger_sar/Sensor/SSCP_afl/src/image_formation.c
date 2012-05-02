#include "common.h"

#include "rag_rmd.h"

rmd_guid_t post_FormImage_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter post_FormImage_codelet\n");RAG_FLUSH;
#endif
#ifdef RAG_DIG_SPOT
	assert(n_db == 6);
#else
	assert(n_db == 5);
#endif
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,in_dbg,0);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif
	rmd_guid_t arg_scg =  { .data = arg };
	if( refImage_dbg.data == NULL_GUID.data ) {
#ifdef TRACE_LVL_2
xe_printf("//// refImage_dbg == NULL_GUID\n");RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,in_dbg,0); 
	} else {
#ifdef TRACE_LVL_2
xe_printf("//// refImage_dbg == struct complexData **\n");RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,curImage_dbg,0); 
	RMD_DB_RELEASE(refImage_dbg);
	}
	RMD_DB_RELEASE(in_dbg);
	RMD_DB_RELEASE(image_params_dbg);
	RMD_DB_RELEASE(radar_params_dbg);
	RMD_DB_RELEASE(curImage_dbg);
#ifdef RAG_DIG_SPOT
	RMD_DB_RELEASE(dig_spot_dbg);
#endif
#ifdef TRACE_LVL_2
xe_printf("//// leave post_FormImage_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

rmd_guid_t FormImage_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter FormImage_codelet\n");RAG_FLUSH;
#endif
#ifdef RAG_DIG_SPOT
	assert(n_db == 6);
#else
	assert(n_db == 5);
#endif

RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,2);
RAG_REF_MACRO_BSM( struct complexData **,curImage,curImage_ptr,curImage_lcl,curImage_dbg,3);
RAG_REF_MACRO_BSM( struct complexData **,refImage,refImage_ptr,refImage_lcl,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_REF_MACRO_BSM( struct DigSpotVars,dig_spot,dig_spot_ptr,dig_spot_lcl,dig_spot_dbg,5);
#endif

#ifdef TRACE_LVL_2
xe_printf("//// create a codelet for post_FormImage function\n");RAG_FLUSH;
#endif
	rmd_guid_t post_FormImage_clg;
	retval = rmd_codelet_create(
		&post_FormImage_clg,     // rmd_guid_t *new_guid
		 post_FormImage_codelet, // rmd_codelet_ptr func_ptr
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

#ifdef TRACE_LVL_2
xe_printf("//// create an instance for post_FormImage\n");RAG_FLUSH;
#endif
	rmd_guid_t post_FormImage_scg;
	retval = rmd_codelet_sched(
		&post_FormImage_scg,	// rmd_guid_t* scheduled codelet's guid
		arg,			// uint64_t arg
		post_FormImage_clg);	// rmd_guid_t created codelet's guid
	assert(retval==0);

//RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,in_dbg,0);
// done by post_backproject_codelet
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif
	if(refImage != NULL) {
#ifdef TRACE_LVL_2
xe_printf("//// Copy curImage to refImage\n");RAG_FLUSH;
#endif
		for(int m=0; m<image_params->Iy; m++) {
			struct complexData *ref_m;
			ref_m = (struct complexData *)RAG_GET_PTR(refImage+m);
			struct complexData *cur_m;
			cur_m = (struct complexData *)RAG_GET_PTR(curImage+m);
// RAG refImage should be in DRAM
			BSMtoBSM(ref_m, cur_m, image_params->Ix*sizeof(struct complexData));
		} // for m
	} // if !NULL

#ifdef TRACE_LVL_2
xe_printf("//// Zero curImage\n");RAG_FLUSH;
#endif
	for(int n=0; n<image_params->Iy; n++) {
		bsm_memset(RAG_GET_PTR(curImage+n), 0, image_params->Ix*sizeof(struct complexData));
	}
	
	if(image_params->TF > 1) {
#ifndef RAG_DIG_SPOT
// to stop defined but unused warning by compiler
		radar_params = radar_params;
		xe_printf("!!! DIGITAL SPOTLIGHTING NOT YET SUPPORTED !!!\n");RAG_FLUSH;
		exit(1);
#else // RAG_DIG_SPOT
#ifdef TRACE_LVL_2
xe_printf("//// FormImage FFTW initialization TF = %d\n",image_params->TF);RAG_FLUSH;
#endif
		fftwf_plan plan_forward;
		fftwf_complex *input, *fft_result;
		struct complexData **Xsubimg;
		float xc, yc;
		input = (fftwf_complex*)fftwf_malloc(image_params->S1 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)fftwf_malloc(image_params->S1 * sizeof(fftwf_complex));
		plan_forward = fftwf_plan_dft_1d(image_params->S1, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

		for(int i=0; i<image_params->P1; i++) {
// Copy data from one pulse to FFT input array (first and second halves swapped)
			for(j=(int)ceilf(image_params->S1/2), n=0; j<image_params->S1; j++, n++) {
				input[n][0] = in->X[i][j].real;
				input[n][1] = in->X[i][j].imag;
			}
			for(int j=0; j<(int)ceilf(image_params->S1/2); j++, n++) {
				input[n][0] = in->X[i][j].real;
				input[n][1] = in->X[i][j].imag;
			}

			// Perform FFT
			fftwf_execute(plan_forward);

			// Copy FFT results to X2
			memcpy(&dig_spot->X2[i][0], fft_result, image_params->S1*sizeof(struct complexData));
		}

		for(int i=0; i<image_params->TF; i++) {
			for(int j=0; j<image_params->TF; j++) {
				// Subimage center
				xc = image_params->xr[(image_params->Sx-1)/2 + i*image_params->Sx];
				yc = image_params->yr[(image_params->Sy-1)/2 + j*image_params->Sy];

				// Perform Digital Spotlighting
				Xsubimg = DigSpot(xc, yc, dig_spot, image_params, radar_params, in);

				// Adjust platform locations
				for(int n=0; n<image_params->P2; n++) {
					dig_spot->Pt2[n][0] = in->Pt[n*image_params->TF][0] + xc;
					dig_spot->Pt2[n][1] = in->Pt[n*image_params->TF][1] + yc;
					dig_spot->Pt2[n][2] = in->Pt[n*image_params->TF][2];
				}

#ifdef TRACE_LVL_2
xe_printf("//// Perform backprojection over subimage\n");RAG_FLUSH;
#endif
		struct Corners_t *corners, *corners_ptr,corners_lcl; rmd_guid_t corners_dbg;
		corners_ptr   = bsm_malloc(&corners_dbg,sizeof(struct Corners_t));
		corners = &corners_lcl;
		corners->x1   = i*image_params->Sx;
		corners->x2   = (i+1)*image_params->Sx;
		corners->y1   = j*image_params->Sy;
		corners->y2   = (j+1)*image_params->Sy;
		corners->slot = 0;
		REM_STX_ADDR(corners_ptr,corners_lcl,struct Corners_t);
		struct Inputs tmp_in, *tmp_in_ptr; rmd_guid_t tmp_in_dbg;
		tmp_in_ptr = bsm_malloc(&tmp_in_dbg,sizeof(struct Inputs));
		tmp_in.Pt = dig_spot->Pt2
		tmp_in.Pt_edge_dbg = NULL_GUID;
		tmp_in.Pt_data_dbg = NULL_GUID;
		tmp_in.Pl = NULL;
		tmp_in.Pl_dbg = NULL_GUID;
		tmp_in.X  = Xsubimg;
		tmp_in.X_edge_dbg  = NULL_GUID;
		tmp_in.X_data_dbg  = NULL_GUID;;
		REM_STX_ADDR_SIZE(tmp_in_ptr,tmp_in,sizeof(struct Inputs));
				BackProj(1,tmp_dbg, corners_dbg, // Xup, platpos
 				image_params_dbg, radar_params_dbg,
				curImage_dbg, refImage_dbg);
			}
		}
		RMD_DB_RELEASE(tmp_in);	
		RMD_DB_RELEASE(corners_dbg);	
		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_destroy_plan(plan_forward);
#endif // RAG_DIG_SPOT
	} else { // else not digital spot light
#ifdef TRACE_LVL_2
xe_printf("//// Perform backprojection over full image\n");RAG_FLUSH;
#endif
		struct Corners_t *corners, *corners_ptr,corners_lcl; rmd_guid_t corners_dbg;
		corners_ptr = bsm_malloc(&corners_dbg,sizeof(struct Corners_t));
		corners = &corners_lcl;
		corners->x1   = 0;
		corners->x2   = image_params->Ix;
		corners->y1   = 0;
		corners->y2   = image_params->Iy;
		corners->slot = 0;
		REM_STX_ADDR(corners_ptr,corners_lcl,struct Corners_t);
#ifdef TRACE_LVL_2
xe_printf("//// create a codelet for BackProj function\n");RAG_FLUSH;
#endif
	rmd_guid_t BackProj_clg;
	retval = rmd_codelet_create(
		&BackProj_clg,		// rmd_guid_t *new_guid
		 BackProj_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		6,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE_LVL_2
xe_printf("//// create an instance for post_FormImage\n");RAG_FLUSH;
#endif
	rmd_guid_t BackProj_scg;
	retval = rmd_codelet_sched(
		&BackProj_scg,		// rmd_guid_t* scheduled codelet's guid
		post_FormImage_scg.data,// uinit64_t arg
		BackProj_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,in_dbg,0); // Xin, platpos
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,corners_dbg,1);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,curImage_dbg,4);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,refImage_dbg,5);
		RMD_DB_RELEASE(in_dbg);
		RMD_DB_RELEASE(corners_dbg);
		RMD_DB_RELEASE(image_params_dbg);
		RMD_DB_RELEASE(radar_params_dbg);
		RMD_DB_RELEASE(curImage_dbg);
		if(refImage != NULL)RMD_DB_RELEASE(refImage_dbg);
	} // endif digital spot light
#ifdef TRACE_LVL_2
xe_printf("//// leave FormImage_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
