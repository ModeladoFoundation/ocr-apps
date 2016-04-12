# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

all:
	gcc -I. -g -c messaging.c mpi.c
	gcc -I. -g testing.c messaging.c mpi.c
#	gcc -I. -g ../examples/mpi_ibcast.c messaging.c mpi.c
#	gcc -I. -g testing.c messaging.c mpi.c -empilite_entry -lpthread
#	gcc -I. -g ../examples/mpi_measure.c messaging.c mpi.c -empilite_entry -lpthread
	MPILITE_RANK=4 MPILITE_POOL_SIZE=1 ./a.out

clean:
	rm *.o a.out core.*
