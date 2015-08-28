# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

MAKE := make

ifndef OCR_TYPE
  $(error Need to specify OCR_TYPE!)
endif
ifndef APPS_ROOT
  $(warning APPS_ROOT not set. Using default path. APPS_ROOT can also be set by sourcing app_env.bash from "apps/"!)

  MYAPPS_ROOT:=$(shell tmydir=`pwd`; tpath="." ; while [[ $$tmydir != "/" ]]; do if [[ -d "apps" ]]; then break; fi; cd ..; tmydir=`pwd`; tpath=$$tpath/..; done; echo $$tmydir)/apps


  APPS_ROOT       = $(MYAPPS_ROOT)
endif


TARGET = comd-mpi

SRCDIR = $(APPS_ROOT)/CoMD/refactored/mpilite/src-mpi
BLDDIR = build/$(OCR_TYPE)

SRC_NAMES = cmdLineParser.c decomposition.c  haloExchange.c  \
	linkCells.c mycommand.c performanceTimers.c  \
	timestep.c CoMD.c eam.c initAtoms.c  \
	ljForce.c parallel.c random.c yamlOutput.c

SRC := $(addprefix $(SRCDIR)/,$(SRC_NAMES))


ROSE_TOOL_ROOT = $(APPS_ROOT)/../hll/rose/tools/globals2db
ROSE_TOOL = $(ROSE_TOOL_ROOT)/bin/globals2db
ROSE_TARGET = rose_$(TARGET)
ROSE_OPT = -rose:binary

CFLAGS = -D__leaf__=__externally_visible__ -g -DDO_MPI -DDOUBLE
LDFLAGS = -lpthread -lm -lmpilite
CC_INC = -I. -I$(SRCDIR)
MPI_VERSION = $(APPS_ROOT)/libs/$(OCR_TYPE)
MPICC = $(CC)
MPI_FLAGS = -I$(MPI_VERSION)/include -L$(MPI_VERSION)/lib/release -L$(MPI_VERSION)/lib -lmpifort -lmpi -lmpigi -ldl -lrt -lpthread -lgcc
ZGRVIEW = java -jar /opt/ZGRViewer-0.8.2/zgrviewer/target/zgrviewer-0.8.2.jar

HERE=$(CURDIR)

all: run


$(ROSE_TOOL):
	cd $(ROSE_TOOL_ROOT)/src ; \
	make ; \
	cd $(HERE)


$(ROSE_TARGET): $(ROSE_TOOL)
	cd $(BLDDIR) ; \
	$(ROSE_TOOL) $(ROSE_OPT) $(CFLAGS) $(CC_INC) $(SRC) $(MPI_FLAGS) >& $(HERE)/build.log ; \
	cd $(HERE)

create_dir:
	mkdir -p $(BLDDIR)


CoMD_info:
	cd $(BLDDIR) ; \
	$(SRCDIR)/generate_info_header "CoMD_info.h"  "$(CC)" "$(CFLAGS)" "$(LDFLAGS)" ; \
	cd $(HERE)

rose: $(ROSE_TOOL) create_dir CoMD_info $(ROSE_TARGET)

run: $(ROSE_TOOL) create_dir CoMD_info $(ROSE_TARGET)
	make -f Makefile.$(OCR_TYPE) TARGET=$(TARGET) debug
	make -f Makefile.$(OCR_TYPE) TARGET=$(TARGET) install
	make -f Makefile.$(OCR_TYPE) TARGET=$(TARGET) run

clean:
	rm -f build.log
	make -f Makefile.$(OCR_TYPE) TARGET=$(TARGET) clean

