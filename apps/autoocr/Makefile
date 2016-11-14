# top level Makefile for autoocr tests

test:
	ifeq (3.9.0,$(findstring 3.9.0, $(shell $(CLANG) --version | grep 3.9.0)))
	    $(info >   The clang is NOT version 3.5 or 3.6.  The autoocr plugin requires one of these older versions.)
	    $(info >   Please checkout commit #d4fe7d84154520328d59d864b433aaaef028721a in the tg repository.)
	    $(info >   This commit is for llvm/clang 3.6.  You will need to rebuild xe-llvm.)
	    $(error exiting)
	endif

all: test ocr raw

ocr:
	-cd fib && $(MAKE) ocr
	-cd newton-raphson && $(MAKE) ocr
	-cd qsort && $(MAKE) ocr
	-cd uts && $(MAKE) ocr

raw:
	-cd fib && $(MAKE) raw
	-cd newton-raphson && $(MAKE) raw
	-cd qsort && $(MAKE) raw
	-cd uts && $(MAKE) raw

clean:
	-cd fib && $(MAKE) clean
	-cd newton-raphson && $(MAKE) clean
	-cd qsort && $(MAKE) clean
	-cd uts && $(MAKE) clean

run.ocr:
	-cd fib && $(MAKE) run.ocr N=8
	-cd newton-raphson && $(MAKE) run.ocr B=3 E=5
	-cd qsort && $(MAKE) run.ocr
	-cd uts && $(MAKE) run.ocr

run.raw:
	-cd fib && $(MAKE) run.raw N=8
	-cd newton-raphson && $(MAKE) run.raw B=3 E=5
	-cd qsort && $(MAKE) run.raw
	-cd uts && $(MAKE) run.raw

.PHONY: all clean ocr raw run.ocr run.raw
