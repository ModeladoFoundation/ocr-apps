# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.
include ../Makefile-rose.def
include ../Makefile-mpi.def
TOP := $(dir $(lastword $(MAKEFILE_LIST)))

# Where the binaries will be installed
BINDIR = $(TOP)bin

# AST Generator
ASTGEN = astGenerate
# Set this variable to yes to generate the AST for
# the whole program
WHOLE_PROG_AST =

# attributeTraversals
SYNTHESIZED_ATTR_TEST = synthesizedAttrTest

# Binary for ocr translator tool
OCRTRANSLATE = ocrTranslate

# All binaries
TARGETS := $(OCRTRANSLATE)

ifndef ROSE_INS
  $(error Set ROSE environment in ../Makefile-rose.def!)
endif

ifndef MPI_INSTALL
  $(error Set MPI environment in ../Makefile-mpi.def!)
endif

.PHONY: all
all: $(TARGETS)

$(SYNTHESIZED_ATTR_TEST): $(SYNTHESIZED_ATTR_TEST).cpp
	$(CXX) -g $(ROSE_INC) $< -o $@ $(ROSE_LNK)
	cp $(SYNTHESIZED_ATTR_TEST) ../bin

$(ASTGEN): $(ASTGEN).cpp
ifeq ($(WHOLE_PROG_AST), yes)
	$(CXX) -g -O2 -DWHOLE_PROG $(ROSE_INC) $< -o $@ $(ROSE_LNK)
else
	$(CXX) -g -O2 $(ROSE_INC) $< -o $@ $(ROSE_LNK)
endif
	cp $(ASTGEN) ../bin

CC := g++
CFLAGS := $(ROSE_INC) -I$(MPI_INCLUDE_DIR) -g
OCRTRANSLATE_HEADERS := ocrObjectInfo.h ocrPragmaParsing.h ocrTranslateEngine.h ocrAstInfo.h ocrAstBuilder.h logger.h ocrSymbolTable.h
OCRTRANSLATE_SRCS := ocrObjectInfo.cpp \
	ocrSymbolTable.cpp \
	ocrPragmaParsing.cpp \
	ocrAstInfo.cpp \
	ocrTranslateEngine.cpp \
	ocrAstBuilder.cpp \
	ocrTranslate.cpp \
	logger.cpp

OCRTRANSLATE_OBJS := $(notdir $(OCRTRANSLATE_SRCS:.cpp=.o))

%.o: %.cpp $(OCRTRANSLATE_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OCRTRANSLATE): $(OCRTRANSLATE_OBJS)
	$(CC) -o $@ $^ $(ROSE_LNK)

.PHONY: install
install: $(TARGETS)
	mkdir -p $(BINDIR)
	cp -t $(BINDIR) $^

.PHONY: clean
clean:
	rm -f ../bin/$(OCRTRANSLATE)
	rm -f $(OCRTRANSLATE) ../tests/rose_*
	rm -f *.o
	rm -f *~
