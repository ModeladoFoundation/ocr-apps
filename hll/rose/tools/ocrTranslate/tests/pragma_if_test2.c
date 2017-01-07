#include <stdio.h>
#include <stdlib.h>
/*
 * Test for parser
 * Annotations inside if/else block
 */

void init(double* in_, int size) {
  time_t t;
  int i;
  srand((unsigned) time(&t));
  for(i = 0; i < size; ++i) in_[i] = (double)(rand() % 100)/7;
}

void do_add(double* in_, int size) {
  int i;
  for(i = 1; i < size; ++i) in_[i] += in_[i-1];
}

void do_diff(double* in_, int size) {
  int i;
  for(i = 1; i < size; ++i) in_[i] -= in_[i-1];
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting two input arguments\n");
    return -1;
  }

  int size  = atoi(argv[1]);
  int switch_flag = atoi(argv[2]);

#pragma ocr datablock begin DATABLOCK(DBK_in)
  double* in;
#pragma ocr datablock end

  in = malloc(size * sizeof(double));

#pragma ocr task begin TASK(TASK_init) \
  DEP_EVTs(NONE) DEP_DBKs(DBK_in) DEP_ELEMs(in:size)
  init(in, size);
#pragma ocr task end OEVENT(OEVT_init)

  // How should we add annotations under conditional constructs?
  // There are two cases
  // 1 - We can add it at the top level. Under this case the marked region
  // will be outlined to an OCR Edt and the Edt will be setup to be invoked
  // at the pragma location
  // 2 - We may need to add the annotations inside the branches when the Edts
  // under the two branches may have different input dependencies as illustrated
  // here
  if(switch_flag) {
#pragma ocr task begin TASK(TASK_do_add) \
  DEP_EVTs(NONE) DEP_DBKs(DBK_in) DEP_ELEMs(size)
    do_add(in, size);
#pragma ocr task end OEVENT(OEVT_fin)
  }
  else {
#pragma ocr task begin TASK(TASK_do_add) \
  DEP_EVTs(OEVT_init) DEP_DBKs(DBK_in) DEP_ELEMs(size)
    do_diff(in, size);
#pragma ocr task end OEVENT(OEVT_fin)
  }
  return 0;
}
