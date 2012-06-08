ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../../../ss
endif
ifndef AFL_INSTALL_BASE
	AFL_INSTALL_BASE = ../../../../../tools/rmd_afl
endif

RMD_RT_INSTALL=$(RMD_INSTALL_BASE)/runtime/libxe

CC=gcc

CFLAGS= -g -O3 -std=c99 -DRAG_AFL -Wall \
	-I $(AFL_INSTALL_BASE)/common \
	-I $(AFL_INSTALL_BASE)/rmd_memory \
	-I $(AFL_INSTALL_BASE)/rmd_threading \
	-I $(AFL_INSTALL_BASE) \
	-I $(AFL_INSTALL_BASE)/rmd_codelet \
	-I $(RMD_INSTALL_BASE)/runtime/common \
	-I $(RMD_INSTALL_BASE)/rmdkrnl/inc \
	-I. \
	-DDEBUG_SSCP_OFF -DRAG_DIG_SPOT_OFF -DRAG_AFFINE -DRAG_THIN_OFF \
	-DTRACE_OFF                           \
	-DTRACE_LVL_1_OFF -DTRACE_LVL_2_OFF   \
	-DTRACE_LVL_3_OFF -DTRACE_LVL_4_OFF   \
	-DDEBUG_OFF                           \
	-DDEBUG_LVL_1_OFF -DTRACE_LVL_2_OFF   \
	-DDEBUG_LVL_3_OFF -DDEBUG_LVL_4_OFF   \
	-DRAG_IMPLICIT_INPUTS -DRAG_IMPLICIT_INPUTS_TEST_OFF \
	-DRAG_QSORT_OFF -DRAG_SINCOS -DRAG_PURE_FLOAT \
	-DRAG_PETER_DIST_AND_TRIG     -DRAG_SPAD    -DRAG_DRAM \
	-DRAG_HIST_BIN_DIFFS_OFF

# -DDEBUG_SSCP can be used to output image and corr_map data for debugging purposes

LDFLAGS=$(AFL_INSTALL_BASE)/rmd_codelet/rmd_codelet.a \
	$(AFL_INSTALL_BASE)/rmd_afl.a \
	-lpthread -lxml2 -lc -ldl -lz -lm -lrt

INCS =	common.h rag_rmd.h

SRCS=	main.c \
	inputs.c \
	image_formation.c \
	digspot.c \
	back_proj.c \
	registration.c \
	ccd.c \
	cfar.c \
	sinc_interp.c \
	sinc.c \
	fftw3.c \
	rag_rmd.c \
	./Data.c \
	./PlatformPosition.c \
	./PulseTransmissionTime.c

all: sscp

Data.c:	../../datagen/small/Data.bin
	../utils/dataImage 1 <../../datagen/small/Data.bin >Data.c 
PlatformPosition.c: ../../datagen/small/PlatformPosition.bin
	../utils/dataImage 2 <../../datagen/small/PlatformPosition.bin >PlatformPosition.c
PulseTransmissionTime.c: ../../datagen/small/PulseTransmissionTime.bin
	../utils/dataImage 3 <../../datagen/small/PulseTransmissionTime.bin >PulseTransmissionTime.c

sscp:	$(INCS) $(SRCS)
	$(CC) $(CFLAGS) -o sscp $(SRCS) $(LDFLAGS)

run:	sscp
	/usr/bin/time ./sscp -c sscp.cfg >detects.txt
	egrep -e '(x=....\...m)' detects.txt | sort >Detects.txt
	-../utils/viewImage 1 <Detects.txt >cur.bmp
	-../utils/viewImage 2 <Detects.txt >ref.bmp
	-../utils/viewImage 3 <Detects.txt >all.bmp
	-../utils/viewCorr    >corr.bmp
	wc  detects.txt
	wc  Detects.txt Detects.gold
	cmp Detects.txt Detects.gold

#valgrind ./sscp -d -c sscp.cfg >detects.txt
#./sscp -c sscp.cfg \
#	../../datagen/small/Data.bin \
#	../../datagen/small/PlatformPosition.bin \
#	../../datagen/small/PulseTransmissionTime.bin \
#	Detects.txt

clean:
	$(RM) sscp detects.txt Detects.txt images_debug.bin corr_debug.bin \
		cur.bmp ref.bmp all.bmp corr.bmp \
		./Data.c ./PlatformPosition.c ./PulseTransmissionTime.c
