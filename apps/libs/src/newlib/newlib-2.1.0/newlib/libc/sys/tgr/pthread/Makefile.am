## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../include -I$(srcdir)/machine/$(machine_dir) $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS) -I$(srcdir)/..

LIB_SOURCES= \
	attr.c barrier.c condvar.c \
	events.c getcpuclockid.c getreent.c join.c lockfile.c manager.c \
	oldsemaphore.c ptclock_gettime.c ptclock_settime.c \
	rwlock.c semaphore.c \
	spinlock.c #\
	#timer_create.c timer_getoverr.c timer_routines.c \
	#timer_delete.c timer_gettime.c timer_settime.c

# Because of how libtool moves objects around, these files must be built last.
LIBADD_OBJS = \
	$(lpfx)mutex.$(oext) $(lpfx)specific.$(oext) \
	$(lpfx)pthread.$(oext) $(lpfx)cancel.$(oext)

SUBDIRS = .

tooldir = $(tooldir)
toollibdir = $(top_toollibdir)

toollib_LIBRARIES = libpthread.a
libpthread_a_SOURCES = $(LIB_SOURCES)
libpthread_a_LIBADD = $(LIBADD_OBJS)
libpthread_a_CFLAGS = $(AM_CFLAGS)
## libpthread_a_DEPENDENCIES = defs.h crti.S
libpthread_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(COMPILE)

include $(srcdir)/../../../../Makefile.shared

doc:

AM_CFLAGS = -D_XOPEN_SOURCE=600 -D_GNU_SOURCE=1
ACLOCAL_AMFLAGS = -I ../../../.. -I ../../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## PTHREAD_HEADERS=pthread.h semaphore.h

CFLAGS_pt_initfini = -g0 -fPIC -fno-inline-functions
CFLAGS_noweak=-D__NO_WEAK_PTHREAD_ALIASES
CFLAGS_cancel=-D__NO_WEAK_PTHREAD_ALIASES -D_RPC_THREAD_SAFE_
CFLAGS_pthread=-D__NO_WEAK_PTHREAD_ALIASES -DHAVE_Z_NODELETE

pt-initfini.s: $(srcdir)/pt-initfini.c
	$(CC) -S $(CFLAGS_pt_initfini) -finhibit-size-directive \
		$(patsubst -f%,-fno-%,$(exceptions)) -o $@

# We only have one kind of startup code files.  Static binaries and
# shared libraries are build using the PIC version.
crti.S: pt-initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_PROLOG_BEGINS/,/@_.*_PROLOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@

defs.h: pt-initfini.s
	sed -n -e '/@TESTS_BEGIN/,/@TESTS_END/p' $< | \
		$(AWK) -f defs.awk > $@

crti.o: crti.S defs.h
	$(CC) -g0 $(ASFLAGS) -o $@

$(lpfx)mutex.$(oext): $(srcdir)/mutex.c
	$(LIB_COMPILE) $(CFLAGS_noweak) -c $< -o $@

$(lpfx)specific.$(oext): $(srcdir)/specific.c
	$(LIB_COMPILE) $(CFLAGS_noweak) -c $< -o $@

$(lpfx)pthread.$(oext): $(srcdir)/pthread.c
	$(LIB_COMPILE) $(CFLAGS_pthread) -c $< -o $@

$(lpfx)ptfork.$(oext): $(srcdir)/ptfork.c
	$(LIB_COMPILE) $(CFLAGS_noweak) -c $< -o $@

$(lpfx)cancel.$(oext): $(srcdir)/cancel.c
	$(LIB_COMPILE) $(CFLAGS_cancel) -c $< -o $@

