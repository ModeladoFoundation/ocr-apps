CFLAGS = -DOPEN_MP -fopenmp -g -Wall -Drestrict=__restrict__ -O3 -DNDEBUG -ffast-math -DGRAPH_GENERATOR_MPI -Wno-unused-but-set-variable # -g -pg 

AFL_PATH_FROM_HERE = ../../tools/rmd_afl
AFLINCLUDES = -I$(AFL_PATH_FROM_HERE)/common  -I$(AFL_PATH_FROM_HERE)/rmd_codelet -I$(AFL_PATH_FROM_HERE) -I../../ss/rmdkrnl/inc/  -I../../ss/runtime/common
RCKDEFINES= -DAFL_WRAPPED -DCODELETS
AFLLIBS = ../../tools/rmd_afl/rmd_codelet/rmd_codelet.a ../../tools/rmd_afl/rmd_afl.a 
AFLLDFLAGS= -lxml2 -lc -ldl -lz -lm -lrt

LDFLAGS = -g #-fopenmp -g # -g -pg

MPICC = mpicc

#all: graph500_mpi_simple 
all: graph500_mpi_simple graph500_mpi_one_sided graph500_mpi_replicated graph500_mpi_replicated_csc

GENERATOR_SOURCES = generator/graph_generator.c generator/make_graph.c generator/splittable_mrg.c generator/utils.c
SOURCES = main.c oned_csr.c oned_csc.c utils.c validate.c onesided.c onesided_emul.c
HEADERS = common.h oned_csr.h oned_csc.h redistribute.h mpi_workarounds.h onesided.h

graph500_mpi_simple: bfs_simple.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(AFLINCLUDES) $(LDFLAGS)  $(RCKDEFINES) -o graph500_mpi_simple bfs_simple.c $(SOURCES) $(GENERATOR_SOURCES) $(AFLLIBS) $(AFLLDFLAGS)

graph500_mpi_one_sided: bfs_one_sided.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(AFLINCLUDES) $(LDFLAGS)  $(RCKDEFINES) -o graph500_mpi_one_sided bfs_simple.c $(SOURCES) $(GENERATOR_SOURCES) $(AFLLIBS) $(AFLLDFLAGS)

graph500_mpi_replicated: bfs_replicated.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(AFLINCLUDES) $(LDFLAGS)  $(RCKDEFINES) -o graph500_mpi_replicated bfs_replicated.c $(SOURCES) $(GENERATOR_SOURCES) $(AFLLIBS) $(AFLLDFLAGS)

graph500_mpi_replicated_csc: bfs_replicated.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(AFLINCLUDES) $(LDFLAGS)  $(RCKDEFINES) -o graph500_mpi_replicated_csc bfs_replicated_csc.c $(SOURCES) $(GENERATOR_SOURCES) $(AFLLIBS) $(AFLLDFLAGS)

clean:
	-rm -f graph500_mpi_* *.o *.a *~
