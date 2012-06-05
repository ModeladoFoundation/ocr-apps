ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif
ifndef AFL_INSTALL_BASE
	AFL_INSTALL_BASE = ../../../tools/rmd_afl
endif

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe

CC=gcc

CFLAGS= -g -O3 -std=c99 -DRAG_AFL -Wall \
	-I $(AFL_INSTALL_BASE)/common \
	-I $(AFL_INSTALL_BASE)/rmd_memory \
	-I $(AFL_INSTALL_BASE)/rmd_threading \
	-I $(AFL_INSTALL_BASE) \
	-I $(AFL_INSTALL_BASE)/rmd_codelet \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I. \
	-DTRACE     -DCHECK     -DDEBUG  -DRAG_ATOMIC=1

LDFLAGS=$(AFL_INSTALL_BASE)/rmd_codelet/rmd_codelet.a \
	$(AFL_INSTALL_BASE)/rmd_afl.a \
	-lpthread -lxml2 -lc -ldl -lz -lm -lrt

INCS =	rag_rmd.h

SRCS=	rag_rmd.c gups.c 

all: gups

gups.s:	$(INCS) $(SRCS) Makefile
	$(CC) $(CFLAGS) -S gups.c

gups:	$(INCS) $(SRCS) Makefile
	$(CC) $(CFLAGS) -o gups $(SRCS) $(LDFLAGS)

run:	gups Makefile
	/usr/bin/time ./gups -c gups.cfg

check:	gups Makefile
	valgrind ./gups -d -c gups.cfg

clean:
	$(RM) gups
