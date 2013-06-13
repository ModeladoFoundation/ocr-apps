ifndef OPT
	OPT=-O
endif

ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif
ifndef OCR_INSTALL
	OCR_INSTALL = ../../../ocr/runtime/ocr-x86/ocr-install
endif

GCC	= gcc -std=c99 -DOCR
CFLAGS	= -fpic -g $(OPT) -I$(OCR_INSTALL)/include \
	-DOCR_SPAD_WORKAROUND=0 # 0 (use spad_alloc/spad_free or 1 (malloc/free)
LDFLAGS	= -fpic -g $(OPT) -L$(OCR_INSTALL)/lib -locr
LDLIBS	= -locr

all: RAG_edt.o e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh

RAG_edt.o:  RAG_edt.c RAG_edt.h kernels.h FP.h MEM.h AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c RAG_edt.c

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

lulesh.o:  lulesh.c RAG.h AMO.h MEM.h FP.h RAG_edt.h kernels.h initialize.h Makefile
	$(GCC) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o RAG_edt.o Makefile
	$(GCC) $(LDFLAGS) -o lulesh lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o RAG_edt.o $(LDLIBS)

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
	rm -f RAG_edt.o e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh out.map
