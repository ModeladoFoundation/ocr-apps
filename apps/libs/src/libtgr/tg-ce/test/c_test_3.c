//
// Simple test of ce_ clone/finish related interfaces:
//  clone, finish, waitpid, waitall, detach
//
// This test is non-newlib
//
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
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
//
// Convert a number to
char * int2str( char * buf, int64_t i, char t )
{
	static char chars[] = "0123456789abcdef";
    char sb[32];
	char * p = & sb[sizeof(sb)-1];
	int isnegative = 0;
    int b;          // base

    if( (t == 'D' || t == 'X') && i < 0 ) {
        i = -i;
        isnegative = 1;
    }
    if( t == 'x' || t == 'X' )
        b = 16;
    else if( t == 'd' || t == 'D' )
        b = 10;
    else
        return buf;

	*p = '\0';

	if( i == 0 ) {
		*--p = '0';
	} else {
        uint64_t u = i;
		while( u ) {
			int d = u % b;
			u = u / b;
			*--p = chars[d];
		}
	}
    if( b == 16 ) {
        *--p = 'x';
        *--p = '0';
    }
    if( isnegative )
        *--p = '-';
    return stpcpy( buf, p );
}

static int error( const char *op, int err )
{
    char buf[256];

    char *p = stpcpy( buf, "CE-OS-TEST: error ");
    p = stpcpy( p, op );
    p = stpcpy( p, " - " );
    p = int2str( p, err, 'd' );
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

static void istatus( const char *msg, int64_t i )
{
    char buf[64];
    char *p = buf;

    pid_t pid = tgr_getpid();

    p = stpcpy(p, " XE " );
    p = int2str( p, (int64_t) pid, 'x');
    p = stpcpy(p, ": ");
    p = int2str( p, i, 'x');
    pstatus( msg, buf );
}

int64_t clone( void * arg )
{
    istatus( "Clone", (int64_t) arg );

    pstatus( "In clone: ", (const char *) arg );

    return 0;
}

char arg_to_clone[] = "Arg to clone";

static int test_clone_op( void )
{
    pid_t pid;

    //
    // Test cloning a function and waiting for it to finish
    //
    int status = tgr_clone( & clone, arg_to_clone, & pid );

    if( status ) {
        return error( "clone", status );
    }
    int64_t clone_status;

    status = tgr_waitpid( pid, & clone_status, 1 );   // block
    if( status ) {
        return error("clone wait", status );
    }
    pstatus("CE-OS-TEST: clone test successful", "");
    //
    // Clone a few and then wait for all
    //
    if( tgr_clone( clone, arg_to_clone, & pid ) ||
        tgr_clone( clone, arg_to_clone, & pid ) ||
        tgr_clone( clone, arg_to_clone, & pid ) ) {
        return error( "waitall clone", 1 );
    }
    status = tgr_waitall();   // block

    if( status ) {
        return error("clone wait", status );
    }
    pstatus("CE-OS-TEST: waitall test successful", "");
    //
    // Detach a child and see if we get the expected returns
    // to waitpid()
    //
    if( tgr_clone( clone, arg_to_clone, & pid ) )
        return error( "detach clone", 1 );

    if( tgr_detach( pid ) )
        return error( "detach", 1 );

    if( tgr_waitpid( pid, & clone_status, 0 ) == 0 )
        return error( "detach waitpid no error!", 0 );

    if( errno != EPERM )
        return error( "detach wrong waitpid error", errno );
    //
    // it should eventually finish and a wait should should return ESRCH
    //
    for( int i = 0 ; i < 1000 ; i++ ) {
        if( tgr_waitpid( pid, & clone_status, 0 ) && errno == ESRCH )
            break;
        for( int j = 0 ; j < 100000 ; j++ )
            ;
        //istatus("waitpid iteration", i );
    }
    if( errno != ESRCH )
        return error( "detach clone wait for finish error", errno );

    pstatus("CE-OS-TEST: detach test successful", "");

    return 0;
}

int main( int argc, char ** argv)
{
    return test_clone_op();
}
