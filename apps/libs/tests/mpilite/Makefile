# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

# Top-level makefile

RESULTS_LOG=$(CURDIR)/results.log

MPITESTS = mpi_comm_split mpi_comm1 mpi_comm2 mpi_barrier

.PHONY: $(MPITESTS)

all: clear $(MPITESTS) verify

$(MPITESTS):
	make -C $@ OCR_TYPE=x86 RESULTS_LOG=$(RESULTS_LOG)
	make -C $@ OCR_TYPE=x86-mpi RESULTS_LOG=$(RESULTS_LOG)


verify:
	echo $(shell ./totals.sh $(RESULTS_LOG))

clear:
	$(RM) $(RESULTS_LOG)


CLEANDIRS = $(MPITESTS:%=clean-%)

clean: $(CLEANDIRS)
$(CLEANDIRS): clear
	@$(MAKE) -C $(@:clean-%=%) OCR_TYPE=x86 clean
	@$(MAKE) -C $(@:clean-%=%) OCR_TYPE=x86-mpi clean
	$(RM) $(RESULTS_LOG)
