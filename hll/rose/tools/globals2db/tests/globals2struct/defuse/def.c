#include <stdio.h>

int glob=10;

void bar(int i)
{
    glob = glob+i;
    printf("glob=%d\n", glob);
}

