#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

#ifndef TG_ARCH // FIX-ME
#define dram_free(addr,guid)
#define  bsm_free(addr,guid)
#define spad_free(addr,guid)
#endif

ocrGuid_t backproject_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
  int retval;
#ifdef TRACE_LVL_4
  PRINTF("//////// enter backproject_async\n");RAG_FLUSH;
#endif
  assert(paramc==((sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t)));
  struct corners_t *corners = (struct corners_t *)paramv;
  int m1   = corners->m1;
  int m2   = corners->m2;
  int n1   = corners->n1;
  int n2   = corners->n2;
  assert(depc==6);
  RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,0);
  RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,1);
  RAG_REF_MACRO_BSM( struct complexData **,image,NULL,NULL,image_dbg,2);
  RAG_REF_MACRO_BSM( struct complexData **,Xin,NULL,NULL,Xin_dbg,3);
  RAG_REF_MACRO_BSM( float **,platpos,NULL,NULL,platpos_dbg,4);
  RAG_REF_MACRO_BSM( float *,Tp,NULL,NULL,Tp_dbg,4);

  struct complexData sample, acc, arg;

#ifdef GANESH_STRENGTH_RED_OPT
  float imageParams_S4_1_float = image_params->S4-1;
  int   imageParams_S4_1_int   = image_params->S4-1;
  int   imageParams_P3_int     = image_params->P3;
#define IMAGE_PARAMS_S4_FLOAT imageParams_S4_1_float
#define IMAGE_PARAMS_S4_INT imageParams_S4_1_int
#define IMAGE_PARAMS_S4 imageParams_S4_1_int
#define IMAGE_PARAMS_P3 imageParams_P3_int
#else
#define IMAGE_PARAMS_S4_FLOAT (image_params->S4 - 1)
#define IMAGE_PARAMS_S4_INT (image_params->S4 - 1)
#define IMAGE_PARAMS_S4 (image_params->S4 - 1)
#define IMAGE_PARAMS_P3 image_params->P3
#endif

#ifdef RAG_PETER_DIST_AND_TRIG
  double const Pi = 3.1415926535897932384626433832795029l;
  double ku2   = 2.0*2.0*Pi*radar_params->fc/c_mks_mps;
  double ku2dr = ku2*image_params->dr;

  assert((m2-m1) == (n2-n1));
  int blk_size_whole = (n2-n1);
  int blk_size_half = blk_size_whole/2;
  ocrGuid_t A_m_dbg, Phi_m_dbg, image_ptr_dbg;
  float *A_m = spad_malloc(&A_m_dbg, blk_size_whole*sizeof(float));
  if(A_m == NULL){PRINTF("Error allocating memory for A_m\n");RAG_FLUSH;xe_exit(1);}
  struct complexData *Phi_m = spad_malloc(&Phi_m_dbg, blk_size_whole*sizeof(struct complexData));
  if(Phi_m == NULL){PRINTF("Error allocating memory for Phi_m\n");RAG_FLUSH;xe_exit(1);}
#ifdef RAG_SPAD
#ifdef TRACE_LVL_5
  PRINTF("////////// before spad setup in backproject_async\n");RAG_FLUSH;
#endif
  struct complexData **image_ptr;
  image_ptr = (struct complexData **)spad_malloc(&image_ptr_dbg,(m2-m1)*sizeof(struct complexData *)
			                                       +(m2-m1)*(n2-n1)*sizeof(struct complexData));
  if(image_ptr == NULL) { PRINTF("Error allocating memory for image_ptr\n");RAG_FLUSH;xe_exit(1);}
  struct complexData *image_data_ptr = (struct complexData *)&image_ptr[(m2-m1)];
  if(image_data_ptr == NULL) { PRINTF("Unable to allocate memory for image_ptr\n");RAG_FLUSH;xe_exit(1);}
  for(int m=0;m<(m2-m1);m++) {
    image_ptr[m] = image_data_ptr + m*(n2-n1);
    BSMtoSPAD( image_ptr[m],&image[m+m1][n1],(n2-n1)*sizeof(struct complexData));
  }
#ifdef TRACE_LVL_5
  PRINTF("////////// after spad setup in backproject_async\n");RAG_FLUSH;
#endif
#endif
#ifdef RAG_HIST_BIN_DIFFS
  uint64_t hist[10] = {0,0,0,0,0,0,0,0,0,0,};
#endif
  for(int k=0; k<IMAGE_PARAMS_P3; k++) {
    int old_int_bin = -1;

#ifdef TRACE_LVL_5
  PRINTF("////////// compute distance to R_mid\n");RAG_FLUSH;
#endif
    double zr_mid  =                                   - platpos[k][2]; // Z
    double zr_mid2 = zr_mid * zr_mid;
    double yr_mid = image_params->yr[m1+blk_size_half] - platpos[k][1]; // Y
    double yr_mid2 = yr_mid * yr_mid;
    double sqrt_arg = zr_mid2 + yr_mid2;
    double xr_mid = image_params->xr[n1+blk_size_half] - platpos[k][0]; // X
    double xr_mid2 = xr_mid * xr_mid;
    double R_mid = sqrt (sqrt_arg + xr_mid2);
#ifdef TRACE_LVL_5
  PRINTF("////////// compute coefficients for computing bin\n");RAG_FLUSH;
#endif
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
#ifdef TRACE_LVL_5
  PRINTF("////////// compute cofficients for computing arg\n");RAG_FLUSH;
#endif
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
#ifdef TRACE_LVL_5
  PRINTF("////////// compute Phi_m\n");RAG_FLUSH;
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
#ifdef TRACE_LVL_5
  PRINTF("////////// compute Phi_n\n");RAG_FLUSH;
#endif
    double theta_mid = ku2*R_mid;
    double arg_mid = theta_mid - blk_size_half*uy + blk_size_whole*blk_size_whole/4*vy;
    struct complexData Psi_n;
#ifdef RAG_SINCOS
    double Psi_n_imag, Psi_n_real;
    sincos((arg_mid-2.0*Pi*round(arg_mid/2.0/Pi)), &Psi_n_imag, &Psi_n_real); // RAG -- Changed precision to match latest code from Dan Campbell
    Psi_n.imag = (float)Psi_n_imag;
    Psi_n.real = (float)Psi_n_real;
#else
    Psi_n.real = cos(arg_mid-2.0*Pi*round(arg_mid/2.0/Pi)); // RAG -- Changed precision to match latest code from Dan Campbell
    Psi_n.imag = sin(arg_mid-2.0*Pi*round(arg_mid/2.0/Pi)); // RAG -- Changed precision to match latest code from Dan Campbell
#endif
#ifdef TRACE_LVL_5
  PRINTF("////////// compute Gamma_m\n");RAG_FLUSH;
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
#ifdef RAG_SPAD
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
#ifdef RAG_SPAD
      for(int n=0; n<(n2-n1); n++) {
        float bin = A_m[n] + Bm + (n-blk_size_half)*Cm;
#else
      for(int n=n1; n<n2; n++) {
        float bin = A_m[(n-n1)] + Bm + ((n-n1)-blk_size_half)*Cm;
#endif
	struct complexData sample;
	if(bin >= 0.0f && bin < IMAGE_PARAMS_S4_FLOAT /*image_params->S4-1*/) {
	  struct complexData left,right;

	  int int_bin = (int)floorf(bin);

#ifdef RAG_HIST_BIN_DIFFS
          if(old_int_bin == -1) {
            old_int_bin = int_bin;
	  } else if(int_bin > old_int_bin) {
            if((int_bin-old_int_bin)<9) hist[(int_bin-old_int_bin)]++;
            else hist[9]++;
	  } else if(int_bin < old_int_bin) {
	    if((old_int_bin-int_bin)<9) hist[(old_int_bin-int_bin)]++;
	    else hist[9]++;
	  } else {
	    hist[0]++;
          }
#endif
          float w = bin - int_bin;
          left  = Xin[k][int_bin+0];
          right = Xin[k][int_bin+1];
          sample.real = (1-w)*left.real + w*right.real;
          sample.imag = (1-w)*left.imag + w*right.imag;
#if 0 // RAG to match change in latest code from Dan Campbell
        } else if (bin > IMAGE_PARAMS_S4_FLOAT /*image_params->S4-1*/) {
          sample = Xin[k][IMAGE_PARAMS_S4_INT];
#endif
        } else {
          sample.real = 0.0f;
          sample.imag = 0.0f;
        }
        struct complexData arg;
#ifdef RAG_SPAD
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
#ifdef RAG_SPAD
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
#ifdef RAG_HIST_BIN_DIFFS
  PRINTF("HIST = %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	hist[0], hist[1], hist[2], hist[3], hist[4], hist[5], hist[6], hist[7], hist[8], hist[9]);
#endif
#ifdef RAG_SPAD
  for(int m=0;m<(m2-m1);m++) {
    //PRINTF("I<i %d\n",m);
    SPADtoBSM(&image[m+m1][n1],image_ptr[m],(n2-n1)*sizeof(struct complexData));
   }
   spad_free(image_ptr, image_ptr_dbg);
#endif
   spad_free(Phi_m, Phi_m_dbg);
   spad_free(A_m, A_m_dbg);
#else // not RAG_PETER_DIST_OR_TRIG
#ifdef RAG_PURE_FLOAT
      const float ku = 2.0f*M_PI*radar_params->fc/c_mks_mps;
#else
      const float ku = 2.0*M_PI*radar_params->fc/c_mks_mps;
#endif

      for(int m=m1; m<m2; m++) {
#ifdef DEBUG_LVL_1
	PRINTF("backproject_async m(%d)\n",m);RAG_FLUSH;
#endif
	for(int n=n1; n<n2; n++) {
#ifdef DEBUG_LVL_2
	  PRINTF("backproject_async n(%d)\n",n);RAG_FLUSH;
#endif
	  acc.real = 0;
	  acc.imag = 0;
	  for(int k=0; k<IMAGE_PARAMS_P3; k++) {
#ifdef DEBUG_LVL_3
	    PRINTF("backproject_async k(%d)\n",k);RAG_FLUSH;
#endif
	    double x = (double)image_params->xr[m] - platpos[k][0];
	    double y = (double)image_params->yr[n] - platpos[k][1];
	    double z = (double)                      platpos[k][2];
	    double R = sqrt( x*x + y*y + z*z ); // RAG -- Changed precision to match latest code from Dan Campbell
#ifdef DEBUG_LVL_3
	    PRINTF("backproject_async                 R(%f)\n",R);RAG_FLUSH;
#endif
	    float bin;
	    if(image_params->TF > 1) {
	      bin = (R-radar_params->R0_prime)/image_params->dr;
	    }
	    else {
	      bin = (R-radar_params->R0)/image_params->dr;
	    }

	    if(bin >= 0.0f && bin < image_params->S4-1) {
	      struct complexData left,right;
	      int int_bin = (int)floorf(bin);
	      float w = bin - int_bin;
	      left  = Xin[k][int_bin+0];
	      right = Xin[k][int_bin+1];
	      sample.real = (1-w)*left.real + w*right.real;
	      sample.imag = (1-w)*left.imag + w*right.imag;
#if 0 // RAG to match change in latest code from Dan Campbell
	    } else if (bin > image_params->S4-1) {
	      sample = Xin[k][image_params->S4-1];
#endif
	    } else {
	      sample.real = 0.0f;
	      sample.imag = 0.0f;
	    }
#ifdef RAG_SINCOS
            double arg_imag, arg_real;
	    sincos(2.0*ku*R,&arg_imag,&arg_real); // RAG -- Changed precision to match latest code from Dan Campbell
            arg.imag = (float)arg_imag;
            arg.real = (float)arg_real;
#else
	    arg.real = cos(2.0*ku*R); // RAG -- Changed precision to match latest code from Dan Campbell
	    arg.imag = sin(2.0*ku*R); // RAG -- Changed precision to match latest code from Dan Campbell
#endif
	    acc.real += sample.real*arg.real - sample.imag*arg.imag;
	    acc.imag += sample.real*arg.imag + sample.imag*arg.real;
	  } // for k
#ifdef DEBUG_LVL_2
	  PRINTF("backproject_async                 update image[%d][%d]\n",m,n);RAG_FLUSH;
#endif
	  image[n][m] = acc;
#ifdef RAG_SPAD
	} // for n
#else
      } // for n
#endif
#ifdef RAG_SPAD
    } // for m
#else
  } // for m
#endif
#endif // RAG_PETER_DIST_AND_TRIG
#ifdef TRACE_LVL_4
  PRINTF("//////// leave backproject_async\n");RAG_FLUSH;
#endif
  return NULL_GUID;
} // backproject_async

ocrGuid_t BackProj_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
  int retval;
#ifdef TRACE_LVL_3
  PRINTF("////// enter BackProj_edt\n");RAG_FLUSH;
#endif
  assert(paramc==((sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t)));
  struct corners_t *corners = (struct corners_t *)paramv;
  int m1   = corners->m1;
  int m2   = corners->m2;
  int n1   = corners->n1;
  int n2   = corners->n2;
  assert(depc==7);
  RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,0);
  RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,1);
  RAG_REF_MACRO_BSM( struct complexData **,image,NULL,NULL,image_dbg,2);
  RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,3);
  RAG_REF_MACRO_BSM( struct complexData **,Xin,NULL,NULL,Xin_dbg,4);
  RAG_REF_MACRO_BSM( float **,Pt,NULL,NULL,Pt_dbg,5);
  RAG_REF_MACRO_BSM( float *,Tp,NULL,NULL,Tp_dbg,6);
#if 0
#warn RAG
#endif
  if(image_params->F > 1) {
    fftwf_complex *input, *fft_result, *ifft_result;
    ocrGuid_t input_dbg, fft_result_dbg, ifft_result_dbg;
    fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

#ifdef TRACE_LVL_3
    PRINTF("////// BackProj FFTW initialization F = %d\n",image_params->F);RAG_FLUSH;
#endif
    input         = (fftwf_complex*)fftwf_malloc(&input_dbg, image_params->S3 * sizeof(fftwf_complex));
    fft_result    = (fftwf_complex*)fftwf_malloc(&fft_result_dbg, image_params->S4 * sizeof(fftwf_complex));
    ifft_result   = (fftwf_complex*)fftwf_malloc(&ifft_result_dbg, image_params->S4 * sizeof(fftwf_complex));
    plan_forward  = fftwf_plan_dft_1d(image_params->S3, input,      fft_result,  FFTW_FORWARD, FFTW_ESTIMATE);
    plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

    float scale = 1/(float)image_params->S4;
    struct complexData **Xup;
    ocrGuid_t Xup_dbg;
#ifdef RAG_DRAM
    Xup = (struct complexData **)dram_malloc(&Xup_dbg,(image_params->P3)*sizeof(struct complexData *)
						     +(image_params->P3)*(image_params->S4)*sizeof(struct complexData));
#else
    Xup = (struct complexData **) bsm_malloc(&Xup_dbg,(image_params->P3)*sizeof(struct complexData *)
						     +(image_params->P3)*(image_params->S4)*sizeof(struct complexData));
#endif
    if(Xup == NULL) {
      PRINTF("Error allocating memory for Xup.\n");RAG_FLUSH;
      xe_exit(1);
    }
    struct complexData *Xup_data_ptr = (struct complexData *)&Xup[image_params->P3];
    if ( Xup_data_ptr == NULL) {
      PRINTF("Unable to allocate memory for Xup.\n");RAG_FLUSH;
      xe_exit(1);
    }
    for(int n=0; n<image_params->P3; n++) {
      Xup[n] =  Xup_data_ptr + n*image_params->S4;
    }

    for(int m=0; m<image_params->P3; m++) {
#ifdef RAG_DRAM
      DRAMtoSPAD(input, Xin[m], image_params->S3*sizeof(struct complexData));
#else
      BSMtoSPAD( input, Xin[m], image_params->S3*sizeof(struct complexData));
#endif

      fftwf_execute(plan_forward);

      spad_memset(&fft_result[image_params->S3][0], 0,
	     (image_params->S4-image_params->S3)*sizeof(fftwf_complex));

      fftwf_execute(plan_backward);

      for(int n=0; n<image_params->S4; n++) {
	ifft_result[n][0] *= scale;
	ifft_result[n][1] *= scale;
      }
#ifdef RAG_DRAM
      SPADtoDRAM(Xup[m],ifft_result, image_params->S3*sizeof(struct complexData));
#else
      SPADtoBSM( Xup[m],ifft_result, image_params->S3*sizeof(struct complexData));
#endif
    }
    // Free memory and deallocate plan
    fftwf_free(input, input_dbg);
    fftwf_free(fft_result, fft_result_dbg);
    fftwf_free(ifft_result, ifft_result_dbg);
    fftwf_destroy_plan(plan_forward);
    fftwf_destroy_plan(plan_backward);
#ifdef TRACE
    PRINTF("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n",
	      m1, m2-1, n1, n2-1);RAG_FLUSH;
#endif
#if !defined(TG_ARCH)
    fprintf(stderr,"Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n",
	      m1, m2-1, n1, n2-1);fflush(stderr);
#endif

#ifdef RAG_NEW_BLK_SIZE
    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = RAG_NEW_BLK_SIZE;
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = RAG_NEW_BLK_SIZE;
#else
    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
    assert( ((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M) == 0);
    assert( ((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N) == 0);
#endif

    // create a template for backproject_async function
    ocrGuid_t backproject_async_clg;
    retval = ocrEdtTemplateCreate(
	&backproject_async_clg, // ocrGuid_t *new_guid
	 backproject_async_edt,	// ocr_edt_ptr func_ptr
	(sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t), // paramc
	6);			// depc
    assert(retval==0);
    templateList[__sync_fetch_and_add(&templateIndex,1)] = backproject_async_clg;

#ifdef RAG_NEW_BLK_SIZE
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
	struct corners_t async_corners;
	async_corners.m1   = m;
	async_corners.m2   = (m+BACK_PROJ_ASYNC_BLOCK_SIZE_M)<m2?(m+BACK_PROJ_ASYNC_BLOCK_SIZE_M):m2;
	async_corners.n1   = n;
	async_corners.n2   = (n+BACK_PROJ_ASYNC_BLOCK_SIZE_N)<n2?(n+BACK_PROJ_ASYNC_BLOCK_SIZE_N):n2;
#else
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
	struct corners_t async_corners;
	async_corners.m1   = m;
	async_corners.m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners.n1   = n;
	async_corners.n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
#endif
#ifdef TRACE_LVL_3
	PRINTF("////// create an edt for backproject_async\n");RAG_FLUSH;
#endif
	ocrGuid_t backproject_async_scg;
	retval = ocrEdtCreate(
			&backproject_async_scg,	// *created_edt_guid
			 backproject_async_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			(uint64_t *)&async_corners, // *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,0);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Xup_dbg,3); // Xup
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Pt_dbg,4);  // Platform Position
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Tp_dbg,5);  // Platform Position
      } // for n
    } // for m
#ifndef TG_ARCH
    dram_free(Xup,Xup_dbg); // Xup[]
#else
     bsm_free(Xup,Xup_dbg); // Xup[]
#endif
  } else { // if F
#ifdef TRACE
    PRINTF("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n",
	      m1, m2-1, n1, n2-1);RAG_FLUSH;
#endif
#if !defined(TG_ARCH)
    fprintf(stderr,"Performing backprojection over Ix[%d:%d] and Iy[%d:%d]\n",
	      m1, m2-1, n1, n2-1);fflush(stderr);
#endif

#ifdef RAG_NEW_BLK_SIZE
    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = RAG_NEW_BLK_SIZE;
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = RAG_NEW_BLK_SIZE;
#else
    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
    assert( ((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M) == 0);
    assert( ((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N) == 0);
#endif

    // create a template for backproject_async function
    ocrGuid_t backproject_async_clg;
    retval = ocrEdtTemplateCreate(
	&backproject_async_clg, // ocrGuid_t *new_guid
	 backproject_async_edt,	// ocr_edt_ptr func_ptr
	(sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t), // paramc
	6);			// depc
    assert(retval==0);
    templateList[__sync_fetch_and_add(&templateIndex,1)] = backproject_async_clg;

#ifdef RAG_NEW_BLK_SIZE
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
	struct corners_t async_corners;
	async_corners.m1   = m;
	async_corners.m2   = (m+BACK_PROJ_ASYNC_BLOCK_SIZE_M)<m2?(m+BACK_PROJ_ASYNC_BLOCK_SIZE_M):m2;
	async_corners.n1   = n;
	async_corners.n2   = (n+BACK_PROJ_ASYNC_BLOCK_SIZE_N)<n2?(n+BACK_PROJ_ASYNC_BLOCK_SIZE_N):n2;
#else
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
	struct corners_t async_corners;
	async_corners.m1   = m;
	async_corners.m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners.n1   = n;
	async_corners.n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
#endif
#ifdef TRACE_LVL_3
	PRINTF("////// create an edt for backproject_async\n");RAG_FLUSH;
#endif
	ocrGuid_t backproject_async_scg;
	retval = ocrEdtCreate(
			&backproject_async_scg,	// *created_edt_guid
			 backproject_async_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			(uint64_t *)&async_corners, // *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,0);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Xin_dbg,3); // Xin
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Pt_dbg,4);  // Platform Position
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,Tp_dbg,5);  // Time Pulse
      } // for n
    } // for m
  } // if F

#ifdef TRACE_LVL_3
  PRINTF("////// leave BackProj_edt\n");RAG_FLUSH;
#endif
  return NULL_GUID;
}
