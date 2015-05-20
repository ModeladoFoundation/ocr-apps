#
# Makefile for compiling the OCR libc support using the platform compiler
#
LIB_SRCS = ocr-scaffold.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

TEST_LIB = libocr-scaffold.a
OCR_CLIB = ../../build-x86/libocr-glibc.a
OCR_CXXLIB = ../../build-x86/libocr-glibcxx.a

INCLUDES = -I../../include
CFLAGS += $(INCLUDES) -std=c99 -g
CXXFLAGS += -g
AR_FLAGS = qs

APP_SRCS = c.c
APP_OBJS = $(APP_SRCS:.c=.o)

CXXAPP_SRCS = cplus.cc
CXXAPP_OBJS = $(CXXAPP_SRCS:.cc=.o)

APPS = app cxxapp

all : $(TEST_LIB) $(APPS)

app : $(OCR_CLIB) $(TEST_LIB)
	$(CC) -static -o $@ c.c $^

cxxapp : $(OCR_CXXLIB) $(TEST_LIB)
	$(CXX) -static -o $@ cplus.cc $^

$(TEST_LIB) : $(LIB_OBJS)
	rm -f $@
	$(AR) $(AR_FLAGS) $@ $^

clean :
	rm -rf $(APP_OBJS) $(APPS) $(CXXAPP_OBJS) $(LIB_OBJS) $(TEST_LIB)

.PHONY: clean
