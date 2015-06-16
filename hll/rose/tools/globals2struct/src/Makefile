TOOL = globals2struct

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
