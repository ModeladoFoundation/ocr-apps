/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#include <stdio.h>
#include <mpi.h>

#if 1
int main (argc, argv)
     int argc;
     char *argv[];
{
  int rank, size;
  MPI_Init (&argc, &argv);	/* starts MPI */

  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
  printf( "Hello world from rank %d of %d\n", rank, size );

#if 1
  int number;
  if (rank == 1) {
    number = 1234;
    MPI_Send(&number, 1, MPI_INT, 0, 777, MPI_COMM_WORLD);
    printf("Rank 1 sent data.\n");
  } else if (rank == 0) {
    MPI_Recv(&number, 1, MPI_INT, /* MPI_ANY_SOURCE*/ 1 , 777, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    printf("Rank 0 received number %d\n", number);
  }
#endif
  MPI_Finalize();
  return 0;
}

#else
int main(int argc, char *argv[])
{
    int myid, numprocs, left, right;
    int buffer[10] = {11,22,33,44,55,66,77,88,99,0};
    int buffer2[10] = {1,2,3,4,5,6,7,8,9,0};
    MPI_Request request = MPI_REQUEST_NULL;	// initialization is not necessary..
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    right = (myid + 1) % numprocs;
    left = myid - 1;
    if (left < 0)
        left = numprocs - 1;

    MPI_Irecv(buffer, 10, MPI_INT, left, 123, MPI_COMM_WORLD, &request);
    MPI_Send(buffer2, 10, MPI_INT, right, 123, MPI_COMM_WORLD);
    MPI_Wait(&request, &status);
    printf("received %d,%d,%d...\n", buffer[0], buffer[1], buffer[2]);
    MPI_Finalize();
    return 0;
}
#endif
