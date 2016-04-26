/*
 * This program performs some simple tests of the MPI_Reduce reduction
 * functionality.
    Do long arrays that require reduce to malloc
 */

//#include "test.h"
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

void Test_Init(char* test, int rank, int numRanks)
{
    if (0 == rank)
        {
            printf("** Start Test %s with %d ranks\n", test, numRanks);

        }
}

const int size=100;


int
main( int argc, char **argv)
{
    int rank, numRanks, ret=0, passed, i;

    /* Set up MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);


    int myValue[size];
    const int correctSum = (numRanks*(numRanks+1))/2;


    /* Setup the tests */
    Test_Init("reduceLarge", rank, numRanks);

    /* Perform the test - do sum reductions with different ranks
       as the root - answer should always be the same */

    int sum[size], minusOne[size];
    passed = 1;

    for (int m=0; m<size; m++)
        {
            myValue[m]=rank+1;
            minusOne[m]=-1;
        }
    memcpy((char*)&sum, (char*)&minusOne,size*4);

    // wait for everyone to get set up before starting timer
    MPI_Barrier(MPI_COMM_WORLD);

    double startTime = MPI_Wtime();

    // loop twice to make sure no one has outstanding sends
    for (int j=0; j<2; j++)
        {
            for (i=0; i < numRanks; i++) {
                if (i == rank)
                    {
                        memcpy((char*)&sum, (char*)&myValue,size*4);

                        MPI_Reduce(MPI_IN_PLACE, &sum, size, MPI_INT, MPI_SUM, i, MPI_COMM_WORLD);
                    }
                else
                    {
                        MPI_Reduce(&myValue, NULL, size, MPI_INT, MPI_SUM, i, MPI_COMM_WORLD);
                    }

                if (i == rank)
                    {
                        for(int ii=0;ii<size; ii++)
                            {
                                if (correctSum != sum[ii])
                                    {
                                        printf("  -- rank %d reduced  sum[%d]=%d instead of%d\n",
                                               rank, ii, sum[ii], correctSum);
                                    }
                            }
                    }
                memcpy((char*)&sum, (char*)&minusOne,size*4);

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
