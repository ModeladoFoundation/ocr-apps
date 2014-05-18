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

ocrGuid_t post_FormImage_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter post_FormImage_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0]; // ReadData_edt or Affine_edt
#ifdef RAG_DIG_SPOT
	assert(depc==7);
#else
	assert(depc==6);
#endif
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,in_dbg,0);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif

#ifdef RAG_SIM_NULL_GUID_WORKAROUND
	if( GUID_VALUE(refImage_dbg) == GUID_VALUE(curImage_dbg) ) {	// RAG NULL_GUID FSIM BUG WORKAROUND
#else
	if( GUID_VALUE(refImage_dbg) == GUID_VALUE(NULL_GUID) ) {	// RAG NULL_GUID FSIM BUG WORKAROUND
#endif
#ifdef TRACE_LVL_2
xe_printf("//// refImage_dbg == NULL_GUID (next guid = %ld) %ld\n",GUID_VALUE(arg_scg),GUID_VALUE(in_dbg));RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,in_dbg,0);	// ReadData_edt
	} else {
#ifdef TRACE_LVL_2
xe_printf("//// refImage_dbg == struct complexData ** (next guid = %ld) %ld\n",GUID_VALUE(arg_scg),GUID_VALUE(curImage_dbg));RAG_FLUSH;
#endif
RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);	// Affine_edt
	OCR_DB_RELEASE(refImage_dbg);
	}
	OCR_DB_RELEASE(in_dbg);
	OCR_DB_RELEASE(image_params_dbg);
	OCR_DB_RELEASE(radar_params_dbg);
	OCR_DB_RELEASE(curImage_dbg);
#ifdef RAG_DIG_SPOT
	OCR_DB_RELEASE(dig_spot_dbg);
#endif
#ifdef TRACE_LVL_2
xe_printf("//// leave post_FormImage_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t FormImage_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter FormImage_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0]; // ReadData_edt or Affine_edt
#ifdef RAG_DIG_SPOT
	assert(depc==6);
#else
	assert(depc==5);
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
xe_printf("//// create a template for post_FormImage function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_FormImage_clg;
	retval = ocrEdtTemplateCreate(
			&post_FormImage_clg,	// ocrGuid_t *new_guid
		 	post_FormImage_edt,	// ocr_edt_ptr func_ptr
			1,			// paramc
#ifdef RAG_DIG_SPOT
			7			// depc
#else
			6			// depc
#endif
		);
	assert(retval==0);

#ifdef TRACE_LVL_2
xe_printf("//// create an edt for post_FormImage\n");RAG_FLUSH;
#endif
	ocrGuid_t post_FormImage_scg;
	retval = ocrEdtCreate(
			&post_FormImage_scg,	// *created_edt_guid
			 post_FormImage_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv (arg_scg, i.e. ReadData_edt or Affine_edt)
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,in_dbg,0);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,radar_params_dbg,2);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,refImage_dbg,4);
#ifdef RAG_DIG_SPOT
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,dig_spot_dbg,5);
#endif
#ifdef RAG_SIM_NULL_GUID_WORKAROUND
	if(refImage != curImage) {	// RAG NULL_GUID FSIM BUG WORKAROUND
#else
	if(refImage != NULL) {		// RAG NULL_GUID FSIM BUG WORKAROUND
#endif
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
		xe_exit(1);
#else // RAG_DIG_SPOT
#ifdef TRACE_LVL_2
xe_printf("//// FormImage FFTW initialization TF = %d\n",image_params->TF);RAG_FLUSH;
#endif
		fftwf_plan plan_forward;
		fftwf_complex *input, *fft_result; ocrGuid_t input_dbg, fft_result_dbg;
		struct complexData **Xsubimg;
		float xc, yc;
		input = (fftwf_complex*)fftwf_malloc(&input_dbg,image_params->S1 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)fftwf_malloc(&fft_result_dbg,image_params->S1 * sizeof(fftwf_complex));
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
		struct corners_t *corners, *corners_ptr,corners_lcl; ocrGuid_t corners_dbg;
		corners_ptr   = bsm_malloc(&corners_dbg,sizeof(struct corners_t));
		corners = &corners_lcl;
		corners->m1   = i*image_params->Sx;
		corners->m2   = (i+1)*image_params->Sx;
		corners->n1   = j*image_params->Sy;
		corners->n2   = (j+1)*image_params->Sy;
		REM_STX_ADDR(corners_ptr,corners_lcl,struct corners_t);
		struct Inputs tmp_in, *tmp_in_ptr; ocrGuid_t tmp_in_dbg;
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
		OCR_DB_RELEASE(tmp_in);
		OCR_DB_RELEASE(corners_dbg);
		fftwf_free(input,input_dbg);
		fftwf_free(fft_result,fft_result_dbg);
		fftwf_destroy_plan(plan_forward);
#endif // RAG_DIG_SPOT
	} else { // else not digital spot light
#ifdef TRACE_LVL_2
xe_printf("//// Perform backprojection over full image\n");RAG_FLUSH;
#endif
		struct corners_t *corners, *corners_ptr,corners_lcl; ocrGuid_t corners_dbg;
		corners_ptr = bsm_malloc(&corners_dbg,sizeof(struct corners_t));
		corners = &corners_lcl;
		corners->m1   = 0;
		corners->m2   = image_params->Ix;
		corners->n1   = 0;
		corners->n2   = image_params->Iy;
		REM_STX_ADDR(corners_ptr,corners_lcl,struct corners_t);
#ifdef TRACE_LVL_2
xe_printf("//// create a template for BackProj function\n");RAG_FLUSH;
#endif
	ocrGuid_t BackProj_clg;
	retval = ocrEdtTemplateCreate(
			&BackProj_clg,		// ocrGuid_t *new_guid
		 	 BackProj_edt,		// ocr_edt_ptr func_ptr
			0,			// paramc
			6);			// depc
	assert(retval==0);
#ifdef TRACE_LVL_2
xe_printf("//// create an edt for BackProj\n");RAG_FLUSH;
#endif
	ocrGuid_t BackProj_scg, BackProj_evg;
	retval = ocrEdtCreate(
			&BackProj_scg,		// *created_edt_guid
			 BackProj_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_FINISH,	// properties
			NULL_GUID,		// affinity
			&BackProj_evg);		// *outputEvent
	assert(retval==0);

RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,in_dbg,0); // Xin, platpos
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,corners_dbg,1);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,curImage_dbg,4);
RAG_DEF_MACRO_PASS(BackProj_scg,NULL,NULL,NULL,NULL,refImage_dbg,5);
#ifdef RAG_DIG_SPOT
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,BackProj_evg,6);
#else
RAG_DEF_MACRO_PASS(post_FormImage_scg,NULL,NULL,NULL,NULL,BackProj_evg,5);
#endif
		OCR_DB_RELEASE(in_dbg);
		OCR_DB_RELEASE(corners_dbg);
		OCR_DB_RELEASE(image_params_dbg);
		OCR_DB_RELEASE(radar_params_dbg);
		OCR_DB_RELEASE(curImage_dbg);
		if(refImage != NULL)OCR_DB_RELEASE(refImage_dbg);
	} // endif digital spot light
#ifdef TRACE_LVL_2
xe_printf("//// leave FormImage_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
