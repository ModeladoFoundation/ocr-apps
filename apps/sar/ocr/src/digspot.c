#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

#ifdef RAG_DIG_SPOT
struct complexData** DigSpot(float xc, float yc, struct DigSpotVars *dig_spot, struct ImageParams *image_params, struct RadarParams *radar_params, struct complexData **x, float **Pt, flost *Pt)
{
	int m, n;
	float arg;
	float scale;
	float deltaR;
	fftwf_plan plan_backward;
	fftwf_complex *ifft_input, *ifft_result; ocrGuid_t ifft_input_dbg, ocrGuid_t ifft_result_dbg;
#ifdef DEBUG
PRINTF("DigSpot P1, S1 <%3d,%3d>\n",image_params->P1, image_params->S1);
PRINTF("DigSpot P2, S2 <%3d,%3d>\n",image_params->P2, image_params->S2);
PRINTF("DigSpot P3, S3 <%3d,%3d>\n",image_params->P3, image_params->S3);
RAG_FLUSH;
#endif
	ifft_input  = (fftwf_complex*)fftwf_malloc(&ifft_input_dbg, image_params->S1 * sizeof(fftwf_complex));
	ifft_result = (fftwf_complex*)fftwf_malloc(&ifft_results_dbg, image_params->S1 * sizeof(fftwf_complex));
	plan_backward = fftwf_plan_dft_1d(image_params->S1, ifft_input, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

	PRINTF("Xc = %7.2fm Yc = %7.2fm\n", xc, yc);RAG_FLUSH;

	// FFTW doesn't perform normalization
	scale = 1/(float)image_params->S1;

	for(m=0; m<image_params->P1; m++)
	{
		// (Range from original platform position to new scene center) - r0
		deltaR = sqrtf( (xc-Pt[m][0])*(xc-Pt[m][0]) + (yc-Pt[m][1])*(yc-Pt[m][1]) + Pt[m][2]*Pt[m][2] ) - radar_params->r0;
		// Induce phase shift
		for(n=0; n<image_params->S1; n++) {
#ifdef RAG_PURE_FLOAT
			arg = 4.0f*((float)M_PI)*deltaR*(radar_params->fc+dig_spot->freqVec[n])/c_mks_mps;
#else
			arg = 4.0*M_PI*deltaR*(radar_params->fc+dig_spot->freqVec[n])/c_mks_mps;
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
		sinc_interp(Tp, dig_spot->tmpVector, dig_spot->filtOut, 8, radar_params->PRF/image_params->TF, image_params->TF, image_params->P1);

		for(m=0; m<image_params->P2; m++) {
			dig_spot->X4[m][n].real = dig_spot->filtOut[m].real;
			dig_spot->X4[m][n].imag = dig_spot->filtOut[m].imag;
		}
	}

	fftwf_free(ifft_input, ifft_input_dbg);
	fftwf_free(ifft_result, ifft_result_dbg);
	fftwf_destroy_plan(plan_backward);

	return dig_spot->X4;
}
#endif // RAG_DIG_SPOT
