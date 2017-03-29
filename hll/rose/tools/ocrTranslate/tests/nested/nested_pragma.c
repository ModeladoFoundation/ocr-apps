#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting one argument\n");
#pragma ocr shutdown DEP_EVTs(NONE)
    return -1;
  }
  int size = atoi(argv[1]);
#pragma ocr datablock  DATABLOCK(DBK_A, DBK_B)
  int* A = (int*) malloc(size * sizeof(int));
  int* B = (int*) malloc(size * sizeof(int));

  int i;

#pragma ocr task  TASK(TASK_compute) DEP_DBKs(DBK_A, DBK_B) \
  DEP_EVTs() DEP_ELEMs(in:i, in:size) OEVENT(OEVT_compute)
  {
    for(i = 0; i < size; i++) {
      A[i] = 10+(2*i);
      B[i] = 5-i;
    }

  #pragma ocr datablock  DATABLOCK(DBK_C)
    int* C = (int*) malloc(sizeof(int) * size);

  #pragma ocr task  TASK(TASK_Add) DEP_DBKs(DBK_A, DBK_B, DBK_C) \
    DEP_EVTs(NONE) DEP_ELEMs(in:i, in:size) OEVENT(OEVT_add)
    {
      for(i = 0; i < size; i++) {
	C[i] = A[i] + B[i];
      }
    } // end TASK_Add

  #pragma ocr task  TASK(TASK_print) DEP_DBKs(DBK_C) \
    DEP_EVTs(OEVT_add) DEP_ELEMs(in:i, in:size) OEVENT(OEVT_print)
    {
      for(i = 0; i < size; i++) {
	printf("C[%d]=%d,", i, C[i]);
      }
      printf("\n");
    }  // end TASK_print
  } // end TASK_compute

#pragma ocr shutdown DEP_EVTs(OEVT_compute)
  return 0;
}
