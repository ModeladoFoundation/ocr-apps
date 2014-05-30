include ../Makefile.common

SRCS = Alloc_test.c \
       Accessor_test.c \
       Debug_test.c \
       Tuple_test.c \
       Tuple_iterator_test.c\
       Mapping_test.c \
       HTA_init_test.c \
       HTA_map1_test.c \
       HTA_map2_test.c \
       HTA_map2_irregular.c \
       HTA_reduce_test.c \
       HTA_reduce_test_2.c \
       HTA_map_to_scalar_test.c \
       HTA_map2_to_scalar_test.c \
       HTA_scalar_types_double.c \
       HTA_scalar_types_uint64.c \
       HTA_matmul.c \
       HTA_tile_to_hta.c  \
       HTA_partial_reduction_test.c \
       HTA_sparse_test.c \
       HTA_reduce_h2.c \
       CSR_to_rcs.c \
       HTA_accessor_test.c \
       Region_test.c \
       HTA_nd_rank_offset.c\
       HTA_circshift_test.c
#       RefCount_test.c \
#       HTA_SUMMA.c

#CFLAGS = $(DEBUG) -I$(HTA_INC) -I$(PIL_INC) -DPIL2C -DPILHTA
CFLAGS = -std=c99 $(DEBUG) -I$(HTA_INC) -I$(PIL_INC) -DPILHTA
LDFLAGS = -L$(HTA_LIB) -lhta -lm
OP_SEQC = $(HTA_SRC)/Operation_seq.c
OP_OMP = $(HTA_SRC)/Operation_omp.c
OP_SWARM = $(HTA_SRC)/Operation_swarm.c
OP_OCR = $(HTA_SRC)/Operation_ocr.c
SWARM_FLAGS = -DPIL2SWARM -fopenmp
SWARM_INC = -I$(SWARM)/include
SWARM_LINK = -lpthread $(SWARM)/lib/libswarm.a -lrt -lm
#TARGETS = $(SRCS:.c=) $(SRCS:.c=_omp) $(SRCS:.c=_swarm) $(SRCS:.c=_ocr)
TARGETS = $(SRCS:.c=_omp) $(SRCS:.c=_ocr) #$(SRCS:.c=_swarm)

all: $(TARGETS)
.PHONY: test

$(SHTALIB):
	$(MAKE) -C $(HTA)/src $(MAKECMDGOALS)

%: %.c $(SHTALIB)
	$(CC) $(OPT) -DPIL2C $(PIL_SRC) $(OP_SEQC) $< -o $@ $(CFLAGS) $(LDFLAGS)
#	$(CC) $(OPT) $(PIL_SRC) $< -o $@ $(CFLAGS) $(LDFLAGS)

# $< is the first prerequisite
# $@ is the target
%_omp: %.c $(SHTALIB)
	$(CC) $(OPT) -DPIL2C -fopenmp -DOMP $(PIL_SRC) $(OP_OMP) $< -o $@ $(CFLAGS) -I../src/ $(LDFLAGS)

%_swarm: %.c $(SHTALIB)
	$(CC) $(SWARM_FLAGS) $(CFLAGS) $(OPT) $(SWARM_INC) $(OP_SWARM) $< $(PIL_SRC) -o $@ $(SWARM_LINK) $(LDFLAGS)

%_ocr: %.c $(SHTALIB)
	$(CC) $(OCR_FLAGS) -DPIL2OCR $(CFLAGS) $(OPT) $(OP_OCR) $< $(PIL_SRC) -o $@ $(LDFLAGS) $(OCR_LDFLAGS)
test:
	python auto_test.py

clean:
	$(RM) *.o  $(TARGETS)

realclean:
	$(RM) auto_test_result.txt
