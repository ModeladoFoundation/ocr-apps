#include <stdlib.h>
#include <stdio.h>
/*
 * Test for parser
 * Annotations outside if/else block
 * How should we add annotations under conditional constructs?
 * There are two cases
 * 1 - We can add it at the top level. Under this case the marked region
 * will be outlined to an OCR Edt and the Edt will be setup to be invoked
 * at the pragma location
 * 2 - We may need to add the annotations inside the branches when the Edts
 * under the two branches may have different input dependencies as illustrated
 * here
 */

int main(int argc, char* argv[]) {
  if(argc != 2) {
    printf("Expecting two input arguments\n");
    return -1;
  }

  int size  = atoi(argv[1]);
  int switch_flag = atoi(argv[2]);

#pragma ocr datablock DATABLOCK(DBK_in)
  double* in = malloc(size * sizeof(double));

  int i;

#pragma ocr task TASK(TASK_init)  \
  DEP_DBKs(DBK_in) DEP_EVTs() \
  DEP_ELEMs(i, size) OEVENT(OEVT_init)
  {
    time_t t;
    srand((unsigned) time(&t));
    for(i = 0; i < size; ++i) {
      in[i] = (double)(rand() % 100)/7;
    }
  }

  in = malloc(sizeof(double) * size);

#pragma ocr task TASK(TASK_do_something) \
  DEP_DBKs(DBK_in) DEP_EVTs(OEVT_init) \
  DEP_ELEMs(in:i, in:size, in:switch_flag) OEVENT(OEVT_fin)
  {
    if(switch_flag) {
      for(i = 1; i < size; ++i) {
	in[i] += in[i-1];
      }
    }
    else {
      for(i = 1; i < size; ++i) {
	in[i] -= in[i-1];
      }
    }
  }

  return 0;
}
