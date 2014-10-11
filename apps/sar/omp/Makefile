CRLIBM=../../../apps/libs/x86

all: sscp sscp.dbg clustering

SOURCES_COMMON = \
	src/common.h           \
	src/inputs.c        \
	src/sinc.c	\
	src/ccd.c       \
	src/digspot.c          \
	src/main.c          \
	src/sinc_interp.c\
	src/cfar.c      \
	src/image_formation.c  \
	src/registration.c \
	src/common.c \
	src/downsample.c
SOURCES_SERIAL = src/backproj.c

RM=rm -f

sscp: $(SOURCES_COMMON) $(SOURCES_SERIAL)
	$(CC) -g -o $@ -Wall -O3 -lfftw3f -fopenmp -std=c99 \
	-I$(CRLIBM)/include \
	-DSSCP_DEBUG_off -DRAG_TRACE_off -DRAG_NEW_BLK_SIZE=50 \
	-DRAG -DRAG_CRLIBM -DRAG_PURE_FLOAT -DRAG_PETER -DRAG_BLOCKED \
	$(SOURCES_COMMON) $(SOURCES_SERIAL) \
	-L$(CRLIBM)/lib -lcrlibm -lm

sscp.dbg: $(SOURCES_COMMON) $(SOURCES_SERIAL)
	$(CC) -g -o $@ -Wall -O3 -lfftw3f -fopenmp -std=c99 \
	-I$(CRLIBM)/include \
	-DSSCP_DEBUG -DRAG_TRACE -DRAG_NEW_BLK_SIZE=50 \
	-DRAG -DRAG_CRLIBM -DRAG_PURE_FLOAT -DRAG_PETER -DRAG_BLOCKED \
	$(SOURCES_COMMON) $(SOURCES_SERIAL) \
	-L$(CRLIBM)/lib -lcrlibm -lm

clustering: src/clustering.cpp
	$(CXX) -O3 -Wall -DRAG -o clustering src/clustering.cpp

run:	./sscp ./sscp.dbg
	$(MAKE) -C huge run
	$(MAKE) -C test run

clean:
	$(RM) sscp sscp.dbg clustering
