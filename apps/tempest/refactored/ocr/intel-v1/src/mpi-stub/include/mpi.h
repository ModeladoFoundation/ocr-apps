#ifndef _MPI_STUB_H_
#define _MPI_STUB_H_

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef int MPI_Comm;
typedef int MPI_Request;
typedef int MPI_Datatype;
typedef int MPI_Op;
class MPI_Status
{
    public:
        int MPI_TAG;
};


#define MPI_COMM_WORLD ((MPI_Comm)1)
#define MPI_IN_PLACE 0
#define MPI_SUM 0
#define MPI_MAX 0
#define MPI_ANY_SOURCE 0
#define MPI_ANY_TAG 0
#define MPI_DOUBLE 16
int MPI_Init( int *argc, char ***argv);
int MPI_Finalize(void);

int MPI_Comm_rank( MPI_Comm comm, int *rank );
int MPI_Comm_size( MPI_Comm comm, int *size );


int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status);

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm);

int MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
               MPI_Comm comm, MPI_Request *request);
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
               MPI_Comm comm, MPI_Request *request);

int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status);


int MPI_Barrier(MPI_Comm comm);


int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root,
              MPI_Comm comm);

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm);

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm);

int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm);

int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm);


int MPI_Wait(MPI_Request *request, MPI_Status *status);

int MPI_Waitall(int count, MPI_Request array_of_requests[],
                MPI_Status array_of_statuses[]);
int MPI_Get_count( MPI_Status *status,  MPI_Datatype datatype, int *count );
int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
#ifdef __cplusplus
}
#endif

#endif //_MPI_STUB_H_
