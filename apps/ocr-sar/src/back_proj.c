#ifndef RAG_SIM
#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"
#else
#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"
#include "rag_ocr.h"
#include "common.h"
#endif

ocrGuid_t backproject_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
  int retval;
  assert(paramc==0);
  assert(depc==5);
  RAG_REF_MACRO_SPAD(struct Inputs,in_junk,in_ptr,in,in_dbg,0);
  RAG_REF_MACRO_SPAD(struct corners_t,corners,corners_ptr,corners_lcl,corners_dbg,1);
  RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
  RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,3);
  RAG_REF_MACRO_BSM(struct complexData **,image,NULL,NULL,image_dbg,4);

  struct complexData sample, acc, arg;
  struct complexData **Xin;
  Xin     = in.X;

  float **platpos;
  platpos = in.Pt;

  int m1   = corners->m1;
  int m2   = corners->m2;
  int n1   = corners->n1;
  int n2   = corners->n2;

#ifdef TRACE_LVL_4
  xe_printf("//////// enter backproject_async\n");RAG_FLUSH;
#endif

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
  ocrGuid_t A_m_dbg, Phi_m_dbg, image_ptr_dbg, image_data_ptr_dbg;
  float *A_m = spad_malloc(&A_m_dbg, blk_size_whole*sizeof(float));
  if(A_m == NULL){xe_printf("error malloc of A_m\n");RAG_FLUSH;xe_exit(1);}
  struct complexData *Phi_m = spad_malloc(&Phi_m_dbg, blk_size_whole*sizeof(struct complexData));
  if(Phi_m == NULL){xe_printf("error malloc of Phi_m\n");RAG_FLUSH;xe_exit(1);}
#ifdef RAG_SPAD
#ifdef TRACE_LVL_5
  xe_printf("////////// before spad setup in backproject_async\n");RAG_FLUSH;
#endif
  struct complexData **image_ptr;
  image_ptr = spad_malloc(&image_ptr_dbg, (m2-m1)*sizeof(struct complexData *));
  if(image_ptr == NULL) { xe_printf("error malloc of image_ptr\n");RAG_FLUSH;xe_exit(1);}
  struct complexData *image_data_ptr;
  image_data_ptr = spad_malloc(&image_data_ptr_dbg, (m2-m1)*(n2-n1)*sizeof(struct complexData));
  if(image_data_ptr == NULL) { xe_printf("error malloc of image_data_ptr\n");RAG_FLUSH;xe_exit(1);}
  for(int m=0;m<(m2-m1);m++) {
    image_ptr[m] = image_data_ptr + m*(n2-n1);
    struct complexData *image_m = (struct complexData *)RAG_GET_PTR(image+m+m1);
assert(&image[m+m1][n1] == (image_m+n1));
    BSMtoSPAD(image_ptr[m],image_m+n1,(n2-n1)*sizeof(struct complexData));
  }
#ifdef TRACE_LVL_5
  xe_printf("////////// after spad setup in backproject_async\n");RAG_FLUSH;
#endif
#endif
#ifdef RAG_HIST_BIN_DIFFS
  uint64_t hist[10] = {0,0,0,0,0,0,0,0,0,0,};
#endif
  for(int k=0; k<IMAGE_PARAMS_P3; k++) {
    int old_int_bin = -1;
    float *platpos_k;
    platpos_k = (float *)RAG_GET_PTR(platpos+k);
    float platpos_k_012[3];
    REM_LDX_ADDR(platpos_k_012,platpos_k,float [3]);

    assert(platpos[k][0] == platpos_k_012[0]);
    assert(platpos[k][1] == platpos_k_012[1]);
    assert(platpos[k][2] == platpos_k_012[2]);

#ifdef TRACE_LVL_5
  xe_printf("////////// compute distance to R_mid\n");RAG_FLUSH;
#endif
    struct complexData *Xin_k;
    Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
#ifdef TRACE_LVL_5
  xe_printf("////////// compute distance to R_mid\n");RAG_FLUSH;
#endif
    double zr_mid  = 0.0f - platpos_k_012[2]; // Z
    double zr_mid2 = zr_mid * zr_mid;
    double yr_mid = RAG_GET_FLT(image_params->yr+(m1+blk_size_half)) - platpos_k_012[1]; // Y
    double yr_mid2 = yr_mid * yr_mid;
    double sqrt_arg = zr_mid2 + yr_mid2;
    double xr_mid = RAG_GET_FLT(image_params->xr+(n1+blk_size_half)) - platpos_k_012[0]; // X
    double xr_mid2 = xr_mid * xr_mid;
    double R_mid = sqrt_rn (sqrt_arg + xr_mid2);
#ifdef TRACE_LVL_5
  xe_printf("////////// compute coefficients for computing bin\n");RAG_FLUSH;
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
  xe_printf("////////// compute cofficients for computing arg\n");RAG_FLUSH;
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
  xe_printf("////////// compute Phi_m\n");RAG_FLUSH;
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
  xe_printf("////////// compute Phi_n\n");RAG_FLUSH;
#endif
    double theta_mid = ku2*R_mid;
    double arg_mid = theta_mid - blk_size_half*uy + blk_size_whole*blk_size_whole/4*vy;
    struct complexData Psi_n;
#ifdef RAG_SINCOS
    sincosf((arg_mid-2*Pi*round(arg_mid/2/Pi)), &(Psi_n.imag), &(Psi_n.real));
#else
    Psi_n.real = cosf(arg_mid-2*Pi*round(arg_mid/2/Pi));
    Psi_n.imag = sinf(arg_mid-2*Pi*round(arg_mid/2/Pi));
#endif
#ifdef TRACE_LVL_5
  xe_printf("////////// compute Gamma_m\n");RAG_FLUSH;
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
	  struct complexData *image_m = (struct complexData *)RAG_GET_PTR(image+m);
	  for(int n=n1; n<n2; n++) {
	    float bin = A_m[(n-n1)] + Bm + ((n-n1)-blk_size_half)*Cm;
#endif
	    struct complexData sample;
	    if(bin >= 0.0f && bin < IMAGE_PARAMS_S4_FLOAT /*image_params->S4-1*/) {
	      struct complexData left,right;

#ifdef GANESH_STRENGTH_RED_OPT
	      int int_bin = (int) bin; //(int)floorf(bin);
#else
	      int int_bin = (int)floorf(bin);
#endif

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
	      REM_LDX_ADDR(left, Xin_k+int_bin+0,struct complexData);
	      REM_LDX_ADDR(right,Xin_k+int_bin+1,struct complexData);
	      sample.real = (1-w)*left.real + w*right.real;
	      sample.imag = (1-w)*left.imag + w*right.imag;
	    } else if (bin > IMAGE_PARAMS_S4_FLOAT /*image_params->S4-1*/) {
	      struct complexData edge;
	      REM_LDX_ADDR(edge, Xin_k+IMAGE_PARAMS_S4_INT,struct complexData);
	      sample.real = edge.real;
	      sample.imag = edge.imag;
	    } else {
	      sample.real = 0.0;
	      sample.imag = 0.0;
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
	    struct complexData image_m_n;
	    REM_LDX_ADDR(image_m_n,image_m+n,struct complexData);
	    image_m_n.real += tmp.real;
	    image_m_n.imag += tmp.imag;
	    REM_STX_ADDR(image_m+n,image_m_n,struct complexData);
#endif
	  } // for n
	  tmp.real = Gamma_m.real*WXY.real - Gamma_m.imag*WXY.imag;
	  tmp.imag = Gamma_m.real*WXY.imag + Gamma_m.imag*WXY.real;
	  Gamma_m.real = tmp.real;
	  Gamma_m.imag = tmp.imag;
	} // for m
      } // for k
#ifdef RAG_HIST_BIN_DIFFS
      xe_printf("HIST = %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
		hist[0], hist[1], hist[2], hist[3], hist[4], hist[5], hist[6], hist[7], hist[8], hist[9]);
#endif
#ifdef RAG_SPAD
      for(int m=0;m<(m2-m1);m++) {
	//xe_printf("I<i %d\n",m);
	struct complexData *image_m = (struct complexData *)RAG_GET_PTR(image+m+m1);
	SPADtoBSM(image_m+n1,image_ptr[m],(n2-n1)*sizeof(struct complexData));
      }
      spad_free(image_data_ptr, image_data_ptr_dbg);
      spad_free(image_ptr, image_ptr_dbg);
#endif
      spad_free(Phi_m, Phi_m_dbg);
      spad_free(A_m, A_m_dbg);
#else // not RAG_PETER_DIST_OR_TRIG
#ifdef RAG_PURE_FLOAT
      const float ku = 2.0f*M_PI*radar_params->fc/c_mks_mps;
#else
      const float ku = 2*M_PI*radar_params->fc/c_mks_mps;
#endif

      for(int m=m1; m<m2; m++) {
#ifdef DEBUG_LVL_1
	xe_printf("backproject_async m(%d)\n",m);RAG_FLUSH;
#endif
	for(int n=n1; n<n2; n++) {
#ifdef DEBUG_LVL_2
	  xe_printf("backproject_async n(%d)\n",n);RAG_FLUSH;
#endif
	  acc.real = 0;
	  acc.imag = 0;
	  for(int k=0; k<IMAGE_PARAMS_P3; k++) {
#ifdef DEBUG_LVL_3
	    xe_printf("backproject_async k(%d)\n",k);RAG_FLUSH;
#endif
	    float *platpos_k;
	    platpos_k = (float *)RAG_GET_PTR(platpos+k);
	    float platpos_k_012[3];
	    REM_LDX_ADDR(platpos_k_012,platpos_k,float [3]);

	    assert(platpos[k][0] == platpos_k_012[0]);
	    assert(platpos[k][1] == platpos_k_012[1]);
	    assert(platpos[k][2] == platpos_k_012[2]);

	    struct complexData *Xin_k;
	    Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
	    float x = RAG_GET_FLT(image_params->xr+m) - platpos_k_012[0];
	    float y = RAG_GET_FLT(image_params->yr+n) - platpos_k_012[1];
	    float z =                                 - platpos_k_012[2];
	    float R = sqrtf_rn( x*x + y*y + z*z );
#ifdef DEBUG_LVL_3
	    xe_printf("backproject_async                 R(%f)\n",R);RAG_FLUSH;
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
	      REM_LDX_ADDR(left, Xin_k+int_bin+0,struct complexData);
	      REM_LDX_ADDR(right,Xin_k+int_bin+1,struct complexData);
	      sample.real = (1-w)*left.real + w*right.real;
	      sample.imag = (1-w)*left.imag + w*right.imag;
	    } else if (bin > image_params->S4-1) {
	      struct complexData edge;
	      REM_LDX_ADDR(edge, Xin_k+image_params->S4-1,struct complexData);
	      sample.real = edge.real;
	      sample.imag = edge.imag;
	    } else {
	      sample.real = 0.0f;
	      sample.imag = 0.0f;
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
#ifdef DEBUG_LVL_2
	  xe_printf("backproject_async                 update image[%d][%d]\n",m,n);RAG_FLUSH;
#endif
	  struct complexData *image_n;
	  image_n = (struct complexData *)RAG_GET_PTR(image+n);
	  REM_STX_ADDR(image_n+m,acc,struct complexData);
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
  xe_printf("//////// leave backproject_async\n");RAG_FLUSH;
#endif
  bsm_free(corners_ptr,corners_dbg);
  return NULL_GUID;
} // backproject_async

ocrGuid_t BackProj_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
  int retval;
#ifdef TRACE_LVL_3
  xe_printf("////// enter BackProj_edt\n");RAG_FLUSH;
#endif
  assert(paramc==0);
  assert(depc==6);
  RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,0);
  RAG_REF_MACRO_SPAD(struct corners_t,corners,corners_ptr,corners_lcl,corners_dbg,1);
  RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
  RAG_REF_MACRO_SPAD(struct RadarParams,radar_params,radar_params_ptr,radar_params_lcl,radar_params_dbg,3);
  RAG_REF_MACRO_BSM( struct complexData **,image,image_ptr,image_lcl,image_dbg,4);
  RAG_REF_MACRO_BSM( struct complexData **,refImage,refImage_ptr,curImage_lcl,refImage_dbg,5);
  struct complexData **Xin;
  Xin     = in->X;
  int m1   = corners->m1;
  int m2   = corners->m2;
  int n1   = corners->n1;
  int n2   = corners->n2;
  if(image_params->F > 1) {
    fftwf_complex *input, *fft_result, *ifft_result;
    ocrGuid_t input_dbg, fft_result_dbg, ifft_result_dbg;
    fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

#ifdef TRACE_LVL_3
    xe_printf("////// BackProj FFTW initialization F = %d\n",image_params->F);RAG_FLUSH;
#endif
    input         = (fftwf_complex*)fftwf_malloc(&input_dbg, image_params->S3 * sizeof(fftwf_complex));
    fft_result    = (fftwf_complex*)fftwf_malloc(&fft_result_dbg, image_params->S4 * sizeof(fftwf_complex));
    ifft_result   = (fftwf_complex*)fftwf_malloc(&ifft_result_dbg, image_params->S4 * sizeof(fftwf_complex));
    plan_forward  = fftwf_plan_dft_1d(image_params->S3, input,      fft_result,  FFTW_FORWARD, FFTW_ESTIMATE);
    plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

    float scale = 1/(float)image_params->S4;
    struct complexData **Xup;
    ocrGuid_t Xup_dbg;
    Xup = (struct complexData**)bsm_malloc(&Xup_dbg,image_params->P3*sizeof(struct complexData*));
    if(Xup == NULL) {
      xe_printf("Error allocating edge vector for Xup.\n");RAG_FLUSH;
      xe_exit(1);
    }
    struct complexData* Xup_data_ptr = NULL; ocrGuid_t Xup_data_dbg;
#ifndef RAG_SIM
    Xup_data_ptr = (struct complexData*)dram_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#else
    Xup_data_ptr = (struct complexData*) bsm_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#endif
    if ( Xup_data_ptr == NULL) {
      xe_printf("Error allocating data memory for Xup.\n");RAG_FLUSH;
      xe_exit(1);
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

      spad_memset(&fft_result[image_params->S3][0], 0,
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
    fftwf_free(input, input_dbg);
    fftwf_free(fft_result, fft_result_dbg);
    fftwf_free(ifft_result, ifft_result_dbg);
    fftwf_destroy_plan(plan_forward);
    fftwf_destroy_plan(plan_backward);
#ifdef TRACE
    xe_printf("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is > 1)\n",
	      m1, m2-1, n1, n2-1);RAG_FLUSH;
#endif
    struct Inputs tmp_in, *tmp_in_ptr; ocrGuid_t tmp_in_dbg;
    tmp_in_ptr = bsm_malloc(&tmp_in_dbg,sizeof(struct Inputs));
    tmp_in.Pt = in->Pt;
    tmp_in.Pt_edge_dbg = in->Pt_edge_dbg;
    tmp_in.Pt_data_dbg = in->Pt_data_dbg;
    tmp_in.Tp = NULL;
    tmp_in.Tp_dbg = NULL_GUID;
    tmp_in.X  = Xup;
    tmp_in.X_edge_dbg  = Xup_dbg;
    tmp_in.X_data_dbg  = Xup_data_dbg;;
    REM_STX_ADDR_SIZE(tmp_in_ptr,tmp_in,sizeof(struct Inputs));

    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
    assert( ((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M) == 0);
    assert( ((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N) == 0);

    // create a template for backproject_async function
    ocrGuid_t backproject_async_clg;
    retval = ocrEdtTemplateCreate(
	&backproject_async_clg, // ocrGuid_t *new_guid
	 backproject_async_edt,	// ocr_edt_ptr func_ptr
	0,			// paramc
	5);			// depc
    assert(retval==0);
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_3
	xe_printf("////// create an edt for backproject_async\n");RAG_FLUSH;
#endif
	struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; ocrGuid_t async_corners_dbg;
	async_corners = &async_corners_lcl;
	async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
	async_corners->m1   = m;
	async_corners->m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners->n1   = n;
	async_corners->n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
	REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
	ocrGuid_t backproject_async_scg;
	retval = ocrEdtCreate(
			&backproject_async_scg,	// *created_edt_guid
			 backproject_async_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,tmp_in_dbg,0); // Xup,platpos
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
	OCR_DB_RELEASE(async_corners_dbg);
      } // for n
    } // for m
    bsm_free(tmp_in_ptr,tmp_in_dbg);
#ifndef RAG_SIM
    dram_free(Xup_data_ptr,Xup_data_dbg); // Xup[]
#else
    bsm_free(Xup_data_ptr,Xup_data_dbg); // Xup[]
#endif
    bsm_free(Xup,Xup_dbg);
  } else { // if F
#ifdef TRACE
    xe_printf("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is = 1)\n",
	      m1, m2-1, n1, n2-1);RAG_FLUSH;
#endif
    int BACK_PROJ_ASYNC_BLOCK_SIZE_M = blk_size(m2-m1,32);
    int BACK_PROJ_ASYNC_BLOCK_SIZE_N = blk_size(n2-n1,32);
    assert( ((m2-m1)%BACK_PROJ_ASYNC_BLOCK_SIZE_M) == 0);
    assert( ((n2-n1)%BACK_PROJ_ASYNC_BLOCK_SIZE_N) == 0);
    // create a template for backproject_async function
    ocrGuid_t backproject_async_clg;
    retval = ocrEdtTemplateCreate(
	&backproject_async_clg, // ocrGuid_t *new_guid
	 backproject_async_edt,	// ocr_edt_ptr func_ptr
	0,			// paramc
	5);			// depc
    assert(retval==0);

    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_3
	xe_printf("////// create an edt for backproject_async\n");RAG_FLUSH;
#endif
	struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; ocrGuid_t async_corners_dbg;
	async_corners = &async_corners_lcl;
	async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
	async_corners->m1   = m;
	async_corners->m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners->n1   = n;
	async_corners->n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
	REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
	ocrGuid_t backproject_async_scg;
	retval = ocrEdtCreate(
			&backproject_async_scg,	// *created_edt_guid
			 backproject_async_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			NULL,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,in_dbg,0);	// Xin,platpos
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
	OCR_DB_RELEASE(async_corners_dbg);
      } // for n
    } // for m
    OCR_DB_RELEASE(in_dbg);
    OCR_DB_RELEASE(image_params_dbg);
    OCR_DB_RELEASE(radar_params_dbg);
    OCR_DB_RELEASE(image_dbg);
  } // if F

  bsm_free(corners_ptr,corners_dbg);
#ifdef TRACE_LVL_3
  xe_printf("////// leave BackProj_edt\n");RAG_FLUSH;
#endif
  return NULL_GUID;
}
