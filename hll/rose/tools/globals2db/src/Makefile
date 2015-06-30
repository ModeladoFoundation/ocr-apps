# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


TOOL = globals2db

include $(CURDIR)/../../Makefile.def

all: $(TOOL)

bindir:
	 @mkdir -p ../bin

$(TOOL): $(TOOL).cpp bindir
	$(CXX) -g -O2 $(TOOL_INC) $< -o $@ $(TOOL_LNK)
	mv $(TOOL) ../bin

clean:
	rm -f ../bin/$(TOOL)
	rm -f *~
