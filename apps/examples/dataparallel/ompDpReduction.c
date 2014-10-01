#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define NUM_THREADS 4
#define N 10000

struct timeval ta,tb;

static void compdelay(int delaylength) {
   int  i;
   float a=0.;
   for (i=0; i<delaylength; i++) a+=i;
   if (a < 0) printf("%f \n",a);
}

int main(int argc, char **argv) {
  int i;
  unsigned long result = 0;
  int *a = (int*) malloc(N * sizeof(int));
  for (i = 0; i < N; i++)
    a[i] = i;

  gettimeofday(&ta,0);
#pragma omp parallel for num_threads(NUM_THREADS) schedule(runtime) shared(a) reduction(+:result)
  for (i = 0; i < N; i++) {
    a[i] += i;
    result += i;
    compdelay(1000000);
  }
  gettimeofday(&tb,0);

  for (i = 0; i < N; i++) {
    if (a[i] != i * 2)
      break;
  }
  if (i == N && result == ((N * (N - 1))/2)) {
    printf("Pass\n");
  } else {
    printf("Fail expected:%lu computed:%lu\n", ((N * (N - 1))/2), result);
  }
  printf("The computation took %f seconds\r\n",
         ((tb.tv_sec - ta.tv_sec)*1000000+(tb.tv_usec - ta.tv_usec))*1.0/1000000);
  return 0;
}
