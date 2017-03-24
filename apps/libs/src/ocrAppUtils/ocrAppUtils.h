#ifndef __OCRAPPUTILS_HEADER_H__
#define __OCRAPPUTILS_HEADER_H__

#include "ocr.h"

#define _OCR_TASK_FNC_(X) ocrGuid_t X( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
#define EDT_ARGS u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]
#define MOD(a,b) ((((a)%(b))+(b))%(b))

#ifdef DEBUG_APP
    #define DEBUG_PRINTF(X) PRINTF X
#else
    #define DEBUG_PRINTF(X) do {} while(0)
#endif

typedef ocrGuid_t ocrDBK_t;
typedef ocrGuid_t ocrEVT_t;
typedef ocrGuid_t ocrEDT_t;
typedef ocrGuid_t ocrTML_t;

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} MyOcrTaskStruct_t;

typedef struct
{
    u64 id;
    u64 edtGridDims[1];
} PRM_init1dEdt_t;

typedef struct
{
    u64 id;
    u64 edtGridDims[2];
} PRM_init2dEdt_t;

typedef struct
{
    u64 id;
    u64 edtGridDims[3];
} PRM_init3dEdt_t;

void createEventHelper(ocrGuid_t * evtGuid, u32 nbDeps);
void getAffinityHintsForDBandEdt( ocrHint_t* PTR_myDbkAffinityHNT, ocrHint_t* PTR_myEdtAffinityHNT );

void partition_bounds(u64 id, u64 lb_g, u64 ub_g, u64 R, u64* s, u64* e);
void getPartitionID(u64 i, u64 lb_g, u64 ub_g, u64 R, u64* id);

void splitDimension_Cart2D(u64 Num_procs, u64* Num_procsx, u64* Num_procsy);
void splitDimension_Cart3D(u64 Num_procs, u64* Num_procsx, u64* Num_procsy, u64* Num_procsz);

int globalRankFromCoords_Cart1D( int id_x, int NR_X );
int globalRankFromCoords_Cart2D( int id_x, int id_y, int NR_X, int NR_Y );
int globalRankFromCoords_Cart3D( int id_x, int id_y, int id_z, int NR_X, int NR_Y, int NR_Z );

void globalCoordsFromRank_Cart1D( int id, int NR_X, int* id_x );
void globalCoordsFromRank_Cart2D( int id, int NR_X, int NR_Y, int* id_x, int* id_y );
void globalCoordsFromRank_Cart3D( int id, int NR_X, int NR_Y, int NR_Z, int* id_x, int* id_y, int* id_z );

void forkSpmdEdts_Cart1D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv );
void forkSpmdEdts_Cart2D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv );
void forkSpmdEdts_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv );

void forkSpmdEdts_staticScheduler_Cart2D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv );
void forkSpmdEdts_staticScheduler_Cart3D( ocrGuid_t (*initEdt)(u32, u64*, u32, ocrEdtDep_t*), u64* edtGridDims, ocrGuid_t* spmdDepv );

#endif
