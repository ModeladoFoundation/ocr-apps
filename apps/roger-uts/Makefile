
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

FSIM	= $(RMD_INSTALL_BASE)/xe_sim/f_sim

CC	= $(PREFIX)-clang -DRAG_SIM -DTRACE_OFF -DDEBUG_OFF -DRAG_BLOCK
## -DTRACE 
## -DDEBUG 
## -DRAG_BLOCK
CFLAGS	= -g -ggdb -g3 -O2 -std=c99 -frmd-extensions -DBRG_C99_TYPES -DBRG_RNG -DRAG_DEPTH=4 \
	-I $(RMD_INSTALL_BASE)/xe_sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/common/include \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common

LD	= $(PREFIX)-ld
LDFLAGS	= -L$(RMD_RT_LIBDIR) -L$(RMD_RT_INSTALL) -T ./uts_newlib_normal.ld -static -Map=out.map
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


$(TARGET): $(TARGET).o $(OBJS) ./uts-main-with-newlib.ld
	$(LD) $(LDFLAGS) -o $@ $(TARGET).o $(OBJS) $(LDLIBS)

run:	$(TARGET)
	$(FSIM) -s -c $(TARGET).cfg

clean:
	$(RM) -rf $(TARGET) *.s *.o logs/*
