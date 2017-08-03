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

#include "ocr.h"

#include <stdio.h>
#include <math.h>
//#include <unistd.h>

#include "block.h"
#include "proto.h"

_OCR_TASK_FNC_( FNC_timestepLoop )
{
    timestepLoopPRM_t* timestepLoopPRM = (timestepLoopPRM_t*) paramv;
    int ts = timestepLoopPRM->ts;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int irefine = 0;
    int r = BLOCKCOUNT_RED_HANDLE_LB + (irefine%2); //reserved for block counts
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
    ocrDBK_t DBK_in = PTR_redObjects->DBK_in;

    r = FINALIZE_RED_HANDLE_LB;
    PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
    ocrDBK_t DBK_in_finalize = PTR_redObjects->DBK_in;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t TML_timestepLoop = PTR_rankTemplateH->TML_timestepLoop;
    ocrTML_t TML_stageLoop = PTR_rankTemplateH->TML_stageLoop;
    ocrTML_t TML_refineLoop = PTR_rankTemplateH->TML_refineLoop;
    ocrTML_t TML_move = PTR_rankTemplateH->TML_move;
    ocrTML_t TML_finalize = PTR_rankTemplateH->TML_finalize;

    PTR_rankH->ts = ts;

    int number = bp->number;
    int num_tsteps = PTR_cmd->num_tsteps;
    int num_refine = PTR_cmd->num_refine;
    int uniform_refine = PTR_cmd->uniform_refine;
    int refine_freq = PTR_cmd->refine_freq;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d num_tsteps %d bp->number %d\n", __func__, ilevel, PTR_rankH->myRank, ts, num_tsteps, number ));

    ocrDbRelease(DBK_rankH);

    if( number >= 0  && ts <= num_tsteps ) {

        // Do one stageLoop
        ocrGuid_t stageLoopEDT, stageLoopOEVT, stageLoopOEVTS;

        int istage = 0;
        stageLoopPRM_t stageLoopPRM = {istage, ts};
        ocrEdtCreate( &stageLoopEDT, TML_stageLoop, //FNC_stageLoop
                      EDT_PARAM_DEF, (u64*)&stageLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &stageLoopOEVT );
        createEventHelper(&stageLoopOEVTS, 1);
        ocrAddDependence( stageLoopOEVT, stageLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, stageLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, stageLoopEDT, _idep++, DB_MODE_RW );

        ocrEVT_t continuationOEVTS;
        createEventHelper(&continuationOEVTS, 1);

        if (num_refine && !uniform_refine) {

           movePRM_t movePRM = {ts};
           ocrGuid_t moveEDT;
           ocrEVT_t moveOEVT, moveOEVTS;
           ocrEdtCreate( &moveEDT, TML_move,
                         EDT_PARAM_DEF, (u64*)&movePRM, EDT_PARAM_DEF, NULL,
                         EDT_PROP_FINISH, &myEdtAffinityHNT, &moveOEVT); //FNC_move
            createEventHelper(&moveOEVTS, 1);
            ocrAddDependence( moveOEVT, moveOEVTS, 0, DB_MODE_NULL );

           _idep = 0;
           ocrAddDependence( DBK_rankH, moveEDT, _idep++, DB_MODE_RW );
           ocrAddDependence( stageLoopOEVTS, moveEDT, _idep++, DB_MODE_NULL );

           if (!(ts%refine_freq)) {

                ocrGuid_t refineLoopEDT;

                refineLoopPRM_t refineLoopPRM = {irefine, ts};
                ocrEdtCreate( &refineLoopEDT, TML_refineLoop,
                              EDT_PARAM_DEF, (u64*)&refineLoopPRM, EDT_PARAM_DEF, NULL,
                              EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_refineLoop
                _idep = 0;
                ocrAddDependence( DBK_rankH, refineLoopEDT, _idep++, DB_MODE_RW );
                ocrAddDependence( DBK_in, refineLoopEDT, _idep++, DB_MODE_RW );
                ocrAddDependence( moveOEVTS, refineLoopEDT, _idep++, DB_MODE_NULL );

                //next timestep task will be set up by the refineLoop
            }
            else
                ocrAddDependence( moveOEVTS, continuationOEVTS, 0, DB_MODE_NULL );
        }
        else
            ocrAddDependence( stageLoopOEVTS, continuationOEVTS, 0, DB_MODE_NULL );

        //start next timestep
        ts += 1;
        timestepLoopPRM_t timestepLoopPRM = {ts};
        ocrGuid_t timestepLoopEDT;
        ocrEdtCreate( &timestepLoopEDT, TML_timestepLoop,
                      EDT_PARAM_DEF, (u64*)&timestepLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_timestepLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( continuationOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL );

    }
    else if( number >= 0  && ts > num_tsteps ) {
        ocrGuid_t finalizeEDT;

        ocrEdtCreate( &finalizeEDT, TML_finalize,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_finalize
        _idep = 0;
        ocrAddDependence( DBK_rankH, finalizeEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_in_finalize, finalizeEDT, _idep++, DB_MODE_RW );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_stageLoop )
{
    stageLoopPRM_t* stageLoopPRM = (stageLoopPRM_t*) paramv;

    int ts = stageLoopPRM->ts;
    int istage = stageLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage ));

    // Do one stage
    ocrGuid_t stageEDT, stageOEVT, stageOEVTS;

    ocrEdtCreate( &stageEDT, PTR_rankTemplateH->TML_stage, //FNC_stage
                  EDT_PARAM_DEF, (u64*)stageLoopPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &stageOEVT );
    createEventHelper(&stageOEVTS, 1);
    ocrAddDependence( stageOEVT, stageOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, stageEDT, _idep++, DB_MODE_RW );

    istage += 1; //increment

    if( istage < PTR_cmd->stages_per_ts )
    {
        //start next stage
        ocrGuid_t stageLoopEDT;
        stageLoopPRM->istage = istage;

        ocrEdtCreate( &stageLoopEDT, PTR_rankTemplateH->TML_stageLoop,
                      EDT_PARAM_DEF, (u64*)stageLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL ); //FNC_stageLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, stageLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( stageOEVTS, stageLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_stage )
{
    stageLoopPRM_t* stageLoopPRM = (stageLoopPRM_t*) paramv;

    int ts = stageLoopPRM->ts;
    int istage = stageLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage ));

    // Do one stage
    ocrGuid_t varsLoopEDT, varsLoopOEVT, varsLoopOEVTS;

    int istart = 0;
    varsLoopPRM_t varsLoopPRM = {istart, istage, ts};
    ocrEdtCreate( &varsLoopEDT, PTR_rankTemplateH->TML_varsLoop, //FNC_varsLoop
                  EDT_PARAM_DEF, (u64*)&varsLoopPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &varsLoopOEVT );
    createEventHelper(&varsLoopOEVTS, 1);
    ocrAddDependence( varsLoopOEVT, varsLoopOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, varsLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, varsLoopEDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_varsLoop )
{
    varsLoopPRM_t* varsLoopPRM = (varsLoopPRM_t*) paramv;

    int istart = varsLoopPRM->istart;
    int ts = varsLoopPRM->ts;
    int istage = varsLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    // Do one vars
    ocrGuid_t varsEDT, varsOEVT, varsOEVTS;

    ocrEdtCreate( &varsEDT, PTR_rankTemplateH->TML_vars, //FNC_vars
                  EDT_PARAM_DEF, (u64*)varsLoopPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &varsOEVT );
    createEventHelper(&varsOEVTS, 1);
    ocrAddDependence( varsOEVT, varsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, varsEDT, _idep++, DB_MODE_RW );

    istart += PTR_cmd->comm_vars; //increment

    if( istart < PTR_cmd->num_vars )
    {
        //start next vars
        ocrGuid_t varsLoopEDT;
        varsLoopPRM->istart = istart;

        ocrEdtCreate( &varsLoopEDT, PTR_rankTemplateH->TML_varsLoop,
                      EDT_PARAM_DEF, (u64*)varsLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL ); //FNC_varsLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, varsLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( varsOEVTS, varsLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_vars )
{
    varsLoopPRM_t* varsLoopPRM = (varsLoopPRM_t*) paramv;

    int istart = varsLoopPRM->istart;
    int ts = varsLoopPRM->ts;
    int istage = varsLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    int iend = MIN( istart + PTR_cmd->comm_vars, PTR_cmd->num_vars-1 );

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    ocrDBK_t DBK_array = bp->DBK_array;
    ocrDBK_t DBK_work = bp->DBK_work;

    // Do one comm
    ocrGuid_t commEDT, commOEVT, commOEVTS;

    int iAxis = 0;
    commPRM_t commPRM = {iAxis, istart, iend, istage, ts};

    ocrEdtCreate( &commEDT, PTR_rankTemplateH->TML_comm, //FNC_comm
                  EDT_PARAM_DEF, (u64*)&commPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &commOEVT );
    createEventHelper(&commOEVTS, 1);
    ocrAddDependence( commOEVT, commOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, commEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, commEDT, _idep++, DB_MODE_RW );

    // Do one calcLoop
    ocrGuid_t calcLoopEDT, calcLoopOEVT, calcLoopOEVTS;

    calcLoopPRM_t calcLoopPRM = {istart, iend, istage, ts};
    ocrEdtCreate( &calcLoopEDT, PTR_rankTemplateH->TML_calcLoop, //FNC_calcLoop
                  EDT_PARAM_DEF, (u64*)&calcLoopPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &calcLoopOEVT );
    createEventHelper(&calcLoopOEVTS, 1);
    ocrAddDependence( calcLoopOEVT, calcLoopOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, calcLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, calcLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_work, calcLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( commOEVTS, calcLoopEDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_calcLoop )
{
    calcLoopPRM_t* calcLoopPRM = (calcLoopPRM_t*) paramv;

    int istart = calcLoopPRM->istart;
    int iend = calcLoopPRM->iend;
    int ts = calcLoopPRM->ts;
    int istage = calcLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_array = depv[_idep++].guid;
    ocrDBK_t DBK_work = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    // Do one calc
    ocrGuid_t calcEDT, calcOEVT, calcOEVTS;

    calcPRM_t calcPRM = {istart, istage, ts};
    ocrEdtCreate( &calcEDT, PTR_rankTemplateH->TML_calc, //FNC_calc
                  EDT_PARAM_DEF, (u64*)&calcPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &calcOEVT );
    createEventHelper(&calcOEVTS, 1);
    ocrAddDependence( calcOEVT, calcOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, calcEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, calcEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_work, calcEDT, _idep++, DB_MODE_RW );

    ocrEVT_t continuationOEVTS;
    createEventHelper(&continuationOEVTS, 1);

    //if (PTR_cmd->checksum_freq && !(istage%PTR_cmd->checksum_freq) { //TODO: This was the baseline code
    if (PTR_cmd->checksum_freq && !(ts%PTR_cmd->checksum_freq) && !istage ) { //changed from istage% to ts%

        int r = CHECKSUM_RED_HANDLE_LB + (istart)%2;
        redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
        ocrDBK_t DBK_gridSum_in = PTR_redObjects->DBK_in;

        ocrGuid_t checkSumEDT, checkSumOEVT, checkSumOEVTS;

        checkSumPRM_t checkSumPRM = {istart, istage, ts};
        ocrEdtCreate( &checkSumEDT, PTR_rankTemplateH->TML_checkSum, //FNC_checkSum
                      EDT_PARAM_DEF, (u64*)&checkSumPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &checkSumOEVT );

        createEventHelper(&checkSumOEVTS, 1);
        ocrAddDependence( checkSumOEVT, checkSumOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, checkSumEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_array, checkSumEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_gridSum_in, checkSumEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( calcOEVTS, checkSumEDT, _idep++, DB_MODE_RW );

        ocrAddDependence( checkSumOEVTS, continuationOEVTS, 0, DB_MODE_NULL );
    }
    else
        ocrAddDependence( calcOEVTS, continuationOEVTS, 0, DB_MODE_NULL );


    istart += 1;

    if( istart <= iend )
    {
        //start next vars
        ocrGuid_t calcLoopEDT, calcLoopOEVT, calcLoopOEVTS;

        calcLoopPRM_t calcLoopPRM = {istart, iend, istage, ts};
        ocrEdtCreate( &calcLoopEDT, PTR_rankTemplateH->TML_calcLoop, //FNC_calcLoop
                      EDT_PARAM_DEF, (u64*)&calcLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &calcLoopOEVT );
        createEventHelper(&calcLoopOEVTS, 1);
        ocrAddDependence( calcLoopOEVT, calcLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, calcLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_array, calcLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_work, calcLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( continuationOEVTS, calcLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_checkSumLoop )
{
    checkSumLoopPRM_t* checkSumLoopPRM = (checkSumLoopPRM_t*) paramv;

    int istart = checkSumLoopPRM->istart;
    int iend = checkSumLoopPRM->iend;
    int ts = checkSumLoopPRM->ts;
    int istage = checkSumLoopPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;
    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;
    ocrDBK_t DBK_array = bp->DBK_array;

    int ilevel = PTR_rankH->ilevel;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    int r = CHECKSUM_RED_HANDLE_LB + (istart)%2;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
    ocrDBK_t DBK_gridSum_in = PTR_redObjects->DBK_in;

    ocrGuid_t checkSumEDT, checkSumOEVT, checkSumOEVTS;

    checkSumPRM_t checkSumPRM = {istart, istage, ts};
    ocrEdtCreate( &checkSumEDT, PTR_rankTemplateH->TML_checkSum, //FNC_checkSum
                  EDT_PARAM_DEF, (u64*)&checkSumPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &checkSumOEVT );

    createEventHelper(&checkSumOEVTS, 1);
    ocrAddDependence( checkSumOEVT, checkSumOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, checkSumEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, checkSumEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_gridSum_in, checkSumEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, checkSumEDT, _idep++, DB_MODE_RW );

    istart += 1;

    if( istart <= iend )
    {
        //start next vars
        ocrGuid_t checkSumLoopEDT, checkSumLoopOEVT, checkSumLoopOEVTS;

        checkSumLoopPRM_t checkSumLoopPRM = {istart, iend, istage, ts};
        ocrEdtCreate( &checkSumLoopEDT, PTR_rankTemplateH->TML_checkSumLoop, //FNC_checkSumLoop
                      EDT_PARAM_DEF, (u64*)&checkSumLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, &checkSumLoopOEVT );
        createEventHelper(&checkSumLoopOEVTS, 1);
        ocrAddDependence( checkSumLoopOEVT, checkSumLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, checkSumLoopEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( checkSumOEVTS, checkSumLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_checkSum )
{
    checkSumPRM_t* checkSumPRM = (checkSumPRM_t*) paramv;

    int istart = checkSumPRM->istart;
    int ts = checkSumPRM->ts;
    int istage = checkSumPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_array = depv[_idep++].guid;
    ocrDBK_t DBK_gridSum_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* PTR_array      = depv[_idep++].ptr;
    double* PTR_grid_sum_in = depv[_idep++].ptr; //1 double

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;
    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    bp->array = PTR_array;

    int ilevel = PTR_rankH->ilevel;

    int r = CHECKSUM_RED_HANDLE_LB + (istart)%2;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;

    int var = istart;

    check_sum( DBK_rankH, PTR_rankH, ts, istage, var, DBK_gridSum_in, PTR_grid_sum_in );

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    ocrDbRelease( DBK_rankH );

    //printEDT
    ocrGuid_t printEDT;

    printPRM_t printPRM = {istart, istage, ts};
    ocrEdtCreate( &printEDT, PTR_rankTemplateH->TML_print, //FNC_print
                  EDT_PARAM_DEF, (u64*)&printPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, printEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redDownOEVT, printEDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_print )
{
    checkSumPRM_t* checkSumPRM = (checkSumPRM_t*) paramv;

    int istart = checkSumPRM->istart;
    int ts = checkSumPRM->ts;
    int istage = checkSumPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_gridSum_out = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* PTR_gridSum_out = depv[_idep++].ptr; //1 double

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    double* grid_sum = PTR_rankH->grid_sum;

    int var = istart;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    double sum = PTR_gridSum_out[0];

    //if (PTR_cmd->report_diffusion && !my_pe)
    if( ts != 0 ) {
        if ( PTR_cmd->report_diffusion && !PTR_rankH->seqRank ) {
          PRINTF("ts %d CHECKSUM sum %f old %f diff %f tol 10^(-%f) ilevel %d id_l %d istage %d var %d \n", ts,sum, grid_sum[var], fabs(sum - grid_sum[var]), log(1.0/PTR_rankH->tol)/log(10), ilevel, PTR_rankH->myRank, istage, var );
        }

        if (fabs(sum - grid_sum[var])/grid_sum[var] > PTR_rankH->tol) {
           if (!PTR_rankH->seqRank)
              PRINTF("Time step %d sum %f (old %f) variable %d difference too large\n", ts, sum, grid_sum[var], var);
        }
    }
    else {
        if ( PTR_cmd->report_diffusion && !PTR_rankH->seqRank ){
          if(var==0) PRINTF("INITIAL\n");
          PRINTF("ts %d CHECKSUM sum %f ilevel %d id_l %d istage %d var %d \n", ts, sum, ilevel, PTR_rankH->myRank, istage, var );
          if(var==PTR_cmd->num_vars-1) PRINTF("\n");

       }

    }

    grid_sum[var] = sum;

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_finalize )
{
    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* in = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    int r = FINALIZE_RED_HANDLE_LB;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ts = PTR_rankH->ts;
    ocrTML_t TML_reduceAllUp = PTR_rankTemplateH->TML_reduceAllUp;

    #ifdef PRINTBLOCKINFO
    PRINTF( "%s ilevel %d id_l %d ts %d seqRank %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts, PTR_rankH->seqRank );
    #endif

    PTR_rankH->total_time = timer()-PTR_rankH->tBegin;

    in[0] = PTR_rankH->total_time;

    //cleanup work; destroy datablocks, etc. TODO
    //

    ocrDbRelease( DBK_in );
    ocrEventSatisfy( redUpIEVT, DBK_in ); //All blocks provide partial sums

    ocrDbRelease( DBK_rankH );

    int phase = -11;
    reducePRM_t reducePRM = {-1, ts, phase, r};
    ocrGuid_t reduceAllUpEDT, reduceAllUpOEVT, reduceAllUpOEVTS;

    ocrEdtCreate( &reduceAllUpEDT, TML_reduceAllUp, //FNC_reduceAllUp
                  EDT_PARAM_DEF, (u64*) &reducePRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &reduceAllUpOEVT );
    createEventHelper(&reduceAllUpOEVTS, 1);
    ocrAddDependence( reduceAllUpOEVT, reduceAllUpOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, reduceAllUpEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redUpIEVT, reduceAllUpEDT, _idep++, DB_MODE_RW );

    ocrGuid_t finalizeBarrierEDT, finalizeBarrierOEVT, finalizeBarrierOEVTS;
    ocrGuid_t finalizeBarrierTML;

    _paramc = 0; _depc = 2;
    ocrEdtTemplateCreate( &(finalizeBarrierTML), FNC_finalizeBarrier, _paramc, _depc );

    ocrEdtCreate( &finalizeBarrierEDT, finalizeBarrierTML, //FNC_finalizeBarrier
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, finalizeBarrierEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redDownOEVT, finalizeBarrierEDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_finalizeBarrier )
{
    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* in = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    ocrEVT_t wrapUpEVT = PTR_rankH->globalParamH.ocrParamH.EVT_OUT_spmdJoin_reduction;

    #ifdef PRINTBLOCKINFO
    PRINTF( "%s ilevel %d id_l %d ts %d seqRank %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->ts, PTR_rankH->seqRank );
    #endif

    if( PTR_rankH->seqRank == 0 ) {
        PRINTF( "\nCompute phase elapsed time is %f seconds\n\n", in[0] );
        int j;

        PRINTF( "Active block stats at the end of the simulation\n" );
        for (j = 0; j <= PTR_rankH->cur_max_level; j++)
            PRINTF("level %d #blocks %d\n", j, PTR_rankH->num_blocks[j] );

        PRINTF("\n" );

        ocrAddDependence( NULL_GUID,  wrapUpEVT, 0, DB_MODE_RO ); //wrapUpEdt
    }

    return NULL_GUID;
}
