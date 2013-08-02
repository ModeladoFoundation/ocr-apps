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

CC	= $(PREFIX)-clang -DFSIM -DFSIM_DEBUG -frmd-extensions
CFLAGS	= -g -ggdb -g3 $(OPT) -fno-builtin \
	-I$(RMD_INSTALL_BASE)/xe-sim/include \
	-I$(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I$(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I$(RMD_INSTALL_BASE)/runtime/common \
	-I$(RMD_INSTALL_BASE)/common/include

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_INSTALL) -T ./rmd-libxe-big.ld    -static -Map=out.map
LDLIBS	= -lxert

CUT	= cut
GREP	= grep
RM	= rm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.s: %.c
	$(CC) $(CFLAGS) -S -o $@ $<

all: RAG_edt.o e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh

e_sqrt.o:  $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/math/e_sqrt.c Makefile
	$(CC) $(CFLAGS) \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/include \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libc/include \
	-c $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/math/e_sqrt.c

s_cbrt.o:  $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common/s_cbrt.c Makefile
	$(CC) $(CFLAGS) \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/include \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libc/include \
	-c $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common/s_cbrt.c

RAG_edt.o:  RAG_edt.c RAG_edt.h kernels.h RAG.h AMO.h FP.h MEM.h Makefile
	$(CC) $(CFLAGS) -c RAG_edt.c

FP.o:  FP.c FP.h RAG.h Makefile
	$(CC) $(CFLAGS) -c FP.c

MEM.o:  MEM.c MEM.h RAG.h Makefile
	$(CC) $(CFLAGS) -c MEM.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(CC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.c RAG.h RAG_edt.h kernels.h initialize.h AMO.h MEM.h FP.h Makefile
	$(CC) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o RAG_edt.o Makefile
	$(LD) $(LDFLAGS) -o lulesh lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o RAG_edt.o $(LDLIBS)

run:	lulesh
	$(RM) -f logs/*
	date
	$(RMD_INSTALL_BASE)/xe-sim/fsim -s -c lulesh.cfg -c $(RMD_INSTALL_BASE)/xe-sim/configs/eth-32-thors-8-blocks.cfg
	date
	grep CONSOLE */*CE.00 | grep -i RAG:

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	time nohup $(RMD_INSTALL_BASE)/xe-sim/fsim -s -c lulesh.cfg -c $(RMD_INSTALL_BASE)/xe-sim/configs/localhost.cfg
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f RAG_edt.o e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh out.map
