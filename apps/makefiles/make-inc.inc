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

include $(APPS_ROOT)/makefiles/make-varcheck.inc
include $(APPS_ROOT)/makefiles/make-select.inc
