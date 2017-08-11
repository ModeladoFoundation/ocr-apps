// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

#ifndef __PROTO_H_
#define __PROTO_H_

#define ENABLE_EXTENSION_LABELING

#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"

#include <stddef.h>
#include "ocrAppUtils.h"

#define ENABLE_EXTENSION_LABELING

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

typedef struct {
    int irefine;
    int dummy;
} continuationPRM_t;

typedef struct {
    int irefine, ts;
} refineLoopPRM_t;

typedef refineLoopPRM_t reduceBlockCountsPRM_t;
typedef refineLoopPRM_t updateBlockCountsPRM_t;

typedef struct {
    int istage, ts;
} stageLoopPRM_t;

typedef struct {
    int istart, istage, ts;
    int dummy;
} varsLoopPRM_t;

typedef struct {
    int iAxis, istart, iend, istage, ts;
    int dummy;
} commPRM_t;

typedef struct {
    int istart, iend, istage, ts;
} calcLoopPRM_t;

typedef struct {
    int ts;
    int dummy;
} loadbalancePRM_t;

typedef struct {
    int irefine, ts, phase, r;
    int number, dummy;
} reducePRM_t;

typedef loadbalancePRM_t timestepLoopPRM_t;

typedef varsLoopPRM_t calcPRM_t;

typedef calcLoopPRM_t checkSumLoopPRM_t;

typedef calcPRM_t checkSumPRM_t;

typedef checkSumPRM_t printPRM_t;

typedef struct {
    int irefine;
    int flag;
} createChildBlocksPRM_t;

typedef struct {
    int irefine;
    int ilevel;
    int phase;
    int iter;
} refineLevelsPRM_t;

typedef refineLevelsPRM_t scatterRefinePRM_t;

typedef struct {
    int irefine;
    int iAxis;
    int flag;
    int dummy;
} commRefnNbrsPRM_t;

typedef struct {
    int irefine;
    int flag;
    int child_refine;
    int dummy;
} commRefnSibsPRM_t;

typedef struct {
    int irefine;
    int dummy;
} splitBlocksPRM_t;

typedef struct {
    int irefine;
    int dummy;
} refineAllLevelsPRM_t;

typedef struct {
    int ts;
    int dummy;
} redistributeblocksPRM_t;

typedef struct {
    int ts;
    int dummy;
} movePRM_t;

typedef commRefnSibsPRM_t exchangeDataUnclesPRM_t;

// main.c
void print_help_message(void);
void allocate(void);
void deallocate(void);
int check_input(Command cmd);
Command parseCommandLine(int argc, char** argv);
void initGlobalOcrParamH( globalOcrParamH_t* ocrParamH, u64 nRanks, Command* cmd );
void initCommand(Command* cmd);

ocrGuid_t wrapUpEdt( EDT_ARGS );
ocrGuid_t initEdt( EDT_ARGS );

// block.c
_OCR_TASK_FNC_( FNC_splitBlocks );
_OCR_TASK_FNC_( FNC_createChildBlocks );
_OCR_TASK_FNC_( FNC_createChildBlocks1 );
_OCR_TASK_FNC_( FNC_joinChildBlocks );
_OCR_TASK_FNC_( FNC_continuation );
void split_blocks(rankH_t* PTR_rankH, rankH_t* PTR_children_rankHs[8], int irefine);
void consolidate_blocks(rankH_t* PTR_rankH, rankH_t* PTR_children_rankHs[8], int irefine);

//load_balance.c
_OCR_TASK_FNC_( FNC_loadbalance );
_OCR_TASK_FNC_( FNC_redistributeblocks );
_OCR_TASK_FNC_( FNC_idgather );
void printGatheredBlockIDs( rankH_t* PTR_rankH, int* blockids_gathered );
int mapBlocktoPD( rankH_t* PTR_rankH, int lb_opt );
u64 mortonZvalue(unsigned int x, unsigned int y, unsigned int z, unsigned int ilevel);

// check_sum.c
void check_sum( ocrDBK_t DBK_rankH, rankH_t* PTR_rankH, ocrDBK_t DBK_octTreeRedH, octTreeRedH_t* PTR_octTreeRedH, int ts, int istage, int var, ocrDBK_t DBK_gridSum_in, double* sum );

// comm_block.c
void comm_proc(void);

// comm.c
_OCR_TASK_FNC_( FNC_comm );
_OCR_TASK_FNC_( commHaloNbrsEdt );
_OCR_TASK_FNC_( packHalosEdt );
_OCR_TASK_FNC_( unpackHalosEdt );
void reset_haloSendPTRs( rankH_t *PTR_rankH, int iAxis, int phase );
void reset_haloRecvPTRs( rankH_t *PTR_rankH, int iAxis, int phase );
void pack_face( rankH_t* PTR_rankH, double *send_buf, int face_case, int dir, int start, int num_comm );
void unpack_face( rankH_t* PTR_rankH, double *recv_buf, int face_case, int dir, int start, int num_comm );
void apply_bc( rankH_t* PTR_rankH, int l, int start, int num_comm );

// comm_parent.c
ocrGuid_t commRefnSibsEdt(EDT_ARGS);
ocrGuid_t packRefnBufsSibsEdt(EDT_ARGS);
ocrGuid_t unpackRefnBufsSibsEdt(EDT_ARGS);
void reset_refnRecvSibsPTRs( rankH_t *PTR_rankH, int phase, s64 flag );
void reset_refnSendSibsPTRs( rankH_t *PTR_rankH, int phase );

// comm_refine.c
int blockNnbrs(block *bp, int iAxis);
void reset_refnSendPTRs( rankH_t *PTR_rankH, int iAxis, int phase );
ocrGuid_t commRefnNbrsEdt(EDT_ARGS);
ocrGuid_t exchangeDataEdt(EDT_ARGS);
ocrGuid_t packRefnBufsEdt(EDT_ARGS);
ocrGuid_t unpackRefnBufsEdt(EDT_ARGS);

// comm_util.c
int return_buf_size(rankH_t* PTR_rankH, int dir, int fcase, int sendOrRecvFlag);
size_t reductionDBKsize( int reductionHandle, rankH_t* PTR_rankH );
void reductionHandle_update( int reductionHandle, rankH_t* PTR_rankH, reductionPrivate_t* PTR_redH);

// driver.c
_OCR_TASK_FNC_( FNC_timestepLoop );
_OCR_TASK_FNC_( FNC_stageLoop );
_OCR_TASK_FNC_( FNC_stage );
_OCR_TASK_FNC_( FNC_varsLoop );
_OCR_TASK_FNC_( FNC_vars );
_OCR_TASK_FNC_( FNC_calcLoop );
_OCR_TASK_FNC_( FNC_checkSumLoop );
_OCR_TASK_FNC_( FNC_checkSum );
_OCR_TASK_FNC_( FNC_print );
_OCR_TASK_FNC_( FNC_finalize );
_OCR_TASK_FNC_( FNC_finalizeBarrier );

// init.c
void init(rankH_t* PTR_rankH, octTreeRedH_t* PTR_octTreeRedH);
void initTemplates(rankH_t* PTR_rankH);
void initDBKs( rankH_t* PTR_rankH, octTreeRedH_t* PTR_octTreeRedH );

// move.c
_OCR_TASK_FNC_( FNC_move );
void move( rankH_t* PTR_rankH );
void mark_refinementIntention( rankH_t* PTR_rankH, u64 irefine );
void check_objects( rankH_t* PTR_rankH );
int check_block(rankH_t* PTR_rankH, double cor[3][2]);

// plot.c
void plot(int);

// profile.c
void init_profile(rankH_t* PTR_rankH);
void ma_malloc_counter( timerH_t* PTR_timerH, void* ptr, size_t size, char *file, int line);

// refine.c
_OCR_TASK_FNC_( FNC_refineLoop );
_OCR_TASK_FNC_( FNC_refine );
_OCR_TASK_FNC_( FNC_refineAllLevels );
_OCR_TASK_FNC_( FNC_refine1AllLevels );
_OCR_TASK_FNC_( FNC_refine1Level );
_OCR_TASK_FNC_( FNC_bcast );
_OCR_TASK_FNC_( FNC_scatterRefine );
_OCR_TASK_FNC_( FNC_reduceBlockCounts );
_OCR_TASK_FNC_( FNC_updateBlockCounts );
int refine_level1(rankH_t* PTR_rankH, int ilevel);
int refine_level2(rankH_t* PTR_rankH, int ilevel);
int refine_level_sibsAndnbrs1(block* bp, int level);
int refine_level_sibsAndnbrs2(block* bp, int level);
int refine_level(void);
void reset_all(rankH_t* PTR_rankH);
void reset_neighbors(void);
void redistribute_blocks(double *, double *, double *, double *, int *, int);

// stencil.c
void stencil_calc(int);
_OCR_TASK_FNC_( FNC_calc );

// util.c
double timer(void);
void ma_malloc_counter( timerH_t* PTR_timerH, void* ptr, size_t size, char *file, int line);
void transferTimer( timerH_t* dst, timerH_t* src );
void resetTimer( timerH_t* PTR_timerH );
int binary_search_double( int n, int quarry, int * A);
void sortArray( int n, int *A);
_OCR_TASK_FNC_( FNC_reduceAllUp );
_OCR_TASK_FNC_( FNC_reduceAllDown );
_OCR_TASK_FNC_( FNC_accumulator );
_OCR_TASK_FNC_( FNC_reduceAllRootNodes );

#endif
