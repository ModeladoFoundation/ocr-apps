## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

SUBDIRS = $(machine_dir) .

noinst_DATA = $(CRT)

crt0.o: $(machine_dir)/crt0.o
	rm -f $@
	ln $(machine_dir)/$@ $@ >/dev/null 2>/dev/null || cp $(machine_dir)/$@ $@

crt0t.o: $(machine_dir)/crt0t.o
	rm -f $@
	ln $(machine_dir)/$@ $@ >/dev/null 2>/dev/null || cp $(machine_dir)/$@ $@

crtend.o: $(machine_dir)/crtend.o
	rm -f $@
	ln $(machine_dir)/$@ $@ >/dev/null 2>/dev/null || cp $(machine_dir)/$@ $@

$(CRT:%=$(machine_dir)/%): ; @true

CLEANFILES = $(CRT)

doc:

ACLOCAL_AMFLAGS = -I ../../../.. -I ../../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
