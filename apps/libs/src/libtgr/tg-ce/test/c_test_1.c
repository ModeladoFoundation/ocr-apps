//
// Simple test of ce_ interfaces: Allocate/free memory, console print,
//	write to fd 1 (stdout).
//
//
#include <stdint.h>
#include <sys/types.h>
#include <sys/tgr.h>

//#include <libswtest.h>

//extern int8_t tgr_mmap( void ** addr,  size_t len );
//extern int8_t tgr_munmap( void * addr,  size_t len );
//extern int8_t tgr_write( int fd, size_t *wroteCount, const void * ptr, size_t len);

extern void ce_print( char *msg );

int empty[1024];    // just to provide global data

//
// Copy and return a ptr to the end '\0'
//
char * stpcpy( char * to, const char * from )
{
	while( (*to++ = *from++) )
		;
	return --to;
}

#define STATUS( key, expr )	ce_print( (expr) ? "OK_" key : "FAIL_" key )

int main( int argc, char ** argv)
{
	const char msg[] = "argc = ";
	char * tmp = NULL;

    ce_print( "c_test_1 starting\n");

    if( tgr_mmap( (void **) & tmp, sizeof(msg) + 1 ) != 0 ) {
        ce_print( "tgr_mmap failed\n");
        return 0;
    }
    ce_print( "tgr_mmap succeeded\n");
	//
	// Argc should have just 1 argument - the program name
	// Test the value survived the call to ce_memalloc()
	//
	STATUS( "1", argc == 1 );
	//
	// Validate we got memory
	//
	STATUS( "2", tmp != NULL );

	char * p = stpcpy( tmp, msg );
	*p++ = '0' + argc;
	*p = '\0';

    ssize_t wrote = 0;

	int status = tgr_write( 1, & wrote, tmp, sizeof(msg) );

    if( status != 0 || wrote != sizeof(msg) ) {
        ce_print( "tgr_unmap failed\n");
        return 0;
    }

	tgr_munmap( tmp, 0 );

    return 0;
}
