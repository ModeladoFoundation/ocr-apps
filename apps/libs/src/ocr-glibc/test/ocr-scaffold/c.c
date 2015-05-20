#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int mainEdt( int argc, char **argv )
{
    printf("appmain output: argc = %d\n", argc);

    struct stat st;

    int ret = stat( "/dev/null", & st );

    return 0;
}
