ifndef HC_HOME
	HC_HOME=/home/ragolliv/hc-fedora15-gcc-4.6-boost-1.46-r21561
endif

INCS =	common.h          

C_SRCS=	inputs.c \
	digspot.c \
	sinc_interp.c \
	sinc.c \
	fftw3.c \
	RAG_fetch_and_add.c

C_OBJS= inputs.o \
	digspot.o \
	sinc_interp.o \
	sinc.o \
	fftw3.o \
	RAG_fetch_and_add.o

HC_SRCS=main.hc \
	image_formation.hc \
	backproj.hc \
	registration.hc \
	ccd.hc \
	cfar.hc

HC_OBJS=main.o \
	image_formation.o \
	backproj.o \
	registration.o \
	ccd.o \
	cfar.o

.SUFFIXES:
.SUFFIXES: .o .c .hc

CC=gcc
HCC=hcc

all: sscp

# -DDEBUG_SSCP can be used to output binary data for debugging purposes
CFLAGS=	-g -O3 -std=c99 -DFFTW_NO_Complex \
	-I.  -I$(HC_HOME)/include/hc \
	-DDEBUG_SSCP -DTRACE -DDEBUG_OFF -DRAG_PURE_FLOAT \
 	-DRAG_IMPLICIT_INPUTS_OFF -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_DIG_SPOT_OFF -DRAG_AFFINE_ON -DRAG_THIN_OFF \
	-DRAG_QSORT_OFF -DRAG_SINCOS

LDFLAGS= -lpthread -L$(HC_HOME)/lib -lhc -lxml2 -lm

sscp:	$(C_OBJS) $(HC_OBJS)
	$(CC) -o sscp $(C_OBJS) $(HC_OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.hc.o:
	$(HCC) $(CFLAGS) -rose:C99 -c $<

$(C_OBJS):	$(INCS) $(C_SRCS)

$(HC_OBJS):	$(INCS) $(HC_SRCS)

clean:
	$(RM) sscp *.o rose_*.c
