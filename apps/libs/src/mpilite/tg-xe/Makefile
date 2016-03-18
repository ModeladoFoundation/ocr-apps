# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.
TG_PREFIX     := $(TG_INSTALL)/bin/xstg-linux-elf

AS	= $(TG_PREFIX)-as
ASFLAGS	=

AR	= $(TG_PREFIX)-ar
ARFLAGS	= rcs

#CC	= $(TG_PREFIX)-clang -DOCR -DTG_ARCH -frmd-extensions
CC	= $(TG_PREFIX)-clang -DOCR -DTG_ARCH

CP = cp
RM = rm

CFLAGS	=  -ggdb -g3 -fno-builtin -DHAVE_CONFIG_H -I.

OBJCOPY	= $(TG_PREFIX)-objcopy

SRCS := ../mpilite.c ../mpi_ocr.c../mpi_ocr_messaging.c
OBJS := mpilite.o mpi_ocr.o mpi_ocr_messaging.o
INCLUDES := ../mpi.h ../mpi_ocr_messaging.h ../mpi_ocr.h
LIB := libmpilite.a

# installed files
INSTALL_MPI_H := ${APPS_LIBS_INSTALL}/include/mpi.h
INSTALL_MPI_A := ${APPS_LIBS_INSTALL}/lib/$(LIB)

INC_FLAGS := -I.. -I$(OCR_ROOT)/inc -isystem $(APPS_LIBS_INSTALL)/include

CFLAGS += -std=c99

CFLAGS := -g $(CFLAGS) $(INC_FLAGS)

.PHONY: all
all: install

.PHONY: debug
debug: CFLAGS += -g -O0
debug: install

.PHONY: compile
compile: $(OBJS)

.PHONY: link
link: $(LIB)

%.o: ../%.c Makefile $(INCLUDES)
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -c $< -o $@


$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_MPI_H): ../mpi.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $< $@

$(INSTALL_MPI_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install: $(INSTALL_MPI_H) $(INSTALL_MPI_A)

installold: $(../mpi.h) $(LIB)
	@echo "Installing tg standard files"
	@cp $(../mpi.h) ${APPS_LIBS_ROOT}/include/
	@cp $(LIB) ${APPS_LIBS_ROOT}/lib/

.PHONY: $(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@

.PHONY: run
run:

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	for i in mpi.h ; do rm -f ${APPS_LIBS_ROOT}/include/$$i ; done
	rm -f $(APPS_LIBS_ROOT)/lib/$(LIB)

.PHONY: squeeky
squeeky: clean uninstall
