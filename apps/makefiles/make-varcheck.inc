# Verbosity of make
V ?= 0
AT_0 := @
AT_1 :=
AT = $(AT_$(V))

# This is set to yes by the make-libs.inc file because
# for libraries, there is no workload to define
_INTERNAL_NO_CHECK_WORKLOAD ?= no
##########################################
# Sanity checks for external dependences #
##########################################

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

# OCR_TYPE also needs to be defined
ifeq (,$(OCR_TYPE))
  $(error OCR_TYPE needs to be defined)
endif

# ARCH also needs to be defined. This is the
# architecture for the *user* application (ie:
# on TG for example, it should be tg-xe as no user
# code runs on the CE)
ifeq (,$(ARCH))
  # For now, for legacy reason, we attempt to "guess" the
  # architecture
  ifeq ($(OCR_TYPE), tg)
    $(info Setting ARCH to tg-xe)
    ARCH := tg-xe
  else
    $(info Setting ARCH to x86)
    ARCH := x86
  endif
else
  # Another legacy support for now (tg == tg-xe)
  ifeq ($(ARCH),tg)
    $(info Changing ARCH from tg to tg-xe)
    ARCH := tg-xe
  endif
  ifneq (,$(filter-out x86 x86-test tg-xe,$(ARCH)))
    $(error Invalid ARCH value: $(ARCH))
  endif
endif

ifeq ($(_INTERNAL_NO_CHECK_WORKLOAD), no)
  # WORKLOAD_INSTALL_ROOT needs to be defined
  # in all cases
  ifeq (,$(WORKLOAD_INSTALL_ROOT))
    $(error WORKLOAD_INSTALL_ROOT needs to be defined)
  else
    _T := $(WORKLOAD_INSTALL_ROOT)
    WORKLOAD_INSTALL_ROOT := $(realpath $(_T))
    ifeq (,$(WORKLOAD_INSTALL_ROOT))
      # We can create this directory
      @echo "Creating WORKLOAD_INSTALL_ROOT: $(_T)"
      $(shell mkdir -p "$(_T)")
      WORKLOAD_INSTALL_ROOT := $(realpath $(_T))
    endif
  endif
endif # _INTERNAL_NO_CHECK_WORKLOAD

##########################################
# Default values for dependent variables #
##########################################

# Set the other variables using APPS_ROOT
# (not all need to be valid, this is checked later)
APPS_LIBS_ROOT         ?= $(APPS_ROOT)/libs/src/
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install/
OCR_ROOT               ?= $(APPS_ROOT)/../../ocr/ocr/
OCR_INSTALL            ?= $(APPS_ROOT)/../../ocr/ocr/install/
TG_ROOT                ?= $(APPS_ROOT)/../../tg/tg/
TG_INSTALL             ?= $(APPS_ROOT)/../../tg/tg/install/

ifeq ($(_INTERNAL_NO_CHECK_WORKLOAD), no)
  WORKLOAD_INSTALL  := $(WORKLOAD_INSTALL_ROOT)/$(OCR_TYPE)
endif
# Other variables that are used (but not in all mode)
# OCR_BUILD_ROOT
# WORKLOAD_SRC
# WORKLOAD_BUILD_ROOT

##########################################
#    Check for required directories      #
##########################################

# Note that the variable RUN_MODE can be used
# to indicate that some portion of the makefiles
# are skipped (either if you are running the app
# and know that everything is already built or
# if you are building the app knowing that the
# runtime is already built)

# In all cases, we need the *_INSTALL directories (TG_INSTALL
# only for TG) and the location of libraries
_T := $(OCR_INSTALL)
OCR_INSTALL := $(realpath $(_T))
ifeq (,$(OCR_INSTALL))
  # Create the directory
  @echo "Creating OCR_INSTALL: $(_T)"
  $(shell mkdir -p "$(_T)")
  OCR_INSTALL := $(realpath $(_T))
endif

_T := $(APPS_LIBS_ROOT)
APPS_LIBS_ROOT := $(realpath $(_T))
ifeq (,$(APPS_LIBS_ROOT))
  $(error APPS_LIBS_ROOT is not a valid path: $(_T))
endif

_T := $(APPS_LIBS_INSTALL_ROOT)
APPS_LIBS_INSTALL_ROOT := $(realpath $(_T))
ifeq (,$(APPS_LIBS_INSTALL_ROOT))
  # Create the directory
  @echo "Creating APPS_LIBS_INSTALL_ROOT: $(_T)"
  $(shell mkdir -p "$(_T)")
  APPS_LIBS_INSTALL_ROOT := $(realpath $(_T))
endif

# Now take care of APPS_LIBS_INSTALL
_T := $(APPS_LIBS_INSTALL_ROOT)/$(ARCH)
APPS_LIBS_INSTALL := $(realpath $(_T))
ifeq (,$(APPS_LIBS_INSTALL))
  # Create the directory
  @echo "Creating APPS_LIBS_INSTALL: $(_T)"
  $(shell mkdir -p "$(_T)")
  APPS_LIBS_INSTALL := $(realpath $(_T))
endif

ifeq ($(_INTERNAL_NO_CHECK_WORKLOAD), no)
  _T := $(WORKLOAD_INSTALL)
  WORKLOAD_INSTALL := $(realpath $(_T))
  ifeq (,$(WORKLOAD_INSTALL))
    # Create the directory
    @echo "Creating WORKLOAD_INSTALL: $(_T)"
    $(shell mkdir -p "$(_T)")
    WORKLOAD_INSTALL := $(realpath $(_T))
  endif
endif

ifeq (, $(filter-out tg tg-x86, $(OCR_TYPE)))
  TG_INSTALL := $(realpath $(TG_INSTALL))
  ifeq (,$(TG_INSTALL))
    # We do not install this because we never build the
    # TG toolchain
    $(error TG_INSTALL is invalid)
  endif
endif

# Legacy support for RUN_JENKINS env variable
RUN_MODE ?= $(RUN_JENKINS)
ifeq ($(RUN_MODE), runApp)
  # Nothing additional for the runApp mode
else ifeq ($(RUN_MODE), buildApp)

  ifeq ($(_INTERNAL_NO_CHECK_WORKLOAD), no)
    # Build directory for workload
    _T := $(WORKLOAD_BUILD_ROOT)/$(OCR_TYPE)
    WORKLOAD_BUILD := $(realpath $(_T))
    ifeq (,$(WORKLOAD_BUILD))
      # Create the directory
      @echo "Creating WORKLOAD_BUILD: $(_T)"
      $(shell mkdir -p "$(_T)")
      WORKLOAD_BUILD := $(realpath $(_T))
    endif

    # Source directory for workload
    _T := $(WORKLOAD_SRC)
    WORKLOAD_SRC := $(realpath $(WORKLOAD_SRC))
    ifeq (,$(WORKLOAD_SRC))
      $(error WORKLOAD_SRC is not a valid path: $(_T))
    endif
  endif # End of _INTERNAL_NO_CHECK_WORKLOAD
  # End of buildApp mode
else

  ifeq ($(_INTERNAL_NO_CHECK_WORKLOAD), no)
    # Build directory for workload
    _T := $(WORKLOAD_BUILD_ROOT)/$(OCR_TYPE)
    WORKLOAD_BUILD := $(realpath $(_T))
    ifeq (,$(WORKLOAD_BUILD))
      # Create the directory
      @echo "Creating WORKLOAD_BUILD: $(_T)"
      $(shell mkdir -p "$(_T)")
      WORKLOAD_BUILD := $(realpath $(_T))
    endif

    # Source directory for workload
    _T := $(WORKLOAD_SRC)
    WORKLOAD_SRC := $(realpath $(WORKLOAD_SRC))
    ifeq (,$(WORKLOAD_SRC))
      $(error WORKLOAD_SRC is not a valid path: $(_T))
    endif
  endif # End of _INTERNAL_NO_CHECK_WORKLOAD

  # OCR source directory
  _T := $(OCR_ROOT)
  OCR_ROOT := $(realpath $(_T))
  ifeq (,$(OCR_ROOT))
    $(error OCR_ROOT is not a valid path: $(_T))
  endif

  # OCR build directory
  OCR_BUILD_ROOT ?= $(OCR_ROOT)/build
  _T := $(OCR_BUILD_ROOT)
  OCR_BUILD_ROOT := $(realpath $(_T))
  ifeq (,$(OCR_BUILD_ROOT))
    @echo "Creating OCR_BUILD_ROOT: $(_T)"
    $(shell mkdir -p "$(_T)")
    OCR_BUILD_ROOT := $(realpath $(_T))
  endif

  ifeq (, $(filter-out tg tg-x86, $(OCR_TYPE)))
    # TG root
    _T := $(TG_ROOT)
    TG_ROOT := $(realpath $(_T))
    ifeq (,$(TG_ROOT))
      $(error TG_ROOT is not a valid path: $(_T))
    endif
  endif

endif # End of RUN_MODE ifeq
