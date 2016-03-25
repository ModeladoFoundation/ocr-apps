#
# For x86 (using package in x86_64) use the makefile build by configure
#

ARCH                 	?= x86
OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

CRLIBM_ROOT     ?= ../x86_64

CRLIBM_ROOT    := $(shell cd "${CRLIBM_ROOT}" && pwd)

CP=cp
RM=rm
MKDIR=mkdir

.PHONY: squeaky
squeaky:
	$(AT)(cd $(CRLIBM_ROOT);./RAG-clobber.sh)

.PHONY: configure
configure: $(CRLIBM_ROOT)/Makefile

$(CRLIBM_ROOT)/Makefile: $(CRLIBM_ROOT)/RAG-install.sh
	$(AT)(cd $(CRLIBM_ROOT);./RAG-install.sh)

.PHONY: all
all: $(CRLIBM_ROOT)/Makefile
	$(AT)$(MAKE) -C $(CRLIBM_ROOT) all
	$(AT)$(MKDIR) -p lib include
	$(AT)$(CP) $(CRLIBM_ROOT)/crlibm.h    include/crlibm.h
	$(AT)$(CP) $(CRLIBM_ROOT)/libcrlibm.a lib/libcrlibm.a

.PHONY: clean
clean:
	$(AT)$(RM) -rf lib include bin \
	$(CRLIBM_ROOT)/scs_lib/*.o \
	$(CRLIBM_ROOT)/tests/*.o \
	$(CRLIBM_ROOT)/*.o

.PHONY: check
check:
	$(AT)$(MAKE) -C $(CRLIBM_ROOT) check

lib/libcrlibm.a: | all

include/crlibm.h: | all

$(APPS_LIBS_INSTALL)/lib/libcrlibm.a: lib/libcrlibm.a | $(APPS_LIBS_INSTALL)/lib
	$(AT)$(CP) -f lib/libcrlibm.a $(APPS_LIBS_INSTALL)/lib/libcrlibm.a

$(APPS_LIBS_INSTALL)/include/crlibm.h: include/crlibm.h | $(APPS_LIBS_INSTALL)/include
	$(AT)$(CP) -f include/crlibm.h $(APPS_LIBS_INSTALL)/include/crlibm.h

.PHONY: $(APPS_LIBS_INSTALL)/lib $(APPS_LIBS_INSTALL)/include
$(APPS_LIBS_INSTALL)/lib $(APPS_LIBS_INSTALL)/include:
	$(AT)mkdir -p $@

.PHONY: install
install: $(APPS_LIBS_INSTALL)/lib/libcrlibm.a $(APPS_LIBS_INSTALL)/include/crlibm.h

.PHONY: uninstall
uninstall:
	$(AT)$(RM) -f $(APPS_LIBS_INSTALL)/lib/libcrlibm.a
	$(AT)$(RM) -f $(APPS_LIBS_INSTALL)/include/crlibm.h

.PHONY: debug
debug:
	echo "Currently no x86 support for make debug"

.PHONY: run
run:
	echo "Currently no x86 support for make run"
