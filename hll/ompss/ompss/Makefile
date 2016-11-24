
CXX = g++
AR=gcc-ar

DESTDIR ?= $(APPS_LIBS_INSTALL_ROOT)/$(OCR_TYPE)

#DEBUG=-Wall -Wextra -Wno-unused-parameter -O0 -g3
RELEASE=-DRELEASE -O3 -march=native -fno-exceptions -fno-rtti -fvisibility-inlines-hidden -flto

OCR_EXTENSIONS=-DENABLE_EXTENSION_RTITF -DENABLE_EXTENSION_LEGACY
CPPFLAGS=\
 -Isrc\
 -Iinclude/nanos6\
 -I$(OCR_INSTALL)/include\
 -DOCR_TYPE_H=$(OCR_TYPE).h\
 $(OCR_EXTENSIONS)

CXXFLAGS=\
 -std=c++11\
 $(DEBUG)\
 $(RELEASE)

LDFLAGS=\
 -fuse-linker-plugin\
 -shared\
 -Wl,--discard-all\
 -Wl,-rpath,$(OCR_INSTALL)/lib\
 -L$(OCR_INSTALL)/lib\
 -locr_$(OCR_TYPE)

ifeq ($(EXTRAE),yes)
CPPFLAGS += -I$(EXTRAE_HOME)/include -D_ENABLE_EXTRAE\
LDFLAGS :=\
 -L$(EXTRAE_HOME)/lib\
 -lpttrace\
 $(LDFLAGS)
endif

LIBS=$(addprefix lib/,\
 libnanos6.so\
 libnanos6.a\
)

OBJ=\
 src/dependences\
 src/main\
 src/task\
 src/outline

.PHONY: all install clean

all: $(LIBS)

install: $(LIBS)
	@install -C -d $(DESTDIR)/include $(DESTDIR)/lib
	@install -C -m 644 include/nanos6/nanos6_rt_interface.h -t $(DESTDIR)/include
	@install -C -m 755 lib/*.so -t $(DESTDIR)/lib
	@install -C -m 644 lib/*.a  -t $(DESTDIR)/lib

clean:
	rm -f lib/* src/*.o

%.so.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $^ -o $@

%.a.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@

lib/%.a: $(addsuffix .a.o,$(OBJ))
	$(AR) cru $@ $^

lib/%.so: $(addsuffix .so.o,$(OBJ))
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

