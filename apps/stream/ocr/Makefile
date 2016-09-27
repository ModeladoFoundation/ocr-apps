ifndef OCR_INSTALL
    $(error OCR_INSTALL is not defined)
endif

CC=gcc

INCLUDES=${OCR_INSTALL}/include
CFLAGS=-O2 -g -I$(INCLUDES)

LIBDIR=${OCR_INSTALL}/lib
LFLAGS=-L$(LIBDIR)
LIBS=-locr_x86

DEPS =
OBJ = stream.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

BIN=stream

.PHONY: default clean
default: $(BIN)
all: $(BIN) run

stream: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

run: $(BIN)
ifeq ($(OCR_CONFIG),)
	./$(BIN) -ocr:cfg malloc.cfg
else
	./$(BIN) -ocr:cfg $(OCR_CONFIG)
endif

clean:
	rm -f *.o $(BIN)

