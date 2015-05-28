XSTACK=../../..

OCR_INSTALL= ../../../ocr/install/x86
CRLIBM_INC = ../../../apps/libs/x86/include

CC=gcc
CFLAGS=-g -std=c99 -I$(OCR_INSTALL)/include -I$(CRLIBM_INC) -DRAG
LDFLAGS=-L$(OCR_INSTALL)/lib -Xlinker -rpath=$(OCR_INSTALL)/lib -Wl,-Bstatic -locr -Wl,-Bdynamic -lpthread -lm

all:	viewImage    viewCorr    dataImage    viewData \
	viewImageTxt viewCorrTxt dataImageAsm detects_ascii_to_txt

viewImage:	../ocr/src/common.h      viewImage.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o  viewImage viewImage.c ../ocr/src/inputs.c $(LDFLAGS)

viewCorr:	../ocr/src/common.h     viewCorr.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o viewCorr viewCorr.c ../ocr/src/inputs.c $(LDFLAGS)

viewImageTxt:	../ocr/src/common.h     viewImageTxt.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o viewImageTxt viewImageTxt.c ../ocr/src/inputs.c $(LDFLAGS)

viewCorrTxt:	../ocr/src/common.h     viewCorrTxt.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o viewCorrTxt viewCorrTxt.c ../ocr/src/inputs.c $(LDFLAGS)

dataImage:	../ocr/src/common.h      dataImage.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o  dataImage dataImage.c ../ocr/src/inputs.c $(LDFLAGS)

dataImageAsm:	../ocr/src/common.h     dataImageAsm.c ../ocr/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o dataImageAsm dataImageAsm.c ../ocr/src/inputs.c $(LDFLAGS)

viewData:	../datagen-tsml/src/common.h      viewData.c ../datagen-tsml/src/inputs.c Makefile
	$(CC) $(CFLAGS) -I../datagen-tsml/src -o  viewData viewData.c ../datagen-tsml/src/inputs.c $(LDFLAGS)

detects_ascii_to_txt:	detects_ascii_to_txt.c Makefile
	$(CC) $(CFLAGS) -I../ocr/src -o detects_ascii_to_txt detects_ascii_to_txt.c $(LDFLAGS)

clean:
	$(RM) viewImage    viewCorr    dataImage    viewData \
	      viewImageTxt viewCorrTxt dataImageAsm detects_ascii_to_txt
