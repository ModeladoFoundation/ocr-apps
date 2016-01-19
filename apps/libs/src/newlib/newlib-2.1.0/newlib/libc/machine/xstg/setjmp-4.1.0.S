 /*
  ** jmp_buf:
  **   r64-r508 r509(sp) r510(fp) r511(ra) = 448 registers = 3584 bytes
  **  r0 = address of jmp_buf (incoming)
  **  r1 = return val for longjmp
  **  r2 = loop var - offset of current register to save (bytes)
  **  r3 = tmp reg
  **  REGS = 0x2E000 -> address of the XE's PRF (agent relative)
  **  Procedure -
  **    Since the first reg slot in jmp_buf is for R64, we subtract the offset
  **    of R64 in the PRF from our jmp_buf addr (r0) and then we can
  **    use the PRF reg offset (r2) as an offset into both the PRF and jmp_buf.
  **    After that we loop, copying from PRF to jmp_buf (setjmp) or jmp_buf to
  **    PRF (longjmp), incrementing our offset (r2) until we've reached the end
  **    of the PRF.
  */
    .text
    .globl setjmp
    .align 8
    .type setjmp, @function
setjmp:                            /* Function arguments start at r0 */
    movimm r2, 64 * 8, 64          /* r2 = offset of first register to store */
    subi   r0, r0, r2, 64          /* r0 = jmp_buf - initial reg offset */

.Lsetlp:
    load   r3, r2, 0x2E000, 64     /* load r3 with register at mem[r2 + REGS] */
    store  r3, r2, r0, 64          /* store r3 at mem[jmp_buf + r2 - init r2] */
    addi   r2, r2, 8, 64           /* incr loop var to next reg offset */
    cmpimm r3, r2, 512 * 8, 0, 64, ULT /* compare reg offset to end reg offset */
    jct    .Lsetlp, r3, 0, T       /* jump to longlp if reg off (r2) < R512 */

    movimm r1, 0, 64               /* return value in r1 */
    jabs   r511                    /* return */
setjmp_end:
    .size	setjmp, setjmp_end-setjmp

    .globl longjmp
    .align 8
    .type longjmp, @function
longjmp:                           /* r0 is the jmpbuf, r1 is val */
    movimm r2, 64 * 8, 64          /* r2 = offset of first register to store */
    subi   r0, r0, r2, 64          /* r0 = jmp_buf - initial reg offset */

.Llonglp:
    load   r3, r0, r2, 64          /* load r3 at mem[jmp_buf + r2 - init r2] */
    store  r3, r2, 0x2E000, 64     /* store r3 in register at mem[r2 + REGS] */
    addi   r2, r2, 8, 64           /* incr loop var to next register offset */
    cmpimm r3, r2, 512 * 8, 0, 64, ULT /* compare reg offset to end reg offset */
    jct    .Llonglp, r3, 0, T      /* jump to longlp if reg off (r2) < R512 */

    movimm r3, 1, 64
    select r1, r1, r3, r1, 64      /* retVal = (val != 0) ? val : 1; */
    jabs   r511
longjmp_end:
    .size longjmp, longjmp_end-longjmp
