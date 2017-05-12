#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
#pragma ocr datablock  DATABLOCK(DBK_ARR)
  int* arr;

  int size = 10;

  arr = (int*) malloc(sizeof(int) * size);

#pragma ocr task TASK(TASK_VecInit) DEP_DBKs(DBK_ARR) \
  DEP_EVTs() DEP_ELEMs(size) OEVENT(OEVT_VecInit)
  {
    int i, j, k;
    j = 5;
    k = 10;
    arr[0] = j+k;

#pragma ocr loop TASK(TASK_ArrSum) DEP_DBKs(DBK_ARR) DEP_EVTs(NONE) DEP_ELEMs(i, j, k, size) \
  OEVENT(OEVT_FOR)
    for(i=1, k=2; i < size; i++) {
      arr[i] = arr[i-1]+k;
    }
  }

#pragma ocr task TASK(TASK_Print) DEP_DBKS(DBK_ARR) \
  DEP_EVTs(OEVT_VecInit) DEP_ELEMs(size) OEVENT(OEVT_Print)
  {
    int i;
    for(i=0; i < size; i++) {
      printf("arr[%d]=%d\n", i, arr[i]);
    }
  }

#pragma ocr shutdown DEP_EVTs(OEVT_Print)

  return 0;
}
