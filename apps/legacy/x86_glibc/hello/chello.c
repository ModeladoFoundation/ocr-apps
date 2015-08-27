#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int mainEdt( int argc, char **argv )
{
	const time_t t = time(NULL);
	char *timeString = ctime(&t);
    struct stat st;
	int ret;

	printf("Hello World!\n");

	printf("%s: argc = %d\n", __FUNCTION__, argc);

	timeString[24] = '\0'; // clobber newline
	fprintf(stdout, "time = %s\n", timeString);

	printf("float = %f\n", 1.0);

	ret = stat( "/dev/null", & st );

    return 0;
}
