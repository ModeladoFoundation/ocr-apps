A_PREFIX=Intel_BenchMark
A_SUFFIX=
A_TARGET=$(A_PREFIX)$(A_SUFFIX)

B_PREFIX=Intel_Base
B_SUFFIX=.a
B_TARGET=$(B_PREFIX)$(B_SUFFIX)

A_PATH=../$(A_PREFIX)
B_PATH=../$(B_PREFIX)

CC=icc

A_FLAGS=-O3 -fPIC -ipo -openmp -std=c++11 -use-intel-optimized-headers

ifeq ($(HOST),sam)
A_INCS=-I. -I$(B_PATH) -I/opt/intel/mkl/include
A_LIBS=-L. -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -lboost_system -lboost_filesystem -lboost_regex -licudata -licui18n -licule -licuuc
else
A_INCS=-I. -I$(B_PATH) -I/opt/intel/mkl/include -I$(ANALYTICS_ROOT)/include
A_LIBS=-L. -L$(ANALYTICS_LIB) -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -lboost_system -lboost_filesystem -lboost_regex -licudata -licui18n -licule -licuuc -lrt
endif

A_FILES := $(wildcard *.cpp)

A_OBJS := $(sort $(filter-out stdafx.o, $(patsubst %.cpp, %.o, $(A_FILES))))

all: $(A_TARGET)			# Build all targets

vars:					# Show Makefile variable values
	@echo "       CC "$(CC)
	@echo "   A_PATH "$(A_PATH)
	@echo "   B_PATH "$(B_PATH)
	@echo " A_TARGET "$(A_TARGET)
	@echo "  A_FLAGS "$(A_FLAGS)
	@echo "   A_INCS "$(A_INCS)
	@echo "   A_LIBS "$(A_LIBS)
	@echo "   A_OBJS "$(A_OBJS)
#	@echo "LD_LIBRARY_PATH "$(LD_LIBRARY_PATH)

help:					# Show Makefile targets
	@egrep "[a-z]\w+:" Makefile | sort

clean:					# Remove make build object and target files
	@(cd $(B_PATH); make clean; cd $(A_PATH); echo " ")
	-rm -f *{.o,~} $(A_TARGET) $(B_PATH)/*{.o,$(B_SUFFIX),~}

.PHONY:	clean vars help install rebuild $(B_PATH)/$(B_TARGET)

rebuild: clean $(A_TARGET)		# Clean and build target

install: $(A_TARGET)			# Build and install
	cp -p $(B_PATH)/$(B_TARGET) $(ANALYTICS_LIB)
	cp -p $(A_TARGET) $(ANALYTICS_ROOT)/bin
	chmod g+w $(ANALYTICS_LIB)/$(B_TARGET) $(ANALYTICS_ROOT)/bin/$(A_TARGET)

$(A_TARGET): $(B_PATH)/$(B_TARGET) $(A_OBJS)
	$(CC) $(A_FLAGS) $(A_INCS) $^ -o $@ $(A_LIBS)

$(B_PATH)/$(B_TARGET):
	@(cd $(B_PATH); make; cd $(A_PATH); echo " ")

%.o: %.cpp
	$(CC) $(A_FLAGS) $(A_INCS) -c $< -o $@

%: %.cpp
	$(CC) $(A_FLAGS) $(A_INCS) -o $@ $<
