ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../../trunk/ss
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

CC	= $(PREFIX)-clang -DRAG_SIM
# -DDEBUG_SSCP can be used to output binary data for debugging purposes
# -D__float128=char is a fix for fftw-3.3 built with gcc 4.6 and quad precision
#  and building the app with hc.  
#  TODO: file bugs keeping fftw from being built with hc
CFLAGS	= -g -ggdb -g3 -O0 -std=c99 -DDEBUG_SSCP -DTRACE -DRAG_WORKAROUND -DDEBUG_OFF \
	-DRAG_PURE_FLOAT=1 -DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_DIG_SPOT_ON=0 -DRAG_AFFINE_ON=1 -DRAG_THIN_ON=0 -DRAG_QSORT_ON=0 \
	-I $(RMD_INSTALL_BASE)/xe_sim/include \
	-I $(RMD_INSTALL_BASE)/common \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I $(RMD_INSTALL_BASE)/runtime/libxe/inc \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I.

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

%.s: %.c
	$(CC) $(CFLAGS) -fno-builtin -S -o $@ $<

$(TARGET): $(INCS) $(OBJS) ./sscp-with-newlib.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

run:	$(TARGET) $(TARGET).cfg
	rm -rf *log*
	$(FSIM) -c $(TARGET).cfg
	grep -i CONSOLE *log.CE.00

power:	$(TARGET) $(TARGET).cfg
	$(POWER) -r $(TARGET).cfg

clean:
	$(RM) -rf $(TARGET) *.o out.map *.log.*
