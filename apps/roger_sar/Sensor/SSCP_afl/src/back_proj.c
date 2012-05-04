#include "common.h"

#include "rag_rmd.h"

rmd_guid_t backproject_final_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE_LVL_4
xe_printf("//////// enter backproject_final\n");RAG_FLUSH;
#endif
	assert(n_db>1);
	rmd_guid_t arg_scg = { .data = arg };
	RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,db[0],0);
#ifdef TRACE_LVL_4
xe_printf("//////// leave backproject_final\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

rmd_guid_t backproject_async_codelet(uint64_t rmd_arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
	assert(n_db == 5);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,in_dbg,0);
RAG_REF_MACRO_SPAD(struct Corners_t,corners,corners_ptr,corners_lcl,corners_dbg,1);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,radar_params_dbg,3);
RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_dbg,4);

	struct complexData sample, acc, arg;
	struct Inputs in, *in_ptr;
	RMD_DB_MEM(&in_ptr, in_dbg);
	REM_LDX_ADDR(in,in_ptr,struct Inputs);
	struct complexData **Xin;
	Xin     = in.X;

	float **platpos;
	platpos = in.Pt;

	int x1   = corners->x1;
	int x2   = corners->x2;
	int y1   = corners->y1;
	int y2   = corners->y2;
	int slot = corners->slot;

#ifdef TRACE_LVL_4
xe_printf("//////// enter backproject_async slot %d\n",slot);RAG_FLUSH;
#endif

	struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
	RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
	REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
	image_params = &image_params_lcl;

	struct complexData **image;
	RMD_DB_MEM(&image,      image_dbg);

	struct RadarParams *radar_params, *radar_params_ptr, radar_params_lcl;
	RMD_DB_MEM(&radar_params_ptr,  radar_params_dbg);
	REM_LDX_ADDR(radar_params_lcl, radar_params_ptr, struct RadarParams);
	radar_params = &radar_params_lcl;

        float R, bin, w;

#ifdef RAG_PURE_FLOAT
	const float ku = 2.0f*M_PI*radar_params->fc/c_mks_mps;
#else
	const float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif
	for(int m=x1; m<x2; m++) {
#ifdef DEBUG
xe_printf("backproject_async m(%d)\n",m);RAG_FLUSH;
#endif
		for(int n=y1; n<y2; n++) {
#ifdef DEBUG
xe_printf("backproject_async n(%d)\n",n);RAG_FLUSH;
#endif
			acc.real = 0;
			acc.imag = 0;
			for(int k=0; k<image_params->P3; k++) {
#ifdef DEBUG
xe_printf("backproject_async k(%d)\n",k);RAG_FLUSH;
#endif
				float *platpos_k;
				platpos_k = (float *)RAG_GET_PTR(platpos+k);
				float platpos_k_012[3];
				REM_LDX_ADDR(platpos_k_012,platpos_k,float [3]);
#ifdef RAG_AFL
assert(platpos[k][0] == platpos_k_012[0]);
assert(platpos[k][1] == platpos_k_012[1]);
assert(platpos[k][2] == platpos_k_012[2]);
#endif
				float x = RAG_GET_FLT(image_params->xr+m) - platpos_k_012[0];
				float y = RAG_GET_FLT(image_params->yr+n) - platpos_k_012[1];
				float z =                                 - platpos_k_012[2];
				R = sqrtf( x*x + y*y + z*z );
#ifdef DEBUG
xe_printf("backproject_async                 R(%f)\n",R);RAG_FLUSH;
#endif
				if(image_params->TF > 1) {
					bin = (R-radar_params->R0_prime)/image_params->dr;
				}
				else {
					bin = (R-radar_params->R0)/image_params->dr;
				}

				if(bin >= 0 && bin < image_params->S4-1) {
					struct complexData *Xin_k;
					Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
					struct complexData left,right;
					int int_bin = (int)floorf(bin);
					w = bin - int_bin;
					REM_LDX_ADDR(left, Xin_k+int_bin+0,struct complexData);
					REM_LDX_ADDR(right,Xin_k+int_bin+1,struct complexData);
					sample.real = (1-w)*left.real + w*right.real;
					sample.imag = (1-w)*left.imag + w*right.imag;
				} else if (bin > image_params->S4-1) {
					struct complexData *Xin_k;
					Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
					struct complexData edge;
					REM_LDX_ADDR(edge, Xin_k+image_params->S4-1,struct complexData);
					sample.real = edge.real;
					sample.imag = edge.imag;
				} else {
					sample.real = 0.0f;
					sample.imag = 0.0f;
				}
#ifdef RAG_SINCOS
				sincosf(2*ku*R,&arg.imag,&arg.real);
#else
				arg.real = cosf(2*ku*R);
				arg.imag = sinf(2*ku*R);
#endif
				acc.real += sample.real*arg.real - sample.imag*arg.imag;
				acc.imag += sample.real*arg.imag + sample.imag*arg.real;
			} // for k
#ifdef DEBUG
xe_printf("backproject_async                 update image[%d][%d]\n",m,n);RAG_FLUSH;
#endif
			struct complexData *image_n;
			image_n = (struct complexData *)RAG_GET_PTR(image+n);
			REM_STX_ADDR(image_n+m,acc,struct complexData);
                } // for n
        } // for m
#ifdef TRACE_LVL_4
xe_printf("//////// leave backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
	rmd_guid_t arg_scg = { .data = rmd_arg };
	RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,NULL_GUID,slot);
	bsm_free(corners_ptr,corners_dbg);
	return NULL_GUID;
} // backproject_async

rmd_guid_t BackProj_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter BackProj_codelet\n");RAG_FLUSH;
#endif
assert(n_db == 6);
RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,0);
RAG_REF_MACRO_SPAD(struct Corners_t,corners,corners_ptr,corners_lcl,corners_dbg,1);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,3);
RAG_REF_MACRO_BSM( struct complexData **,image,image_ptr,image_lcl,image_dbg,4);
RAG_REF_MACRO_BSM( struct complexData **,refImage,refImage_ptr,curImage_lcl,refImage_dbg,5);
	struct complexData **Xin;
	Xin     = in->X;
#if 0
	float **platpos;
	platpos = in.Pt;
#endif
	int x1   = corners->x1;
	int x2   = corners->x2;
	int y1   = corners->y1;
	int y2   = corners->y2;
	int slot = corners->slot;
assert(slot==0);
	if(image_params->F > 1) {
		fftwf_complex *input, *fft_result, *ifft_result;
		fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

#ifdef TRACE_LVL_3
xe_printf("////// BackProj FFTW initialization F = %d\n",image_params->F);RAG_FLUSH;
#endif
		input         = (fftwf_complex*)fftwf_malloc(image_params->S3 * sizeof(fftwf_complex));
		fft_result    = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		ifft_result   = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		plan_forward  = fftwf_plan_dft_1d(image_params->S3, input,      fft_result,  FFTW_FORWARD, FFTW_ESTIMATE);
		plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

		float scale = 1/(float)image_params->S4;
		struct complexData **Xup;
	        rmd_guid_t Xup_dbg;
		Xup = (struct complexData**)bsm_malloc(&Xup_dbg,image_params->P3*sizeof(struct complexData*));
		if(Xup == NULL) {
			fprintf(stderr,"Error allocating edge vector for Xup.\n");
			exit(1);
		}
      		struct complexData* Xup_data_ptr = NULL; rmd_guid_t Xup_data_dbg;
#ifdef RAG_AFL
		Xup_data_ptr = (struct complexData*)dram_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#else
		Xup_data_ptr = (struct complexData*) bsm_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#endif
		if ( Xup_data_ptr == NULL) {
			fprintf(stderr,"Error allocating data memory for Xup.\n");
			exit(1);
		}
		for(int n=0; n<image_params->P3; n++) {
			RAG_PUT_PTR(&Xup[n], Xup_data_ptr + n*image_params->S4);
		}

		for(int m=0; m<image_params->P3; m++) {
			struct complexData *Xup_m;
			Xup_m = (struct complexData *)RAG_GET_PTR(Xup+m);
			struct complexData *Xin_m;
			Xin_m = (struct complexData *)RAG_GET_PTR(Xin+m);
			REM_LDX_ADDR_SIZE(input, Xin_m,
				image_params->S3*sizeof(struct complexData));

			fftwf_execute(plan_forward);

			memset(&fft_result[image_params->S3][0], 0,
				(image_params->S4-image_params->S3)*sizeof(fftwf_complex));
			
			fftwf_execute(plan_backward);			

			for(int n=0; n<image_params->S4; n++) {
				ifft_result[n][0] *= scale;
				ifft_result[n][1] *= scale;
			}
			REM_STX_ADDR_SIZE(Xup_m,ifft_result,
				image_params->S3*sizeof(struct complexData));
		}
		// Free memory and deallocate plan
		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_free(ifft_result);
		fftwf_destroy_plan(plan_forward);
		fftwf_destroy_plan(plan_backward);
#ifdef TRACE
xe_printf("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is > 1)\n",
			x1, x2, y1, y2);RAG_FLUSH;
#endif
		struct Inputs tmp_in, *tmp_in_ptr; rmd_guid_t tmp_in_dbg;
		tmp_in_ptr = bsm_malloc(&tmp_in_dbg,sizeof(struct Inputs));
		tmp_in.Pt = in->Pt;
		tmp_in.Pt_edge_dbg = in->Pt_edge_dbg;
		tmp_in.Pt_data_dbg = in->Pt_data_dbg;
		tmp_in.Tp = NULL;
		tmp_in.Tp_dbg = NULL_GUID;
		tmp_in.X  = Xup;
		tmp_in.X_edge_dbg  = Xup_dbg;
		tmp_in.X_data_dbg  = Xup_data_dbg;;
		REM_STX_ADDR_SIZE(tmp_in_ptr,tmp_in,sizeof(struct Inputs));

		assert( ((x2-x1)%BACK_PROJ_ASYNC_BLOCK_SIZE) == 0);
		assert( ((y2-y1)%BACK_PROJ_ASYNC_BLOCK_SIZE) == 0);

// create a codelet for backproject_async function
	rmd_guid_t backproject_async_clg;
	retval = rmd_codelet_create(
		&backproject_async_clg,     // rmd_guid_t *new_guid
		 backproject_async_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		5,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
// create a codelet for backproject_final function
	rmd_guid_t backproject_final_clg;
	retval = rmd_codelet_create(
		&backproject_final_clg,     // rmd_guid_t *new_guid
		 backproject_final_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		((x2-x1)/BACK_PROJ_ASYNC_BLOCK_SIZE)*((y2-y1)/BACK_PROJ_ASYNC_BLOCK_SIZE)+1, // int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
// create an instance for backproject_final
	rmd_guid_t backproject_final_scg;
	retval = rmd_codelet_sched(
		&backproject_final_scg,		// rmd_guid_t* scheduled codelet's guid
		arg,				// uint64_t arg
		backproject_final_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
RAG_DEF_MACRO_PASS(backproject_final_scg,NULL,NULL,NULL,NULL,in_dbg,slot++);
		for(int m=x1; m<x2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE) {
			for(int n=y1; n<y2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE) {
#ifdef TRACE_LVL_3
xe_printf("////// create an instance for backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
				struct Corners_t *async_corners, *async_corners_ptr, async_corners_lcl; rmd_guid_t async_corners_dbg;
				async_corners = &async_corners_lcl;
				async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct Corners_t));
				async_corners->x1   = m;
				async_corners->x2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE;
				async_corners->y1   = n;
				async_corners->y2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE;
				async_corners->slot = slot++;
				REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct Corners_t);
				rmd_guid_t backproject_async_scg;
				retval = rmd_codelet_sched(
		&backproject_async_scg,		// rmd_guid_t* scheduled codelet's guid
		backproject_final_scg.data,	// uint64_t arg
		backproject_async_clg);		// rmd_guid_t created codelet's guid
				assert(retval==0);

RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,tmp_in_dbg,0); // Xup,platpos
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
				RMD_DB_RELEASE(async_corners_dbg);
			} // for n
		} // for m
		bsm_free(tmp_in_ptr,tmp_in_dbg);
#ifdef RAG_AFL
		dram_free(Xup_data_ptr,Xup_data_dbg); // Xup[]
#else
		 bsm_free(Xup_data_ptr,Xup_data_dbg); // Xup[]
#endif
		bsm_free(Xup,Xup_dbg);
	} else { // if F
#ifdef TRACE
xe_printf("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is = 1)\n",
			x1, x2, y1, y2);RAG_FLUSH;
#endif
		assert( ((x2-x1)%BACK_PROJ_ASYNC_BLOCK_SIZE) == 0);
		assert( ((y2-y1)%BACK_PROJ_ASYNC_BLOCK_SIZE) == 0);
// create a codelet for backproject_async function
	rmd_guid_t backproject_async_clg;
	retval = rmd_codelet_create(
		&backproject_async_clg,     // rmd_guid_t *new_guid
		 backproject_async_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		5,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
// create a codelet for backproject_final function
	rmd_guid_t backproject_final_clg;
	retval = rmd_codelet_create(
		&backproject_final_clg,     // rmd_guid_t *new_guid
		 backproject_final_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		((x2-x1)/BACK_PROJ_ASYNC_BLOCK_SIZE)*((y2-y1)/BACK_PROJ_ASYNC_BLOCK_SIZE)+1, // int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
// create an instance for backproject_final
	rmd_guid_t backproject_final_scg;
	retval = rmd_codelet_sched(
		&backproject_final_scg,		// rmd_guid_t* scheduled codelet's guid
		arg,				// uint64_t arg
		backproject_final_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
RAG_DEF_MACRO_PASS(backproject_final_scg,NULL,NULL,NULL,NULL,in_dbg,slot++);
		for(int m=x1; m<x2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE) {
			for(int n=y1; n<y2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE) {
#ifdef TRACE_LVL_3
xe_printf("////// create an instance for backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
				struct Corners_t *async_corners, *async_corners_ptr, async_corners_lcl; rmd_guid_t async_corners_dbg;
				async_corners = &async_corners_lcl;
				async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct Corners_t));
				async_corners->x1   = m;
				async_corners->x2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE;
				async_corners->y1   = n;
				async_corners->y2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE;
				async_corners->slot = slot++;
				REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct Corners_t);
				rmd_guid_t backproject_async_scg;
				retval = rmd_codelet_sched(
		&backproject_async_scg,		// rmd_guid_t* scheduled codelet's guid
		backproject_final_scg.data,	// uint64_t arg
		backproject_async_clg);		// rmd_guid_t created codelet's guid
				assert(retval==0);

RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,in_dbg,0);	// Xin,platpos
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
				RMD_DB_RELEASE(async_corners_dbg);
			} // for n
		} // for m
		RMD_DB_RELEASE(in_dbg);
		RMD_DB_RELEASE(image_params_dbg);
		RMD_DB_RELEASE(radar_params_dbg);
		RMD_DB_RELEASE(image_dbg);
	} // if F

	bsm_free(corners_ptr,corners_dbg);
#ifdef TRACE_LVL_3
xe_printf("////// leave BackProj_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
