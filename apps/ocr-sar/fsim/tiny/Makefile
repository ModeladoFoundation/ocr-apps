XSTACK = ../../../..

RT_INSTALL    =$(XSTACK)/ss/runtime/libxe
RT_LIBDIR     =$(XSTACK)/ss/bin/rmd-linux-elf/lib
CRLIBM_INCDIR =$(XSTACK)/apps/crlibm-1.0beta4/xe-sim/include
CRLIBM_LIBDIR =$(XSTACK)/apps/crlibm-1.0beta4/xe-sim/lib

PREFIX	      =$(XSTACK)/ss/bin/bin/rmd-linux-elf

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

CC	= $(PREFIX)-clang -frmd-extensions

# -DDEBUG_SSCP can be used to output binary data for debugging purposes
# -D__float128=char is a fix for fftw-3.3 built with gcc 4.6 and quad precision
#  and building the app with hc.
#	-DDEBUG -DDEBUG_LVL_1 -DDEBUG_LVL_2 -DDEBUG_LVL_3 -DDEBUG_LVL_4 -DDEBUG_LVL_5 \
#	-DTRACE -DTRACE_LVL_1 -DTRACE_LVL_2 -DTRACE_LVL_3 -DTRACE_LVL_4 -DTRACE_LVL_5 \
#  TODO: file bugs keeping fftw from being built with hc
#-frmd-bb-profiling

CFLAGS	= -g -ggdb -g3 -O3 -std=c99 -fno-builtin -DRAG_SIM \
	-DDEBUG_SSCP -DRAG_SIM_NULL_GUID_WORKAROUND -DRAG_NEW_BLK_SIZE \
	-DTRACE -DTRACE_LVL_1 \
	-DRAG_DIG_SPOT_ON=0 -DRAG_AFFINE_ON=1 -DRAG_THIN_ON=0 -DRAG_QSORT_ON=0 \
	-DRAG_PURE_FLOAT -DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_off \
	-DRAG_SINCOS -DRAG_PETER_DIST_AND_TRIG -DRAG_SPAD -DRAG_DRAM \
	-DGANESH_STRENGTH_RED_OPT \
	-I . \
	-I $(XSTACK)/ss/xe-sim/include \
	-I $(XSTACK)/ss/rmdkrnl/inc \
	-I $(XSTACK)/ss/runtime/libxe/inc \
	-I $(XSTACK)/ss/runtime/common \
	-I $(XSTACK)/ss/common/include \
	-I $(CRLIBM_INCDIR)

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RT_LIBDIR) -L$(RT_INSTALL) -L$(CRLIBM_LIBDIR) -T $(RT_INSTALL)/rmd-libxe-icache.ld -static -Map=out.map
LDLIBS	= -lcrlibm -lxert

CUT	= cut
GREP	= grep
EGREP	= egrep
DIFF	= diff
CMP	= cmp
WC	= wc
RM	= rm
MKDIR	= mkdir

TARGET  = sscp

INCS =	common.h fftw3.h rag_ocr.h

OBJS=	main.o \
	inputs.o \
	image_formation.o \
	digspot.o \
	back_proj.o \
	registration.o \
	ccd.o \
	cfar.o \
	sinc_interp.o \
	sinc.o \
	fftw3.o \
	rag_ocr.o \
	e_sqrt.o \
	ef_sqrt.o

BIN_OBJS=Data.o PlatformPosition.o PulseTransmissionTime.o

%.i: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -E -o $@ $<

%.s: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -S -o $@ $<

%.o: %.s $(INCS) Makefile
	$(AS) -o $@ $<

%.o: %.c $(INCS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(TARGET)

Data.o: Data.s
	$(AS) -o Data.o Data.s

PlatformPosition.o: PlatformPosition.s
	$(AS) -o PlatformPosition.o PlatformPosition.s

PulseTransmissionTime.o: PulseTransmissionTime.s
	$(AS) -o PulseTransmissionTime.o PulseTransmissionTime.s

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

Data.s:	../../datagen/tiny/Data.bin
	../../utils/dataImageAsm 1 <../../datagen/tiny/Data.bin >Data.s

PlatformPosition.s:	../../datagen/tiny/PlatformPosition.bin
	../../utils/dataImageAsm 2 <../../datagen/tiny/PlatformPosition.bin >PlatformPosition.s

PulseTransmissionTime.s:	../../datagen/tiny/PulseTransmissionTime.bin
	../../utils/dataImageAsm 3 <../../datagen/tiny/PulseTransmissionTime.bin >PulseTransmissionTime.s

$(TARGET): $(INCS) $(OBJS) $(BIN_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(BIN_OBJS) $(LDLIBS)

FSIM	= $(XSTACK)/ss/xe-sim/fsim
#MACHINE_CFG = $(XSTACK)/ss/xe-sim/configs/localhost.cfg
MACHINE_CFG = $(XSTACK)/ss/xe-sim/configs/eth-8-bar-4-blocks.cfg

run:	$(TARGET) $(TARGET).cfg
	$(RM) -rf detects.txt images_debug.txt corr_debug.txt
	$(RM) -rf logs
	$(MKDIR) -p logs
	/usr/bin/time $(FSIM) -s -c $(TARGET).cfg -c $(MACHINE_CFG)
	$(GREP) -h -e 'x=0x' logs/sscp.log.*CE.00 | $(GREP) -e 'y=0x' | $(GREP) -e 'p=0x' >detects.txt

view:	./detects.txt
	../../utils/detects_ascii_to_txt <detects.txt | sort >Detects.txt
	-$(EGREP) -h '(>>> cur|>>> ref)' logs/sscp.log.*CE.00 | $(GREP) 0x >images_debug.txt
	-../../utils/viewImageTxt 1 <Detects.txt >cur.bmp
	-../../utils/viewImageTxt 2 <Detects.txt >ref.bmp
	-../../utils/viewImageTxt 3 <Detects.txt >all.bmp
	-$(GREP) -h '>>> corr' logs/sscp.log.*CE.00 | $(GREP) 0x >corr_debug.txt
	-../../utils/viewCorrTxt    >corr.bmp
	-$(DIFF) Detects.txt Detects.gold
	-$(CMP)  Detects.txt Detects.gold
	-$(WC)   Detects.txt Detects.gold
####	$(GREP) -i CONSOLE sscp.log.CE.00

clean-log:
	$(RM) -f logs/$(TARGET).log.*

clean:
	$(RM) -f $(TARGET) $(OBJS)

clean-all:
	$(RM) -f $(TARGET) $(OBJS) $(BIN_OBJS) \
		Detects.txt detects.txt out.map \
		images_debug.txt corr_debug.txt \
		cur.bmp ref.bmp all.bmp corr.bmp \
		./Data.s ./PlatformPosition.s ./PulseTransmissionTime.s \
		logs/$(TARGET).log.*
