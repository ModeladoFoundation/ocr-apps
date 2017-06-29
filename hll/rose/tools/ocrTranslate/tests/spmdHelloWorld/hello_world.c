#include "mpi.h"
#include <stdio.h>


int main(int argc, char* argv[]) {
  #pragma ocr datablock DATABLOCK(DBK_ARR)
  int array[100];

#pragma ocr spmd region NTASKS(100) DEP_DBKs(DBK_ARR) DEP_EVTs() DEP_ELEMs()	\
    OEVENT(OEVT_spmd)
  {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    #pragma ocr task TASK(TASK_ArrayWrite) DEP_DBKs(DBK_ARR) DEP_EVTs() \
      DEP_ELEMs(rank, size) OEVENT(OEVT_arrayWrite)
    {
      array[rank] = rank;
    }

    #pragma ocr task TASK(TASK_Print) DEP_DBKs(DBK_ARR) DEP_EVTs(OEVT_arrayWrite) \
      DEP_ELEMs(rank, size) OEVENT(OEVT_print)
    {
      printf("Hello World from rank=%d of %d\n", array[rank], size);
    }

    #pragma ocr spmd finalize DEP_EVTs(OEVT_print)
    MPI_Finalize();
  }

  #pragma ocr shutdown DEP_EVTs(OEVT_spmd)
  return 0;
}
