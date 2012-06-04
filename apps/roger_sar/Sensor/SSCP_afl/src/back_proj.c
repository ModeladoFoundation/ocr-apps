#include "common.h"

#include "rag_rmd.h"

rmd_guid_t backproject_finish_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  int retval;
#ifdef TRACE_LVL_4
  xe_printf("//////// enter backproject_finish\n");RAG_FLUSH;
#endif
  assert(n_db>1);
  rmd_guid_t arg_scg = { .data = arg };
  RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,db[0],0);
#ifdef TRACE_LVL_4
  xe_printf("//////// leave backproject_finish\n");RAG_FLUSH;
#endif
  return NULL_GUID;
}

rmd_guid_t backproject_async_codelet(uint64_t rmd_arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  int retval;
  assert(n_db == 5);
  RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,in_dbg,0);
  RAG_REF_MACRO_SPAD(struct corners_t,corners,corners_ptr,corners_lcl,corners_dbg,1);
  RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_params_dbg,2);
  RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,radar_params_dbg,3);
  RAG_REF_MACRO_PASS(NULL,NULL,NULL,NULL,image_dbg,4);

  struct complexData sample, acc, arg;
  struct Inputs in, *in_ptr;
  RMD_DB_MEM(&in_ptr, in_dbg);
  REM_LDX_ADDR(in,in_ptr,struct Inputs);
  struct complexData **Xin;
  Xin     = in.X;

  float **platpos;
  platpos = in.Pt;

  int m1   = corners->m1;
  int m2   = corners->m2;
  int n1   = corners->n1;
  int n2   = corners->n2;
  int slot = corners->slot;

#ifdef TRACE_LVL_4
  xe_printf("//////// enter backproject_async slot %d\n",slot);RAG_FLUSH;
#endif

  struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
  RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
  REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
  image_params = &image_params_lcl;
  float imageParams_S4_1_float = image_params->S4-1;
  int imageParams_S4_1_int = image_params->S4-1;
  int imageParams_P3_int = image_params->P3;
#ifdef GANESH_STRENGTH_RED_OPT
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

  struct complexData **image;
  RMD_DB_MEM(&image,      image_dbg);

  struct RadarParams *radar_params, *radar_params_ptr, radar_params_lcl;
  RMD_DB_MEM(&radar_params_ptr,  radar_params_dbg);
  REM_LDX_ADDR(radar_params_lcl, radar_params_ptr, struct RadarParams);
  radar_params = &radar_params_lcl;

#ifdef RAG_PETER_DIST_AND_TRIG
  double const Pi = 3.1415926535897932384626433832795029l;
  double ku2   = 2.0*2.0*Pi*radar_params->fc/c_mks_mps;
  double ku2dr = ku2*image_params->dr;

  assert((m2-m1) == (n2-n1));
  int blk_size_whole = (n2-n1);
  int blk_size_half = blk_size_whole/2;
  float *A_m = malloc(blk_size_whole*sizeof(float));
  if(A_m == NULL){xe_printf("error malloc of A_m\n");RAG_FLUSH;exit(1);}
  struct complexData *Phi_m = malloc(blk_size_whole*sizeof(struct complexData));
  if(Phi_m == NULL){xe_printf("error malloc of Phi_m\n");RAG_FLUSH;exit(1);}
#ifdef RAG_SPAD
  struct complexData **image_ptr;
  image_ptr = malloc((m2-m1)*sizeof(struct complexData *));
  if(image_ptr == NULL) { xe_printf("error malloc of image_ptr\n");RAG_FLUSH;exit(1);}
  struct complexData *image_data_ptr;
  image_data_ptr = malloc((m2-m1)*(n2-n1)*sizeof(struct complexData));
  if(image_data_ptr == NULL) { xe_printf("error malloc of image_data_ptr\n");RAG_FLUSH;exit(1);}
  for(int m=0;m<(m2-m1);m++) {
    image_ptr[m] = image_data_ptr + m*(n2-n1);
    struct complexData *image_m = (struct complexData *)RAG_GET_PTR(image+m+m1);
    BSMtoSPAD(image_ptr[m],image_m+n1,(n2-n1)*sizeof(struct complexData));
  }
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
#ifdef RAG_AFL
    assert(platpos[k][0] == platpos_k_012[0]);
    assert(platpos[k][1] == platpos_k_012[1]);
    assert(platpos[k][2] == platpos_k_012[2]);
#endif
    // compute distance to R_mid
    struct complexData *Xin_k;
    Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
    // compute distance to R_mid
    double zr_mid  = 0.0f - platpos_k_012[2]; // Z
    double zr_mid2 = zr_mid * zr_mid;
    double yr_mid = RAG_GET_FLT(image_params->yr+(m1+blk_size_half)) - platpos_k_012[1]; // Y
    double yr_mid2 = yr_mid * yr_mid;
    double sqrt_arg = zr_mid2 + yr_mid2;
    double xr_mid = RAG_GET_FLT(image_params->xr+(n1+blk_size_half)) - platpos_k_012[0]; // X
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
    // compute Phi_m
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
    // compute Psi_n
    double theta_mid = ku2*R_mid;
    double arg_mid = theta_mid - blk_size_half*uy + blk_size_whole*blk_size_whole/4*vy;
    struct complexData Psi_n;
#ifdef RAG_SINCOS
    sincosf((arg_mid-2*Pi*round(arg_mid/2/Pi)), &(Psi_n.imag), &(Psi_n.real));
#else
    Psi_n.real = cosf(arg_mid-2*Pi*round(arg_mid/2/Pi));
    Psi_n.imag = sinf(arg_mid-2*Pi*round(arg_mid/2/Pi));
#endif
    // compute Gamma_m
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
      free(image_data_ptr);
      free(image_ptr);
#endif
      free(Phi_m);
      free(A_m);
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
#ifdef RAG_AFL
	    assert(platpos[k][0] == platpos_k_012[0]);
	    assert(platpos[k][1] == platpos_k_012[1]);
	    assert(platpos[k][2] == platpos_k_012[2]);
#endif
	    struct complexData *Xin_k;
	    Xin_k = (struct complexData *)RAG_GET_PTR(Xin+k);
	    float x = RAG_GET_FLT(image_params->xr+m) - platpos_k_012[0];
	    float y = RAG_GET_FLT(image_params->yr+n) - platpos_k_012[1];
	    float z =                                 - platpos_k_012[2];
	    float R = sqrtf( x*x + y*y + z*z );
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
  xe_printf("//////// leave backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
  rmd_guid_t arg_scg = { .data = rmd_arg };
  RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,NULL_GUID,slot);
  bsm_free(corners_ptr,corners_dbg);
  return NULL_GUID;
} // backproject_async

rmd_guid_t BackProj_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  int retval;
#ifdef TRACE_LVL_3
  xe_printf("////// enter BackProj_codelet\n");RAG_FLUSH;
#endif
  assert(n_db == 6);
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
  int slot = corners->slot;
  assert(slot==0);
  if(image_params->F > 1) {
    fftwf_complex *input, *fft_result, *ifft_result;
    fftwf_plan plan_forward, plan_backward;	// FFTW plan variables

#ifdef TRACE_LVL_3
    xe_printf("////// BackProj FFTW initialization F = %d\n",image_params->F);RAG_FLUSH;
#endif
    input         = (fftwf_complex*)fftwf_malloc(image_params->S3 * sizeof(fftwf_complex));
    fft_result    = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
    ifft_result   = (fftwf_complex*)fftwf_malloc(image_params->S4 * sizeof(fftwf_complex));
    plan_forward  = fftwf_plan_dft_1d(image_params->S3, input,      fft_result,  FFTW_FORWARD, FFTW_ESTIMATE);
    plan_backward = fftwf_plan_dft_1d(image_params->S4, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

    float scale = 1/(float)image_params->S4;
    struct complexData **Xup;
    rmd_guid_t Xup_dbg;
    Xup = (struct complexData**)bsm_malloc(&Xup_dbg,image_params->P3*sizeof(struct complexData*));
    if(Xup == NULL) {
      fprintf(stderr,"Error allocating edge vector for Xup.\n");
      exit(1);
    }
    struct complexData* Xup_data_ptr = NULL; rmd_guid_t Xup_data_dbg;
#ifdef RAG_AFL
    Xup_data_ptr = (struct complexData*)dram_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#else
    Xup_data_ptr = (struct complexData*) bsm_malloc(&Xup_data_dbg,image_params->P3*image_params->S4*sizeof(struct complexData));
#endif
    if ( Xup_data_ptr == NULL) {
      fprintf(stderr,"Error allocating data memory for Xup.\n");
      exit(1);
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

      memset(&fft_result[image_params->S3][0], 0,
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
    fftwf_free(input);
    fftwf_free(fft_result);
    fftwf_free(ifft_result);
    fftwf_destroy_plan(plan_forward);
    fftwf_destroy_plan(plan_backward);
#ifdef TRACE
    xe_printf("////// Performing backprojection over Ix[%d:%d] and Iy[%d:%d] (F is > 1)\n",
	      m1, m2-1, n1, n2-1);RAG_FLUSH;
#endif
    struct Inputs tmp_in, *tmp_in_ptr; rmd_guid_t tmp_in_dbg;
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

    // create a codelet for backproject_async function
    rmd_guid_t backproject_async_clg;
    retval = rmd_codelet_create(
				&backproject_async_clg,     // rmd_guid_t *new_guid
				backproject_async_codelet, // rmd_codelet_ptr func_ptr
				0,			// size_t code_size
				0,			// uinit64_t default_arg
				5,			// int n_dep
				1,			// int buffer_in
				false,			// bool gen_out
				0);			// uint64_t prop
    assert(retval==0);
    // create a codelet for backproject_finish function
    rmd_guid_t backproject_finish_clg;
    retval = rmd_codelet_create(
				&backproject_finish_clg,     // rmd_guid_t *new_guid
				backproject_finish_codelet, // rmd_codelet_ptr func_ptr
				0,			// size_t code_size
				0,			// uinit64_t default_arg
				((m2-m1)/BACK_PROJ_ASYNC_BLOCK_SIZE_M)*((n2-n1)/BACK_PROJ_ASYNC_BLOCK_SIZE_N)+1, // int n_dep
				1,			// int buffer_in
				false,			// bool gen_out
				0);			// uint64_t prop
    assert(retval==0);
    // create an instance for backproject_finish
    rmd_guid_t backproject_finish_scg;
    retval = rmd_codelet_sched(
			       &backproject_finish_scg,		// rmd_guid_t* scheduled codelet's guid
			       arg,				// uint64_t arg
			       backproject_finish_clg);		// rmd_guid_t created codelet's guid
    assert(retval==0);
    RAG_DEF_MACRO_PASS(backproject_finish_scg,NULL,NULL,NULL,NULL,in_dbg,slot++);
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_3
	xe_printf("////// create an instance for backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
	struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; rmd_guid_t async_corners_dbg;
	async_corners = &async_corners_lcl;
	async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
	async_corners->m1   = m;
	async_corners->m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners->n1   = n;
	async_corners->n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
	async_corners->slot = slot++;
	REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
	rmd_guid_t backproject_async_scg;
	retval = rmd_codelet_sched(
				   &backproject_async_scg,		// rmd_guid_t* scheduled codelet's guid
				   backproject_finish_scg.data,	// uint64_t arg
				   backproject_async_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,tmp_in_dbg,0); // Xup,platpos
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
	RMD_DB_RELEASE(async_corners_dbg);
      } // for n
    } // for m
    bsm_free(tmp_in_ptr,tmp_in_dbg);
#ifdef RAG_AFL
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
    // create a codelet for backproject_async function
    rmd_guid_t backproject_async_clg;
    retval = rmd_codelet_create(
				&backproject_async_clg,     // rmd_guid_t *new_guid
				backproject_async_codelet, // rmd_codelet_ptr func_ptr
				0,			// size_t code_size
				0,			// uinit64_t default_arg
				5,			// int n_dep
				1,			// int buffer_in
				false,			// bool gen_out
				0);			// uint64_t prop
    assert(retval==0);
    // create a codelet for backproject_finish function
    rmd_guid_t backproject_finish_clg;
    retval = rmd_codelet_create(
				&backproject_finish_clg,     // rmd_guid_t *new_guid
				backproject_finish_codelet, // rmd_codelet_ptr func_ptr
				0,			// size_t code_size
				0,			// uinit64_t default_arg
				((m2-m1)/BACK_PROJ_ASYNC_BLOCK_SIZE_M)*((n2-n1)/BACK_PROJ_ASYNC_BLOCK_SIZE_N)+1, // int n_dep
				1,			// int buffer_in
				false,			// bool gen_out
				0);			// uint64_t prop
    assert(retval==0);
    // create an instance for backproject_finish
    rmd_guid_t backproject_finish_scg;
    retval = rmd_codelet_sched(
			       &backproject_finish_scg,		// rmd_guid_t* scheduled codelet's guid
			       arg,				// uint64_t arg
			       backproject_finish_clg);		// rmd_guid_t created codelet's guid
    assert(retval==0);
    RAG_DEF_MACRO_PASS(backproject_finish_scg,NULL,NULL,NULL,NULL,in_dbg,slot++);
    for(int m=m1; m<m2; m+=BACK_PROJ_ASYNC_BLOCK_SIZE_M) {
      for(int n=n1; n<n2; n+=BACK_PROJ_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_3
	xe_printf("////// create an instance for backproject_async slot %d\n",slot);RAG_FLUSH;
#endif
	struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; rmd_guid_t async_corners_dbg;
	async_corners = &async_corners_lcl;
	async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
	async_corners->m1   = m;
	async_corners->m2   = m+BACK_PROJ_ASYNC_BLOCK_SIZE_M;
	async_corners->n1   = n;
	async_corners->n2   = n+BACK_PROJ_ASYNC_BLOCK_SIZE_N;
	async_corners->slot = slot++;
	REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
	rmd_guid_t backproject_async_scg;
	retval = rmd_codelet_sched(
				   &backproject_async_scg,		// rmd_guid_t* scheduled codelet's guid
				   backproject_finish_scg.data,	// uint64_t arg
				   backproject_async_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,in_dbg,0);	// Xin,platpos
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,1);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,radar_params_dbg,3);
	RAG_DEF_MACRO_PASS(backproject_async_scg,NULL,NULL,NULL,NULL,image_dbg,4);
	RMD_DB_RELEASE(async_corners_dbg);
      } // for n
    } // for m
    RMD_DB_RELEASE(in_dbg);
    RMD_DB_RELEASE(image_params_dbg);
    RMD_DB_RELEASE(radar_params_dbg);
    RMD_DB_RELEASE(image_dbg);
  } // if F

  bsm_free(corners_ptr,corners_dbg);
#ifdef TRACE_LVL_3
  xe_printf("////// leave BackProj_codelet\n");RAG_FLUSH;
#endif
  return NULL_GUID;
}
