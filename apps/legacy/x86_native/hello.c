#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int
main( int argc, char **argv )
{
    void * addr;
    uint64_t len = 32*1024;
    time_t t = time(NULL);
    char* timeString = ctime(&t);

    printf("Hello World!\n");

    timeString[24] = '\0'; // clobber newline
    fprintf(stdout, "time = %s\n", timeString);

    printf("float = %f\n", 1.0);

    return 0;
}
