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

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =

all-local: $(CRT)

crt0.o: machine/crt0.o
	rm -f $@
	ln machine/$@ $@ >/dev/null 2>/dev/null || cp machine/$@ $@

crt0t.o: machine/crt0t.o
	rm -f $@
	ln machine/$@ $@ >/dev/null 2>/dev/null || cp machine/$@ $@

crtend.o: machine/crtend.o
	rm -f $@
	ln machine/$@ $@ >/dev/null 2>/dev/null || cp machine/$@ $@

$(CRT:%=machine/%): ; @true

AM_CFLAGS = -I $(srcdir)/../../stdio
ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
