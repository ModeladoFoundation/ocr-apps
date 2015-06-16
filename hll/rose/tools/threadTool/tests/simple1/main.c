#include <stdio.h>
#include <stdlib.h>

#define LEN 4

int arr1d[LEN];
int arr2d[LEN][LEN];
static int arr3d[LEN][LEN][LEN];
int x=5;
int y=6;
static int z=6;


int main(int argc, char ** argv)
{
  int i,j,k;

  if (argc < 2 || argc > 2)
  {
    printf("Usage: main <integer number>\n");
    return(1);
  }

  int inp = atoi(argv[1]);

  for (i=0; i<LEN; i++) {
    arr1d[i]=0;
    for (j=0; j<LEN; j++) {
      arr2d[i][j]=0;
      for (k=0; k<LEN; k++) {
        arr3d[i][j][k]=0;
      }
    }
  }

  for (i=0; i<LEN; i++) {
    arr1d[i] = arr1d[0] + (i+1)*inp;
    printf("%d\t", arr1d[i]);
  }
  printf("\n");



  for (j=0; j<LEN; j++) {
    for (i=0; i<LEN; i++) {
      arr2d[i][j] = arr2d[j][0] + (j+1)*inp;
      printf("%d\t", arr2d[i][j]);
    }
    printf("\n");
  }


  for (i=0; i<LEN; i++) {
    for (j=0; j<LEN; j++) {
      for (k=0; k<LEN; k++) {
        arr3d[i][j][k] = arr3d[i][j][0] + (k+1)*inp;
        printf("%d\t", arr3d[i][j][k]);
      }
      printf("\n");
    }
  }

  printf("x=%d\n", x*arr1d[1]);
  printf("y=%d\n", y*arr1d[2]);
  printf("z=%d\n", z*arr1d[3]);
  return 0;
}


