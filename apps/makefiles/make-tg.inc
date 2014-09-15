# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


# Get absolute paths. This is useful since we go in
# subfolders to do make and that screws up the relative
# paths
APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)
APPS_LIBS_ROOT        := $(shell cd "${APPS_LIBS_ROOT}" && pwd)
TG_INSTALL            := $(shell cd "${TG_INSTALL}" && pwd)
TG_SRC                := $(shell cd "${TG_SRC}" && pwd)
WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_INSTALL_ROOT := $(shell mkdir -p "${WORKLOAD_INSTALL_ROOT}" && cd "${WORKLOAD_INSTALL_ROOT}" && pwd)
OCR_SRC               := $(shell cd "${OCR_SRC}" && pwd)
OCR_INSTALL_ROOT      := $(shell mkdir -p ${OCR_INSTALL_ROOT} && cd "${OCR_INSTALL_ROOT}" && pwd)
OCR_BUILD_ROOT        := $(shell cd "${OCR_BUILD_ROOT}" && pwd)

OCR_INSTALL_XE   := ${OCR_INSTALL_ROOT}/${XE_TARGET}
OCR_BUILD_XE     := ${OCR_BUILD_ROOT}/${XE_TARGET}
OCR_INSTALL_CE   := ${OCR_INSTALL_ROOT}/${CE_TARGET}
OCR_BUILD_CE     := ${OCR_BUILD_ROOT}/${CE_TARGET}

WORKLOAD_INSTALL := ${WORKLOAD_INSTALL_ROOT}/${OCR_TYPE}
WORKLOAD_BUILD   := ${WORKLOAD_BUILD_ROOT}/${OCR_TYPE}

REQ_LIBS         ?=

PREFIX           := ${TG_INSTALL}/bin/rmd-linux-elf

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

CC	= $(PREFIX)-clang
CFLAGS_BASE	= -I $(OCR_INSTALL_XE)/include -DTG_ARCH -I $(APPS_LIBS_ROOT)/include -I $(TG_INSTALL)/include

OBJCOPY	= $(PREFIX)-objcopy
STRIP	= $(PREFIX)-strip

LD	= $(PREFIX)-ld
LDFLAGS_BASE	= -L $(APPS_LIBS_ROOT)/lib -L $(TG_INSTALL)/ld-scripts -T fsim-xe-icache.ld -static -Map=$(WORKLOAD_BUILD)/out.map

CUT	= cut
GREP	= grep
RM	= rm
CP      = cp
MKDIR   = mkdir


CFLAGS  := $(CFLAGS_BASE) $(CFLAGS)
LDFLAGS := $(LDFLAGS_BASE) $(LDFLAGS)

OBJS := $(addprefix $(WORKLOAD_BUILD)/,$(notdir $(SRCS:.c=.o)))

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

ifeq ($(RUN_TORQUE), yes)
RUN_CMD := $(TG_INSTALL)/bin/fsim-scripts/fsim-wrapper.sh
MACHINE_CONFIG :=
else
RUN_CMD := $(TG_INSTALL)/bin/fsim
MACHINE_CONFIG ?= $(TG_INSTALL)/fsim-configs/localhost.cfg
MACHINE_CONFIG := -c $(MACHINE_CONFIG)
endif

ifeq ($(RUN_JENKINS), yes)
RUN_CMD := $(TG_INSTALL)/bin/fsim-scripts/fsim-torque.sh
MACHINE_CONFIG :=
endif

.PHONY: install
.ONESHELL:
install: default | ${WORKLOAD_INSTALL}
	@echo "Installing from $(WORKLOAD_BUILD) into $(WORKLOAD_INSTALL)..."
	@echo ""
	@echo "####"
	@echo "To run: ${RUN_CMD} -s ${MACHINE_CONFIG} -c ${WORKLOAD_INSTALL}/config.cfg"
	@echo "####"
	@$(RM) -f $(WORKLOAD_INSTALL)/$(TARGET)* $(WORKLOAD_INSTALL)/config.cfg $(WORKLOAD_INSTALL)/rmdkrnl
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_INSTALL)/
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET).blob $(WORKLOAD_INSTALL)/
	@$(CP) $(WORKLOAD_SRC)/config.cfg $(WORKLOAD_INSTALL)/
	@$(CP) $(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl $(WORKLOAD_INSTALL)/

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
run_internal: install
	@$(MKDIR) -p $(WORKLOAD_EXEC)/logs
	@$(RM) -rf $(WORKLOAD_EXEC)/logs/*
	cd $(WORKLOAD_EXEC)
	WORKLOAD_EXEC=$(WORKLOAD_EXEC) WORKLOAD_INSTALL=$(WORKLOAD_INSTALL) bash -c '$(RUN_CMD) -s $(MACHINE_CONFIG) -c $(WORKLOAD_INSTALL)/config.cfg'
	$(eval WORKLOAD_OUTS := $(shell find -L $(WORKLOAD_EXEC)/logs/* -name '*.app_output' -print))

postrun: run_internal

.PHONY: run
run: postrun

.PHONY: default
default: reqlibs $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_BUILD)/$(TARGET).blob $(WORKLOAD_BUILD)/rmdkrnl

# Builds any libraries required by the workload
.PHONY: parallel-reqlibs
parallel-reqlibs: $(FULL_REQ_LIBS)

.PHONY: $(FULL_REQ_LIBS)
$(FULL_REQ_LIBS):
	ARCH=tg APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
	$(MAKE) -C $@ install

.PHONY: reqlibs
reqlibs: parallel-reqlibs

# Require libocr.a as this means all include files are up-to-date
$(WORKLOAD_BUILD)/%.o: $(WORKLOAD_SRC)/%.c $(OCR_INSTALL_XE)/lib/libocr.a | $(WORKLOAD_BUILD)
	$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<
#	$(CC) $(CFLAGS) -fno-builtin -c -o $@.tmp $<
#	$(OBJCOPY) -R .data_local -R .rodata_local -R .bss_local -R .data_block -R .rodata_block -R .bss_block $@.tmp $@
#	$(RM) $@.tmp

$(WORKLOAD_BUILD)/$(TARGET).dbg: $(OBJS) $(OCR_INSTALL_XE)/lib/libocr.a | $(WORKLOAD_BUILD)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) --whole-archive $(OCR_INSTALL_XE)/lib/libocr.a

$(WORKLOAD_BUILD)/$(TARGET): $(WORKLOAD_BUILD)/$(TARGET).dbg | $(WORKLOAD_BUILD)
	$(CP) $(WORKLOAD_BUILD)/$(TARGET).dbg $(WORKLOAD_BUILD)/$(TARGET)
	$(STRIP) $(WORKLOAD_BUILD)/$(TARGET)

# Build the final blob
# Note the order of the dependence. IMPORTANT!! Do not randomly change
$(WORKLOAD_BUILD)/$(TARGET).blob: $(OCR_SRC)/scripts/Blob/aggregate_binary_files.sh \
$(WORKLOAD_BUILD)/$(TARGET).blob.ce $(WORKLOAD_BUILD)/$(TARGET).blob.xe \
$(WORKLOAD_BUILD)/$(TARGET).blob.arg | $(WORKLOAD_BUILD)
	$^ $(WORKLOAD_INPUTS) $@

# Build the CE blob
$(WORKLOAD_BUILD)/$(TARGET).blob.ce: $(OCR_INSTALL_CE)/config/default.cfg \
$(WORKLOAD_BUILD)/rmdkrnl.dbg $(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe | $(WORKLOAD_BUILD)
	TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.ce \
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe -ocr:cfg $(OCR_INSTALL_CE)/config/default.cfg

$(OCR_INSTALL_CE)/config/default.cfg: $(OCR_INSTALL_CE)/lib/libocr.a

# Build the XE blob
$(WORKLOAD_BUILD)/$(TARGET).blob.xe: debug_post_prerun $(OCR_INSTALL_XE)/config/default.cfg \
$(WORKLOAD_BUILD)/$(TARGET).dbg $(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe | $(WORKLOAD_BUILD)
	TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.xe \
	ARGS_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.arg APP_FILE=$(WORKLOAD_BUILD)/$(TARGET).dbg \
	$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe -ocr:cfg $(OCR_INSTALL_XE)/config/default.cfg $(WORKLOAD_ARGS)

$(OCR_INSTALL_XE)/config/default.cfg: $(OCR_INSTALL_XE)/lib/libocr.a

# The arg blob is build automatically by the XE builder
$(WORKLOAD_BUILD)/$(TARGET).blob.arg: $(WORKLOAD_BUILD)/$(TARGET).blob.xe | $(WORKLOAD_BUILD)

# Local version of the RMDKRNL (built with the CE's libocr)
# We build locally so we can run without impacting TG_INSTALL
.ONESHELL:
$(WORKLOAD_BUILD)/rmdkrnl.dbg: $(OCR_INSTALL_CE)/lib/libocr.a | $(WORKLOAD_BUILD)
	@echo "Building RMDKRNL from $(TG_SRC)..."
	-@$(RM) -rf $(WORKLOAD_BUILD)/build.rmdkrnl
	@$(MKDIR) -p $(WORKLOAD_BUILD)/build.rmdkrnl
	@$(CP) $(TG_SRC)/build/build.rmdkrnl/make-me.make $(WORKLOAD_BUILD)/build.rmdkrnl
	OCR_INSTALL=$(OCR_INSTALL_CE) TG_SRC=$(TG_SRC) \
	TG_INSTALL=$(TG_INSTALL) \
	$(MAKE) -C $(WORKLOAD_BUILD)/build.rmdkrnl -f make-me.make $(TARGETS)

$(WORKLOAD_BUILD)/rmdkrnl: $(WORKLOAD_BUILD)/rmdkrnl.dbg | $(WORKLOAD_BUILD)

# XE libocr.a
$(OCR_INSTALL_XE)/lib/libocr.a: $(OCR_BUILD_XE)/Makefile
	@echo "Building XE libocr.a..."
	OCR_TYPE=$(XE_TARGET) OCR_INSTALL=$(OCR_INSTALL_XE) OCR_SRC=$(OCR_SRC) \
	$(MAKE) -C $(OCR_BUILD_XE) $(TARGETS) install

# XE builder
$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe: $(OCR_BUILD_ROOT)/$(XE_BUILDER)/Makefile
	@echo "Building XE builder..."
	OCR_TYPE=$(XE_BUILDER) OCR_INSTALL=$(OCR_INSTALL_ROOT)/$(XE_BUILDER) \
	OCR_SRC=$(OCR_SRC) $(MAKE) -C $(OCR_BUILD_ROOT)/$(XE_BUILDER) $(TARGETS) install

# CE libocr.a
$(OCR_INSTALL_CE)/lib/libocr.a: $(OCR_BUILD_CE)/Makefile
	@echo "Building CE libocr.a..."
	OCR_TYPE=$(CE_TARGET) OCR_INSTALL=$(OCR_INSTALL_CE) OCR_SRC=$(OCR_SRC) \
	$(MAKE) -C $(OCR_BUILD_CE) $(TARGETS) install

# CE builder
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe: $(OCR_BUILD_ROOT)/$(CE_BUILDER)/Makefile
	@echo "Building CE builder..."
	OCR_TYPE=$(CE_BUILDER) OCR_INSTALL=$(OCR_INSTALL_ROOT)/$(CE_BUILDER) \
	OCR_SRC=$(OCR_SRC) $(MAKE) -C $(OCR_BUILD_ROOT)/$(CE_BUILDER) $(TARGETS) install

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