#include <stdlib.h>
#include "multi_source_file1.h"
#include "multi_source_file2.h"

int main() {
  h_mem = malloc(sizeof(int) * 10);
  hd_mem = malloc(sizeof(double) * 1);
  return 0;
}
