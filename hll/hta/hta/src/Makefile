include ../Makefile.common

PILLIB = Operation
#SRC = Tuple.c HTA.c Mapping.c Alloc.c Region.c RefCount.c Distribution.c HTA_partial_reduce.c CSR.c HTA_operations.c
SRC = Tuple.c HTA.c Mapping.c Alloc.c Distribution.c CSR.c HTA_operations.c HTA_partial_reduce.c Region.c HTA_collective.c
OBJS = $(SRC:.c=.o) 

CFLAGS = $(DEBUG) $(OPT) -std=c99 -fPIC -DPILHTA -I../include -I$(PIL_INC)


all: $(SHTALIB)

$(SHTALIB): $(OBJS)
	$(CC) -shared -o $(SHTALIB) $(OBJS)

$(PILLIB)_seq.c: $(PILLIB).pil
	$(PIL)/pilc/pilc -o C $< > $@
	cp pil.h ../include/

$(PILLIB)_omp.c: $(PILLIB).pil
	$(PIL)/pilc/pilc -o omp $< > $@
	cp pil.h ../include/

$(PILLIB)_swarm.c: $(PILLIB).pil
	$(PIL)/pilc/pilc -o swarm $< > $(PILLIB).swc
	$(SWARM)/bin/swarm2c -DPIL2SWARM -DPILHTA -I$(PIL_INC) -I$(HTA_INC) -I$(SWARM_INC) -o $@ $(PILLIB).swc 
	cp pil.h ../include/

$(PILLIB)_ocr.c: $(PILLIB).pil
	$(PIL)/pilc/pilc -o ocr $< > $@
	cp pil.h ../include/


pil: $(PILLIB)_omp.c $(PILLIB)_ocr.c # $(PILLIB)_swarm.c

%.o: %.c pil
	$(CC) $(CFLAGS) -I./ -c $<

clean:
	$(RM) -f *.o $(SHTALIB) $(DHTALIB)
	$(RM) -f $(PILLIB)*.c $(PILLIB).swc $(PILLIB)_omp.c $(PILLIB)_ocr.c pil.h pil.swh
