/* This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, nprocs;

    MPI_Init(&argc,&argv);

    MPI_Comm newcomm;
    MPI_Comm_dup(MPI_COMM_WORLD, &newcomm);
    MPI_Comm_size(newcomm,&nprocs);
    MPI_Comm_rank(newcomm,&rank);
    int i;
    for(i=0;i<2;i++) {
	MPI_Barrier(newcomm);
	printf("Rank %d of %d, round %d\n", rank, nprocs, i);fflush(stdout);
    }
    printf("Done!\n");
    MPI_Finalize();
    return 0;
}
