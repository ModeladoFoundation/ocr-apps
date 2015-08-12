#include <ocr.h>

#include "hpgmg.h"
#include "utils.h"
#include <string.h>


// deps: level finer
// par: interpolation type
ocrGuid_t interpolate_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("interpolate_level_edt\n");

  level_type* l = (level_type*) depv[1].ptr;  // coarse
  level_type* f = (level_type*) depv[0].ptr;  // fine


  ///// time interpolate /////
  l->time_temp[3] = time();
  ////////////////////////////

  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[1].ptr)+l->boxes);
  ocrGuid_t fine[125];
  int count = f->num_boxes/l->num_boxes;


  ocrGuid_t i,i_t;
  ocrEdtTemplateCreate(&i_t, interpolate_edt, 1, 3+count);

  // for all boxes create a restrict_edt
  int b;
  ocrGuid_t currentAffinity = NULL_GUID;
  for(b = 0; b < l->num_boxes; ++b) {
#ifdef ENABLE_EXTENSION_AFFINITY
    u64 acount = 1;
    ocrAffinityQuery(boxes[b], &acount, &currentAffinity);
#endif
    get_fine_boxes(f,l,b,fine);
    ocrEdtCreate(&i, i_t, 1, paramv, 3+count, NULL, 0, currentAffinity, NULL);
    ocrAddDependence(depv[1].guid, i, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], i, 1, DB_MODE_CONST);
    ocrAddDependence(depv[0].guid, i, 2, DB_MODE_CONST);
    int fc;
    for(fc = 0; fc < count; ++fc)
      ocrAddDependence(fine[fc], i, 3+fc, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(i_t);

  return NULL_GUID;
}

// deps: level - box - level - fine_0..fine_count-1
// par: interpolation type
ocrGuid_t interpolate_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("interpolate_edt\n");

  ASSERT(paramv[0]==VC_INTERPOLATE || paramv[0]==FMG_INTERPOLATE);

  if (depc <= 11) {
    void (*interpolation_f)(level_type*, box_type*, level_type*, box_type*, int)
      = paramv[0]==VC_INTERPOLATE ? interpolation_pc : interpolation_pl;

    int o;
    for(o = 3; o < depc; ++o)
      interpolation_f((level_type*)depv[0].ptr, (box_type*) depv[1].ptr,
                      (level_type*)depv[2].ptr, (box_type*) depv[o].ptr, o-3);
  } else {
    void (*interpolation_f_others)(level_type*, box_type*, level_type*, box_type*, int)
      = paramv[0]==VC_INTERPOLATE ? interpolation_pc_all : interpolation_pl_all;

    int o;
    for(o = 3; o < depc; ++o)
      interpolation_f_others((level_type*)depv[0].ptr, (box_type*) depv[1].ptr,
                      (level_type*)depv[2].ptr, (box_type*) depv[o].ptr, o-3);
  }

  return NULL_GUID;
}

void interpolation_pc(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant)
{
  int sjStride = l->jStride;
  int skStride = l->kStride;
  int djStride = f->jStride;
  int dkStride = f->kStride;

  double* src = (double*)(((char*)lb)+l->u) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)fb)+f->u) + NUM_GHOSTS * (1+djStride+dkStride);

  int io,jo,ko;
  io = octant&1 ? (l->box_dim>>1) : 0;
  jo = octant&2 ? (l->box_dim>>1) : 0;
  ko = octant&4 ? (l->box_dim>>1) : 0;
  int i,j,k,ii,jj,kk,c;
  for(k = 0; k < f->box_dim; ++k) {
    kk=ko+((k)>>1);
    for(j = 0; j < f->box_dim; ++j) {
      jj=jo+((j)>>1);
      double* src_ptr = src+jj*sjStride+kk*skStride;
      double* dst_ptr = dst+j*djStride+k*dkStride;
      for(i = 0; i < f->box_dim; ++i) {
        ii=io+((i)>>1);
        dst_ptr[i] = 1.0*dst_ptr[i] +  src_ptr[ii];
      }
    }
  }
}

void interpolation_pl(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant)
{
  int sjStride = l->jStride;
  int skStride = l->kStride;
  int djStride = f->jStride;
  int dkStride = f->kStride;

  double* src = (double*)(((char*)lb)+l->u) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)fb)+f->u) + NUM_GHOSTS * (1+djStride+dkStride);

  apply_bcs(l,lb,src);

  int io,jo,ko;
  io = octant&1 ? (l->box_dim>>1) : 0;
  jo = octant&2 ? (l->box_dim>>1) : 0;
  ko = octant&4 ? (l->box_dim>>1) : 0;
  int i,j,k,ii,jj,kk,c;
  for(k = 0; k < f->box_dim; ++k) {
    kk=ko+((k)>>1);
    int delta_k = k&1? skStride:-skStride;
    for(j = 0; j < f->box_dim; ++j) {
      int delta_j = j&1? sjStride:-sjStride;
      jj=jo+((j)>>1);
      double* src_ptr = src+jj*sjStride+kk*skStride;
      double* dst_ptr = dst+j*djStride+k*dkStride;
      for(i = 0; i < f->box_dim; ++i) {
        int delta_i = i&1 ? 1:-1;
        ii=io+((i)>>1);
        dst_ptr[i] = 0.421875*src_ptr[ii]                +0.140625*src_ptr[ii+delta_k]+
                     0.140625*src_ptr[ii+delta_j]        +0.046875*src_ptr[ii+delta_j+delta_k]+
                     0.140625*src_ptr[ii+delta_i]        +0.046875*src_ptr[ii+delta_i+delta_k]+
                     0.046875*src_ptr[ii+delta_i+delta_j]+0.015625*src_ptr[ii+delta_i+delta_j+delta_k];
      }
    }
  }
}



void interpolation_pc_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant) {
  int sjStride = l->jStride;
  int skStride = l->kStride;
  int djStride = f->jStride;
  int dkStride = f->kStride;

  double* src = (double*)(((char*)lb)+l->u) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)fb)+f->u) + NUM_GHOSTS * (1+djStride+dkStride);

  int x,y,z;
  get_tuple(fb->global_box_id, f->boxes_in.i, f->boxes_in.j, f->boxes_in.k, &x,&y,&z);
  int i,j,k,ii,jj,kk,c;
  int ijk = x + y*sjStride + z*skStride;
  for(k = 0; k < f->box_dim; ++k) {
    for(j = 0; j < f->box_dim; ++j) {
      double* dst_ptr = dst+j*djStride+k*dkStride;
      for(i = 0; i < f->box_dim; ++i) {
        dst_ptr[i] = 1.0*dst_ptr[i] +  src[ijk];
      }
    }
  }
}

void interpolation_pl_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant) {
  int sjStride = l->jStride;
  int skStride = l->kStride;
  int djStride = f->jStride;
  int dkStride = f->kStride;

  double* src = (double*)(((char*)lb)+l->u) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)fb)+f->u) + NUM_GHOSTS * (1+djStride+dkStride);

  int x,y,z;
  get_tuple(fb->global_box_id, f->boxes_in.i, f->boxes_in.j, f->boxes_in.k, &x,&y,&z);
  int ijk = x + y*sjStride + z*skStride;

  apply_bcs(l,lb,src);

  int i,j,k,ii,jj,kk,c;
  for(k = 0; k < f->box_dim; ++k) {
    int delta_k = k&1? skStride:-skStride;
    for(j = 0; j < f->box_dim; ++j) {
      int delta_j = j&1? sjStride:-sjStride;
      double* dst_ptr = dst+j*djStride+k*dkStride;
      for(i = 0; i < f->box_dim; ++i) {
        int delta_i = i&1 ? 1:-1;
        dst_ptr[i] = 0.421875*src[ijk]                +0.140625*src[ijk+delta_k]+
                     0.140625*src[ijk+delta_j]        +0.046875*src[ijk+delta_j+delta_k]+
                     0.140625*src[ijk+delta_i]        +0.046875*src[ijk+delta_i+delta_k]+
                     0.046875*src[ijk+delta_i+delta_j]+0.015625*src[ijk+delta_i+delta_j+delta_k];
      }
    }
  }
}
