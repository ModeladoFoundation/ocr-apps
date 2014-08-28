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
wq
EOF

#grep RAG crlibm_config.h

ex crlibm_private.h <<EOF
%s;^#if defined(__GNUC__)$;#if defined(__GNUC__) \&\& \!defined(TG_ARCH) // RAG -- added TG_ARCH because llvm compiler doesn't have __buildin_fabs();
wq
EOF

#grep TG_ARCH crlibm_private.h

ex crlibm_private.c <<EOF
25:a
#ifdef TG_ARCH
#include "tg-defs.h"
#endif
.
%s;^#if (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64))$;#if defined(CRLIBM_HAS_FPU_CONTROL) \&\& (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64)) // RAG -- added CRLIBM_HAS_FPU_CONTROL;
wq
EOF

#grep TG_ARCH crlibm_private.c

ex trigo_accurate.c <<EOF
26:a
#ifdef TG_ARCH
#include "tg-defs.h"
#endif
.
wq
EOF

#grep TG_ARCH trigo_accurate.c

cat >tg-defs.h <<EOF
#ifdef TG_ARCH
#include "ocr-std.h"

#define printf( ... )          PRINTF(__VA_ARGS__)
#define fprintf(filedesc, ... ) PRINTF(__VA_ARGS__)
#define fflush(filedesc)

#define exit(code) { PRINTF("exit(%d)\n",code); ocrShutdown(); }
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

(cd tg; make clean)
(cd tg; make all)
(cd tg; make test)
(cd tg; make run)
(cd tg; make install)

(cd x86; make clean)
(cd x86; make all)
(cd x86; make run)
(cd x86; make install)
