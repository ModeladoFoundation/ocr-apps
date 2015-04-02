#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ocr.h>

int
main( int argc, char **argv )
{

    ocrGuid_t db;
    ocrGuid_t myGuid;
    void * addr;
    uint64_t len = 32*1024;
    time_t t = time(NULL);
    char* timeString = ctime(&t);

    timeString[24] = '\0'; // clobber newline
    fprintf(stdout, "time = %s\n", timeString);

    printf("float = %f\n", 1.0);

    (void) ocrAffinityGetCurrent( & myGuid );

    uint8_t ret = ocrDbCreate( & db, (uint64_t *) & addr,  len, 0, myGuid, NO_ALLOC );

    printf("ocrDbCreate: %d\n", ret );

    ret = ocrDbDestroy( db );

    printf("ocrDbDestroy: %d\n", ret );

    return 0;
}
