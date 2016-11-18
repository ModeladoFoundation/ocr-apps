#include "ocr.h"
#include "ocr-std.h"

#include "ocrAppUtils.h"

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
    ocrGuid_t spmdJoinReductionRangeGUID;

    ocrGuid_t EVT_OUT_spmdJoin_reduction;
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
    ocrGuid_t DBK_spmdJoin_reductionH;

    ocrGuid_t EVT_OUT_norm_reduction, EVT_OUT_timer_reduction;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
    ocrGuid_t haloRangeGUID;
} rankH_t;

static void timestamp(const char* msg);
