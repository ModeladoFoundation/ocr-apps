#ifndef TG
double tg_asm_sqrt(double arg) {
	double result;
	__asm__ __volatile__ (
		"sqrtsd %[aRes],%[aArg] \n\t"	// use the SSE2 scalar 64-bit sqrt
			: [aRes] "=x" (result)	// use xmm registers
			: [aArg] "x"  (arg)	// use xmm registers
			: "fpsr"		// thought this should be mxcsr
	);
	return result;
}
#else
double tg_asm_sqrt(double arg) {
	double result;
	__asm__ __volatile__ (
		"sqrtF %[aRes], %[aArg]\n\t"	// what is the op code on tg?
			: [aRes] "=r" (result)
			: [aArg] "r"  (arg)
			: // "FloatpointStatusRegisterAKAFlags" // what is this called
	);
	return result;
}
#endif
