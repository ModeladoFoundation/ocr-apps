#include "common.h"

#include "rag_rmd.h"

rmd_guid_t FormImage(
#if RAG_DIG_SPOT_ON
	rmd_guid_t dig_spot_dbg,
#endif
	rmd_guid_t image_params_dbg,
	rmd_guid_t radar_params_dbg,
	rmd_guid_t in_dbg,
	rmd_guid_t curImage_dbg)
{
#if RAG_DIG_SPOT_ON
	struct DigSpotVars *dig_spot,     *dig_spot_ptr,     dig_spot_lcl;
#endif
	struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
	struct RadarParams *radar_params, *radar_params_ptr, radar_params_lcl;
	struct Inputs       *in;
	struct complexData **curImage;

#if RAG_DIG_SPOT_ON
	RMD_DB_MEM(&dig_spot_ptr,      dig_spot_dbg);
	REM_LDX_ADDR(dig_spot_lcl,     dig_spot_ptr,     struct DigSpotVars);
	dig_spot = &dig_spot_lcl;
#endif
	RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
	REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
	image_params = &image_params_lcl;

	RMD_DB_MEM(&radar_params_ptr,  radar_params_dbg);
	REM_LDX_ADDR(radar_params_lcl, radar_params_ptr, struct RadarParams);
	radar_params = &radar_params_lcl;

	RMD_DB_MEM(&in      ,      in_dbg      );
	RMD_DB_MEM(&curImage,      curImage_dbg);

#ifdef TRACE
xe_printf("// FormImage Zero curImage\n");RAG_FLUSH;
#endif
	for(int n=0; n<image_params->Iy; n++) {
// &curImage[n][0]
		bsm_memset(RAG_GET_PTR(curImage+n), 0, image_params->Ix*sizeof(struct complexData));
	}
	
	if(image_params->TF > 1)
	{
#if !RAG_DIG_SPOT_ON
// to stop defined but unused warning by compiler
		radar_params = radar_params;
		xe_printf("!!! DIGITAL SPOTLIGHTING NOT YET SUPPORTED !!!\n");RAG_FLUSH;
		exit(1);
#else // RAG_DIG_SPOT_ON
#ifdef TRACE
xe_printf("// FormImage FFTW initialization TF = %d\n",image_params->TF);RAG_FLUSH;
#endif
		fftwf_plan plan_forward;
		fftwf_complex *input, *fft_result;
		struct complexData **Xsubimg;
		float xc, yc;
		input = (fftwf_complex*)fftwf_malloc(image_params->S1 * sizeof(fftwf_complex));
		fft_result = (fftwf_complex*)fftwf_malloc(image_params->S1 * sizeof(fftwf_complex));
		plan_forward = fftwf_plan_dft_1d(image_params->S1, input, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

		for(int i=0; i<image_params->P1; i++)
		{
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

		for(int i=0; i<image_params->TF; i++)
		{
			for(int j=0; j<image_params->TF; j++)
			{
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

#ifdef TRACE
xe_printf("// Perform backprojection over subimage\n");RAG_FLUSH;
#endif
		int corners[4], *corners_ptr; rmd_guid_t corners_dbg;
		corners_ptr = bsm_malloc(&corners_dbg,4*sizeof(int));
		corners[0] = i*image_params->Sx;
		corners[1] = (i+1)*image_params->Sx;
		corners[2] = j*image_params->Sy;
		corners[3] = (j+1)*image_params->Sy;
		REM_STX_ADDR_SIZE(corners_ptr,corners,4*sizeof(int));
		struct Inputs tmp_in, *tmp_in_ptr; rmd_guid_t tmp_in_dbg;
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
				BackProj(1,tmp_dbg, corners_dbg,
 				image_params_dbg, curImage_dbg,
				radar_params_dbg);
			}
		}
		RMD_DB_RELEASE(tmp_in);	
		RMD_DB_RELEASE(corners_dbg);	
		fftwf_free(input);
		fftwf_free(fft_result);
		fftwf_destroy_plan(plan_forward);
#endif // RAG_DIG_SPOT_ON
	} else {
#ifdef TRACE
xe_printf("// Perform backprojection over full image\n");RAG_FLUSH;
#endif
		int corners[4], *corners_ptr; rmd_guid_t corners_dbg;
		corners_ptr = bsm_malloc(&corners_dbg,4*sizeof(int));
		corners[0] = 0;
		corners[1] = image_params->Ix;
		corners[2] = 0;
		corners[3] = image_params->Iy;
		REM_STX_ADDR_SIZE(corners_ptr,corners,4*sizeof(int));
		BackProj(0,in_dbg, corners_dbg, image_params_dbg, curImage_dbg, radar_params_dbg);
		RMD_DB_RELEASE(in_dbg);
		RMD_DB_RELEASE(corners_dbg);
	}
#ifdef TRACE
xe_printf("// FormImage done\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
