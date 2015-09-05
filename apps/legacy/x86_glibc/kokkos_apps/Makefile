KOKKOS_INSTALL = ../../../libs/src/trilinos/install

include $(KOKKOS_INSTALL)/include/Makefile.export.Kokkos

CXX = $(Kokkos_CXX_COMPILER)
LINK = $(CXX)

KOKKOS_DEVICES = "Serial"
KOKKOS_ARCH = "SNB"

KOKKOS_LIBS = $(Kokkos_LIBRARY_DIRS) $(Kokkos_LIBRARIES)
KOKKOS_CXXFLAGS = $(Kokkos_CXX_FLAGS) $(Kokkos_INCLUDE_DIRS)

CXXFLAGS += $(KOKKOS_CXXFLAGS) -I.
CPPFLAGS =
LINKFLAGS = -static
DEPFLAGS = -M
#
# OCR libs
#
ARCH=x86_native
APPS_ROOT  ?= $(realpath $(CURDIR)/../../../)
APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_SHIM_LIB = $(LIBS)/libocr-glibcxx.a
OCR_LIB = $(LIBS)/libocrscaffold-glibc.a
#OCR_LIB = cxxmain.o libocrsal.a

OBJ = hello_world.o g2l.o
EXE = hello_world g2l

default: hello
	echo "Start Build"

all:
	make build SRC=hello_world.cpp EXE=hello_world OBJ=hello_world.o
	make build SRC=g2l.cpp EXE=g2l OBJ=g2l.o


############## Common ##############
build:
	@echo SRC $(SRC)
	@echo OBJ $(OBJ)
	@echo EXE $(EXE)
	make $(EXE)

$(EXE): $(OBJ) $(KOKKOS_LINK_DEPENDS)
	@echo ' - Linking $@'
	@$(LINK) $(LINKFLAGS) $(OBJ) $(KOKKOS_LIBS) $(OCR_SHIM_LIB) $(OCR_LIB) -o $@

clean:
	rm -f $(OBJ) $(EXE)

# Compilation rules

%.o:%.cpp $(KOKKOS_CPP_DEPENDS)
	@echo ' - $@'
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(EXTRA_INC) -c $<

.PHONY: build clean hello
