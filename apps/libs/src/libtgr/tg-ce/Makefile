#
# Makefile to build the tgr-ce library and a tgkrnl that links it in
#
TG_ROOT   ?= ../../../../../../tg/tg
TG_INSTALL = $(TG_ROOT)/install

APPS_REPO ?= ../../../../..
APPS_INSTALL = $(APPS_REPO)/apps/libs/install/tg-xe
APPS_INCLUDE = $(APPS_INSTALL)/include

TGR_SRC = src
TGR_INC = include

TGKRNL_ROOT = $(TG_ROOT)/tgkrnl
TGKRNL_INC  = $(TGKRNL_ROOT)/inc
TGKRNL_BUILD = $(TG_ROOT)/build/build.tgkrnl

TGR_CE_BUILD = $(addsuffix /../build-tg-ce,$(shell pwd))

FSIM = $(TG_INSTALL)/bin/fsim

TGR_CE_SRCS =   ce-os-svc.c \
                ce-xe-intf.c \
                cfg_parse.c  \
                memory.c     \
                mem_util.c   \
                tgr-alarms.c \
                tgr-main.c   \
                tgr-run.c    \
                util.c       \
                xe-elf64.c

TGR_CE_OBJS = $(addprefix $(TGR_CE_BUILD)/,$(TGR_CE_SRCS:.c=.o))

TGR_CE_LIB = $(TGR_CE_BUILD)/tgr-ce-lib.a

INCLUDES = -I$(APPS_INCLUDE) \
           -I$(TGR_INC) \
           -I$(TG_ROOT)/common/include \
           -I$(TGKRNL_INC) \
           -I$(TG_ROOT)/fsim/include

OPT = -g -ggdb -g3
#
# These are the flags used to build tgkrnl
#
WARNINGS = -Wall -Wstrict-aliasing=0

CFLAGS_TGKRNL =  -std=c99 -m64 -mcmodel=large \
                 -fno-builtin -fno-stack-protector -fPIC
#
# We set -nostdinc to ensure we get the system includes from newlib
#
CFLAGS = $(OPT) -nostdinc $(INCLUDES) $(WARNINGS) $(CFLAGS_TGKRNL)
#
# verbose enables compiler cmdline printing
#
AT = @
ifdef VERBOSE
	AT =
endif

################################# Targets #####################################

all default : tgr-ce

install : tgr-ce
	@echo " * tgkrnl-tgr -> $(TG_INSTALL)/lib"
	@cp $(TGR_CE_BUILD)/tgkrnl-tgr $(TG_INSTALL)/lib

tgr-ce : $(TGR_CE_BUILD) $(TGR_CE_BUILD)/tgkrnl-tgr

$(TGR_CE_BUILD)/%.o : $(TGR_SRC)/%.c
	@echo " - $(notdir $@)"
	$(AT)$(CC) $(CFLAGS) -c $< -o $@

$(TGR_CE_LIB) : $(TGR_CE_OBJS)
	@echo " - $(notdir $@)"
	$(AT)ar cr $@ $^

#
# Build a tgkrnl linked with our tgr library
#
$(TGR_CE_BUILD)/tgkrnl-tgr : $(TGR_CE_LIB)
	@echo " ** building tgkrnl"
	$(AT)cd $(TGKRNL_BUILD) ;\
		$(MAKE) -f make-me.make TGR_LIB=$^ clean default
	@echo " * tgkrnl.dbg -> $@"
	$(AT)cp $(TGKRNL_BUILD)/tgkrnl.dbg $@

$(TGR_CE_BUILD) :
	@echo "making '$@'"
	@mkdir $@

.PHONY: clean ctags

ctags:
	ctags -R include src $(TG_ROOT)/common/include $(TG_ROOT)/tgkrnl

clean :
	rm -rf $(TGR_CE_BUILD)
