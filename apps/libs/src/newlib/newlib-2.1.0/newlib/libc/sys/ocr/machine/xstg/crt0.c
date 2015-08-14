//
// XSTG crt0 code
//
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
//
// Defining USE_STACK causes start() to be given the top of stack as param 0
// and is assumed to be a pointer to argc and the rest of the argv/envp 'thunk'
// Otherwise, it is assumed that param 0, 1, and 2 (r0, r1, r2) are initialized
// as argc, argv, and envp, as CE OCR does.
//
#define USE_STACK   1
//
// Defining OCR_INIT includes the OCR startup calls and init() that allocates
// an ocrDb for the stack.
//
#ifdef OCR_INIT
#include <sys/ocr.h>

extern u64 _init( int argc, char** argv, ocrConfig_t *oc );
extern void _fini (void);

static void change_stack( int argc, char** argv, uint64_t sp );

#endif // OCR_INIT

char ** environ;

extern int main( int argc, char** argv, char **envp );
static void app_start( int argc, char ** argv ) __attribute__((used));

//
// SW alarm code to indicate successful crt startup
//
#define XE_READY    0xF0    // defined in xe-sim/include/xe-abi.h
//
// Utility types
//
typedef void (* fptr_t)(void);
typedef unsigned long * ulongptr_t;    // not provided by stdint.h :-(

//
// constructor list, w/placeholder first entry
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
extern uint64_t __eh_frame_hdr_size;
extern uint64_t __eh_frame_start;
extern uint64_t __eh_frame_size;

static struct eh_info {
    ulongptr_t hdr_start;
    ulongptr_t hdr_size;
    ulongptr_t frame_start;
    ulongptr_t frame_size;
} eh_object = {
    .hdr_start   = & __eh_frame_hdr_start,
    .hdr_size    = & __eh_frame_hdr_size,
    .frame_start = & __eh_frame_start,
    .frame_size  = & __eh_frame_size
};

void __get_eh_info( struct eh_info * info )
{
    *info = eh_object;
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

//
// Clear all our bss regions
// Everything is 8-byte aligned so we do it by longs (ints are 32b)
//
extern uint64_t _fbss_local, _ebss_local;
extern uint64_t _fbss_block, _ebss_block;
extern uint64_t _fbss, _ebss;

static void __init_bss()
{
    static struct {
        ulongptr_t start;
        ulongptr_t end;
    } regions[] = {
        { &_fbss_local,  &_ebss_local },   // local
        { &_fbss_block,  &_ebss_block },   // block
        { &_fbss,  &_ebss }                // global
    };
    for( int i = 0 ; i < sizeof(regions)/sizeof(regions[0]) ; i++ ) {
		ulongptr_t p = regions[i].start;
		ulongptr_t e = regions[i].end;

		for( ; p < e ; p++ )
			*p = 0L;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Everything starts here
// We assume that an XE is started with the following stack state:
//      [ argv and env strings]
//      NULL
//      ...
//      envp[0]
//      NULL
//      ...
//      argv[0]
// SP-> argc
//
// We also assume that the initial register state is:
//      R0 = argc
//      R1 = & argv[0]
//      R2 = & envp[0]
//      R509 = & argc   (SP)
//
__asm__
 (
  "    .org 0x0\n"
  "    .section .text,\"ax\"\n"
  "    .global _start\n"
  "    .align 8\n"
  "    .type _start,@function\n"
  "_start:\n"
#ifdef USE_STACK
  "    bitop1   r0, r509, 0, OR, 64\n"
#endif // USE_STACK
  "    movimm   r511, 0, 64\n"
  "    jlrel    r511, __start\n"
  "    movimm   r2, 0, 64\n"
  "    movimm   r3, 0, 64\n"
  "    alarm    193\n"        // XE_ASSERT_ERROR
  );

//
// Called from _start() to continue startup
//
static void start () __asm__ ("__start") __attribute__ ((used));

#ifdef USE_STACK
static void
start ( int *sp )
{
    int argc = *sp;
    char ** argv = (char **)sp + 1;
    char ** envp = argv;

    for( ; *envp ; envp++ )
        ;
     envp++;

#else // ! USE_STACK
static void
start (int argc, char **argv, char **envp)
{
#endif // USE_STACK

    environ = envp;

#ifdef OCR_INIT
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
    //
    // We finally call a function that will change to the newly allocated stack
    // and continue the starting of the app
    //
    change_stack( ocrConfig.userArgc, ocrConfig.userArgv, newstack_top );

#else // ! OCR_INIT
    //
    // notify of successful startup, to the point of calling user code
    // matching termination alarm is in _Exit()
    //
    __asm__ __volatile__(
        "alarm %2"
        : /* no outputs */ : "{r2}" (0), "{r3}" (0), "L" (XE_READY)
        );
    app_start( argc, argv );
#endif // OCR_INIT
    //
    // never get here
    //
}

static void app_start( int argc, char ** argv )
{
    //
    // zero bss
    //
    __init_bss();
    //
    // Do C++ startup
    //
    __do_global_ctors();
    //
    // Call the user program.
    //
    exit( main( argc, argv, environ ) );
}

#ifdef OCR_INIT
//
// We change stacks here. The new SP is in r2.
// app_start() just inherits the first 2 args (r0, r1).
// As a precaution we push a null return addr
// and if app_start() returns we alarm ASSERT_ERROR (0xC1 = 193)
// with line and file str addr = 0.
//
// Implementation works with no explicit opt and with -Os
//
static void change_stack( int argc, char** argv, uint64_t sp )
{
    //
    // push a null on the new stack
    //
	sp -= 8;
	*(uint64_t *)sp = 0;
    //
    // replace the stack ptr with new value
    // and then call app_start to start executing on the new stack
    // passing through argc and argv.
    // the compiler function prolog hasn't modified r0,
    // but we need to force it to preserve r1 (argv)
    // alarm on return
    //
    __asm__(
        "bitop1      r509, %0, 0, OR, 64\n"
        "    bitop1      r0, %1, 0, OR, 64\n"
        "    bitop1      r1, %2, 0, OR, 64\n"
        "    jlrel       r511, app_start\n"
        "    movimm      r2, 0, 64\n"
        "    movimm      r3, 0, 64\n"
        "    alarm       193"        // XE_ASSERT_ERROR
        : /* no outputs */ : "r" (sp), "r" (argc), "r" (argv)
        );
 }
 #endif // OCR_INIT
