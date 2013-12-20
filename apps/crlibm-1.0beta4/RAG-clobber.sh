#!/bin/sh
# RAG-clobber.sh for xstack/apps/crlibm-1.0beta4
rm -rf	./docs ./gappa ./maple ./scs_lib ./tests ./xe-sim/logs ./x86_64
rm -f	./ocr/test ./xe-sim/out.map ./xe-sim/test
rm -f	./AUTHORS \
	./COPYING \
	./COPYING.LIB \
	./ChangeLog \
	./INSTALL \
	./Makefile \
	./Makefile.am \
	./Makefile.in \
	./NEWS \
	./README \
	./TODO \
	./VERSION \
	./aclocal.m4 \
	./asincos.c \
	./asincos.h \
	./atan-itanium.c \
	./atan-pentium.c \
	./atan_accurate.c \
	./atan_accurate.h \
	./atan_fast.c \
	./atan_fast.h \
	./config.guess \
	./config.log \
	./config.status \
	./config.sub \
	./configure \
	./configure.ac \
	./crlibm.h \
	./crlibm_config.h \
	./crlibm_config.h.in \
	./crlibm_private.c \
	./crlibm_private.h \
	./csh_fast.c \
	./csh_fast.h \
	./depcomp \
	./double-extended.h \
	./exp-td-standalone.c \
	./exp-td.c \
	./exp-td.h \
	./expm1-standalone.c \
	./expm1.c \
	./expm1.h \
	./install-sh \
	./log-de.c \
	./log-de.h \
	./log-td.c \
	./log-td.h \
	./log.c \
	./log.h \
	./log10-td.c \
	./log10-td.h \
	./log1p.c \
	./log2-td.c \
	./log2-td.h \
	./missing \
	./pow.c \
	./pow.h \
	./rem_pio2_accurate.c \
	./rem_pio2_accurate.h \
	./stamp-h1 \
	./trigo_accurate.c \
	./trigo_accurate.h \
	./trigo_fast.c \
	./trigo_fast.h \
	./trigpi.c \
	./trigpi.h \
	./triple-double.c \
	./triple-double.h \
	./xe-stuff.h 

rm -rf	./atan_fast.o \
	./asincos.o \
	./atan_accurate.o \
	./crlibm_private.o \
	./csh_fast.o \
	./expm1-standalone.o \
	./expm1.o \
	./exp-td.o \
	./exp-td-standalone.o \
	./log.o \
	./log1p.o \
	./pow.o \
	./rem_pio2_accurate.o \
	./trigo_fast.o \
	./trigo_accurate.o \
	./trigpi.o \
	./triple-double.o

rm -f	./ocr/test.o

rm -f	./xe-sim/crlibm_private.o \
	./xe-sim/triple-double.o \
	./xe-sim/exp-td.o \
	./xe-sim/exp-td-standalone.o \
	./xe-sim/expm1-standalone.o \
	./xe-sim/expm1.o \
	./xe-sim/log.o \
	./xe-sim/log1p.o \
	./xe-sim/rem_pio2_accurate.o \
	./xe-sim/trigo_fast.o \
	./xe-sim/trigo_accurate.o \
	./xe-sim/trigpi.o \
	./xe-sim/asincos.o \
	./xe-sim/pow.o \
	./xe-sim/atan_fast.o \
	./xe-sim/atan_accurate.o \
	./xe-sim/csh_fast.o \
	./xe-sim/scs_private.o \
	./xe-sim/double2scs.o \
	./xe-sim/print_scs.o \
	./xe-sim/division_scs.o \
	./xe-sim/addition_scs.o \
	./xe-sim/multiplication_scs.o \
	./xe-sim/scs2double.o \
	./xe-sim/zero_scs.o \
	./xe-sim/test.o \
	
rm -rf ./libcrlibm.a ./xe-sim/libcrlibm.a
