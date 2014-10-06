#include <assert.h>
#include "common.h"

struct complexData **Xup;

void BackProj(struct complexData *Xin[], float **platpos, int x1, int x2, int y1, int y2, struct ImageParams *image_params, struct complexData *image[], struct RadarParams *radar_params)
{
#ifdef RAG
    fftwf_complex *input = NULL, *fft_result = NULL, *ifft_result = NULL;
    fftwf_plan plan_forward = NULL, plan_backward = NULL; // FFTW plan variables
#else
    fftwf_complex *input, *fft_result, *ifft_result;
    fftwf_plan plan_forward, plan_backward; // FFTW plan variables
#endif

    if(image_params->F > 1) {
     // FFTW initialization
        input = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S3 * sizeof(fftwf_complex));
        fft_result = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S4 * sizeof(fftwf_complex));
        ifft_result = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S4 * sizeof(fftwf_complex));
        plan_forward = fftwf_plan_dft_1d(image_params->S3, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
        plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

        const float scale = (float)image_params->F / (float)image_params->S4;
#ifdef RAG_DEBUG
       printf("RAG scale=F/S4 %f / %f = %f\n",(float)image_params->F,(float)image_params->S4,scale);
#endif

        Xup = (struct complexData**)MALLOC_CHECKED(image_params->P3*sizeof(struct complexData*));
        for(int n=0; n<image_params->P3; n++) {
            Xup[n] = (struct complexData*)MALLOC_CHECKED(image_params->S4*sizeof(struct complexData));
        }

        for(int m=0; m<image_params->P3; m++) {
            memcpy(input, &Xin[m][0], image_params->S3*sizeof(struct complexData));

            fftwf_execute(plan_forward);

            memcpy(
                &fft_result[image_params->S4-(image_params->S3/2)+1][0],
                &fft_result[image_params->S3/2+1][0],
                sizeof(fftwf_complex) * (image_params->S3/2-1));
            // Split Nyquist in half and replicate to the largest negative frequency.  We
            // are here assuming that S3 is an even number.
            fft_result[image_params->S3/2][0] *= 0.5f;
            fft_result[image_params->S3/2][1] *= 0.5f;
            fft_result[image_params->S4-(image_params->S3/2)][0] = fft_result[image_params->S3/2][0];
            fft_result[image_params->S4-(image_params->S3/2)][1] = fft_result[image_params->S3/2][1];
            memset(&fft_result[image_params->S3/2+1][0], 0, sizeof(fftwf_complex)*(image_params->S4-(image_params->S3)-1));

            fftwf_execute(plan_backward);

            for(int n=0; n<image_params->S4; n++) {
                Xup[m][n].real = scale*ifft_result[n][0];
                Xup[m][n].imag = scale*ifft_result[n][1];
            }
        }
    }

    printf("Performing backprojection over Ix2[%d:%d] and Iy2[%d:%d]\n", x1, x2, y1, y2);
#ifdef RAG_PETER
    double ku2   = 2.0*2.0*M_PI*radar_params->fc/SPEED_OF_LIGHT;
    double ku2dr = ku2*image_params->dr;
#ifndef RAG_NEW_BLK_SIZE
#define RAG_NEW_BLK_SIZE 50
#endif
#pragma omp parallel for collapse(2)
    for(int m1=x1; m1<=x2; m1+=RAG_NEW_BLK_SIZE) {
        for(int n1=y1; n1<=y2; n1+=RAG_NEW_BLK_SIZE) {
            int m2 = ((m1+RAG_NEW_BLK_SIZE)<x2)?(m1+RAG_NEW_BLK_SIZE):(x2+1);
            int n2 = ((n1+RAG_NEW_BLK_SIZE)<y2)?(n1+RAG_NEW_BLK_SIZE):(y2+1);
//fprintf(stderr,"m1 %d m2 %d n1 %d n2 %d\n",m1,m2, n1, n2);
            assert((m2-m1) == (n2-n1));
            int blk_size_whole = (n2-n1);
            int blk_size_half = blk_size_whole/2;
            float *A_m = malloc(blk_size_whole*sizeof(float));
            if(A_m == NULL){fprintf(stderr,"Error allocating memory for A_m\n");exit(1);}
            struct complexData *Phi_m = malloc(blk_size_whole*sizeof(struct complexData));
            if(Phi_m == NULL){fprintf(stderr,"Error allocating memory for Phi_m\n");exit(1);}
#ifdef RAG_BLOCKED
            struct complexData **image_ptr = NULL;
            image_ptr = (struct complexData **)malloc((m2-m1)*sizeof(struct complexData *)
                                                     +(m2-m1)*(n2-n1)*sizeof(struct complexData));
            if(image_ptr == NULL) { fprintf(stderr,"Error allocating memory for image_ptr\n");exit(1);}
            struct complexData *image_data_ptr = (struct complexData *)&image_ptr[(m2-m1)];
            if(image_data_ptr == NULL) { fprintf(stderr,"Unable to allocate memory for image_ptr\n");exit(1);}
            for(int m=0;m<(m2-m1);m++) {
              image_ptr[m] = image_data_ptr + m*(n2-n1);
              memcpy( image_ptr[m],&image[m+m1][n1],(n2-n1)*sizeof(struct complexData));
            }
#endif
	    for(int k=0; k<image_params->P3; k++) {
		double zr_mid  =                                   - platpos[k][2]; // Z
		double zr_mid2 = zr_mid * zr_mid;
		double yr_mid = image_params->yr[m1+blk_size_half] - platpos[k][1]; // Y
		double yr_mid2 = yr_mid * yr_mid;
		double sqrt_arg = zr_mid2 + yr_mid2;
		double xr_mid = image_params->xr[n1+blk_size_half] - platpos[k][0]; // X
		double xr_mid2 = xr_mid * xr_mid;
		double R_mid = sqrt (sqrt_arg + xr_mid2);
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
		for(int n=1;n<blk_size_whole;n++) {
		    A_m[n] = A_m[n-1] + (ax+bx) + (2*(n-1)-blk_size_whole)*bx;
		}
		float ux = ku2dr*ax;
		float uy = ku2dr*ay;
		float vx = ku2dr*bx;
		float vy = ku2dr*by;
		float wxy = ku2dr*cxy;
		struct complexData WXY;
#ifdef RAG_SINCOS
		sincosf(wxy, &WXY.imag, &WXY.real);
#else
		WXY.real = cosf(wxy);
		WXY.imag = sinf(wxy);
#endif
		struct complexData VX2;
#ifdef RAG_SINCOS
		sincosf(2*vx, &VX2.imag, &VX2.real);
#else
		VX2.real = cosf(2*vx);
		VX2.imag = sinf(2*vx);
#endif
		struct complexData VY2;
#ifdef RAG_SINCOS
		sincosf(2*vy, &VY2.imag, &VY2.real);
#else
		VY2.real = cosf(2*vy);
		VY2.imag = sinf(2*vy);
#endif
		struct complexData UX_VX;
#ifdef RAG_SINCOS
		sincosf((ux+(1-blk_size_whole)*vx), &UX_VX.imag, &UX_VX.real);
#else
		UX_VX.real = cosf(ux+(1-blk_size_whole)*vx);
		UX_VX.imag = sinf(ux+(1-blk_size_whole)*vx);
#endif
#ifdef RAG_SINCOS
		sincosf((-blk_size_half*ux + blk_size_whole*blk_size_whole/4*vx), &(Phi_m[0].imag), &(Phi_m[0].real));
#else
		Phi_m[0].real = cosf(-blk_size_half*ux + blk_size_whole*blk_size_whole/4*vx);
		Phi_m[0].imag = sinf(-blk_size_half*ux + blk_size_whole*blk_size_whole/4*vx);
#endif
		for(int n=1;n<blk_size_whole;n++) {
		    struct complexData tmp;
                    Phi_m[n].real = Phi_m[n-1].real*UX_VX.real - Phi_m[n-1].imag*UX_VX.imag;
                    Phi_m[n].imag = Phi_m[n-1].real*UX_VX.imag + Phi_m[n-1].imag*UX_VX.real;
                    tmp.real = UX_VX.real*VX2.real - UX_VX.imag*VX2.imag;
                    tmp.imag = UX_VX.real*VX2.imag + UX_VX.imag*VX2.real;
                    UX_VX.real = tmp.real;
                    UX_VX.imag = tmp.imag;
		}
		double theta_mid = ku2*R_mid;
		double arg_mid = theta_mid - blk_size_half*uy + blk_size_whole*blk_size_whole/4*vy;
		struct complexData Psi_n;
#ifdef RAG_SINCOS
		double Psi_n_imag, Psi_n_real;
		sincos((arg_mid-2.0*M_PI*round(arg_mid/2.0/M_PI)), &Psi_n_imag, &Psi_n_real); // RAG -- Changed precision to match latest code from Dan Campbell
		Psi_n.imag = (float)Psi_n_imag;
		Psi_n.real = (float)Psi_n_real;
#else
		Psi_n.real = cos(arg_mid-2.0*M_PI*round(arg_mid/2.0/M_PI)); // RAG -- Changed precision to match latest code from Dan Campbell
		Psi_n.imag = sin(arg_mid-2.0*M_PI*round(arg_mid/2.0/M_PI)); // RAG -- Changed precision to match latest code from Dan Campbell
#endif
		struct complexData Gamma_m;
#ifdef RAG_SINCOS
		sincosf((-blk_size_half*wxy), &(Gamma_m.imag), &(Gamma_m.real));
#else
		Gamma_m.real = cosf(-blk_size_half*wxy);
		Gamma_m.imag = sinf(-blk_size_half*wxy);
#endif
		struct complexData Gamma_m_n_base_d;
#ifdef RAG_SINCOS
		sincosf((uy + (1-blk_size_whole)*vy - blk_size_half*wxy), &(Gamma_m_n_base_d.imag), &(Gamma_m_n_base_d.real));
#else
		Gamma_m_n_base_d.real = cosf(uy + (1-blk_size_whole)*vy - blk_size_half*wxy);
		Gamma_m_n_base_d.imag = sinf(uy + (1-blk_size_whole)*vy - blk_size_half*wxy);
#endif
		struct complexData Gamma_m_n_base;
#ifdef RAG_SINCOS
		sincosf((blk_size_whole*blk_size_whole/4*wxy), &(Gamma_m_n_base.imag), &(Gamma_m_n_base.real));
#else
		Gamma_m_n_base.real = cosf(blk_size_whole*blk_size_whole/4*wxy);
		Gamma_m_n_base.imag = sinf(blk_size_whole*blk_size_whole/4*wxy);
#endif
#ifdef RAG_BLOCKED
		for(int m=0; m<(m2-m1); m++) {
                    float Bm = bin_mid + (m-blk_size_half)*(ay+(m-blk_size_half)*by);
                    float Cm = (m-blk_size_half)*cxy;
#else
		for(int m=m1; m<m2; m++) {
                    float Bm = bin_mid + ((m-m1)-blk_size_half)*(ay+((m-m1)-blk_size_half)*by);
                    float Cm = ((m-m1)-blk_size_half)*cxy;
#endif
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
#ifdef RAG_BLOCKED
                    for(int n=0; n<(n2-n1); n++) {
			float bin = A_m[n] + Bm + (n-blk_size_half)*Cm;
#else
                    for(int n=n1; n<n2; n++) {
			float bin = A_m[(n-n1)] + Bm + ((n-n1)-blk_size_half)*Cm;
#endif
			struct complexData sample;
			if(bin >= 0.0f && bin < (float)(image_params->S4-1)) {
				struct complexData left,right;
				int int_bin = (int)floorf(bin);
				float w = bin - int_bin;
				left  = Xin[k][int_bin+0];
				right = Xin[k][int_bin+1];
				sample.real = (1-w)*left.real + w*right.real;
				sample.imag = (1-w)*left.imag + w*right.imag;
			} else {
				sample.real = 0.0f;
				sample.imag = 0.0f;
			}
			struct complexData arg;
#ifdef RAG_BLOCKED
			arg.real = Phi_m[n].real*Gamma_m_n.real - Phi_m[n].imag*Gamma_m_n.imag;
			arg.imag = Phi_m[n].real*Gamma_m_n.imag + Phi_m[n].imag*Gamma_m_n.real;
#else
			arg.real = Phi_m[(n-n1)].real*Gamma_m_n.real - Phi_m[(n-n1)].imag*Gamma_m_n.imag;
			arg.imag = Phi_m[(n-n1)].real*Gamma_m_n.imag + Phi_m[(n-n1)].imag*Gamma_m_n.real;
#endif
			struct complexData tmp;
			tmp.real = Gamma_m_n.real*Gamma_m.real - Gamma_m_n.imag*Gamma_m.imag;
			tmp.imag = Gamma_m_n.real*Gamma_m.imag + Gamma_m_n.imag*Gamma_m.real;
			Gamma_m_n.real = tmp.real;
			Gamma_m_n.imag = tmp.imag;

			tmp.real = sample.real*arg.real - sample.imag*arg.imag;
			tmp.imag = sample.real*arg.imag + sample.imag*arg.real;
#ifdef RAG_BLOCKED
			image_ptr[m][n].real += tmp.real;
			image_ptr[m][n].imag += tmp.imag;
#else
			image[m][n].real += tmp.real;
			image[m][n].imag += tmp.imag;
#endif
		    } // for n
		    tmp.real = Gamma_m.real*WXY.real - Gamma_m.imag*WXY.imag;
		    tmp.imag = Gamma_m.real*WXY.imag + Gamma_m.imag*WXY.real;
		    Gamma_m.real = tmp.real;
		    Gamma_m.imag = tmp.imag;
		} // for m
	    } // for k
#ifdef RAG_BLOCKED
	    for(int m=0;m<(m2-m1);m++) {
             //xe_printf("I<i %d\n",m);
               memcpy(&image[m+m1][n1],image_ptr[m],(n2-n1)*sizeof(struct complexData));
	    }
            free(image_ptr);
#endif
            free(Phi_m);
            free(A_m);
	} // n1 y2
    } // m1 x2
#else // RAG_PETER
#ifdef RAG_PURE_FLOAT
    double ku = 2.0f*M_PI*radar_params->fc/SPEED_OF_LIGHT;
#else
    double ku = 2.0*M_PI*radar_params->fc/SPEED_OF_LIGHT;
#endif
    int pixel_cnt=0;
    #pragma omp parallel for
    for(int m=x1; m<=x2; m++)
    {
        for(int n=y1; n<=y2; n++)
        {
#ifdef RAG
            (void)__sync_fetch_and_add(&pixel_cnt,1);
#else
            #pragma omp critical
            pixel_cnt++;
#if 0
            printf("%d%%\r", 100*pixel_cnt/((y2-y1+1)*(x2-x1+1)));
            fflush(stdout);
#endif
#endif
	    struct complexData acc;
            acc.real = 0;
            acc.imag = 0;
            for(int k=0; k<image_params->P3; k++)
            {
#ifdef RAG
  	        double x = (double)image_params->xr2[m] - platpos[k][0];
	        double y = (double)image_params->yr2[n] - platpos[k][1];
	        double z = (double)               platpos[k][2];
	        double R = sqrt( x*x + y*y + z*z ); // RAG -- Changed precision to match latest code from Dan Campbell
#else

                double R = sqrt(
                    ((double)image_params->xr2[m]-platpos[k][0])*((double)image_params->xr2[m]-platpos[k][0]) +
                    ((double)image_params->yr2[n]-platpos[k][1])*((double)image_params->yr2[n]-platpos[k][1]) +
                    (double)platpos[k][2]*platpos[k][2] );
#endif

                float bin;
                if(image_params->TF > 1) {
                    bin = (R-radar_params->R0_prime)/image_params->dr;
                }
                else {
                    bin = (R-radar_params->R0)/image_params->dr;
                }

	        struct complexData sample;
                if(image_params->F > 1) {
                    if(bin >= 0 && bin < image_params->S4-1) {
#ifdef RAG
	                struct complexData left,right;
          	        int int_bin = (int)floorf(bin);
	                float w = bin - int_bin;
          	        left  = Xup[k][int_bin+0];
          	        right = Xup[k][int_bin+1];
          	        sample.real = (1-w)*left.real + w*right.real;
          	        sample.imag = (1-w)*left.imag + w*right.imag;
#else
                        w = bin - floorf(bin);
                        sample.real = (1-w)*Xup[k][(int)floorf(bin)].real + w*Xup[k][(int)floorf(bin)+1].real;
                        sample.imag = (1-w)*Xup[k][(int)floorf(bin)].imag + w*Xup[k][(int)floorf(bin)+1].imag;
#endif
                    } else {
                        sample.real = 0.0;
                        sample.imag = 0.0;
                    }
                } else {
                    if(bin >= 0 && bin < image_params->S4-1) {
#ifdef RAG
	                struct complexData left,right;
          	        int int_bin = (int)floorf(bin);
	                float w = bin - int_bin;
          	        left  = Xin[k][int_bin+0];
          	        right = Xin[k][int_bin+1];
          	        sample.real = (1-w)*left.real + w*right.real;
          	        sample.imag = (1-w)*left.imag + w*right.imag;
#else
                        w = bin - floorf(bin);
                        sample.real = (1-w)*Xin[k][(int)floorf(bin)].real + w*Xin[k][(int)floorf(bin)+1].real;
                        sample.imag = (1-w)*Xin[k][(int)floorf(bin)].imag + w*Xin[k][(int)floorf(bin)+1].imag;
#endif
                    } else {
                        sample.real = 0.0;
                        sample.imag = 0.0;
                    }
                }

	        struct complexData arg;
#ifdef RAG_SINCOS
                double arg_imag, arg_real;
	        sincos(2.0*ku*R,&arg_imag,&arg_real); // RAG -- Changed precision to match latest code from Dan Campbell
                arg.imag = (float)arg_imag;
                arg.real = (float)arg_real;
#else
                arg.real = cos(2.0*ku*R);
                arg.imag = sin(2.0*ku*R);
#endif
                acc.real += sample.real*arg.real - sample.imag*arg.imag;
                acc.imag += sample.real*arg.imag + sample.imag*arg.real;
            }
            image[n][m].real = acc.real;
            image[n][m].imag = acc.imag;
        }
    }
#endif // RAG_PETER

    if(image_params->F > 1)
    {
        for(int n=0; n<image_params->P3; n++) {
            FREE_AND_NULL(Xup[n]);
        }
        FREE_AND_NULL(Xup);

    // Free memory and deallocate plan
        fftwf_free(input);
        fftwf_free(fft_result);
        fftwf_free(ifft_result);
        fftwf_destroy_plan(plan_forward);
        fftwf_destroy_plan(plan_backward);
    }
}
