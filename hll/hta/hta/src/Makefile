include ../Makefile.common

PIL_SRC += Alloc.c HTA_collective.c
ifeq ($(FOR_CNC), 1)
PIL_SRC += $(PILLIB)_omp.c
endif
MPI_SRC = Alloc_mpi.c
SRC = Tuple.c HTA.c Mapping.c Distribution.c CSR.c HTA_operations.c HTA_partial_reduce.c Region.c HTA_partition.c Config.c \
 HTAblas/dgemm.c Comm.c
PIL_OBJS = $(PIL_SRC:.c=.o)
MPI_OBJS = $(MPI_SRC:.c=.o)

OP_OMP = $(PILLIB)_omp.c
OP_SWARM = $(PILLIB)_swarm.c
OP_OCR = $(PILLIB)_ocr.c

CFLAGS = $(DEBUG) $(OPT) -std=c99 -fPIC -I../include -I$(PIL_INC)

ifeq ($(PROFILE), 1)
    CFLAGS += -L$(PROFILE_LIB) $(LPROFILE) -I$(PROFILE_INC)
endif

ifeq ($(FOR_CNC), 1)
CFLAGS += -DFOR_CNC -DPIL2C -DPILHTA
endif

ifeq ($(SPMD_PIL), 1)
    PILLIB = Operation_spmd
    HTA_LIB = $(HTA_SPMD_PIL_LIB)
    OBJS = $(SRC:.c=-spmd.o)
    CFLAGS += -DSPMD_PIL
    SWARMCFLAG = -DSPMD_PIL
    PIL_OP = $(PILLIB).pil
    PIL_MODE_OMP = -m spmd
    PIL_MODE_SWARM = -m sharedspmd
    PIL_MODE_OCR = -m sharedspmd
else
    PILLIB = Operation
    HTA_LIB = $(HTA_PIL_LIB)
    OBJS = $(SRC:.c=.o)
    PIL_OP = $(PILLIB).pil
    PIL_MODE_OMP =
    PIL_MODE_SWARM =
    PIL_MODE_OCR =
endif

.NOTPARALLEL: pil_swarm pil_omp pil_ocr $(HTA_MPI_LIB) $(HTA_LIB)
.PHONY: pil_swarm pil_omp pil_ocr all debug clean
all: $(HTA_LIB) $(HTA_MPI_LIB)
	@echo "================================================================================"
	@echo "The HTA library has been compiled."
	@echo "Location   = $(HTA_LIB)"
	@echo "SPMD_PIL   = $(SPMD_PIL)"
	@echo "DEBUG      = $(DEBUG)"
	@echo "OPT        = $(OPT)"
	@echo "TRACING    = $(TRACING)"
	@echo "PROFILE    = $(PROFILE)"
	@echo "================================================================================"

debug: OPT = -O0
debug: DEBUG = -DDEBUG -g
debug: $(HTA_LIB) $(HTA_MPI_LIB)

$(HTA_LIB): $(OBJS) $(PIL_OBJS)
	$(CC) -shared -o $@ $(OBJS) $(PIL_OBJS)

$(HTA_MPI_LIB): $(OBJS) $(MPI_OBJS)
	$(CC) -shared -o $@ $(OBJS) $(MPI_OBJS)

#======================================
# PIL Generated Code
#======================================
$(PILLIB)_seq.c: $(PIL_OP)
	$(PIL)/pilc/pilc -o C $< > $@

$(PILLIB)_omp.c: $(PIL_OP)
	$(PIL)/pilc/pilc $(PIL_MODE_OMP) -o omp $< > $@

$(PILLIB).swc: $(PIL_OP)
	$(PIL)/pilc/pilc $(PIL_MODE_SWARM) -o swarm $< > $(PILLIB).swc

$(PILLIB)_swarm.c: $(PILLIB).swc
	$(SWARM)/bin/swarm2c $(SWARMCFLAG) -DPIL2SWARM $(DTRACING) -DPILHTA -I./ -I$(PIL_INC) -I$(HTA_INC) -I$(SWARM_INC) -I$(HOME)/install/include -o $@ $(PILLIB).swc

$(PILLIB)_ocr.c: $(PIL_OP)
	$(PIL)/pilc/pilc $(PIL_MODE_OCR) -o ocr $< > $@

pil_swarm: $(OP_SWARM)
pil_omp: $(OP_OMP)
pil_ocr: $(OP_OCR)
pil: pil_ocr pil_omp
#pil: pil_omp pil_swarm
#pil: $(OP_SWARM)
	cp pil.h ../include/
	touch pil

%.o: %.c pil
	$(CC) $(CFLAGS) -c $< -o $@

%-spmd.o: %.c pil
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(HTA_PIL_LIB) $(HTA_SPMD_PIL_LIB) $(HTA_MPI_LIB) HTAblas/*.o
	$(RM) *_swarm.c *.swc *_omp.c *_ocr.c pil.h pil.swh
	$(RM) *_swarm.c *.swc *_omp.c *_ocr.c pil.h pil.swh
	$(RM) ../include/pil.h
	$(RM) pil
