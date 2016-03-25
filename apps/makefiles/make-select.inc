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

##########################################
#         Select proper makefile         #
##########################################

ifeq (,$(filter-out x86 x86-mpi x86-gasnet x86-mpi x86-phi tg-x86, $(OCR_TYPE)))
  include ${APPS_ROOT}/makefiles/make-x86.inc
else ifeq (,$(filter-out tg, $(OCR_TYPE)))
  include ${APPS_ROOT}/makefiles/make-tg-xe.inc
else
  $(error Unknown OCR_TYPE)
endif
