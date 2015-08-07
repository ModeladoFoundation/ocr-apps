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

// Rename user's main fn
#define main __mpiOcrMain

// The OCR x86-mpi runtime uses a "real" MPI, e.g. Intel MPI for
// communicating between Policy Domains. The 11 function names below are
// the ones used. However, MPI-Lite also provides these same
// functions. When an app links against x86-mpi OCR, the executable only
// gets the MPI-Lite versions of these functions, not the "real" ones, and
// execution dies a horrible death.
// The correct fix is for the OCR runtime to bind tightly to Intel MPI
// functions, and not let them be exposed to the app. But this is not being
// done.
// Instead, a workaround is for MPI-Lite to rename these functions - both
// the definition when mpilite is built, and the references in the user
// program. That is what is done below if OCR_X86_MPI is set to 1. The
// ifndef permits setting -DOCR_X86_MPI=1 during the mpilite build, and
// also the app build.
// The disadvantage is that in the debugger the name is MPILite_Init
// instead of MPI_Init....

#ifndef OCR_X86_MPI
    #define OCR_X86_MPI 0
#endif

#if OCR_X86_MPI
    // For ocr_type=x86-mpi, need to hide these symbols which are
    // used by the runtime from Intel MPI
    #define MPI_Init MPIlite_Init
    #define MPI_Finalize MPIlite_Finalize
    #define MPI_Irecv MPIlite_Irecv
    #define MPI_Isend MPIlite_Isend
    #define MPI_Iprobe MPIlite_Iprobe
    #define MPI_Get_count MPIlite_Get_count
    #define MPI_Recv MPIlite_Recv
    #define MPI_Test MPIlite_Test
    #define MPI_Comm_rank MPIlite_Comm_rank
    #define MPI_Comm_size MPIlite_Comm_size
    #define MPI_Barrier MPIlite_Barrier
#endif

#ifndef __x86_64__
 // If not on x86, only have PRINTF, so substitute it for printf
// replace printf with PRINTF, but don't include ocr.h
#define printf PRINTF

#ifdef _STDIO_H
    // so stdio.h has been included, need decl for PRINTF.
    // If not included yet, the #define printf PRINTF will provide a decl
    // for PRINTF if stdio.h is included. If it's not included, then it
    // doesn't matter
    unsigned int PRINTF(const char *, ...);
#endif
#endif // ndef __x86_64__

#define MPI_BOTTOM              ((void *)0)
#define MPI_SUCCESS             (0)
#define MPI_ERR_ARG             (-1)
#define MPI_ERR_TYPE            (-2)
#define MPI_ERR_REQUEST         (-3)
#define MPI_UNDEFINED           (-4)
#define MPI_ERR_INTERN          (-5)
#define MPI_ERR_TOPOLOGY        (-6)
#define MPI_ERR_TRUNCATE        (-7)

#define MPI_THREAD_SINGLE       0
#define MPI_THREAD_FUNNELED     1
#define MPI_THREAD_SERIALIZED   2
#define MPI_THREAD_MULTIPLE     3

#define MPI_STATUS_IGNORE       (0)
#define MPI_STATUSES_IGNORE     (0)

#define MPI_IN_PLACE            ((void *)-1)  // currently only for Reduce/Allreduce

#define MPI_GROUP_NULL          ((void *)0)
#define MPI_GROUP_MAX           (64)    // must be >= FFWD_GLOBAL_MAX
typedef struct {
    int gpid[MPI_GROUP_MAX];
    int size;
    int rank;    // cached value 'my rank' this is process-private value
} _MPI_Group;
typedef _MPI_Group    *MPI_Group;

typedef struct {
    int ndims;
    int reorder;
    struct {
            int dims;
            int periods;
    } a[0];    // aos
} _MPI_Topology;
typedef _MPI_Topology    *MPI_Topology;

typedef struct {
    MPI_Group     group;
    MPI_Topology    topology;
    // plus context
} _MPI_Comm;
typedef int    MPI_Comm;

#define MPI_COMM_NULL    (-1)
#define MPI_COMM_WORLD   ( 0)
#define MPI_COMM_SELF    (-2)

struct ffwd_status {
    int count;
    int datatype;
    int tag;
    int source;
};

struct ffwd_message {
    int op;  // what operation should be done?: send, recv, probe
    int count;
    int datatype;
    int tag;
    int rank;    // dest or source depending on context.. global pid
    int comm;
    int flag;
    volatile int status;
    void *buf;
};

typedef struct {
    struct ffwd_status mq_status;
} MPI_Status;    // also modify mpif.h MPI_STATUS_SIZE if the size changes
#define MPI_SOURCE        mq_status.source
#define MPI_TAG           mq_status.tag

// TODO: the largest tag value should be available through the attribute MPI_TAG_UB
#define MPI_ANY_SOURCE    (-1)
#define MPI_ANY_TAG       (-1)

typedef struct ffwd_message *    MPI_Request;

#define MPI_REQUEST_NULL  NULL

typedef int MPI_Op;

#define MPI_SUM           1
#define MPI_MIN           2
#define MPI_MAX           3
#define MPI_MINLOC        4
#define MPI_MAXLOC        5
#define MPI_PROD          6
#define MPI_LAND          7
#define MPI_BAND          8
#define MPI_LOR           9
#define MPI_BOR          10
#define MPI_LXOR         11
#define MPI_BXOR         12

typedef int MPI_Datatype;

#define MPI_CHAR                 1
#define MPI_SIGNED_CHAR          2
#define MPI_SHORT                3
#define MPI_INT                  4
#define MPI_LONG                 5
#define MPI_LONG_LONG            6
#define MPI_LONG_LONG_INT        7
#define MPI_BYTE                 8
#define MPI_UNSIGNED_CHAR        9
#define MPI_UINT8_T             MPI_UNSIGNED_CHAR
#define MPI_UNSIGNED_SHORT      10
#define MPI_UINT16_T            MPI_UNSIGNED_SHORT
#define MPI_UNSIGNED            11
#define MPI_UINT32_T            MPI_UNSIGNED
#define MPI_UNSIGNED_LONG       12
#define MPI_UINT64_T            MPI_UNSIGNED_LONG
#define MPI_UNSIGNED_LONG_LONG  13
#define MPI_FLOAT               14
#define MPI_DOUBLE              15
// TODO: MPI_C_BOOL

// special
//extern MPI_Datatype MPI_UB;

typedef struct {
    double a;
    int b;
} double_int;

#define MPI_DOUBLE_INT          16
#define _MPI_LAST_DATATYPE      16

typedef void MPI_User_function( void *invec, void *inoutvec, int *len, MPI_Datatype *datatype);
int MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op);
int MPI_Op_free( MPI_Op *op);
#define MPI_USER_FUNC_MAX       256
#define MPI_USER_FUNC_BASE      1024
//ocr extern MPI_User_function *user_func_table[MPI_USER_FUNC_MAX];



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
double MPI_Wtick( void );	// Accuracy in seconds of Wtime
double MPI_Wtime( void );    // Time in seconds since an arbitrary time in the past.
int MPI_Send (void *buf,int count, MPI_Datatype
              datatype, int dest, int tag, MPI_Comm comm);
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
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


int MPI_Alltoallv(const void *sendbuf, const int *sendcounts,
                  const int *sdispls, MPI_Datatype sendtype, void *recvbuf,
                  const int *recvcounts, const int *rdispls, MPI_Datatype recvtype,
                  MPI_Comm comm);

// derived types
typedef int MPI_Aint;

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
// MPI_Type_indexed    // same datatype. located at specified locations, for example the diagonal elements of a square matrix
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

#ifdef __cplusplus
}
#endif

#endif
