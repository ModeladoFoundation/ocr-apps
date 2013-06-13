GCC=gcc -std=c99 -Dc99Const=
CXX=g++          -Dc99Const=const

all: lulesh-cpp lulesh-c99

lulesh-cpp: lulesh.cc domain.hh
	$(CXX) -g $(OPT) lulesh.cc -lm -o lulesh-cpp

lulesh-c99: lulesh.c domain.h
	$(GCC) -g $(OPT) -DRAG lulesh.c  -lm -o lulesh-c99

run:	lulesh-c99
	./lulesh-c99

clean:
	rm -f *~ lulesh-cpp lulesh-c99
