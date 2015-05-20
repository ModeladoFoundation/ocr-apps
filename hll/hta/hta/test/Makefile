include ../Makefile.common

SRCS = Alloc_test.c \
       Accessor_test.c \
       Debug_test.c \
       Tuple_test.c \
       Dist_test.c \
       Dist_test_2d.c \
       Tuple_iterator_test.c\
       Mapping_test.c \
       HTA_init_test.c \
       HTA_partition_test.c \
       HTA_partition_init.c \
       HTA_iterator_test.c \
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
       HTA_nd_rank_offset.c \
       HTA_circshift_test.c \
       HTA_layout_test.c \
       HTA_assign_test.c \
       Comm_bcast.c \
       pow_hta.c \
       TLA_bellman_ford.c
#       HTA_cholesky.c \
#       HTAblas_matmul.c
#       HTA_performance_test.c \
#       RefCount_test.c \
#       HTA_SUMMA.c
#LMKL = -L/opt/intel/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lrt
CFLAGS_PIL = -std=gnu99 $(DEBUG) -I$(HTA_INC) -I$(PIL_INC) -DPILHTA
ifeq ($(PROFILE), 1)
    CFLAGS_PIL += -L$(PROFILE_LIB) $(LPROFILE) -I$(PROFILE_INC)
endif
CFLAGS_MPI = -std=gnu99 $(DEBUG) -I$(HTA_INC) -DMPI
OP_MPI = $(HTA_SRC)/Operation_mpi.c
OCR_SPMD_EXEC = $(SRCS:.c=_spmd_ocr)
OMP_SPMD_EXEC = $(SRCS:.c=_spmd_omp)
SWARM_SPMD_EXEC = $(SRCS:.c=_spmd_swarm)
OCR_FJ_EXEC = $(SRCS:.c=_ocr)
OMP_FJ_EXEC = $(SRCS:.c=_omp)
SWARM_FJ_EXEC = $(SRCS:.c=_swarm)
ifeq ($(SPMD_PIL), 1)
    HTA_LIB = $(HTA_SPMD_PIL_LIB)
    CFLAGS_PIL += -DSPMD_PIL
    OP_OMP = $(HTA_SRC)/Operation_spmd_omp.c
    OP_SWARM = $(HTA_SRC)/Operation_spmd_swarm.c
    OP_OCR = $(HTA_SRC)/Operation_spmd_ocr.c
    OCR_EXEC = $(OCR_SPMD_EXEC)
    OMP_EXEC = $(OMP_SPMD_EXEC)
    SWARM_EXEC = $(SWARM_SPMD_EXEC)
    LHTA = -lhta_spmd
    TEST_SCRIPT_NAME = auto_test_spmd.py
    TEST_RESULT_NAME = auto_test_result_spmd.txt
else
    HTA_LIB = $(HTA_PIL_LIB)
    OP_OMP = $(HTA_SRC)/Operation_omp.c
    OP_SWARM = $(HTA_SRC)/Operation_swarm.c
    OP_OCR = $(HTA_SRC)/Operation_ocr.c
    OCR_EXEC = $(OCR_FJ_EXEC)
    OMP_EXEC = $(OMP_FJ_EXEC)
    SWARM_EXEC = $(SWARM_FJ_EXEC)
    LHTA = -lhta
    TEST_SCRIPT_NAME = auto_test.py
    TEST_RESULT_NAME = auto_test_result.txt
endif
ifeq ($(FOR_CNC), 1)
    OP_OMP =
endif
LDFLAGS_PIL = -L$(HTA)/lib $(LHTA) -lm -lpthread ${LMKL}
LDFLAGS_MPI = -L$(HTA)/lib -lhta_mpi -lm -lpthread
OP_SEQC = $(HTA_SRC)/Operation_seq.c
SWARM_FLAGS = -DPIL2SWARM -fopenmp ${DTRACING}
MPI_EXEC = $(SRCS:.c=_mpi)
#TARGETS = $(SWARM_EXEC) $(OCR_EXEC) $(OMP_EXEC) #$(MPI_EXEC)
TARGETS = $(OMP_EXEC) $(OCR_EXEC)


# all has to be the first
all: $(TARGETS)

ocr: $(OCR_EXEC)
omp: $(OMP_EXEC)
swarm: $(SWARM_EXEC)
mpi: $(MPI_EXEC)

.PHONY: test clean realclean

$(HTA_LIB):
	$(MAKE) -C $(HTA)/src $@

$(HTA_MPI_LIB):
	$(MAKE) -C $(HTA)/src $@

%: %.c $(HTA_LIB)
	$(CC) $(OPT) -DPIL2C $(PIL_SRC) $(OP_SEQC) $< -o $@ $(CFLAGS) $(LDFLAGS)

# $< is the first prerequisite
# $@ is the target
%_omp: %.c $(HTA_LIB)
	$(CC) $(OPT) -DPIL2C -fopenmp -DOMP $(PIL_SRC) $(OP_OMP) $< -o $@ $(CFLAGS_PIL) -I../src $(LDFLAGS_PIL)

%_swarm: %.c $(HTA_LIB)
	$(CC) $(SWARM_FLAGS) $(CFLAGS_PIL) $(OPT) -I$(SWARM_INC) $(OP_SWARM) $< $(PIL_SRC) -o $@ $(SWARM_LINK) $(LDFLAGS_PIL)

%_ocr: %.c $(HTA_LIB)
	$(CC) $(OCR_FLAGS) -DPIL2OCR $(CFLAGS_PIL) $(OPT) $(OP_OCR) $< $(PIL_SRC) -o $@ $(LDFLAGS_PIL) $(OCR_LDFLAGS)

%_spmd_omp: %.c $(HTA_LIB)
	$(CC) $(OPT) -DPIL2C -fopenmp -DOMP $(PIL_SRC) $(OP_OMP) $< -o $@ $(CFLAGS_PIL) -I../src $(LDFLAGS_PIL)

%_spmd_swarm: %.c $(HTA_LIB)
	$(CC) $(SWARM_FLAGS) $(CFLAGS_PIL) $(OPT) -I$(SWARM_INC) $(OP_SWARM) $< $(PIL_SRC) -o $@ $(SWARM_LINK) $(LDFLAGS_PIL)

%_spmd_ocr: %.c $(HTA_LIB)
	$(CC) $(OCR_FLAGS) -DPIL2OCR $(CFLAGS_PIL) $(OPT) $(OP_OCR) $< $(PIL_SRC) -o $@ $(LDFLAGS_PIL) $(OCR_LDFLAGS)

%_mpi: %.c $(HTA_MPI_LIB)
	$(MPICC) $(MPI_FLAGS) $(CFLAGS_MPI) $(OPT) $(OP_MPI) $< -o $@ $(LDFLAGS_MPI)


#test: $(SHTALIB) $(OBJECTS)
test: all
	python $(TEST_SCRIPT_NAME)
	cat $(TEST_RESULT_NAME)

clean:
	$(RM) -f *.o  $(OCR_FJ_EXEC) $(OCR_SPMD_EXEC) $(OMP_FJ_EXEC) $(OMP_SPMD_EXEC) $(SWARM_FJ_EXEC) $(SWARM_SPMD_EXEC)
	$(RM) -f auto_test_result.txt auto_test_result_spmd.txt
	$(RM) -f *.log core.*
