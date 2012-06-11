ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif

ifdef RUN_SUFFIX
	RUN_SUFFIX := $(RUN_SUFFIX); export RUN_SUFFIX
else
	
endif	
ifdef TECH_NODE 
	TECH_NODE := $(TECH_NODE); export TECH_NODE
else
	
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
POWER_ANALYSIS_SCRIPT_TECHSCALED  = $(RMD_INSTALL_BASE)/xe_sim/powerAnalysisScript_TechScaled.sh
POWER_ANALYSIS_GRAPHS  = $(RMD_INSTALL_BASE)/xe_sim/tools/powerResultDir/E*pdf
POWER_ANALYSIS_RUNLOGS = $(RMD_INSTALL_BASE)/xe_sim/tools/powerResultDir/UHPC.Power*

CC	= $(PREFIX)-clang

CFLAGS	= -g -ggdb -g3 -O1 -std=c99 -frmd-extensions -DRAG_SIM \
	-I $(RMD_INSTALL_BASE)/xe_sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/common/include \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I.  -DTRACE     -DCHECK_OFF -DDEBUG_OFF -DCACHE_RUN=$(CACHE_RUN) \
	-DRAG_ATOMIC=$(RAG_ATOMIC) -DRAG_CACHE=$(RAG_CACHE)

OBJCOPY	= $(PREFIX)-objcopy

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_LIBDIR) -L$(RMD_RT_INSTALL) -T ./gups-with-newlib.ld -static -Map=out.map
LDLIBS	= -lxert --start-group -lc -lg -lm -lgloss_lxe --end-group

CUT	= cut
GREP	= grep
RM	= rm

TARGET  = gups

INCS =	rag_rmd.h

OBJS=	gups.o \
	rag_rmd.o

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<

%.o: %.s
	$(AS) -o $@ $<

%.s: %.c
	$(CC) $(CFLAGS) -fno-builtin -S -o $@ $<

$(TARGET): $(INCS) $(OBJS) ./gups-with-newlib.ld Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

run:	$(TARGET) $(TARGET).cfg Makefile
	/usr/bin/time $(FSIM) -s -c $(TARGET).cfg

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
	$(RM) -rf /tmp/roger_f_sim.????

clean:
	$(RM) -f $(TARGET) rag_rmd.o gups.o gups.s out.map
