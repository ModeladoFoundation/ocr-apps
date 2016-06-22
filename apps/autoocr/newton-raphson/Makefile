#!/bin/bash

include ../Makefile.common


clean:
	@rm -rf autoocr
	@rm -rf raw/build
	@rm -f default.cfg

########
######## RAW
########
raw:	raw/build/newton
raw/build/newton: raw/newton.c
	@mkdir -p raw/build
	gcc raw/newton.c -o raw/build/newton -lm

check_input_raw:
	@[ "$(E)" != "" -a "$(B)" != "" ] || ($(MAKE) Usage TYPE=raw && exit 1);

Usage:
	@echo ""
	@echo "Run Newton-Raphson example"
	@echo ""
	@echo "Usage: make  ${TYPE} E=<exp> B=<base>"
	@echo ""
	@echo "Example -- Compute cubed root of 15"
	@echo "    make $(TYPE) B=3 E=15"
	@echo ""


run.raw:	raw/build/newton check_input_raw
	@echo ""
	@echo "Running raw newton"
	@echo "   b = $B, e = $E"
	@echo ""
	@raw/build/newton $B $E


########
######## keyword / autoocr
########
autoocr/newton.c:	keyword/newton.c
	@mkdir -p autoocr
	$(MAKE) run INPUT=$(CURDIR)/keyword/newton.c
	mv keyword/newton.ocr.c autoocr/newton.c

ocr:	autoocr/build/newton
autoocr/build/newton: autoocr/newton.c
	@mkdir -p autoocr/build
	gcc   -g -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread autoocr/newton.c -o autoocr/build/newton -lm


check_input_ocr:
	@[ "$(E)" != "" -a "$(B)" != "" ] || ($(MAKE) Usage TYPE=ocr && exit 1);

run.ocr:	ocr check_input_ocr
	@echo ""
	@echo ""
	@echo "Running OCR Legacy newton with 64 threads, |threads| < |EDTs|"
	@echo "   n = $N"
	@echo ""
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib autoocr/build/newton $B $E
	@echo ""
	@echo "Finished"
