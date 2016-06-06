# top level Makefile for autoocr tests
clean:
	-cd fib && make clean
	-cd newton-raphson && make clean
	-cd qsort && make clean
	-cd uts && make clean
all:
	-cd fib && make raw
	-cd fib && make ocr
	-cd newton-raphson && make raw
	-cd newton-raphson && make ocr
	-cd qsort && make raw
	-cd qsort && make ocr
	-cd uts && make raw
	-cd uts && make ocr

run.ocr:
	-cd fib && make run.ocr N=8
	-cd newton-raphson && make run.ocr B=3 E=5
	-cd qsort && make run.ocr
	-cd uts && make run.ocr

run.raw:
	-cd fib && make run.raw N=8
	-cd newton-raphson && make run.raw B=3 E=5
	-cd qsort && make run.raw
	-cd uts && make run.raw
