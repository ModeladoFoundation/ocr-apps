#This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

MAKE := make
TESTLOG := run.log
TARGET := mpi_comm1

ifndef OCR_TYPE
  $(error Need to specify OCR_TYPE!)
endif

ifeq ($(OCR_TYPE), x86-mpi)
	FLAGS = OCR_NUM_NODES=8 OCR_NODEFILE=$(HOME)/8nodes
endif

include $(CURDIR)/../Makefile.def

all:
	$(FLAGS) $(MAKE) -f Makefile.$(OCR_TYPE) install
	$(FLAGS) $(MAKE) -f Makefile.$(OCR_TYPE) run > $(TESTLOG)
	@if test `grep -e "rank= 6 newrank= 2 recvbuf= 22" $(TESTLOG) | wc -l` -eq 1; \
		then echo "$(TARGET)-$(OCR_TYPE) PASSED!" $(OUTPUT); \
		else echo "$(TARGET)-$(OCR_TYPE) FAILED!" $(OUTPUT); \
	fi

clean:
	$(MAKE) -f Makefile.$(OCR_TYPE) uninstall clean
	$(RM) $(TESTLOG)

