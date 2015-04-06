//
// XSTG crt0 code
//
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ocr.h>
//
// Everything starts here
//
__asm__
 (
  " .org 0x0\n"
  " .section .init,\"ax\"\n"
  " .global _start\n"
  " .align 8\n"
  " .type _start,@function\n"
  "_start:\n"
  " jrel __start\n"
  );

extern u64 _init( int argc, char** argv, ocrConfig_t *oc );
extern void _fini (void);
extern int main( int argc, char** argv, char **envp );
static void change_stack( int argc, char** argv, uint64_t sp );
//
// Called from _start() to continue startup
//
static void start () __asm__ ("__start") __attribute ((__used__));
static void
start (int argc, char **argv, char **env)
{
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
    //
    // never get here
    //
}

static void app_start( int argc, char ** argv )
{
    // Call the user program.
    //
    exit( main( argc, argv, environ ) );
}
//
// We change stacks here. The new SP is in r2.
// app_start() just inherits the first 2 args (r0, r1).
// As a precaution we push a null return addr
// and if app_start() returns we alarm ASSERT_ERROR (0xC1 = 193)
// with line and file str addr = 0.
//
__asm__
 (
    ".text\n"
    ".align    8\n"
    ".type    change_stack,@function\n"
"change_stack:\n"
    "bitop1   r509, r2, 0, OR, 64\n"
    "subI     r509, r509, 8, 64\n"
    "movimm   r2, 0, 64\n"
    "store    r2, r509, 0, 64\n"
    "jlrel    r511, app_start\n"
    "movimm   r2, 0, 64\n"
    "movimm   r3, 0, 64\n"
    "alarm    193\n"
".Lcstmp:\n"
    ".size    change_stack, .Lcstmp-change_stack\n"
 );
