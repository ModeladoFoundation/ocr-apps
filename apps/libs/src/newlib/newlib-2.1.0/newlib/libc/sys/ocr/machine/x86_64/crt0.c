/* libc/sys/linux/crt0.c - Run-time initialization */

/* FIXME: This should be rewritten in assembler and
          placed in a subdirectory specific to a platform.
          There should also be calls to run constructors. */

/* Written 2000 by Werner Almesberger */


#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/ocr.h>


extern char ** environ;

extern u64 _init( int argc, char **argv, ocrConfig_t *oc );
extern void _fini (void);
extern int main( int argc,char **argv,char **envp );

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
    __asm__( "xorl %ebp, %ebp" );               // clear FP
    _real_start( psp + 1 ); // pass the addr of ABI area
}

//
// Note: This cannot be made static as the compiler will cleverly figure out
// that it is only called by _start() and so merge the 2 functions, breaking
// the code in _start().
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
    // Allocate our stack and have OCR copy and return a db version of argc/argv
    //
    ocrConfig_t ocrConfig;

    uint64_t newstack_top = _init( argc, argv, & ocrConfig );

    if( newstack_top == 0L )
        exit( -1 );
    //
    // prepare for the end
    //
    if (atexit (_fini) != 0)
      exit (-1);

    /* Note: do not clear the .bss section.  When running with shared
     *       libraries, certain data items such __mb_cur_max or environ
     *       may get placed in the .bss, even though they are initialized
     *       to non-zero values.  Clearing the .bss will end up zeroing
     *       out their initial values.  The .bss is already initialized
     *       by this time by Linux.  */

    exit( main( ocrConfig.userArgc, ocrConfig.userArgv, environ ) );
}
