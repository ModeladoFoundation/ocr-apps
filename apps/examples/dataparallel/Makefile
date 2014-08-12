CFLAGS=-g -I${OCR_INSTALL}/include
LDFLAGS=-L${OCR_INSTALL}/lib -locr
#TARGET=finishDp
TARGET=edtDp

.PHONY: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $^ 

run: $(TARGET)
	./$(TARGET) -ocr:cfg $(OCR_CONFIG)

clean:
	rm -f $(TARGET) core.*
