
OMPSS_INC=include
OMPSS_LIB=lib

CXX = g++

OCR_EXTENSIONS=-DENABLE_EXTENSION_RTITF -DENABLE_EXTENSION_LEGACY
CPPFLAGS=-I$(OMPSS_INC)/nanos6 -I$(OCR_INSTALL)/include -DOCR_TYPE_H=$(OCR_TYPE).h $(OCR_EXTENSIONS)
LDFLAGS=-L$(OCR_INSTALL)/lib -locr_$(OCR_TYPE)
COMMON=-fPIC -O3 -march=native
CXXFLAGS=-std=c++11 $(COMMON)

LIBS=\
 libnanos6.so\
 libnanos6.a

SRC=\
 src/dependences.cc\
 src/main.c\
 src/task.c\
 src/outline.c

OBJ=\
 src/dependences.o\
 src/main.o\
 src/task.o\
 src/outline.o

.PHONY: all clean

all: $(addprefix $(OMPSS_LIB)/, $(LIBS))

clean:
	rm -f $(OMPSS_LIB)/* $(OBJ)

$(OMPSS_LIB)/%.a: $(OBJ)
	$(AR) cru $@ $^

$(OMPSS_LIB)/%.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) $(LDFLAGS) -o $@ $^

