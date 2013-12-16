ifndef OPT
	OPT=-O3
endif

ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif

PREFIX=$(RMD_INSTALL_BASE)/bin/bin/rmd-linux-elf

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe
RMD_RT_LIBDIR =$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

CC	= $(PREFIX)-clang -DFSIM -frmd-extensions

CFLAGS	= -g -ggdb -g3 $(OPT) -fno-builtin \
	-I$(RMD_INSTALL_BASE)/xe-sim/include \
	-I$(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I$(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I$(RMD_INSTALL_BASE)/runtime/common \
	-I$(RMD_INSTALL_BASE)/common/include \
	-DHAVE_CONFIG_H -I. -I../scs_lib -g -O2 -g

## -DPROCESSOR_HAS_FMA=1

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_INSTALL) -T $(RMD_RT_INSTALL)/rmd-libxe-icache.ld    -static -Map=out.map
LDLIBS	= -lxert

CUT	= cut
GREP	= grep
RM	= rm
MKDIR	= mkdir

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.s: %.c
	$(CC) $(CFLAGS) -S -o $@ $<

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
	zero_scs.o 

all: libcrlibm.a test

scs_private.o:	../scs_lib/scs_private.c ../crlibm_config.h ../scs_lib/scs.h ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o scs_private.o -c ../scs_lib/scs_private.c

double2scs.o:	../scs_lib/double2scs.c ../crlibm_config.h ../scs_lib/scs.h ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o double2scs.o -c ../scs_lib/double2scs.c

print_scs.o:	../scs_lib/print_scs.c ../crlibm_config.h ../scs_lib/scs.h ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o print_scs.o -c ../scs_lib/print_scs.c

division_scs.o:	../scs_lib/division_scs.c ../crlibm_config.h ../scs_lib/scs.h ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o division_scs.o -c ../scs_lib/division_scs.c

addition_scs.o:	../scs_lib/addition_scs.c ../crlibm_config.h ../scs_lib/scs.h ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o addition_scs.o -c ../scs_lib/addition_scs.c

multiplication_scs.o: ../scs_lib/multiplication_scs.c  ../crlibm_config.h  ../scs_lib/scs.h  ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o multiplication_scs.o -c  ../scs_lib/multiplication_scs.c

scs2double.o:	 ../scs_lib/scs2double.c  ../crlibm_config.h  ../scs_lib/scs.h  ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o scs2double.o -c  ../scs_lib/scs2double.c

zero_scs.o:	 ../scs_lib/zero_scs.c  ../crlibm_config.h  ../scs_lib/scs.h  ../scs_lib/scs_private.h
	$(CC) $(CFLAGS) -o zero_scs.o -c  ../scs_lib/zero_scs.c

crlibm_private.o:	 ../crlibm_private.c ../crlibm.h ../crlibm_private.h ../xe-stuff.h
	$(CC) $(CFLAGS) -o crlibm_private.o -c  ../crlibm_private.c

triple-double.o:	 ../triple-double.c ../crlibm_private.h ../triple-double.h
	$(CC) $(CFLAGS) -o triple-double.o -c  ../triple-double.c

exp-td.o:	 ../exp-td.c ../crlibm.h
	$(CC) $(CFLAGS) -o exp-td.o -c  ../exp-td.c

exp-td-standalone.o:	 ../exp-td-standalone.c ../crlibm.h
	$(CC) $(CFLAGS) -o exp-td-standalone.o -c  ../exp-td-standalone.c

expm1-standalone.o:	 ../expm1-standalone.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../expm1.h
	$(CC) $(CFLAGS) -o expm1-standalone.o -c  ../expm1-standalone.c

expm1.o:	 ../expm1.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../expm1.h
	$(CC) $(CFLAGS) -o expm1.o -c  ../expm1.c

log.o:	 ../log.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../log.h
	$(CC) $(CFLAGS) -o log.o -c  ../log.c -Wno-division-by-zero

log1p.o:	 ../log1p.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../log-td.h
	$(CC) $(CFLAGS) -o log1p.o -c  ../log1p.c

rem_pio2_accurate.o:	 ../rem_pio2_accurate.c ../rem_pio2_accurate.h
	$(CC) $(CFLAGS) -o rem_pio2_accurate.o -c  ../rem_pio2_accurate.c -Wno-parentheses-equality

trigo_fast.o:	 ../trigo_fast.c ../crlibm.h ../crlibm_private.h ../trigo_fast.h
	$(CC) $(CFLAGS) -o trigo_fast.o -c  ../trigo_fast.c -Wno-parentheses-equality

trigo_accurate.o:	 ../trigo_accurate.c ../trigo_accurate.h
	$(CC) $(CFLAGS) -o trigo_accurate.o -c  ../trigo_accurate.c

trigpi.o:	 ../trigpi.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../trigpi.h
	$(CC) $(CFLAGS) -o trigpi.o -c  ../trigpi.c

asincos.o:	 ../asincos.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../asincos.h
	$(CC) $(CFLAGS) -o asincos.o -c  ../asincos.c

pow.o:	 ../pow.c ../crlibm.h ../crlibm_private.h ../triple-double.h ../pow.h
	$(CC) $(CFLAGS) -o pow.o -c  ../pow.c

atan_fast.o:	../atan_fast.c ../crlibm.h ../crlibm_private.h ../atan_fast.h
	$(CC) $(CFLAGS) -o atan_fast.o -c  ../atan_fast.c

atan_accurate.o:	 ../atan_accurate.c ../crlibm_private.h ../atan_accurate.h ../atan_fast.h
	$(CC) $(CFLAGS) -o atan_accurate.o -c  ../atan_accurate.c

csh_fast.o:	 ../csh_fast.c ../crlibm.h ../crlibm_private.h ../csh_fast.h ../triple-double.h
	$(CC) $(CFLAGS) -o csh_fast.o -c  ../csh_fast.c

libcrlibm.a:	$(OBJS)
	$(AR) cru ./libcrlibm.a $(OBJS)

test.o:	test.c ../crlibm.h ../xe-stuff.h
	$(CC) $(CFLAGS) -DRAG -o test.o -I.. -c test.c

test:	test.o libcrlibm.a
	$(LD) $(LDFLAGS) -o test test.o ./libcrlibm.a $(LDLIBS)

run:	test test.cfg
	$(MKDIR) -p logs
	$(RM) -f logs/*
	date
	$(RMD_INSTALL_BASE)/xe-sim/fsim -s -c test.cfg -c $(RMD_INSTALL_BASE)/xe-sim/configs/localhost.cfg
	date
	grep CONSOLE logs/*blk00.CE.00

clean:
	rm -f libcrlibm.a $(OBJS) test.o test out.map logs/* lib/* include/*

install: libcrlibm.a ../crlibm.h ../crlibm_config.h ../scs_lib/scs.h ../xe-stuff.h
	mkdir -p lib
	cp libcrlibm.a lib
	mkdir -p include
	cp ../crlibm.h ../crlibm_config.h ../scs_lib/scs.h ../xe-stuff.h include
