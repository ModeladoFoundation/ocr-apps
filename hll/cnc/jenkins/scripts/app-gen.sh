#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
WORKLOAD_SRC=`${ROOT}/jenkins/scripts/get-app-path.sh ${1}`

echo ">>> CnC-OCR: Running translator for example ${1}"
cd "${WORKLOAD_SRC}" && ${ROOT}/bin/cncocr_t

OCR_TYPE="${OCR_TYPE-x86-pthread-x86}"

# Only continue to generate the regression-friendly
# makefile if this is a regression job
[ "$2" = "regression" ] || exit 0

cat <<EOF > "${WORKLOAD_SRC}/Makefile.${OCR_TYPE}"
-include Makefile

OCR_TYPE := ${OCR_TYPE}
EOF

head -n3 < "${WORKLOAD_SRC}/Makefile" >> "${WORKLOAD_SRC}/Makefile.${OCR_TYPE}"

cat <<'EOF' >> "${WORKLOAD_SRC}/Makefile.${OCR_TYPE}"
OCR_CONFIG := $(XSTACK_ROOT)/hll/cnc/resources/cncocr/config/m8w.cfg

CFLAGS := $(filter-out -g,$(CFLAGS))

WORKLOAD_BUILD_ROOT   ?= $(WORKLOAD_SRC)/build
WORKLOAD_BUILD        ?= $(WORKLOAD_BUILD_ROOT)/$(OCR_TYPE)
WORKLOAD_INSTALL_ROOT ?= $(WORKLOAD_SRC)/install
WORKLOAD_INSTALL      ?= $(WORKLOAD_INSTALL_ROOT)/$(OCR_TYPE)

all: build
build: $(WORKLOAD_BUILD)/$(TARGET)
install: $(WORKLOAD_INSTALL)/$(TARGET)

$(WORKLOAD_BUILD)/$(TARGET): $(SRCS) $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(OCR_AR) $(LDFLAGS)
	@echo ">>> BUILD DONE"

$(WORKLOAD_INSTALL)/$(TARGET): $(WORKLOAD_BUILD)/$(TARGET)
	mkdir -p $(@D)
	rm -f $@
	cp $< $@
	cp $(WORKLOAD_SRC)/Makefile.$(OCR_TYPE) $(@D)/Makefile.$(OCR_TYPE)
	@echo ">>> INSTALL DONE"

run: $(WORKLOAD_INSTALL)/$(TARGET)
	OCR_CONFIG=$(OCR_CONFIG) $(RUN_TOOL) $(WORKLOAD_INSTALL)/$(TARGET) $(WORKLOAD_ARGS)

.PHONY: all build install run
EOF

