#include <ocr.h>
#include <ocr-std.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <extensions/ocr-affinity.h>
#include <extensions/ocr-labeling.h>

#include "block.h"
#include "comm.h"
#include "timer.h"
#include "proto.h"
#include "SPMDappUtils.h"
#include "utils.h"
#include "refineOCR.h"

ocrGuid_t stencilEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );

    //PRINTF("StencilEdt!\n");
    /*PRM_stencilEdt_t * PRM_stencilEdt = (PRM_stencilEdt_t *)paramv;

    ocrGuid_t stencilTML, stencilGUID;
    comm_t * comms = (comm_t *)depv[0].ptr;

    if( PRM_stencilEdt->id == 0 ) PRINTF("%d\n", PRM_stencilEdt->timestep);
    if( PRM_stencilEdt->timestep < 100 ) {
        ocrEdtTemplateCreate( &stencilTML, stencilEdt, paramc, depc );
        PRM_stencilEdt->timestep++;

        ocrEdtCreate( &stencilGUID, stencilTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                                                                        EDT_PROP_NONE, NULL_HINT, NULL );

        addDepsAndSatisfy( stencilGUID, comms->rcv, comms->snd );

        ocrDbRelease( depv[0].guid );
        ocrAddDependence( depv[0].guid, stencilGUID, 0, DB_MODE_RW );

    } else {
        if( PRM_stencilEdt->id == 0 ) {
            sleep(10);
            ocrShutdown();
        }

    */
    /*ocrGuid_t blockGUID;
    ocrEdtCreate( &blockGUID, PRM_block.blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, 1, NULL,
                        EDT_PROP_NONE, NULL_HINT, NULL );

    ocrAddDependence( NULL_GUID, blockGUID, 0, DB_MODE_RW );*/
    return NULL_GUID;
}


ocrGuid_t haloRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    if( depc == 1 )
        return NULL_GUID;

    if( depc == 4 ) {
        //combine the 4 values.
        //return the composite
        //PRINTF("all four evts from neighbors have been satisfied!\n");
    }

    return NULL_GUID;
}

ocrGuid_t haloSnd( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //if 1:1 relationship
        //send data along.

    if( paramc == (sizeof(ocrGuid_t)/sizeof(u64))+1 )
    {
        ocrGuid_t sendChannel;
        memcpy( &sendChannel, paramv, sizeof(ocrGuid_t) );
        ocrEventSatisfy( sendChannel, NULL_GUID );
    }
    else
    {
        ocrGuid_t *sendChannels = (ocrGuid_t *)paramv;
        u64 i;
        for( i = 0; i < 4; i++ )
            ocrEventSatisfy( sendChannels[i], NULL_GUID );
    }

    return NULL_GUID;
}

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );
    ocrGuid_t blockGUID, blockTML;

    if( PRM_block.timestep < 10 )
    {
        if( PRM_block.timestep == 4 )
        {
            PRM_block.timestep++;
            ocrEdtCreate( &blockGUID, PRM_block.refineCtrlTML, EDT_PARAM_DEF, (u64 *)&PRM_block,
            EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
        } else {

            PRM_block.timestep++;

            PRINTF("%ld\n", PRM_block.id);

            ocrGuid_t stencilGUID, stencilOutEVT;

            ocrEdtCreate( &stencilGUID, PRM_block.stencilTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                                EDT_PROP_NONE, NULL_HINT, &stencilOutEVT );

            u32 pCount = sizeof( block_t )/sizeof( u64 );
            ocrEdtTemplateCreate( &blockTML, blockEdt, pCount, 1 );
            ocrEdtCreate( &blockGUID, blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                                EDT_PROP_NONE, NULL_HINT, NULL );

            u64 i;

            for( i = 0; i < 6; i++ )
            {
                ocrGuid_t rcvGUID, rcvOUT;
                if( PRM_block.comms.neighborRefineLvls[i] <= PRM_block.refLvl ) // I expect only one value from this neighbor channel.
                {
                    ocrEdtCreate( &rcvGUID, PRM_block.haloRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, stencilGUID, i+1, DB_MODE_RW );
                    ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RW );
                }
                else
                { //I expect 4 values from these neighbors.
                    ocrEdtCreate( &rcvGUID, PRM_block.haloRcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, stencilGUID, i+1, DB_MODE_RW );
                    u64 base = i*5, offs = 0;

                    for( offs = 0; offs < 4; offs++ ) ocrAddDependence( PRM_block.comms.rcv[base + (offs + 1)], rcvGUID, offs, DB_MODE_RW );
                }

            }
            ocrAddDependence( stencilOutEVT, blockGUID, 0, DB_MODE_RW );

            for( i = 0; i < 6; i++ )
            {
                u32 pCount = (sizeof(ocrGuid_t)/sizeof(u64)) + 1;
                ocrGuid_t sndGUID;
                if( PRM_block.comms.neighborRefineLvls[i] <= PRM_block.refLvl )
                {
                    ocrEdtCreate(&sndGUID, PRM_block.haloSndTML, pCount, (u64 *)&PRM_block.comms.snd[i*5], 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
                }
                else
                {
                    u32 base = i*5;
                    ocrEdtCreate(&sndGUID, PRM_block.haloSndTML, pCount*4, (u64 *)&PRM_block.comms.snd[base+1], 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                }
            }

            ocrAddDependence( NULL_GUID, stencilGUID, 0, DB_MODE_RW );
            ocrEdtTemplateDestroy( blockTML );
        }
        return NULL_GUID;
    }

    PRINTF("BLOCK %ld finished.\n", PRM_block.id);

    return NULL_GUID;
}

ocrGuid_t connect( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    PRM_initEdt_t * PRM_initEdt = (PRM_initEdt_t *)paramv;
    comm_t * block = depv[0].ptr;

    block_t PRM_block;

    u64 i;

    for( i = 0; i < 6; i++ ){
        connect_t * tmp = depv[i+1].ptr;
        memcpy( &PRM_block.comms.rcv[i * 5], &block->rcv[i], sizeof(ocrGuid_t) );
        memcpy( &PRM_block.comms.snd[i * 5], &tmp->EVT, sizeof(ocrGuid_t) );
        PRM_block.comms.neighborRefineLvls[i] = 0;
    }

    PRM_block.id = PRM_initEdt->id;
    PRM_block.rootId = PRM_block.id;
    PRM_block.timestep = 0;
    PRM_block.refLvl = 0;
    PRM_block.maxRefLvl = 4;
    ocrGuid_t blockGUID;

    u64 pCount = (sizeof( block_t ) / sizeof( u64 ));
    pCount++;

    ocrEdtTemplateCreate( &PRM_block.blockTML, blockEdt, pCount, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.refineTML, refineEdt, pCount, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.refineSndTML, haloRefineSend, EDT_PARAM_UNK, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.refineRcvTML, haloRefineRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.refineCtrlTML, refineControlEdt, pCount, 0);
    ocrEdtTemplateCreate( &PRM_block.communicateIntentTML, communicateIntentEdt, pCount, 1 );
    ocrEdtTemplateCreate( &PRM_block.willRefineTML, willRefineEdt, pCount, 1 );
    ocrEdtTemplateCreate( &PRM_block.updateIntentTML, updateIntentEdt, pCount, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.haloSndTML, haloSnd, EDT_PARAM_UNK, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block.stencilTML, stencilEdt, pCount, 7 );
    ocrEdtTemplateCreate( &PRM_block.haloRcvTML, haloRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );

    ocrEdtCreate( &blockGUID, PRM_block.blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, 1,
                                NULL, EDT_PROP_NONE, NULL_HINT, NULL );

    ocrDbRelease( depv[0].guid );
    ocrAddDependence( NULL_GUID, blockGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t blockInit( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    PRM_initEdt_t * PRM_initEdt = (PRM_initEdt_t *)paramv;
    range_t * rStruct = (range_t *)depv[1].ptr;
    u64 dir;
    ocrGuid_t sends[6];
    ocrGuid_t rcvs[6];

    comm_t * comms;
    ocrGuid_t DBKComm;
    ocrDbCreate( &DBKComm, (void **)&comms, sizeof( comm_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    u64 id = PRM_initEdt->id;

    u64 xDim = PRM_initEdt->edtGridDims[0];
    u64 yDim = PRM_initEdt->edtGridDims[1];
    u64 zDim = PRM_initEdt->edtGridDims[2];

    u64 xPos = id % xDim;
    u64 yPos = (id / xDim) % yDim;
    u64 zPos = id / (xDim * yDim);

    //end set send labeled GUIDs

    u64 pSize = (xDim * yDim);
    //begin set rcv labeled GUIDs
    for( dir = 0; dir < 6; dir++ ) {
        ocrGuidFromIndex( &sends[dir], rStruct->range[dir], id );
        ocrEventCreate( &sends[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        switch(dir){
            case 0:
                if( (xPos > 0) ){
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir+1], id-1 );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );
                break;
            case 1:
                if( xPos < (xDim - 1) ){
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir-1], id+1 );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );
                break;
            case 2:
                if( (yPos > 0) ){
                    u64 rangeIndex = (id - xDim);
                    ocrGuidFromIndex( &sends[dir], rStruct->range[dir], id );
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir+1], rangeIndex );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                    ocrEventCreate( &sends[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );

                break;
            case 3:
                if( (yPos < yDim - 1) ) {
                    u64 rangeIndex = (id + xDim);
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir-1], rangeIndex );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );

                break;
            case 4:
                if( (zPos > 0) ){
                    u64 rangeIndex = (id - pSize);
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir+1], rangeIndex );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );

                break;
            case 5:
                if( (zPos < zDim - 1) ) {
                    u64 rangeIndex = (id + pSize);
                    ocrGuidFromIndex( &rcvs[dir], rStruct->range[dir-1], rangeIndex );
                    ocrEventCreate( &rcvs[dir], OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
                } else memcpy( &rcvs[dir], &sends[dir], sizeof(ocrGuid_t) );

                break;
            default:
                break;
        }
    }
    //end set rcv labeled GUIDs
    u64 i;

    ocrGuid_t connectGUID, connectTML;
    ocrEdtTemplateCreate( &connectTML, connect, paramc, 7 );

    ocrGuid_t channelRcvs[6];

    memcpy( channelRcvs, comms->rcv, sizeof(ocrGuid_t) * 6);

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 100;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    for( dir = 0; dir < 6; dir++ ) {
        ocrEventCreateParams( &comms->rcv[dir], OCR_EVENT_CHANNEL_T, false, &params );
    }

    ocrEdtCreate( &connectGUID, connectTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
          NULL_HINT, NULL );


    for( i = 0; i < 6; i++ ) {
        ocrAddDependence( rcvs[i], connectGUID, i+1, DB_MODE_RW );
    }


    for( dir = 0; dir < 6; dir++ ) {
            ocrGuid_t DBKChannel;
            connect_t *dummy;

            ocrDbCreate( &DBKChannel, (void **)&dummy, sizeof(connect_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
            memcpy( &dummy->EVT, &comms->rcv[dir], sizeof(ocrGuid_t) );

            ocrDbRelease( DBKChannel );
            ocrEventSatisfy( sends[dir], DBKChannel );
    }

    ocrDbRelease( DBKComm );

    ocrAddDependence( DBKComm, connectGUID, 0, DB_MODE_RW );

    return NULL_GUID;

}

ocrGuid_t mainEdt( u32 paramc, u64 paramv, u32 depc, ocrEdtDep_t depv[] )
{
    int i;
    //int i, ierr, object_num;
    int * params;
    //double * objs;
    ocrGuid_t DBKArgs;
    ocrGuid_t DBKmyRange;


    range_t * rStruct;

    ocrDbCreate( &DBKArgs, (void **)&params, sizeof( int ) * 35, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    ocrDbCreate( &DBKmyRange, (void **)&rStruct, sizeof( range_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

#include "param.h"

    counter_malloc = 0;
    size_malloc = 0.0;

    /* set initial values*/

    void * programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);

    for( i = 1; i < argc; i++ ) {
        char *str;
        str = getArgv(programArgv, i);

        if (!strcmp(str, "--max_blocks"))
            max_num_blocks = atoi( getArgv( programArgv ,++i) );
        else if (!strcmp(str, "--target_active"))
            target_active = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--target_max"))
            target_max = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--target_min"))
            target_min = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--num_refine"))
            num_refine = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--block_change"))
            block_change = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--uniform_refine"))
            uniform_refine = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--nx"))
            x_block_size = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--ny"))
            y_block_size = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--nz"))
            z_block_size = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--num_vars"))
            num_vars = atoi( getArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--comm_vars"))
            comm_vars = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_x"))
            init_block_x = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_y"))
            init_block_y = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_z"))
            init_block_z = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--reorder"))
            reorder = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npx"))
            npx = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npy"))
            npy = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npz"))
            npz = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--inbalance"))
            inbalance = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--lb_opt"))
            lb_opt = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--refine_freq"))
            refine_freq = atoi( getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--report_diffusion"))
            report_diffusion = atoi(getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--error_tol"))
            error_tol = atoi(getArgv( programArgv, ++i) );
        else if (!strcmp(str, "--num_tsteps"))
            num_tsteps = atoi(getArgv(programArgv, ++i) );
        else if (!strcmp(str, "--stages_per_ts"))
            stages_per_ts = atoi(getArgv(programArgv, ++i) );
        else if (!strcmp(str, "--checksum_freq"))
            checksum_freq = atoi(getArgv(programArgv, ++i) );
        else if (!strcmp(str, "--stencil"))
            stencil = atoi(getArgv(programArgv, ++i));
        else if (!strcmp(str, "--permute"))
            permute = 1;
        else if (!strcmp(str, "--report_perf"))
            report_perf = atoi(getArgv(programArgv, ++i));
        else if (!strcmp(str, "--plot_freq"))
            plot_freq = atoi(getArgv(programArgv, ++i));
        else if (!strcmp(str, "--code"))
            code = atoi(getArgv(programArgv, ++i));
        else if (!strcmp(str, "--blocking_send"))
            nonblocking = 0;
        else if (!strcmp(str, "--refine_ghost"))
            refine_ghost = 1;
        else if (!strcmp(str, "--num_objects"))
            num_objects = atoi(getArgv(programArgv, ++i));
    }

      params[ 0] = max_num_blocks;
      params[ 1] = target_active;
      params[ 2] = num_refine;
      params[ 3] = uniform_refine;
      params[ 4] = x_block_size;
      params[ 5] = y_block_size;
      params[ 6] = z_block_size;
      params[ 7] = num_vars;
      params[ 8] = comm_vars;
      params[ 9] = init_block_x;
      params[10] = init_block_y;
      params[11] = init_block_z;
      params[12] = reorder;
      params[13] = npx;
      params[14] = npy;
      params[15] = npz;
      params[16] = inbalance;
      params[17] = refine_freq;
      params[18] = report_diffusion;
      params[19] = error_tol;
      params[20] = num_tsteps;
      params[21] = stencil;
      params[22] = report_perf;
      params[23] = plot_freq;
      params[24] = num_objects;
      params[25] = checksum_freq;
      params[26] = target_max;
      params[27] = target_min;
      params[28] = stages_per_ts;
      params[29] = lb_opt;
      params[30] = block_change;
      params[31] = code;
      params[32] = permute;
      params[33] = nonblocking;
      params[34] = refine_ghost;

    u64 numRanks = x_block_size * y_block_size * z_block_size;

    for( i = 0; i < 6; i++ )
        ocrGuidRangeCreate( &rStruct->range[i], numRanks, GUID_USER_EVENT_STICKY );

    ocrDbRelease( DBKArgs );
    ocrDbRelease( DBKmyRange );
    ocrGuid_t dv[2] = { DBKArgs, DBKmyRange };
    u64 gridDims[3] = {x_block_size, y_block_size, z_block_size};

    forkSpmdEdts_Cart3D( blockInit, gridDims, dv );

    return NULL_GUID;
}
