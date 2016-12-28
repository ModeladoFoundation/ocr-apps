/*
 * Simple test for ocrTranslate tool
 */

#include <stdio.h>
#include <stdlib.h>

void increment(double* x, int size) {
  int iter;
  for(iter = 0; iter < size; ++iter) x[iter] = x[iter]+1;
}

void decrement(double* x, int size) {
  int iter;
  for(iter = 0; iter < size; ++iter) x[iter] = x[iter]-1;
}

void init(double* x, int size) {
  time_t t;
  int iter;

  srand((unsigned) time(&t));
  for(iter = 0; iter < size; ++iter) x[iter] = (double)(rand() % 100)/7;
}

void print(double* x, int size) {
  int iter;
  printf("[");
  for(iter = 0; iter < size; ) {
    printf("%f", x[iter++]);
    if(iter < size) printf(", ");
  }
  printf("]\n");
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting one input argument\n");
    return -1;
  }

  int size  = atoi(argv[1]);

#pragma ocr datablock begin DATABLOCK(DBK_in)
  double* in;
  in = (double*) malloc(size * sizeof(double));
#pragma ocr datablock end

#pragma ocr task begin TASK(TASK_init)	\
  DEP_EVTs(NONE) DEP_DBKs(DBK_in) DEP_ELEMs(in:size, in:argc1234,    out:argv)
  init(in, size);
#pragma ocr task end OEVENT(OEVT_init)

#pragma ocr task begin TASK(TASK_increment) \
  DEP_EVTs(OEVT_init) DEP_DBKs(DBK_in) DEP_ELEMs(in:size)
  increment(in, size);
#pragma ocr task end OEVENT(OEVT_increment)

#pragma ocr task begin TASK(TASK_print) \
  DEP_EVTs(OEVT_increment) DEP_DBKs(DBK_in) DEP_ELEMs(in:size)
  print(in, size);
#pragma ocr task end OEVENT(OEVT_print)

  return 0;
}
