#include <stdio.h>
#include <stdlib.h>
int n = 0;

int sub()
{
  n = 1;
  return 0;
}

int
main (int argc, char** argv)
{
  //int n = atoi(argv[1]), r;
  int n = 0;
  int *blk;
  sub();

  //printf("sizeof(int) = %ld\n", sizeof(n));
//  blk = (int *) malloc(512 * sizeof(*blk));

//  blk[n] += 13;

  return n;
}
