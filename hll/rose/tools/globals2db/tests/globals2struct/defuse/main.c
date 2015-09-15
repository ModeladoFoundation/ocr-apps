// This file is subject to the license agreement located in the file LICENSE
// and cannot be distributed without it. This notice cannot be
// removed or modified.

#include <stdio.h>

extern void foo(int i);
extern void bar(int i);

int main(int argc, char**argv)
{
    foo(5);
    bar(7);
    return 0;
}
