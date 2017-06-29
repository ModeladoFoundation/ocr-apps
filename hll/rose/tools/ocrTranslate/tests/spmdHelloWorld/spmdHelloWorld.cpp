#include "spmd.h"
#include <stdlib.h>
#include <stdio.h>

ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  ocrShutdown();
  return NULL_GUID;
}

ocrGuid_t myRankEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  int rank = spmdMyRank();
  printf("myRankEdt: Hello, world! from rank=%d\n", rank);
  return NULL_GUID;
}

ocrGuid_t spmdTaskEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  int rank = spmdMyRank();
  printf("spmdTaskEdt: Hello, world! from rank=%d\n", rank);

  ocrGuid_t myRankEdtTemplGuid;
  ocrEdtTemplateCreate(&myRankEdtTemplGuid, myRankEdt, 0, 0);
  ocrGuid_t myRankOutEvtGuid;
  ocrEventCreate(&myRankOutEvtGuid, OCR_EVENT_ONCE_T, EVT_PROP_NONE);

  ocrGuid_t myRankEdtGuid;
  ocrEdtCreate(&myRankEdtGuid, myRankEdtTemplGuid, 0, NULL, 0, NULL, EDT_PROP_NONE, NULL, &myRankOutEvtGuid);

  spmdRankFinalize(myRankOutEvtGuid, false);
  return NULL_GUID;
}

ocrGuid_t spmd_mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
  int nprocs = 10;
  ocrGuid_t spmdTaskEdtTemplGuid;
  ocrEdtTemplateCreate(&spmdTaskEdtTemplGuid, spmdTaskEdt, 0, 0);
  ocrGuid_t spmdFinishEvtGuid;
  ocrEventCreate(&spmdFinishEvtGuid, OCR_EVENT_ONCE_T, EVT_PROP_NONE);

  ocrGuid_t shutdownEdtTemplGuid;
  ocrGuid_t shutdownEdtGuid;
  ocrEdtTemplateCreate(&shutdownEdtTemplGuid, shutdownEdt, 0, 1);
  ocrEdtCreate(&shutdownEdtGuid, shutdownEdtTemplGuid, 0, 0, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);
  ocrAddDependence(spmdFinishEvtGuid, shutdownEdtGuid, 0, DB_MODE_NULL);

  spmdEdtSpawn(spmdTaskEdtTemplGuid, nprocs, 0, 0, 0, 0, 0, 0, spmdFinishEvtGuid);

  return NULL_GUID;
}
