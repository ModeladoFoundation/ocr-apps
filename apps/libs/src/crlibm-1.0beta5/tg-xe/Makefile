ARCH                 	?= tg-xe
OCR_TYPE                ?= tg-xe
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

CRLIBM_ROOT    ?= ../x86_64

CRLIBM_ROOT    := $(shell cd "${CRLIBM_ROOT}" && pwd)

TG_PREFIX     := $(TG_INSTALL)/bin/xstg-linux-elf

AS	= $(TG_PREFIX)-as
ASFLAGS	=

AR	= $(TG_PREFIX)-ar
ARFLAGS	= rcs

CC	= $(TG_PREFIX)-clang -DOCR -DTG_ARCH
#RAG TEST -fxstg-extensions

CP = cp
RM = rm

CFLAGS	= -g -ggdb -g3 -O3 -fno-builtin -fxstg-extensions \
	-DHAVE_CONFIG_H -I. -I$(CRLIBM_ROOT)/scs_lib -I$(OCR_INSTALL)/include

## -DPROCESSOR_HAS_FMA=1

OBJCOPY	= $(TG_PREFIX)-objcopy

CUT	= cut
GREP	= grep
RM	= rm
MKDIR	= mkdir

%.o:	%.c
	$(AT)$(CC) $(CFLAGS) -c -o $@ $<
%.s: %.c
	$(AT)$(CC) $(CFLAGS) -S -o $@ $<

OBJS=	crlibm_private.o \
	triple-double.o \
	exp-td.o \
	exp-td-standalone.o \
	expm1-standalone.o \
	expm1.o \
	log.o \
	log1p.o \
	rem_pio2_accurate.o \
	trigo_fast.o \
	trigo_accurate.o \
	trigpi.o \
	asincos.o \
	pow.o \
	atan_fast.o \
	atan_accurate.o \
	csh_fast.o \
	scs_private.o \
	double2scs.o \
	print_scs.o \
	division_scs.o \
	addition_scs.o \
	multiplication_scs.o \
	scs2double.o \
	zero_scs.o \
	e_sqrt.o ef_sqrt.o

.PHONY: squeaky
squeaky:
	$(AT)(cd $(CRLIBM_ROOT);./RAG-clobber.sh)

.PHONY: configure
configure: $(CRLIBM_ROOT)/Makefile

$(CRLIBM_ROOT)/Makefile: $(CRLIBM_ROOT)/RAG-install.sh
	$(AT)(cd $(CRLIBM_ROOT);./RAG-install.sh)

.PHONY: all
all: lib/libcrlibm.a

scs_private.o: $(CRLIBM_ROOT)/scs_lib/scs_private.c $(CRLIBM_ROOT)/crlibm_config.h $(CRLIBM_ROOT)/scs_lib/scs.h $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o scs_private.o -c $(CRLIBM_ROOT)/scs_lib/scs_private.c

double2scs.o: $(CRLIBM_ROOT)/scs_lib/double2scs.c $(CRLIBM_ROOT)/crlibm_config.h $(CRLIBM_ROOT)/scs_lib/scs.h $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o double2scs.o -c $(CRLIBM_ROOT)/scs_lib/double2scs.c

print_scs.o: $(CRLIBM_ROOT)/scs_lib/print_scs.c $(CRLIBM_ROOT)/crlibm_config.h $(CRLIBM_ROOT)/scs_lib/scs.h $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o print_scs.o -c $(CRLIBM_ROOT)/scs_lib/print_scs.c

division_scs.o: $(CRLIBM_ROOT)/scs_lib/division_scs.c $(CRLIBM_ROOT)/crlibm_config.h $(CRLIBM_ROOT)/scs_lib/scs.h $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o division_scs.o -c $(CRLIBM_ROOT)/scs_lib/division_scs.c

addition_scs.o: $(CRLIBM_ROOT)/scs_lib/addition_scs.c $(CRLIBM_ROOT)/crlibm_config.h $(CRLIBM_ROOT)/scs_lib/scs.h $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o addition_scs.o -c $(CRLIBM_ROOT)/scs_lib/addition_scs.c

multiplication_scs.o: $(CRLIBM_ROOT)/scs_lib/multiplication_scs.c  $(CRLIBM_ROOT)/crlibm_config.h  $(CRLIBM_ROOT)/scs_lib/scs.h  $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o multiplication_scs.o -c  $(CRLIBM_ROOT)/scs_lib/multiplication_scs.c

scs2double.o: $(CRLIBM_ROOT)/scs_lib/scs2double.c  $(CRLIBM_ROOT)/crlibm_config.h  $(CRLIBM_ROOT)/scs_lib/scs.h  $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o scs2double.o -c  $(CRLIBM_ROOT)/scs_lib/scs2double.c

zero_scs.o: $(CRLIBM_ROOT)/scs_lib/zero_scs.c  $(CRLIBM_ROOT)/crlibm_config.h  $(CRLIBM_ROOT)/scs_lib/scs.h  $(CRLIBM_ROOT)/scs_lib/scs_private.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o zero_scs.o -c  $(CRLIBM_ROOT)/scs_lib/zero_scs.c

crlibm_private.o: $(CRLIBM_ROOT)/crlibm_private.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/tg-defs.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o crlibm_private.o -c  $(CRLIBM_ROOT)/crlibm_private.c

triple-double.o: $(CRLIBM_ROOT)/triple-double.c $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o triple-double.o -c  $(CRLIBM_ROOT)/triple-double.c

exp-td.o: $(CRLIBM_ROOT)/exp-td.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o exp-td.o -c  $(CRLIBM_ROOT)/exp-td.c

exp-td-standalone.o: $(CRLIBM_ROOT)/exp-td-standalone.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o exp-td-standalone.o -c  $(CRLIBM_ROOT)/exp-td-standalone.c

expm1-standalone.o: $(CRLIBM_ROOT)/expm1-standalone.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/expm1.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o expm1-standalone.o -c  $(CRLIBM_ROOT)/expm1-standalone.c

expm1.o: $(CRLIBM_ROOT)/expm1.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/expm1.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o expm1.o -c  $(CRLIBM_ROOT)/expm1.c

log.o: $(CRLIBM_ROOT)/log.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/log.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o log.o -c  $(CRLIBM_ROOT)/log.c -Wno-division-by-zero

log1p.o: $(CRLIBM_ROOT)/log1p.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/log-td.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o log1p.o -c  $(CRLIBM_ROOT)/log1p.c

rem_pio2_accurate.o: $(CRLIBM_ROOT)/rem_pio2_accurate.c $(CRLIBM_ROOT)/rem_pio2_accurate.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o rem_pio2_accurate.o -c  $(CRLIBM_ROOT)/rem_pio2_accurate.c -Wno-parentheses-equality

trigo_fast.o: $(CRLIBM_ROOT)/trigo_fast.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/trigo_fast.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o trigo_fast.o -c  $(CRLIBM_ROOT)/trigo_fast.c -Wno-parentheses-equality

trigo_accurate.o: $(CRLIBM_ROOT)/trigo_accurate.c $(CRLIBM_ROOT)/trigo_accurate.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o trigo_accurate.o -c  $(CRLIBM_ROOT)/trigo_accurate.c

trigpi.o: $(CRLIBM_ROOT)/trigpi.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/trigpi.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o trigpi.o -c  $(CRLIBM_ROOT)/trigpi.c

asincos.o: $(CRLIBM_ROOT)/asincos.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/asincos.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o asincos.o -c  $(CRLIBM_ROOT)/asincos.c

pow.o: $(CRLIBM_ROOT)/pow.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/pow.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o pow.o -c  $(CRLIBM_ROOT)/pow.c

atan_fast.o: $(CRLIBM_ROOT)/atan_fast.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/atan_fast.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o atan_fast.o -c  $(CRLIBM_ROOT)/atan_fast.c

atan_accurate.o: $(CRLIBM_ROOT)/atan_accurate.c $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/atan_accurate.h $(CRLIBM_ROOT)/atan_fast.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o atan_accurate.o -c  $(CRLIBM_ROOT)/atan_accurate.c

csh_fast.o: $(CRLIBM_ROOT)/csh_fast.c $(CRLIBM_ROOT)/crlibm.h $(CRLIBM_ROOT)/crlibm_private.h $(CRLIBM_ROOT)/csh_fast.h $(CRLIBM_ROOT)/triple-double.h $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) -o csh_fast.o -c  $(CRLIBM_ROOT)/csh_fast.c

e_sqrt.o: $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/math/e_sqrt.c $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/include \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/common \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libc/include \
	-c $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/math/e_sqrt.c

ef_sqrt.o: $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/math/ef_sqrt.c $(CRLIBM_ROOT)/Makefile
	$(AT)$(CC) $(CFLAGS) \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/include \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/common \
	-I $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libc/include \
	-c $(APPS_LIBS_ROOT)/newlib/newlib-2.1.0/newlib/libm/math/ef_sqrt.c

lib/libcrlibm.a: $(OBJS) Makefile $(CRLIBM_ROOT)/Makefile
	$(AT)$(AR) cru ./libcrlibm.a $(OBJS)
	$(AT)$(MKDIR) -p lib include
	$(AT)$(CP) -f libcrlibm.a lib/libcrlibm.a
	$(AT)$(CP) -f $(CRLIBM_ROOT)/crlibm.h include/crlibm.h

include/crlibm.h: lib/libcrlibm.a

.PHONY: clean
clean:
	$(AT)$(RM) -f out.map libcrlibm.a $(OBJS)
	$(AT)$(RM) -rf lib include

$(APPS_LIBS_INSTALL)/include/crlibm.h: include/crlibm.h | $(APPS_LIBS_INSTALL)/include
	$(AT)$(CP) -f include/crlibm.h $(APPS_LIBS_INSTALL)/include/crlibm.h

$(APPS_LIBS_INSTALL)/lib/libcrlibm.a: lib/libcrlibm.a | $(APPS_LIBS_INSTALL)/lib
	$(AT)$(CP) -f lib/libcrlibm.a  $(APPS_LIBS_INSTALL)/lib/libcrlibm.a

.PHONY: install
install: $(APPS_LIBS_INSTALL)/lib/libcrlibm.a $(APPS_LIBS_INSTALL)/include/crlibm.h

.PHONY: uninstall
uninstall:
	$(AT)$(RM) -f $(APPS_LIBS_INSTALL)/lib/libcrlibm.a
	$(AT)$(RM) -f $(APPS_LIBS_INSTALL)/include/crlibm.h

.PHONY: $(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@

.PHONY: debug
debug:
	echo "Currently no tg support for make debug"

.PHONY: check
check:
	echo "Currently no tg support for make check"

.PHONY: run
run:
	echo "Currently no tg support for make run"
