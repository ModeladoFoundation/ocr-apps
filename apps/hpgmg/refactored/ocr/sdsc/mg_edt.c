#include <ocr.h>

#include "hpgmg.h"
#include <string.h>
#include <math.h>
#include "utils.h"

#ifdef TG_ARCH
#include "strings.h"
#endif



// deps: level0
ocrGuid_t init_ur_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("init_ur_level_edt\n");
  level_type* l = (level_type*)depv[0].ptr;
  ocrGuid_t i_t, i;
  ocrEdtTemplateCreate(&i_t, init_ur_edt, 4, 2);
  u64 pv[4] = {l->u, l->f_Av, l->f, paramv[0]};
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)l)+l->boxes);

  int b;
  for(b = 0; b < l->num_boxes; ++b) {
    ocrEdtCreate(&i, i_t, 4, pv, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, i, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], i, 1, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(i_t);

  return NULL_GUID;
}


// deps: level0
ocrGuid_t time_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  level_type* l = (level_type*)depv[0].ptr;

  if (paramv[0] == 4) { // total time - stored in level[0]
    if (fabs(l->time_temp[4]) <= mu) {
      l->time_temp[4] = time();
    } else {
        l->time_operators[4] += time() - l->time_temp[4];
        PRINTF("Time = %f\n", time() - l->time_temp[4]);
        l->time_temp[4] = 0.0;
    }
  } else {
    if (fabs(l->time_temp[paramv[0]]) > mu) {
      l->time_operators[paramv[0]] += (time() - l->time_temp[paramv[0]]);
      l->time_temp[paramv[0]] = 0.0;
    }
  }

/*
  if (paramv[0] == 0) {  // smooth time per level
   if (fabs(l->time_temp[0]) > mu) {
     l->time_operators[0] += (time() - l->time_temp[0]);
     l->time_temp[0] = 0.0;
   }
  } else if (paramv[0] == 1) { // residual time per level
   if (fabs(l->time_temp[1]) > mu) {
     l->time_operators[1] += (time() - l->time_temp[1]);
     l->time_temp[1] = 0.0;
   }
  } else if (paramv[0] == 2) { // restriction time per level
    if (fabs(l->time_temp[2]) > mu) {
     l->time_operators[2] += (time() - l->time_temp[2]);
     l->time_temp[2] = 0.0;
    }
  } else if (paramv[0] == 3) { // interpolation time per level
   if (fabs(l->time_temp[3]) > mu) {
     l->time_operators[3] += (time() - l->time_temp[3]);
     l->time_temp[3] = 0.0;
   }
  } else if (paramv[0] == 4) { // total time - stored in level[0]
    if (fabs(l->time_temp[4]) <= mu) {
//      l->time_operators[4] = time();
      l->time_temp[4] = time();
    } else {
//      l->time_operators[4] += time()-l->time_operators[4];
        l->time_operators[4] = time() - l->time_temp[4];
        l->time_temp[4] = 0.0;
        PRINTF("Time = %f\n", l->time_operators[4]);
    }

  }
*/

  return NULL_GUID;
}



// deps: level0 - box
// pars: offset u - offset f_Av - offset f
// initializes vector u to 0 and f_Av to f
ocrGuid_t init_ur_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("init_ur_edt\n");

  level_type* l = (level_type*)depv[0].ptr;
  box_type *b  = (box_type *)depv[1].ptr;
  double *u = (double *)(((char*)b)+ paramv[0]);
  double *f_Av_g = (double *)(((char*)b)+ paramv[1]);
  double *f_g = (double *)(((char*)b)+ paramv[2]);


  double *f_Av = f_Av_g + NUM_GHOSTS * (1 + l->jStride + l->kStride); // ignore the ghost zones
  double *f = f_g + NUM_GHOSTS * (1 + l->jStride + l->kStride);

  // initialize u to 0, including ghost zones
  bzero(u, (l->volume)*sizeof(double));

  // only if coarsest grid. This is similar to initializing u zero in Sam's code
  if (l->level == (paramv[3]-1))
    return NULL_GUID;


  // initialize f_Av to f. Ghost zones NOT included
  int i,j,k;
//  static double sum1 = 0.0, sum2 = 0.0;
  for (k = 0; k <  l->box_dim; k++)
    for (j = 0; j <  l->box_dim; j++)
      for (i = 0; i < l->box_dim; i++) {
        int ijk = i + j * l->jStride + k * l->kStride;
        f_Av[ijk] = f[ijk];
      }

  return NULL_GUID;
}


// deps: mg_level, level0, all boxes
// pars: u, u_true, vec_temp
ocrGuid_t finalize_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("finalize_edt\n");

  level_type* l = (level_type*)depv[1].ptr;
  int n = l->num_boxes;
  int ii,i,j,k;
  int jStride, kStride;
  double max_norm = 0.0;

  jStride = l->jStride;
  kStride = l->kStride;


  // compute max norm
  for (ii = 0; ii < n; ii++) {
    box_type *b  = (box_type *)depv[ii+3].ptr;
    double *u_g = (double *)(((char*)b)+ paramv[0]);
    double *u_true_g = (double *)(((char*)b)+ paramv[1]);
    double *vec_temp_g = (double *)(((char*)b)+ paramv[2]);
    double *u = u_g + NUM_GHOSTS * (1 + l->jStride + l->kStride);
    double *u_true = u_true_g + NUM_GHOSTS * (1 + l->jStride + l->kStride);
    double *vec_temp = vec_temp_g + NUM_GHOSTS * (1 + l->jStride + l->kStride);

    for (i = 0; i < l->box_dim; i++) {
      for (j = 0; j < l->box_dim; j++) {
        for (k = 0; k < l->box_dim; k++) {
	  int ijk = i + j*jStride + k*kStride;
	  vec_temp[ijk] = u[ijk] - u_true[ijk];
	  if (max_norm < fabs(vec_temp[ijk]))
	    max_norm = fabs(vec_temp[ijk]);
	}
      }
    }
  }
  PRINTF("h = %f  ||error|| = %22.15f\n\n", l->h, max_norm);

  PRINTF("Time = %22f\n", l->time_operators[4]/TIMED);

  ocrDbDestroy(depv[0].guid);
  ocrShutdown();

  return NULL_GUID;
}


ocrGuid_t print_timing_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

  int i, j, n;
  n = paramv[0];

  ocrGuid_t tmp;
  double *mat;
  ocrDbCreate(&tmp, (void **)&mat, sizeof(double)*n*4,0,NULL_GUID,NO_ALLOC);

  for (i = 0; i < n; i++) {
    level_type *l  = (level_type *)depv[i].ptr;
    mat[i] = l->time_operators[0]/TIMED;
    mat[i+n] = l->time_operators[1]/TIMED;
    mat[i+2*n] = l->time_operators[2]/TIMED;
    mat[i+3*n] = l->time_operators[3]/TIMED;
  }

  PRINTF("             ");
  for (i=0;i<n;i++)
   PRINTF(" %f  ", ((level_type *)depv[i].ptr)->h);

  PRINTF("\n-----------------------------------------------------------------------\n");

  PRINTF("Smooth:      ");
  for (i=0;i<n;i++)
     PRINTF(" %f  ", mat[i+0*n]);

  PRINTF("\nResidual:    ");
  for (i=0;i<n;i++)
     PRINTF(" %f  ", mat[i+1*n]);

  PRINTF("\nRestriction: ");
  for (i=0;i<n;i++)
     PRINTF(" %f  ", mat[i+2*n]);

  PRINTF("\nInterpolate: ");
  for (i=0;i<n;i++)
     PRINTF(" %f  ", mat[i+3*n]);

  PRINTF("\n\nTotal Time = %10f\n", ((level_type*)depv[0].ptr)->time_operators[4]/TIMED);
  ocrDbDestroy(tmp);

  return NULL_GUID;
}



// deps: level0
ocrGuid_t mulv_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("mulv_level_edt\n");
  level_type* l = (level_type*)depv[0].ptr;
  ocrGuid_t m_t, m;
  ocrEdtTemplateCreate(&m_t, mulv_edt, 3, 2);
  u64 pv[3] = {l->u, l->f_Av, l->f};
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)l)+l->boxes);

  int b;
  for(b = 0; b < l->num_boxes; ++b) {
    ocrEdtCreate(&m, m_t, 3, pv, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, m, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], m, 1, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(m_t);

  return NULL_GUID;
}

// deps: level0 - box
// pars: offset u - offset f_Av - offset f
ocrGuid_t mulv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("mulv_edt\n");

  level_type* l = (level_type*)depv[0].ptr;
  box_type *b = (box_type*)depv[1].ptr;

  double *vec_temp = (double*)((char*)b+ l->vec_temp);
  double *Dinv = (double*)((char*)b+ l->Dinv);

  mul_vectors(l,vec_temp, 1.0, vec_temp, Dinv);

  return NULL_GUID;
}

// deps: level0
ocrGuid_t norm_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("norm_level_edt\n");
  level_type* l = (level_type*)depv[0].ptr;
  ocrGuid_t n_t, n;
  ocrEdtTemplateCreate(&n_t, norm_edt, 3, 2);
  u64 pv[3] = {l->u, l->f_Av, l->f};
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)l)+l->boxes);

  int b;
  for(b = 0; b < l->num_boxes; ++b) {
    ocrEdtCreate(&n, n_t, 3, pv, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, n, 0, DB_MODE_RW);
    ocrAddDependence(boxes[b], n, 1, DB_MODE_CONST);
  }

  ocrEdtTemplateDestroy(n_t);

  return NULL_GUID;
}


ocrGuid_t norm_final_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("norm_level_edt\n");
  level_type* l = (level_type*)depv[0].ptr;
  double *box_norms = (double*)(((char*)l) + l->b_norms);

  int b;
  double max_norm = 0.0;
  for(b = 0; b < l->num_boxes; ++b) {
    if (max_norm < box_norms[b])
     max_norm = box_norms[b];
  }

  PRINTF("f-cycle,    norm=%22.20f\n",max_norm);

  return NULL_GUID;
}


// deps: level0 - box
// pars: offset u - offset f_Av - offset f
ocrGuid_t norm_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("norm_edt\n");

  level_type* l = (level_type*)depv[0].ptr;
  box_type* b = (box_type*)depv[1].ptr;
  double *temp = (double*)((char*)b+ l->vec_temp);
  double *box_norms = (double*)(((char*)l) + l->b_norms);

  box_norms[b->global_box_id]  =  norm_coarse(l, temp);

  return NULL_GUID;
}

// deps: level
ocrGuid_t exchange_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{

  VERBOSEP("exchange_level_edt\n");
  level_type* l = (level_type*) depv[0].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[0].ptr)+l->boxes);

  ///// time smooth operator /////
  if (fabs(l->time_temp[0]) >= mu) {
    l->time_operators[0] += (time() - l->time_temp[0]);
    l->time_temp[0] = 0.0;
  }
  ////////////////////////////////


  ocrGuid_t neighbors[26];

  u64 iter = paramv[0];


  ocrGuid_t x,x_t;
  // for all boxes create a exchange_edt
  int b;
  if (paramc == 1) {
   ocrEdtTemplateCreate(&x_t, exchange_edt, 1, 8);

   for(b = 0; b < l->num_boxes; ++b) {
      get_neighbor_guids(b,l,neighbors);
      ocrEdtCreate(&x, x_t, 1, &iter, 8, NULL, 0, NULL_GUID, NULL);
      ocrAddDependence(depv[0].guid, x, 0, DB_MODE_CONST);
      ocrAddDependence(boxes[b], x, 1, DB_MODE_RW);
      ocrAddDependence(neighbors[0], x, 2, DB_MODE_CONST);
      ocrAddDependence(neighbors[1], x, 3, DB_MODE_CONST);
      ocrAddDependence(neighbors[2], x, 4, DB_MODE_CONST);
      ocrAddDependence(neighbors[3], x, 5, DB_MODE_CONST);
      ocrAddDependence(neighbors[4], x, 6, DB_MODE_CONST);
      ocrAddDependence(neighbors[5], x, 7, DB_MODE_CONST);
    }
  } else if (paramc == 2) {
   ocrEdtTemplateCreate(&x_t, exchange_edt, paramc, 28);
    for(b = 0; b < l->num_boxes; ++b) {
      get_neighbor_guids_all(b,l,neighbors);
      ocrEdtCreate(&x, x_t, paramc, paramv, 28, NULL, 0, NULL_GUID, NULL);
      ocrAddDependence(depv[0].guid, x, 0, DB_MODE_CONST);
      ocrAddDependence(boxes[b], x, 1, DB_MODE_RW);
      int ii;
      for (ii=0;ii<26;ii++) {
        ocrAddDependence(neighbors[ii], x, ii+2, DB_MODE_CONST);
      }
    }
  }

  ocrEdtTemplateDestroy(x_t);

  return NULL_GUID;
}

// deps: level - box - neighbor0 - neighbor1 - neighbor2
// deps:               neighbor3 - neighbor4 - neighbor5
ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("exchange_edt\n");
  level_type* l = (level_type*) depv[0].ptr;

  box_type *nb[26];
  box_type* b = (box_type*) depv[1].ptr;
  double *x_id = (double*)((char*)b+ l->u) +  NUM_GHOSTS * (1+l->jStride+l->kStride);
  double *vec_temp = (double*)((char*)b+ l->vec_temp) +  NUM_GHOSTS * (1+l->jStride+l->kStride);
  int i,j,k;

  if (paramc == 1) {

    nb[0] = (box_type*) depv[2].ptr;
    nb[1] = (box_type*) depv[3].ptr;
    nb[2] = (box_type*) depv[4].ptr;
    nb[3] = (box_type*) depv[5].ptr;
    nb[4] = (box_type*) depv[6].ptr;
    nb[5] = (box_type*) depv[7].ptr;
    u64 iter = paramv[0];

  #ifndef STENCIL_FUSE_BC
    if ((iter&1) == 0)
      apply_bcs(l,b,x_id);
    else
      apply_bcs(l,b,vec_temp);
  #endif

    for (i=0;i<6;i++) {
      if (nb[i]->global_box_id == -1) {
        continue;
      } else {
        if ((iter&1) == 0)
          populate_boundary(l,x_id,nb[i],l->u,i);
        else
          populate_boundary(l,vec_temp,nb[i],l->vec_temp,i);
      }
    }
  } else if (paramc == 2) {
    int i;
    for (i =0;i<26;i++) {
      nb[i] = (box_type*) depv[i+2].ptr;
      if (nb[i]->global_box_id == -1)
        continue;
      update_boundary_all(l,b,l->u,nb[i],l->u,0);
    }
 }
  return NULL_GUID;
}



ocrGuid_t zero_vector_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  level_type* l = (level_type*) depv[0].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[0].ptr)+l->boxes);

  ocrGuid_t r,r_t;
  ocrEdtTemplateCreate(&r_t, zero_vector_edt, paramc, 2);

  // for all boxes create a restrict_edt
  int b;
  for(b = 0; b < l->num_boxes; ++b) {
    ocrEdtCreate(&r, r_t, paramc, paramv, 2, NULL, 0, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, r, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], r, 1, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(r_t);

  return NULL_GUID;
}

// deps: level - box
ocrGuid_t zero_vector_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{


  level_type* l = (level_type*) depv[0].ptr;
  box_type *b = (box_type*)depv[1].ptr;
  if( paramv[0] == 0) {
    double *x = (double*)((char*)b+ l->u);
    bzero(x,l->volume*sizeof(double));
  }
  return NULL_GUID;
}

