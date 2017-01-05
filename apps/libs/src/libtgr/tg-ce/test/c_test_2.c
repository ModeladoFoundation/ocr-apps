//
// Simple test of ce_ file/dir related interfaces:
//  Getcwd, mkdir, rmdir, open (create), close, stat, read, write, unlink
//
// This test is non-newlib
//
#include <stdint.h>
#include <fcntl.h>
#include <sys/tgr.h>

extern void ce_print( char *msg );

//
// Copy and return a ptr to the end '\0'
//
char * stpcpy( char * to, const char * from )
{
	while( (*to++ = *from++) )
		;
	return --to;
}
static int error( const char *op, int err )
{
    char buf[256];

    char *p = stpcpy( buf, "CE-OS-TEST: error ");
    p = stpcpy( p, op );
    p = stpcpy( p, "\n" );
    ce_print(buf);
    return 1;
}
static void pstatus( const char *msg, const char *data )
{
    char buf[256];

    char *p = stpcpy( buf, msg);
    p = stpcpy( p, data );
    p = stpcpy( p, "\n" );
    ce_print(buf);
}

//
// test data to write/read
//
static int testdata[] =
{
    0,1,2,3,4,5,6,7,
    10,11,12,13,14,15,16,17,
    20,21,22,23,24,25,26,27
};

#define TEST_DATA_LEN  (sizeof(testdata)/sizeof(testdata[0]))

static int cmpdata[64];

//
// Exercise some file/filesys related ops
//
static int test_file_ops()
{
    const char * subdir = "ce-os-subdir";
    const char * testfile = "ce-os-test";
    char buffer[ 256 ];
    int  status;
    int  fd;
    //
    // get the current directory
    //
    status = tgr_getcwd( buffer, sizeof(buffer) );

    if( status )
        return error("getcwd", status);

    pstatus("CE-OS-TEST: cwd = ", buffer );
    //
    // add our subdir name and make the dir and chdir to it
    //
    char *p = stpcpy( buffer, buffer ); // hack to find the wd end
    p = stpcpy( p, "/" );
    p = stpcpy( p, subdir );

    status = tgr_mkdir( buffer, 0777 );
    if( status )
        return error("mkdir", status);

    status = tgr_chdir( buffer );

    if( status )
        return error("chdir", status);

    pstatus("CE-OS-TEST: cwd = ", buffer );
    //
    // create a file and write some data to it, close it
    //
    status = tgr_open( &fd, testfile, O_CREAT | O_WRONLY, 0666 );

    if( status )
        return error("open", status);

    pstatus("CE-OS-TEST: opened ", testfile);

#if 1
    pstatus("CE-OS-TEST: write testdata to ", testfile );

    ssize_t wrote;
    status = tgr_write( fd, &wrote, testdata, sizeof(testdata) );

    if( wrote < 0 || wrote != sizeof(testdata) )
        return error( "write", (int) wrote );

    pstatus("CE-OS-TEST: write succeeded", "");
#endif // 0

    status = tgr_close( fd );

    if( status )
        return error("close", status);

    pstatus("CE-OS-TEST: closed fd", "" );
    //
    // stat the file and see if it thinks it has the same size we do
    //
    struct stat statb;

    status = tgr_stat( testfile, & statb );

    if( status )
        return error("stat", status);

    if( statb.st_size != sizeof(testdata) )
        return error("stat size", statb.st_size );

    pstatus("CE-OS-TEST: stat matches", "");
    //
    // reopen the file, read the data and compare to what we wrote
    //
    status = tgr_open( &fd, testfile, O_CREAT | O_RDONLY, 0666 );

    if( status )
        return error("open", status);

    pstatus("CE-OS-TEST: re-opened ", testfile );

#if 1
    ssize_t read;
    status = tgr_read( fd, &read, cmpdata, sizeof(cmpdata) );

    if( status < 0 || read != sizeof(testdata) )
        return error( "read", (int) read );

    for( int i = 0 ; i < TEST_DATA_LEN ; i++ ) {
        if( cmpdata[i] != testdata[i] )
            return error( "data cmp", i );
    }
    pstatus("CE-OS-TEST: data compared equal", "");
#endif // 0

    status = tgr_close( fd );

    if( status )
        return error("re-fileclose", status);

    pstatus("CE-OS-TEST: re-closed fd", "" );
    //
    // rm the file
    //
    status = tgr_unlink( testfile );
    if( status )
        return error("unlink", status);
    //
    // cd .. and rm our subdir
    //
    status = tgr_chdir( ".." );

    if( status )
        return error("chdir ..", status);
    //
    // Remove our detritus
    //
    status = tgr_rmdir( subdir );

    if( status )
        return error("rmdir ..", status);

    pstatus("CE-OS-TEST: test successful", "");

    return 0;
}


int main( int argc, char ** argv)
{
    return test_file_ops();
}
