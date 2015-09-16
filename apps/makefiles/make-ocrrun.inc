# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# Get absolute paths. This is useful since we go in
# subfolders to do make and that screws up the relative
# paths
ifeq ($(RUN_JENKINS), runApp)
OCR_BUILD_ROOT        :=
OCR_BUILD             :=
WORKLOAD_BUILD_ROOT   :=
WORKLOAD_BUILD        :=
APPS_ROOT             :=
WORKLOAD_SRC          :=
else ifeq ($(RUN_JENKINS), buildApp)
OCR_ROOT              := $(shell cd "${OCR_ROOT}" && pwd)
OCR_BUILD_ROOT        :=
OCR_BUILD             :=
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_BUILD        := ${WORKLOAD_BUILD_ROOT}/${OCR_TYPE}
APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)
WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
else
OCR_ROOT               := $(shell cd "${OCR_ROOT}" && pwd)
OCR_BUILD_ROOT        := $(shell cd "${OCR_BUILD_ROOT}" && pwd)
OCR_BUILD             := ${OCR_BUILD_ROOT}/${OCR_TARGET}
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_BUILD        := ${WORKLOAD_BUILD_ROOT}/${OCR_TYPE}
APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)
WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
endif

# Tool to use as a run wrapper
RUN_TOOL              ?=

APPS_LIBS_ROOT        := $(shell cd "${APPS_LIBS_ROOT}" && pwd)

HOSTNAME              := $(shell hostname)

OCR_INSTALL_ROOT      := $(shell mkdir -p "${OCR_INSTALL_ROOT}" && cd "${OCR_INSTALL_ROOT}" && pwd)
OCR_INSTALL      := ${OCR_INSTALL_ROOT}/${OCR_TARGET}

WORKLOAD_INSTALL_ROOT := $(shell mkdir -p "${WORKLOAD_INSTALL_ROOT}" && cd "${WORKLOAD_INSTALL_ROOT}" && pwd)
WORKLOAD_INSTALL := ${WORKLOAD_INSTALL_ROOT}/${OCR_TYPE}

# Config file specific settings
CONFIG_GENERATE       := ${OCR_ROOT}/scripts/Configs/config-generator.py
GEN_CONFIG_FILE       := ${WORKLOAD_INSTALL}/generated.cfg
CONFIG_NUM_THREADS    ?= 8
ifeq ($(strip $(OCR_CONFIG)),)
OCR_CONFIG := $(GEN_CONFIG_FILE)
endif
#CONFIG_FLAGS - may be set in user portion of Makefile

REQ_LIBS ?=

ifeq ($(CC), cc)
# We want to use gcc as the default
# but allow for the user to set something else if needed
CC = gcc
endif

ifeq ($(CFLAGS_WARN_ERROR), no)
CFLAGS_BASE_WARN_ERROR =
else
CFLAGS_BASE_WARN_ERROR = -Werror
endif

CFLAGS_BASE = ${CFLAGS_BASE_WARN_ERROR} -I. -I${OCR_INSTALL}/include -I $(APPS_LIBS_ROOT)/include

LDFLAGS_BASE = -L ${OCR_INSTALL}/lib -L $(APPS_LIBS_ROOT)/lib -locr

CFLAGS  := $(CFLAGS_BASE) $(CFLAGS)
LDFLAGS := $(LDFLAGS_BASE) $(LDFLAGS)

STRIP   = strip

CUT = cut
GREP    = grep
RM  = rm
CP      = cp
MKDIR   = mkdir

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(addprefix $(WORKLOAD_BUILD)/,$(OBJS))

# Pick between libocr.so and libocr.a
# Gasnet uses libocr.a
LIBOCREXT = so
ifeq ($(OCR_TYPE), x86-gasnet)
LIBOCREXT = a
endif

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

### Install rules ###

.PHONY: install_internal
.ONESHELL:
install_internal: preinstall | $(WORKLOAD_INSTALL)
	@echo "Installing from $(WORKLOAD_BUILD) into $(WORKLOAD_INSTALL)..."
	@echo ""
	@echo "####"
	@echo "To run, use 'OCR_TYPE=${OCR_TYPE} make run <args>' or, manually (in bash):"
	@echo "export OCR_INSTALL=${OCR_INSTALL} && export LD_LIBRARY_PATH=${OCR_INSTALL}/lib "\
	"&& export OCR_CONFIG=${OCR_INSTALL}/config/default.cfg (default one) and run "\
	"the executable $(TARGET) in $(WORKLOAD_INSTALL)"
	@echo "####"
	@$(RM) -f $(WORKLOAD_INSTALL)/$(TARGET)
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_INSTALL)/
ifeq ($(RUN_JENKINS), buildApp)
ifeq ($(OCR_CONFIG), $(GEN_CONFIG_FILE))
	$(CONFIG_GENERATE) --remove-destination --threads $(CONFIG_NUM_THREADS) --output $(GEN_CONFIG_FILE) --guid COUNTED_MAP --target $(COMM_LAYER) $(CONFIG_FLAGS)
endif # OCR_CONFIG
endif # RUN_JENKINS

postinstall: install_internal

.PHONY: install
install: postinstall

### Run rules ###

ifeq (yes, $(DEBUGMAKE))
.PHONY: debug_pre_prerun
debug_pre_prerun:
	@echo "Initial arguments are '${WORKLOAD_ARGS}' and inputs are '${WORKLOAD_INPUTS}'"

prerun: debug_pre_prerun

debug_post_prerun: prerun
	@echo "Post prerun arguments are '${WORKLOAD_ARGS}' and inputs are '${WORKLOAD_INPUTS}'"
else
debug_post_prerun: prerun

endif

.PHONY: run_internal
.ONESHELL:
run_internal: WORKLOAD_EXEC ?= $(WORKLOAD_INSTALL)
ifeq ($(RUN_JENKINS), runApp)
run_internal: debug_post_prerun
else
debug_post_prerun: install
run_internal: install debug_post_prerun
ifeq ($(OCR_CONFIG), $(GEN_CONFIG_FILE))
	$(CONFIG_GENERATE) --remove-destination --threads $(CONFIG_NUM_THREADS) --output $(GEN_CONFIG_FILE) --guid COUNTED_MAP --target $(COMM_LAYER) $(CONFIG_FLAGS)
endif # OCR_CONFIG
endif # RUN_JENKINS
ifeq ($(HOSTNAME), bar1)
	$(error Please refrain from running jobs on bar1!)
endif
	LD_LIBRARY_PATH=$(OCR_INSTALL)/lib:$(APPS_LIBS_ROOT) OCR_CONFIG=$(OCR_CONFIG) \
	bash -c 'cd $(WORKLOAD_EXEC) && $(RUN_TOOL) $(OCR_INSTALL)/bin/ocrrun ./$(TARGET) -ocr:cfg $(OCR_CONFIG) $(WORKLOAD_ARGS) $(WORKLOAD_INPUTS)'

postrun: run_internal

.PHONY: run
run: postrun

### Build rules ###

# Enforce order for build_internal
$(WORKLOAD_BUILD)/$(TARGET): prebuild
prebuild: reqlibs

# The next three rules force postbuild to run after build_internal
# and default to run postbuild
.PHONY: build_internal
build_internal: reqlibs prebuild $(WORKLOAD_BUILD)/$(TARGET)

postbuild: build_internal

.PHONY: default
default: postbuild

# Builds any libraries required by the workload
.PHONY: parallel-reqlibs
parallel-reqlibs: $(FULL_REQ_LIBS)

.PHONY: $(FULL_REQ_LIBS)
$(FULL_REQ_LIBS):
	lockfile "/tmp/$(subst /,_,$@)_lock"
	echo "Grabbed lock"; date; \
	ARCH=x86 APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
	$(MAKE) -C $@ install; \
	echo "Releasing lock"; date
	rm -f "/tmp/$(subst /,_,$@)_lock"

.PHONY: reqlibs
reqlibs: parallel-reqlibs

# Require libocr as this means all include files are up-to-date
$(WORKLOAD_BUILD)/%.o: $(WORKLOAD_SRC)/%.c | libocr $(WORKLOAD_BUILD)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WORKLOAD_BUILD)/%.o: $(WORKLOAD_SRC)/%.cpp | libocr $(WORKLOAD_BUILD)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WORKLOAD_BUILD)/$(TARGET).dbg: $(OBJS) | libocr $(WORKLOAD_BUILD)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(WORKLOAD_BUILD)/$(TARGET): $(WORKLOAD_BUILD)/$(TARGET).dbg | $(WORKLOAD_BUILD)
	@$(RM) -f $(WORKLOAD_BUILD)/$(TARGET)
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET).dbg $(WORKLOAD_BUILD)/$(TARGET)

# libocr
.PHONY: libocr
ifeq ($(RUN_JENKINS), runApp)
# When running with Jenkins, we don't check anything and
# assume that it is all built and up to date
libocr: ;
else ifeq ($(RUN_JENKINS), buildApp)
# If we are running in Jenkins and building the app, we just need to make sure the library is built
# This will error out if it is not
libocr: | $(OCR_INSTALL)/lib/libocr.$(LIBOCREXT)
	@echo "Checked that libocr.$(LIBOCREXT) exists."
$(OCR_INSTALL)/lib/libocr.$(LIBOCREXT):
	$(error Cannot find libocr.$(LIBOCREXT) in $(OCR_INSTALL))
else
# In all other cases, we build the library
libocr: $(OCR_BUILD)/Makefile
	@echo "Building libocr.$(LIBOCREXT)..."
	OCR_TYPE=$(OCR_TYPE) OCR_INSTALL=$(OCR_INSTALL) OCR_ROOT=$(OCR_ROOT) $(MAKE) -C $(OCR_BUILD) $(TARGETS) install
$(OCR_INSTALL)/lib/libocr.$(LIBOCREXT): | libocr
endif

# If the visualizer is being used, add necessary CFLAGS to application's makefile
ifeq ($(ENABLE_VISUALIZER), yes)
CFLAGS += -DOCR_ENABLE_EDT_NAMING -DOCR_ENABLE_VISUALIZER
endif

.PHONY: clean
clean:
	-$(RM) -rf $(WORKLOAD_BUILD)

.PHONY: uninstall
uninstall:
	-$(RM) -rf $(WORKLOAD_INSTALL)

$(WORKLOAD_BUILD):
	mkdir -p $(WORKLOAD_BUILD)

$(WORKLOAD_INSTALL):
	mkdir -p $(WORKLOAD_INSTALL)

# Default rules. If the user defines them later, those recipes will be used
# Otherwise, the recipe does nothing by default

# The rules obviously run before/after the build, install and run phases
.PHONY: prebuild postbuild preinstall postinstall prerun postrun
prebuild:
postbuild:
preinstall:
postintall:
prerun:
postrun:
