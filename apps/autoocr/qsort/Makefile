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
	$(MAKE) run INPUT=$(CURDIR)/keyword/qsort.c
	@mv keyword/qsort.ocr.c autoocr/qsort.c

ocr:	autoocr/build/qsort
autoocr/build/qsort:	autoocr/qsort.c
	@mkdir -p autoocr/build
	gcc   -g -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread autoocr/qsort.c -o autoocr/build/qsort


run.ocr:	ocr
	@echo ""
	@echo ""
	@echo "Running OCR Legacy qsort with 64 threads, |threads| < |EDTs|"
	@echo "   n = $N"
	@echo ""
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib autoocr/build/qsort $N
	@echo ""
	@echo "Finished"


########
######## HERO
########
qsort.con:      hero/qsort.con.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/qsort.con.c -lm -o hero/build/qsort.con

run.qsort.con:  qsort.con
	@mkdir -p hero/build
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/qsort.con $N

qsort-block:      hero/qsort-block.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/qsort-block.c -lm -o hero/build/qsort-block

run.qsort-block:  qsort-block
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/qsort-block $N
