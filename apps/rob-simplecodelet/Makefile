#################################
# Note - this makefile is crap, and will require tedious hacking if you
# dare move anything from where it is in the svn repo.  It may even include
# superfluous or nonexistant include directories.
#
# I understand that early on we needed to get things going and
# therefore hacked in whatever paths worked.  But the layout we have
# is ridiculous.  (Whew, I feel a little better now.)  --RobK


ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../..
endif
PREFIX=$(RMD_INSTALL_BASE)/bin/bin/rmd-linux-elf

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe

CUT	= cut
GREP	= grep
RM	= rm

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

DEFINITIONS = -DCODELETS -DRMD_DB_MEM -DPRINT_DEBUG_INFO -DFSIM


CC	= $(PREFIX)-clang


NOWARNINGFLAGS =  -Wno-unused-value
CFLAGS  = -I../../../runtime/libxe/inc -I../../../runtime/common -I$(RMD_INSTALL_BASE)/common/include -O2 -frmd-extensions $(DEFINITIONS) $(MOREINCS) -I../../../rmdkrnl/inc

LD	= $(PREFIX)-ld
LDFLAGS = -L$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib -L$(RMD_RT_INSTALL) -T rmd_newlib_big.ld -Map=out.map -static
LDLIBS = -lxert --start-group -lc -lg -lm -lgloss_lxe --end-group 

###########
#Pre-ASE flags
#LDFLAGS	=  -L$(RMD_INSTALL_BASE)/bin/rmd-linux-elf/lib -L$(RMD_RT_INSTALL) -T rob_hack_newlib_big.ld -static -Map=out.map 
#LDLIBS	= -lxert --start-group -lc -lg -lm -lgloss_lxe --end-group
###########

SRCS = stest.c

OBJS = ${SRCS:.c=.o}

TARGET = stest

all: ${SRCS} $(TARGET)

$(TARGET): $(OBJS)

%.o: %.c %.h 
	$(CC) $(CFLAGS) $(NOWARNINGFLAGS) -fno-builtin -c -o $@ $<
#	$(CC) $(CFLAGS) $*.c -S -o $*.s
#	$(CC) $(CFLAGS) -c $*.c -emit-llvm -o $*.bc
#	$(CC) $(CFLAGS) -c $*.c -o $*.o
#	$(OBJCOPY) -R .data_local -R .data_block -R .bss_local -R .bss_block $@

$(TARGET): $(TARGET).o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

debug: $(TARGET)

clean:
	rm -rf $(TARGET) make-edgelist *.s *.o *.bc out.map *~

