
CXX = g++
AR=gcc-ar

DESTDIR ?= $(APPS_LIBS_INSTALL_ROOT)/$(OCR_TYPE)

#DEBUG=-Wall -Wextra -Wno-unused-parameter -O0 -g3
RELEASE=-DRELEASE -O3 -march=native -fno-exceptions -fno-rtti -fvisibility-inlines-hidden -flto

OCR_EXTENSIONS=-DENABLE_EXTENSION_RTITF -DENABLE_EXTENSION_LEGACY
CPPFLAGS=\
 -Isrc\
 -Iinclude\
 -I$(OCR_INSTALL)/include\
 -DOCR_TYPE_H=$(OCR_TYPE).h\
 $(OCR_EXTENSIONS)

CXXFLAGS=\
 -std=c++11\
 $(DEBUG)\
 $(RELEASE)

LDFLAGS=\
 -fuse-linker-plugin\
 -Wl,--discard-all\
 -Wl,-rpath,$(OCR_INSTALL)/lib\
 -L$(OCR_INSTALL)/lib\
 -locr_$(OCR_TYPE)\
 -Wl,-rpath,$(abspath lib)

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
 libompss-ocr.so\
)

COMMON_OBJ=\
 obj/dependences\
 obj/task\
 obj/outline\
 obj/ocr_interface


INIT_OBJ=\
 obj/main

.PHONY: all install clean

all: $(LIBS)

install: $(LIBS)
	@install -C -d $(DESTDIR)/include $(DESTDIR)/lib
	@install -C -m 644 include/nanos6/nanos6_rt_interface.h -t $(DESTDIR)/include
	@install -C -m 755 lib/*.so -t $(DESTDIR)/lib
	@install -C -m 644 lib/*.a  -t $(DESTDIR)/lib

clean:
	rm -f lib/* obj/*.o include/ompss_ocr_interface.h

include/ompss_ocr_interface.h: include/ompss_ocr_interface.h.template obj/gen_interface
	@obj/gen_interface > sedscript
	@sed include/ompss_ocr_interface.h.template -f sedscript > $@
	@rm sedscript

obj/%.so.o: src/%.cc include/ompss_ocr_interface.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@

obj/%.a.o: src/%.cc include/ompss_ocr_interface.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

lib/libompss-ocr.so: $(addsuffix .so.o,$(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) $^ -shared $(LDFLAGS) -o $@

lib/libnanos6.so: $(addsuffix .so.o,$(COMMON_OBJ) $(INIT_OBJ))
	$(CXX) $(CXXFLAGS) $^ -shared $(LDFLAGS) -o $@

lib/libnanos6.a: $(addsuffix .a.o,$(COMMON_OBJ) $(INIT_OBJ))
	$(AR) cru $@ $^

obj/gen_interface: src/gen_interface.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -fPIC -o $@
