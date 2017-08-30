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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "proto.h"
#include "timer.h"

void updateNeighborLevels( block* bp);

void printBlockSibsInfo(block* bp, int flag)
{
    int c;

    if( flag == 0 ) {
        DEBUG_PRINTF(( " sib level before " ));
        for( c = 0; c < 8; c++ ) {
            DEBUG_PRINTF(( "%d ", bp->sib_level[c] ));
        }
        DEBUG_PRINTF(( "\n" ));

        DEBUG_PRINTF(( " sib_refine_recv " ));
        for( c = 0; c < 8; c++ ) {
            DEBUG_PRINTF(( "%d ", bp->sib_refine_recv[c] ));
        }
        DEBUG_PRINTF(( "\n" ));
    }
    else {

        DEBUG_PRINTF(( " sib level after " ));
        for( c = 0; c < 8; c++ ) {
            DEBUG_PRINTF(( "%d ", bp->sib_level[c] ));
        }
        DEBUG_PRINTF(( "\n" ));
    }

}

_OCR_TASK_FNC_( FNC_splitBlocks )
{
    splitBlocksPRM_t* splitBlocksPRM = (splitBlocksPRM_t*) paramv;
    int irefine = splitBlocksPRM->irefine;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_octTreeRedH = depv[_idep++].guid;
    ocrDBK_t DBK_consolidateRedH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    octTreeRedH_t* PTR_octTreeRedH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_consolidateRedH = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);

    int ilevel = PTR_rankH->ilevel;

    block *bp = &PTR_rankH->blockH;

    u64 ts = PTR_rankH->ts;

    #ifdef DEBUG_APP_COARSE
    ocrPrintf( "%s ilevel %d id_l %d irefine %d FINAL bp->refine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, bp->refine, PTR_rankH->ts);
    #endif

    s32 _ichild;
    int c;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t TML_createChildBlocks = PTR_rankTemplateH->TML_createChildBlocks;
    ocrTML_t TML_continuation = PTR_rankTemplateH->TML_continuation;

    int refine = bp->refine;

    if( bp->number >= 0 ) { //active node

        printBlockSibsInfo(bp, 0);

        for( c = 0; c < 8; c++ ) {
            if( bp->sib_level[c] != -2 ) { //if not root
                bp->sib_level[c] = bp->level + bp->sib_refine_recv[c]; //Correct logic
            }
        }

        printBlockSibsInfo(bp, 1);

        if( bp->refine == COARSEN ) {
            double tmp = 1.0;
            reductionLaunch(PTR_consolidateRedH, DBK_consolidateRedH, &tmp);
        }

        if( bp->refine == REFINE || (bp->refine == COARSEN && PTR_rankH->isibling == 0 ) ) {

            ocrDBK_t parentRankDBK;
            ocrDBK_t childrenRankDBKs[8];

            ocrEVT_t createChildTriggerEvent;
            if( bp->refine == REFINE ) { //refine
                parentRankDBK = DBK_rankH;
                memcpy( childrenRankDBKs, PTR_sharedOcrObjH->childrenRankDBKs, 8*sizeof(ocrDBK_t) );

                createChildTriggerEvent = NULL_GUID;
            }
            else if( bp->refine == COARSEN ) {
                parentRankDBK = PTR_sharedOcrObjH->parentRankDBK;
                memcpy( childrenRankDBKs, PTR_sharedOcrObjH->siblingsRankDBKs, 8*sizeof(ocrDBK_t) );

                int r = COARSEN_RED_HANDLE_LB+irefine%2; //reserved
                redObjects_t* PTR_redObjects = &PTR_octTreeRedH->blockRedObjects[r];

                ocrEVT_t consolidateRedDownIEVT = PTR_redObjects->downIEVT;

                createChildTriggerEvent = consolidateRedDownIEVT;
            }

            ocrDbRelease( DBK_rankH );
            ocrDbRelease( DBK_octTreeRedH );

            ocrGuid_t createChildBlocksEDT;
            createChildBlocksPRM_t createChildBlocksPRM = {irefine, refine};

            ocrEdtCreate( &createChildBlocksEDT, TML_createChildBlocks, //FNC_createChildBlocks
                          EDT_PARAM_DEF, (u64*)&createChildBlocksPRM, EDT_PARAM_DEF, NULL,
                          EDT_PROP_FINISH, &myEdtAffinityHNT, NULL );

            _idep = 0; _ichild = 0;
            ocrAddDependence( parentRankDBK, createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocksEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( createChildTriggerEvent, createChildBlocksEDT, _idep++, DB_MODE_RW );
        }
        else if( bp->refine == STAY ) {

            updateNeighborLevels( bp );
            DEBUG_PRINTF(("\nunmodified block ilevel %d id_l %d nei_level %d %d %d %d %d %d sib_level %d %d %d %d %d %d %d %d\n", bp->level, PTR_rankH->myRank, bp->nei_level[0], bp->nei_level[1], bp->nei_level[2], bp->nei_level[3], bp->nei_level[4], bp->nei_level[5], bp->sib_level[0], bp->sib_level[1], bp->sib_level[2], bp->sib_level[3], bp->sib_level[4], bp->sib_level[5], bp->sib_level[6], bp->sib_level[7]));

            ocrDbRelease( DBK_rankH );
            ocrDbRelease( DBK_octTreeRedH );

            ocrGuid_t continuationEDT;
            continuationPRM_t continuationPRM = {irefine};
            ocrEdtCreate( &continuationEDT, TML_continuation,
                          EDT_PARAM_DEF, (u64*)&continuationPRM, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_continuation
            _idep = 0;
            ocrAddDependence( DBK_rankH, continuationEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( DBK_octTreeRedH, continuationEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( NULL_GUID, continuationEDT, _idep++, DB_MODE_NULL );

        }
        else {
           ocrDbRelease(DBK_rankH);
           ocrDbRelease(DBK_octTreeRedH);
        }

    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_continuation )
{
    s32 _idep, _paramc, _depc;

    continuationPRM_t* continuationPRM = (continuationPRM_t*) paramv;

    int irefine = continuationPRM->irefine;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_octTreeRedH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    octTreeRedH_t* PTR_octTreeRedH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    ocrTML_t TML_refineLoop = PTR_rankTemplateH->TML_refineLoop;
    ocrTML_t TML_reduceBlockCounts = PTR_rankTemplateH->TML_reduceBlockCounts;
    ocrTML_t TML_timestepLoop = PTR_rankTemplateH->TML_timestepLoop;
    ocrTML_t TML_loadbalance = PTR_rankTemplateH->TML_loadbalance;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ilevel = PTR_rankH->ilevel;
    int ts = PTR_rankH->ts;

    irefine += 1;

    int num_refine_step = (PTR_rankH->ts!=0) ? PTR_cmd->block_change : PTR_cmd->num_refine;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, irefine, PTR_rankH->ts ));

    int r = BLOCKCOUNT_RED_HANDLE_LB+(irefine%2);
    redObjects_t* PTR_redObjects = &PTR_octTreeRedH->blockRedObjects[r];
    ocrDBK_t DBK_in = PTR_redObjects->DBK_in;

    ocrDbRelease( DBK_rankH );
    ocrDbRelease( DBK_octTreeRedH );

    //start next refine
    if( irefine < num_refine_step ) {

        ocrGuid_t refineLoopEDT;
        refineLoopPRM_t refineLoopPRM = {irefine, ts};
        ocrEdtCreate( &refineLoopEDT, TML_refineLoop,
                      EDT_PARAM_DEF, (u64*)&refineLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_refineLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, refineLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_octTreeRedH, refineLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, refineLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, refineLoopEDT, _idep++, DB_MODE_NULL);
    }
    else {
        //update block counts
        ocrGuid_t reduceBlockCountsEDT, reduceBlockCountsOEVT, reduceBlockCountsOEVTS;
        reduceBlockCountsPRM_t reduceBlockCountsPRM = {irefine, ts};
        ocrEdtCreate( &reduceBlockCountsEDT, TML_reduceBlockCounts,
                      EDT_PARAM_DEF, (u64*)&reduceBlockCountsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &reduceBlockCountsOEVT ); //FNC_reduceBlockCounts
        createEventHelper(&reduceBlockCountsOEVTS, 1);
        ocrAddDependence( reduceBlockCountsOEVT, reduceBlockCountsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, reduceBlockCountsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_octTreeRedH, reduceBlockCountsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, reduceBlockCountsEDT, _idep++, DB_MODE_RW );

        //Loadbalancing
        loadbalancePRM_t loadbalancePRM = {ts};
        ocrGuid_t loadbalanceEDT, loadbalanceOEVT, loadbalanceOEVTS;
        ocrEdtCreate( &loadbalanceEDT, TML_loadbalance,
                      EDT_PARAM_DEF, (u64*)&loadbalancePRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &loadbalanceOEVT ); //FNC_loadbalance
        createEventHelper(&loadbalanceOEVTS, 1);
        ocrAddDependence( loadbalanceOEVT, loadbalanceOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, loadbalanceEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_octTreeRedH, loadbalanceEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( reduceBlockCountsOEVTS, loadbalanceEDT, _idep++, DB_MODE_RW );

        //start timesteploop
        ts += 1;
        timestepLoopPRM_t timestepLoopPRM = {ts};
        ocrGuid_t timestepLoopEDT;
        ocrEdtCreate( &timestepLoopEDT, TML_timestepLoop,
                      EDT_PARAM_DEF, (u64*)&timestepLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_timestepLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_octTreeRedH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( loadbalanceOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL );
    }

}

_OCR_TASK_FNC_( FNC_createChildBlocks )
{
    createChildBlocksPRM_t* createChildBlocksPRM = (createChildBlocksPRM_t*) paramv;

    int irefine = createChildBlocksPRM->irefine;
    int flag = createChildBlocksPRM->flag;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts ));

    ocrTML_t TML_createChildBlocks1 = PTR_rankTemplateH->TML_createChildBlocks1;

    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ilevel = PTR_rankH->ilevel;

    block *bp = &PTR_rankH->blockH;

    u64 ts = PTR_rankH->ts;

    ocrDBK_t DBK_array = bp->DBK_array;

    rankH_t* PTR_children_rankHs[8];
    ocrDBK_t childrenRankDBKs[8];
    ocrDBK_t DBK_array_children[8];

    int o;
    s32 _ichild;

    // Define the 8 children
    for (o = 0; o < 8; o++) {
        childrenRankDBKs[o] = depv[o+1].guid;
        PTR_children_rankHs[o] = depv[o+1].ptr;

        DBK_array_children[o] = PTR_children_rankHs[o]->blockH.DBK_array;

        ocrDbRelease(childrenRankDBKs[o]);
    }

    ocrDbRelease(DBK_rankH);

    ocrGuid_t createChildBlocks1EDT;
    ocrEdtCreate( &createChildBlocks1EDT, TML_createChildBlocks1, //FNC_createChildBlocks1
                  EDT_PARAM_DEF, (u64*)createChildBlocksPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, NULL );

    _idep = 0; _ichild = 0;
    ocrAddDependence( DBK_rankH, createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( childrenRankDBKs[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, createChildBlocks1EDT, _idep++, DB_MODE_RW );
    _ichild = 0;
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array_children[_ichild++], createChildBlocks1EDT, _idep++, DB_MODE_RW );

    return NULL_GUID;

}

_OCR_TASK_FNC_( FNC_createChildBlocks1 )
{
    createChildBlocksPRM_t* createChildBlocksPRM = (createChildBlocksPRM_t*) paramv;

    int irefine = createChildBlocksPRM->irefine;
    int flag = createChildBlocksPRM->flag;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts ));

    ocrTML_t TML_refineLoop = PTR_rankTemplateH->TML_refineLoop;
    ocrTML_t TML_timestepLoop = PTR_rankTemplateH->TML_timestepLoop;
    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ilevel = PTR_rankH->ilevel;

    block *bp = &PTR_rankH->blockH;

    u64 ts = PTR_rankH->ts;

    s32 _ichild;

    rankH_t* PTR_children_rankHs[8];
    ocrDBK_t childrenRankDBKs[8];
    ocrDBK_t children_DBK_octTreeRedH[8];
    ocrDBK_t DBK_array_children[8];

    block *childrenbp[8];

    int o;

    bp->array = depv[9].ptr;

    ocrTML_t TML_continuation = PTR_rankTemplateH->TML_continuation;
    ocrDBK_t DBK_octTreeRedH = PTR_sharedOcrObjH->DBK_octTreeRedH;

    // Define the 8 children
    for (o = 0; o < 8; o++) {
        childrenRankDBKs[o] = depv[o+1].guid;
        DBK_array_children[o] = depv[o+10].guid;

        PTR_children_rankHs[o] = depv[o+1].ptr;
        childrenbp[o] = &(PTR_children_rankHs[o]->blockH);
        childrenbp[o]->array = depv[o+10].ptr;
        children_DBK_octTreeRedH[o] = PTR_children_rankHs[o]->sharedOcrObjH.DBK_octTreeRedH;
        DEBUG_PRINTF(( "child o %d DBK_array "GUIDF" array PTR %p\n", o, depv[o+10].guid, childrenbp[o]->array ));
    }

    if( flag == 1 ) {

        split_blocks(PTR_rankH, PTR_children_rankHs, irefine);

        ocrDbRelease(DBK_rankH);

        for (o = 0; o < 8; o++) {

            ocrDbRelease(childrenRankDBKs[o]);
            ocrDbRelease(DBK_array_children[o]);

            ocrGuid_t continuationEDT;
            continuationPRM_t continuationPRM = {irefine};
            ocrEdtCreate( &continuationEDT, TML_continuation,
                          EDT_PARAM_DEF, (u64*)&continuationPRM, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_continuation
            _idep = 0;
            ocrAddDependence( childrenRankDBKs[o], continuationEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( children_DBK_octTreeRedH[o], continuationEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( NULL_GUID, continuationEDT, _idep++, DB_MODE_NULL );
        }
    }
    else if( flag == - 1 ) {
        consolidate_blocks(PTR_rankH, PTR_children_rankHs, irefine);

        ocrDbRelease(DBK_rankH);
        for (o = 0; o < 8; o++) {
            ocrDbRelease(childrenRankDBKs[o]);
            ocrDbRelease(DBK_array_children[o]);
        }

        DEBUG_PRINTF(( "PARENT irefine %d ts %d\n", irefine, ts ));

        ocrGuid_t continuationEDT;
        continuationPRM_t continuationPRM = {irefine};
        ocrEdtCreate( &continuationEDT, TML_continuation,
                      EDT_PARAM_DEF, (u64*)&continuationPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_continuation
        _idep = 0;
        ocrAddDependence( DBK_rankH, continuationEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_octTreeRedH, continuationEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, continuationEDT, _idep++, DB_MODE_NULL );

    }
    else if( flag == 0 ) {

    }

    return NULL_GUID;
}

// This routine splits blocks that are being refined into 8 new blocks,
// copies the data over to the new blocks, and then disconnects the
// original block from the mesh and connects the new blocks to the
// mesh.  The information in old block is also transferred to a parent
// block, which also contains information to identify its children.
void split_blocks(rankH_t* PTR_rankH, rankH_t* PTR_children_rankHs[8], int irefine)
{
    int side[6][2][2] = { { {0, 2}, {4, 6} }, { {1, 3}, {5, 7} },
                          { {0, 1}, {4, 5} }, { {2, 3}, {6, 7} },
                          { {0, 1}, {2, 3} }, { {4, 5}, {6, 7} } };
    int off[6] = {1, -1, 2, -2, 4, -4};
    int mul[3][3] = { {1, 2, 0}, {0, 2, 1}, {0, 1, 2} };
    int i, j, k, m, n, o, v, nl, xp, yp, zp, c, c1, other,
        i1, i2, j1, j2, k1, k2, dir, fcase, pe, f, p,
        level, sib[8], offset, d, half_size;
    int cp;
    block *bp1;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts ));

    block *bp = &PTR_rankH->blockH;
    block *pp = bp;

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int x_block_half = PTR_rankH->x_block_half;
    int y_block_half = PTR_rankH->y_block_half;
    int z_block_half = PTR_rankH->z_block_half;

    if (PTR_cmd->stencil == 7)  // add to face case when diags are needed
       f = 0;
    else
       f = 1;

    level = bp->level;

    if (bp->refine == REFINE) {

        nl = bp->number - PTR_rankH->block_start[level];
        zp = nl/((PTR_rankH->p2[level]*PTR_cmd->npx*PTR_cmd->init_block_x)*
                 (PTR_rankH->p2[level]*PTR_cmd->npy*PTR_cmd->init_block_y));
        yp = (nl%((PTR_rankH->p2[level]*PTR_cmd->npx*PTR_cmd->init_block_x)*
                  (PTR_rankH->p2[level]*PTR_cmd->npy*PTR_cmd->init_block_y)))/
             (PTR_rankH->p2[level]*PTR_cmd->npx*PTR_cmd->init_block_x);
        xp = nl%(PTR_rankH->p2[level]*PTR_cmd->npx*PTR_cmd->init_block_x);

        pp->number = -1; //DEACTIVATE
        pp->refine = STAY;
        pp->cen[0] = bp->cen[0];
        pp->cen[1] = bp->cen[1];
        pp->cen[2] = bp->cen[2];

        // Define the 8 children
        for (o = 0; o < 8; o++) {

            PTR_children_rankHs[o]->tBegin = PTR_rankH->tBegin;

            PTR_children_rankHs[o]->ts = PTR_rankH->ts;
            //copy object data to children
            Command* PTR_cmd_child = &(PTR_children_rankHs[o]->globalParamH.cmdParamH);
            memcpy(PTR_cmd_child, PTR_cmd, sizeof(Command));
            memcpy(PTR_children_rankHs[o]->grid_sum, PTR_rankH->grid_sum, MAX_NUM_VARS*sizeof(double));

            bp1 = &(PTR_children_rankHs[o]->blockH);

            sib[o] = m; //TODO

            bp1->refine = STAY;
            bp1->level = PTR_children_rankHs[o]->ilevel; //level + 1;
            bp1->child_number = o;

            i1 = (o%2);
            j1 = ((o/2)%2);
            k1 = (o/4);
            int tmp_number = - PTR_children_rankHs[o]->myRank_g;//bp1->number;
            bp1->number = ((2*zp+k1)*(PTR_rankH->p2[level+1]*PTR_cmd->npy*PTR_cmd->init_block_y) +
                           (2*yp+j1))*(PTR_rankH->p2[level+1]*PTR_cmd->npx*PTR_cmd->init_block_x) +
                          2*xp + i1 + PTR_rankH->block_start[level+1];
            DEBUG_PRINTF(( "%s parent %d child block ilevel %d id_l %d ts %d\n", __func__, PTR_rankH->myRank, PTR_children_rankHs[o]->ilevel, PTR_children_rankHs[o]->myRank, PTR_rankH->ts ));

            bp1->cen[0] = bp->cen[0] +
                          (2*i1 - 1)*PTR_rankH->p2[PTR_cmd->num_refine - level - 1];
            bp1->cen[1] = bp->cen[1] +
                          (2*j1 - 1)*PTR_rankH->p2[PTR_cmd->num_refine - level - 1];
            bp1->cen[2] = bp->cen[2] +
                          (2*k1 - 1)*PTR_rankH->p2[PTR_cmd->num_refine - level - 1];
            half_size = PTR_rankH->p2[PTR_cmd->num_refine - level - 1];
            i1 *= x_block_half;
            j1 *= y_block_half;
            k1 *= z_block_half;
            for (v = 0; v < PTR_cmd->num_vars; v++)
               for (i2 = i = 1; i <= x_block_half; i++, i2+=2)
                  for (j2 = j = 1; j <= y_block_half; j++, j2+=2)
                     for (k2 = k = 1; k <= z_block_half; k++, k2+=2) {
                        IN(bp1,v,i2  ,j2  ,k2  ) =
                        IN(bp1,v,i2+1,j2  ,k2  ) =
                        IN(bp1,v,i2  ,j2+1,k2  ) =
                        IN(bp1,v,i2+1,j2+1,k2  ) =
                        IN(bp1,v,i2  ,j2  ,k2+1) =
                        IN(bp1,v,i2+1,j2  ,k2+1) =
                        IN(bp1,v,i2  ,j2+1,k2+1) =
                        IN(bp1,v,i2+1,j2+1,k2+1) =
                              IN(bp,v,i+i1,j+j1,k+k1)/8.0;
                        //DEBUG_PRINTF(( "%f\n", IN(bp,v,i+i1,j+j1,k+k1) ));
                    }

        }

        #if 1
        for (c = 0; c < 6; c++) {
            cp = 2*(c/2) + (c+1)%2;
            for (i = 0; i < 2; i++) {
                for (j = 0; j < 2; j++) {

                    //internal faces
                    o = side[c][i][j]+off[c];
                    bp1 = &(PTR_children_rankHs[o]->blockH);
                    bp1->nei_level[c] = level + 1;

                    //external boundarues
                    if( bp->nei_level[cp] != -2 ) { //if not a boundary
                        if( bp->nei_level[cp] == bp->level ) {
                            bp1->nei_level[cp] = bp->nei_level[cp] + bp->nei_refine_recv[cp][0];
                        }
                        else if( bp->nei_level[cp] == bp->level - 1 ) {
                            bp1->nei_level[cp] = bp->nei_level[cp] + bp->nei_refine_recv[cp][0];
                        }
                        else if( bp->nei_level[cp] == bp->level + 1 ) {
                            bp1->nei_level[cp] = bp1->level + bp->nei_refine_recv[cp][2*i+j]; //TODO
                        }
                    }
                    else {
                        bp1->nei_level[cp] = bp->nei_level[cp];
                    }
                }
            }
        }

        updateNeighborLevels( bp );

        DEBUG_PRINTF(("\nparent level %d nei_level %d %d %d %d %d %d sib level %d %d %d %d %d %d %d %d ts %d\n", bp->level, bp->nei_level[0], bp->nei_level[1], bp->nei_level[2], bp->nei_level[3], bp->nei_level[4], bp->nei_level[5], bp->sib_level[0], bp->sib_level[1], bp->sib_level[2], bp->sib_level[3], bp->sib_level[4], bp->sib_level[5], bp->sib_level[6], bp->sib_level[7],  PTR_rankH->ts));

        int j;
        for (o = 0; o < 8; o++) {
            bp1 = &(PTR_children_rankHs[o]->blockH);
            for( j = 0; j < 8; j++ ) {
                bp1->sib_level[j] = level+1;
            }

            DEBUG_PRINTF(("child level %d nei_level %d %d %d %d %d %d sib level %d %d %d %d %d %d %d %d ts %d\n", bp1->level, bp1->nei_level[0], bp1->nei_level[1], bp1->nei_level[2], bp1->nei_level[3], bp1->nei_level[4], bp1->nei_level[5], bp1->sib_level[0], bp1->sib_level[1], bp1->sib_level[2], bp1->sib_level[3], bp1->sib_level[4], bp1->sib_level[5], bp1->sib_level[6], bp1->sib_level[7],  PTR_rankH->ts));

        }
        #endif
    }

}

void updateNeighborLevels( block* bp)
{
    int bp_nei_refine_max[6] = {-1,-1,-1,-1,-1,-1};

    int c, i, j;

    for (c = 0; c < 6; c++) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                if( bp->nei_level[c] != -2 ) { //if not a boundary
                    if( bp->nei_level[c] == bp->level + 1 ) {
                        bp_nei_refine_max[c] = MAX( bp->nei_refine_recv[c][2*i+j], bp_nei_refine_max[c] );
                    }
                }
            }
        }
    }

    for (c = 0; c < 6; c++) {
        if( bp->nei_level[c] != -2 ) { //if not a boundary
            if( bp->nei_level[c] == bp->level ) {
                bp->nei_level[c] += bp->nei_refine_recv[c][0];
            }
            else if( bp->nei_level[c] == bp->level - 1 ) {
                bp->nei_level[c] += bp->nei_refine_recv[c][0];
            }
            else if( bp->nei_level[c] == bp->level + 1 ) {
                bp->nei_level[c] += bp_nei_refine_max[c];
            }
        }
    }

}

// This routine takes blocks that are to be coarsened and recombines them.
// Before this routine can be called, all of the child blocks need to be on
// the same processor as the parent.  A new block is created and the parent
// and child blocks are inactivated.
void consolidate_blocks(rankH_t* PTR_rankH, rankH_t* PTR_children_rankHs[8], int irefine)
{
    int side[6][2][2] = { { {0, 2}, {4, 6} }, { {1, 3}, {5, 7} },
                          { {0, 1}, {4, 5} }, { {2, 3}, {6, 7} },
                          { {0, 1}, {2, 3} }, { {4, 5}, {6, 7} } };
    int mul[3][3] = { {1, 2, 0}, {0, 2, 1}, {0, 1, 2} };
    int n, p, i, j, k, i1, j1, k1, i2, j2, k2, level, o, v, f, c, offset,
        other, c1, dir, fcase, pe, nl, pos[3], d, in;
    block *bp1;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts ));

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int x_block_half = PTR_rankH->x_block_half;
    int y_block_half = PTR_rankH->y_block_half;
    int z_block_half = PTR_rankH->z_block_half;

    block *bp = &PTR_rankH->blockH;
    block *pp = bp;

    level = PTR_rankH->ilevel; //bp->level;

    if (PTR_cmd->stencil == 7)  // add to face case when diags are needed
       f = 0;
    else
       f = 1;

    if(pp->number < 0) { //TODO

        PTR_rankH->tBegin = PTR_children_rankHs[0]->tBegin;
        PTR_rankH->ts = PTR_children_rankHs[0]->ts;
        //
        //copy object data to parent
        Command* PTR_cmd_child = &(PTR_children_rankHs[0]->globalParamH.cmdParamH);
        memcpy(PTR_cmd, PTR_cmd_child, sizeof(Command));
        memcpy(PTR_rankH->grid_sum, PTR_children_rankHs[0]->grid_sum, MAX_NUM_VARS*sizeof(double));

        bp->number = PTR_rankH->myRank_g;//-pp->number;
        bp->level = PTR_rankH->ilevel;
        bp->refine = STAY;
        bp->cen[0] = pp->cen[0];
        bp->cen[1] = pp->cen[1];
        bp->cen[2] = pp->cen[2];
        // Copy child arrays back to new block.
        for (o = 0; o < 8; o++) {
            bp1 = &(PTR_children_rankHs[o]->blockH);
            bp1->number = -1; //DEACTIVATED
            i1 = (o%2)*x_block_half;
            j1 = ((o/2)%2)*y_block_half;
            k1 = (o/4)*z_block_half;
            for (v = 0; v < PTR_cmd->num_vars; v++)
               for (i2 = i = 1; i <= x_block_half; i++, i2+=2)
                  for (j2 = j = 1; j <= y_block_half; j++, j2+=2)
                     for (k2 = k = 1; k <= z_block_half; k++, k2+=2)
                         IN(bp,v,i+i1,j+j1,k+k1) =
                             IN(bp1,v,i2  ,j2  ,k2  ) +
                             IN(bp1,v,i2+1,j2  ,k2  ) +
                             IN(bp1,v,i2  ,j2+1,k2  ) +
                             IN(bp1,v,i2+1,j2+1,k2  ) +
                             IN(bp1,v,i2  ,j2  ,k2+1) +
                             IN(bp1,v,i2+1,j2  ,k2+1) +
                             IN(bp1,v,i2  ,j2+1,k2+1) +
                             IN(bp1,v,i2+1,j2+1,k2+1);

            updateNeighborLevels( bp1 );
       }

       for (c = 0; c < 6; c++) {
          o = side[c][0][0]; // first child on this side
          bp1 = &(PTR_children_rankHs[o]->blockH);

          bp->nei_level[c] = bp1->nei_level[c];
       }

        for (o = 0; o < 8; o++) {
            bp1 = &(PTR_children_rankHs[0]->blockH); //0th sibling

        }

       DEBUG_PRINTF(("parent level %d nei_level %d %d %d %d %d %d sib level %d %d %d %d %d %d %d %d ts %d\n", bp->level, bp->nei_level[0], bp->nei_level[1], bp->nei_level[2], bp->nei_level[3], bp->nei_level[4], bp->nei_level[5], bp->sib_level[0], bp->sib_level[1], bp->sib_level[2], bp->sib_level[3], bp->sib_level[4], bp->sib_level[5], bp->sib_level[6], bp->sib_level[7],  PTR_rankH->ts));
    }
}
