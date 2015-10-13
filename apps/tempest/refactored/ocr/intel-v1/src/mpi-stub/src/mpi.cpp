#include <mpi.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

int MPI_Init( int *argc, char ***argv) {return 0;};
int MPI_Finalize(void) {return 0;};

int MPI_Comm_rank( MPI_Comm comm, int *rank ){ *rank=0; return 0;}
int MPI_Comm_size( MPI_Comm comm, int *size ){ *size=1; return 0;}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
               MPI_Comm comm, MPI_Request *request){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
             MPI_Op op, MPI_Comm comm){
  memcpy(recvbuf, sendbuf, count*datatype);
}

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Comm_free(MPI_Comm *comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Wait(MPI_Request *request, MPI_Status *status){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Waitall(int count, MPI_Request array_of_requests[],
                MPI_Status array_of_statuses[]){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root,
              MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

int MPI_Barrier(MPI_Comm comm){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}
int MPI_Get_count( MPI_Status *status,  MPI_Datatype datatype, int *count ){
                  count = 0;
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}
int MPI_Test( MPI_Request *request, int *flag, MPI_Status *status ){
    printf ("ERROR: MPI SHOULD NOT BE CALLED!\n");
    exit (1);
}

#ifdef __cplusplus
}
#endif
