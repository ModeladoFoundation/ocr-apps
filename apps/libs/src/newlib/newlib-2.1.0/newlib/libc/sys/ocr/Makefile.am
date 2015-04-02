## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/include $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

SUBDIRS = machine

GENERAL_SOURCES = \
	environ.c \
	errno.c  \
	_exit.c \
	fini.c   \
	gettod.c \
	init.c \
	inode.c \
	io.c \
	isatty.c \
	malloc.c \
	mallocr.c \
    ocr_shim.c \
	process.c \
	times.c \
	wait.c

CRT = crt0.o

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
#lib_a_LIBADD = $(ADD_OBJS)
#lib_a_DEPENDENCIES = $(ADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =

#lib.a:  $(lib_a_OBJECTS)
#	rm -f $@
#	$(AR) $(AR_FLAGS) $@ *.o
#	$(RANLIB) $@

#objectlist.awk.in: $(noinst_LTLIBRARIES) $(SUBLIBS)
#	-rm -f objectlist.awk.in
#	for i in `ls *.lo` ; \
#	  do \
#	    echo $$i `pwd`/$$i >> objectlist.awk.in ; \
#	done
#	for j in $(SUBLIBS) ; \
#	  do \
#	    export i=`echo $$j | sed -e 's,\(.*\)\/[^\/]*$$,\1,'`; \
#	    if test $$i != "." && test -f $$i/objectlist.awk.in; then \
#	        cat $$i/objectlist.awk.in >> objectlist.awk.in ; \
#	    fi; \
#	done

all-local: $(CRT)

crt0.o: machine/crt0.o
	rm -f $@
	ln machine/crt0.o $@ >/dev/null 2>/dev/null || cp machine/crt0.o $@

machine/crt0.o: ; @true

AM_CFLAGS = -I $(srcdir)/../../stdio
ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
