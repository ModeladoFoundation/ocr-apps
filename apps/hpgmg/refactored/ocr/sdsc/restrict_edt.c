#include <ocr.h>

#include "hpgmg.h"
#include "utils.h"

// deps: level coarser
ocrGuid_t restrict_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  level_type* l = (level_type*) depv[0].ptr;
  level_type* c = (level_type*) depv[1].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[1].ptr)+c->boxes);
  ocrGuid_t fine[125];
  int count = l->num_boxes/c->num_boxes;

  ///// time restriction /////
  l->time_temp[2] = time();
  ////////////////////////////

  VERBOSEPA("restrict_level_edt %u\n", l->level);

  ocrGuid_t r,r_t;
  ocrEdtTemplateCreate(&r_t, restrict_edt, paramc, 3+count);

  // for all boxes create a restrict_edt
  int b;
  ocrGuid_t currentAffinity = NULL_GUID;
  for(b = 0; b < c->num_boxes; ++b) {
    get_fine_boxes(l,c,b,fine);
#ifdef ENABLE_EXTENSION_AFFINITY
    u64 acount = 1;
    ocrAffinityQuery(boxes[b], &acount, &currentAffinity);
#endif
    ocrEdtCreate(&r, r_t, paramc, paramv, 3+count, NULL, 0, currentAffinity, NULL);
    ocrAddDependence(depv[1].guid, r, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], r, 1, DB_MODE_RW);
    ocrAddDependence(depv[0].guid, r, 2, DB_MODE_CONST);
    int fc;
    for(fc = 0; fc < count; ++fc)
      ocrAddDependence(fine[fc], r, 3+fc, DB_MODE_CONST);
  }

  ocrEdtTemplateDestroy(r_t);

  return NULL_GUID;
}

// deps: level - coarse - level - fine_0..fine_count-1
ocrGuid_t restrict_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEPA("restrict_edt %u\n", ((level_type*) depv[2].ptr)->level);

  int o;
  int flag = 0;
  if (paramc == 2)
    flag = 1;
  if (depc <= 11) {
    for(o = 3; o < depc; ++o)
      restriction_f((level_type*)depv[0].ptr, (box_type*) depv[1].ptr,
                    (level_type*)depv[2].ptr, (box_type*) depv[o].ptr, o-3,
                    depc==4 ? ((level_type*)depv[0].ptr)->box_dim : ((level_type*)depv[0].ptr)->box_dim>>1, flag);
  } else { // when all boxes are combined

    for(o = 3; o < depc; ++o)
      restriction_f_all((level_type*)depv[0].ptr, (box_type*) depv[1].ptr,
                    (level_type*)depv[2].ptr, (box_type*) depv[o].ptr, o-3,
                    ((level_type*)depv[0].ptr)->box_dim, flag);
  }
  if(((ocrGuid_t)paramv[0]) != NULL_GUID)
    ocrEventSatisfy((ocrGuid_t)paramv[0], depv[1].guid);

  return NULL_GUID;
}

void restriction_f_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant, int dim, int flag)
{

  int sjStride = f->jStride;
  int skStride = f->kStride;
  int djStride = l->jStride;
  int dkStride = l->kStride;

  double* src;
  if (!flag)
    src = (double*)(((char*)fb)+f->f_Av) + NUM_GHOSTS * (1+sjStride+skStride);
  else
    src = (double*)(((char*)fb)+f->vec_temp) + NUM_GHOSTS * (1+sjStride+skStride);

  double* dst = (double*)(((char*)lb)+l->f_Av) +  NUM_GHOSTS * (1+djStride+dkStride);

  double* src_ptr = src;
  double* dst_ptr = dst;

  int i,j,k,ii=0;
  get_tuple(fb->global_box_id, f->boxes_in.i, f->boxes_in.j, f->boxes_in.k, &i,&j,&k);

  int ijk = i + j*djStride + k*dkStride;
  dst_ptr[ijk] = (src_ptr[ii]+src_ptr[ii+1] + src_ptr[ii+sjStride]+src_ptr[ii+1+sjStride]+src_ptr[ii+skStride]+
                       src_ptr[ii+1+skStride]+src_ptr[ii+sjStride+skStride]+src_ptr[ii+1+sjStride+skStride])*0.125;
}


void restriction_f(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant, int dim, int flag)
{

  int sjStride = f->jStride;
  int skStride = f->kStride;
  int djStride = l->jStride;
  int dkStride = l->kStride;

  double* src;
  if (!flag)
    src = (double*)(((char*)fb)+f->f_Av) + NUM_GHOSTS * (1+sjStride+skStride);
  else
    src = (double*)(((char*)fb)+f->vec_temp) + NUM_GHOSTS * (1+sjStride+skStride);

  double* dst = (double*)(((char*)lb)+l->f_Av) +  NUM_GHOSTS * (1+djStride+dkStride);

  int io,jo,ko;
  io = octant&1 ? (l->box_dim>>1) : 0;
  jo = octant&2 ? (l->box_dim>>1) : 0;
  ko = octant&4 ? (l->box_dim>>1) : 0;
  int i,j,k,ii,jj,kk;

  for(k = 0; k < dim; ++k) {
    kk=k<<1;
    for(j = 0; j < dim; ++j) {
      jj=j<<1;
      double* src_ptr = src+jj*sjStride+kk*skStride;
      double* dst_ptr = dst+io+(j+jo)*djStride+(k+ko)*dkStride;
      for(i = 0; i < dim; ++i) {
        ii=i<<1;
         dst_ptr[i] = (src_ptr[ii]+src_ptr[ii+1] + src_ptr[ii+sjStride]+src_ptr[ii+1+sjStride]+src_ptr[ii+skStride]+
                       src_ptr[ii+1+skStride]+src_ptr[ii+sjStride+skStride]+src_ptr[ii+1+sjStride+skStride])*0.125;
      }
    }
  }
}


// side = 0 (cell) 1 (face I), 2 (face J), 3 (face K)
void restrict_generic(level_type* l, box_type* lb, int l_off, level_type* f, box_type* fb, int f_off, int octant, int dim, int side)
{

  int sjStride = f->jStride;
  int skStride = f->kStride;
  int djStride = l->jStride;
  int dkStride = l->kStride;

  double* src = (double*)(((char*)fb)+f_off) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)lb)+l_off) +  NUM_GHOSTS * (1+djStride+dkStride);

  int io,jo,ko;
  io = octant&1 ? (l->box_dim>>1) : 0;
  jo = octant&2 ? (l->box_dim>>1) : 0;
  ko = octant&4 ? (l->box_dim>>1) : 0;
  int i,j,k,ii,jj,kk;

  if (side == 0) {
    for(k = 0; k < dim; ++k) {
      kk=k<<1;
      for(j = 0; j < dim; ++j) {
        jj=j<<1;
        double* src_ptr = src+jj*sjStride+kk*skStride;
        double* dst_ptr = dst+io+(j+jo)*djStride+(k+ko)*dkStride;
        for(i = 0; i < dim; ++i) {
          ii=i<<1;
          dst_ptr[i] = (src_ptr[ii]+src_ptr[ii+1] + src_ptr[ii+sjStride]+src_ptr[ii+1+sjStride]+src_ptr[ii+skStride]+
                       src_ptr[ii+1+skStride]+src_ptr[ii+sjStride+skStride]+src_ptr[ii+1+sjStride+skStride])*0.125;
        }
      }
    }
  } else if (side == 1) {
    for(k = 0; k < dim; ++k) {
      kk=k<<1;
      for(j = 0; j < dim; ++j) {
        jj=j<<1;
        double* src_ptr = src+jj*sjStride+kk*skStride;
        double* dst_ptr = dst+io+(j+jo)*djStride+(k+ko)*dkStride;
        for(i = 0; i <= dim; ++i) {
          ii=i<<1;
          dst_ptr[i] = (src_ptr[ii]+src_ptr[ii+sjStride]+src_ptr[ii+skStride]+src_ptr[ii+sjStride+skStride])*0.25;
        }
      }
    }
  } else if (side == 2) {
    for(k = 0; k < dim; ++k) {
      kk=k<<1;
      for(j = 0; j <= dim; ++j) {
        jj=j<<1;
        double* src_ptr = src+jj*sjStride+kk*skStride;
        double* dst_ptr = dst+io+(j+jo)*djStride+(k+ko)*dkStride;
        for(i = 0; i < dim; ++i) {
          ii=i<<1;
          dst_ptr[i] = (src_ptr[ii]+src_ptr[ii+1]+src_ptr[ii+skStride]+src_ptr[ii+1+skStride])*0.25;
        }
      }
    }
  } else if (side == 3) {
    for(k = 0; k <= dim; ++k) {
      kk=k<<1;
      for(j = 0; j < dim; ++j) {
        jj=j<<1;
        double* src_ptr = src+jj*sjStride+kk*skStride;
        double* dst_ptr = dst+io+(j+jo)*djStride+(k+ko)*dkStride;
        for(i = 0; i < dim; ++i) {
          ii=i<<1;
          dst_ptr[i] = (src_ptr[ii]+src_ptr[ii+sjStride]+src_ptr[ii+1]+src_ptr[ii+sjStride+1])*0.25;
        }
      }
    }
  }
}

// side = 0 (cell) 1 (face I), 2 (face J), 3 (face K)
void restrict_generic_all(level_type* l, box_type* lb, int l_off, level_type* f, box_type* fb, int f_off, int side) {
  int sjStride = f->jStride;
  int skStride = f->kStride;
  int djStride = l->jStride;
  int dkStride = l->kStride;

  double* src = (double*)(((char*)fb)+f_off) + NUM_GHOSTS * (1+sjStride+skStride);
  double* dst = (double*)(((char*)lb)+l_off) +  NUM_GHOSTS * (1+djStride+dkStride);

  double* src_ptr = src;
  double* dst_ptr = dst;
  int i,j,k,ii=0;
  get_tuple(fb->global_box_id, f->boxes_in.i, f->boxes_in.j, f->boxes_in.k, &i,&j,&k);
  if (side == 0) {

    int ijk = i + j*djStride + k*dkStride;
    dst_ptr[ijk] = (src_ptr[ii]+src_ptr[ii+1] + src_ptr[ii+sjStride]+src_ptr[ii+1+sjStride]+src_ptr[ii+skStride]+
                       src_ptr[ii+1+skStride]+src_ptr[ii+sjStride+skStride]+src_ptr[ii+1+sjStride+skStride])*0.125;

  } else if (side == 1) {
     int ijk = i + j*djStride + k*dkStride;
     dst_ptr[ijk] = (src_ptr[ii]+src_ptr[ii+sjStride]+src_ptr[ii+skStride]+src_ptr[ii+sjStride+skStride])*0.25;
     if (i == (l->box_dim-1))
       dst_ptr[ijk+1] = (src_ptr[ii+2]+src_ptr[ii+2+sjStride]+src_ptr[ii+2+skStride]+src_ptr[ii+2+sjStride+skStride])*0.25;
  } else if (side == 2) {
     int ijk = i + j*djStride + k*dkStride;
     dst_ptr[ijk] = (src_ptr[ii]+src_ptr[ii+1]+src_ptr[ii+skStride]+src_ptr[ii+1+skStride])*0.25;
     if (j == (l->box_dim-1))
       dst_ptr[ijk+djStride] = (src_ptr[ii+2*sjStride]+src_ptr[ii+2*sjStride+1]+src_ptr[ii+2*sjStride+skStride]+src_ptr[ii+2*sjStride+1+skStride])*0.25;
  } else if (side == 3) {
     int ijk = i + j*djStride + k*dkStride;
     dst_ptr[ijk] = (src_ptr[ii]+src_ptr[ii+sjStride]+src_ptr[ii+1]+src_ptr[ii+sjStride+1])*0.25;
     if (k == (l->box_dim-1))
       dst_ptr[ijk+dkStride] = (src_ptr[ii+2*skStride]+src_ptr[ii+2*skStride+sjStride]+src_ptr[ii+2*skStride+1]+src_ptr[ii+2*skStride+sjStride+1])*0.25;
  }
}

