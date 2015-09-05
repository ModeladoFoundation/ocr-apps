#include <stdio.h>

extern int glob;


void foo(int i)
{
    glob=glob+i;
    printf("glob=%d\n", glob);
}


