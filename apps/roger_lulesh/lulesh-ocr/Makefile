ifndef OPT
	OPT=-O
endif

ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif

GCC	= gcc -std=c99 -DOCR
CFLAGS	= -fpic -g $(OPT)

all: e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh

e_sqrt.o:  $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/math/e_sqrt.c Makefile
	$(GCC) $(CFLAGS) \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/include \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libc/include \
	-c $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/math/e_sqrt.c

s_cbrt.o:  $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common/s_cbrt.c Makefile
	$(GCC) $(CFLAGS) \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/include \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common \
	-I $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libc/include \
	-c $(RMD_INSTALL_BASE)/newlib/src-1.19/newlib/libm/common/s_cbrt.c

FP.o:  FP.c FP.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c FP.c

MEM.o:  MEM.c MEM.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c MEM.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.c RAG.h AMO.h MEM.h FP.h Makefile
	$(GCC) $(CFLAGS) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o Makefile
	$(GCC) $(LDFLAGS) -o lulesh lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o $(LDLIBS)

run:	lulesh
	./lulesh

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	time nohup ./lulesh
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh out.map nohup.out
