// these values are copied from mpilite/mpi.h.
// Since MPIlite uses different values than MPI,
// this tool must replace the MPI value with the
// MPIlite value.
#define MPILITE_COMM_WORLD   ( 0)
#define MPILITE_COMM_SELF    ( 2)

#define MPILITE_CHAR                 1
#define MPILITE_SIGNED_CHAR          2
#define MPILITE_SHORT                3
#define MPILITE_INT                  4
#define MPILITE_LONG                 5
#define MPILITE_LONG_LONG            6
#define MPILITE_LONG_LONG_INT        7
#define MPILITE_BYTE                 8
#define MPILITE_UNSIGNED_CHAR        9
#define MPILITE_UINT8_T             MPILITE_UNSIGNED_CHAR
#define MPILITE_UNSIGNED_SHORT      10
#define MPILITE_UINT16_T            MPILITE_UNSIGNED_SHORT
#define MPILITE_UNSIGNED            11
#define MPILITE_UINT32_T            MPILITE_UNSIGNED
#define MPILITE_UNSIGNED_LONG       12
#define MPILITE_UINT64_T            MPILITE_UNSIGNED_LONG
#define MPILITE_UNSIGNED_LONG_LONG  13
#define MPILITE_FLOAT               14
#define MPILITE_DOUBLE              15
#define MPILITE_DOUBLE_INT          16
#define _MPILITE_LAST_DATATYPE      16

#define MPILITE_SUM           1
#define MPILITE_MIN           2
#define MPILITE_MAX           3
#define MPILITE_MINLOC        4
#define MPILITE_MAXLOC        5
#define MPILITE_PROD          6
#define MPILITE_LAND          7
#define MPILITE_BAND          8
#define MPILITE_LOR           9
#define MPILITE_BOR          10
#define MPILITE_LXOR         11
#define MPILITE_BXOR         12


// these values are defined in the official mpi.h
typedef int MPI_Comm;
#define MPI_COMM_WORLD ((MPI_Comm)0x44000000)
#define MPI_COMM_SELF  ((MPI_Comm)0x44000001)

typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)0x4c000101)
#define MPI_SIGNED_CHAR    ((MPI_Datatype)0x4c000118)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)0x4c000102)
#define MPI_BYTE           ((MPI_Datatype)0x4c00010d)
#define MPI_SHORT          ((MPI_Datatype)0x4c000203)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)0x4c000204)
#define MPI_INT            ((MPI_Datatype)0x4c000405)
#define MPI_UNSIGNED       ((MPI_Datatype)0x4c000406)
#define MPI_LONG           ((MPI_Datatype)0x4c000807)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)0x4c000808)
#define MPI_FLOAT          ((MPI_Datatype)0x4c00040a)
#define MPI_DOUBLE         ((MPI_Datatype)0x4c00080b)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)0x4c000809)
#define MPI_UNSIGNED_LONG_LONG ((MPI_Datatype)0x4c000819)
#define MPI_LONG_LONG      MPI_LONG_LONG_INT
#define MPI_DOUBLE_INT        ((MPI_Datatype)0x8c000001)
// There are more, but MPI-lite does not support them now.

/* Collective operations */
typedef int MPI_Op;
#define MPI_MAX     (MPI_Op)(0x58000001)
#define MPI_MIN     (MPI_Op)(0x58000002)
#define MPI_SUM     (MPI_Op)(0x58000003)
#define MPI_PROD    (MPI_Op)(0x58000004)
#define MPI_LAND    (MPI_Op)(0x58000005)
#define MPI_BAND    (MPI_Op)(0x58000006)
#define MPI_LOR     (MPI_Op)(0x58000007)
#define MPI_BOR     (MPI_Op)(0x58000008)
#define MPI_LXOR    (MPI_Op)(0x58000009)
#define MPI_BXOR    (MPI_Op)(0x5800000a)
#define MPI_MINLOC  (MPI_Op)(0x5800000b)
#define MPI_MAXLOC  (MPI_Op)(0x5800000c)
// There are more, but MPI-lite does not support them now.


unsigned long get_mpi_datatype(string str)
{
    unsigned long type;
    if (strncmp(str.c_str(), "((MPI_Datatype )", 16) == 0)
    {
        string substr = str.substr(16, 10);
        unsigned long orig = strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_CHAR:
            type=MPILITE_CHAR;
            break;
          case MPI_SIGNED_CHAR:
            type=MPILITE_SIGNED_CHAR;
            break;
          case MPI_UNSIGNED_CHAR:
            type=MPILITE_UNSIGNED_CHAR;
            break;
          case MPI_BYTE:
            type=MPILITE_CHAR;
            break;
          case MPI_SHORT:
            type=MPILITE_SHORT;
            break;
          case MPI_UNSIGNED_SHORT:
            type=MPILITE_UNSIGNED_SHORT;
            break;
          case MPI_INT:
            type=MPILITE_INT;
            break;
          case MPI_UNSIGNED:
            type=MPILITE_UNSIGNED;
            break;
          case MPI_LONG:
            type=MPILITE_LONG;
            break;
          case MPI_UNSIGNED_LONG:
            type=MPILITE_UNSIGNED_LONG;
            break;
          case MPI_FLOAT:
            type=MPILITE_FLOAT;
            break;
          case MPI_DOUBLE:
            type=MPILITE_DOUBLE;
            break;
          case MPI_LONG_LONG_INT:
            type=MPILITE_LONG_LONG_INT;
            break;
          case MPI_UNSIGNED_LONG_LONG:
            type=MPILITE_UNSIGNED_LONG_LONG;
            break;
          case MPI_DOUBLE_INT:
            type=MPILITE_DOUBLE_INT;
            break;
          default:
            printf("ERROR: get_mpi_datatype(): Unsupported datatype.\n");
            exit(-1);
        }
        return type;
    }
    else
    {
        printf("ERROR: get_mpi_datatype(): Internal error.\n");
        exit(-1);
    }
}


unsigned long get_mpi_comm(string str)
{
    unsigned long comm=0;
    if (strncmp(str.c_str(), "((MPI_Comm )", 12) == 0)
    {
        string substr = str.substr(12, 10);
        unsigned long orig=strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_COMM_WORLD:
            comm=MPILITE_COMM_WORLD;
            break;
          case MPI_COMM_SELF:
            comm=MPILITE_COMM_SELF;
            break;
          default:
            printf("ERROR: get_mpi_comm(): Unsupported COMM.\n");
            exit(-1);
        }
        return comm;
    }
    else
    {
        printf("ERROR: get_mpi_comm(): Internal error.\n");
        exit(-1);
    }
}


unsigned long get_mpi_op(string str)
{
    unsigned long op=0;
    if (strncmp(str.c_str(), "((MPI_Op )", 10) == 0)
    {
        string substr = str.substr(10, 10);
        unsigned long orig=strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_MAX:
            op=MPILITE_MAX;
            break;
          case MPI_MIN:
            op=MPILITE_MIN;
            break;
          case MPI_SUM:
            op=MPILITE_SUM;
            break;
          case MPI_PROD:
            op=MPILITE_PROD;
            break;
          case MPI_LAND:
            op=MPILITE_LAND;
            break;
          case MPI_BAND:
            op=MPILITE_BAND;
            break;
          case MPI_LOR:
            op=MPILITE_LOR;
            break;
          case MPI_BOR:
            op=MPILITE_BOR;
            break;
          case MPI_LXOR:
            op=MPILITE_LXOR;
            break;
          case MPI_BXOR:
            op=MPILITE_BXOR;
            break;
          case MPI_MINLOC:
            op=MPILITE_MINLOC;
            break;
          case MPI_MAXLOC:
            op=MPILITE_MAXLOC;
            break;
          default:
            printf("ERROR: get_mpi_op(): Unsupported MPI_Op.\n");
            exit(-1);
        }
        return op;
    }
    else
    {
        printf("ERROR: get_mpi_op(): Internal error.\n");
        exit(-1);
    }
}

