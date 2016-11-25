//
// XSTG crt0 code
//
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/tgr.h>

char ** environ;

extern int main( int argc, char** argv, char **envp );
static void app_start( int argc, char ** argv, char ** envp ) __attribute__((used));

//
// SW alarm code to indicate successful crt startup
//
#define XE_READY    0xF0    // defined in xe-sim/include/xe-abi.h
//
// Utility types
//
typedef void (* fptr_t)(void);
typedef unsigned long * ulongptr_t;    // not provided by stdint.h :-(

//////////////////// Stack unwinding support /////////////////////////
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

struct eh_info {
    ulongptr_t hdr_start;
    ulongptr_t hdr_size;
    ulongptr_t frame_start;
    ulongptr_t frame_size;
};

void __get_eh_info( struct eh_info * info )
{
    info->hdr_start   = & __eh_frame_hdr_start;
    info->hdr_size    = & __eh_frame_hdr_size;
    info->frame_start = & __eh_frame_start;
    info->frame_size  = & __eh_frame_size;

    if ( info->hdr_size == 0 || info->hdr_start == 0 )
        info->hdr_size = info->hdr_start = 0;
}

///////////////////// C++ constructor support ////////////////////////
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

///////////////////// BSS initialization ////////////////////////////
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

/////////////////////// PIE runtime relocations //////////////////////
// Do the runtime data relocations necessary due to pointers in
// initialized data. We assume the RAR registers have been initialized
// before we started running and that all locations are 8-byte aligned.
//
__attribute__((weak))
extern unsigned long __rtreloc_start;

__attribute__((weak))
extern unsigned long __rtreloc_end;

__attribute__((weak))
extern unsigned long __rtreloc_size;

typedef struct {
    unsigned long target_offset : 32,
                  target_rar    : 16,
                  symbol_rar    : 16;
} rtreloc_entry;

#define AR_PRF_ADDR ((ulongptr_t)0x2e000L)    // XE agent relative PRF address
#define REG_ADDR(r) (AR_PRF_ADDR + (r))


void __rtreloc( void )
{
    rtreloc_entry * entries = (rtreloc_entry *) & __rtreloc_start;
    uint64_t reloc_count = ((uint64_t) &__rtreloc_size) >> 3;

    for(  ; reloc_count > 0 ; entries++, reloc_count-- ) {
        if( entries->symbol_rar != 0 && entries->target_rar != 0 ) {
            ulongptr_t target =
                    (ulongptr_t) (*REG_ADDR(entries->target_rar) +
                                          entries->target_offset);
            *target += * REG_ADDR(entries->symbol_rar);
        }
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
  "    bitop1   r0, r509, 0, OR, 64\n"
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

static void
start ( int *sp )
{
    int argc = *sp;
    char ** argv = (char **)sp + 1;
    char ** envp = argv;

    for( ; *envp ; envp++ )
        ;
     envp++;

    __rtreloc();

    //
    // Have tgr parse argc/argv
    //
    if (tgr_init( &argc, &argv, &envp ) == -1)
        exit(-1);

    //
    // prepare for the end
    //
    if (atexit (tgr_fini) != 0)
        exit (-1);

    __asm__ __volatile__(
        "alarm %2"
        : /* no outputs */ : "{r2}" (0), "{r3}" (0), "L" (XE_READY)
        );

    app_start( argc, argv, envp );
}

static void app_start( int argc, char ** argv, char ** envp )
{
    //
    // zero bss
    //
    __init_bss();

    environ = envp;
    //
    // Do C++ startup
    //
    __do_global_ctors();
    //
    // Call the user program.
    //
    exit( main( argc, argv, environ ) );
}
