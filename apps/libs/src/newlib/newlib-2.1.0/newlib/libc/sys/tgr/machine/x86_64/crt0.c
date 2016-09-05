//
// libc/sys/tgr/machine/x86_64/crt0.c
// - Run-time initialization with the TG runtime
//
// Start running - collect our argc, argv, env, call _init() to change stacks
// and then call main()
//

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/tgr.h>


extern char ** environ;

extern int main( int argc,char **argv, char **envp );

static void __do_global_ctors( void );

/* x86_64 entry point stack state

   %rdx Contains a function pointer to be registered with `atexit'.
        This is how the dynamic linker arranges to have DT_FINI
        functions called for shared libraries that have been loaded
        before this code runs.

   %rsp The stack contains the arguments and environment:
        0(%rsp)             argc
        8(%rsp)             argv[0]
        ...
        (8*argc)(%rsp)      NULL
        (8*(argc+1))(%rsp)  envp[0]
        ...
                            NULL
 */
void _real_start( uint64_t * psp );

//
// Everything starts here ...
//
void _start()
{
    //
    // The compiler will push/save a reg for this local so we
    // compensate for that when calling _real_start.
    // have no return address clear the frame pointer.
    //
    uint64_t * psp;
    __asm__( "mov %%rsp, %0" : "=r" (psp) );    // grab the sp
    __asm__( "and  $0xfffffffffffffff0,%rsp" ); // make sure sp is on 16 byte boundry
    //__asm__( "xorl %ebp, %ebp" );               // clear FP TODO: this is causing a segfault when we try to call _real_start
    _real_start( psp + 3 ); // pass the addr of ABI area
}

//
// Note: This cannot be made static as the compiler will cleverly figure out
// that it is only called by _start() and so merge the 2 functions, breaking
// the code in _start().
//
// NOTE: we assume that the bss is cleared before we get here.
//
void _real_start( uint64_t * psp )
{
    /*
     * The argument block begins at 'psp', captured by _start()
     * The calculation assumes x86_64 with uint64_t int and pointers.
     */
    int64_t argc = * (psp);
    char **argv = (char **) (psp + 1);

    char ** environ = argv + argc + 1;  // accounting for the terminating NULL
    //
    // Allocate our stack
    //

    if (tgr_init( &argc, &argv, &environ ) == -1)
        exit( -1 );

    //
    // prepare for the end
    //
    if (atexit (tgr_fini) != 0)
        exit (-1);

    //
    // Do C++ startup
    //
    //__do_global_ctors(); //TODO re-enable. Currently crashes

    /* Note: do not clear the .bss section.  When running with shared
     *       libraries, certain data items such __mb_cur_max or environ
     *       may get placed in the .bss, even though they are initialized
     *       to non-zero values.  Clearing the .bss will end up zeroing
     *       out their initial values.  The .bss is already initialized
     *       by this time by Linux.  */

    exit( main( argc, argv, environ ) );
}

//
// Utility types
//
typedef void (* fptr_t)(void);
typedef unsigned long * ulongptr_t;    // not provided by stdint.h :-(

void *__dso_handle __attribute__ ((weak));

//
// C++ support - constructor list, w/placeholder first entry
// Copied from XSTG implementation
//
const fptr_t __CTOR_LIST__[]
    __attribute__ (( aligned(sizeof(void *)) ))
    __attribute__ (( section(".ctors") )) = { (fptr_t) -1, };

extern const fptr_t __CTOR_LIST_END__[];

//
// TODO: since clang registers destructors w/atexit() there's
//       no need to keep a destructor list - remove
// destructor list, w/placeholder first entry
//
const fptr_t __DTOR_LIST__[]
    __attribute__ (( aligned(sizeof(void *)) ))
    __attribute__ (( section(".dtors") )) = { (fptr_t) -1, };

//
// Stack unwinding support
//
// eh_frame_hdr and eh_frame list
// The sections are added by the ldscript and it creates
// the following symbols for access.
// Sizes may be 0.
//
extern uint64_t __eh_frame_hdr_start;
extern uint64_t __eh_frame_hdr_end;
extern uint64_t __eh_frame_start;
extern uint64_t __eh_frame_end;

struct eh_info {
    ulongptr_t hdr_start;
    ulongptr_t hdr_size;
    ulongptr_t frame_start;
    ulongptr_t frame_size;
};

void __get_eh_info( struct eh_info * info )
{
    info->frame_start = &__eh_frame_start;
    info->frame_size  = &__eh_frame_end - &__eh_frame_start;

    info->hdr_start = &__eh_frame_hdr_start;
    info->hdr_size  = &__eh_frame_hdr_end - &__eh_frame_hdr_start;

    if ( info->hdr_size == 0 || info->hdr_start == 0 )
        info->hdr_size = info->hdr_start = 0;
}
//
// initialize any static constructors
//  Note that destructors are registered w/atexit() by clang in the ctors,
//  so no corresponding __do_global_dtors().
//
static void __do_global_ctors( void )
{
    for (const fptr_t *p = __CTOR_LIST_END__; p > __CTOR_LIST__ + 1; ) {
        (*(*--p))();
    }
}
