# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# This file contains common definitions for using the ROSE tool globals2db.

ROSE_ROOT=/opt/edg4x-rose/installTree
BOOST_ROOT=/opt/boost_1_46_1/installTree
LD_LIBRARY_PATH += $(ROSE_ROOT)/lib:$(BOOST_ROOT)/lib:/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server

ROSE_TOOL_ROOT = $(APPS_ROOT)/../hll/rose/tools/globals2db
ROSE_TOOL = $(ROSE_TOOL_ROOT)/bin/globals2db
ROSE_OPT = -rose:skipfinalCompileStep

CFLAGS += -D__leaf__=__externally_visible__
CC_INC += -I. -I$(SRCDIR)
LDFLAGS += -L$(ROSE_ROOT)/lib -L$(BOOST_ROOT)/lib
MPI_VERSION = $(APPS_ROOT)/libs/$(OCR_TYPE)
MPICC = $(CC)
MPI_FLAGS = -I$(MPI_VERSION)/include -L$(MPI_VERSION)/lib/release -L$(MPI_VERSION)/lib -lmpifort -lmpi -lmpigi -ldl -lrt -lpthread -lgcc
ZGRVIEW = java -jar /opt/ZGRViewer-0.8.2/zgrviewer/target/zgrviewer-0.8.2.jar

HERE=$(CURDIR)
