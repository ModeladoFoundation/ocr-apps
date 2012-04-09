#include "common.h"

#include "rag_rmd.h"

#define BLOCK_SIZE 32

rmd_guid_t backproject_async_body(
	rmd_guid_t in_dbg, //Xin[],
	rmd_guid_t corners_dbg,
	rmd_guid_t image_params_dbg,
	rmd_guid_t radar_params_dbg,
	rmd_guid_t image_dbg
) {
#ifdef TRACE
xe_printf("backproject_async_body begin\n");RAG_FLUSH;
#endif
	struct complexData sample, acc, arg;
	struct Inputs in, *in_ptr;
	RMD_DB_MEM(&in_ptr, in_dbg);
	REM_LDX_ADDR(in,in_ptr,struct Inputs);
	struct complexData **Xin;
	Xin     = in.X;

	float **platpos;
	platpos = in.Pt;

	int corners[4], *corners_ptr;
	RMD_DB_MEM(&corners_ptr, corners_dbg);
	REM_LDX_ADDR_SIZE(corners,corners_ptr,4*sizeof(int));
	int x1 = corners[0];
	int x2 = corners[1];
	int y1 = corners[2];
	int y2 = corners[3];

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

#if RAG_PURE_FLOAT
	const float ku = 2.0f*M_PI*radar_params->fc/c_mks_mps;
#else
	const float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif
	for(int m=x1; m<x2; m++) {
#ifdef DEBUG
xe_printf("backproject_async_body m(%d)\n",m);RAG_FLUSH;
#endif
		for(int n=y1; n<y2; n++) {
#ifdef DEBUG
xe_printf("backproject_async_body n(%d)\n",n);RAG_FLUSH;
#endif
			acc.real = 0;
			acc.imag = 0;
			for(int k=0; k<image_params->P3; k++) {
#ifdef DEBUG
xe_printf("backproject_async_body k(%d)\n",k);RAG_FLUSH;
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
xe_printf("backproject_async_body                 R(%f)\n",R);RAG_FLUSH;
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
#if 1
				sincosf(2*ku*R,&arg.imag,&arg.real);
#else
				arg.real = cosf(2*ku*R);
				arg.imag = sinf(2*ku*R);
#endif
				acc.real += sample.real*arg.real - sample.imag*arg.imag;
				acc.imag += sample.real*arg.imag + sample.imag*arg.real;
			} // for k
#ifdef DEBUG
xe_printf("backproject_async_body                 update image[%d][%d]\n",m,n);RAG_FLUSH;
#endif
			struct complexData *image_n;
			image_n = (struct complexData *)RAG_GET_PTR(image+n);
			REM_STX_ADDR(image_n+m,acc,struct complexData);
                } // for n
        } // for m
#ifdef TRACE
xe_printf("backproject_async_body end\n");RAG_FLUSH;
#endif
	bsm_free(corners_ptr,corners_dbg);
	return NULL_GUID;
} // backproject_async_body

rmd_guid_t BackProj( uint64_t free_in_dbg, // if 0 don't free in_dbg;
	rmd_guid_t in_dbg,
	rmd_guid_t corners_dbg,
	rmd_guid_t image_params_dbg,
	rmd_guid_t image_dbg,
	rmd_guid_t radar_params_dbg)
{
	struct Inputs in, *in_ptr;
	RMD_DB_MEM(&in_ptr, in_dbg);
	REM_LDX_ADDR(in,in_ptr,struct Inputs);
	struct complexData **Xin;
	Xin     = in.X;

#if 0
	float **platpos;
	platpos = in.Pt;
#endif

	int corners[4], *corners_ptr;
	RMD_DB_MEM(&corners_ptr, corners_dbg);
	REM_LDX_ADDR_SIZE(corners,corners_ptr,4*sizeof(int));
	int x1 = corners[0];
	int x2 = corners[1];
	int y1 = corners[2];
	int y2 = corners[3];

	struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
	RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
	REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
	image_params = &image_params_lcl;

	struct complexData **image;
	RMD_DB_MEM(&image,      image_dbg);

#if 0
	struct RadarParams *radar_params, *radar_params_ptr, radar_params_lcl;
	RMD_DB_MEM(&radar_params_ptr,  radar_params_dbg);
	REM_LDX_ADDR(radar_params_lcl, radar_params_ptr, struct RadarParams);
	radar_params = &radar_params_lcl;
#endif

	if(image_params->F > 1) {
		fftwf_complex *input, *fft_result, *ifft_result;
		fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

#ifdef TRACE
xe_printf("// BackProj FFTW initialization F = %d\n",image_params->F);RAG_FLUSH;
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
		Xup_data_ptr = (struct complexData*)bsm_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
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

xe_printf("// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is > 1)\n",
			x1, x2, y1, y2);RAG_FLUSH;
		struct Inputs tmp_in, *tmp_in_ptr; rmd_guid_t tmp_in_dbg;
		tmp_in_ptr = bsm_malloc(&tmp_in_dbg,sizeof(struct Inputs));
		tmp_in.Pt = in.Pt;
		tmp_in.Pt_edge_dbg = in.Pt_edge_dbg;
		tmp_in.Pt_data_dbg = in.Pt_data_dbg;
		tmp_in.Tp = NULL;
		tmp_in.Tp_dbg = NULL_GUID;
		tmp_in.X  = Xup;
		tmp_in.X_edge_dbg  = Xup_dbg;
		tmp_in.X_data_dbg  = Xup_data_dbg;;
		REM_STX_ADDR_SIZE(tmp_in_ptr,tmp_in,sizeof(struct Inputs));
		assert( ((x2-x1)%BLOCK_SIZE) == 0);
		assert( ((y2-y1)%BLOCK_SIZE) == 0);
		for(int m=x1; m<x2; m+=BLOCK_SIZE) {
			for(int n=y1; n<y2; n+=BLOCK_SIZE) {
				int corners[4], *corners_ptr; rmd_guid_t corners_dbg;
				corners_ptr = bsm_malloc(&corners_dbg,4*sizeof(int));
				corners[0] = m;
				corners[1] = m+BLOCK_SIZE;
				corners[2] = n;
				corners[3] = n+BLOCK_SIZE;
				REM_STX_ADDR_SIZE(corners_ptr,corners,4*sizeof(int));
				backproject_async_body(
					tmp_in_dbg, // Xup,platpos,
					corners_dbg,
					image_params_dbg,radar_params_dbg,image_dbg);
				RMD_DB_RELEASE(corners_dbg);
			} // for n
		} // for m
		bsm_free(tmp_in_ptr,tmp_in_dbg);
		bsm_free(Xup_data_ptr,Xup_data_dbg); // Xup[]
		bsm_free(Xup,Xup_dbg);
	} else { // if F
xe_printf("// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is = 1)\n",
			x1, x2, y1, y2);RAG_FLUSH;
		assert( ((x2-x1)%BLOCK_SIZE) == 0);
		assert( ((y2-y1)%BLOCK_SIZE) == 0);
		for(int m=x1; m<x2; m+=BLOCK_SIZE) {
			for(int n=y1; n<y2; n+=BLOCK_SIZE) {
				int corners[4], *corners_ptr; rmd_guid_t corners_dbg;
				corners_ptr = bsm_malloc(&corners_dbg,4*sizeof(int));
				corners[0] = m;
				corners[1] = m+BLOCK_SIZE;
				corners[2] = n;
				corners[3] = n+BLOCK_SIZE;
				REM_STX_ADDR_SIZE(corners_ptr,corners,4*sizeof(int));
				backproject_async_body(
					in_dbg, // Xin, platpos,
					corners_dbg, //m,m+BLOCK_SIZE,n,n+BLOCK_SIZE,
					image_params_dbg,radar_params_dbg,image_dbg);
				RMD_DB_RELEASE(corners_dbg);
			} // for n
		} // for m
		RMD_DB_RELEASE(in_dbg);
	} // if F
#if RAG_DIG_SPOT_ON
// RAG ////////////////////////////////////////////////////////////////////////////////////
// This code to free X's data blocks isn't correct yet, a reminder to finish             //
// We need to delete digspot's Xsubimg here, but right now digspot doesn't pass us guids //
// Not supporting digspot so we don't have to worry about it for a while                 //
// RAG ////////////////////////////////////////////////////////////////////////////////////
	if(free_in_dbg && (in.X_data_dbg.data != NULL_GUID.data))bsm_free(RAG_GET_PTR(in.X[0]),in.X_data_dbg);
	if(free_in_dbg && (in.X_edge_dbg.data != NULL_GUID.data))bsm_free(in.X,in.X_edge_dbg);
// RAG /////////////////////////////////////////////////////////////////////////////////
#endif
	if(free_in_dbg)bsm_free(in_ptr,in_dbg);
	bsm_free(corners_ptr,corners_dbg);
	return NULL_GUID;
}
