###  UTS - Unbalanced Tree Search  ###

include ../Makefile.common

RAW_SRCS = raw/uts.c
OCR_SRCS = autoocr/uts.c
RAW_INCS = raw/uts.h
OCR_INCS = keyword/uts.h

# ------------------------------------- #
# Set Random Number Generator sources:
# ------------------------------------- #

# Set the default RNG
ifndef RNG
RNG=BRG
endif

ifeq ($(RNG), Devine)
RNG_SRCS = rng/devine_sha1.c
RNG_INCS = rng/devine_sha1.h
RNG_DEFS = -DDEVINE_RNG
endif
ifeq ($(RNG), BRG)
RNG_SRCS = rng/brg_sha1.c
RNG_INCS = rng/brg_sha1.h
RNG_DEFS = -DBRG_RNG
endif
ifeq ($(RNG), ALFG)
RNG_SRCS = rng/alfg.c
RNG_INCS = rng/alfg.h
RNG_DEFS = -DUTS_ALFG
endif

CC=gcc
UTS_CFLAGS=-O3 -std=c99
OCR_CFLAGS=-O3 -std=c99 -DOCR_ENABLE_EDT_NAMING -DOCR_ASSERT -DENABLE_EXTENSION_AFFINITY -I${REL}/install/include -L${REL}/install/lib -locr_x86 -lpthread
LDFLAGS=-lm

all:	raw ocr

uts:	all

clean:
	@rm -rf autoocr
	@rm -rf raw/build
	@rm -f default.cfg

########
######## RAW
########
raw:	raw/build/uts
raw/build/uts:	$(RAW_INCS) $(RAW_SRCS) raw/$(RNG_INCS) raw/$(RNG_SRCS)
	@mkdir -p raw/build
	$(CC) $(UTS_CFLAGS) $(RNG_DEFS) -o raw/build/uts $+ $(LDFLAGS)

run.raw:	raw
	@echo "==================== UTS ORIGINAL ========================="
	@echo "(T6) Hybrid ------------------ Tree size = 3443, tree depth  = 42, num leaves = 2575 (74.79%)"
	time raw/build/uts -g 1 -t 2 -a 0 -d 8 -b 4 -r 1 -q 0.234375 -m 4

run_long_orig: uts_orig
	@echo "(T4) Hybrid ------------------ Tree size = 4132453, tree depth = 134, num leaves = 3108986 (75.23%)"
	time ./uts_orig -g 1 -t 2 -a 0 -d 16 -b 6 -r 1 -q 0.234375 -m 4
	@echo "(T1) Geometric [fixed] ------- Tree size = 4130071, tree depth = 10, num leaves = 3305118 (80.03%)"
	time ./uts_orig -t 1 -a 3 -d 10 -b 4 -r 19
	@echo "(T5) Geometric [linear dec.] - Tree size = 4147582, tree depth = 20, num leaves = 2181318 (52.59%)"
	time ./uts_orig -t 1 -a 0 -d 20 -b 4 -r 34
	@echo "(T2) Geometric [cyclic] ------ Tree size = 4117769, tree depth = 81, num leaves = 2342762 (56.89%)"
	time ./uts_orig -t 1 -a 2 -d 16 -b 6 -r 502
	@echo "(T3) Binomial ---------------- Tree size = 4112897, tree depth = 1572, num leaves = 3599034 (87.51%)"
	time ./uts_orig -t 0 -b 2000 -q 0.124875 -m 8 -r 42

########
######## keyword / autoocr
########
$(OCR_SRCS):	keyword/uts.c
	@mkdir -p autoocr
	$(MAKE) run INPUT=$(CURDIR)/keyword/uts.c
	mv keyword/uts.ocr.c autoocr/uts.c

ocr:	autoocr/build/uts
autoocr/build/uts: $(INCS) $(OCR_SRCS) keyword/$(RNG_INCS) keyword/$(RNG_SRCS)
	@mkdir -p autoocr/build
	$(CC) -Ikeyword -o autoocr/build/uts $(OCR_CFLAGS) $(RNG_DEFS) $+ $(LDFLAGS)


run.ocr:	ocr
	@echo "===================== UTS OCR ============================"
	@echo "(T6) Hybrid ------------------ Tree size = 3443, tree depth  = 42, num leaves = 2575 (74.79%)"
	@rm -f default.cfg
	@${REL}/scripts/Configs/config-generator.py --threads 64
	OCR_CONFIG=default.cfg LD_LIBRARY_PATH=${REL}/install/lib time autoocr/build/uts -g 1 -t 2 -a 0 -d 8 -b 4 -r 1 -q 0.234375 -m 4
