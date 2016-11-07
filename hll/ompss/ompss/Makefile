
OMPSS_INC=include
OMPSS_LIB=lib

CXX = g++

#DEBUG=-Wall -Wextra -Wno-unused-parameter -O0 -g3 #-fsanitize=address
RELEASE=-O3 -march=native

OCR_EXTENSIONS=-DENABLE_EXTENSION_RTITF -DENABLE_EXTENSION_LEGACY

CPPFLAGS=\
 -I$(OMPSS_INC)/nanos6\
 -I$(OCR_INSTALL)/include\
 -DOCR_TYPE_H=$(OCR_TYPE).h\
 $(OCR_EXTENSIONS)

CXXFLAGS=\
 -std=c++11\
 -fno-rtti\
 -fno-exceptions\
 -fvisibility-inlines-hidden\
 $(DEBUG)\
 $(RELEASE)

LDFLAGS=\
 -shared\
 -Wl,--discard-all\
 -Wl,-rpath,$(OCR_INSTALL)/lib\
 -L$(OCR_INSTALL)/lib\
 -locr_$(OCR_TYPE)

LIBS=\
 libnanos6.so\
 libnanos6.a

OBJ=\
 src/dependences\
 src/main\
 src/task\
 src/outline

.PHONY: all clean

all: $(addprefix $(OMPSS_LIB)/, $(LIBS))

clean:
	rm -f $(OMPSS_LIB)/* $(OBJ)

%.so.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $^ -o $@

%.a.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@

$(OMPSS_LIB)/%.a: $(addsuffix .a.o,$(OBJ))
	$(AR) cru $@ $^

$(OMPSS_LIB)/%.so: $(addsuffix .so.o,$(OBJ))
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

