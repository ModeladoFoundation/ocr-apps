#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define N 1000000

struct timeval ta,tb;

static void compdelay(int delaylength) {
   int  i;
   float a=0.;
   for (i=0; i<delaylength; i++) a+=i;
   if (a < 0) printf("%f \n",a);
}

int main() {
  int i;
  int *a = (int*) calloc(N, sizeof(int));
  for (i = 0; i < N; i++)
    a[i] = i;

  gettimeofday(&ta,0);
#pragma omp parallel for schedule(runtime) shared(a)
  for (i = 0; i < N; i++) {
    a[i] += i;
    compdelay(10000);
  }
  gettimeofday(&tb,0);

  for (i = 0; i < N; i++) {
    if (a[i] != i * 2)
      break;
  }

  if (i == N) {
    printf("Pass\n");
  } else {
    printf("Fail\n");
  }
  printf("The computation took %f seconds\r\n",
         ((tb.tv_sec - ta.tv_sec)*1000000+(tb.tv_usec - ta.tv_usec))*1.0/1000000);
  return 0;
}
