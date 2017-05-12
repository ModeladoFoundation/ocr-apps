#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

#pragma ocr datablock DATABLOCK(DBK_ARR)
  int* arr;

  int size = 5;

  arr = (int*) malloc(sizeof(int) * size * size);

#pragma ocr task TASK(TASK_VecInit) DEP_DBKs(DBK_ARR) \
  DEP_EVTs() DEP_ELEMs(size) OEVENT(OEVT_VecInit)
  {
    int i, j, k;
    k = 5;
    arr[0] = k;
#pragma ocr loop TASK(TASK_Outer) DEP_DBKs(DBK_ARR) DEP_EVTs() DEP_ELEMs(i, j, size, k) \
  OEVENT(OEVT_outerLoop)
    for(i = 1; i < size*size; i=i+size) {
      arr[i] = k;
#pragma ocr loop TASK(TASK_Inner) DEP_DBKs(DBK_ARR) DEP_EVTs(NONE) DEP_ELEMs(i, j, k, size) \
  OEVENT(OEVT_innerLoop)
      for(j = i; j < i+size; j++) {
	arr[j] = arr[j-1] + k;
      }
    }
  }

#pragma ocr task TASK(TASK_Print) DEP_DBKs(DBK_ARR) DEP_EVTs(OEVT_VecInit) DEP_ELEMs(size) \
  OEVENT(OEVT_Print)
  {
    int i, j;
    for(i = 0; i < size * size; i=i+size) {
      for(j = i; j < i+size; j++) {
	printf("arr[%d][%d] = %d ", i, j, arr[j]);
      }
      printf("\n");
    }
  }

#pragma ocr shutdown DEP_EVTs(OEVT_Print)

  return 0;
}
