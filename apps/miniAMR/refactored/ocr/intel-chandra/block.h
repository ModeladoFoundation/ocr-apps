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

#ifndef __BLOCK_H_
#define __BLOCK_H_

#include "ocrAppUtils.h"
#include "reduction.h"

#include "param.h"
#include "timer.h"

#define INDEXIN(var,i,j,k) ( (k)+(j)*(z_block_size+2)+(i)*(y_block_size+2)*(z_block_size+2)+(var)*(x_block_size+2)*(y_block_size+2)*(z_block_size+2) )
#define IN(bp,var,i,j,k) bp->array[ INDEXIN(var,i,j,k) ]

#define WORK(bp,i,j,k) bp->work[ INDEXIN(0,i,j,k) ]

//-----------------------------------
// DO NOT CHANGE THESE VALUES
// THE CODE WILL BREAK OTHERWISE
#define REFINE (1)
#define STAY (0)
#define COARSEN (-1)
//-----------------------------------

#define BLOCKCOUNT_RED_HANDLE_LB (0)
#define BLOCKCOUNT_RED_HANDLE_UB (BLOCKCOUNT_RED_HANDLE_LB+1)
#define REFINEINTENT_RED_HANDLE_LB (BLOCKCOUNT_RED_HANDLE_UB+1)
#define REFINEINTENT_RED_HANDLE_UB (REFINEINTENT_RED_HANDLE_LB+MAX_REFINE_LEVELS-1)
#define QUIESCENCE_RED_HANDLE_LB (REFINEINTENT_RED_HANDLE_UB+1)
#define QUIESCENCE_RED_HANDLE_UB (QUIESCENCE_RED_HANDLE_LB+MAX_REFINE_LEVELS-1)
#define CHECKSUM_RED_HANDLE_LB (QUIESCENCE_RED_HANDLE_UB+1)
#define CHECKSUM_RED_HANDLE_UB (CHECKSUM_RED_HANDLE_LB+2-1)
#define COARSEN_RED_HANDLE_LB (CHECKSUM_RED_HANDLE_UB+1)
#define COARSEN_RED_HANDLE_UB (COARSEN_RED_HANDLE_LB+2-1)
#define FINALIZE_RED_HANDLE_LB (COARSEN_RED_HANDLE_UB+1)
#define FINALIZE_RED_HANDLE_UB (FINALIZE_RED_HANDLE_LB+2-1)
#define BLOCKIDGATHER_RED_HANDLE_LB (FINALIZE_RED_HANDLE_UB+1)
#define BLOCKIDGATHER_RED_HANDLE_UB (BLOCKIDGATHER_RED_HANDLE_LB+2-1)
#define MAX_REDUCTION_HANDLES (BLOCKIDGATHER_RED_HANDLE_UB+1)

typedef struct {
   int number;      //Global id of the block
   int level;
   int refine;      // 1 - refine; 0 - may coarsen or unchanged; -1 - edge levels
   int new_proc;
   int parent;       // if original block -1,
                     // else if on node, number in structure
                     // else (-2 - parent->number)
   int parent_node;
   int child_number;
   int nei_refine[6];
   int nei_refine_recv[6][4];
   int nei_level[6];  /* 0 to 5 = W, E, S, N, D, U; use -2 for boundary */
   int nei[6][2][2];  /* negative if off processor (-1 - proc) */

   int sib_refine[8], sib_refine_recv[8];
   int sib_level[8];

   int cen[3];
   double *array;  //((x_block_size+2)*(y_block_size+2)*(z_block_size+2);
   double* work;
   //Baseline code - the above is not a contiguous array!!
   //OCR code - contiguous array is used due to ease of implementation. Non-continues array
   //implies array of DBs which makes it cumbersome.
   ocrDBK_t DBK_array;
   ocrDBK_t DBK_work;
} block;

typedef struct {
   int type;
   int bounce;
   double cen[3];
   double orig_cen[3];
   double move[3];
   double orig_move[3];
   double size[3];
   double orig_size[3];
   double inc[3];
} object;

typedef struct {
    int max_num_blocks;
    int target_active;
    int num_refine;
    int uniform_refine;
    int x_block_size, y_block_size, z_block_size;
    int num_vars;
    int comm_vars;
    int init_block_x, init_block_y, init_block_z;
    int reorder;
    int npx, npy, npz;
    int inbalance;
    int refine_freq;
    int report_diffusion;
    int error_tol;
    int num_tsteps;
    int stencil;
    int report_perf;
    int plot_freq;
    int num_objects;
    int checksum_freq;
    int target_max;
    int target_min;
    int stages_per_ts;
    int lb_opt;
    int block_change;
    int code;
    int permute;
    int nonblocking;
    int refine_ghost;
    object objects[MAX_OBJECTS];
} Command;

typedef struct
{
    ocrGuid_t haloRangeGUID[MAX_REFINE_LEVELS];
    ocrGuid_t redUpRangeGUID[MAX_REDUCTION_HANDLES];
    ocrGuid_t initRedRangeGUID;
    ocrEVT_t EVT_OUT_spmdJoin_reduction;
} globalOcrParamH_t;

typedef struct
{
    Command cmdParamH;
    globalOcrParamH_t ocrParamH;
} globalParamH_t;

typedef struct
{
    ocrTML_t TML_refine, TML_refineLoop;
    ocrTML_t TML_reduceBlockCounts, TML_updateBlockCounts;
    ocrTML_t TML_commRefnNbrs, exchangeDataTML, packRefnBufsTML, unpackRefnBufsTML;
    ocrTML_t TML_commRefnSibs, packRefnBufsSibsTML, unpackRefnBufsSibsTML;

    ocrTML_t TML_refineAllLevels, TML_refine1AllLevels, TML_refine1Level;
    ocrTML_t TML_reduceAllUp, TML_accumulator, TML_reduceAllRootNodes, TML_reduceAllDown;
    ocrTML_t TML_bcast;
    ocrTML_t TML_scatterRefine;

    ocrTML_t TML_splitBlocks, TML_createChildBlocks, TML_createChildBlocks1;
    ocrTML_t TML_continuation;
    ocrTML_t TML_timestepLoop, TML_stageLoop, TML_stage;
    ocrTML_t TML_loadbalance, TML_idgather, TML_redistributeblocks;
    ocrTML_t TML_move;
    ocrTML_t TML_varsLoop, TML_vars;
    ocrTML_t TML_comm;
    ocrTML_t TML_calcLoop, TML_calc;
    ocrTML_t TML_checkSumLoop, TML_checkSum;
    ocrTML_t TML_print, TML_finalize, TML_finalizeBarrier;

    ocrTML_t commHaloNbrsTML, packHalosTML, unpackHalosTML;
} rankTemplateH_t;

#define NUM_NBR_CHANNELS 6
#define NUM_PARENT_CHANNELS 1
#define NUM_CHILDREN_CHANNELS 8
#define NUM_SIBLINGS_CHANNELS 8

typedef struct
{
    ocrEVT_t haloCurrSendEVTs[NUM_NBR_CHANNELS];
    ocrEVT_t haloCoarSendEVTs[NUM_NBR_CHANNELS];
    ocrEVT_t haloRefnSendEVTs[NUM_NBR_CHANNELS][4];

    ocrEVT_t haloCurrRecvEVTs[NUM_NBR_CHANNELS];
    ocrEVT_t haloCoarRecvEVTs[NUM_NBR_CHANNELS];
    ocrEVT_t haloRefnRecvEVTs[NUM_NBR_CHANNELS][4];

    ocrDBK_t haloCurrSendDBKs[NUM_NBR_CHANNELS][2];
    ocrDBK_t haloCoarSendDBKs[NUM_NBR_CHANNELS][2];
    ocrDBK_t haloRefnSendDBKs[NUM_NBR_CHANNELS][4][2];

    double* haloCurrSendPTRs[NUM_NBR_CHANNELS][2];
    double* haloCoarSendPTRs[NUM_NBR_CHANNELS][2];
    double* haloRefnSendPTRs[NUM_NBR_CHANNELS][4][2];

    double* haloCurrRecvPTRs[NUM_NBR_CHANNELS][2];
    double* haloCoarRecvPTRs[NUM_NBR_CHANNELS][2];
    double* haloRefnRecvPTRs[NUM_NBR_CHANNELS][4][2];

    ocrDBK_t refnCurrSendDBKs[NUM_NBR_CHANNELS][2]; //double-buffered DBKs
    ocrDBK_t refnCoarSendDBKs[NUM_NBR_CHANNELS][2];
    ocrDBK_t refnRefnSendDBKs[NUM_NBR_CHANNELS][4][2];

    int* refnCurrSendPTRs[NUM_NBR_CHANNELS][2];
    int* refnCoarSendPTRs[NUM_NBR_CHANNELS][2];
    int* refnRefnSendPTRs[NUM_NBR_CHANNELS][4][2];

    int* refnCurrRecvPTRs[NUM_NBR_CHANNELS][2];
    int* refnCoarRecvPTRs[NUM_NBR_CHANNELS][2];
    int* refnRefnRecvPTRs[NUM_NBR_CHANNELS][4][2];

    ocrDBK_t refnCurrSendSibsDBKs[NUM_CHILDREN_CHANNELS][2];
    int* refnCurrSendSibsPTRs[NUM_CHILDREN_CHANNELS][2];
    int* refnCurrRecvSibsPTRs[NUM_CHILDREN_CHANNELS][2];

    ocrEVT_t haloParentSendEVTs[NUM_PARENT_CHANNELS]; //one parent
    ocrEVT_t haloChildrenSendEVTs[NUM_CHILDREN_CHANNELS]; //8 children
    ocrEVT_t haloSiblingsSendEVTs[NUM_SIBLINGS_CHANNELS]; //8 siblings including self

    ocrEVT_t haloParentRecvEVTs[NUM_PARENT_CHANNELS]; //one parent
    ocrEVT_t haloChildrenRecvEVTs[NUM_CHILDREN_CHANNELS]; //8 children
    ocrEVT_t haloSiblingsRecvEVTs[NUM_SIBLINGS_CHANNELS]; //8 siblings including self

} doubleBufferedOcrObj_t;

typedef struct{
    ocrDBK_t DBK_in, DBK_out;
    ocrEVT_t upIEVT, upOEVT, downIEVT, downOEVT;
} redObjects_t;

typedef struct {
    int number; //active or inactive node
    int ilevel, myRank, isibling;
    ocrTML_t TML_reduceAllUp, TML_reduceAllDown, TML_reduceAllRootNodes, TML_bcast, TML_accumulator;

    ocrDBK_t DBK_octTreeRedH;//self-referencial DBK GUID
    ocrDBK_t parentredH;
    ocrDBK_t childrenredH[8];
    ocrDBK_t siblingsredH[8];

    int size[MAX_REDUCTION_HANDLES];
    redObjects_t blockRedObjects[MAX_REDUCTION_HANDLES];
    redObjects_t parentRedObjects[MAX_REDUCTION_HANDLES];
    redObjects_t childrenRedObjects[8][MAX_REDUCTION_HANDLES];
    redObjects_t siblingsRedObjects[8][MAX_REDUCTION_HANDLES];

    ocrDBK_t DBK_redRootH[MAX_REDUCTION_HANDLES];
} octTreeRedH_t;

typedef struct
{
    ocrGuid_t DBK_rankH;

    ocrEVT_t currRankDBKs[6];
    ocrEVT_t coarRankDBKs[6];
    ocrEVT_t refnRankDBKs[6][4];

    ocrDBK_t parentRankDBK;
    ocrDBK_t childrenRankDBKs[8];
    ocrDBK_t siblingsRankDBKs[8];

    int myRank_g;
    int parent_myRank_g;

    //Reduction range guids: Double buffered
    ocrGuid_t childrenredRangeGUID[2];
    ocrGuid_t siblingredRangeGUID[2];

    ocrEVT_t initRedOEVT;

    ocrDBK_t DBK_octTreeRedH;
    octTreeRedH_t octTreeRedH;

    //ocrDBK_t parentredH;
    //ocrDBK_t childrenredH[8];
    //ocrDBK_t siblingsredH[8];

    //redObjects_t blockRedObjects[MAX_REDUCTION_HANDLES];
    //redObjects_t parentRedObjects[MAX_REDUCTION_HANDLES];
    //redObjects_t childrenRedObjects[8][MAX_REDUCTION_HANDLES];
    //redObjects_t siblingsRedObjects[8][MAX_REDUCTION_HANDLES];


    doubleBufferedOcrObj_t doubleBufferedOcrObjH[2];
} sharedOcrObj_t;

typedef struct {
    int my_pe;
    int num_pes;

    int ilevel;
    int myRank, nRanks; //rank and nranks at ilevel
    int iblock, jblock, kblock; //local coordinates of the block in each partition

    int iblock_g, jblock_g, kblock_g; //global coordinates of the block in ilevel
    int gx, gy, gz; //block grid size at ilevel
    int myRank_g; //global id of the block

    u64 zValue;
    u64 zValue_parent;

    int isibling;

    reductionPrivate_t *PTR_initRedH;
    ocrDBK_t DBK_initRedH;

    //ocrDBK_t DBK_redRootH[MAX_REDUCTION_HANDLES];

    sharedOcrObj_t sharedOcrObjH;

    ocrHint_t myDbkAffinityHNT, myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT_lazyDB;

    globalParamH_t globalParamH;
    rankTemplateH_t rankTemplateH;
    Command cmd;

    int ts;

    int max_num_parents;
    int num_parents;
    int max_active_parent;
    int cur_max_level;
    int cur_min_level;
    int num_blocks[MAX_REFINE_LEVELS+1];        //(num_refine+1); global num blocks in each level
    int local_num_blocks[MAX_REFINE_LEVELS+1];  //(num_refine+1); local num blocks in each level
    int block_start[MAX_REFINE_LEVELS+2];  //(num_refine+1) == global_blocks*(0,1,8,8^2,8^3,..,8^(num_ref-1))
    int num_active;
    int max_active_block;
    int max_possible_num_blocks;
    int global_active;
    int x_block_half, y_block_half, z_block_half;

    double tol;
    double grid_sum[MAX_NUM_VARS];

    int p8[MAX_REFINE_LEVELS+2], p2[MAX_REFINE_LEVELS+2]; //(num_refine+2)
    int mesh_size[3];
    int max_mesh_size;
    int *from, *to; //(num_pes)
    int msg_len[3][4];
    int global_max_b;

    int num_objects;

    int num_dots;
    int max_num_dots;
    int max_active_dot;

    timerH_t timerH;

    double tBegin, total_time;
    ocrDBK_t DBK_active_blockids, DBK_active_blockzValues;
    int *active_blockids, *active_blockzValues;
    int active_blockcount;
    int seqRank; //among the active blocks

    block blockH;
} rankH_t;

#endif
