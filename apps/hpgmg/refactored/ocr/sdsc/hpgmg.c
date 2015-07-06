#include <ocr.h>
#include <stdlib.h>

#include "hpgmg.h"

//#define WARMUP 1
//#define TIMED 5

ocrGuid_t top_warm(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t top_loop(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t finalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t finalize_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t test0_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  u64 argc = getArgc(depv[0].ptr);
  if(argc!=3){
    PRINTF("usage: hpgmg log2_box_dim target_boxes\n");
    ocrShutdown(); return NULL_GUID;
  }

  ocrGuid_t args;
  char* argv[2];
  argv[0] = getArgv(depv[0].ptr,1);
  argv[1] = getArgv(depv[0].ptr,2);

  int log2_box_dim=atoi(argv[0]);
  int target_boxes=atoi(argv[1]);

  if(log2_box_dim<4){
    PRINTF("log2_box_dim must be at least 4\n");
    ocrShutdown(); return NULL_GUID;
  }

  if(target_boxes<1){
    PRINTF("target_boxes_per_rank must be at least 1\n");
    ocrShutdown(); return NULL_GUID;
  }

  // calculate the problem size...
  int box_dim=1<<log2_box_dim;
  int boxes_in_i = 1000;
  int total_boxes = boxes_in_i*boxes_in_i*boxes_in_i;
  while(total_boxes>target_boxes){
    boxes_in_i--;
    total_boxes = boxes_in_i*boxes_in_i*boxes_in_i;
  }

  ocrGuid_t mg; mg_type* mg_ptr;
  ocrDbCreate(&mg, (void**)&mg_ptr, sizeof(mg_type), 0,NULL_GUID,NO_ALLOC);

  // initialization
  init_all(mg_ptr, box_dim, boxes_in_i, BC_DIRICHLET, 0);

  VERBOSEP("Finished initialization\n");

  // start loop
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, top_warm, 0, 1);
  ocrEdtCreate(&edt, tmp, 0, NULL, 1, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(mg, edt, 0, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  ocrDbDestroy(depv[0].guid);

  return NULL_GUID;
}

ocrGuid_t top_warm(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("Top warm\n");
  ocrGuid_t e; ocrEventCreate(&e, OCR_EVENT_ONCE_T, 1);
  ocrGuid_t cont = do_solves(e, (mg_type*)depv[0].ptr, WARMUP,1);
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, top_loop, 0, 2);
  ocrEdtCreate(&edt, tmp, 0, NULL, 2, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_CONST);
  ocrAddDependence(cont, edt, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  ocrEventSatisfy(e, depv[0].guid);

  return NULL_GUID;
}

ocrGuid_t top_loop(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("Top loop\n");
  // Start timers!
  ocrGuid_t e; ocrEventCreate(&e, OCR_EVENT_ONCE_T, 1);
  ocrGuid_t cont = do_solves(e, (mg_type*)depv[0].ptr, TIMED,0);
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, finalize, 0, 3);
  ocrEdtCreate(&edt, tmp, 0, NULL, 3, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_CONST);
  ocrAddDependence(((mg_type *)(depv[0].ptr))->levels[0], edt, 1, DB_MODE_CONST); // only fine grid required to compute error
  ocrAddDependence(cont, edt, 2, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  ocrEventSatisfy(e, depv[0].guid);

  return NULL_GUID;
}

ocrGuid_t finalize(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
   int b;
   // Print all the timing information
  ocrGuid_t tm, i_tm, fin;
  ocrEdtTemplateCreate(&i_tm, print_timing_edt, 1, ((mg_type*)(depv[0].ptr))->num_levels);
  u64 num_levels = ((mg_type*)(depv[0].ptr))->num_levels;
  ocrEdtCreate(&tm, i_tm, 1, &num_levels, num_levels, NULL, 0, NULL_GUID, &fin);

   // Set up finalize_edt (depends on print_timing_edt's output event)
  level_type *l = (level_type*)(depv[1].ptr);
  ocrGuid_t tmp,edt;
  int num_boxes = l->num_boxes;
  u64 pv[3] = {l->u, l->u_true, l->vec_temp};
  ocrEdtTemplateCreate(&tmp, finalize_edt, 3, num_boxes+3);
  ocrEdtCreate(&edt, tmp, 3, pv, num_boxes+3, NULL, 0, NULL_GUID, NULL);

 ocrGuid_t* boxes = (ocrGuid_t*)(((char*)l)+l->boxes);
  for (b = 0; b < num_boxes; b++) {
    ocrAddDependence(boxes[b], edt, b+3, DB_MODE_RW);
  }
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_CONST);
  ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_CONST);
  ocrAddDependence(fin, edt, 2, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

   // Allow print_timing_edt to run now
  for (b = 0; b < num_levels; b++) {
    ocrAddDependence(((mg_type*)(depv[0].ptr))->levels[b], tm, b, DB_MODE_CONST);
  }

  ocrEdtTemplateDestroy(i_tm);


  return NULL_GUID;
}
