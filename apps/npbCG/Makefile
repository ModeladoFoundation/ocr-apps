APP     = cg_ocr
OBJECTS = cg_ocr.o main_edt.o util_ocr.o makea_ocr.o cg_graph.o cg_edt.o

CFLAGS  = -g -O3 -I$(OCR_INSTALL)/include
LDFLAGS = -lm -L$(OCR_INSTALL)/lib -locr

.PHONY: all

all: $(APP)

$(APP): $(OBJECTS)

$(OBJECTS): $(APP).h la_ocr.h

.PHONY: clean

clean:
	$(RM) *.o $(APP)
