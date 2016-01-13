# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# This file is meant to be used by libraries to build other
# libraries. If you depend on other libraries within your
# library, set REQ_LIBS to the name of the libraries that
# you depend on and then likst reqlibs as a dependence
# of your build target. Note that reqlibs is a PHONY
# target so if you do not want it to infect the rebuilding
# of your actual target, either list it as an order-only
# dependence (using "|") or list it as a dependence
# of another phony target (like 'all', or whatever).

# You need to make sure that you have the following environement
# variables defined if needed (there are no default but these will
# normally be passed down naturally to the building of your library)
#   - ARCH              tg or x86
#   - APPS_ROOT         Path to the apps directory
#   - APPS_LIBS_ROOT    Path to the libs directory (typically $(APPS_ROOT)/lib)
#   - OCR_ROOT          Path to the OCR code base
#   - TG_INSTALL        Path to the TG environment installation
#   - TG_ROOT           Path to the TG root

ifndef ARCH
endif

ifeq ($(RUN_JENKINS), runApp)
APPS_ROOT             :=
else ifeq ($(RUN_JENKINS), buildApp)
OCR_ROOT              := $(shell cd "${OCR_ROOT}" && pwd)
APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)
else
TG_ROOT               := $(shell cd "${TG_ROOT}" && pwd)
OCR_ROOT              := $(shell cd "${OCR_ROOT}" && pwd)
APPS_ROOT             := $(shell cd "${APPS_ROOT}" && pwd)
endif

APPS_LIBS_ROOT        := $(shell cd "${APPS_LIBS_ROOT}" && pwd)

TG_INSTALL            := $(shell cd "${TG_INSTALL}" && pwd)

REQ_LIBS ?=

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

# Builds any libraries we depend on
.PHONY: parallel-reqlibs
parallel-reqlibs: $(FULL_REQ_LIBS)

.PHONY: $(FULL_REQ_LIBS)
$(FULL_REQ_LIBS):
	lockfile "/tmp/$(subst /,_,$@)_lock"
	echo "Grabbed lock"; date; \
	ARCH=$(ARCH) APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
	OCR_ROOT=$(OCR_ROOT) \
	TG_ROOT=$(TG_ROOT) TG_INSTALL=$(TG_INSTALL) \
	$(MAKE) -C $@ install; \
	echo "Releasing lock"; date
	rm -f "/tmp/$(subst /,_,$@)_lock"

.PHONY: reqlibs
reqlibs: parallel-reqlibs
