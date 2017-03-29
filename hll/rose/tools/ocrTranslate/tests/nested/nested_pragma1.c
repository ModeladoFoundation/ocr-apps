#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 1) {
    printf("Expecting one argument\n");
    return -1;
  }
  int size = atoi(argv[1]);
  int *A = (int*) malloc(sizeof(int) * size);
  int* B = (int*) malloc(sizeof(int) * size);

  int i;
#pragma ocr task begin
  for(i = 0; i < size; i++) {
    A[i] = 10+i;
    B[i] = 5-i;
  }

  int* C = (int*) malloc(sizeof(int) * size);
  #pragma ocr task begin
  for(i = 0; i < size; i++) {
    C[i] = A[i] + B[i];
  }
  #pragma ocr task end
#pragma ocr task end

#pragma ocr task begin
  for(i = 0; i < size; i++) {
    printf("C[%d]=%d,", i, C[i]);
  }
  printf("\n");
#pragma ocr task end
  return 0;
}
