#include "common.h"

void backproject_async_body(
	struct corners_t *corners,
	struct complexData *Xin[],
	float**platpos,
	struct ImageParams *image_params,
	struct RadarParams *radar_params,
	struct complexData *image[]
) {
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
#ifdef DEBUG_RAG
fprintf(stderr,"m1 %d m2 %d n1 %d n2 %d ku %f\n",m1,m2,n1,n2,ku);fflush(stderr);
#endif
#ifdef RAG_PETER_DIST_AND_TRIG
	double const Pi = 3.1415926535897932384626433832795029l;
	double ku2   = 2.0*2.0*Pi*radar_params->fc/c_mks_mps;
	double ku2dr = ku2*image_params->dr;

	assert((m2-m1) == (n2-n1));
	int blk_size_whole = (n2-n1);
	int blk_size_half = blk_size_whole/2;
	float *A_m = malloc(blk_size_whole*sizeof(float));
	if(A_m == NULL){fprintf(stderr,"error malloc of A_m\n");fflush(stderr);exit(1);}
	struct complexData *Phi_m = malloc(blk_size_whole*sizeof(struct complexData));
	if(Phi_m == NULL){fprintf(stderr,"error malloc of Phi_m\n");fflush(stderr);exit(1);}
	for(int k=0; k<image_params->P3; k++) {
// compute distance to R_mid
		double zr_mid  = 0.0f - platpos[k][2]; // Z
		double zr_mid2 = zr_mid * zr_mid;
		double yr_mid = image_params->yr[m1+blk_size_half] - platpos[k][1]; // Y
		double yr_mid2 = yr_mid * yr_mid;
		double sqrt_arg = zr_mid2 + yr_mid2;
		double xr_mid = image_params->xr[n1+blk_size_half] - platpos[k][0]; // X
		double xr_mid2 = xr_mid * xr_mid;
		double R_mid = sqrt (sqrt_arg + xr_mid2);
// compute coefficients for computing bin
		float ax  =  xr_mid/R_mid;
		float ay  =  yr_mid/R_mid;
		float bx  =  image_params->dr*(1-ax*ax)/2/R_mid;
		float by  =  image_params->dr*(1-ay*ay)/2/R_mid;
		float cxy = -image_params->dr*ax*ay/R_mid;
		float bin_mid;
		if(image_params->TF > 1) {
			bin_mid = (R_mid-radar_params->R0_prime)/image_params->dr;
		} else {
			bin_mid = (R_mid-radar_params->R0)/image_params->dr;
		}
		A_m[0] = -blk_size_half*(ax - blk_size_half*bx);
		for( int n=1;n<blk_size_whole;n++) {
			A_m[n] = A_m[n-1] + (ax+bx) + (2*(n-1)-blk_size_whole)*bx;
		}
// compute cofficients for computing arg
		float ux = ku2dr*ax;
		float uy = ku2dr*ay;
		float vx = ku2dr*bx;
		float vy = ku2dr*by;
		float wxy = ku2dr*cxy;
		struct complexData WXY;
		WXY.real = cosf(wxy);
		WXY.imag = sinf(wxy);
		struct complexData VX2;
		VX2.real = cosf(2*vx);
		VX2.imag = sinf(2*vx);
		struct complexData VY2;
		VY2.real = cosf(2*vy);
		VY2.imag = sinf(2*vy);
		struct complexData UX_VX;
		UX_VX.real = cosf(ux+(1-blk_size_whole)*vx);
		UX_VX.imag = sinf(ux+(1-blk_size_whole)*vx);
// compute Phi_m
		Phi_m[0].real = cosf(-blk_size_half*ux + blk_size_whole*blk_size_whole/4*vx);
		Phi_m[0].imag = sinf(-blk_size_half*ux + blk_size_whole*blk_size_whole/4*vx);
		for( int n=1;n<blk_size_whole;n++) {
			struct complexData tmp;
			Phi_m[n].real = Phi_m[n-1].real*UX_VX.real - Phi_m[n-1].imag*UX_VX.imag;
			Phi_m[n].imag = Phi_m[n-1].real*UX_VX.imag + Phi_m[n-1].imag*UX_VX.real;
			tmp.real = UX_VX.real*VX2.real - UX_VX.imag*VX2.imag;
			tmp.imag = UX_VX.real*VX2.imag + UX_VX.imag*VX2.real;
			UX_VX.real = tmp.real;
			UX_VX.imag = tmp.imag;
		}
// compute Psi_n
		double theta_mid = ku2*R_mid;
		double arg_mid = theta_mid - blk_size_half*uy + blk_size_whole*blk_size_whole/4*vy;
		struct complexData Psi_n;
		Psi_n.real = cosf(arg_mid-2*Pi*round(arg_mid/2/Pi));
		Psi_n.imag = sinf(arg_mid-2*Pi*round(arg_mid/2/Pi));
// compute Gamma_m
		struct complexData Gamma_m;
		Gamma_m.real = cosf(-blk_size_half*wxy);
		Gamma_m.imag = sinf(-blk_size_half*wxy);
		struct complexData Gamma_m_n_base_d;
		Gamma_m_n_base_d.real = cosf(uy + (1-blk_size_whole)*vy - blk_size_half*wxy);
		Gamma_m_n_base_d.imag = sinf(uy + (1-blk_size_whole)*vy - blk_size_half*wxy);
		struct complexData Gamma_m_n_base;
		Gamma_m_n_base.real = cosf(blk_size_whole*blk_size_whole/4*wxy);
		Gamma_m_n_base.imag = sinf(blk_size_whole*blk_size_whole/4*wxy);
		for(int m=m1; m<m2; m++) {
			float Bm = bin_mid + ((m-m1)-blk_size_half)*(ay+((m-m1)-blk_size_half)*by);
			float Cm = ((m-m1)-blk_size_half)*cxy;
			struct complexData tmp;
			struct complexData Gamma_m_n;
			Gamma_m_n.real = Psi_n.real*Gamma_m_n_base.real - Psi_n.imag*Gamma_m_n_base.imag;
			Gamma_m_n.imag = Psi_n.real*Gamma_m_n_base.imag + Psi_n.imag*Gamma_m_n_base.real;
			tmp.real = Gamma_m_n_base.real*Gamma_m_n_base_d.real - Gamma_m_n_base.imag*Gamma_m_n_base_d.imag;
			tmp.imag = Gamma_m_n_base.real*Gamma_m_n_base_d.imag + Gamma_m_n_base.imag*Gamma_m_n_base_d.real;
			Gamma_m_n_base.real = tmp.real;
			Gamma_m_n_base.imag = tmp.imag;
			tmp.real = Gamma_m_n_base_d.real*VY2.real - Gamma_m_n_base_d.imag*VY2.imag;
			tmp.imag = Gamma_m_n_base_d.real*VY2.imag + Gamma_m_n_base_d.imag*VY2.real;
			Gamma_m_n_base_d.real = tmp.real;
			Gamma_m_n_base_d.imag = tmp.imag;
			for(int n=n1; n<n2; n++) {
        			float bin = A_m[(n-n1)] + Bm + ((n-n1)-blk_size_half)*Cm;
				struct complexData sample;
				if(bin >= 0 && bin < image_params->S4-1) {
					float w = bin - floorf(bin);
					sample.real = (1-w)*Xin[k][(int)floorf(bin)].real + w*Xin[k][(int)floorf(bin)+1].real;
					sample.imag = (1-w)*Xin[k][(int)floorf(bin)].imag + w*Xin[k][(int)floorf(bin)+1].imag;
				} else if (bin > image_params->S4-1) {
					sample.real = Xin[k][image_params->S4-1].real;
					sample.imag = Xin[k][image_params->S4-1].imag;
				} else {
					sample.real = 0.0;
					sample.imag = 0.0;
				}
				struct complexData arg;
				arg.real = Phi_m[n-n1].real*Gamma_m_n.real - Phi_m[n-n1].imag*Gamma_m_n.imag;
				arg.imag = Phi_m[n-n1].real*Gamma_m_n.imag + Phi_m[n-n1].imag*Gamma_m_n.real;
				struct complexData tmp;
				tmp.real = Gamma_m_n.real*Gamma_m.real - Gamma_m_n.imag*Gamma_m.imag;
				tmp.imag = Gamma_m_n.real*Gamma_m.imag + Gamma_m_n.imag*Gamma_m.real;
				Gamma_m_n.real = tmp.real;
				Gamma_m_n.imag = tmp.imag;

				tmp.real = sample.real*arg.real - sample.imag*arg.imag;
				tmp.imag = sample.real*arg.imag + sample.imag*arg.real;
				image[m][n].real += tmp.real;
				image[m][n].imag += tmp.imag;
			} // for n
			tmp.real = Gamma_m.real*WXY.real - Gamma_m.imag*WXY.imag;
			tmp.imag = Gamma_m.real*WXY.imag + Gamma_m.imag*WXY.real;
			Gamma_m.real = tmp.real;
			Gamma_m.imag = tmp.imag;
		} // for m
	} // for k
	free(Phi_m);
	free(A_m);
#else
#ifdef RAG_PURE_FLOAT
	float ku = 2.0f*M_PI*(radar_params->fc/c_mks_mps);
#else
	float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif
	for(int m=m1; m<m2; m++) {
		for(int n=n1; n<n2; n++) {
			struct complexData acc = { .real = 0; .imag = 0 };
			for(int k=0; k<image_params->P3; k++) {
				float bin;
				float xr_delta = image_params->xr[n]-platpos[k][0];
				float yr_delta = image_params->yr[m]-platpos[k][1];
				float zr_delta =               0.0f -platpos[k][2];
				float R = sqrtf	( xr_delta*xr_delta
						+ yr_delta*yr_delta
						+ zr_delta*zr_delta );
				if(image_params->TF > 1) {
					bin = (R-radar_params->R0_prime)/image_params->dr;
				} else {
					bin = (R-radar_params->R0)/image_params->dr;
				}
				struct complexData sample;
				if(bin >= 0 && bin < image_params->S4-1) {
					float w = bin - floorf(bin);
					sample.real = (1-w)*Xin[k][(int)floorf(bin)].real + w*Xin[k][(int)floorf(bin)+1].real;
					sample.imag = (1-w)*Xin[k][(int)floorf(bin)].imag + w*Xin[k][(int)floorf(bin)+1].imag;
				} else if (bin == image_params->S4-1) {
					sample.real = Xin[k][image_params->S4-1].real;
					sample.imag = Xin[k][image_params->S4-1].imag;
				} else {
					sample.real = 0.0;
					sample.imag = 0.0;
				}
				struct complexData arg;
#ifdef RAG_SINCOS
				sincosf(2*ku*R,&arg.imag,&arg.real);
#else
				arg.real = cosf(2*ku*R);
				arg.imag = sinf(2*ku*R);
#endif
				acc.real += sample.real*arg.real - sample.imag*arg.imag;
				acc.imag += sample.real*arg.imag + sample.imag*arg.real;
			} // for k
			image[m][n].real = acc.real;
			image[m][n].imag = acc.imag;
                } // for n
	} // for m
#endif
	free(corners);
} // backproject_async_body

void BackProj(struct complexData *Xin[], float **platpos, int n1, int n2, int m1, int m2, struct ImageParams *image_params, struct complexData *image[], struct RadarParams *radar_params)
{
	fftwf_complex *input, *fft_result, *ifft_result;
	fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

	// FFTW initialization
#ifdef DEBUG_RAG
fprintf(stderr,"BackProj FFTW initialization F = %d\n",image_params->F);fflush(stderr);
#endif

	if(image_params->F > 1) {
		struct complexData **Xup;
		input = (fftwf_complex*)fftwf_malloc(image_params->S3 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		ifft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		plan_forward = fftwf_plan_dft_1d(image_params->S3, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
		plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);
		float scale = 1/(float)image_params->S4;

		Xup = (struct complexData**)malloc(image_params->P3*sizeof(struct complexData*));
		if(Xup == NULL) {
			fprintf(stderr,"Error allocating edge vector for Xup.\n");
			exit(1);
		}
		struct complexData *Xup_data_ptr;
		Xup_data_ptr = (struct complexData*)malloc(image_params->P3*image_params->S4*sizeof(struct complexData));
		if (Xup_data_ptr == NULL) {
			fprintf(stderr,"Error allocating data memory for Xup.\n");
			exit(1);
		}
		for(int n=0; n<image_params->P3; n++) {
			Xup[n] = Xup_data_ptr + n*image_params->S4;
		} // for n

		for(int m=0; m<image_params->P3; m++) {
			memcpy(input, &Xin[m][0], image_params->S3*sizeof(struct complexData));

			fftwf_execute(plan_forward);

			memset(&fft_result[image_params->S3][0], 0, (image_params->S4-image_params->S3)*sizeof(fftwf_complex));
			
			fftwf_execute(plan_backward);			

			for(int n=0; n<image_params->S4; n++) {
				Xup[m][n].real = scale*ifft_result[n][0];
				Xup[m][n].imag = scale*ifft_result[n][1];
			} // for n
		} // for m
		// Free memory and deallocate plan
		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_free(ifft_result);
		fftwf_destroy_plan(plan_forward);
		fftwf_destroy_plan(plan_backward);

		fprintf(stderr,"Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n", n1, n2-1, m1, m2-1);fflush(stderr);
		int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
		int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
		assert(((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M)==0);
		assert(((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N)==0);
finish {
		for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
			for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
				struct corners_t *corners;
				corners = malloc(sizeof(struct corners_t));
				corners->m1 = m;
				corners->m2 = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
				corners->n1 = n;
				corners->n2 = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
		      async   IN(corners,Xup,platpos,image_params,radar_params,image)
	  backproject_async_body(corners,Xup,platpos,image_params,radar_params,image);
			} // for n
		} // for m
} // finish
		free(Xup_data_ptr);
		free(Xup);
	} else {
		fprintf(stderr,"Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n", n1, n2-1, m1, m2-1);fflush(stderr);
		int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
		int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
		assert(((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M)==0);
		assert(((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N)==0);
finish {
		for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
			for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
				struct corners_t *corners;
				corners = malloc(sizeof(struct corners_t));
				corners->m1 = m;
				corners->m2 = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
				corners->n1 = n;
				corners->n2 = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
		      async   IN(corners,Xin,platpos,image_params,radar_params,image)
	  backproject_async_body(corners,Xin,platpos,image_params,radar_params,image);
			} // for n
		} // for m
} // finish
	} // endif

	return;
}
