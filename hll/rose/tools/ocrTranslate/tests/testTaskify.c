/*
 * Simple test for ocrTranslate tool
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting one input argument\n");
    return -1;
  }

  int size  = atoi(argv[1]);

#pragma ocr datablock begin DATABLOCK(DBK_in)
  double* in;
#pragma ocr datablock end
  in = (double*) malloc(size * sizeof(double));
  int iter;

#pragma ocr task begin TASK(TASK_init)	\
  DEP_EVTs(NONE) DEP_DBKs(DBK_in) DEP_ELEMs(in:size, in:iter)
  time_t t;
  iter = 0;
  srand((unsigned) time(&t));
  for(iter = 0; iter < size; ++iter) {
    in[iter] = (double)(rand() % 100)/7;
  }
#pragma ocr task end OEVENT(OEVT_init)

#pragma ocr task begin TASK(TASK_increment) \
  DEP_EVTs(OEVT_init) DEP_DBKs(DBK_in) DEP_ELEMs(in:size, in:iter)
  for(iter = 0; iter < size; ++iter) in[iter] = in[iter]+1;
#pragma ocr task end OEVENT(OEVT_increment)

#pragma ocr task begin TASK(TASK_print) \
  DEP_EVTs(OEVT_increment) DEP_DBKs(DBK_in) DEP_ELEMs(in:size, in:iter)
  printf("[");
  for(iter = 0; iter < size; ) {
    printf("%f", in[iter++]);
    if(iter < size) printf(", ");
  }
  printf("]\n");
#pragma ocr task end OEVENT(OEVT_print)

  return 0;
}
