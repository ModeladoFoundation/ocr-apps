#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int right_neighbor(int myrank_, int size_) {
  if(myrank_ == size_-1) return 0;
  return myrank_+1;
}

int left_neighbor(int myrank_, int size_) {
  if(myrank_ == 0) return size_-1;
  return myrank_-1;
}

int main(int argc, char* argv[]) {

#pragma ocr spmd region NTASKS(16) DEP_DBKs() DEP_EVTs() DEP_ELEMs()	\
    OEVENT(OEVT_spmd)
  {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int bsize = 5;
    #pragma ocr datablock DATABLOCK(DBK_BUFF)
    float *sbuff;

    sbuff = (float*) malloc(sizeof(float) * bsize);

    if(rank % 2  == 0) {

      #pragma ocr task TASK(TASK_ArrWrite) DEP_DBKs(DBK_BUFF) DEP_EVTs() \
	DEP_ELEMs(rank, size, bsize) OEVENT(OEVT_arrWrite)
      {
	int i;
	for(i = 0; i < bsize; ++i) {
	  sbuff[i] = (float) rank + 1.0;
	}
      }

      #pragma ocr spmd send SEND_DBK(DBK_BUFF) DEP_EVTs(OEVT_arrWrite) OEVENT(OEVT_sright)
      MPI_Send(sbuff, bsize, MPI_FLOAT, right_neighbor(rank, size), 0, MPI_COMM_WORLD);

      #pragma ocr spmd finalize DEP_EVTs(OEVT_sright)
    }
    else {
      MPI_Status status;
      #pragma ocr spmd recv RECV_EVT(EVT_RLEFT) DEP_EVTs() OEVENT(OEVT_rleft)
      MPI_Recv(sbuff, bsize, MPI_FLOAT, left_neighbor(rank, size), 0, MPI_COMM_WORLD, &status);

       #pragma ocr task TASK(TASK_Print) DEP_DBKs() DEP_EVTs(EVT_RLEFT) \
       DEP_ELEMs(rank, size, bsize) OEVENT(OEVT_print)
      {
      	int i;
	printf("rank=%d\n", rank);
      	for(i = 0; i < bsize; ++i) {
      	  printf("sbuff[%d]=%.2f ", i, sbuff[i]);
      	}
      	printf("\n");
      }

      #pragma ocr spmd finalize DEP_EVTs(OEVT_print)
    }

    MPI_Finalize();
  }

  #pragma ocr shutdown DEP_EVTs(OEVT_spmd)
  return 0;
}
