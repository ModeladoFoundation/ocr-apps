// This file is subject to the license agreement located in the file LICENSE
// and cannot be distributed without it. This notice cannot be
// removed or modified.

#include <stdio.h>

int glob=10;

void bar(int i)
{
    glob = glob+i;
    printf("glob=%d\n", glob);
}

