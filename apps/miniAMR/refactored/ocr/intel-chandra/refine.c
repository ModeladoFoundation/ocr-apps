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
#include <string.h>

#include "block.h"
#include "proto.h"

_OCR_TASK_FNC_( FNC_refine )
{
    s32 _idep, _paramc, _depc;
    refineLoopPRM_t* refineLoopPRM = (refineLoopPRM_t*) paramv;

    int irefine = refineLoopPRM->irefine;
    int ts = refineLoopPRM->ts;

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_in = depv[1].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    int* PTR_in = depv[1].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    PTR_rankH->ts = ts;

    int num_refine_step = (PTR_rankH->ts!=0) ? PTR_cmd->block_change : PTR_cmd->num_refine;

    if( bp->number >= 0 ) {

        ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
        myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
        myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

        ocrTML_t TML_reduceBlockCounts = PTR_rankTemplateH->TML_reduceBlockCounts;
        ocrTML_t TML_commRefnNbrs = PTR_rankTemplateH->TML_commRefnNbrs;
        ocrTML_t TML_commRefnSibs = PTR_rankTemplateH->TML_commRefnSibs;
        ocrTML_t TML_refineAllLevels = PTR_rankTemplateH->TML_refineAllLevels;
        ocrTML_t TML_splitBlocks = PTR_rankTemplateH->TML_splitBlocks;

        reset_all( PTR_rankH );

        mark_refinementIntention( PTR_rankH, irefine );

        #ifdef DEBUG_APP_COARSE
        PRINTF( "%s ilevel %d id_l %d irefine %d REFINE %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, irefine, bp->refine, PTR_rankH->ts);
        #endif

        int r = BLOCKCOUNT_RED_HANDLE_LB + (irefine%2); //reserved for block counts
        redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

        ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
        ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

        ocrDBK_t DBK_redUpH = PTR_rankH->DBK_redUpH[COARSEN_RED_HANDLE_LB + irefine%2];

        ocrDbRelease(DBK_rankH);

        ocrGuid_t reduceBlockCountsEDT, reduceBlockCountsOEVT, reduceBlockCountsOEVTS;
        reduceBlockCountsPRM_t reduceBlockCountsPRM = {irefine, ts};
        ocrEdtCreate( &reduceBlockCountsEDT, TML_reduceBlockCounts,
                      EDT_PARAM_DEF, (u64*)&reduceBlockCountsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &reduceBlockCountsOEVT ); //FNC_reduceBlockCounts
        createEventHelper(&reduceBlockCountsOEVTS, 1);
        ocrAddDependence( reduceBlockCountsOEVT, reduceBlockCountsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, reduceBlockCountsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, reduceBlockCountsEDT, _idep++, DB_MODE_RW );

        int iAxis = 0;
        int flag = 1;
        commRefnNbrsPRM_t commRefnNbrsPRM = {irefine, iAxis, flag};
        ocrGuid_t commRefnNbrsEDT, commRefnNbrsOEVT, commRefnNbrsOEVTS;

        ocrEdtCreate( &commRefnNbrsEDT, TML_commRefnNbrs, //commRefnNbrsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnNbrsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &commRefnNbrsOEVT );
        createEventHelper(&commRefnNbrsOEVTS, 1);
        ocrAddDependence( commRefnNbrsOEVT, commRefnNbrsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnNbrsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( reduceBlockCountsOEVTS, commRefnNbrsEDT, _idep++, DB_MODE_RO ); //TODO - is this dependency really needed?

        flag = 1;
        commRefnSibsPRM_t commRefnSibsPRM = {irefine, flag, 0};
        ocrGuid_t commRefnSibsEDT, commRefnSibsOEVT, commRefnSibsOEVTS;

        ocrEdtCreate( &commRefnSibsEDT, TML_commRefnSibs, //commRefnSibsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnSibsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &commRefnSibsOEVT );
        createEventHelper(&commRefnSibsOEVTS, 1);
        ocrAddDependence( commRefnSibsOEVT, commRefnSibsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnSibsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commRefnNbrsOEVTS, commRefnSibsEDT, _idep++, DB_MODE_RO );

        refineAllLevelsPRM_t refineAllLevelsPRM = {irefine};
        ocrGuid_t refineAllLevelsEDT, refineAllLevelsOEVT, refineAllLevelsOEVTS;

        ocrEdtCreate( &refineAllLevelsEDT, TML_refineAllLevels, //FNC_refineAllLevels
                      EDT_PARAM_DEF, (u64*) &refineAllLevelsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &refineAllLevelsOEVT );
        createEventHelper(&refineAllLevelsOEVTS, 1);
        ocrAddDependence( refineAllLevelsOEVT, refineAllLevelsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, refineAllLevelsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commRefnSibsOEVTS, refineAllLevelsEDT, _idep++, DB_MODE_RO );
        DEBUG_PRINTF(( "Depending on redDownOEVT "GUIDF" \n", redDownOEVT ));

        splitBlocksPRM_t splitBlocksPRM = {irefine};
        ocrGuid_t splitBlocksEDT, splitBlocksOEVT, splitBlocksOEVTS;

        ocrEdtCreate( &splitBlocksEDT, TML_splitBlocks, //FNC_splitBlocks
                      EDT_PARAM_DEF, (u64*)&splitBlocksPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, &splitBlocksOEVT );
        createEventHelper(&splitBlocksOEVTS, 1);
        ocrAddDependence( splitBlocksOEVT, splitBlocksOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, splitBlocksEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_redUpH, splitBlocksEDT, _idep++, DB_MODE_RW ); //COARSEN_RED_HANDLE_LB
        ocrAddDependence( refineAllLevelsOEVTS, splitBlocksEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_reduceBlockCounts )
{
    s32 _idep, _paramc, _depc;
    refineLoopPRM_t* refineLoopPRM = (refineLoopPRM_t*) paramv;

    int irefine = refineLoopPRM->irefine;
    int ts = refineLoopPRM->ts;

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_in = depv[1].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    int* PTR_in = depv[1].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    timerH_t* PTR_timerH = &(PTR_rankH->timerH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t TML_reduceAllUp = PTR_rankTemplateH->TML_reduceAllUp;
    ocrTML_t TML_updateBlockCounts = PTR_rankTemplateH->TML_updateBlockCounts;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d REFINE %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, irefine, bp->refine, PTR_rankH->ts));

    int r = BLOCKCOUNT_RED_HANDLE_LB + (irefine%2); //reserved for block counts
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    int i;
    for( i = 0; i <= PTR_cmd->num_refine; i++ )
        PTR_in[i] = 0; //Default value

    PTR_in[bp->level]++;
    ocrDbRelease( DBK_in );
    ocrEventSatisfy( redUpIEVT, DBK_in ); //All blocks provide partial sums

    ocrDbRelease(DBK_rankH);

    int phase = r;
    reducePRM_t reducePRM = {irefine, ts, phase, r};
    ocrGuid_t reduceAllUpEDT;

    ocrEdtCreate( &reduceAllUpEDT, TML_reduceAllUp, //FNC_reduceAllUp
                  EDT_PARAM_DEF, (u64*) &reducePRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, reduceAllUpEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( redUpIEVT, reduceAllUpEDT, _idep++, DB_MODE_RO );

    ocrGuid_t updateBlockCountsEDT;
    updateBlockCountsPRM_t updateBlockCountsPRM = {irefine, ts};
    ocrEdtCreate( &updateBlockCountsEDT, TML_updateBlockCounts,
                  EDT_PARAM_DEF, (u64*)&updateBlockCountsPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_updateBlockCounts
    _idep = 0;
    ocrAddDependence( DBK_rankH, updateBlockCountsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redDownOEVT, updateBlockCountsEDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_updateBlockCounts )
{
    updateBlockCountsPRM_t* updateBlockCountsPRM = (updateBlockCountsPRM_t*) paramv;
    int irefine = updateBlockCountsPRM->irefine;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* PTR_out = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    int num_refine_step = (PTR_rankH->ts!=0) ? PTR_cmd->block_change : PTR_cmd->num_refine;

    int j, cur_max_level, cur_min_level;

    for (j = PTR_cmd->num_refine; j >= 0; j--)
       if (PTR_out[j]) {
          cur_max_level = j;
          break;
    }

    for (j = 0; j <= PTR_cmd->num_refine; j++)
       if (PTR_out[j]) {
          cur_min_level = j;
          break;
    }

    for (j = 0; j <= cur_max_level; j++) {
        PTR_rankH->num_blocks[j] = PTR_out[j];
        PTR_out[j] = 0;
    }

    PTR_rankH->cur_min_level = cur_min_level;
    PTR_rankH->cur_max_level = cur_max_level;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ts %d cur_min_level %d blocks %d cur_max_level %d blocks %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, PTR_rankH->ts, cur_min_level, PTR_rankH->num_blocks[cur_min_level], cur_max_level, PTR_rankH->num_blocks[cur_max_level] ));

    #ifdef PRINTBLOCKSTATS
    if( PTR_rankH->seqRank == 0 ) {
        PRINTF( "Active block stats before refinement ilevel %d id_l %d\n", PTR_rankH->ilevel, PTR_rankH->myRank );
        for (j = 0; j <= PTR_rankH->cur_max_level; j++)
            PRINTF("ts %d irefine %d level %d #blocks %d\n", PTR_rankH->ts, irefine, j, PTR_rankH->num_blocks[j] );

        PRINTF("\n" );
    }
    #endif

    ocrDbRelease(DBK_rankH);

    return NULL_GUID;
}

//All active blocks enter into this
//paramv must start at cur_max_level
_OCR_TASK_FNC_( FNC_refineAllLevels )
{
    refineAllLevelsPRM_t* refineAllLevelsPRM = (refineAllLevelsPRM_t*) paramv;
    int irefine = refineAllLevelsPRM->irefine;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    int cur_max_level, cur_min_level;

    cur_min_level = PTR_rankH->cur_min_level;
    cur_max_level = PTR_rankH->cur_max_level;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ts %d cur_min_level %d blocks %d cur_max_level %d blocks %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, PTR_rankH->ts, cur_min_level, PTR_rankH->num_blocks[cur_min_level], cur_max_level, PTR_rankH->num_blocks[cur_max_level] ));

    int ilevel = PTR_rankH->cur_max_level;

    int iter = 0; //start at iteration 0
    int r = REFINEINTENT_RED_HANDLE_LB+(ilevel+iter)%(MAX_REFINE_LEVELS); //current level being iterated
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
    ocrDBK_t DBK_in = PTR_redObjects->DBK_in;
    ocrDBK_t DBK_out = PTR_redObjects->DBK_out;

    int phase = 10;
    refineLevelsPRM_t refine1Levels = { irefine, ilevel, phase, iter };
    ocrGuid_t refine1AllLevelsEDT, refine1AllLevelsOEVT, refine1AllLevelsOEVTS;

    ocrEdtCreate( &refine1AllLevelsEDT, PTR_rankTemplateH->TML_refine1AllLevels, //FNC_refine1AllLevels
                  EDT_PARAM_DEF, (u64*) &refine1Levels, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &refine1AllLevelsOEVT );
    createEventHelper(&refine1AllLevelsOEVTS, 1);
    ocrAddDependence( refine1AllLevelsOEVT, refine1AllLevelsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_in, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_out, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, refine1AllLevelsEDT, _idep++, DB_MODE_NULL );

    phase = 20;
    refineLevelsPRM_t refine2Levels = { irefine, ilevel, phase, iter };
    ocrGuid_t refine2AllLevelsEDT, refine2AllLevelsOEVT, refine2AllLevelsOEVTS;

    ocrEdtCreate( &refine2AllLevelsEDT, PTR_rankTemplateH->TML_refine1AllLevels, //FNC_refine1AllLevels
                  EDT_PARAM_DEF, (u64*) &refine2Levels, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &refine2AllLevelsOEVT );
    createEventHelper(&refine2AllLevelsOEVTS, 1);
    ocrAddDependence( refine2AllLevelsOEVT, refine2AllLevelsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, refine2AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_in, refine2AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_out, refine2AllLevelsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( refine1AllLevelsOEVTS, refine2AllLevelsEDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

//All active blocks enter into this
//paramv must start at cur_max_level
_OCR_TASK_FNC_( FNC_refine1AllLevels ) //Make sure irefine is set to '0' the first time.
{
    refineLevelsPRM_t* refineLevelsPRM = (refineLevelsPRM_t*) paramv;
    int irefine = refineLevelsPRM->irefine;
    int ilevel = refineLevelsPRM->ilevel;
    int phase = refineLevelsPRM->phase;
    int iter = refineLevelsPRM->iter;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_in = depv[_idep++].guid;
    ocrGuid_t DBK_out = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* PTR_in = depv[_idep++].ptr;
    int* PTR_out = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    DEBUG_PRINTF(( "%s ilevel_iter %d ilevel %d id_l %d irefine %d phase %d, iter %d ts %d\n", __func__, ilevel, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, phase, iter, PTR_rankH->ts ));

    PTR_in[0] = 0;
    PTR_out[0] = 1;
    ocrDbRelease( DBK_in );
    ocrDbRelease( DBK_out );

    // Do one refine until quiescence has reached
    ocrGuid_t refine1LevelEDT;

    ocrEdtCreate( &refine1LevelEDT, PTR_rankTemplateH->TML_refine1Level, //FNC_refine1Level
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, refine1LevelEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_in, refine1LevelEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_out, refine1LevelEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, refine1LevelEDT, _idep++, DB_MODE_NULL );

    ilevel -= 1; //decrement

    if( ilevel >= 0 )
    {
        int iter = 0; //start at iteration 0
        int r = REFINEINTENT_RED_HANDLE_LB+(ilevel+iter)%(MAX_REFINE_LEVELS); //current level being iterated
        redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
        ocrDBK_t DBK_in = PTR_redObjects->DBK_in;
        ocrDBK_t DBK_out = PTR_redObjects->DBK_out;

        r = (QUIESCENCE_RED_HANDLE_LB+ilevel+1); //reserved
        PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
        ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;

        //start next level
        refineLevelsPRM->ilevel = ilevel;
        refineLevelsPRM->iter = iter; //reset to zero

        ocrGuid_t refine1AllLevelsEDT;
        ocrEdtCreate( &refine1AllLevelsEDT, PTR_rankTemplateH->TML_refine1AllLevels,
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL ); //FNC_refine1AllLevels
        _idep = 0;
        ocrAddDependence( DBK_rankH, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_out, refine1AllLevelsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( redDownOEVT, refine1AllLevelsEDT, _idep++, DB_MODE_NULL ); //Quiescence reduction input here?

    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_refine1Level ) //Make sure irefine is set to '0' the first time.
{
    refineLevelsPRM_t* refineLevelsPRM = (refineLevelsPRM_t*) paramv;
    int irefine = refineLevelsPRM->irefine;
    int ilevel = refineLevelsPRM->ilevel;
    int phase = refineLevelsPRM->phase;
    int iter = refineLevelsPRM->iter;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_lchange = depv[_idep++].guid;
    ocrDBK_t DBK_gchange = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* PTR_lchange = depv[_idep++].ptr;
    int* PTR_gchange = depv[_idep++].ptr;

    block *bp = &PTR_rankH->blockH;

    int gchange = PTR_gchange[0];

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    PTR_lchange[0] = 0;
    PTR_gchange[0] = 0;
    ocrDbRelease( DBK_gchange );

    int lchange = 0;

    if( gchange == 0 && iter != 0 ) {
        //Quiescence has reached

        DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ilevel_iter %d lchange %d phase %d iter %d ts %d Quiescence\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, ilevel, lchange, phase, iter, PTR_rankH->ts ));

        if( ilevel != 0 ) {
            int r = (QUIESCENCE_RED_HANDLE_LB+ilevel); //reserved
            redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

            ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

            PTR_lchange[0] = lchange;
            ocrDbRelease( DBK_lchange );

            ocrEventSatisfy( redUpIEVT, DBK_lchange ); //All blocks provide partial sums

            reducePRM_t reducePRM = {irefine, PTR_rankH->ts, phase, r};
            ocrGuid_t reducelchangeEDT;

            ocrEdtCreate( &reducelchangeEDT, PTR_rankTemplateH->TML_reduceAllUp, //FNC_reduceAllUp
                          EDT_PARAM_DEF, (u64*)&reducePRM, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

            _idep = 0;
            ocrAddDependence( DBK_rankH, reducelchangeEDT, _idep++, DB_MODE_RO );
            ocrAddDependence( redUpIEVT, reducelchangeEDT, _idep++, DB_MODE_RO );
        }

        return NULL_GUID;
    }
    else {

        if( bp->level == ilevel ) {
            if( phase == 10) lchange += refine_level1(PTR_rankH, ilevel);
            else if( phase == 20) lchange += refine_level2(PTR_rankH, ilevel);
            //bp->refine, nei_refine[] and sib_refine[] may have been updated
        }

        DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ilevel_iter %d lchange %d phase %d iter %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, ilevel, lchange, phase, iter, PTR_rankH->ts ));


        int r = REFINEINTENT_RED_HANDLE_LB+(ilevel+iter)%(MAX_REFINE_LEVELS); //current level being iterated
        redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

        ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
        ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

        PTR_lchange[0] = lchange;
        ocrDbRelease( DBK_lchange );

        ocrEventSatisfy( redUpIEVT, DBK_lchange ); //All blocks provide partial sums

        reducePRM_t reducePRM = {irefine, PTR_rankH->ts, phase, r};
        ocrGuid_t reducelchangeEDT;

        ocrEdtCreate( &reducelchangeEDT, PTR_rankTemplateH->TML_reduceAllUp, //FNC_reduceAllUp
                      EDT_PARAM_DEF, (u64*) &reducePRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, reducelchangeEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( redUpIEVT, reducelchangeEDT, _idep++, DB_MODE_RO );

        ocrGuid_t scatterRefineEDT, scatterRefineOEVT, scatterRefineOEVTS;

        ocrEdtCreate( &scatterRefineEDT, PTR_rankTemplateH->TML_scatterRefine, //FNC_scatterRefine
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &scatterRefineOEVT );
        createEventHelper(&scatterRefineOEVTS, 1);
        ocrAddDependence( scatterRefineOEVT, scatterRefineOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, scatterRefineEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( redDownOEVT, scatterRefineEDT, _idep++, DB_MODE_RO );

        refineLevelsPRM->iter = iter+1;

        r = REFINEINTENT_RED_HANDLE_LB+(ilevel+iter+1)%(MAX_REFINE_LEVELS); //current level being iterated
        PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
        ocrDBK_t DBK_in = PTR_redObjects->DBK_in;
        //ocrDBK_t DBK_out = PTR_redObjects->DBK_out;

        //ALL blocks (regardless of their level) participate in reduction
        ocrGuid_t refine1LevelEDT, refine1LevelOEVT, refine1LevelOEVTS;

        ocrEdtCreate( &refine1LevelEDT, PTR_rankTemplateH->TML_refine1Level, //FNC_refine1Level
                      EDT_PARAM_DEF, (u64*) refineLevelsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &refine1LevelOEVT );
        createEventHelper(&refine1LevelOEVTS, 1);
        ocrAddDependence( refine1LevelOEVT, refine1LevelOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, refine1LevelEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, refine1LevelEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_gchange, refine1LevelEDT, _idep++, DB_MODE_RW ); //reduction result
        ocrAddDependence( scatterRefineOEVTS, refine1LevelEDT, _idep++, DB_MODE_NULL );

        return NULL_GUID;

    }
}

_OCR_TASK_FNC_( FNC_scatterRefine )
{
    scatterRefinePRM_t* scatterRefinePRM = (scatterRefinePRM_t*) paramv;
    int irefine = scatterRefinePRM->irefine;
    int ilevel = scatterRefinePRM->ilevel;
    int phase = scatterRefinePRM->phase;
    int iter = scatterRefinePRM->iter;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_gchange = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* PTR_gchange = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    int gchange = PTR_gchange[0];

    DEBUG_PRINTF(( "%s ilevel_iter %d ilevel %d id_l %d irefine %d gchange %d phase %d iter %d ts %d\n", __func__, ilevel, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, gchange, phase, iter, PTR_rankH->ts ));

    if( gchange )
    {
        int iAxis = 0;
        int flag = -1;
        commRefnNbrsPRM_t commRefnNbrsReversePRM = {irefine, iAxis, flag};
        ocrGuid_t commRefnNbrsReverseEDT, commRefnNbrsReverseOEVT, commRefnNbrsReverseOEVTS;

        //refineIntentionNbrsHaloExchangeReverse();
        ocrEdtCreate( &commRefnNbrsReverseEDT, PTR_rankTemplateH->TML_commRefnNbrs, //commRefnNbrsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnNbrsReversePRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &commRefnNbrsReverseOEVT );
        createEventHelper(&commRefnNbrsReverseOEVTS, 1);
        ocrAddDependence( commRefnNbrsReverseOEVT, commRefnNbrsReverseOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnNbrsReverseEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, commRefnNbrsReverseEDT, _idep++, DB_MODE_RO );

        //refineIntentionSibsReverseHaloExchange();
        flag = -1;
        commRefnSibsPRM_t commRefnSibsReversePRM = {irefine, flag, 0};
        ocrGuid_t commRefnSibsReverseEDT, commRefnSibsReverseOEVT, commRefnSibsReverseOEVTS;

        ocrEdtCreate( &commRefnSibsReverseEDT, PTR_rankTemplateH->TML_commRefnSibs, //commRefnSibsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnSibsReversePRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &commRefnSibsReverseOEVT );
        createEventHelper(&commRefnSibsReverseOEVTS, 1);
        ocrAddDependence( commRefnSibsReverseOEVT, commRefnSibsReverseOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnSibsReverseEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commRefnNbrsReverseOEVTS, commRefnSibsReverseEDT, _idep++, DB_MODE_RO );

        iAxis = 0;
        flag = 1;
        commRefnNbrsPRM_t commRefnNbrsPRM = {irefine, iAxis, flag};
        ocrGuid_t commRefnNbrsEDT, commRefnNbrsOEVT, commRefnNbrsOEVTS;

        //refineIntentionNbrsHaloExchange();
        ocrEdtCreate( &commRefnNbrsEDT, PTR_rankTemplateH->TML_commRefnNbrs, //commRefnNbrsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnNbrsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &commRefnNbrsOEVT );
        createEventHelper(&commRefnNbrsOEVTS, 1);
        ocrAddDependence( commRefnNbrsOEVT, commRefnNbrsOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnNbrsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commRefnSibsReverseOEVTS, commRefnNbrsEDT, _idep++, DB_MODE_RO );

        //refineIntentionSibsHaloExchange();
        flag = 1;
        commRefnSibsPRM_t commRefnSibsPRM = {irefine, flag, 0};
        ocrGuid_t commRefnSibsEDT;

        ocrEdtCreate( &commRefnSibsEDT, PTR_rankTemplateH->TML_commRefnSibs, //commRefnSibsEdt
                      EDT_PARAM_DEF, (u64*) &commRefnSibsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, commRefnSibsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commRefnNbrsOEVTS, commRefnSibsEDT, _idep++, DB_MODE_RO );

    }

    return NULL_GUID;
}

//Make sure nei_level and sib_level arrays are updated
//Make sure the nei_refine and sib_refine arrays are updated
//prior to calling into this function
int refine_level1(rankH_t* PTR_rankH, int ilevel)
{
    block *bp = &PTR_rankH->blockH;

    int lchange = 0;

    lchange += refine_level_sibsAndnbrs1( bp, ilevel );

    return lchange;
}

int refine_level2(rankH_t* PTR_rankH, int ilevel)
{
    block *bp = &PTR_rankH->blockH;

    int lchange = 0;

    lchange += refine_level_sibsAndnbrs2( bp, ilevel );

    return lchange;
}

int refine_level_sibsAndnbrs1(block* bp, int level)
{
    int i, b;
    DEBUG_PRINTF(( "%s\n", __func__));

    int lchange = 0;
    //Block is marked for refinement
    if (bp->refine == 1) {
       //Make sure the siblings can't coarsen
        for (b = 0; b < 8; b++) {
            if(bp->sib_refine[b] == -1) {
               bp->sib_refine[b] = 0;
               DEBUG_PRINTF(( "HERE 11 \n"));
               lchange++;
            }
        }

        for (i = 0; i < 6; i++) {
            /* neighbors in this level can not unrefine */
            if (bp->nei_level[i] == level && bp->nei_refine[i] == -1) {
                bp->nei_refine[i] = 0;
               DEBUG_PRINTF(( "HERE 12 \n"));
                lchange++;
            }
            /* neighbors in level below must refine */
            else if (bp->nei_level[i] == level-1 && bp->nei_refine[i] != 1) {
                bp->nei_refine[i] = 1;
               DEBUG_PRINTF(( "HERE 13 \n"));
                lchange++;
            }
            else if (bp->nei_level[i] == level+1) {
                 //A finer neighbor can be in any state
            }
        }
    }
    else if (bp->refine == -1) {
    //If the block has a 'refined' neighbor, then it can't coarsen; Mark it's siblings as well
        for (i = 0; i < 6; i++) {
            if (bp->nei_level[i] == level+1) {
               bp->refine = 0;
               lchange++;
               DEBUG_PRINTF(( "HERE 21 \n"));

                for (b = 0; b < 8; b++) {
                    if(bp->sib_refine[b] == -1) {
                       bp->sib_refine[b] = 0;
                       lchange++;
               DEBUG_PRINTF(( "HERE 22 \n"));
                    }
                }
            }
        }

        //If any siblings are marked for refinement,then I can't coarsen
        #if 1
        for (i = 0; i < 8; i++) {
            if( bp->sib_level[i] == bp->level && bp->level != 0 ) {
                if( bp->sib_refine[i] > -1 ) {
                    bp->refine = 0;
                    DEBUG_PRINTF(( "HERE 23 \n"));
                    lchange++;

                //Coarsening works without this
                    for (b = 0; b < 8; b++) {
                        if(bp->sib_refine[b] == -1) {
                           bp->sib_refine[b] = 0;
                           lchange++;
                        }
                    }
                }
            }
        }
        #endif
    }

    return lchange;
}

int refine_level_sibsAndnbrs2(block* bp, int level)
{
    int i, b;

    int lchange = 0;

    /* Check for blocks at this level that will remain at this level
       their coarser neighbors can't coarsen */
    if (bp->refine == 0)
    {
        for (i = 0; i < 6; i++) {
            if (bp->nei_level[i] == level-1) {
                if (bp->nei_refine[i] == -1) {
                   bp->nei_refine[i] = 0;
                   lchange++;
                }
            }
            //else if (bp->nei_level[i] == level) {
            //    //No restrictions here
            //}
            //else if (bp->nei_level[i] == level+1) {
            //    //no restrictions here TODO
            //    if (bp->nei_refine[i] == 1) {
            //       bp->refine = 1;
            //       lchange++;
            //    }
            //}
        }
    }

    if (bp->refine == 0) //Make sure the siblings at '0'
    {
        for (b = 0; b < 8; b++) {
            if(bp->sib_refine[b] == -1) {
               bp->sib_refine[b] = 0;
               lchange++;
            }
        }
    }

    return lchange;
}

_OCR_TASK_FNC_( FNC_refineLoop ) //Make sure irefine is set to '0' the first time.
{
    refineLoopPRM_t* refineLoopPRM = (refineLoopPRM_t*) paramv;

    int irefine = refineLoopPRM->irefine;
    int ts = refineLoopPRM->ts;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* PTR_in = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int num_refine_step = (PTR_rankH->ts!=0) ? PTR_cmd->block_change : PTR_cmd->num_refine;

    if( bp->number >= 0  && irefine < num_refine_step ) {

        int ilevel = PTR_rankH->ilevel;
        DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, irefine, PTR_rankH->ts ));

        int i;
        for( i = 0; i <= PTR_cmd->num_refine; i++ )
            PTR_in[i] = 0; //Default value

        ocrDbRelease(DBK_in);

        // Do one refine
        ocrGuid_t refineEDT, refineOEVT, refineOEVTS;

        ocrEdtCreate( &refineEDT, PTR_rankTemplateH->TML_refine, //FNC_refine
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &refineOEVT );
        createEventHelper(&refineOEVTS, 1);
        ocrAddDependence( refineOEVT, refineOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, refineEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in, refineEDT, _idep++, DB_MODE_RW );

        irefine += 1; //increment

    }

    return NULL_GUID;
}

// Reset the neighbor lists on blocks so that matching them against objects
// can set those which can be refined.
void reset_all( rankH_t* PTR_rankH )
{
    int n, c, in;
    block *bp = &PTR_rankH->blockH;

    bp->refine = -1;
    for (c = 0; c < 6; c++) {
        bp->nei_refine[c] = 0;
        if (bp->nei_level[c] >= 0) bp->nei_refine[c] = -1;
    }

    for (c = 0; c < 8; c++) {
        bp->sib_refine[c] = -1;
        bp->sib_refine_recv[c] = 0;
    }

    //if ANY of my siblings are parents, then mark ALL the siblings as '0'
    ////i.e., if ANY of my siblings are refined/at a higher level, then set myself to '0'


    for (c = 0; c < 8; c++) ;// ;
        //if (bp->sib_level[c] > bp->level) bp->refine = 0; //TODO: Disabling this makes it a correct code

}
