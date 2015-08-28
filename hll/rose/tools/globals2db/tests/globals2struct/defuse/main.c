#include <stdio.h>

extern void foo(int i);
extern void bar(int i);

int main(int argc, char**argv)
{
    foo(5);
    bar(7);
    return 0;
}
