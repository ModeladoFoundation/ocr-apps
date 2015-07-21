/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include <stdio.h>
#include <stdlib.h>

#define LEN4 4
#define LEN5 5
#define LEN6 6

static int foo1d[LEN4]={1,2,3,4};
static int foo2d[LEN4][LEN5];
int arr2d[LEN4][LEN5];

int foo(int argc, char**argv)
{
    int i,j;

    int inp = atoi(argv[1]);

    for (i=0; i<LEN4; i++) {
        for (j=0; j<LEN5; j++) {
            foo2d[i][j]=0;
        }
    }


    for (i=0; i<LEN4; i++) {
        foo1d[i] = foo1d[0] + (i+1)*inp;
        printf("%d\t", foo1d[i]);
    }
    printf("\n");



    for (i=0; i<LEN4; i++) {
        for (j=0; j<LEN5; j++) {
            foo2d[i][j] = foo2d[i][0] + (j+1)*inp;
            printf("%d\t", foo2d[i][j]);
        }
        printf("\n");
    }

    for (i=0; i<LEN4; i++) {
        for (j=0; j<LEN5; j++) {
            arr2d[i][j] += arr2d[i][0] + (j+1)*inp;
            printf("%d\t", arr2d[i][j]);
        }
        printf("\n");
    }

    return 0;
}
