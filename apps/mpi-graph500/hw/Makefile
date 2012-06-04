CFLAGS = -fopenmp -g -Wall -Drestrict=__restrict__ -O3 -DNDEBUG -ffast-math -DGRAPH_GENERATOR_MPI -Wno-unused-but-set-variable # -g -pg 
# CFLAGS = -g -Wall -Drestrict= -DGRAPH_GENERATOR_MPI
LDFLAGS = -g #-fopenmp -g # -g -pg
MPICC = mpicc

all: mpihw

SOURCES = mpihw.c
HEADERS = 

mpihw: 
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o mpihw mpihw.c  -lm


