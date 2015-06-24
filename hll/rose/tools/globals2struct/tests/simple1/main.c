/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

// Test case for globals2struct

#include <stdio.h>
#include <stdlib.h>

#define LEN4 4
#define LEN5 5
#define LEN6 6

int arr1d[LEN4]={1,2,3,4};
int arr2d[LEN4][LEN5];
int arr3d[LEN4][LEN5][LEN6];
int x=5, y=6, z=7;

static int bar1d[LEN4]={5,6,7,8};

extern int foo(int argc, char ** argv);


int main(int argc, char ** argv)
{
  int i,j,k;

  if (argc < 2 || argc > 2)
  {
    printf("Usage: main <integer number>\n");
    return(1);
  }

  int inp = atoi(argv[1]);

  for (i=0; i<LEN4; i++) {
    for (j=0; j<LEN5; j++) {
      arr2d[i][j]=0;
      for (k=0; k<LEN6; k++) {
        arr3d[i][j][k]=0;
      }
    }
  }

  for (i=0; i<LEN4; i++) {
    arr1d[i] = arr1d[0] + (i+1)*inp;
    printf("%d\t", arr1d[i]);
  }
  printf("\n");



  for (i=0; i<LEN4; i++) {
      for (j=0; j<LEN5; j++) {
      arr2d[i][j] = arr2d[i][0] + (j+1)*inp;
      printf("%d\t", arr2d[i][j]);
    }
    printf("\n");
  }


  for (i=0; i<LEN4; i++) {
    for (j=0; j<LEN5; j++) {
      for (k=0; k<LEN6; k++) {
        arr3d[i][j][k] = arr3d[i][j][0] + (k+1)*inp;
        printf("%d\t", arr3d[i][j][k]);
      }
      printf("\n");
    }
  }

  for (i=0; i<LEN4; i++) {
    bar1d[i] = bar1d[0] + (i+1)*inp;
    printf("%d\t", bar1d[i]);
  }
  printf("\n");



  printf("x=%d\n", x*arr1d[1]);
  printf("y=%d\n", y*arr1d[2]);
  printf("z=%d\n", z*arr1d[3]);

  // Call a function in a different file.
  // foo() uses file statics.
  // Ensure that they are initialized correctly.
  foo(argc, argv);


  return 0;
}


