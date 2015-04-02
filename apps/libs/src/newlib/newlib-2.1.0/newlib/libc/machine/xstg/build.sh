echo " /*" > test.S
echo "  **  jmp_buf:" >> test.S
echo "  **   r255-r508 r509(sp) r510(fp) r511(ra)" >> test.S
echo "  **   Uses 0-2048 of stack" >> test.S
echo "  */" >> test.S
echo "    .text" >> test.S
echo "    .globl setjmp" >> test.S
echo "    .align 8" >> test.S
echo "    .type setjmp, @function" >> test.S
echo "setjmp:    /* Function arguments start at r2 */" >> test.S

for r in {255..511}; do
  let o=($r-255)*8
  echo "    store r2, r"$r", "$o", 64" >> test.S
  if [ $r -eq 508 ]; then echo >> test.S; fi
done

echo >> test.S
echo "    movimm    r1, 0, 64	/* return value in r1! */" >> test.S
echo "    jlabs     r20, r63" >> test.S
echo "setjmp_end:" >> test.S
echo "    .size	setjmp, setjmp_end-setjmp" >> test.S
echo >> test.S
echo "    .globl longjmp" >> test.S
echo "    .align 8" >> test.S
echo "    .type longjmp, @function" >> test.S
echo "longjmp:    /* r2 is the jmpbuf, r3 is val */" >> test.S

for r in {255..511}; do
  let o=($r-255)*8
  echo "    load r"$r", r2, "$o", 64" >> test.S
  if [ $r -eq 508 ]; then echo >> test.S; fi
done

echo >> test.S
echo "    movimm    r1, 1, 64" >> test.S
echo "    select    r1, r3, r1, r3, 64 /* retVal = (val != 0) ? val : 1; */" >> test.S
echo "    jlabs     r20, r63" >> test.S
echo "longjmp_end:" >> test.S
echo "    .size longjmp, longjmp_end-longjmp" >> test.S

