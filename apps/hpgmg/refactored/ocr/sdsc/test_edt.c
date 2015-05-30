#include <ocr.h>

#include "hpgmg.h"

ocrGuid_t test_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t test_box_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t test0_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  PRINTF("Test edt: depc is %d\n", depc);
  int i;
  mg_type* mg_ptr =  (mg_type*)depv[0].ptr;
  for (i=0;i<10;i++)
    PRINTF("%d, ", i);
  PRINTF("\n");

  for (i=0;i<mg_ptr->max_levels;i++)
    PRINTF("level%d guid is %u\n ", i, mg_ptr->levels[i]);


  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, test_level_edt, 0, mg_ptr->max_levels);
  ocrEdtCreate(&edt, tmp, 0, NULL, mg_ptr->max_levels, NULL, 0, NULL_GUID, NULL);
  for (i=0;i<mg_ptr->max_levels;i++)
    ocrAddDependence( mg_ptr->levels[i], edt, i,  DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  return NULL_GUID;
}

ocrGuid_t test_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

  PRINTF("test_level_edt: depc is %u\n", depc);

  int i,j,k;
  for (i=0;i<depc;i++) {
    level_type* l = (level_type*)depv[i].ptr;
    PRINTF("level%u: num boxes - %u, eigenvalue - %f\n", i, l->num_boxes, l->dominant_eigenvalue_of_DinvA);
    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, test_box_edt, 0, l->num_boxes+1);
    ocrEdtCreate(&edt, tmp, 0, NULL, l->num_boxes+1, NULL, 0, NULL_GUID, NULL);
    ocrAddDependence(depv[i].guid, edt, 0, DB_MODE_CONST);
    for (j=0;j<l->num_boxes;j++) {
      ocrGuid_t b = ((ocrGuid_t*)(((char*)l)+ l->boxes))[j];
      ocrAddDependence(b, edt, j+1, DB_MODE_CONST);
    }
    ocrEdtTemplateDestroy(tmp);
  }
  return NULL_GUID;
}


ocrGuid_t test_box_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

  int i,j,k;
  level_type *l = ((level_type*) depv[0].ptr);
  for (i=1;i<depc;i++) {
    u32 boxid = ((box_type *)depv[i].ptr)->global_box_id;
    int vol, dim;
    vol = l->volume;
    dim = l->box_dim;
    PRINTF("level->h: %f, Boxid: %u, volume: %u, dim: %u\n", l->h,boxid, vol, dim);
  }
  return NULL_GUID;
}

