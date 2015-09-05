## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

SUFFIXES = .S .c .lo .o .obj

#LIB_SOURCES = setjmp.S memcpy.S memset.S

AM_CFLAGS = -I$(srcdir)/../..
AM_CCASFLAGS = -I$(srcdir)/../.. $(INCLUDES)

#noinst_LIBRARIES = lib.a
#lib_a_SOURCES = $(LIB_SOURCES)
#lib_a_CFLAGS = $(AM_CFLAGS)
#lib_a_CCASFLAGS = $(AM_CCASFLAGS)
#noinst_DATA =

include $(srcdir)/../../../../../Makefile.shared

all-local: crt0.o crt0t.o crtend.o

crt0.o: crt0.c
	$(CC) -DOCR_INIT -Os -c $< -o $@

crt0t.o: crt0.c
	$(CC) -DUSE_STACK -Os -c $< -o $@

crtend.o: crtend.c
	$(CC) -Os -c $< -o $@

ACLOCAL_AMFLAGS = -I ../../../../.. -I ../../../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
