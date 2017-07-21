#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {

  #pragma ocr spmd region NTASKS(8) DEP_DBKs() DEP_EVTs() \
    DEP_ELEMs() OEVENT(OEVT_spmd)
  {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int bsize = 100;
    double local, global;
    #pragma ocr datablock DATABLOCK(DBK_data)
    double* data;
    data = (double*)malloc(sizeof(double) * bsize);

    #pragma ocr task TASK(TASK_init) DEP_DBKs(DBK_data) DEP_EVTs() \
      DEP_ELEMs(bsize, local) OEVENT(OEVT_init)
    {
      int i;
      for(i = 0; i < bsize; i++) {
      data[i] = 1.11111;
      }
    }

    #pragma ocr task TASK(TASK_reduce) DEP_DBKs(DBK_data) DEP_EVTs(OEVT_init) \
      DEP_ELEMs(rank, bsize, local, global) OEVENT(OEVT_reduction)
    {
      int i;
      for(i = 0; i < bsize; i++) {
	local += data[i];
      }

      int root = 0;
      #pragma ocr spmd reduce REDUCE_EVT(EVT_REDUCE_DBK) DEP_EVTs() OEVENT(OEVT_spmdreduce)
      MPI_Reduce(&local, &global, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);

      if(rank == root) {
       #pragma ocr task TASK(TASK_print) DEP_DBKs() DEP_EVTs(EVT_REDUCE_DBK) \
	 DEP_ELEMs(rank, root) OEVENT(OEVT_print)
	{
	  printf("reduced value=%f\n", global);
	}
      }
    }

    #pragma ocr spmd finalize DEP_EVTs(OEVT_reduction)
    MPI_Finalize();
  }

  #pragma ocr shutdown DEP_EVTs(OEVT_spmd)
}
