#include "common.h"

struct complexData** DigSpot(float xc, float yc, struct DigSpotVars *dig_spot, struct ImageParams *image_params, struct RadarParams *radar_params, struct Inputs *in)
{
	int m, n;
	float arg;
	float scale;
	float deltaR;
	fftwf_plan plan_backward;
	fftwf_complex *ifft_input, *ifft_result;

	ifft_input = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S1 * sizeof(fftwf_complex));
	ifft_result = (fftwf_complex*)FFTWF_MALLOC_CHECKED(image_params->S1 * sizeof(fftwf_complex));
	plan_backward = fftwf_plan_dft_1d(image_params->S1, ifft_input, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

	printf("Xc = %7.2fm Yc = %7.2fm\n", xc, yc);

	// FFTW doesn't perform normalization
	scale = 1/(float)image_params->S1;

	for(m=0; m<image_params->P1; m++)
	{
		// (Range from original platform position to new scene center) - r0
		deltaR = sqrtf( (xc-in->Pt[m][0])*(xc-in->Pt[m][0]) + (yc-in->Pt[m][1])*(yc-in->Pt[m][1]) + in->Pt[m][2]*in->Pt[m][2] ) - radar_params->r0;

		// Induce phase shift
		for(n=0; n<image_params->S1; n++) {
#ifdef RAG_PURE_FLOAT
			arg = 4.0f*M_PI*deltaR*(radar_params->fc+dig_spot->freqVec[n])/SPEED_OF_LIGHT;
#else
			arg = 4.0*M_PI*deltaR*(radar_params->fc+dig_spot->freqVec[n])/SPEED_OF_LIGHT;
#endif
			ifft_input[n][0] = dig_spot->X2[m][n].real*cosf(arg) - dig_spot->X2[m][n].imag*sinf(arg);
			ifft_input[n][1] = dig_spot->X2[m][n].real*sinf(arg) + dig_spot->X2[m][n].imag*cosf(arg);
		}

		// Perform inverse FFT
		fftwf_execute(plan_backward);

		// Range gating
		for(n=0; n<image_params->S2/2; n++) {
			dig_spot->X3[m][n].real = scale*ifft_result[image_params->S1-image_params->S2/2+n][0];
			dig_spot->X3[m][n].imag = scale*ifft_result[image_params->S1-image_params->S2/2+n][1];
			dig_spot->X3[m][image_params->S2/2+n].real = scale*ifft_result[n][0];
			dig_spot->X3[m][image_params->S2/2+n].imag = scale*ifft_result[n][1];
		}
	}

	for(n=0; n<image_params->S2; n++)
	{
		for(m=0; m<image_params->P1; m++) {
			dig_spot->tmpVector[m].real = dig_spot->X3[m][n].real;
			dig_spot->tmpVector[m].imag = dig_spot->X3[m][n].imag;
		}

		// Sinc filtering and interpolation
		sinc_interp(in->Tp, dig_spot->tmpVector, dig_spot->filtOut, 8, radar_params->PRF/image_params->TF, image_params->TF, image_params->P1);

		for(m=0; m<image_params->P2; m++) {
			dig_spot->X4[m][n].real = dig_spot->filtOut[m].real;
			dig_spot->X4[m][n].imag = dig_spot->filtOut[m].imag;
		}
	}

	fftwf_free(ifft_input);
	fftwf_free(ifft_result);
	fftwf_destroy_plan(plan_backward);

	return dig_spot->X4;
}
