/*
 * Simple test for ocrTranslate tool
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting one input argument\n");
#pragma ocr shutdown DEP_EVTs()
    return 0;
  }

  int size  = atoi(argv[1]);

#pragma ocr datablock DATABLOCK(DBK_in)
  int* in;

  in = (int*) malloc(size * sizeof(int));
  int iter;

#pragma ocr task TASK(TASK_init) \
  DEP_DBKs(DBK_in) DEP_EVTs() DEP_ELEMs(in:size, in:iter)  OEVENT(OEVT_init)
  {
    printf("TASK_init\n");
    time_t t;
    iter = 0;
    srand((unsigned) time(&t));
    for(iter = 0; iter < size; ++iter) {
      in[iter] = 10;
    }
  }

#pragma ocr task TASK(TASK_increment) \
  DEP_DBKs(DBK_in) DEP_EVTs(OEVT_init) DEP_ELEMs(in:size, in:iter) OEVENT(OEVT_increment)
  {
    printf("TASK_increment\n");
    for(iter = 0; iter < size; ++iter) in[iter] = in[iter]+1;
  }

#pragma ocr task TASK(TASK_print) \
  DEP_DBKs(DBK_in) DEP_EVTs(OEVT_increment) DEP_ELEMs(in:size, in:iter) OEVENT(OEVT_print)
  {
    printf("TASK_print\n");
    printf("[");
    for(iter = 0; iter < size; ) {
      printf("%d", in[iter++]);
      if(iter < size) printf(", ");
    }
    printf("]\n");
  }

#pragma ocr shutdown DEP_EVTs(OEVT_print)
  return 0;
}

