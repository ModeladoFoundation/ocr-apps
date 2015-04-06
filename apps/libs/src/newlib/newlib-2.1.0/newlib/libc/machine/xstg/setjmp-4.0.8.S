 /*
 **  jmp_buf:
 **   r40-r60 r61(rsp) r62(fp) r63(ret addr)
 **   0-168
 */
    .text
    .globl setjmp
    .align 8
    .type setjmp, @function
setjmp:	/* Function arguments start at r2 */
    store64.loc r2, 0, r40
    store64.loc r2, 8, r41
    store64.loc r2, 16, r42
    store64.loc r2, 24, r43
    store64.loc r2, 32, r44
    store64.loc r2, 40, r45
    store64.loc r2, 48, r46
    store64.loc r2, 56, r47
    store64.loc r2, 64, r48
    store64.loc r2, 72, r49
    store64.loc r2, 80, r50
    store64.loc r2, 88, r51
    store64.loc r2, 96, r52
    store64.loc r2, 104, r53
    store64.loc r2, 112, r54
    store64.loc r2, 120, r55
    store64.loc r2, 128, r56
    store64.loc r2, 136, r57
    store64.loc r2, 144, r58
    store64.loc r2, 152, r59
    store64.loc r2, 160, r60 /* probably don't have to save/restore this ... */

    store64.loc r2, 168, r61
    store64.loc r2, 176, r62
    store64.loc r2, 184, r63

    movimmz     r1, 0	/* return value in r1! */
    jlabs       r20, r63
setjmp_end:
    .size setjmp, setjmp_end-setjmp

    .globl longjmp
    .align 8
    .type longjmp, @function
longjmp: /* r2 is the jmpbuf, r3 is val */
    load64.loc  r40, r2, 0
    load64.loc  r41, r2, 8
    load64.loc  r42, r2, 16
    load64.loc  r43, r2, 24
    load64.loc  r44, r2, 32
    load64.loc  r45, r2, 40
    load64.loc  r46, r2, 48
    load64.loc  r47, r2, 56
    load64.loc  r48, r2, 64
    load64.loc  r49, r2, 72
    load64.loc  r50, r2, 80
    load64.loc  r51, r2, 88
    load64.loc  r52, r2, 96
    load64.loc  r53, r2, 104
    load64.loc  r54, r2, 112
    load64.loc  r55, r2, 120
    load64.loc  r56, r2, 128
    load64.loc  r57, r2, 136
    load64.loc  r58, r2, 144
    load64.loc  r59, r2, 152
    load64.loc  r60, r2, 160 /* probably don't have to save/restore this ... */

    load64.loc  r61, r2, 168
    load64.loc  r62, r2, 176
    load64.loc  r63, r2, 184

    movimmz     r1, 1
    select      r1, r3, r1, r3 /* retVal = (val != 0) ? val : 1; */
    jlabs       r20, r63
longjmp_end:
    .size longjmp, longjmp_end-longjmp
