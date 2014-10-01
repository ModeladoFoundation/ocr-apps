A_PREFIX=Intel_Base
A_SUFFIX=.a
A_TARGET=$(A_PREFIX)$(A_SUFFIX)
A_TARGET2=lib$(A_PREFIX).so

A_PATH=../$(A_PREFIX)

CC=icc
A_FLAGS=-O3 -ipo -openmp -std=c++11 -use-intel-optimized-headers -fPIC

ifeq ($(HOST),sam)	
A_INCS=-I. -I/opt/intel/mkl/include
A_LIBS=-lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -lboost_system -lboost_filesystem -lboost_regex
else
A_INCS=-I. -I/opt/intel/mkl/include -I$(ANALYTICS_ROOT)/include
A_LIBS=-L$(ANALYTICS_LIB) -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -lboost_system -lboost_filesystem -lboost_regex -lrt
endif

A_FILES := $(wildcard *.cpp)
A_OBJS := $(sort $(patsubst %.cpp, %.o, $(A_FILES)))


all: $(A_TARGET) $(A_TARGET2)		# Build all targets

vars:					# Show Makefile variable values
	@echo "       CC "$(CC)
	@echo " A_TARGET "$(A_TARGET)
	@echo "  A_FLAGS "$(A_FLAGS)
	@echo "   A_INCS "$(A_INCS)
	@echo "   A_LIBS "$(A_LIBS)
#	@echo "  A_FILES "$(A_FILES)
	@echo "   A_OBJS "$(A_OBJS)
#	@echo "LD_LIBRARY_PATH "$(LD_LIBRARY_PATH)

help:					# Show Makefile targets
	@egrep "[a-z]\w+:" Makefile | sort

clean:					# Remove make build object and target files
	-rm -f *{.so,.o,$(A_SUFFIX),~}

.PHONY:	clean vars help install rebuild

rebuild: clean $(A_TARGET)		# Clean and build target

install: $(A_TARGET)			# Build and install
	cp -p $(A_TARGET) $(A_TARGET2) $(ANALYTICS_LIB)
	chmod g+w $(ANALYTICS_LIB)/{$(A_TARGET),$(A_TARGET2)}

$(A_TARGET): $(A_OBJS)			# Build target and its required object files
	@rm -f $@
	xiar rcs $@ $(A_OBJS)

%.o: %.cpp
	$(CC) $(A_FLAGS) $(A_INCS) -c $< -o $@

%: %.cpp
	$(CC) $(A_FLAGS) $(A_INCS) -o $@ $<

%.so: $(A_TARGET)
	$(CC) -shared $(A_FLAGS) $(A_INCS) $< -o $@ $(A_LIBS)

