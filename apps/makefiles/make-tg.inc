# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


# Get absolute paths. This is useful since we go in
# subfolders to do make and that screws up the relative
# paths
ifeq ($(RUN_JENKINS), runApp)
TG_ROOT               :=
OCR_ROOT              :=
OCR_BUILD_ROOT        :=
OCR_BUILD_XE          :=
OCR_BUILD_CE          :=
WORKLOAD_BUILD_ROOT   :=
WORKLOAD_BUILD        :=
APPS_LIBS_ROOT        :=
WORKLOAD_SRC          :=
else ifeq ($(RUN_JENKINS), buildApp)
TG_ROOT               := $(shell cd "${TG_ROOT}" && pwd)
OCR_ROOT              := $(shell cd "${OCR_ROOT}" && pwd)
OCR_BUILD_ROOT        :=
OCR_BUILD_XE          :=
OCR_BUILD_CE          :=
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_BUILD        := ${WORKLOAD_BUILD_ROOT}/${OCR_TYPE}
APPS_LIBS_ROOT        := $(shell cd "${APPS_LIBS_ROOT}" && pwd)
WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
OCR_CE_CONFIG         := $(WORKLOAD_BUILD)/ce.cfg
OCR_XE_CONFIG         := $(WORKLOAD_BUILD)/xe.cfg
else
TG_ROOT               := $(shell cd "${TG_ROOT}" && pwd)
OCR_ROOT              := $(shell cd "${OCR_ROOT}" && pwd)
OCR_BUILD_ROOT        := $(shell cd "${OCR_BUILD_ROOT}" && pwd)
OCR_BUILD_XE          := ${OCR_BUILD_ROOT}/${XE_TARGET}
OCR_BUILD_CE          := ${OCR_BUILD_ROOT}/${CE_TARGET}
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_BUILD        := ${WORKLOAD_BUILD_ROOT}/${OCR_TYPE}
APPS_LIBS_ROOT        := $(shell cd "${APPS_LIBS_ROOT}" && pwd)
WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
OCR_CE_CONFIG         := $(WORKLOAD_BUILD)/ce.cfg
OCR_XE_CONFIG         := $(WORKLOAD_BUILD)/xe.cfg
endif

APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)

HOSTNAME              := $(shell hostname)

TG_INSTALL            := $(shell cd "${TG_INSTALL}" && pwd)

OCR_INSTALL_ROOT := $(shell mkdir -p ${OCR_INSTALL_ROOT} && cd "${OCR_INSTALL_ROOT}" && pwd)
OCR_INSTALL_XE   := ${OCR_INSTALL_ROOT}/${XE_TARGET}
OCR_INSTALL_CE   := ${OCR_INSTALL_ROOT}/${CE_TARGET}

FIX_CE_CONFIG    := ${OCR_ROOT}/scripts/Configs/ce_config_fix.py
FIX_MEM_CONFIG   := ${OCR_ROOT}/scripts/Configs/mem_config_fix.py

WORKLOAD_INSTALL_ROOT := $(shell mkdir -p "${WORKLOAD_INSTALL_ROOT}" && cd "${WORKLOAD_INSTALL_ROOT}" && pwd)
WORKLOAD_INSTALL      := ${WORKLOAD_INSTALL_ROOT}/${OCR_TYPE}

REQ_LIBS         ?=

PREFIX           := $(TG_INSTALL)/bin/rmd-linux-elf

AS	= $(PREFIX)-as
ASFLAGS	=

AR	= $(PREFIX)-ar
ARFLAGS	= rcs

CC	= $(PREFIX)-clang
CFLAGS_BASE	= -frmd-extensions -I $(OCR_INSTALL_XE)/include -DTG_ARCH -I $(APPS_LIBS_ROOT)/include -I $(TG_INSTALL)/include

OBJCOPY	= $(PREFIX)-objcopy
STRIP	= $(PREFIX)-strip
OBJDUMP = $(PREFIX)-objdump

LD	= $(PREFIX)-ld
LDFLAGS_BASE	= -L $(APPS_LIBS_ROOT)/lib -L $(TG_INSTALL)/ld-scripts -T fsim-xe-icache.ld -static -Map=$(WORKLOAD_BUILD)/out.map

CUT	= cut
GREP	= grep
SED     = sed
RM	= rm
CP      = cp
MKDIR   = mkdir


CFLAGS  := $(CFLAGS_BASE) $(CFLAGS)
LDFLAGS := $(LDFLAGS_BASE) $(LDFLAGS)

OBJS := $(patsubst %.c,$(WORKLOAD_BUILD)/%.o,$(SRCS))

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

ifeq ($(RUN_TORQUE), yes)
RUN_CMD := $(TG_INSTALL)/bin/fsim-scripts/fsim-wrapper.sh
MACHINE_CONFIG :=
else
RUN_CMD := $(TG_INSTALL)/bin/fsim
MACHINE_CONFIG ?= $(TG_INSTALL)/fsim-configs/localhost.cfg
MACHINE_CONFIG := -c $(MACHINE_CONFIG)
endif

ifeq ($(RUN_JENKINS), runApp)
RUN_CMD := $(TG_INSTALL)/bin/fsim-scripts/fsim-torque.sh
MACHINE_CONFIG :=
endif

### Install rules ###
.PHONY: install_internal
.ONESHELL:
ifeq ($(RUN_JENKINS), runApp)
install_internal: preinstall | ${WORKLOAD_INSTALL}
else
# Order of both should not matter here
install: preinstall $(WORKLOAD_SRC)/config.cfg $(WORKLOAD_INSTALL)/rmdkrnl | ${WORKLOAD_INSTALL}
endif
	@echo "Installing from $(WORKLOAD_BUILD) into $(WORKLOAD_INSTALL)..."
	@echo ""
	@echo "####"
	@echo "To run, use 'OCR_TYPE=${OCR_TYPE} make run <args>' or run manually:"
	@echo "(from $(WORKLOAD_INSTALL)): ${RUN_CMD} -s ${MACHINE_CONFIG} -c ${WORKLOAD_INSTALL}/config.cfg"
	@echo "####"
	@$(RM) -f $(WORKLOAD_INSTALL)/$(TARGET) $(WORKLOAD_INSTALL)/$(TARGET).blob
	@$(RM) -f $(WORKLOAD_INSTALL)/config.cfg
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_INSTALL)/
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET).blob $(WORKLOAD_INSTALL)/
	@$(CP) $(WORKLOAD_SRC)/config.cfg $(WORKLOAD_INSTALL)/

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
.SHELLFLAGS=-ec
run_internal: WORKLOAD_EXEC ?= $(WORKLOAD_INSTALL)
ifeq ($(RUN_JENKINS), runApp)
run_internal:
else
run_internal: install
endif
ifeq ($(HOSTNAME), bar1)
	$(error Please refrain from running jobs on bar1!)
endif
	@$(MKDIR) -p $(WORKLOAD_EXEC)/logs
	@$(RM) -rf $(WORKLOAD_EXEC)/logs/*
	@$(RM) -f $(WORKLOAD_EXEC)/console.out
	WORKLOAD_EXEC=$(WORKLOAD_EXEC) WORKLOAD_INSTALL=$(WORKLOAD_INSTALL) \
	bash -ec 'cd $(WORKLOAD_EXEC) && $(RUN_CMD) -s $(MACHINE_CONFIG) -c $(WORKLOAD_INSTALL)/config.cfg && \
	$(GREP) -H "CONSOLE: XE[0-7]" $(WORKLOAD_EXEC)/logs/*brd*.chp*.unt*.blk*.CE.00 \
	| $(SED) "s/.*\.blk\([0-9]\+\)\.CE\.00:CONSOLE:/BLK\1/" > $(WORKLOAD_EXEC)/console.out'

postrun: run_internal
postrun: WORKLOAD_OUTS := `find -L $(WORKLOAD_EXEC)/logs/* -name '*.app_output' -print`

.PHONY: run
run: postrun

### Build rules ###

# Enforce ordering
$(WORKLOAD_BUILD)/$(TARGET).blob: | prebuild
$(WORKLOAD_BUILD)/$(TARGET): | prebuild
prebuild: reqlibs

# The next three rules force postbuild to run after build_internal
# and default to run postbuild
.PHONY: build_internal
build_internal: reqlibs prebuild $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_BUILD)/$(TARGET).blob rmdkrnl

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
	ARCH=tg APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
	$(MAKE) -C $@ install; \
	echo "Releasing lock"; date
	rm -f "/tmp/$(subst /,_,$@)_lock"

.PHONY: reqlibs
reqlibs: parallel-reqlibs

# Require libocrxe as this means all include files are up-to-date
$(WORKLOAD_BUILD)/%.o: $(WORKLOAD_SRC)/%.c | libocrxe $(WORKLOAD_BUILD)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<
#	$(CC) $(CFLAGS) -fno-builtin -c -o $@.tmp $<
#	$(OBJCOPY) -R .data_local -R .rodata_local -R .bss_local -R .data_block -R .rodata_block -R .bss_block $@.tmp $@
#	$(RM) $@.tmp

# Two pre-requisites here that seem to be the same but serve different purposes:
#  - libocr.a: Execute this rule if libocr.a changes due to being rebuilt (due to the libocrxe rule)
#  - libocrxe: This rule will ALWAYS execute (it is a PHONY target) and it will
#              check if libocr.a needs to be rebuilt. If we listed this as a
#              regular pre-requisite, we would *always* rebuild the blob (PHONY rules are infectious)
# In other words, since we use the .a (and not a .so), we need to run this rule if the .a changes.
# Before that, thought, we need to check if we need to update the .a and that is the purpose
# of the libocrxe dependence
# Other rules for the blobs follow similar ideas
$(WORKLOAD_BUILD)/$(TARGET).dbg: $(OBJS) $(OCR_INSTALL_XE)/lib/libocr.a | libocrxe $(WORKLOAD_BUILD)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) --whole-archive $(OCR_INSTALL_XE)/lib/libocr.a

$(WORKLOAD_BUILD)/$(TARGET): $(WORKLOAD_BUILD)/$(TARGET).dbg | $(WORKLOAD_BUILD)
	@$(RM) -f $(WORKLOAD_BUILD)/$(TARGET)
	@$(CP) $(WORKLOAD_BUILD)/$(TARGET).dbg $(WORKLOAD_BUILD)/$(TARGET)
	@$(STRIP) $(WORKLOAD_BUILD)/$(TARGET)

# Build the final blob
# Note the order of the dependence. IMPORTANT!! Do not randomly change
$(WORKLOAD_BUILD)/$(TARGET).blob: $(OCR_ROOT)/scripts/Blob/aggregate_binary_files.sh \
$(WORKLOAD_BUILD)/$(TARGET).blob.ce $(WORKLOAD_BUILD)/$(TARGET).blob.xe \
$(WORKLOAD_BUILD)/$(TARGET).blob.arg | $(WORKLOAD_BUILD)
	echo "WORKLOAD_INPUTS is $(WORKLOAD_INPUTS) and args are ###$^###"
	$^ $(WORKLOAD_INPUTS) $@

# Build the CE blob
$(WORKLOAD_BUILD)/$(TARGET).blob.ce: $(OCR_CE_CONFIG) $(WORKLOAD_SRC)/config.cfg \
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe \
$(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl.dbg | rmdkrnl.dbg builderce $(WORKLOAD_BUILD)
	@echo "#### BUILDING CE BLOB ####"
	cp $(OCR_INSTALL_CE)/config/default.cfg $(OCR_CE_CONFIG)
	$(FIX_CE_CONFIG) --fsimcfg $(WORKLOAD_SRC)/config.cfg --ocrcfg $(OCR_CE_CONFIG)
	$(FIX_MEM_CONFIG) --ocrcfg $(OCR_CE_CONFIG) --binsize $(shell objdump -x $(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl.dbg | grep " _end" | cut -d\  -f1 | sed -e 's/^0*/0x/g')
	TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.ce \
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe -ocr:cfg $(OCR_CE_CONFIG)

$(OCR_CE_CONFIG): | rmdkrnl

# Build the XE blob
$(WORKLOAD_BUILD)/$(TARGET).dbg: | debug_post_prerun
$(WORKLOAD_BUILD)/$(TARGET).blob.xe: $(OCR_XE_CONFIG) \
$(WORKLOAD_BUILD)/$(TARGET).dbg $(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe | debug_post_prerun builderxe $(WORKLOAD_BUILD)
	@echo "#### BUILDING XE BLOB ####"
	cp $(OCR_INSTALL_XE)/config/default.cfg $(OCR_XE_CONFIG)
	$(FIX_MEM_CONFIG) --ocrcfg $(OCR_XE_CONFIG) --binsize $(shell $(OBJDUMP) -x $(WORKLOAD_BUILD)/$(TARGET).dbg | grep _end_local | cut -d\  -f1 | sed -e 's/^0*/0x/g')
	TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.xe \
	ARGS_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.arg APP_FILE=$(WORKLOAD_BUILD)/$(TARGET).dbg \
	$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe -ocr:cfg $(OCR_XE_CONFIG) $(WORKLOAD_ARGS)
# XE blob must be phony in order to rebuild the args blob on every run
.PHONY: $(WORKLOAD_BUILD)/$(TARGET).blob.xe

$(OCR_XE_CONFIG): | libocrxe

# The arg blob is build automatically by the XE builder
$(WORKLOAD_BUILD)/$(TARGET).blob.arg: $(WORKLOAD_BUILD)/$(TARGET).blob.xe | $(WORKLOAD_BUILD)

# XE libocr
.PHONY: libocrxe
ifeq ($(RUN_JENKINS), runApp)
libocrxe: ;
$(OCR_INSTALL_XE)/lib/libocr.a: ;
else ifeq ($(RUN_JENKINS), buildApp)
# If we are running in Jenkins, and building the app, we just need to make sure the library is built
# This will error out if it is not
libocrxe: | $(OCR_INSTALL_XE)/lib/libocr.a
	@echo "Checked that XE libocr.a exists."
$(OCR_INSTALL_XE)/lib/libocr.a:
	$(error Cannot find XE libocr.a in $(OCR_INSTALL_XE))
else
# In all other cases, we build the library
libocrxe: $(OCR_BUILD_XE)/Makefile
	@echo "Building XE libocr.a..."
	OCR_TYPE=$(XE_TARGET) OCR_INSTALL=$(OCR_INSTALL_XE) OCR_ROOT=$(OCR_ROOT) \
	$(MAKE) -C $(OCR_BUILD_XE) $(TARGETS) install
$(OCR_INSTALL_XE)/lib/libocr.a: | libocrxe
endif

# Note: builderxe, rmdkrnl, builderce all follow the same idea as libocrxe

# XE builder
.PHONY: builderxe
ifeq ($(RUN_JENKINS), runApp)
builderxe: ;
$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe: ;
else ifeq ($(RUN_JENKINS), buildApp)
builderxe: | $(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe
	@echo "Checked that XE builder exists."
$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe:
	$(error Cannot find XE builder in $(OCR_INSTALL_ROOT)/$(XE_BUILDER))
else
# In all other cases, we build the builder
builderxe: $(OCR_BUILD_ROOT)/$(XE_BUILDER)/Makefile
	@echo "Building XE builder..."
	OCR_TYPE=$(XE_BUILDER) OCR_INSTALL=$(OCR_INSTALL_ROOT)/$(XE_BUILDER) OCR_ROOT=$(OCR_ROOT) \
	$(MAKE) -C $(OCR_BUILD_ROOT)/$(XE_BUILDER) $(TARGETS) install
$(OCR_INSTALL_ROOT)/$(XE_BUILDER)/bin/builder.exe: | builderxe
endif

# Local version of the RMDKRNL (built with the CE's libocr)
# We build locally so we can run without impacting TG_INSTALL
# This will internally build libocrce.a if needed (ie: we don't build it
# explicitly here)
.PHONY: rmdkrnl

ifeq ($(RUN_JENKINS), runApp)
rmdkrnl: ;
$(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl: ;
$(WORKLOAD_INSTALL)/rmdkrnl: ;
else
# In all other cases, we build the kernel
# (including for building the app)
.ONESHELL:
rmdkrnl: $(TG_ROOT)/build/build.rmdkrnl/make-me.make \
| $(WORKLOAD_BUILD)/build.rmdkrnl
	@echo "Building rmdkrnl..."
	OCR_ROOT=$(OCR_ROOT) OCR_TARGET=$(CE_TARGET) OCR_INSTALL=$(OCR_INSTALL_CE) OCR_BUILD=$(OCR_BUILD_CE) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) OCRAPPS_RUN_JENKINS=$(RUN_JENKINS) \
	$(MAKE) -C $(WORKLOAD_BUILD)/build.rmdkrnl -f make-me.make $(TARGETS)

$(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl: | rmdkrnl

# This target will effectively always run which is what we want
# because for some reason, pre-reqs are evaluated before (and so when rmdkrnl recipe
# remakes build.rmdkrnl/rmdkrnl, nothing happens
$(WORKLOAD_INSTALL)/rmdkrnl: $(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl rmdkrnl $(WORKLOAD_INSTALL)
	@echo "Installing rmdkrnl..."
	@$(RM) -f $(WORKLOAD_INSTALL)/rmdkrnl
	@$(CP) $(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl $(WORKLOAD_INSTALL)/
endif

.PHONY: rmdkrnl.dbg
rmdkrnl.dbg: rmdkrnl

$(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl.dbg: $(WORKLOAD_BUILD)/build.rmdkrnl/rmdkrnl

# CE builder
.PHONY: builderce
ifeq ($(RUN_JENKINS), runApp)
builderce: ;
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe: ;
else ifeq ($(RUN_JENKINS), buildApp)
builderce: | $(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe
	@echo "Checked that CE builder exists."
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe:
	$(error Cannot find CE builder in $(OCR_INSTALL_ROOT)/$(CE_BUILDER))
else
# In all other cases, we build the builder
builderce: $(OCR_BUILD_ROOT)/$(CE_BUILDER)/Makefile
	@echo "Building CE builder..."
	OCR_TYPE=$(CE_BUILDER) OCR_INSTALL=$(OCR_INSTALL_ROOT)/$(CE_BUILDER) OCR_ROOT=$(OCR_ROOT) \
	$(MAKE) -C $(OCR_BUILD_ROOT)/$(CE_BUILDER) $(TARGETS) install
$(OCR_INSTALL_ROOT)/$(CE_BUILDER)/bin/builder.exe: | builderce
endif

.PHONY: clean
clean:
	-$(RM) -rf $(WORKLOAD_BUILD)

.PHONY: uninstall
uninstall:
	-$(RM) -rf $(WORKLOAD_INSTALL)

$(WORKLOAD_BUILD):
	mkdir -p $(WORKLOAD_BUILD)

$(WORKLOAD_BUILD)/build.rmdkrnl: $(TG_ROOT)/build/build.rmdkrnl/make-me.make | $(WORKLOAD_BUILD)
	mkdir -p $(WORKLOAD_BUILD)/build.rmdkrnl
	@$(RM) -f $(WORKLOAD_BUILD)/build.rmdkrnl/make-me.make
	@$(CP) $(TG_ROOT)/build/build.rmdkrnl/make-me.make $(WORKLOAD_BUILD)/build.rmdkrnl

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
