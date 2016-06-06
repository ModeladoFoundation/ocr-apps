#!/bin/bash

include ../Makefile.common


clean:
	@cd autoocr && rm -f newton.c
	@cd autoocr/build && rm -f newton
	@cd hero/build && rm -f *
	@cd raw/build && rm -f newton
	@rm -f default.cfg

########
######## RAW
########
raw:	raw/build/newton
raw/build/newton: raw/newton.c
	@gcc raw/newton.c -o raw/build/newton -lm

check_input_raw:
	@[ "$(E)" != "" -a "$(B)" != "" ] || (make Usage TYPE=raw && exit 1);

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
	make run INPUT=$(CURDIR)/keyword/newton.c
	mv keyword/newton.ocr.c autoocr/newton.c

ocr:	autoocr/newton.c
	@gcc   -g -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread autoocr/newton.c -o autoocr/build/newton -lm


check_input_ocr:
	@[ "$(E)" != "" -a "$(B)" != "" ] || (make Usage TYPE=ocr && exit 1);

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
