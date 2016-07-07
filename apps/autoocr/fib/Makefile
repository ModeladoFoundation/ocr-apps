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
raw:	raw/build/fib
raw/build/fib: raw/fib.c
	@mkdir -p raw/build
	gcc raw/fib.c -o raw/build/fib

check_input:
	@[ "$(N)" != "" ] || (echo "usage: make <> N=<value>!" && exit 1)

run.raw:	raw/build/fib check_input
	@echo ""
	@echo "Running raw fib"
	@echo "   n = $N"
	@echo ""
	@raw/build/fib $N


########
######## keyword / autoocr
########
autoocr/fib.c:	keyword/fib.c
	@mkdir -p autoocr
	$(MAKE) run INPUT=$(CURDIR)/keyword/fib.c OUTPUT=$(CURDIR)/autoocr/fib.c

ocr:	autoocr/build/fib
autoocr/build/fib: autoocr/fib.c
	@mkdir -p autoocr/build
	gcc   -g -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread autoocr/fib.c -o autoocr/build/fib

run.ocr:	ocr check_input
	@echo ""
	@echo ""
	@echo "Running OCR Legacy fib with 32 threads, |threads| < |EDTs|"
	@echo "   n = $N"
	@echo ""
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib autoocr/build/fib $N
	@echo ""
	@echo "Finished"

########
######## HERO
########
fib.rag:      hero/fib.rag.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/fib.rag.c -lm -o hero/build/fib.rag

run.fib.rag:  fib.rag check_input
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/fib.rag $N

fib.con:      hero/fib.con.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/fib.con.c -lm -o hero/build/fib.con

run.fib.con:  fib.con check_input
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/fib.con $N

fib-block:      hero/fib-block.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/fib-block.c -lm -o hero/build/fib-block

run.fib-block:  fib-block check_input
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/fib-block $N

fib-contn:      hero/fib-contn.c
	@mkdir -p hero/build
	gcc   -Wall -g -Werror -DOCR_TYPE_H=x86.h -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${OCR_ROOT}/install/include -L${OCR_ROOT}/install/lib -locr_x86 -lpthread hero/fib-contn.c -lm -o hero/build/fib-contn

run.fib-contn:  fib-contn check_input
	@rm -f default.cfg
	@${OCR_ROOT}/scripts/Configs/config-generator.py --threads 32
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${OCR_ROOT}/install/lib hero/build/fib-contn $N
