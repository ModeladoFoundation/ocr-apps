#include "common.h"

struct complexData **Xup;

void backproject_async_body(
	struct corners_t *corners,
	struct complexData *Xin[],
	float**platpos,
	struct ImageParams *image_params,
	struct RadarParams *radar_params,
	struct complexData *image[]
) {
	struct complexData sample;
	struct complexData acc;
	struct complexData arg;
        float R, bin, w;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
#ifdef RAG_PURE_FLOAT
	float ku = 2.0f*M_PI*radar_params->fc/c_mks_mps;
#else
	float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif
#ifdef DEBUG_RAG
fprintf(stderr,"m1 %d m2 %d n1 %d n2 %d ku %f\n",m1,m2,n1,n2,ku);fflush(stderr);
#endif
	for(int m=m1; m<m2; m++) {
		for(int n=n1; n<n2; n++) {
			
			acc.real = 0;
			acc.imag = 0;
			for(int k=0; k<image_params->P3; k++) {
				R = sqrtf( 
		   (image_params->xr[n]-platpos[k][0])*(image_params->xr[n]-platpos[k][0])
		 + (image_params->yr[m]-platpos[k][1])*(image_params->yr[m]-platpos[k][1])
		 + platpos[k][2]*platpos[k][2] );
				
				if(image_params->TF > 1) {
					bin = (R-radar_params->R0_prime)/image_params->dr;
				}
				else {
					bin = (R-radar_params->R0)/image_params->dr;
				}
if(image_params->F > 1) {
					if(bin >= 0 && bin < image_params->S4-1) {
						w = bin - floorf(bin);
						sample.real = (1-w)*Xup[k][(int)floorf(bin)].real + w*Xup[k][(int)floorf(bin)+1].real;
						sample.imag = (1-w)*Xup[k][(int)floorf(bin)].imag + w*Xup[k][(int)floorf(bin)+1].imag;
					}
					else if (bin > image_params->S4-1) {
						sample.real = Xup[k][image_params->S4-1].real;
						sample.imag = Xup[k][image_params->S4-1].imag;
					}
					else {
						sample.real = 0.0;
						sample.imag = 0.0;
					}
				}
				else {
					if(bin >= 0 && bin < image_params->S4-1) {
						w = bin - floorf(bin);
						sample.real = (1-w)*Xin[k][(int)floorf(bin)].real + w*Xin[k][(int)floorf(bin)+1].real;
						sample.imag = (1-w)*Xin[k][(int)floorf(bin)].imag + w*Xin[k][(int)floorf(bin)+1].imag;
					}
					else if (bin > image_params->S4-1) {
						sample.real = Xin[k][image_params->S4-1].real;
						sample.imag = Xin[k][image_params->S4-1].imag;
					}
					else {
						sample.real = 0.0;
						sample.imag = 0.0;
					}
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
			image[m][n].real = acc.real;
			image[m][n].imag = acc.imag;
                } // for n
	} // for m
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
		input = (fftwf_complex*)fftwf_malloc(image_params->S3 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		ifft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
		plan_forward = fftwf_plan_dft_1d(image_params->S3, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
		plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);
		float scale = 1/(float)image_params->S4;

		Xup = (struct complexData**)malloc(image_params->P3*sizeof(struct complexData*));
		if(Xup == NULL) {
			fprintf(stderr,"Error allocating memory for Xup.\n");
			exit(1);
		}
		for(int n=0; n<image_params->P3; n++) {
			Xup[n] = (struct complexData*)malloc(image_params->S4*sizeof(struct complexData));
			if (Xup[n] == NULL) {
				fprintf(stderr,"Error allocating memory for Xup.\n");
				exit(1);
			}
		}

		for(int m=0; m<image_params->P3; m++)
		{
			memcpy(input, &Xin[m][0], image_params->S3*sizeof(struct complexData));

			fftwf_execute(plan_forward);

			memset(&fft_result[image_params->S3][0], 0, (image_params->S4-image_params->S3)*sizeof(fftwf_complex));
			
			fftwf_execute(plan_backward);			

			for(int n=0; n<image_params->S4; n++)
			{
				Xup[m][n].real = scale*ifft_result[n][0];
				Xup[m][n].imag = scale*ifft_result[n][1];
			}
		}
		// Free memory and deallocate plan
		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_free(ifft_result);
		fftwf_destroy_plan(plan_forward);
		fftwf_destroy_plan(plan_backward);
	}

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
#if 1
            async     IN(corners,Xin,platpos,image_params,radar_params,image)
#endif
  backproject_async_body(corners,Xin,platpos,image_params,radar_params,image);
		} // for n
	} // for m
} // finish

	return;
}
