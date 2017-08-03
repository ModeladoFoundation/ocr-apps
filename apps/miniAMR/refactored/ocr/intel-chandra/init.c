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

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "block.h"
//#include "comm.h"
#include "proto.h"

// Initialize the problem and setup initial blocks.
void init(rankH_t* PTR_rankH)
{
   #if 1
   int n, var, i, j, k, l, m, o, size, dir, i1, i2, j1, j2, k1, k2, ib, jb, kb;
   block *bp;

   Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
   timerH_t* PTR_timerH = &(PTR_rankH->timerH);

   //init_profile(PTR_rankH);
    resetTimer( PTR_timerH );

    ocrHint_t myDbkAffinityHNT, myEdtAffinityHNT;

    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;
    PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;

   int num_refine = PTR_cmd->num_refine;

   PTR_rankH->tol = pow(10.0, ((double) -PTR_cmd->error_tol));

   PTR_rankH->p2[0] = PTR_rankH->p8[0] = 1;
   for (i = 0; i < (num_refine+1); i++)
   {
      PTR_rankH->p8[i+1] = PTR_rankH->p8[i]*8;
      PTR_rankH->p2[i+1] = PTR_rankH->p2[i]*2;
      //sorted_index[i] = 0;
   }
   //sorted_index[num_refine+1] = 0;
   PTR_rankH->num_blocks[0] = PTR_rankH->num_pes*PTR_cmd->init_block_x*PTR_cmd->init_block_y*PTR_cmd->init_block_z;
   PTR_rankH->local_num_blocks[0] = PTR_cmd->init_block_x*PTR_cmd->init_block_y*PTR_cmd->init_block_z;
   PTR_rankH->block_start[0] = 0;
   PTR_rankH->global_max_b = PTR_rankH->num_pes*PTR_cmd->init_block_x*PTR_cmd->init_block_y*PTR_cmd->init_block_z;
   int num = PTR_rankH->global_max_b;
   PTR_rankH->block_start[0] = 0;

   for (i = 1; i <= num_refine; i++)
   {
      PTR_rankH->block_start[i] = PTR_rankH->block_start[i-1] + num;
      num *= 8;
      PTR_rankH->num_blocks[i] = 0;
      PTR_rankH->local_num_blocks[i] = 0;
   }

   PTR_rankH->block_start[num_refine+1] = PTR_rankH->block_start[num_refine] + num;

   PTR_rankH->max_possible_num_blocks = PTR_rankH->block_start[num_refine+1];

   ///* initialize for communication arrays, which are initialized below */
   //zero_comm_list();

   int x_block_size = PTR_cmd->x_block_size;
   int y_block_size = PTR_cmd->y_block_size;
   int z_block_size = PTR_cmd->z_block_size;

   PTR_rankH->x_block_half = PTR_cmd->x_block_size/2;
   PTR_rankH->y_block_half = PTR_cmd->y_block_size/2;
   PTR_rankH->z_block_half = PTR_cmd->z_block_size/2;

   int x_block_half = PTR_rankH->x_block_half;
   int y_block_half = PTR_rankH->y_block_half;
   int z_block_half = PTR_rankH->z_block_half;

   if (!PTR_cmd->code)
   {
      /* for E/W (X dir) messages:
         0: whole -> whole (7), 1: whole -> whole (27),
         2: whole -> quarter, 3: quarter -> whole */
      PTR_rankH->msg_len[0][0] = PTR_rankH->msg_len[0][1] = y_block_size*z_block_size;
      PTR_rankH->msg_len[0][2] = PTR_rankH->msg_len[0][3] = y_block_half*z_block_half;
      /* for N/S (Y dir) messages */
      PTR_rankH->msg_len[1][0] = x_block_size*z_block_size;
      PTR_rankH->msg_len[1][1] = (x_block_size+2)*z_block_size;
      PTR_rankH->msg_len[1][2] = PTR_rankH->msg_len[1][3] = x_block_half*z_block_half;
      /* for U/D (Z dir) messages */
      PTR_rankH->msg_len[2][0] = x_block_size*y_block_size;
      PTR_rankH->msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      PTR_rankH->msg_len[2][2] = PTR_rankH->msg_len[2][3] = x_block_half*y_block_half;
   }
   else if (PTR_cmd->code == 1)
   {
      /* for E/W (X dir) messages */
      PTR_rankH->msg_len[0][0] = PTR_rankH->msg_len[0][1] = (y_block_size+2)*(z_block_size+2);
      PTR_rankH->msg_len[0][2] = (y_block_half+1)*(z_block_half+1);
      PTR_rankH->msg_len[0][3] = (y_block_half+2)*(z_block_half+2);
      /* for N/S (Y dir) messages */
      PTR_rankH->msg_len[1][0] = PTR_rankH->msg_len[1][1] = (x_block_size+2)*(z_block_size+2);
      PTR_rankH->msg_len[1][2] = (x_block_half+1)*(z_block_half+1);
      PTR_rankH->msg_len[1][3] = (x_block_half+2)*(z_block_half+2);
      /* for U/D (Z dir) messages */
      PTR_rankH->msg_len[2][0] = PTR_rankH->msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      PTR_rankH->msg_len[2][2] = (x_block_half+1)*(y_block_half+1);
      PTR_rankH->msg_len[2][3] = (x_block_half+2)*(y_block_half+2);
   }
   else
   {
      /* for E/W (X dir) messages */
      PTR_rankH->msg_len[0][0] = PTR_rankH->msg_len[0][1] = (y_block_size+2)*(z_block_size+2);
      PTR_rankH->msg_len[0][2] = (y_block_half+1)*(z_block_half+1);
      PTR_rankH->msg_len[0][3] = (y_block_size+2)*(z_block_size+2);
      /* for N/S (Y dir) messages */
      PTR_rankH->msg_len[1][0] = PTR_rankH->msg_len[1][1] = (x_block_size+2)*(z_block_size+2);
      PTR_rankH->msg_len[1][2] = (x_block_half+1)*(z_block_half+1);
      PTR_rankH->msg_len[1][3] = (x_block_size+2)*(z_block_size+2);
      /* for U/D (Z dir) messages */
      PTR_rankH->msg_len[2][0] = PTR_rankH->msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      PTR_rankH->msg_len[2][2] = (x_block_half+1)*(y_block_half+1);
      PTR_rankH->msg_len[2][3] = (x_block_size+2)*(y_block_size+2);
   }


   PTR_rankH->max_active_block = PTR_cmd->init_block_x*PTR_cmd->init_block_y*PTR_cmd->init_block_z;
   PTR_rankH->num_active = PTR_rankH->max_active_block;
   PTR_rankH->global_active = PTR_rankH->num_active*PTR_rankH->num_pes;
   PTR_rankH->num_parents = PTR_rankH->max_active_parent = 0;

   PTR_rankH->ts = 0; //Initial time-step is set to 0

   PTR_rankH->seqRank = PTR_rankH->myRank; //set it local rank within a level for now
   PTR_rankH->active_blockcount = PTR_rankH->block_start[1];

   int ilevel = PTR_rankH->ilevel;

   size = PTR_rankH->p2[num_refine+1]; //*-ilevel];
                             /* block size is p2[num_refine+1-level]
                              * smallest block is size p2[1], so can find
                              * its center */
   PTR_rankH->mesh_size[0] = PTR_cmd->npx*PTR_cmd->init_block_x*size;
   PTR_rankH->max_mesh_size = PTR_rankH->mesh_size[0];
   PTR_rankH->mesh_size[1] = PTR_cmd->npy*PTR_cmd->init_block_y*size;
   if (PTR_rankH->mesh_size[1] > PTR_rankH->max_mesh_size)
      PTR_rankH->max_mesh_size = PTR_rankH->mesh_size[1];
   PTR_rankH->mesh_size[2] = PTR_cmd->npz*PTR_cmd->init_block_z*size;
   if (PTR_rankH->mesh_size[2] > PTR_rankH->max_mesh_size)
      PTR_rankH->max_mesh_size = PTR_rankH->mesh_size[2];
   if ((PTR_rankH->num_pes+1) > PTR_rankH->max_mesh_size)
      PTR_rankH->max_mesh_size = PTR_rankH->num_pes + 1;

   bp = &PTR_rankH->blockH;

   size_t dbk_size;

   dbk_size = sizeof(double)*PTR_cmd->num_vars*(x_block_size+2)*(y_block_size+2)*(z_block_size+2);
   ocrDbCreate( &(bp->DBK_array), (void **) &bp->array,
                dbk_size,
                DB_PROP_NONE, &myDbkAffinityHNT, NO_ALLOC );
   ma_malloc_counter( PTR_timerH, bp->array, dbk_size, __FILE__, __LINE__ );

   DEBUG_PRINTF(( "%s ilevel %d id_l %d DBK_array "GUIDF" array %p\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, bp->DBK_array, bp->array ));

   dbk_size = sizeof(double)*1*(x_block_size+2)*(y_block_size+2)*(z_block_size+2);
   ocrDbCreate( &(bp->DBK_work), (void **) &bp->work,
                dbk_size,
                DB_PROP_NONE, &myDbkAffinityHNT, NO_ALLOC );
   ma_malloc_counter( PTR_timerH, bp->work, dbk_size, __FILE__, __LINE__ );

   size = PTR_rankH->p2[num_refine+1-ilevel];  // block size is p2[num_refine+1-level]

   bp->level = PTR_rankH->ilevel;
   bp->number = ((bp->level==0) ? 1:-1)*PTR_rankH->myRank_g; //Level 0 is active in the initialization phase
   bp->parent = -1;
   bp->cen[0] = PTR_rankH->iblock_g*size + size/2;
   bp->cen[1] = PTR_rankH->jblock_g*size + size/2;
   bp->cen[2] = PTR_rankH->kblock_g*size + size/2;

   for (var = 0; var < PTR_cmd->num_vars; var++)
      for (ib = 1; ib <= x_block_size; ib++)
         for (jb = 1; jb <= y_block_size; jb++)
            for (kb = 1; kb <= z_block_size; kb++) {
               #ifdef VERIFICATION_RUN
               IN(bp,var,ib,jb,kb) = pow(8.0,(PTR_cmd->num_refine));
               #else
               IN(bp,var,ib,jb,kb) = ((double) rand())/((double) RAND_MAX);
               #endif
            }

    ocrDbRelease(bp->DBK_array);
    ocrDbRelease(bp->DBK_work);

    memset( PTR_rankH->grid_sum, 0, MAX_NUM_VARS*sizeof(double) );

   //for (var = 0; var < PTR_cmd->num_vars; var++)
      //PTR_rankH->grid_sum[var] = 0.0; //updated later

    int npx = PTR_cmd->npx;
    int npy = PTR_cmd->npy;
    int npz = PTR_cmd->npz;

    //globalCoordsFromRank_Cart3D( PTR_rankH->my_pe, npx, npy, npz, &i, &j, &k );
    i2 = PTR_rankH->iblock;
    j2 = PTR_rankH->jblock;
    k2 = PTR_rankH->kblock;

    bp->nei_refine[0] = 0;
    bp->nei_level[0] = 0;
    //bp->nei[0][0][0] = PTR_rankH->block_start[ilevel-1]+globalRankFromCoords_Cart3D(PTR_rankH->iblock_g-1, PTR_rankH->jblock_g, PTR_rankH->kblock_g, PTR_rankH->gx, PTR_rankH->gy, PTR_rankH->gz);

    if(PTR_rankH->iblock_g==0) // 0 boundary
    {
        bp->nei_level[0] = -2;
        bp->nei[0][0][0] = 0;
    }

    bp->nei_refine[1] = 0;
    bp->nei_level[1] = 0;
    //bp->nei[1][0][0] = PTR_rankH->block_start[ilevel-1]+globalRankFromCoords_Cart3D(PTR_rankH->iblock_g+1, PTR_rankH->jblock_g, PTR_rankH->kblock_g, PTR_rankH->gx, PTR_rankH->gy, PTR_rankH->gz);

    if(PTR_rankH->iblock_g==PTR_rankH->gx-1) // 0 boundary
    {
        bp->nei_level[1] = -2;
        bp->nei[1][0][0] = 0;
    }

    bp->nei_refine[2] = 0;
    bp->nei_level[2] = 0;
    //bp->nei[2][0][0] = PTR_rankH->block_start[ilevel-1]+globalRankFromCoords_Cart3D(PTR_rankH->iblock_g, PTR_rankH->jblock_g-1, PTR_rankH->kblock_g, PTR_rankH->gx, PTR_rankH->gy, PTR_rankH->gz);

    if(PTR_rankH->jblock_g==0) // 0 boundary
    {
        bp->nei_level[2] = -2;
        bp->nei[2][0][0] = 0;
    }

    bp->nei_refine[3] = 0;
    bp->nei_level[3] = 0;
    //bp->nei[3][0][0] = PTR_rankH->block_start[ilevel-1]+globalRankFromCoords_Cart3D(PTR_rankH->iblock_g, PTR_rankH->jblock_g+1, PTR_rankH->kblock_g, PTR_rankH->gx, PTR_rankH->gy, PTR_rankH->gz);

    if(PTR_rankH->jblock_g==PTR_rankH->gy-1) // 0 boundary
    {
        bp->nei_level[3] = -2;
        bp->nei[3][0][0] = 0;
    }

    bp->nei_refine[4] = 0;
    bp->nei_level[4] = 0;
    //bp->nei[4][0][0] = PTR_rankH->block_start[ilevel-1]+globalRankFromCoords_Cart3D(PTR_rankH->iblock_g, PTR_rankH->jblock_g, PTR_rankH->kblock_g-1, PTR_rankH->gx, PTR_rankH->gy, PTR_rankH->gz);

    if(PTR_rankH->kblock_g==0) // 0 boundary
    {
        bp->nei_level[4] = -2;
        bp->nei[4][0][0] = 0;
    }

    bp->nei_refine[5] = 0;
    bp->nei_level[5] = 0;

    if(PTR_rankH->kblock_g==PTR_rankH->gz-1) // 0 boundary
    {
        bp->nei_level[5] = -2;
        bp->nei[5][0][0] = 0;
    }

    int c;
    for (c = 0; c < 8; c++) {
        bp->sib_level[c] = (ilevel!=0) ? ilevel:-2; //No siblings for the coarsest blocks
        bp->sib_refine[c] = -1; //(ilevel!=0) ?-1:-2; //NO SIBLINGS //WORKS for coarsening
    }

    initTemplates(PTR_rankH);
    initDBKs(PTR_rankH);

    PTR_timerH->counter_malloc_init = PTR_timerH->counter_malloc;
    PTR_timerH->size_malloc_init = PTR_timerH->size_malloc;

#endif
}

void initDBKs( rankH_t* PTR_rankH )
{
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);

    size_t dbk_size;

    int isibling = PTR_rankH->isibling;
    int ilevel = PTR_rankH->ilevel;

    int r;
    for( r = 0; r <MAX_REDUCTION_HANDLES; r++ ) {
        reductionPrivate_t* PTR_redUpH;
        ocrDbCreate( &PTR_rankH->DBK_redUpH[r], (void**) &PTR_redUpH, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

        PTR_redUpH->nrank = (ilevel==0)?PTR_rankH->nRanks:8;
        PTR_redUpH->myrank = (ilevel==0)?PTR_rankH->myRank:PTR_rankH->isibling;
        PTR_redUpH->ndata = MAX_REFINE_LEVELS;
        PTR_redUpH->reductionOperator = REDUCTION_U4_ADD;
        PTR_redUpH->rangeGUID = (ilevel==0)?PTR_rankH->globalParamH.ocrParamH.redUpRangeGUID[r]:PTR_sharedOcrObjH->siblingredRangeGUID;
        PTR_redUpH->reductionTML = NULL_GUID;
        PTR_redUpH->new = 1;  //first time
        PTR_redUpH->type = ALLREDUCE;
        PTR_redUpH->returnEVT = PTR_sharedOcrObjH->blockRedObjects[r].downIEVT;

        reductionHandle_update( r, PTR_rankH, PTR_redUpH );

        ocrDbRelease( PTR_rankH->DBK_redUpH[r] );
    }


    ocrDbCreate( &PTR_rankH->DBK_initRedH, (void**) &PTR_rankH->PTR_initRedH, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

    PTR_rankH->PTR_initRedH->nrank = PTR_rankH->block_start[PTR_cmd->num_refine+1];
    PTR_rankH->PTR_initRedH->myrank = PTR_rankH->myRank_g;
    PTR_rankH->PTR_initRedH->ndata = 1;
    PTR_rankH->PTR_initRedH->reductionOperator = REDUCTION_U4_ADD;
    PTR_rankH->PTR_initRedH->rangeGUID = PTR_rankH->globalParamH.ocrParamH.initRedRangeGUID;
    PTR_rankH->PTR_initRedH->reductionTML = NULL_GUID;
    PTR_rankH->PTR_initRedH->new = 1;  //first time
    PTR_rankH->PTR_initRedH->type = ALLREDUCE;
    createEventHelper(&(PTR_rankH->sharedOcrObjH.initRedOEVT), 1);
    PTR_rankH->PTR_initRedH->returnEVT = PTR_rankH->sharedOcrObjH.initRedOEVT;

    //ocrDbRelease( PTR_rankH->DBK_initRedH ); //will be released by the reduction library

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j, phase;

    int f;
    if (PTR_cmd->stencil == 7)  // add to face case when diags are needed
       f = 0;
    else
       f = 1;

    int size;
    //boundary halos - size depends on the face
    for (i = 0; i < 6; i++) {

        int fcase = (i%2)*10;

        for( phase = 0; phase < 2; phase++ ) {

            size = return_buf_size( PTR_rankH, i/2, fcase+f, 1); //peer

            PTR_dBufH1->haloCurrSendPTRs[i][phase] = NULL;
            dbk_size = size*sizeof(double);
            ocrDbCreate( &(PTR_dBufH1->haloCurrSendDBKs[i][phase]), (void**) &(PTR_dBufH1->haloCurrSendPTRs[i][phase]), size*sizeof(double), 0, NULL_HINT, NO_ALLOC );
            ma_malloc_counter( PTR_timerH, PTR_dBufH1->haloCurrSendPTRs[i][phase], dbk_size, __FILE__, __LINE__ );
            ocrDbRelease( PTR_dBufH1->haloCurrSendDBKs[i][phase] );

            size = return_buf_size( PTR_rankH, i/2, fcase+2, 1); //coarse neighbor
            //face+[2,3,4,5] //
            PTR_dBufH1->haloCoarSendPTRs[i][phase] = NULL;
            dbk_size = size*sizeof(double);
            ocrDbCreate( &(PTR_dBufH1->haloCoarSendDBKs[i][phase]), (void**) &(PTR_dBufH1->haloCoarSendPTRs[i][phase]), size*sizeof(double), 0, NULL_HINT, NO_ALLOC );
            ma_malloc_counter( PTR_timerH, PTR_dBufH1->haloCoarSendPTRs[i][phase], dbk_size, __FILE__, __LINE__ );
            ocrDbRelease( PTR_dBufH1->haloCoarSendDBKs[i][phase] );

            for( j = 0; j < 4; j++ ) {
                size = return_buf_size( PTR_rankH, i/2, fcase+6+j, 1); //refined neighbor
                dbk_size = size*sizeof(double);
                PTR_dBufH1->haloRefnSendPTRs[i][j][phase] = NULL;
                ocrDbCreate( &(PTR_dBufH1->haloRefnSendDBKs[i][j][phase]), (void**) &(PTR_dBufH1->haloRefnSendPTRs[i][j][phase]), size*sizeof(double), 0, NULL_HINT, NO_ALLOC );
                ma_malloc_counter( PTR_timerH, PTR_dBufH1->haloRefnSendPTRs[i][j][phase], dbk_size, __FILE__, __LINE__ );
                ocrDbRelease( PTR_dBufH1->haloRefnSendDBKs[i][j][phase] );

            }

        }
    }

    //refinement intention buffers - just one int
    for (i = 0; i < 6; i++) {
        for( phase = 0; phase < 2; phase++ ) {
            PTR_dBufH1->refnCurrSendPTRs[i][phase] = NULL;
            ocrDbCreate( &(PTR_dBufH1->refnCurrSendDBKs[i][phase]), (void**) &(PTR_dBufH1->refnCurrSendPTRs[i][phase]), sizeof(int), 0, NULL_HINT, NO_ALLOC );
            ocrDbRelease( PTR_dBufH1->refnCurrSendDBKs[i][phase] );

            PTR_dBufH1->refnCoarSendPTRs[i][phase] = NULL;
            ocrDbCreate( &(PTR_dBufH1->refnCoarSendDBKs[i][phase]), (void**) &(PTR_dBufH1->refnCoarSendPTRs[i][phase]), sizeof(int), 0, NULL_HINT, NO_ALLOC );
            ocrDbRelease( PTR_dBufH1->refnCoarSendDBKs[i][phase] );
            for( j = 0; j < 4; j++ ) {
                PTR_dBufH1->refnRefnSendPTRs[i][j][phase] = NULL;
                ocrDbCreate( &(PTR_dBufH1->refnRefnSendDBKs[i][j][phase]), (void**) &(PTR_dBufH1->refnRefnSendPTRs[i][j][phase]), sizeof(int), 0, NULL_HINT, NO_ALLOC );
                ocrDbRelease( PTR_dBufH1->refnRefnSendDBKs[i][j][phase] );
            }

        }
    }

    for( i = 0; i < 8; i++ ) {
        PTR_dBufH1->refnCurrSendSibsPTRs[i] = NULL;
        ocrDbCreate( &(PTR_dBufH1->refnCurrSendSibsDBKs[i]), (void**) &(PTR_dBufH1->refnCurrSendSibsPTRs[i]), sizeof(int), 0, NULL_HINT, NO_ALLOC );
        ocrDbRelease( PTR_dBufH1->refnCurrSendSibsDBKs[i] );
    }
}

void initTemplates(rankH_t* PTR_rankH)
{
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    s32 _idep, _paramc, _depc;

    _paramc = PARAMC_U64(refineLoopPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_refine), FNC_refine, _paramc, _depc );
    _paramc = PARAMC_U64(refineLoopPRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_refineLoop), FNC_refineLoop, _paramc, _depc );

    _paramc = PARAMC_U64(reduceBlockCountsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_reduceBlockCounts), FNC_reduceBlockCounts, _paramc, _depc );
    _paramc = PARAMC_U64(updateBlockCountsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_updateBlockCounts), FNC_updateBlockCounts, _paramc, _depc );

    _paramc = PARAMC_U64(commRefnNbrsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_commRefnNbrs), commRefnNbrsEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commRefnNbrsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->exchangeDataTML), exchangeDataEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commRefnNbrsPRM_t); _depc = EDT_PARAM_UNK;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->packRefnBufsTML), packRefnBufsEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commRefnNbrsPRM_t); _depc = EDT_PARAM_UNK;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->unpackRefnBufsTML), unpackRefnBufsEdt, _paramc, _depc );

    _paramc = PARAMC_U64(commRefnSibsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_commRefnSibs), commRefnSibsEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commRefnSibsPRM_t); _depc = 9;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->packRefnBufsSibsTML), packRefnBufsSibsEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commRefnSibsPRM_t); _depc = 10;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->unpackRefnBufsSibsTML), unpackRefnBufsSibsEdt, _paramc, _depc );

    _paramc = PARAMC_U64(refineAllLevelsPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_refineAllLevels), FNC_refineAllLevels, _paramc, _depc );

    _paramc = PARAMC_U64(refineLevelsPRM_t); _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_refine1AllLevels), FNC_refine1AllLevels, _paramc, _depc );

    _paramc = PARAMC_U64(refineLevelsPRM_t); _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_refine1Level), FNC_refine1Level, _paramc, _depc );

    _paramc = PARAMC_U64(reducePRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_reduceAllUp), FNC_reduceAllUp, _paramc, _depc );
    _paramc = PARAMC_U64(reducePRM_t); _depc = 10;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_accumulator), FNC_accumulator, _paramc, _depc );
    _paramc = PARAMC_U64(reducePRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_reduceAllRootNodes), FNC_reduceAllRootNodes, _paramc, _depc );
    _paramc = PARAMC_U64(reducePRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_reduceAllDown), FNC_reduceAllDown, _paramc, _depc );
    _paramc = PARAMC_U64(reducePRM_t); _depc = 10;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_bcast), FNC_bcast, _paramc, _depc );

    _paramc = PARAMC_U64(scatterRefinePRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_scatterRefine), FNC_scatterRefine, _paramc, _depc );

    _paramc = PARAMC_U64(splitBlocksPRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_splitBlocks), FNC_splitBlocks, _paramc, _depc );

    _paramc = PARAMC_U64(createChildBlocksPRM_t); _depc = 10;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_createChildBlocks), FNC_createChildBlocks, _paramc, _depc );
    _paramc = PARAMC_U64(createChildBlocksPRM_t); _depc = 18;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_createChildBlocks1), FNC_createChildBlocks1, _paramc, _depc );

    _paramc = PARAMC_U64(timestepLoopPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_timestepLoop), FNC_timestepLoop, _paramc, _depc );
    _paramc = PARAMC_U64(stageLoopPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_stageLoop), FNC_stageLoop, _paramc, _depc );
    _paramc = PARAMC_U64(stageLoopPRM_t); _depc = 1;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_stage), FNC_stage, _paramc, _depc );

    _paramc = PARAMC_U64(movePRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_move), FNC_move, _paramc, _depc );

    _paramc = PARAMC_U64(varsLoopPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_varsLoop), FNC_varsLoop, _paramc, _depc );
    _paramc = PARAMC_U64(varsLoopPRM_t); _depc = 1;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_vars), FNC_vars, _paramc, _depc );

    _paramc = PARAMC_U64(commPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_comm), FNC_comm, _paramc, _depc );
    _paramc = PARAMC_U64(commPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->commHaloNbrsTML), commHaloNbrsEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commPRM_t); _depc = EDT_PARAM_UNK;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->packHalosTML), packHalosEdt, _paramc, _depc );
    _paramc = PARAMC_U64(commPRM_t); _depc = EDT_PARAM_UNK;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->unpackHalosTML), unpackHalosEdt, _paramc, _depc );

    _paramc = PARAMC_U64(calcLoopPRM_t); _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_calcLoop), FNC_calcLoop, _paramc, _depc );
    _paramc = PARAMC_U64(calcPRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_calc), FNC_calc, _paramc, _depc );

    _paramc = PARAMC_U64(checkSumLoopPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_checkSumLoop), FNC_checkSumLoop, _paramc, _depc );
    _paramc = PARAMC_U64(checkSumPRM_t); _depc = 4;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_checkSum), FNC_checkSum, _paramc, _depc );

    _paramc = PARAMC_U64(printPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_print), FNC_print, _paramc, _depc );

    _paramc = 0; _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_finalize), FNC_finalize, _paramc, _depc );
    _paramc = 0; _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_finalizeBarrier), FNC_finalizeBarrier, _paramc, _depc );

    _paramc = 0; _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_idgather), FNC_idgather, _paramc, _depc );
    _paramc = PARAMC_U64(redistributeblocksPRM_t); _depc = 3;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_redistributeblocks), FNC_redistributeblocks, _paramc, _depc );
    _paramc = PARAMC_U64(loadbalancePRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_loadbalance), FNC_loadbalance, _paramc, _depc );

    _paramc = PARAMC_U64(continuationPRM_t); _depc = 2;
    ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_continuation), FNC_continuation, _paramc, _depc );
}
