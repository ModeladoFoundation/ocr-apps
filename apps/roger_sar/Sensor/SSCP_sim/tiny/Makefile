ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../../../ss
endif

ifdef RUN_SUFFIX
	RUN_SUFFIX := $(RUN_SUFFIX); export RUN_SUFFIX
else
	
endif	
ifdef TECH_NODE 
	TECH_NODE := $(TECH_NODE); export TECH_NODE
else
	
endif	

ifdef USE_ICACHE
 SSCP_NEWLIB=./sscp-with-newlib-icache.ld
else
 SSCP_NEWLIB=./sscp-with-newlib.ld
endif 

PREFIX=$(RMD_INSTALL_BASE)/bin/bin/rmd-linux-elf

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe
RMD_RT_LIBDIR =$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

FSIM	= $(RMD_INSTALL_BASE)/xe-sim/fsim 
POWER_ANALYSIS_SCRIPT  = $(RMD_INSTALL_BASE)/xe-sim/powerAnalysisScript.sh
POWER_ANALYSIS_SCRIPT_TECHSCALED  = $(RMD_INSTALL_BASE)/xe-sim/powerAnalysisScript-TechScaled.sh
POWER_ANALYSIS_GRAPHS  = $(RMD_INSTALL_BASE)/xe-sim/tools/powerResultDir/E*pdf
POWER_ANALYSIS_RUNLOGS = $(RMD_INSTALL_BASE)/xe-sim/tools/powerResultDir/UHPC.Power*
MACHINE_CFG = $(RMD_INSTALL_BASE)/xe-sim/configs/rogersFedora17-x64.cfg

CC	= $(PREFIX)-clang
# -DDEBUG_SSCP can be used to output binary data for debugging purposes
#	-DRAG_SPAD -DRAG_DRAM_OFF
#-frmd-bb-profiling 

CFLAGS	= -g -ggdb -g3 -O3 -std=c99 -frmd-extensions \
	-DRAG_SIM -I $(RMD_INSTALL_BASE)/xe-sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/common/include \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I. \
	-DDEBUG_SSCP      -DRAG_DIG_SPOT_OFF -DRAG_AFFINE -DRAG_THIN_OFF \
	-DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_QSORT_OFF -DRAG_SINCOS -DRAG_PURE_FLOAT \
	-DDEBUG                           \
	-DDEBUG_LVL_OFF -DTRACE_LVL_2_OFF   \
	-DDEBUG_LVL_3_OFF -DDEBUG_LVL_4_OFF   \
	-DTRACE                                   \
	-DTRACE_LVL_1     -DTRACE_LVL_2       \
	-DTRACE_LVL_3 -DTRACE_LVL_4   \
	-DRAG_PETER_DIST_AND_TRIG -DRAG_SPAD -DRAG_DRAM \
	-DGANESH_STRENGTH_RED_OPT \
	-DRAG_KEEP_CURRENT_OFF -DDEBUG_RMD_OFF

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_LIBDIR) -L$(RMD_RT_INSTALL) -T $(SSCP_NEWLIB)  -static -Map=out.map
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

%.o: %.s
	$(AS) -o $@ $<

%.s: %.c
	$(CC) $(CFLAGS) -fno-builtin -S -o $@ $<

Data.s:	../../datagen/tiny/Data.bin
	../utils/dataImageAsm 1 <../../datagen/tiny/Data.bin >Data.s

PlatformPosition.s:	../../datagen/tiny/PlatformPosition.bin
	../utils/dataImageAsm 2 <../../datagen/tiny/PlatformPosition.bin >PlatformPosition.s

PulseTransmissionTime.s:	../../datagen/tiny/PulseTransmissionTime.bin
	../utils/dataImageAsm 3 <../../datagen/tiny/PulseTransmissionTime.bin >PulseTransmissionTime.s

$(TARGET): $(INCS) $(OBJS) ./sscp-with-newlib.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

run:	$(TARGET) $(TARGET).cfg
	/usr/bin/time $(FSIM) -s -c $(TARGET).cfg -c $(MACHINE_CFG)
	grep -h -e 'x=0x' logs/sscp.log.*CE.00 | grep -e 'y=0x' | grep -e 'p=0x' >detects.txt
	../utils/detects_ascii_to_txt <detects.txt | sort >Detects.txt
	-egrep -h '(>>> cur|>>> ref)' logs/sscp.log.*CE.00 | grep 0x >images_debug.txt
	-../utils/viewImageTxt 1 <Detects.txt >cur.bmp
	-../utils/viewImageTxt 2 <Detects.txt >ref.bmp
	-../utils/viewImageTxt 3 <Detects.txt >all.bmp
	-grep -h '>>> corr' logs/sscp.log.*CE.00 | grep 0x >corr_debug.txt
	-../utils/viewCorrTxt    >corr.bmp
	-diff Detects.txt Detects.gold
	-cmp  Detects.txt Detects.gold
	-wc   Detects.txt Detects.gold
####	grep -i CONSOLE sscp.log.CE.00

powerclean:
	@-$(POWER_ANALYSIS_SCRIPT) -r 2>/dev/null 	

power:	
	@$(POWER_ANALYSIS_SCRIPT) $(TARGET).cfg && echo -e "\nPower Analysis Graphs Generated:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_GRAPHS)
	@echo -e "\nPower Analysis Run Logs:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_RUNLOGS)	

powertech:	
	@$(POWER_ANALYSIS_SCRIPT_TECHSCALED) $(TARGET).cfg && echo -e "\nPower Analysis Graphs Generated:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_GRAPHS)
	@echo -e "\nPower Analysis Run Logs:\n--------------------------------------\n" && ls $(POWER_ANALYSIS_RUNLOGS)	

logclean:
	$(RM) -f logs/$(TARGET).log.*

clean:
	$(RM) -f $(TARGET) *.o out.map \
		detects.txt Detects.txt \
		images_debug.txt corr_debug.txt \
		cur.bmp ref.bmp all.bmp corr.bmp \
		./Data.s ./PlatformPosition.s ./PulseTransmissionTime.s
