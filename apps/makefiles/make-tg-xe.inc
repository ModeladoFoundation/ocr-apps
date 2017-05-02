# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ifneq (, $(filter-out tg-xe, $(ARCH)))
  $(error Wrong Makefile included. make-tg.inc only allows ARCH=tg-xe: got $(ARCH))
endif

# Get absolute paths. This is useful since we go in
# subfolders to do make and that screws up the relative
# paths



##########################################
#        User definable variables        #
##########################################
# Tool to use as a wrapper to run
RUN_TOOL              ?=

ENERGY_CONFIG :=
ifeq ($(ENERGY), yes)
ENERGY_CONFIG := -c $(TG_INSTALL)/fsim-configs/Energy.cfg
endif

# Configurations to use
OCR_CE_CONFIG ?=
OCR_XE_CONFIG ?=
TG_CONFIG ?=

# Use a job scheduler
# Can be 'no', 'torque', or 'slurm'
RUN_TORQUE  ?= no
# If RUN_TORQUE is no, you can specify a configuration file
# for the simulator

# GDB support
RUN_GDB         ?= no
RUN_GDB_HALT_XE ?= no
RUN_GDB_HALT_CE	?= no

# OCR targets (you will almost *never* need to change these)
XE_TARGET   ?= tg-xe
XE_BUILDER  ?= builder-xe
CE_TARGET   ?= tg-ce
CE_BUILDER  ?= builder-ce

##########################################
#    End of user definable varaibles     #
##########################################

HOSTNAME              := $(shell hostname)

ifeq ($(RUN_MODE), runApp)
  OCR_BUILD_XE          :=
  OCR_BUILD_CE          :=
else ifeq ($(RUN_MODE), buildApp)
  OCR_BUILD_XE          :=
  OCR_BUILD_CE          :=
  OCR_BUILD_XEBUILDER   :=
  OCR_BUILD_CEBUILDER   :=
  OCR_CE_CONFIG_DEFAULT := $(WORKLOAD_BUILD)/ce.cfg
  OCR_XE_CONFIG_DEFAULT := $(WORKLOAD_BUILD)/xe.cfg
  TG_CONFIG_DEFAULT     := $(WORKLOAD_BUILD)/config.cfg
else
  OCR_BUILD_XE          := ${OCR_BUILD_ROOT}/${XE_TARGET}
  OCR_BUILD_CE          := ${OCR_BUILD_ROOT}/${CE_TARGET}
  OCR_BUILD_XEBUILDER   := $(OCR_BUILD_ROOT)/$(XE_BUILDER)
  OCR_BUILD_CEBUILDER   := $(OCR_BUILD_ROOT)/$(CE_BUILDER)
  OCR_CE_CONFIG_DEFAULT := $(WORKLOAD_BUILD)/ce.cfg
  OCR_XE_CONFIG_DEFAULT := $(WORKLOAD_BUILD)/xe.cfg
  TG_CONFIG_DEFAULT     := $(WORKLOAD_BUILD)/config.cfg
endif

FIX_TG_FSIM_CONFIG := ${OCR_INSTALL}/share/ocr/scripts/Configs/tg-fsim_config_fix.py
FIX_CE_CONFIG    := ${OCR_INSTALL}/share/ocr/scripts/Configs/ce_config_fix.py
FIX_XE_CONFIG   := ${OCR_INSTALL}/share/ocr/scripts/Configs/xe_config_fix.py

ifeq (,$(strip $(OCR_CE_CONFIG)))
  OCR_CE_CONFIG := $(OCR_CE_CONFIG_DEFAULT)
endif
ifeq (,$(strip $(OCR_XE_CONFIG)))
  OCR_XE_CONFIG := $(OCR_XE_CONFIG_DEFAULT)
endif
ifeq (,$(strip $(TG_CONFIG)))
  TG_CONFIG := $(TG_CONFIG_DEFAULT)
endif

# We modify the configuration so we use a modified copy
OCR_CE_CONFIG_ORIG := $(OCR_CE_CONFIG)
OCR_XE_CONFIG_ORIG := $(OCR_XE_CONFIG)

OCR_CE_CONFIG := $(OCR_CE_CONFIG)_fix
OCR_XE_CONFIG := $(OCR_XE_CONFIG)_fix


WORKLOAD_EXEC         ?= $(WORKLOAD_INSTALL)
WORKLOAD_LOGS         ?= $(WORKLOAD_EXEC)/logs

REQ_LIBS         ?=

PREFIX  := $(TG_INSTALL)/bin/xstg-linux-elf
AS	:= $(PREFIX)-as
ASFLAGS	:=

AR	:= $(PREFIX)-ar
ARFLAGS	:= rcs

CC	:= $(PREFIX)-clang

OBJCOPY	:= $(PREFIX)-objcopy
STRIP	:= $(PREFIX)-strip
OBJDUMP := $(PREFIX)-objdump
LD	:= $(PREFIX)-ld

CUT	:= cut
GREP	:= grep
SED     := sed
RM	:= rm
CP      := cp
MKDIR   := mkdir

SECTION_STRING := "^  0"
LINKER_SCRIPT := ocr-default.ld

CFLAGS_BASE  := -fxstg-extensions -I $(OCR_INSTALL)/include -DTG_ARCH -I $(APPS_LIBS_INSTALL)/include -I $(TG_INSTALL)/include
LDFLAGS_BASE := -L $(APPS_LIBS_INSTALL)/lib -L $(TG_INSTALL)/ld-scripts -T $(LINKER_SCRIPT) -static -emainEdt -Map=$(WORKLOAD_BUILD)/out.map


CFLAGS  := $(CFLAGS_BASE) $(CFLAGS) -DOCR_TYPE_H=$(XE_TARGET).h
LDFLAGS := $(LDFLAGS_BASE) $(LDFLAGS)

OBJS := $(patsubst %.c,$(WORKLOAD_BUILD)/%.o,$(SRCS))

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

# Special case to include old libc files
USE_OLD_LIBC := $(strip $(foreach count, $(shell seq -s ' ' $(words $(REQ_LIBS))), $(if $(findstring libc,$(word $(count), $(REQ_LIBS))),yes)))

ifeq ($(USE_OLD_LIBC), yes)
  CFLAGS := -I $(APPS_LIBS_INSTALL)/include.old $(CFLAGS)
endif

ifneq ($(RUN_TORQUE), no)
  RUN_CMD := $(TG_INSTALL)/bin/scripts/fsim-wrapper.sh
  MACHINE_CONFIG :=
# GDB is not supported for fsim-wrapper.sh for now
  RUN_GDB := no
else
  ifdef MACHINE_CONFIG
    MACHINE_CONFIG := -f $(MACHINE_CONFIG)
  endif
  ifdef PROCESS_PER_NODE
    PROCESS_PER_NODE := -p $(PROCESS_PER_NODE)
  endif
  RUN_CMD := $(TG_INSTALL)/bin/scripts/mpi_launch.sh
endif

# This sounds to make the assumption that whether or not
# RUN_TORQUE is set if RUN_MODE is runApp we are already

ifeq ($(RUN_MODE), runApp)
  RUN_CMD := $(TG_INSTALL)/bin/scripts/fsim-torque.sh
  MACHINE_CONFIG :=
endif

ifeq ($(RUN_GDB), yes)
  GDB_CFLAGS := -DTG_GDB_SUPPORT
else
  GDB_CFLAGS :=
endif

.PHONY: all
all: $(WORKLOAD_BUILD)/$(TARGET).blob $(WORKLOAD_BUILD)/$(TARGET)

### Install rules ###
.PHONY: install_internal
ifeq ($(RUN_MODE), runApp)
  install_internal: preinstall | ${WORKLOAD_INSTALL}
else
  # Order of both should not matter here
  install: preinstall $(TG_CONFIG) $(WORKLOAD_INSTALL)/tgkrnl | ${WORKLOAD_INSTALL}
endif
	@echo "Installing from $(WORKLOAD_BUILD) into $(WORKLOAD_INSTALL)..."
	@echo ""
	@echo "####"
	@echo "To run, use 'OCR_TYPE=${OCR_TYPE} make run <args>' or run manually:"
	@echo "(from $(WORKLOAD_EXEC)): ${RUN_CMD} ${MACHINE_CONFIG} ${PROCESS_PER_NODE} -D $(WORKLOAD_LOGS) -L $(WORKLOAD_LOGS) -c ${WORKLOAD_INSTALL}/config.cfg $(ENERGY_CONFIG) -c $(TG_INSTALL)/fsim-configs/dvfs-default.cfg"
	@echo "####"
	$(AT)$(RM) -f $(WORKLOAD_INSTALL)/$(TARGET) $(WORKLOAD_INSTALL)/$(TARGET).blob
	$(AT)$(RM) -f $(WORKLOAD_INSTALL)/config.cfg
	$(AT)$(CP) $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_INSTALL)/
	$(AT)$(CP) $(WORKLOAD_BUILD)/$(TARGET).blob $(WORKLOAD_INSTALL)/
	$(AT)$(CP) $(TG_CONFIG) $(WORKLOAD_INSTALL)/
	$(AT)$(FIX_TG_FSIM_CONFIG) --tginstall $(TG_INSTALL) --fsimcfg $(WORKLOAD_INSTALL)/config.cfg

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
run_internal:
ifeq ($(RUN_MODE), runApp)
  run_internal:
else
  run_internal: install
endif
ifeq ($(HOSTNAME), bar1)
	$(error Please refrain from running jobs on bar1!)
endif
	$(AT)$(MKDIR) -p $(WORKLOAD_LOGS)
	$(AT)$(RM) -rf $(WORKLOAD_LOGS)/*
	$(AT)$(RM) -f $(WORKLOAD_EXEC)/console.out
  ifeq ($(RUN_GDB), yes)
	$(AT)rm -f $(WORKLOAD_INSTALL)/gdbscript-xe $(WORKLOAD_INSTALL)/gdbscript-ce $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
# Generate the XE GDB script
	$(AT)echo "target xstg -c $(WORKLOAD_INSTALL)/config.cfg,$(TG_INSTALL)/fsim-configs/dvfs-default.cfg,$(ORIG_MACHINE_CONFIG) -f $(TG_INSTALL)/bin/fsim -w $(WORKLOAD_INSTALL) -l $(WORKLOAD_LOGS)" > $(WORKLOAD_INSTALL)/gdbscript-xe
	$(AT)echo "break __xeDoAssert" >> $(WORKLOAD_INSTALL)/gdbscript-xe
    ifeq ($(RUN_GDB_HALT_XE), yes)
	$(AT)echo "break xePdSwitchRunlevel" >> $(WORKLOAD_INSTALL)/gdbscript-xe
	$(AT)echo "run" >> $(WORKLOAD_INSTALL)/gdbscript-xe
	$(AT)echo "disable 2" >> $(WORKLOAD_INSTALL)/gdbscript-xe
    else
	$(AT)echo "run" >> $(WORKLOAD_INSTALL)/gdbscript-xe
    endif
# Take care of the CE GDB script
	$(AT)echo "break __ceWaitForGdb" > $(WORKLOAD_INSTALL)/gdbscript-ce
	$(AT)echo "target remote :1234" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "interrupt" >> $(WORKLOAD_INSTALL)/gdbscript-ce
	$(AT)echo "break __ceDoAssert" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "next" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "continue" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "finish" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "disable 1" >> $(WORKLOAD_INSTALL)/gdbscript-ce
#	$(AT)echo "set __gdbConnected=1" >> $(WORKLOAD_INSTALL)/gdbscript-ce
    ifeq ($(RUN_GDB_HALT_CE), no)
	$(AT)echo "continue" >> $(WORKLOAD_INSTALL)/gdbscript-ce
    endif
# Finally, generate the TMUX configuration to launch the two GDBs as well as a tail
# on the output. Note that for now, we only deal with ONE block. This could be extended to deal with multiple
# blocks though but we would have to parse some configuration file to figure it out
	$(AT)echo "session_name: GDB $(TARGET)" > $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "environment:" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "  WORKLOAD_EXEC: $(WORKLOAD_EXEC)" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "  WORKLOAD_INSTALL: $(WORKLOAD_INSTALL)" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "  WORKLOAD_LOGS: $(WORKLOAD_LOGS)" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "  FSIM_EXE: fsim" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "windows:" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "  - window_name: Block 1" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "    layout: main-vertical" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "    panes:" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "    - shell_command:" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "      - sleep 5" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "      - while [ ! -f $(WORKLOAD_LOGS)/*.rck000.cub00.skt0.cls000.blk00.CE.00 ]; do sleep 1; done" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "      - tail -f $(WORKLOAD_LOGS)/*.rck000.cub00.skt0.cls000.blk00.CE.00" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
    ifeq ($(RUN_GDB_HALT_XE), no)
      ifeq ($(RUN_GDB_HALT_CE), no)
	$(AT)echo "      focus: true" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
      endif
    endif
	$(AT)echo "    - shell_command:" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "      - sleep 10" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
	$(AT)echo "      - gdb -se $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl.dbg -x $(WORKLOAD_INSTALL)/gdbscript-ce" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
    ifeq ($(RUN_GDB_HALT_XE), no)
      ifeq ($(RUN_GDB_HALT_CE), yes)
	$(AT)echo "      focus: true" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
      endif
    endif
	$(AT)echo "    - $(TG_INSTALL)/bin/xstg-linux-elf-gdb -se $(WORKLOAD_BUILD)/$(TARGET).dbg -x $(WORKLOAD_INSTALL)/gdbscript-xe" >> $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml
# Finally, launch the thing
	bash -ec 'cd $(WORKLOAD_LOGS) && tmuxp load $(WORKLOAD_INSTALL)/gdbscript-tmux.yaml'
	$(AT)$(GREP) -H "CONSOLE: XE[0-7]" $(WORKLOAD_LOGS)/*rck*.cub*.skt*.cls*.blk*.CE.00 \
	| $(SED) "s/.*\.blk\([0-9]\+\)\.CE\.00:CONSOLE:/BLK\1/" > $(WORKLOAD_EXEC)/console.out
  else
	$(AT)WORKLOAD_EXEC=$(WORKLOAD_EXEC) WORKLOAD_INSTALL=$(WORKLOAD_INSTALL) WORKLOAD_LOGS=$(WORKLOAD_LOGS) \
	RUN_TORQUE=$(RUN_TORQUE) \
	bash -ec 'cd $(WORKLOAD_LOGS) && ${RUN_CMD} ${MACHINE_CONFIG} ${PROCESS_PER_NODE} -D $(WORKLOAD_LOGS) -L $(WORKLOAD_LOGS) -c ${WORKLOAD_INSTALL}/config.cfg $(ENERGY_CONFIG) -c $(TG_INSTALL)/fsim-configs/dvfs-default.cfg && \
	$(GREP) -H "CONSOLE: XE[0-7]" $(WORKLOAD_LOGS)/*rck*.cub*.skt*.cls*.blk*.CE.00 \
	| $(SED) "s/.*\.blk\([0-9]\+\)\.CE\.00:CONSOLE:/BLK\1/" > $(WORKLOAD_EXEC)/console.out'
  endif

postrun: run_internal
postrun: WORKLOAD_OUTS := `find -L $(WORKLOAD_LOGS)/* -name '*.app_output' -print`

.PHONY: run
run: postrun

### Build rules ###

# Enforce ordering
$(WORKLOAD_BUILD)/$(TARGET).blob: | prebuild
$(WORKLOAD_BUILD)/$(TARGET): | prebuild
prebuild: reqlibs

ifeq ($(WITH_NEWLIB), yes)
  TG_CONFIG_TEMPLATE_CFG ?= $(APPS_ROOT)/tg-configs/config-newlib-template.cfg
else
  TG_CONFIG_TEMPLATE_CFG ?= $(APPS_ROOT)/tg-configs/config-template.cfg
endif

.PHONY: $(TG_CONFIG)
$(TG_CONFIG):
ifeq ($(TG_CONFIG_DEFAULT), $(TG_CONFIG))
	@echo "Generating TG config file for $(TARGET)"
	@echo "[environment]" > $(TG_CONFIG)
	@echo "   TARGET = $(TARGET)" >> $(TG_CONFIG)
	@cat  $(TG_CONFIG_TEMPLATE_CFG) >> $(TG_CONFIG)
endif

# The next three rules force postbuild to run after build_internal
# and default to run postbuild
.PHONY: build_internal
build_internal: reqlibs prebuild $(WORKLOAD_BUILD)/$(TARGET) $(WORKLOAD_BUILD)/$(TARGET).blob tgkrnl

postbuild: build_internal

.PHONY: default
default: postbuild

# Builds any libraries required by the workload
.PHONY: parallel-reqlibs
parallel-reqlibs: $(FULL_REQ_LIBS)

# Build libocrxe first as the libraries may need OCR to be properly installed
.PHONY: $(FULL_REQ_LIBS)
$(FULL_REQ_LIBS): | libocrxe
	$(AT)lockfile "/tmp/$(subst /,_,$@)_lock"; \
	echo "Grabbed lock"; date; \
	ARCH=$(ARCH) APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
	APPS_LIBS_INSTALL_ROOT=$(APPS_LIBS_INSTALL_ROOT) APPS_LIBS_INSTALL=$(APPS_LIBS_INSTALL) \
	OCR_ROOT=$(OCR_ROOT) OCR_BUILD_ROOT=$(OCR_BUILD_ROOT) \
	OCR_INSTALL=$(OCR_INSTALL) OCR_TYPE=$(OCR_TYPE) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) \
	$(MAKE) -C $@ install; \
	status=$$?; \
	echo "Releasing lock"; date; \
	rm -f "/tmp/$(subst /,_,$@)_lock"; \
	exit $$status

.PHONY: reqlibs
reqlibs: parallel-reqlibs

# This does a more complicate dependency computation so all the prereqs listed
# will also become command-less, prereq-less targets. This causes make
# to remake anything that depends on that file which is
# exactly what we want. This is adapted from:
# http://scottmcpeak.com/autodepend/autodepend.html
# Other information on some of the other rules for dependences can be found
# here: http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colon
#   touch:  sets the time to the same as the .o to avoid
#           erroneous detection next time around

# Delete default rules so it makes sure to use ours
%.o: %.c

# Require libocrxe as this means all include files are up-to-date
$(WORKLOAD_BUILD)/%.o: $(WORKLOAD_SRC)/%.c | libocrxe $(WORKLOAD_BUILD)
	@echo "Compiling $<"
	$(AT)$(MKDIR) -p $(@D)
	$(AT)$(CC) $(CFLAGS) -fno-builtin -MMD -c -o $@ $<
	$(AT)cp -f $(@:.o=.d) $(@:.o=.d.tmp)
	$(AT)sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d.tmp) | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/: /' >> $(@:.o=.d)
	$(AT)touch -r $@ $(@:.o=.d)
	$(AT)rm -f $(@:.o=.d.tmp)
#	$(AT)$(CC) $(CFLAGS) -fno-builtin -c -o $@.tmp $<
#	$(AT)$(OBJCOPY) -R .data_local -R .rodata_local -R .bss_local -R .data_block -R .rodata_block -R .bss_block $@.tmp $@
#	$(AT)$(RM) $@.tmp

# Include auto-generated dependence files
$(WORKLOAD_BUILD)/%.d: ;
.PRECIOUS: $(WORKLOAD_BUILD)/%.d
-include $(OBJS:.o=.d)

# Two pre-requisites here that seem to be the same but serve different purposes:
#  - libocr.a: Execute this rule if libocr.a changes due to being rebuilt (due to the libocrxe rule)
#  - libocrxe: This rule will ALWAYS execute (it is a PHONY target) and it will
#              check if libocr.a needs to be rebuilt. If we listed this as a
#              regular pre-requisite, we would *always* rebuild the blob (PHONY rules are infectious)
# In other words, since we use the .a (and not a .so), we need to run this rule if the .a changes.
# Before that, thought, we need to check if we need to update the .a and that is the purpose
# of the libocrxe dependence
# Other rules for the blobs follow similar ideas
$(WORKLOAD_BUILD)/$(TARGET).dbg: $(OBJS) $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a | libocrxe $(WORKLOAD_BUILD)
	$(AT)$(LD) -o $@ $(OBJS) --whole-archive $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a --no-whole-archive $(LDFLAGS)

$(WORKLOAD_BUILD)/$(TARGET): $(WORKLOAD_BUILD)/$(TARGET).dbg | $(WORKLOAD_BUILD)
	$(AT)$(RM) -f $(WORKLOAD_BUILD)/$(TARGET)
	$(AT)$(CP) $(WORKLOAD_BUILD)/$(TARGET).dbg $(WORKLOAD_BUILD)/$(TARGET)
	$(AT)$(STRIP) $(WORKLOAD_BUILD)/$(TARGET)

# Build the final blob
# Note the order of the dependence. IMPORTANT!! Do not randomly change
$(WORKLOAD_BUILD)/$(TARGET).blob: $(OCR_INSTALL)/share/ocr/scripts/Blob/aggregate_binary_files.sh \
$(WORKLOAD_BUILD)/$(TARGET).blob.ce $(WORKLOAD_BUILD)/$(TARGET).blob.xe \
$(WORKLOAD_BUILD)/$(TARGET).blob.arg | $(WORKLOAD_BUILD)
	@echo "WORKLOAD_INPUTS is $(WORKLOAD_INPUTS) and args are ###$^###"
	$(AT)$^ $(WORKLOAD_INPUTS) $@

# Build the CE blob
$(WORKLOAD_BUILD)/$(TARGET).blob.ce: $(OCR_CE_CONFIG) $(TG_CONFIG) \
$(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe \
$(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl.dbg | tgkrnl.dbg builderce $(WORKLOAD_BUILD)
	@echo "#### BUILDING CE BLOB ####"
	$(AT)$(FIX_CE_CONFIG) --fsimcfg $(TG_CONFIG) --ocrcfg $(OCR_CE_CONFIG)
	$(AT)TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.ce \
	$(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe -ocr:cfg $(OCR_CE_CONFIG)

$(OCR_CE_CONFIG_DEFAULT): $(OCR_INSTALL)/share/ocr/config/$(CE_TARGET)/default.cfg
	$(AT)$(CP) $< $@

# Need libocrce (built internally by tgkrnl) to generate the config file
$(OCR_INSTALL)/share/ocr/config/$(CE_TARGET)/default.cfg: | tgkrnl

# ALways remake this
.PHONY: $(OCR_CE_CONFIG)
$(OCR_CE_CONFIG): $(OCR_CE_CONFIG_ORIG)
	$(AT)$(CP) $< $@

# Build the XE blob
$(WORKLOAD_BUILD)/$(TARGET).dbg: | debug_post_prerun
$(WORKLOAD_BUILD)/$(TARGET).blob.xe: $(OCR_XE_CONFIG) \
$(WORKLOAD_BUILD)/$(TARGET).dbg $(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe | debug_post_prerun builderxe $(WORKLOAD_BUILD) sectionSizeCheck
	@echo "#### BUILDING XE BLOB ####"
	$(AT)$(FIX_XE_CONFIG) --ocrcfg $(OCR_XE_CONFIG) --fsimcfg $(TG_CONFIG) --binstart $(shell $(OBJDUMP) -x $(WORKLOAD_BUILD)/$(TARGET).dbg | \
	grep $(SECTION_STRING) | awk '{ print $$4 }' | sed -e 's/^0*/0x/g') --binend $(shell $(OBJDUMP) -x $(WORKLOAD_BUILD)/$(TARGET).dbg | grep _end_local | cut -d\  -f1 | sed -e 's/^0*/0x/g')
	$(AT)TG_INSTALL=$(TG_INSTALL) WORKLOAD_BUILD=$(WORKLOAD_BUILD) STRUCT_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.xe \
	ARGS_OUT=$(WORKLOAD_BUILD)/$(TARGET).blob.arg APP_FILE=$(WORKLOAD_BUILD)/$(TARGET).dbg \
	$(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe -ocr:cfg $(OCR_XE_CONFIG) $(WORKLOAD_ARGS)
# XE blob must be phony in order to rebuild the args blob on every run
.PHONY: $(WORKLOAD_BUILD)/$(TARGET).blob.xe

.PHONY: sectionSizeCheck
sectionSizeCheck: $(WORKLOAD_BUILD)/$(TARGET).dbg
	$(eval ROSIZE:=$(shell $(OBJDUMP) -x $(WORKLOAD_BUILD)/$(TARGET).dbg | grep "^  0 .rodata_local "| cut -d\  -f5 | tr '[:lower:]' '[:upper:]'))
	$(eval ROSIZE:=$(shell echo "ibase=16; $(ROSIZE)"|bc))
	$(eval ROLIMIT:=$(shell echo "ibase=16; 80000"|bc))
	$(eval CHECK:=$(shell test $(ROSIZE) -lt $(ROLIMIT); echo $$?))
ifeq ($(CHECK), 0)
	$(error "Section .rodata size $(ROSIZE) exceeds limit $(ROLIMIT) - please bump up IPM start address in CE's .cfg file")
endif

$(OCR_XE_CONFIG_DEFAULT): $(OCR_INSTALL)/share/ocr/config/$(XE_TARGET)/default.cfg
	$(AT)$(CP) $< $@

# Need libocrxe to generate the config file
$(OCR_INSTALL)/share/ocr/config/$(XE_TARGET)/default.cfg: | libocrxe

# Need libocrxe (or one of them) to get the aggregate script
$(OCR_INSTALL)/share/ocr/scripts/Blob/aggregate_binary_files.sh: | libocrxe

# Always remake this
.PHONY: $(OCR_XE_CONFIG)
$(OCR_XE_CONFIG): $(OCR_XE_CONFIG_ORIG)
	$(AT)$(CP) $< $@

# The arg blob is build automatically by the XE builder
$(WORKLOAD_BUILD)/$(TARGET).blob.arg: $(WORKLOAD_BUILD)/$(TARGET).blob.xe | $(WORKLOAD_BUILD)

# XE libocr
.PHONY: libocrxe
ifeq ($(RUN_MODE), runApp)
  libocrxe: ;
  $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a: ;
else ifeq ($(RUN_MODE), buildApp)
  # If we are running in Jenkins, and building the app, we just need to make sure the library is built
  # This will error out if it is not
  libocrxe: | $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a
	@echo "Checked that XE libocr_$(XE_TARGET).a exists."
  $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a:
	$(error Cannot find XE libocr_$(XE_TARGET).a in $(OCR_INSTALL))
else
  # In all other cases, we build the library
  libocrxe: $(OCR_BUILD_XE)/Makefile
	@echo "Building XE libocr.a..."
	$(AT)V=$(V) OCR_TYPE=$(XE_TARGET) OCR_INSTALL=$(OCR_INSTALL) OCR_ROOT=$(OCR_ROOT) OCR_BUILD=$(OCR_BUILD_XE) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) \
	CFLAGS_USER=$(GDB_CFLAGS) $(MAKE) -C $(OCR_BUILD_XE) $(TARGETS) install
  $(OCR_INSTALL)/lib/libocr_$(XE_TARGET).a: | libocrxe
endif

# Note: builderxe, tgkrnl, builderce all follow the same idea as libocrxe

# XE builder
.PHONY: builderxe
ifeq ($(RUN_MODE), runApp)
  builderxe: ;
  $(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe: ;
else ifeq ($(RUN_MODE), buildApp)
  builderxe: | $(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe
	@echo "Checked that XE builder exists."
  $(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe:
	$(error Cannot find XE builder in $(OCR_INSTALL))
else
  # In all other cases, we build the builder
  builderxe: $(OCR_BUILD_ROOT)/$(XE_BUILDER)/Makefile
	@echo "Building XE builder..."
	$(AT)OCR_TYPE=$(XE_BUILDER) OCR_INSTALL=$(OCR_INSTALL) OCR_ROOT=$(OCR_ROOT) OCR_BUILD=$(OCR_BUILD_XEBUILDER) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) \
	$(MAKE) -C $(OCR_BUILD_ROOT)/$(XE_BUILDER) $(TARGETS) install
  $(OCR_INSTALL)/bin/ocrBuilder_$(XE_BUILDER).exe: | builderxe
endif

# Local version of the TGKRNL (built with the CE's libocr)
# We build locally so we can run without impacting TG_INSTALL
# This will internally build libocrce.a if needed (ie: we don't build it
# explicitly here)
.PHONY: tgkrnl

ifeq ($(RUN_MODE), runApp)
  tgkrnl: ;
  $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl: ;
  $(WORKLOAD_INSTALL)/tgkrnl: ;
else
  # In all other cases, we build the kernel
  # (including for building the app)
  tgkrnl: $(TG_ROOT)/build/build.tgkrnl/make-me.make \
  | $(WORKLOAD_BUILD)/build.tgkrnl
	@echo "Building tgkrnl..."
	$(AT)V=$(V) OCR_ROOT=$(OCR_ROOT) OCR_TYPE=$(CE_TARGET) OCR_INSTALL=$(OCR_INSTALL) OCR_BUILD=$(OCR_BUILD_CE) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) OCRAPPS_RUN_MODE=$(RUN_MODE) \
	WITH_LIBOCR=yes CFLAGS_USER=$(GDB_CFLAGS) $(MAKE) -C $(WORKLOAD_BUILD)/build.tgkrnl -f make-me.make $(TARGETS)
	strip -s -o $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl.dbg

  $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl: | tgkrnl

  # This target will effectively always run which is what we want
  # because for some reason, pre-reqs are evaluated before (and so when tgkrnl recipe
  # remakes build.tgkrnl/tgkrnl, nothing happens
  $(WORKLOAD_INSTALL)/tgkrnl: $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl tgkrnl $(WORKLOAD_INSTALL)
	@echo "Installing tgkrnl..."
	$(AT)$(RM) -f $(WORKLOAD_INSTALL)/tgkrnl
	$(AT)$(CP) $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl $(WORKLOAD_INSTALL)/
endif

.PHONY: tgkrnl.dbg
tgkrnl.dbg: tgkrnl

$(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl.dbg: $(WORKLOAD_BUILD)/build.tgkrnl/tgkrnl

# CE builder
.PHONY: builderce
ifeq ($(RUN_MODE), runApp)
  builderce: ;
  $(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe: ;
else ifeq ($(RUN_MODE), buildApp)
  builderce: | $(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe
	@echo "Checked that CE builder exists."
  $(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe:
	$(error Cannot find CE builder in $(OCR_INSTALL))
else
  # In all other cases, we build the builder
  builderce: $(OCR_BUILD_ROOT)/$(CE_BUILDER)/Makefile
	@echo "Building CE builder..."
	$(AT)OCR_TYPE=$(CE_BUILDER) OCR_INSTALL=$(OCR_INSTALL) OCR_ROOT=$(OCR_ROOT) OCR_BUILD=$(OCR_BUILD_CEBUILDER) \
	$(MAKE) -C $(OCR_BUILD_ROOT)/$(CE_BUILDER) $(TARGETS) install
  $(OCR_INSTALL)/bin/ocrBuilder_$(CE_BUILDER).exe: | builderce
endif

.PHONY: clean
clean:
	$(AT)-$(RM) -rf $(WORKLOAD_BUILD)

.PHONY: uninstall
uninstall:
	$(AT)-$(RM) -rf $(WORKLOAD_INSTALL)

$(WORKLOAD_BUILD):
	$(AT)$(MKDIR) -p $(WORKLOAD_BUILD)

$(WORKLOAD_BUILD)/build.tgkrnl: | $(WORKLOAD_BUILD) # $(TG_ROOT)/build/build.tgkrnl/make-me.make | $(WORKLOAD_BUILD)
	$(AT)$(MKDIR) -p $(WORKLOAD_BUILD)/build.tgkrnl
	$(AT)$(RM) -f $(WORKLOAD_BUILD)/build.tgkrnl/make-me.make
	$(AT)$(CP) $(TG_ROOT)/build/build.tgkrnl/make-me.make $(WORKLOAD_BUILD)/build.tgkrnl

$(WORKLOAD_INSTALL):
	$(AT)$(MKDIR) -p $(WORKLOAD_INSTALL)

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
