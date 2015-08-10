KOKKOS_INSTALL = ../install
KOKKOS_BUILD = ../build_kokkos/

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
ARCH=x86
APPS_ROOT  ?= $(realpath $(CURDIR)/../../../../)
APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_SHIM_LIB = $(LIBS)/libocr-glibcxx.a
OCR_LIB = $(LIBS)/libocrscaffold-glibc.a
#OCR_LIB = cxxmain.o libocrsal.a

export SRC ?=
export OBJ ?= $(SRC:.cpp=.o)
export EXE ?=

default: hello
	echo "Start Build"

############## hello_world app ##############

hello hello_clean : SRC = hello_world.cpp
hello hello_clean : EXE = hello_world
hello: build
hello_clean: clean

############## G2L app ##############

g2l g2l_clean : SRC = G2L_Main.cpp
g2l g2l_clean : EXE = G2L
g2l: build
g2l_clean: clean

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
