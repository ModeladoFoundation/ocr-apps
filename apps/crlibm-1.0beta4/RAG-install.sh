#!/bin/sh
set -x
(cd ..;tar xzf crlibm-1.0beta4/crlibm-1.0beta4.tar.gz)
./configure	--without-gmp	--without-mpfr	-without-ultim	-without-mcr \
		--disable-gmp	--disable-mpfr	--disable-ultim	--disable-mcr \
		--enable-sse2	--disable-dependency-tracking --disable-double-extended \
		--prefix=`pwd`/x86_64

ex crlibm_config.h <<EOF
%s;#define CRLIBM_HAS_FPU_CONTROL 1;/* #undef CRLIBM_HAS_FPU_CONTROL */;
%s;#define HAS_SSE2 1;/* #undef HAS_SSE2 */;
%s;#define HAVE_FENV_H 1;/* #undef HAVE_FENV_H */;
%s;#define HAVE_FLOAT_H 1;/* #undef HAVE_FLOAT_H */;
%s;#define HAVE_LIBM 1;/* #undef HAVE_LIBM */;
%s;#define HAVE_LONG_DOUBLE 1;/* #undef HAVE_LONG_DOUBLE */;
%s;#define HAVE_LONG_DOUBLE_WIDER 1;/* #undef HAVE_LONG_DOUBLE_WIDER */;
%s;#define HAVE_MEMORY_H 1;/* #undef HAVE_MEMORY_H */;
%s;#define HAVE_STRINGS_H 1;/* #undef HAVE_STRINGS_H */;
%s;#define HAVE_STRING_H 1;/* #undef HAVE_STRING_H */;
%s;#define HAVE_SYS_STAT_H 1;/* #undef HAVE_SYS_STAT_H */;
%s;#define HAVE_SYS_TYPES_H 1;/* #undef HAVE_SYS_TYPES_H */;
%s;#define HAVE_UNISTD_H 1;/* #undef HAVE_UNISTD_H */;
2:a
#define RAG 1 // RAG -- post configure mods by install.sh
.
wq
EOF
grep RAG crlibm_config.h

ex crlibm_private.h <<EOF
%s;^#if defined(__GNUC__)$;#if defined(__GNUC__) \&\& \!defined(FSIM) // RAG -- added FSIM because llvm compiler doesn't have __buildin_fabs();
wq
EOF
grep FSIM crlibm_private.h

ex crlibm_private.c <<EOF
25:a
#ifdef FSIM
#include "xe-stuff.h"
#endif
.
%s;^#if (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64))$;#if defined(CRLIBM_HAS_FPU_CONTROL) \&\& (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64)) // RAG -- added CRLIBM_HAS_FPU_CONTROL;
wq
EOF
grep FSIM crlibm_private.c

ex trigo_accurate.c <<EOF
26:a
#ifdef FSIM
#include "xe-stuff.h"
#endif
.
wq
EOF
grep FSIM trigo_accrate.c

cat >xe-stuff.h <<EOF
#ifdef FSIM
#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"

#define  printf( ... )          xe_printf(__VA_ARGS__)
#define fprintf(filedesc, ... ) xe_printf(__VA_ARGS__)
#define fflush(filedesc)

#define exit(code) { xe_printf("exit(%d)\n",code); ocrShutdown(); }
#endif
EOF

ex asincos.c <<EOF
%s/^  double deltatemp, deltah, deltal;$/  double deltatemp,deltah, __attribute__ ((unused)) deltal; \/\/ RAG -- avoid fatal warning/
wq
EOF

make clean
make all
make check
make install

(cd xe-sim;make clean)
(cd xe-sim;make all)
(cd xe-sim;make test)
(cd xe-sim;make run)
(cd xe-sim;make install)
