XSTACK = ../../../..

OCR_INSTALL  =$(XSTACK)/ocr/runtime/ocr-x86/ocr-install
CRLIBM_INCDIR=$(XSTACK)/apps/crlibm-1.0beta4/x86_64/include
CRLIBM_LIBDIR=$(XSTACK)/apps/crlibm-1.0beta4/x86_64/lib

AS	= as
ASFLAGS	=

AR	= ar
ARFLAGS	= rcs

CC	= gcc
# -DDEBUG_SSCP can be used to output binary data for debugging purposes
# -D__float128=char is a fix for fftw-3.3 built with gcc 4.6 and quad precision
#  and building the app with hc.
#	-DDEBUG -DDEBUG_LVL_1 -DDEBUG_LVL_2 -DDEBUG_LVL_3 -DDEBUG_LVL_4 -DDEBUG_LVL_5 \
#	-DTRACE -DTRACE_LVL_1 -DTRACE_LVL_2 -DTRACE_LVL_3 -DTRACE_LVL_4 -DTRACE_LVL_5 \
#  TODO: file bugs keeping fftw from being built with hc
CFLAGS	= -g -O2 -std=c99 -DRAG_SIM_off \
	-DDEBUG_SSCP_off -DRAG_SIM_NULL_GUID_WORKAROUND \
	-DTRACE -DTRACE_LVL_1 \
	-DRAG_DIG_SPOT_ON=0 -DRAG_AFFINE_ON=1 -DRAG_THIN_ON=0 -DRAG_QSORT_ON=0 \
	-DRAG_PURE_FLOAT -DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_off \
	-DRAG_SINCOS -DRAG_PETER_DIST_AND_TRIG -DRAG_SPAD -DRAG_DRAM \
	-DGANESH_STRENGTH_RED_OPT \
	-I. -I$(OCR_INSTALL)/include -I $(CRLIBM_INCDIR)

LD	= gcc -std=c99
LDFLAGS= -L$(OCR_INSTALL)/lib -L$(CRLIBM_LIBDIR) -Xlinker -rpath=$(OCR_INSTALL)/lib
LDLIBS= -lcrlibm -locr

RM	= rm

TARGET  = sscp

INCS =	common.h fftw3.h rag_ocr.h

OBJS=	main.o \
	inputs.o \
	digspot.o \
	registration.o \
	cfar.o \
	sinc_interp.o \
	sinc.o \
	image_formation.o \
	back_proj.o \
	ccd.o \
	fftw3.o \
	rag_ocr.o \
	e_sqrt.o \
	ef_sqrt.o

BIN_OBJS=Data.o PlatformPosition.o PulseTransmissionTime.o

all: Data.c PlatformPosition.c PulseTransmissionTime.c $(TARGET)

%.i: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -E -o $@ $<

%.s: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -S -o $@ $<

%.o: %.s $(INCS) Makefile
	$(CC) -c -o $@ $<

%.o: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

Data.o: Data.c
	$(CC) $(CFLAGS) -c -o Data.o Data.c

PlatformPosition.o: PlatformPosition.c
	$(CC) $(CFLAGS) -c -o PlatformPosition.o PlatformPosition.c

PulseTransmissionTime.o: PulseTransmissionTime.c
	$(CC) $(CFLAGS) -c -o PulseTransmissionTime.o PulseTransmissionTime.c

e_sqrt.o:  $(XSTACK)/ss/newlib/src-1.19/newlib/libm/math/e_sqrt.c Makefile
	$(CC) $(CFLAGS) \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libm/include \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libm/common \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libc/include \
	-c $(XSTACK)/ss/newlib/src-1.19/newlib/libm/math/e_sqrt.c

ef_sqrt.o:  $(XSTACK)/ss/newlib/src-1.19/newlib/libm/math/ef_sqrt.c Makefile
	$(CC) $(CFLAGS) \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libm/include \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libm/common \
	-I $(XSTACK)/ss/newlib/src-1.19/newlib/libc/include \
	-c $(XSTACK)/ss/newlib/src-1.19/newlib/libm/math/ef_sqrt.c

Data.c:                       ../../datagen/tiny/Data.bin
	../../utils/dataImage 1 <../../datagen/tiny/Data.bin >Data.c

PlatformPosition.c:           ../../datagen/tiny/PlatformPosition.bin
	../../utils/dataImage 2 <../../datagen/tiny/PlatformPosition.bin >PlatformPosition.c

PulseTransmissionTime.c:      ../../datagen/tiny/PulseTransmissionTime.bin
	../../utils/dataImage 3 <../../datagen/tiny/PulseTransmissionTime.bin >PulseTransmissionTime.c

$(TARGET): $(INCS) $(OBJS) $(BIN_OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(BIN_OBJS) $(LDLIBS)

run:	$(TARGET) $(TARGET).cfg
#	./$(TARGET) -ocr:cfg ./$(TARGET).cfg
	./$(TARGET) -ocr:cfg ./$(TARGET).cfg >detects.txt

view:	detects.txt images_debug.bin corr_debug.bin
	egrep -e '(x=....\...m)' detects.txt | sort >Detects.txt
	../../utils/viewImage 1 <Detects.txt >cur.bmp
	../../utils/viewImage 2 <Detects.txt >ref.bmp
	../../utils/viewImage 3 <Detects.txt >all.bmp
	../../utils/viewCorr    >corr.bmp
	diff Detects.txt Detects.gold | more
	cmp  Detects.txt Detects.gold
	wc   Detects.txt Detects.gold

clean:
	$(RM) -rf $(TARGET) $(OBJS)

clean-all:
	$(RM) -rf $(TARGET) $(OBJS) $(BIN_OBJS) \
	Detects.txt detects.txt images_debug.bin corr_debug.bin \
	cur.bmp ref.bmp all.bmp corr.bmp \
	Data.c PlatformPosition.c PulseTransmissionTime.c
