CFLAGS=-g -I${OCR_INSTALL}/include
LDFLAGS=-L${OCR_INSTALL}/lib -locr
#TARGET=finishDp
#TARGET=edtDp
TARGET=edtDpReduction

#TARGET_PERF=edtDp_perf
#TARGET_PERF=edtDpUnbalanced_perf
TARGET_PERF=edtDpReduction_perf

TARGET_OMP=ompDp

default: $(TARGET) $(TARGET_PERF)

.PHONY: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $^

$(TARGET_PERF): $(TARGET_PERF).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $^

$(TARGET_OMP): $(TARGET_OMP).c
	gcc -fopenmp -o$@ $^

run: $(TARGET)
	./$(TARGET) -ocr:cfg $(OCR_CONFIG)

perf: $(TARGET_PERF)
	./$(TARGET_PERF) -ocr:cfg $(OCR_CONFIG)

omp: $(TARGET_OMP)
	./$(TARGET_OMP)

clean:
	rm -f edtDp edtDp_perf edtDpUnbalanced_perf edtDpReduction edtDpReduction_perf ompDp finishDp *.out core.*
