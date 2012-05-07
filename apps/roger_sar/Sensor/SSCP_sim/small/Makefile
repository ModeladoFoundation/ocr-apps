ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../../../ss
endif

PREFIX=$(RMD_INSTALL_BASE)/bin/bin/rmd-linux-elf

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe
RMD_RT_LIBDIR =$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

FSIM	= $(RMD_INSTALL_BASE)/xe_sim/f_sim
POWER_ANALYSIS_SCRIPT  = $(RMD_INSTALL_BASE)/xe_sim/powerAnalysisScript.sh
POWER_ANALYSIS_GRAPHS  = $(RMD_INSTALL_BASE)/xe_sim/tools/powerResultDir/E*pdf
POWER_ANALYSIS_RUNLOGS = $(RMD_INSTALL_BASE)/xe_sim/tools/powerResultDir/UHPC.Power*

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
	-DDEBUG_SSCP_OFF -DRAG_DIG_SPOT_OFF -DRAG_AFFINE -DRAG_THIN_OFF \
	-DTRACE                               \
	-DTRACE_LVL_1_OFF -DTRACE_LVL_2_OFF   \
	-DTRACE_LVL_3_OFF -DTRACE_LVL_4_OFF   \
	-DDEBUG_OFF                           \
	-DDEBUG_LVL_1_OFF -DTRACE_LVL_2_OFF   \
	-DDEBUG_LVL_3_OFF -DDEBUG_LVL_4_OFF   \
	-DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_QSORT_OFF -DRAG_SINCOS -DRAG_PURE_FLOAT -DRAG_DRAM_OFF

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
	image_formation.o \
	digspot.o \
	back_proj.o \
	registration.o \
	ccd.o \
	cfar.o \
	sinc_interp.o \
	sinc.o \
	fftw3.o \
	rag_rmd.o \
	./Data.o \
	./PlatformPosition.o \
	./PulseTransmissionTime.o

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
	/usr/bin/time $(FSIM) -c $(TARGET).cfg
	grep -e 'x=0x' sscp.log.CE.00 | grep -e 'y=0x' | grep -e 'p=0x' >detects.txt
	../utils/detects_ascii_to_txt <detects.txt | sort >Detects.txt
	-egrep '(cur|ref)' sscp.log.CE.00 | grep 0x >images_debug.txt
	-../utils/viewImageTxt 1 <Detects.txt >cur.bmp
	-../utils/viewImageTxt 2 <Detects.txt >ref.bmp
	-../utils/viewImageTxt 3 <Detects.txt >all.bmp
	-grep corr sscp.log.CE.00 | grep 0x >corr_debug.txt
	-../utils/viewCorrTxt    >corr.bmp
	-diff Detects.txt Detects.gold
	-cmp  Detects.txt Detects.gold
	-wc   Detects.txt Detects.gold
####	grep -i CONSOLE sscp.log.CE.00


power:	run
	@$(POWER_ANALYSIS_SCRIPT) -r $(TARGET).cfg && echo -e "\nPower Analysis Graphs Generated:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_GRAPHS)
	@echo -e "\nPower Analysis Run Logs:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_RUNLOGS)	

clean:
	$(RM) -f $(TARGET) *.o out.map \
		*.log.* detects.txt Detects.txt \
		images_debug.txt corr_debug.txt \
		cur.bmp ref.bmp all.bmp corr.bmp \
		./Data.s ./PlatformPosition.s ./PulseTransmissionTime.s
