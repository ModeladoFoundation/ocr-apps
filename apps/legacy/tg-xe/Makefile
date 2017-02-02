#
# Makefile
# vim:syntax=make noexpandtab:
#
# If newlib was compiled for tgr instead of ocr set USE_TGR=yes.
# This is required for pthread and openmp examples. Not using this
# will default to using the scaffold with libswtest.
# Not setting USE_TGR will only generate swtest execuables.
#
ARCH=tg-xe

TG_INSTALL ?= $(realpath $(CURDIR)/../../../../tg/tg/install)
APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

export APPS_LIBS_INSTALL=$(APPS_ROOT)/libs/install/$(ARCH)

TARGET = xstg-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
CC = $(TOOL_PREFIX)clang
CXX = $(TOOL_PREFIX)clang++

CFLAGS += -g -O0
CXXFLAGS = $(CFLAGS) -fexceptions -Xlinker --eh-frame-hdr

PIE_FLAGS := -fPIE
SWTEST_FLAGS := -fxstg-swtest
TGRKRNL_FLAGS := -fxstg-tgr

LIBS = -lm -lbuiltins
PIE_LIBS = -lm.p -lbuiltins.p

SRCS = $(wildcard *.c) $(wildcard *.cpp)
APPS = $(patsubst %.c,%,$(patsubst %.cpp,%,$(SRCS)))
TARGET_APPS = $(APPS)

ifeq ($(USE_TGR), yes)
# Use this if newlib is compiled for tgr instead of ocr.
CFLAGS += -fopenmp
else
#Do not build apps which require TGR
TARGET_APPS = $(filter-out omp% pthread%,$(APPS))
PIE_FLAGS += -fxstg-swtest # Required to link in the scaffold
endif

all: $(TARGET_APPS)

default: all

%.fsim: %.c
ifeq ($(USE_TGR), yes)
	@echo " - $@"
	@$(CC) $(CFLAGS) $(TGRKRNL_FLAGS) $< $(LIBS) -o $@
else
	@# Not making this target because it depends on libtgr
	@# Remove it if it exists
	@rm -rf $@
endif

%.swtest: %.c
	@echo " - $@"
	@$(CC) $(CFLAGS) $(SWTEST_FLAGS) $< $(LIBS) -o $@

%.p: %.c
	@echo " - $@"
	@$(CC) $(CFLAGS) $(PIE_FLAGS) $< $(PIE_LIBS) -o $@

%.fsim: %.cpp
ifeq ($(USE_TGR), yes)
	@echo " - $@"
	@$(CXX) $(CXXFLAGS) $(TGRKRNL_FLAGS) $< $(LIBS) -o $@
else
	@# Not making this target because it depends on libtgr
	@# Remove it if it exists
	@rm -rf $@
endif

%.swtest: %.cpp
	@echo " - $@"
	@$(CXX) $(CXXFLAGS) $(SWTEST_FLAGS) $< $(LIBS) -o $@

%.p: %.cpp
	@echo " - $@"
	@$(CXX) $(CXXFLAGS) $(PIE_FLAGS) $< $(PIE_LIBS) -o $@

# The app names themselves are phony rules which make the
# three variants. The $(APPS): %: rule below make all APPS
# explicit targets so that they can be made PHONY
.PHONY: $(APPS)
$(APPS): %: %.fsim %.swtest %.p
	@echo " ** Done making $*"

# If the three variants were not SECONDARY, make would
# automatically remove them as temporary files when making
# one of the APPS.
.SECONDARY: $(APPS:=.fsim) $(APPS:=.swtest) $(APPS:=.p)
.SUFFIXES:

dump:
	@echo TG_INSTALL = $(TG_INSTALL)
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL = $(APPS_LIBS_INSTALL)
	@echo CC = $(CC)
	@echo CXX = $(CXX)

clean:
	@rm -f $(APPS) *.fsim *.swtest *.p

.PHONY: all default dump clean
