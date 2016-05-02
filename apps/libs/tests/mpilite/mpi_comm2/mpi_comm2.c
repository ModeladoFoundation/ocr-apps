/* This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
   MPI_Comm row_comm, col_comm;
   int myrank, size, P=4, Q=4, p, q;

   MPI_Init (&argc, &argv);
   MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
   MPI_Comm_size (MPI_COMM_WORLD, &size);

   /* Determine row and column position */
   p = myrank / Q;
   q = myrank % Q; /* pick a row-major mapping */

   /* Split comm into row and column comms */
   MPI_Comm_split(MPI_COMM_WORLD, p, q, &row_comm);
   /* color by row, rank by column */
   MPI_Comm_split(MPI_COMM_WORLD, q, p, &col_comm);
   /* color by column, rank by row */

   printf("[%d]:My coordinates are (%d,%d)\n",myrank,p,q);
   MPI_Finalize();
}

