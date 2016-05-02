/* This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
/*
 * Shows a short cut method to create a collection of communicators.
 * All processors with the "same color" will be in the same communicator.
 * In this case the color is either 0 or 1 for even or odd processors.
 * Index gives rank in new communicator.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
int main(argc,argv)
int argc;
char *argv[];
{
    int myid, numprocs;
    int color,Zero_one,new_id,new_size;
    MPI_Comm NEW_COMM;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    color=myid % 2;
    MPI_Comm_split(MPI_COMM_WORLD,color,myid,&NEW_COMM);
    MPI_Comm_rank( NEW_COMM, &new_id);
    MPI_Comm_size( NEW_COMM, &new_size);

    Zero_one = -1;
    if(new_id==0)
    {
        Zero_one = color;
        MPI_Bcast(&Zero_one,1,MPI_INT,0, NEW_COMM);
    }

    printf("main: physical rank=%d; group rank=%d\n", myid, new_id);
    MPI_Finalize();
    printf("Done!\n");

}

