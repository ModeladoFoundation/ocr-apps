#!/bin/sh
set -x
rm -f Makefile Makefile.am Makefile.in \
asincos.c atan_accurate.c atan_fast.c atan-itanium.c \
atan-pentium.c crlibm_private.c csh_fast.c expm1.c \
expm1-standalone.c exp-td.c exp-td-standalone.c \
log10-td.c log1p.c log2-td.c log.c log-de.c log-td.c \
pow.c rem_pio2_accurate.c \
trigo_accurate.c trigo_fast.c trigpi.c triple-double.c \
asincos.h atan_accurate.h atan_fast.h crlibm_config.h \
crlibm.h crlibm_private.h csh_fast.h double-extended.h \
expm1.h exp-td.h log10-td.h log2-td.h log-de.h \
log.h log-td.h pow.h rem_pio2_accurate.h tg-defs.h \
trigo_accurate.h trigo_fast.h trigpi.h triple-double.h \
config.guess config.log config.status config.sub configure \
configure.ac aclocal.m4 AUTHORS ChangeLog COPYING \
COPYING.LIB crlibm_config.h.in depcomp INSTALL \
install-sh missing NEWS README stamp-h1 TODO VERSION \
asincos.o atan_accurate.o atan_fast.o \
crlibm_private.o csh_fast.o expm1.o \
expm1-standalone.o exp-td.o exp-td-standalone.o \
log1p.o log.o pow.o rem_pio2_accurate.o \
trigo_accurate.o trigo_fast.o trigpi.o \
triple-double.o libcrlibm.a
rm -rf include scs_lib bin docs lib maple tests gappa autom4te.cache
