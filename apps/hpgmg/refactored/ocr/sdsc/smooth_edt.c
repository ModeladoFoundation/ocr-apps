#include <ocr.h>
#include <extensions/ocr-affinity.h>

#include "hpgmg.h"
#include "operators.h"
#include "utils.h"

// deps: level
ocrGuid_t smooth_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("smooth_level_edt\n");
  level_type* l = (level_type*) depv[0].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[0].ptr)+l->boxes);

  ///// time smooth operator /////
  l->time_temp[0] = time();
  ////////////////////////////////

  ocrGuid_t r,r_t;
  ocrEdtTemplateCreate(&r_t, smooth_edt, 1, 2); // Manu: added paramc = 1

  u64 s = paramv[0];


  // for all boxes create a restrict_edt
  int b;
  ocrGuid_t currentAffinity = NULL_GUID;
  for(b = 0; b < l->num_boxes; ++b) {
#ifdef ENABLE_EXTENSION_AFFINITY
     u64 count = 1;
     ocrAffinityQuery(boxes[b], &count, &currentAffinity);
#endif
    ocrEdtCreate(&r, r_t, 1, &s, 2, NULL, 0, currentAffinity, NULL); // Manu: added paramc = 1
    ocrAddDependence(depv[0].guid, r, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], r, 1, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(r_t);

  return NULL_GUID;
}

// deps: level - box
ocrGuid_t smooth_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("smooth_edt\n");
  level_type* level = (level_type*) depv[0].ptr;
  box_type *box = (box_type*) depv[1].ptr;

  u64 s = paramv[0];
#if DEBUG
PRINTF("s = %u\n", s);
#endif

  // Manu:: these are passed as function args in Sam's code
  double a=0.0, b=1.0;
  double *rhs_id = (double*)((char*)box+ level->f_Av);
  double *x_id = (double*)((char*)box+ level->u);
  double *vec_temp = (double*)((char*)box+ level->vec_temp);

  if((CHEBYSHEV_DEGREE*NUM_SMOOTHS)&1){
    PRINTF("Error: CHEBYSHEV_DEGREE*NUM_SMOOTHS must be even for the chebyshev smoother\n");
    ABORT(0);
  }

  int s1;
  double beta     = 1.000*level->dominant_eigenvalue_of_DinvA;
  double alpha1    = 0.125000*beta;
  double theta    = 0.5*(beta+alpha1);
  double delta    = 0.5*(beta-alpha1);
  double sigma = theta/delta;
  double rho_n = 1/sigma;
  double chebyshev_c1[CHEBYSHEV_DEGREE];
  double chebyshev_c2[CHEBYSHEV_DEGREE];
  chebyshev_c1[0] = 0.0;
  chebyshev_c2[0] = 1/theta;
  for(s1=1;s1<CHEBYSHEV_DEGREE;s1++){
    double rho_nm1 = rho_n;
    rho_n = 1.0/(2.0*sigma - rho_nm1);
    chebyshev_c1[s1] = rho_n*rho_nm1;
    chebyshev_c2[s1] = rho_n*2.0/delta;
  }
#if DEBUG
  PRINTF("level = %u, box = %u, level->volume = %u\n", level->level, box->global_box_id, level->volume);
#endif


  // apply the smoother... Chebyshev ping pongs between x_id and VECTOR_TEMP
  int i,j,k;
  int ghosts = NUM_GHOSTS;
  const int jStride = level->jStride;
  const int kStride = level->kStride;
  const int     dim = level->box_dim;
  const double h2inv = 1.0/(level->h*level->h);
  const double * __restrict__ rhs      = rhs_id + ghosts*(1+jStride+kStride);
  const double * __restrict__ alpha    = (double*)((char*)box+ level->alpha) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_i   = (double*)((char*)box+ level->beta_i) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_j   = (double*)((char*)box+ level->beta_j) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_k   = (double*)((char*)box+ level->beta_k) + ghosts*(1+jStride+kStride);
  const double * __restrict__ Dinv     = (double*)((char*)box+ level->Dinv) + ghosts*(1+jStride+kStride);
  const double * __restrict__ valid    = (double*)((char*)box+ level->valid) + ghosts*(1+jStride+kStride); // cell is inside the domain

        double * __restrict__ x_np1;
  const double * __restrict__ x_n;
  const double * __restrict__ x_nm1;
  if((s&1)==0) {
    x_n = x_id + ghosts*(1+jStride+kStride);
    x_nm1 = vec_temp + ghosts*(1+jStride+kStride);
    x_np1 = vec_temp + ghosts*(1+jStride+kStride);
  } else {
    x_n = vec_temp + ghosts*(1+jStride+kStride);
    x_nm1 = x_id + ghosts*(1+jStride+kStride);
    x_np1 = x_id + ghosts*(1+jStride+kStride);
  }

  const double c1 = chebyshev_c1[s%CHEBYSHEV_DEGREE]; // limit polynomial to degree CHEBYSHEV_DEGREE.
  const double c2 = chebyshev_c2[s%CHEBYSHEV_DEGREE]; // limit polynomial to degree CHEBYSHEV_DEGREE.
  for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
      for(i=0;i<dim;i++){
        int ijk = i + j*jStride + k*kStride;
        double Ax_n   = apply_op_ijk(x_n);
        double lambda =     Dinv_ijk();
        x_np1[ijk] = x_n[ijk] + c1*(x_n[ijk]-x_nm1[ijk]) + c2*lambda*(rhs[ijk]-Ax_n);
      }
    }
  }

  return NULL_GUID;

}
