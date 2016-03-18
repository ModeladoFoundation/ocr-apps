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

# APPS_ROOT needs to be defined at a minimum
ifeq (,$(APPS_ROOT))
$(error APPS_ROOT needs to be defined)
else
_T := $(APPS_ROOT)
APPS_ROOT := $(realpath $(_T))
ifeq (,$(APPS_ROOT))
$(error APPS_ROOT is not a valid path: $(_T))
endif
endif

# Check all other variables using varcheck
_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

REQ_LIBS ?=

FULL_REQ_LIBS := $(addprefix $(APPS_ROOT)/libs/src/,$(REQ_LIBS))

# Builds any libraries we depend on
.PHONY: parallel-reqlibs
parallel-reqlibs: $(FULL_REQ_LIBS)

.PHONY: $(FULL_REQ_LIBS)
$(FULL_REQ_LIBS):
	$(AT)lockfile "/tmp/$(subst /,_,$@)_lock"; \
	echo "Grabbed lock"; date; \
	V=$(V) ARCH=$(ARCH) APPS_ROOT=$(APPS_ROOT) APPS_LIBS_ROOT=$(APPS_LIBS_ROOT) \
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
