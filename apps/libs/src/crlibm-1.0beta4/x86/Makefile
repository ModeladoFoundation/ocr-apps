#
# For x86 (using package in x86_64) use the makefile build by configure
#

CRLIBM_ROOT     ?= ../x86_64

APPS_ROOT       ?= ../../../../../apps
APPS_LIBS_ROOT  ?= ../../../../../apps/libs/x86

CRLIBM_ROOT    := $(shell cd "${CRLIBM_ROOT}" && pwd)
APPS_ROOT      := $(shell cd "${APPS_ROOT}" && pwd)
APPS_LIBS_ROOT := $(shell cd "${APPS_LIBS_ROOT}" && pwd)

CP=cp
RM=rm
MKDIR=mkdir

.PHONY: squeaky
squeaky:
	(cd $(CRLIBM_ROOT);./RAG-clobber.sh)

.PHONY: configure
configure: $(CRLIBM_ROOT)/Makefile

$(CRLIBM_ROOT)/Makefile: $(CRLIBM_ROOT)/RAG-install.sh
	(cd $(CRLIBM_ROOT);./RAG-install.sh)

.PHONY: all
all: $(CRLIBM_ROOT)/Makefile
	$(MAKE) -C $(CRLIBM_ROOT) all
	$(MKDIR) -p lib include
	$(CP) $(CRLIBM_ROOT)/crlibm.h    include/crlibm.h
	$(CP) $(CRLIBM_ROOT)/libcrlibm.a lib/libcrlibm.a

.PHONY: clean
clean:
	$(RM) -rf lib include bin

.PHONY: check
check:
	$(MAKE) -C $(CRLIBM_ROOT) check

lib/libcrlibm.a: | all

include/crlibm.h: | all

$(APPS_LIBS_ROOT)/lib/libcrlibm.a: lib/libcrlibm.a
	$(CP) -f lib/libcrlibm.a $(APPS_LIBS_ROOT)/lib/libcrlibm.a

$(APPS_LIBS_ROOT)/include/crlibm.h: include/crlibm.h
	$(CP) -f include/crlibm.h $(APPS_LIBS_ROOT)/include/crlibm.h

.PHONY: install
install: $(APPS_LIBS_ROOT)/lib/libcrlibm.a $(APPS_LIBS_ROOT)/include/crlibm.h

.PHONY: uninstall
uninstall:
	$(RM) -f $(APPS_LIBS_ROOT)/lib/libcrlibm.a
	$(RM) -f $(APPS_LIBS_ROOT)/include/crlibm.h

.PHONY: debug
debug:
	echo "Currently no x86 support for make debug"

.PHONY: run
run:
	echo "Currently no x86 support for make run"
