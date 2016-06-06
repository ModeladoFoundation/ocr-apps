#include "ocr.h"

#define ENABLE_EXTENSION_LEGACY
#include <extensions/ocr-legacy.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 8

int partition( int a[], int l, int r) {
  int pivot, i, j, t;
  pivot = a[l];
  i = l; j = r+1;

  while(1)
    {
      do ++i; while( a[i] <= pivot && i <= r );
      do --j; while( a[j] > pivot );
      if( i >= j ) break;
      t = a[i]; a[i] = a[j]; a[j] = t;
    }
  t = a[l]; a[l] = a[j]; a[j] = t;
  return j;
}

ocrGuid_t launch_quickSort(ocrGuid_t dbGuid, int l, int r);

ocrGuid_t quickSortEdt (u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  int l = (int)paramv[0];
  int r = (int)paramv[1];
  ocrGuid_t outEvt = (ocrGuid_t)paramv[2];
  int j;
  int *a = (int *)depv[0].ptr;
  ocrGuid_t dbGuid_a = depv[0].guid;
  ocrGuid_t evt1, evt2;
  ocrGuid_t db1, db2;
  void *dbp1, *dbp2;

  if( l < r )
    {
      // divide and conquer
      j = partition( a, l, r);
      evt1 = launch_quickSort(dbGuid_a, l, j-1);
      evt2 = launch_quickSort(dbGuid_a, j+1, r);
      ocrLegacyBlockProgress(evt1, &db1, &dbp1, NULL, LEGACY_PROP_NONE);
      ocrLegacyBlockProgress(evt2, &db2, &dbp2, NULL, LEGACY_PROP_NONE);
    }

  ocrEventSatisfy(outEvt, dbGuid_a);
  return NULL_GUID;
}

ocrGuid_t launch_quickSort(ocrGuid_t dbGuid, int l, int r)
{
    ocrGuid_t edtTemp, edt;
    ocrEdtTemplateCreate(&edtTemp, &quickSortEdt, 3, 1);

    ocrGuid_t returnEvent;
    ocrEventCreate(&returnEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    u64 paramv[3];
    paramv[0] = l;
    paramv[1] = r;
    paramv[2] = returnEvent;

    ocrEdtCreate(&edt, edtTemp, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL_GUID, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(dbGuid, edt, 0, DB_MODE_RW);

    ocrEdtTemplateDestroy(edtTemp);

    return returnEvent;
}

// This keyword would undergo the following transformations:
// 1. Replacement of the keyword with the OCR API
// 2. Liveness analysis to detect all uses of the variable
//    - Use of the data by EDT should be replaced with its guid & unmarshalling
//    - Use of the data outside of EDT should be replaced with the pointer
// 3. Replacement of the __attribute__ ((level)) keyword with an OCR hint


// Assuming that quickSort() is declared a __task, the resultant code would be
// the following, assuming launch_quickSort() follows the same semantics as
// the launchFib() function in the previous example
int main(int argc, char *argv[])
{
  ocrConfig_t cfg;
  ocrGuid_t legacyCtx;

  cfg.userArgc = argc;
  cfg.userArgv = (char **)argv;
  cfg.iniFile = getenv("OCR_CONFIG");

  ocrLegacyInit(&legacyCtx, &cfg);
  ocrGuid_t dbGuid_a;
  int *a;
  ocrDbCreate(&dbGuid_a, (void **)&a, sizeof(int)*(LEN+1), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

  int i;
  for(i = 0; i<LEN; i++) a[i] = rand() % 0x100;

  printf("Before\n");
  for(i=0; i<LEN; i++) printf("%02x ", a[i]); printf("\n");

  ocrGuid_t finishEvt = launch_quickSort(dbGuid_a, 0, LEN);
  ocrLegacyBlockProgress(finishEvt, NULL_GUID, NULL, NULL, LEGACY_PROP_NONE);
  ocrEventDestroy(finishEvt);
  ocrDbDestroy(dbGuid_a);
  ocrLegacyFinalize(legacyCtx, false);

  printf("After\n");
  for(i=1; i<=LEN; i++) printf("%02x ", a[i]); printf("\n");

  return 0;
}
