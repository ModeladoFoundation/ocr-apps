#!/bin/sh
set -x
tar --gzip --extract --strip-component=1 --file=../tar/crlibm-1.0beta4.tar.gz --show-transformed-name

## This option is not supported in this package --disable-assembly
./configure	--without-gmp	--without-mpfr	--without-ultim	--without-mcr \
		--disable-gmp	--disable-mpfr	--disable-ultim	--disable-mcr \
		--enable-sse2	--disable-dependency-tracking \
		--disable-double-extended --prefix=/dev/null

ex crlibm_config.h <<EOF
%s;#define CRLIBM_HAS_FPU_CONTROL 1;/* #undef CRLIBM_HAS_FPU_CONTROL -- RAG */;
%s;#define HAS_SSE2 1;/* #undef HAS_SSE2 -- RAG */;
%s;#define HAVE_FENV_H 1;/* #undef HAVE_FENV_H -- RAG */;
%s;#define HAVE_FLOAT_H 1;/* #undef HAVE_FLOAT_H -- RAG */;
%s;#define HAVE_LIBM 1;/* #undef HAVE_LIBM -- RAG */;
%s;#define HAVE_LONG_DOUBLE 1;/* #undef HAVE_LONG_DOUBLE -- RAG */;
%s;#define HAVE_LONG_DOUBLE_WIDER 1;/* #undef HAVE_LONG_DOUBLE_WIDER -- RAG */;
%s;#define HAVE_MEMORY_H 1;/* #undef HAVE_MEMORY_H -- RAG */;
%s;#define HAVE_STRINGS_H 1;/* #undef HAVE_STRINGS_H -- RAG */;
%s;#define HAVE_STRING_H 1;/* #undef HAVE_STRING_H -- RAG */;
%s;#define HAVE_SYS_STAT_H 1;/* #undef HAVE_SYS_STAT_H -- RAG */;
%s;#define HAVE_SYS_TYPES_H 1;/* #undef HAVE_SYS_TYPES_H -- RAG */;
%s;#define HAVE_UNISTD_H 1;/* #undef HAVE_UNISTD_H -- RAG */;
wq
EOF

grep RAG crlibm_config.h

ex crlibm_private.h <<EOF
%s;^#if defined(__GNUC__)$;#if defined(__GNUC__) \&\& \!defined(TG_ARCH) // RAG -- added TG_ARCH because llvm compiler doesn't have __buildin_fabs();
wq
EOF

grep TG_ARCH crlibm_private.h

ex crlibm_private.c <<EOF
25:a
#ifdef TG_ARCH
#include "tg-defs.h"
#endif
.
%s;^#if (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64))$;#if defined(CRLIBM_HAS_FPU_CONTROL) \&\& (defined(CRLIBM_TYPECPU_X86) || defined(CRLIBM_TYPECPU_AMD64)) // RAG -- added CRLIBM_HAS_FPU_CONTROL;
wq
EOF

grep TG_ARCH crlibm_private.c

ex trigo_accurate.c <<EOF
26:a
#ifdef TG_ARCH
#include "tg-defs.h"
#endif
.
wq
EOF

grep TG_ARCH trigo_accurate.c

cat >tg-defs.h <<EOF
#ifndef TG_DEFS_H
#define TG_DEFS_H

#ifdef TG_ARCH
#include "ocr.h"
#endif

#endif
EOF

grep TG_ARCH tg-defs.h

ex asincos.c <<EOF
%s/^  double deltatemp, deltah, deltal;$/  double deltatemp,deltah, __attribute__ ((unused)) deltal; \/\/ RAG -- avoid fatal warning/
wq
EOF

grep RAG asincos.c

#make clean
#make all
#make check
