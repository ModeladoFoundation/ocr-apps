/*
 * This program performs some simple tests of the MPI_Reduce reduction
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
    int myValue = rank+1;
    const int correctSum = (numRanks*(numRanks+1))/2;

    /* Setup the tests */
    Test_Init("reduce", rank, numRanks);

    /* Perform the test - do sum reductions with different ranks
       as the root - answer should always be the same */

    int sum = -1;
    passed = 1;
    // loop twice to make sure no one has outstanding sends
    for (int j=0; j<2; j++)
        {
    for (i=0; i < numRanks; i++) {
	MPI_Reduce(&myValue, &sum, 1, MPI_INT, MPI_SUM, i, MPI_COMM_WORLD);
	if (i == rank && correctSum != sum)
            {
                printf("  -- rank %d reduced %d instead of %d\n",
                       rank, sum, correctSum);

            }

    }
    }

    if (0 == rank)
        {
            double time = MPI_Wtime() - startTime, timePerRank=time/numRanks;

            printf("** Finish Test %s with %d ranks time:%f time/reduce:%f time/reduce/rank:%f\n",
                   "reduce", numRanks,time, timePerRank, timePerRank/numRanks);

        }

    MPI_Finalize();
    return ret;
}
