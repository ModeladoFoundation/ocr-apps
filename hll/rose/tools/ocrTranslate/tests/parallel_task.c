/*
 * Simple test for ocrTranslate tool
 */

#include <stdio.h>
#include <stdlib.h>

void init_data(int size, int in[]) {
  time_t t;
  int iter = 0;
  srand((unsigned) time(&t));
  for(iter = 0; iter < size; ++iter) {
    in[iter] = 10;
    // in[iter] = (double)(rand() % 100)/7;
  }
}

void print_data(int size, int in[]) {
  int iter;
  printf("TASK_print\n");
  printf("[");
  for(iter = 0; iter < size; ) {
    printf("%d", in[iter++]);
    if(iter < size) printf(", ");
  }
  printf("]\n");
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting one input argument\n");
#pragma ocr shutdown DEP_EVTs()
    return 0;
  }

  int size  = atoi(argv[1]);

#pragma ocr datablock begin DATABLOCK(DBK1, DBK2)
  int *dbk1, *dbk2;
#pragma ocr datablock end
  dbk1 = (int*) malloc(size * sizeof(int));
  dbk2 = (int*) malloc(size * sizeof(int));

#pragma ocr task begin TASK(TASK_initDbk1)	\
  DEP_EVTs() DEP_DBKs(DBK1) DEP_ELEMs(in:size)
  init_data(size, dbk1);
#pragma ocr task end OEVENT(OEVT_initDbk1)

#pragma ocr task begin TASK(TASK_initDbk2)	\
  DEP_EVTs() DEP_DBKs(DBK2) DEP_ELEMs(in:size)
  init_data(size, dbk2);
#pragma ocr task end OEVENT(OEVT_initDbk2)

  int iter;
#pragma ocr task begin TASK(TASK_increment) \
  DEP_EVTs(OEVT_initDbk1) DEP_DBKs(DBK1) DEP_ELEMs(in:size, in:iter)
  printf("TASK_increment\n");
  for(iter = 0; iter < size; ++iter) {
    dbk1[iter] = dbk1[iter]+1;
  }
#pragma ocr task end OEVENT(OEVT_increment)

#pragma ocr task begin TASK(TASK_decrement) \
  DEP_EVTs(OEVT_initDbk2) DEP_DBKs(DBK2) DEP_ELEMs(in:size, in:iter)
  printf("TASK_decrement\n");
  for(iter = 0; iter < size; ++iter) {
    dbk2[iter] = dbk2[iter]-2;
  }
#pragma ocr task end OEVENT(OEVT_decrement)

#pragma ocr task begin TASK(TASK_printInc) \
  DEP_EVTs(OEVT_increment) DEP_DBKs(DBK1) DEP_ELEMs(in:size)
  print_data(size, dbk1);
#pragma ocr task end OEVENT(OEVT_printInc) DESTROY_DBKs(DBK1)

#pragma ocr task begin TASK(TASK_printDec) \
  DEP_EVTs(OEVT_decrement) DEP_DBKs(DBK2) DEP_ELEMs(in:size)
  print_data(size, dbk2);
#pragma ocr task end OEVENT(OEVT_printDec) DESTROY_DBKs(DBK2) DESTROY_EVTs(OEVT_decrement)

#pragma ocr shutdown DEP_EVTs(OEVT_printInc, OEVT_printDec)
  return 0;
}
