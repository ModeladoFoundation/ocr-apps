ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = /home/ragolliv/trunk/ss
endif

PREFIX=$(RMD_INSTALL_BASE)/bin/bin/rmd-linux-elf

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe
RMD_RT_LIBDIR =$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

FSIM	= $(RMD_INSTALL_BASE)/xe_sim/f_sim
POWER	= $(RMD_INSTALL_BASE)/xe_sim/powerAnalysisScript.sh

CC	= $(PREFIX)-clang
# -DDEBUG_SSCP can be used to output binary data for debugging purposes

CFLAGS	= -g -ggdb -g3 -O1 -std=c99 -DRAG_SIM \
	-I $(RMD_INSTALL_BASE)/xe_sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/common/include \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I. \
	-DDEBUG_SSCP -DTRACE -DDEBUG_OFF -DRAG_PURE_FLOAT=1 \
	-DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_DIG_SPOT_ON=0 -DRAG_AFFINE_ON=1 -DRAG_THIN_ON=0 \
	-DRAG_QSORT_ON=0 -DRAG_SINCOS

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_LIBDIR) -L$(RMD_RT_INSTALL) -T ./sscp-with-newlib.ld -static -Map=out.map
LDLIBS	= -lxert --start-group -lc -lg -lm -lgloss_lxe --end-group

CUT	= cut
GREP	= grep
RM	= rm

TARGET  = sscp

INCS =	common.h fftw3.h rag_rmd.h

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
	rag_rmd.o \
	Data.o \
	PlatformPosition.o \
	PulseTransmissionTime.o

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<
	$(OBJCOPY) -R .data_local -R .data_block -R .bss_local -R .bss_block $@

%.o: %.s
	$(AS) -o $@ $<
	$(OBJCOPY) -R .data_local -R .data_block -R .bss_local -R .bss_block $@

%.s: %.c
	$(CC) $(CFLAGS) -fno-builtin -S -o $@ $<

Data.s:	../../datagen/small/Data.bin
	../utils/dataImageAsm 1 <../../datagen/small/Data.bin >Data.s

PlatformPosition.s:	../../datagen/small/PlatformPosition.bin
	../utils/dataImageAsm 2 <../../datagen/small/PlatformPosition.bin >PlatformPosition.s

PulseTransmissionTime.s:	../../datagen/small/PulseTransmissionTime.bin
	../utils/dataImageAsm 3 <../../datagen/small/PulseTransmissionTime.bin >PulseTransmissionTime.s

$(TARGET): $(INCS) $(OBJS) ./sscp-with-newlib.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

run:	$(TARGET) $(TARGET).cfg
	echo /usr/bin/time $(FSIM) -c $(TARGET).cfg
	grep -e 'x=0x' sscp.log.CE.00 | grep -e 'y=0x' | grep -e 'p=0x' >detects.txt
	../utils/detects_ascii_to_txt <detects.txt | sort >Detects.txt
	-diff Detects.txt Detects.gold
	-cmp  Detects.txt Detects.gold
	wc   Detects.txt Detects.gold
	egrep '(cur|ref)' sscp.log.CE.00 | grep 0x >images_debug.txt
	../utils/viewImageTxt 1 <Detects.txt >cur.bmp
	../utils/viewImageTxt 2 <Detects.txt >ref.bmp
	../utils/viewImageTxt 3 <Detects.txt >all.bmp
	grep corr sscp.log.CE.00 | grep 0x >corr_debug.txt
	../utils/viewCorrTxt    >corr.bmp
####	grep -i CONSOLE sscp.log.CE.00

power:	$(TARGET) $(TARGET).cfg
	$(POWER) -r $(TARGET).cfg

clean:
	$(RM) -f $(TARGET) *.o out.map
	$(RM) -f *.log.* detects.txt Detects.txt
	$(RM) -f images_debug.txt corr_debug.txt cur.bmp ref.bmp all.bmp corr.bmp
