# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


TOOL1 = globals2db
TOOL2 = globals2struct

include $(CURDIR)/../../Makefile.def

all: $(TOOL1) $(TOOL2)

bindir:
	 @mkdir -p ../bin

$(TOOL1): $(TOOL1).cpp bindir
	$(CXX) -g -O2 -D__FFWD_DB_ $(TOOL_INC) $< -o $@ $(TOOL_LNK)
	mv $(TOOL1) ../bin

$(TOOL2): $(TOOL1).cpp bindir
	$(CXX) -g -O2 -D__FFWD_STRUCT_ $(TOOL_INC) $< -o $@ $(TOOL_LNK)
	mv $(TOOL2) ../bin

clean:
	rm -f ../bin/$(TOOL1)  ../bin/$(TOOL2)
	rm -f *~
