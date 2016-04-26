/*
 * This program performs some simple tests of the MPI_Bcast broadcast
 * functionality.
 */

//#include "test.h"
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

void Test_Init(char* test, int rank, int numRanks)
{
    if (0 == rank)
        {
            printf("** Start Test %s with %d ranks\n", test, numRanks);

        }
}



int
main( int argc, char **argv)
{
    int rank, numRanks, ret=0, passed, i;

    /* Set up MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    double startTime = MPI_Wtime();

    /* Setup the tests */
    Test_Init("bcast", rank, numRanks);
    int test_array[numRanks];

    /* Perform the test - this operation should really be done
       with an allgather, but it makes a good test... */
    passed = 1;
    // loop twice to make sure no one has outstanding sends
    for (int j=0; j<2; j++)
        {
    for (i=0; i < numRanks; i++) {
	if (i == rank)
	    test_array[i] = i;
	MPI_Bcast(test_array, numRanks, MPI_INT, i, MPI_COMM_WORLD);
	if (test_array[i] != i)
            {
                printf("  -- rank %d received %d instead of %d bcast from %d\n",
                       rank, test_array[i], i,i);

            }

    }
    }

    if (0 == rank)
        {
            double time = MPI_Wtime() - startTime, timePerRank=time/numRanks;

            printf("** Finish Test %s with %d ranks time:%f time/bcast:%f time/bcast/rank:%f\n",
                   "bcast", numRanks,time, timePerRank, timePerRank/numRanks);

        }

    MPI_Finalize();
    return ret;
}
