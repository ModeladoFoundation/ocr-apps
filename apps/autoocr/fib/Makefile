#!/bin/bash

export REL=../../../../ocr/ocr
export TG=../../../../tg/tg


clean:
	@cd autoocr && rm -f fib.c
	@cd autoocr/build && rm -f fib
	@cd hero/build && rm -fr *
	@cd raw/build && rm -f fib
	@rm -f default.cfg


########
######## RAW
########
raw:	raw/build/fib
raw/build/fib: raw/fib.c
	@gcc raw/fib.c -o raw/build/fib

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
	cd $(TG)/autoocr && make run INPUT=$(CURDIR)/keyword/fib.c
	mv keyword/fib.ocr.c autoocr/fib.c

ocr:	autoocr/fib.c
	@gcc   -g -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread autoocr/fib.c -o autoocr/build/fib

run.ocr:	ocr check_input
	@echo ""
	@echo ""
	@echo "Running OCR Legacy fib with 64 threads, |threads| < |EDTs|"
	@echo "   n = $N"
	@echo ""
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib autoocr/build/fib $N
	@echo ""
	@echo "Finished"

########
######## HERO
########

fib.rag:      hero/fib.rag.c
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/fib.rag.c -lm -o hero/build/fib.rag

run.fib.rag:  fib.rag check_input
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/fib.rag $N

fib.con:      hero/fib.con.c
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/fib.con.c -lm -o hero/build/fib.con

run.fib.con:  fib.con check_input
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/fib.con $N

fib-block:      hero/fib-block.c
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/fib-block.c -lm -o hero/build/fib-block

run.fib-block:  fib-block check_input
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/fib-block $N

fib-contn:      hero/fib-contn.c
	gcc   -Wall -g -Werror -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread hero/fib-contn.c -lm -o hero/build/fib-contn

run.fib-contn:  fib-contn check_input
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	@OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib hero/build/fib-contn $N
