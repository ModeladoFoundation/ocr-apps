
OMPSS_ROOT=$(PWD)
OMPSS_INC=$(OMPSS_ROOT)/include
OMPSS_LIB=$(OMPSS_ROOT)/lib

CC ?= gcc

CPPFLAGS=-I$(OMPSS_INC)/nanos6 -I$(OCR_INSTALL)/include
LDFLAGS=-L$(OCR_INSTALL)/lib -locr_$(OCR_TYPE)
CFLAGS=-std=gnu99 -fPIC

LIBS=\
 libnanos6.so\
 libnanos6.a

SRC=\
 src/deps.c\
 src/main.c\
 src/task.c

OBJ=$(SRC:.c=.o)

.PHONY: all clean

all: $(addprefix $(OMPSS_LIB)/, $(LIBS))

clean:
	rm -f $(OMPSS_LIB)/* $(OBJ)

$(OMPSS_LIB)/%.a: $(OBJ)
	$(AR) cru $@ $^

$(OMPSS_LIB)/%.so: $(OBJ)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^

