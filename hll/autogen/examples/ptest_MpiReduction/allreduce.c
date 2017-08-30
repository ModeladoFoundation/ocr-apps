#include <mpi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define _GNU_SOURCE
#define __USE_GNU
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <assert.h>
#include "perfs.h"

#ifndef NB_MSG
#define NB_MSG 1000
#endif

#ifndef NB_RANKS
#define NB_RANKS 8
#endif


int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);
    int myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    int i = 0;
    size_t strSz = 10;
    char prefix[strSz];
    int retCode = snprintf(prefix, strSz, "%d: ", myRank);
    assert(retCode < strSz);
    MPI_Barrier(MPI_COMM_WORLD);
    timestamp_t lstart;
    get_time(&lstart);
    int ub = NB_MSG;
    for(i=0; i<ub;) {
    	double sendBuf = 1.0;
        double recvBuf;
        MPI_Allreduce(&sendBuf, &recvBuf, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        assert(recvBuf == ((double) NB_RANKS));
        i++;
    }
    timestamp_t lstop;
    get_time(&lstop);
    long elap = elapsed_usec(&lstart, &lstop);
    printf("%sAllReduce %lu usec\n", prefix, elap);
    summary_throughput_timer_custom(&lstart,&lstop,ub,prefix);
    MPI_Finalize();

    return 0;
}
