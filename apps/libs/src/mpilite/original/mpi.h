/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#ifndef FFWD_MPI_H
#define FFWD_MPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "messaging.h"

#define MPI_BOTTOM		((void *)0)
#define MPI_SUCCESS		1
#define MPI_ERR_ARG		(-1)
#define MPI_ERR_TYPE		(-2)
#define MPI_ERR_REQUEST		(-3)
#define MPI_ERR_INTERN		(-4)
#define MPI_ERR_TOPOLOGY	(-5)
#define MPI_ERR_OTHER		(-6)
#define MPI_UNDEFINED		(-7)

#define MPI_MAX_ERROR_STRING	(64)
int MPI_Error_string(int errorcode, char *string, int *resultlen);
int MPI_Error_class(int errorcode, int *errorclass);

#define MPI_THREAD_SINGLE	0
#define MPI_THREAD_FUNNELED	1
#define MPI_THREAD_SERIALIZED	2
#define MPI_THREAD_MULTIPLE	3

#define MPI_STATUS_IGNORE	(0)
#define MPI_STATUSES_IGNORE	(0)


#define MPI_IN_PLACE	((void *)-1)	// currently only for Reduce/Allreduce

#define MPI_GROUP_NULL	((void *)0)
#define MPI_GROUP_MAX	(64)	// must be >= FFWD_GLOBAL_MAX
typedef struct {
	int gpid[MPI_GROUP_MAX];
	int size;
#ifndef MPILITE_THREAD_MODEL
	int rank;	// cached value 'my rank' this is process-private value
#endif
} _MPI_Group;
typedef _MPI_Group	*MPI_Group;

typedef struct {
	int ndims;
	int reorder;
	struct {
		int dims;
		int periods;
	} a[0];	// aos
} _MPI_Topology;
typedef _MPI_Topology	*MPI_Topology;

typedef struct {
	MPI_Group 	group;
	MPI_Topology	topology;
	void *		handler;
	// plus context
} _MPI_Comm;
typedef _MPI_Comm	*MPI_Comm;
#define MPI_COMM_NULL	((void *)0)
#define MPI_COMM_WORLD	mpi_comm_world
#define MPI_COMM_SELF	mpi_comm_self
extern MPI_Comm mpi_comm_world;
extern MPI_Comm mpi_comm_self;

typedef void MPI_Comm_errhandler_fn(MPI_Comm *, int *, ...);
typedef MPI_Comm_errhandler_fn *MPI_Errhandler;
// TODO
#define MPI_ERRORS_ARE_FATAL	NULL	// Error handler that forces exit on error (default)
#define MPI_ERRORS_RETURN	((void *)-1)	// Error handler that returns error codes

// This function is identical to MPI_ERRHANDLER_CREATE, whose use is deprecated.
int MPI_Comm_create_errhandler(MPI_Comm_errhandler_fn *function, MPI_Errhandler *errhandler);
int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler);
int MPI_Comm_call_errhandler(MPI_Comm comm, int errorcode);
int MPI_Errhandler_free(MPI_Errhandler *errhandler);
#define MPI_Errhandler_set(X,Y)	MPI_Comm_set_errhandler(X,Y)

int MPI_Attr_get(MPI_Comm comm, int keyval,void *attribute_val, int *flag );
// key-value
#define MPI_TAG_UB		(1)
//#define MPI_HOST		(2)
//#define MPI_IO			(3)
//#define MPI_WTIME_IS_GLOBAL	(4)

typedef struct {
	struct ffwd_status mq_status;
} MPI_Status;	// also modify mpif.h MPI_STATUS_SIZE if the size changes
#define MPI_SOURCE	mq_status.source
#define MPI_TAG		mq_status.tag

// TODO: the largest tag value should be available through the attribute MPI_TAG_UB
#define MPI_ANY_SOURCE	FFWD_MQ_ANY_SOURCE
#define MPI_ANY_TAG	FFWD_MQ_ANY_TAG

#ifdef MPILITE_REQUEST_SHADOWING
typedef struct ffwd_message *	MPI_Request;
#else
typedef struct ffwd_message	MPI_Request;
#endif
#define MPI_REQUEST_NULL	ffwd_message_null

typedef int MPI_Op;

#define MPI_SUM		1
#define MPI_MIN		2
#define MPI_MAX		3
#define MPI_MINLOC	4
#define MPI_MAXLOC	5
#define MPI_PROD	6
#define MPI_LAND	7
#define MPI_BAND	8
#define MPI_LOR		9
#define MPI_BOR		10
#define MPI_LXOR	11
#define MPI_BXOR	12

typedef int MPI_Datatype;
typedef unsigned long uint64_t;	// MPI_Init() has assertions

extern MPI_Datatype MPI_CHAR, MPI_SIGNED_CHAR;
extern MPI_Datatype MPI_SHORT;
extern MPI_Datatype MPI_INT;
extern MPI_Datatype MPI_LONG;
extern MPI_Datatype MPI_LONG_LONG, MPI_LONG_LONG_INT;
extern MPI_Datatype MPI_BYTE, MPI_UNSIGNED_CHAR;
extern MPI_Datatype MPI_UNSIGNED_SHORT;
extern MPI_Datatype MPI_UNSIGNED;
extern MPI_Datatype MPI_UNSIGNED_LONG;
extern MPI_Datatype MPI_UNSIGNED_LONG_LONG;
extern MPI_Datatype MPI_UINT64_T;
extern MPI_Datatype MPI_FLOAT;
extern MPI_Datatype MPI_DOUBLE;
extern MPI_Datatype MPI_LONG_DOUBLE;
// TODO: MPI_C_BOOL

// special
//extern MPI_Datatype MPI_UB;

extern MPI_Datatype MPI_DOUBLE_INT;

typedef void MPI_User_function( void *invec, void *inoutvec, int *len, MPI_Datatype *datatype);
int MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op);
int MPI_Op_free( MPI_Op *op);
#define MPI_USER_FUNC_MAX	256
#define MPI_USER_FUNC_BASE	1024
extern MPI_User_function *user_func_table[MPI_USER_FUNC_MAX];


typedef int MPI_Fint;

/* groups and comms */
int MPI_Group_size(MPI_Group group, int *size);
int MPI_Group_rank(MPI_Group group, int *rank);
int MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
int MPI_Group_free(MPI_Group *group);
int MPI_Comm_rank(MPI_Comm comm, int *rank);
int MPI_Comm_size(MPI_Comm comm, int *size);
int MPI_Comm_group(MPI_Comm comm, MPI_Group *group);
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
int MPI_Comm_free(MPI_Comm *comm);
int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
MPI_Comm MPI_Comm_f2c(MPI_Fint comm);


int MPI_Init_thread( int *argc, char ***argv, int required, int *provided );
int MPI_Init(int *argc, char ***argv);
int MPI_Initialized(int *flag);
int MPI_Finalize(void);
int MPI_Abort(MPI_Comm comm, int errorcode);
double MPI_Wtime( void );	// Time in seconds since an arbitrary time in the past.
int MPI_Send (const void *buf,int count, MPI_Datatype
datatype, int dest, int tag, MPI_Comm comm);
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request);
int MPI_Recv (void *buf,int count, MPI_Datatype
datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Wait(MPI_Request *request, MPI_Status *status);
int MPI_Waitall(int count, MPI_Request *array_of_requests,
    MPI_Status *array_of_statuses);
int MPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int source, int recvtag,
                MPI_Comm comm, MPI_Status *status);
int MPI_Barrier( MPI_Comm comm );
int MPI_Get_count(
               MPI_Status *status,
               MPI_Datatype datatype,
               int *count );
int MPI_Test(MPI_Request *NEW_REQUEST, int *flag, MPI_Status *status);
int MPI_Testall(int count, MPI_Request *array_of_requests, int *flag,
    MPI_Status *array_of_statuses);

int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status);
int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
             MPI_Op op, MPI_Comm comm);

int MPI_Reduce(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Allreduce ( void *sendbuf, void *recvbuf, int count,
                          MPI_Datatype datatype, MPI_Op op, MPI_Comm comm );

#ifdef MPILITE_ASYNC_COLLECTIVE
int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm, MPI_Request *request);
#endif
int MPI_Bcast ( void *buffer, int count, MPI_Datatype datatype, int root,
                      MPI_Comm comm );

int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm);

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm);

int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm);

int MPI_Allgather(void *sendbuf, int  sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
     MPI_Datatype recvtype, MPI_Comm comm);

int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm);

int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm);

int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm);

int MPI_Ialltoall(const void *sendbuf, int sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
    MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);


int MPI_Alltoallv(const void *sendbuf, const int *sendcounts,
                  const int *sdispls, MPI_Datatype sendtype, void *recvbuf,
                  const int *recvcounts, const int *rdispls, MPI_Datatype recvtype,
                  MPI_Comm comm);


// derived types
typedef long MPI_Aint;
#if 0

// general
int MPI_Type_struct(int count,
                   int blocklens[],
                   MPI_Aint indices[],
                   MPI_Datatype old_types[],
                   MPI_Datatype *newtype);

/*
       int MPI_Type_create_struct(int count, int array_of_blocklengths[],
            MPI_Aint array_of_displacements[], MPI_Datatype array_of_types[],
            MPI_Datatype *newtype)
*/

int MPI_Type_free(MPI_Datatype *datatype);
int MPI_Type_commit(MPI_Datatype *datatype);
int MPI_Address(void *location, MPI_Aint *address);
int MPI_Get_address(void *location, MPI_Aint *address);

// same datatype, located in contiguous memory locations, for example sequence of entries in an array
int MPI_Type_vector(int count,
                   int blocklength,
                   int stride,
                   MPI_Datatype old_type,
                   MPI_Datatype *newtype_p);

// same datatype, equally separated entries in an array.
int MPI_Type_contiguous(int count,
                      MPI_Datatype old_type,
                      MPI_Datatype *new_type_p);

int MPI_Type_size(MPI_Datatype datatype, int *size);


// derived datatypes
// MPI_Type_indexed	// same datatype. located at specified locations, for example the diagonal elements of a square matrix
int MPI_Type_indexed(int count, const int array_of_blocklengths[],
    const int array_of_displacements[], MPI_Datatype oldtype,
    MPI_Datatype *newtype);
// The function MPI_Type_create_hindexed is identical to MPI_Type_indexed, except that block displacements in array_of_displacements are specified in bytes, rather than in multiples of the oldtype extent.
int MPI_Type_create_hindexed(int count, const int array_of_blocklengths[],
    const int array_of_displacements[], MPI_Datatype oldtype,
    MPI_Datatype *newtype);

// MPI_Pack
// MPI_Unpack

int MPI_Cart_create(MPI_Comm comm_old, int ndims, const int dims[],
                    const int periods[], int reorder, MPI_Comm *comm_cart);
int MPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[],
                 int coords[]);
int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);

#endif

#ifdef __cplusplus
}
#endif

#endif
