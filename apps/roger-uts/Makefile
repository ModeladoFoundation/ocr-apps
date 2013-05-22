
ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../ss
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
POWER_ANALYSIS_SCRIPT_TECHSCALED  = $(RMD_INSTALL_BASE)/xe-sim/powerAnalysisScript_TechScaled.sh
POWER_ANALYSIS_GRAPHS  = $(RMD_INSTALL_BASE)/xe-sim/tools/powerResultDir/E*pdf
POWER_ANALYSIS_RUNLOGS = $(RMD_INSTALL_BASE)/xe-sim/tools/powerResultDir/UHPC.Power*


CC	= $(PREFIX)-clang -DRAG_SIM -DTRACE_OFF -DDEBUG_OFF -DRAG_BLOCK
## -DTRACE 
## -DDEBUG 
## -DRAG_BLOCK
CFLAGS	= -g -ggdb -g3 -O2 -std=c99 -frmd-extensions -frmd-bb-profiling -DBRG_C99_TYPES -DBRG_RNG -DRAG_DEPTH=4 \
	-I $(RMD_INSTALL_BASE)/xe-sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/common/include \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_LIBDIR) -L$(RMD_RT_INSTALL) -T ./rmd-newlib-big.ld -static -Map=out.map
LDLIBS	= -lxert --start-group -lc -lg -lm -lgloss_lxe --end-group


CUT	= cut
GREP	= grep
RM	= rm

OBJS 	= brg_sha1.o uts.o

TARGET = uts-main

all: $(TARGET)
debug: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<


$(TARGET): $(TARGET).o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TARGET).o $(OBJS) $(LDLIBS)

run:	$(TARGET)
	$(FSIM) -s -c $(TARGET).cfg -c $(RMD_INSTALL_BASE)/xe-sim/configs/localhost.cfg

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
	$(RM) -rf $(TARGET) *.s *.o logs/*
