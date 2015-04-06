#
# Build simple-<arch>
# Targets (arch): xstg or x86
#
# vim:noexpandtab:
#
# You should only need to change REPO_DIR to your name to get this to compile
#

DENNIS = /home/dennis.obrien/repos/xstack.4.1.0
DONN   = /home/donn/Intel/xstack/gerrit/xstack.cur

REPO_DIR = $(DENNIS)
#
# For building simple.c into xstg executable:
#
XSTG_BUILD = $(REPO_DIR)/ss/bin/xstg-linux-elf
XSTG_BIN = $(XSTG_BUILD)/bin
XSTG_CC = $(XSTG_BIN)/clang
XSTG_AS = $(XSTG_BIN)/as
XSTG_LD = $(XSTG_BIN)/ld

XSTG_LIB = $(XSTG_BUILD)/lib

XSTG_LIBPREFIX = $(XSTG_LIB)/crt0.o $(XSTG_LIB)/init.o
XSTG_LIBSUFFIX = $(XSTG_LIB)/fini.o $(XSTG_LIB)/libgloss.a $(XSTG_LIB)/libc.a

xstg: simple-xstg

simple-xstg : simple-xstg.o
	$(XSTG_LD) -o $@ $(XSTG_LIBPREFIX) $< $(XSTG_LIBSUFFIX)

simple-xstg.S : simple.c
	$(XSTG_CC) -v -O0 -S -o $@ $<

simple-xstg.o : simple-xstg.S
	$(XSTG_AS) -v -o $@ $<
#
# For building simple.c into x86_64 executable:
#
x86_BUILD = $(REPO_DIR)/ss/bin/x86_64-linux-elf
x86_BIN = $(x86_BUILD)/bin
#x86_CC = $(x86_BIN)/clang	# we use the XE llvm/clang compiler
x86_AS = $(x86_BIN)/as
x86_LD = $(x86_BIN)/ld

x86_INC = $(x86_BUILD)/include
x86_LIB = $(x86_BUILD)/lib

# x86_LIBPREFIX = $(x86_LIB)/crt0.o $(x86_LIB)/init.o
# x86_LIBSUFFIX = $(x86_LIB)/fini.o $(x86_LIB)/libgloss.a $(x86_LIB)/libc.a
x86_LIBPREFIX = $(x86_LIB)/crt0.o
x86_LIBSUFFIX = $(x86_LIB)/libc.a

x86: simple-x86

simple-x86 : simple-x86.o
	$(x86_LD) -o $@ $(x86_LIBPREFIX) $< $(x86_LIBSUFFIX)

simple-x86.S : simple.c
	$(XSTG_CC) -v -O0 --target=x86_64  -isystem $(x86_INC) -S -o $@ $<

simple-x86.o : simple-x86.S
	$(x86_AS) -v -o $@ $<

.PHONY: xstg x86

clean:
	$(RM) -rf *.S *.o simple-xstg simple-x86
