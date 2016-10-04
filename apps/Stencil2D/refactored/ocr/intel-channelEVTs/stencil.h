#include "ocr.h"
#include "ocr-std.h"

#define _OCR_TASK_FNC_(X) ocrGuid_t X( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )

#ifdef DEBUG_APP
    #define DEBUG_PRINTF(X) PRINTF X
#else
    #define DEBUG_PRINTF(X) do {} while(0)
#endif

//default values
#define NPOINTS 1000
#define NRANKS 16
#define NTIMESTEPS 10

//Compile time value (if not defined at compile time, set default)
#ifndef HALO_RADIUS
    #define HALO_RADIUS 2
#endif

#define EPSILON 1e-8

#ifndef FULL_APP
//#define FULL_APP 0 //To measure only the OCR overhead
#define FULL_APP 1 //To measure the full app performance
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

#define WEIGHT(ii,jj) weight[jj+HALO_RADIUS+(ii+HALO_RADIUS)*(2*HALO_RADIUS+1)]   //To match the legacy PRK kernel
#define INDEXIN(i,j) ( (i+HALO_RADIUS)+(j+HALO_RADIUS)*(np_x+2*HALO_RADIUS) )
#define INDEXOUT(i,j) ( (i)+(j)*(np_x) )
#define IN(i,j) xIn[ INDEXIN(i-ib,j-jb) ]
#define OUT(i,j) xOut[ INDEXOUT(i-ib,j-jb) ]

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

typedef struct
{
    s64 NP, NR; //global problem size, number of ranks
    s64 NP_X;
    s64 NR_X;
    s64 NP_Y;
    s64 NR_Y; //ranks in x and y directions
    s64 NT;
} globalCmdParamH_t;

typedef struct
{
    s64 id;
    s64 id_x, ib, ie, np_x;
    s64 id_y, jb, je, np_y;
    s64 itimestep;
} rankParamH_t;

typedef struct
{
    ocrGuid_t TML_FNC_Lsend;
    ocrGuid_t TML_FNC_Rsend;
    ocrGuid_t TML_FNC_Lrecv;
    ocrGuid_t TML_FNC_Rrecv;
    ocrGuid_t TML_FNC_Bsend;
    ocrGuid_t TML_FNC_Tsend;
    ocrGuid_t TML_FNC_Brecv;
    ocrGuid_t TML_FNC_Trecv;
    ocrGuid_t TML_FNC_update;
    ocrGuid_t TML_timestep;
    ocrGuid_t TML_timestepLoop;
} rankTemplateH_t;

typedef struct
{
    ocrGuid_t haloRangeGUID, normReductionRangeGUID, timerReductionRangeGUID;
    //ocrGuid_t EVT_OUT_norm_reduction, EVT_OUT_timer_reduction;
} globalOcrParamH_t;

typedef struct
{
    globalCmdParamH_t cmdParamH;
    globalOcrParamH_t ocrParamH;

} globalParamH_t;

#ifdef STENCIL_WITH_DBUF_CHRECV
#define NB_SEND_CHANNELS 8
#define NB_RECV_CHANNELS 8
#else
#define NB_SEND_CHANNELS 4
#define NB_RECV_CHANNELS 4
#endif

typedef struct
{
    globalParamH_t globalParamH;
    rankParamH_t rankParamH;
    rankTemplateH_t rankTemplateH;

    ocrGuid_t haloSendEVTs[NB_SEND_CHANNELS];
    ocrGuid_t haloRecvEVTs[NB_RECV_CHANNELS];

    ocrGuid_t DBK_xIn, DBK_xOut, DBK_weight;
    ocrGuid_t DBK_LsendBufs[2], DBK_RsendBufs[2];
    ocrGuid_t DBK_TsendBufs[2], DBK_BsendBufs[2];
    ocrGuid_t DBK_refNorm;

    ocrGuid_t DBK_timers;
    ocrGuid_t DBK_norm_reductionH; //->reductionPrivate_t
    ocrGuid_t DBK_timer_reductionH;

    ocrGuid_t EVT_OUT_norm_reduction, EVT_OUT_timer_reduction;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
    ocrGuid_t haloRangeGUID;
} rankH_t;

static void timestamp(const char* msg);
void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e);
void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id);
void splitDimension(s64 Num_procs, s64 *Num_procsx, s64 *Num_procsy);
static inline int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y );
static inline int getPolicyDomainID( int b, s64* edtGridDims, s64* pdGridDims );

static void timestamp(const char* msg)
{
#ifdef TG_ARCH
  PRINTF(msg);
#else
  time_t t= time(NULL);
  char* timeString = ctime(&t);
  timeString[24] = '\0';
  PRINTF("%s: %s\n", timeString, msg);
#endif
}

void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e)
{
    s64 N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;
}

void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id)
{
    s64 N = ub_g - lb_g + 1;
    s64 s, e;

    s64 r;

    for( r = 0; r < R; r++ )
    {
        s = r*N/R + lb_g;
        e = (r+1)*N/R + lb_g - 1;
        if( s <= i && i <= e )
            break;
    }

    *id = r;
}

void splitDimension(s64 Num_procs, s64* Num_procsx, s64* Num_procsy)
{
    s64 nx, ny;

    nx = (int) sqrt(Num_procs+1);
    for(; nx>0; nx--)
    {
        if (!(Num_procs%nx))
        {
            ny = Num_procs/nx;
            break;
        }
    }
    *Num_procsx = nx; *Num_procsy = ny;
}

static inline int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}

static inline int getPolicyDomainID( int b, s64* edtGridDims, s64* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = b/edtGridDims[0];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];

    s64 pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    s64 pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);

    //Each edt, with id=b, is mapped to a PD. The mapping is similar to how the link cells map to
    //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
    //And, a 3-D subgrid of edts is mapped to a PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords(pd_x, pd_y, PD_X, PD_Y);

    return mapToPD;
}
