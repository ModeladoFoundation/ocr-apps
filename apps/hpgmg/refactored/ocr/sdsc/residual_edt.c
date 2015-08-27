#include <ocr.h>

#include "hpgmg.h"
#include "utils.h"

// deps: level
ocrGuid_t residual_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("residual_level_edt\n");
  level_type* l = (level_type*) depv[0].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)(((char*)depv[0].ptr)+l->boxes);

  //// time residual ////
  l->time_temp[1] = time();
  ///////////////////////

  ocrGuid_t r,r_t;
  ocrEdtTemplateCreate(&r_t, residual_edt, paramc, 2);

  // for all boxes create a restrict_edt
  int b;
  ocrGuid_t currentAffinity = NULL_GUID;
  for(b = 0; b < l->num_boxes; ++b) {
#ifdef ENABLE_EXTENSION_AFFINITY
    u64 count = 1;
    ocrAffinityQuery(boxes[b], &count, &currentAffinity);
#endif
    ocrEdtCreate(&r, r_t, paramc, paramv, 2, NULL, 0, currentAffinity, NULL);
    ocrAddDependence(depv[0].guid, r, 0, DB_MODE_CONST);
    ocrAddDependence(boxes[b], r, 1, DB_MODE_RW);
  }

  ocrEdtTemplateDestroy(r_t);

  return NULL_GUID;
}

// deps: level - box
ocrGuid_t residual_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("residual_edt\n");

  level_type* l = (level_type*) depv[0].ptr;
  box_type *b = (box_type*)depv[1].ptr;

  double *rhs = (double*)((char*)b+ l->f_Av);
  double *x = (double*)((char*)b+ l->u);
  double *res = (double*)((char*)b+ l->vec_temp);

  if (paramc == 1) {
    rhs = (double*)((char*)b+ l->f);
  }

  residual_coarse(l, b, res, x, rhs,0,1); // a = 0, b = 1

  return NULL_GUID;
}
