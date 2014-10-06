#include "common.h"

void FormImage(struct DigSpotVars *dig_spot, struct ImageParams *image_params, struct Inputs *in, struct complexData **curImage2, struct RadarParams *radar_params)
{
	int n, i, j;
	float xc, yc;
	struct complexData **Xsubimg;
	fftwf_plan plan_forward;
	fftwf_complex *input, *fft_result;

	for(n=0; n<image_params->Iy2; n++) {
		memset(&curImage2[n][0], 0, image_params->Ix2*sizeof(struct complexData));
	}

	if(image_params->TF > 1)
	{
		// FFTW initialization
		input = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S1 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S1 * sizeof(fftwf_complex));
		plan_forward = fftwf_plan_dft_1d(image_params->S1, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

		for(i=0; i<image_params->P1; i++)
		{
			// Copy data from one pulse to FFT input array (first and second halves swapped)
			for(j=(int)ceilf(image_params->S1/2), n=0; j<image_params->S1; j++, n++) {
				input[n][0] = in->X[i][j].real;
				input[n][1] = in->X[i][j].imag;
			}
			for(j=0; j<(int)ceilf(image_params->S1/2); j++, n++) {
				input[n][0] = in->X[i][j].real;
				input[n][1] = in->X[i][j].imag;
			}

			// Perform FFT
			fftwf_execute(plan_forward);

			// Copy FFT results to X2
			memcpy(&dig_spot->X2[i][0], fft_result, image_params->S1*sizeof(struct complexData));
		}

		for(i=0; i<image_params->TF; i++)
		{
			for(j=0; j<image_params->TF; j++)
			{
				// Subimage center
				xc = image_params->xr2[(image_params->Sx2-1)/2 + i*image_params->Sx2];
				yc = image_params->yr2[(image_params->Sy2-1)/2 + j*image_params->Sy2];

				// Perform Digital Spotlighting
				Xsubimg = DigSpot(xc, yc, dig_spot, image_params, radar_params, in);

				// Adjust platform locations
				for(n=0; n<image_params->P2; n++) {
					dig_spot->Pt2[n][0] = in->Pt[n*image_params->TF][0] + xc;
					dig_spot->Pt2[n][1] = in->Pt[n*image_params->TF][1] + yc;
					dig_spot->Pt2[n][2] = in->Pt[n*image_params->TF][2];
				}

				// Perform backprojection over subimage
				BackProj(Xsubimg, dig_spot->Pt2, i*image_params->Sx2, (i+1)*image_params->Sx2-1,
                    j*image_params->Sy2, (j+1)*image_params->Sy2-1, image_params, curImage2, radar_params);
			}
		}

		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_destroy_plan(plan_forward);
	}
	else
	{
		// Perform backprojection over full image
		BackProj(in->X, in->Pt, 0, image_params->Ix2-1, 0, image_params->Iy2-1, image_params, curImage2, radar_params);
	}
}
