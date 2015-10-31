#include "ocr.h"
#include "ocr-std.h"

#define _OCR_TASK_FNC_(X) ocrGuid_t X( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )

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

#ifndef PROBLEM_TYPE
#define PROBLEM_TYPE 2
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

#if PROBLEM_TYPE==2

#define WEIGHT(ii,jj) weight[jj+HALO_RADIUS+(ii+HALO_RADIUS)*(2*HALO_RADIUS+1)]   //To match the legacy PRK kernel
#define INDEXIN(i,j) ( (i+HALO_RADIUS)+(j+HALO_RADIUS)*(np_x+2*HALO_RADIUS) )
#define INDEXOUT(i,j) ( (i)+(j)*(np_x) )
#define IN(i,j) xIn[ INDEXIN(i-ib,j-jb) ]
#define OUT(i,j) xOut[ INDEXOUT(i-ib,j-jb) ]

#elif PROBLEM_TYPE==1

#define WEIGHT(ii) weight[ii+HALO_RADIUS]   //To match the legacy PRK kernel
#define INDEXIN(i) ( i+HALO_RADIUS )
#define INDEXOUT(i) ( i )
#define IN(i) xIn[ INDEXIN(i-ib) ]
#define OUT(i) xOut[ INDEXOUT(i-ib) ]

#endif

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
    #if PROBLEM_TYPE==2
    s64 NP_Y;
    s64 NR_Y; //ranks in x and y directions
    #endif
    s64 NT;
} globalParamH_t;

typedef struct
{
    s64 id;
    s64 id_x, ib, ie, np_x;
    #if PROBLEM_TYPE==2
    s64 id_y, jb, je, np_y;
    #endif
    s64 itimestep;
} rankParamH_t;

typedef struct
{
    ocrGuid_t DBK_xIn, DBK_xOut, DBK_weight;
    ocrGuid_t DBK_LsendBufs[2], DBK_RsendBufs[2];
    #if PROBLEM_TYPE==2
    ocrGuid_t DBK_TsendBufs[2], DBK_BsendBufs[2];
    #endif
    ocrGuid_t DBK_refNorm;
} rankDataH_t;

typedef struct
{
    ocrGuid_t EVT_Lsend_fin, EVT_Rsend_fin;
    ocrGuid_t EVT_Lrecv_start, EVT_Rrecv_start;
    ocrGuid_t EVT_Lrecv_fin, EVT_Rrecv_fin;
    #if PROBLEM_TYPE==2
    ocrGuid_t EVT_Bsend_fin, EVT_Tsend_fin;
    ocrGuid_t EVT_Brecv_start, EVT_Trecv_start;
    ocrGuid_t EVT_Brecv_fin, EVT_Trecv_fin;
    #endif
    ocrGuid_t EVT_IN_reduction;
} rankEventH_t;

typedef struct
{
    ocrGuid_t TML_FNC_Lsend;
    ocrGuid_t TML_FNC_Rsend;
    ocrGuid_t TML_FNC_Lrecv;
    ocrGuid_t TML_FNC_Rrecv;
    #if PROBLEM_TYPE==2
    ocrGuid_t TML_FNC_Bsend;
    ocrGuid_t TML_FNC_Tsend;
    ocrGuid_t TML_FNC_Brecv;
    ocrGuid_t TML_FNC_Trecv;
    #endif
    ocrGuid_t TML_FNC_update;
} rankTemplateH_t;

typedef struct
{
    ocrGuid_t DBK_rankParamH; //-->rankParamH_t
    ocrGuid_t DBK_rankDataH; //--> rankDataH_t
    ocrGuid_t DBK_rankEventHs[2]; //--> rankEventH_t[2]
    ocrGuid_t DBK_rankTemplateH;
} rankH_t;

typedef struct
{
    ocrGuid_t DBK_globalParamH; // Commandline parameters/param

    s64 itimestep; //tag
    ocrGuid_t DBK_timers;

    //Reduction is implemented using an "reduction EDT" that depends on NumRanks #of
    //input events and the final result datablock will be passed along an output event
    //Each subdomain EDT satisfies the corresponding input reduction event
    ocrGuid_t DBK_EVT_IN_reduction; //EVT_IN_reduction[NumRanks]
    ocrGuid_t EVT_OUT_reduction;    //Reduction output event

    ocrGuid_t DBK_rankHs; //--> rankH_t[]
} globalH_t;

static void timestamp(const char* msg);
void partition_bounds(s64 id, s64 lb_g, s64 ub_g, s64 R, s64* s, s64* e);
int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y );

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

int globalRankFromCoords( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}
