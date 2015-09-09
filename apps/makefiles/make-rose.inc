# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# This file contains common defintions for using the ROSE tool globals2db.

ifndef ROSE_ROOT
ROSE_ROOT=/opt/edg4x-rose
LD_LIBRARY_PATH += /opt/edg4x-rose/installTree/lib:/opt/boost_1_46_1/installTree/lib:/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server
endif

ROSE_TOOL_ROOT = $(APPS_ROOT)/../hll/rose/tools/globals2db
ROSE_TOOL = $(ROSE_TOOL_ROOT)/bin/globals2db
ROSE_OPT = -rose:binary

CFLAGS += -D__leaf__=__externally_visible__
CC_INC += -I. -I$(SRCDIR)
MPI_VERSION = $(APPS_ROOT)/libs/$(OCR_TYPE)
MPICC = $(CC)
MPI_FLAGS = -I$(MPI_VERSION)/include -L$(MPI_VERSION)/lib/release -L$(MPI_VERSION)/lib -lmpifort -lmpi -lmpigi -ldl -lrt -lpthread -lgcc
ZGRVIEW = java -jar /opt/ZGRViewer-0.8.2/zgrviewer/target/zgrviewer-0.8.2.jar

HERE=$(CURDIR)
