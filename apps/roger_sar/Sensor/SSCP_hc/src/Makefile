ifndef HC_HOME
	HC_HOME=/home/ragolliv/hc-fedora15-gcc-4.6-boost-1.46-r21561
endif

INCS =	common.h          

C_SRCS=	inputs.c \
	digspot.c \
	registration.c \
	cfar.c \
	sinc_interp.c \
	sinc.c \
	fftw3.c

C_OBJS= inputs.o \
	digspot.o \
	registration.o \
	cfar.o \
	sinc_interp.o \
	sinc.o \
	fftw3.o

HC_SRCS=main.hc \
	image_formation.hc \
	ccd.hc \
	backproj.hc 

HC_OBJS=main.o \
	image_formation.o \
	ccd.o \
	backproj.o

.SUFFIXES:
.SUFFIXES: .o .c .hc

CC=gcc
HCC=hcc

all: sscp

# -DDEBUG_SSCP can be used to output binary data for debugging purposes
CFLAGS=	-g -O3 -std=c99 -DFFTW_NO_Complex \
	-I.  -I$(HC_HOME)/include/hc \
	-DDEBUG_SSCP -DTRACE -DDEBUG_OFF -DRAG_PURE_FLOAT=0 \
 	-DRAG_IMPLICIT_INPUTS_OFF -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_DIG_SPOT_ON=0 -DRAG_AFFINE_ON=1 -DRAG_THIN_ON=0 \
	-DRAG_QSORT_ON=0 -DRAG_SINCOS

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
