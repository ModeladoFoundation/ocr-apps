ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99 -DFSIM
CFLAGS= -fpic -g $(OPT)
LDFLAGS = 

all: e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh

e_sqrt.o:  ../../../ss/newlib/src-1.19/newlib/libm/math/e_sqrt.c Makefile
	$(GCC) $(CFLAGS) \
	-I../../../ss/newlib/src-1.19/newlib/libm/include \
	-I../../../ss/newlib/src-1.19/newlib/libm/common \
	-I../../../ss/newlib/src-1.19/newlib/libc/include \
	-c ../../../ss/newlib/src-1.19/newlib/libm/math/e_sqrt.c

s_cbrt.o:  ../../../ss/newlib/src-1.19/newlib/libm/common/s_cbrt.c Makefile
	$(GCC) $(CFLAGS) \
	-I../../../ss/newlib/src-1.19/newlib/libm/include \
	-I../../../ss/newlib/src-1.19/newlib/libm/common \
	-I../../../ss/newlib/src-1.19/newlib/libc/include \
	-c ../../../ss/newlib/src-1.19/newlib/libm/common/s_cbrt.c

FP.o:  FP.c FP.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c FP.c

MEM.o:  MEM.c MEM.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c MEM.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.c RAG.h AMO.h MEM.h FP.h Makefile
	$(GCC) $(CFLAGS) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o Makefile
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o MEM.o FP.o e_sqrt.o s_cbrt.o $(LDFLAGS)

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
	rm -f e_sqrt.o s_cbrt.o FP.o MEM.o AMO.o lulesh.o lulesh nohup.out
