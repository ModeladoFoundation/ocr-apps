# XSTG crt0 code

    .org 0x0
    .section .init, "ax"
    .global _start
    .align 8
    .type _start,@function

_start:
    # SP is set properly by rmdkrnl
    # Set max stack size

    movimm    r508, xstg_crt_sp_base, 64

    load      r508, r508, 0, 64     # Set to the max stack size desired
    subi      r508, r509, r508, 64  # r508 now contains min stack

    # Zero the bss area
    # Gets start of bss
    movimm    r0, _fbss_local, 64
    # Gets end of bss
    movimm    r1, _end_local, 64

    # Make sure the stack does not potentially
    # extend into the code/bss segment
    cmp       r2, r1, r508, 0x0, 64, ULE
    jct       _stkerr, r2, 0x1, T

    # We are going to zero by chunks of
    # 64 bits. It does not really matter if
    # we spill a little bit past '_end'
    movimm    r4, 0x0, 64
    cmp       r2, r0, r1, 0x0, 64, SLE
    jcf       _endl, r2, 0x0, T
_loop:
    store     r0, r4, 0, 64
    addi      r0, r0, 0x8, 64
    cmp       r2, r0, r1, 0x0, 64, SLE
    jct       _loop, r2, 0x0, T
_endl:
    # call .init code
    jlrel     r511, _init

    # use return value from _init to reset sp
# TBD

    # setup _fini to be called atexit
    movimm      r0, %hi(_fini), 32
    movimmshf32 r0, r0, %lo(_fini)
	jlrel     r511, atexit

    # Call main
    jlrel     r511, main

_stkerr:
    # Jump to exit
    jrel      _exit
    #
    # add a symbol which we will weakly define for default SP range check
    #
    .section ".data_local", "aw", @progbits
    .align  8
    .global xstg_crt_sp_base
    .weak   xstg_crt_sp_base
    .type   xstg_crt_sp_base, @object
    .size   xstg_crt_sp_base, 8
xstg_crt_sp_base:
    .quad 0x400

