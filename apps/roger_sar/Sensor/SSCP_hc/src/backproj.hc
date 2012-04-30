#include "common.h"

#define BLOCK_SIZE 64

struct complexData **Xup;

void async_body_backproject(
	struct complexData *Xin[],
	float**platpos,
	int x1, int x2, int y1, int y2,
	struct ImageParams *image_params,
	struct RadarParams *radar_params,
	float ku,
	struct complexData *image[]
) {
#ifdef RAG_DEBUG
fprintf(stderr,"x1 %d x2 %d y1 %d y2 %d ku %f\n",x1,x2,y1,y2,ku);fflush(stderr);
#endif
	struct complexData sample;
	struct complexData acc;
	struct complexData arg;
        float R, bin, w;
	for(int m=x1; m<=x2; m++) {
		for(int n=y1; n<=y2; n++) {
			
			acc.real = 0;
			acc.imag = 0;
			for(int k=0; k<image_params->P3; k++)
			{
				R = sqrtf( 
		   (image_params->xr[m]-platpos[k][0])*(image_params->xr[m]-platpos[k][0])
		 + (image_params->yr[n]-platpos[k][1])*(image_params->yr[n]-platpos[k][1])
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
			image[n][m].real = acc.real;
			image[n][m].imag = acc.imag;
                } // for n
       } // for m
} // async_body_backproject

void BackProj(struct complexData *Xin[], float **platpos, int x1, int x2, int y1, int y2, struct ImageParams *image_params, struct complexData *image[], struct RadarParams *radar_params)
{
#if RAG_PURE_FLOAT
	float ku = 2.0f*((float)(M_PI*radar_params->fc/c_mks_mps);
#else
	float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif
	fftwf_complex *input, *fft_result, *ifft_result;
	fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

	// FFTW initialization
#ifdef RAG_DEBUG
fprintf(stderr,"BackProj FFTW initialization F = %d\n",image_params->F);fflush(stderr);
#endif
	input = (fftwf_complex*)fftwf_malloc(image_params->S3 * sizeof(fftwf_complex));
	fft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
	ifft_result = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
	plan_forward = fftwf_plan_dft_1d(image_params->S3, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
	plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

	if(image_params->F > 1)
	{
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
	}

	fprintf(stderr,"Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n", x1, x2, y1, y2);fflush(stderr);
	assert((x2-x1+1)%BLOCK_SIZE ==0);
	assert((y2-y1+1)%BLOCK_SIZE ==0);
        finish {
          for(int m=x1; m<=x2; m+=BLOCK_SIZE) {
            for(int n=y1; n<=y2; n+=BLOCK_SIZE) {
            async     IN(Xin,platpos,m,n,image_params,radar_params,ku,image)
  async_body_backproject(Xin,platpos,m,m+BLOCK_SIZE-1,n,n+BLOCK_SIZE-1,image_params,radar_params,ku,image);
            } // for n
          } // for m
        } // finish

	// Free memory and deallocate plan
	fftwf_free(input);
	fftwf_free(fft_result);
	fftwf_free(ifft_result);
	fftwf_destroy_plan(plan_forward);
	fftwf_destroy_plan(plan_backward);
}
