#!/bin/bash

include ../Makefile.common


clean:
	@rm -rf autoocr
	@rm -rf hero/build
	@rm -rf raw/build
	@rm -f default.cfg


########
######## RAW
########
raw:	raw/build/qsort
raw/build/qsort: raw/qsort.c
	@mkdir -p raw/build
	gcc raw/qsort.c -o raw/build/qsort

run.raw:	raw/build/qsort
	@echo ""
	@echo "Running raw qsort"
	@echo "   n = $N"
	@echo ""
	@raw/build/qsort $N


########
######## keyword / autoocr
########
autoocr/qsort.c:	keyword/qsort.c
	@mkdir -p autoocr
	$(MAKE) run INPUT=$(CURDIR)/keyword/qsort.c OUTPUT=$(CURDIR)/autoocr/qsort.c

ocr:	autoocr/build/qsort
autoocr/build/qsort:	autoocr/qsort.c
	@mkdir -p autoocr/build
	gcc   -g -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread autoocr/qsort.c -o autoocr/build/qsort


run.ocr:	ocr
	@echo ""
	@echo ""
	@echo "Running OCR Legacy qsort with 32 threads, |threads| < |EDTs|"
	@echo "   n = $N"
	@echo ""
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib autoocr/build/qsort $N
	@echo ""
	@echo "Finished"


########
######## HERO
########
qsort.con:      hero/qsort.con.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/qsort.con.c -lm -o hero/build/qsort.con

run.qsort.con:  qsort.con
	@mkdir -p hero/build
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/qsort.con $N

qsort-block:      hero/qsort-block.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/qsort-block.c -lm -o hero/build/qsort-block

run.qsort-block:  qsort-block
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/qsort-block $N
