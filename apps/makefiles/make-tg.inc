# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# This file will be deprecated. Use make-inc.inc instead
$(warning Using make-tg.inc is deprecated -- include make-inc.inc instead)

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

include $(APPS_ROOT)/makefiles/make-inc.inc
