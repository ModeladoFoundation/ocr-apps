#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

#include <ocr.h>
#include <ocr-std.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <extensions/ocr-affinity.h>
#include <extensions/ocr-labeling.h>

#include "block.h"
#include "comm.h"
#include "timer.h"
#include "proto.h"
#include "ocrAppUtils.h"
#include "utils.h"
#include "refineOCR.h"
#include "check_sumOCR.h"


#ifndef MAX_TIME
    #define MAX_TIME 1000
#endif

#ifndef MAX_REF
    #define MAX_REF 1
#endif



ocrGuid_t stencilEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    block_t * PRM_block = depv[0].ptr;
    u64 i;
    for( i = 1; i < 7; i++ )
    {
        dataBundle_t *bundle = depv[i].ptr;
        ocrAssert( !ocrGuidIsNull( depv[i].guid ) );
        if( bundle->num == 4 )
        {
            u64 j;
            for(j = 0; j < 4; j++) PRM_block->neighborIds[((i-1)*4)+j] = bundle->data[j].id;
        }
        else
        {
           PRM_block->neighborIds[(i-1)*4] = bundle->data[0].id;
        }
    }

    ocrGuid_t tempDBK;
    double *arr;
    ocrDbCreate( &tempDBK, (void **)&arr, sizeof( double ) * depc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    arr[0] = PRM_block->data;
    for( i = 1; i < depc; i++ )
    {
        dataBundle_t * tmp = depv[i].ptr;
        arr[i] = tmp->data[0].data;
        ocrDbDestroy( depv[i].guid );
    }

    PRM_block->data = stencil_calcOCR( arr, depc );

    ocrDbDestroy( tempDBK );

    ocrDbRelease( depv[0].guid );
    return depv[0].guid;
}

ocrGuid_t checkRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    ocrPrintf( "%ld in directions %ld had a hanging satisfy.\n", paramv[0], paramv[1] );

    return NULL_GUID;
}

ocrGuid_t haloRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t outDBK;
    dataBundle_t * newData;
    ocrDbCreate( &outDBK, (void **)&newData, sizeof( dataBundle_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    if( depc == 1 )
    {
        data_t *data = depv[0].ptr;
        //ocrPrintf("%ld\n", data->id);
        newData->data[0] = *data;
        newData->num = 1;
        ocrDbDestroy( depv[0].guid );
    }
    if( depc == 4 ) {
        //for( i = 0; i < 4; i++ ) ocrAssert( !ocrGuidIsNull( depv[i].guid ) );
        newData->num = 4;
        u64 i;
        double sum = 0.0;
        for( i = 0; i < 4; i++ )
        {
            data_t *data = depv[i].ptr;
            sum += data->data;
            ocrDbDestroy( depv[i].guid );
        }
        newData->data[0].data = sum;
    }

    ocrDbRelease( outDBK );
    return outDBK;
}


ocrGuid_t haloSnd( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //if 1:1 relationship
        //send data along.

        double *dummy;
        ocrGuid_t dummyDBK;
        ocrDbCreate(&dummyDBK, (void **)&dummy, sizeof(double), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    if( paramc == (sizeof(ocrGuid_t)/sizeof(u64))+1 )
    {
        ocrGuid_t sendChannel;
        memcpy( &sendChannel, paramv, sizeof(ocrGuid_t) );
        ocrAssert( !ocrGuidIsNull(sendChannel) );
        ocrEventSatisfy( sendChannel, dummyDBK );
    }
    else if( paramc == ((sizeof(ocrGuid_t)/sizeof(u64))+1) * 4)
    {
        ocrGuid_t *sendChannels = (ocrGuid_t *)paramv;
        u64 i;
        for( i = 0; i < 4; i++ )
            ocrEventSatisfy( sendChannels[i], dummyDBK );
    }

    return NULL_GUID;
}

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t * PRM_block = depv[0].ptr;
    ocrGuid_t blockGUID, blockTML;

    if( PRM_block->timestep < MAX_TIME )
    {
        if( PRM_block->id == 0 )
        {
            ocrPrintf("%ld\n", PRM_block->timestep);
            fflush(0);
        }
            PRM_block->timestep++;

            ocrGuid_t stencilGUID, stencilOutEVT;

            u64 i, j;

            ocrGuid_t currentAffinity;
            ocrHint_t edtHNT;
            ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
            ocrAffinityGetCurrent( &currentAffinity );
            ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

            ocrEdtCreate( &stencilGUID, PRM_block->stencilTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, &stencilOutEVT );

            ocrEdtTemplateCreate( &blockTML, blockEdt, 0, 1 );
            if( ( (PRM_block->timestep != 0 ) ) && (PRM_block->timestep % PRM_block->refineFreq == 0) && (PRM_block->maxRefLvl != 0))
            {
                //<BALA>
                //*******************************
                //create and set group hint here.
                //TODO
                //*******************************
                ocrEdtCreate( &blockGUID, PRM_block->refineCtrlTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );
            }
            else
            {
                ocrEdtCreate( &blockGUID, blockTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );
            }

            for( i = 0; i < PRM_block->numObjects; i++ )
                for( j = 0; j < 3; j++ ) PRM_block->objects[i].cen[j] += PRM_block->objects[i].move[j];

            for( i = 0; i < 6; i++ )
            {
                ocrGuid_t rcvGUID, rcvOUT;
                if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl ) // I expect only one value from this neighbor channel.
                {
                    ocrEdtCreate( &rcvGUID, PRM_block->haloRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                    ocrAddDependence( rcvOUT, stencilGUID, i+1, DB_MODE_RW );
                    ocrAddDependence( PRM_block->rcv[i*5], rcvGUID, 0, DB_MODE_RW );
                }
                else
                { //I expect 4 values from these neighbors.
                    ocrEdtCreate( &rcvGUID, PRM_block->haloRcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                    ocrAddDependence( rcvOUT, stencilGUID, i+1, DB_MODE_RW );
                    u64 base = i*5, offs;

                    for( offs = 0; offs < 4; offs++ ) ocrAddDependence( PRM_block->rcv[base + (offs + 1)], rcvGUID, offs, DB_MODE_RW );
                }
            }
            for( i = 0; i < 6; i++ )
            {
                data_t * data;
                ocrGuid_t dataDBK;
                if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
                {
                    ocrDbCreate( &dataDBK, (void **)&data, sizeof( data_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    data->data = PRM_block->data;
                    data->id = PRM_block->id;
                    ocrEventSatisfy( PRM_block->snd[i*5], dataDBK );
                }
                else
                {
                    u32 base = i*5;
                    u32 offs;
                    for( offs = 1; offs < 5; offs++ )
                    {
                        ocrDbCreate( &dataDBK, (void **)&data, sizeof( data_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                        data->data = PRM_block->data;
                        data->id = PRM_block->id;
                        ocrEventSatisfy( PRM_block->snd[base+offs], dataDBK );
                    }
                }
            }

            ocrDbRelease( depv[0].guid );
            ocrAddDependence( stencilOutEVT, blockGUID, 0, DB_MODE_RW );
            ocrAddDependence( depv[0].guid, stencilGUID, 0, DB_MODE_RW );
            ocrEdtTemplateDestroy( blockTML );

        return NULL_GUID;
    }

    if( !ocrGuidIsNull( PRM_block->parentEVT ) )
    {
        ocrEventSatisfy( PRM_block->parentEVT, NULL_GUID );
    }
    else
    {
        ocrEdtTemplateDestroy( PRM_block->blockTML);
        ocrEdtTemplateDestroy( PRM_block->refineTML );
        ocrEdtTemplateDestroy( PRM_block->refineRcvTML );
        ocrEdtTemplateDestroy( PRM_block->refineCtrlTML);
        ocrEdtTemplateDestroy( PRM_block->communicateIntentTML );
        ocrEdtTemplateDestroy( PRM_block->willRefineTML );
        ocrEdtTemplateDestroy( PRM_block->updateIntentTML );
        ocrEdtTemplateDestroy( PRM_block->stencilTML );
        ocrEdtTemplateDestroy( PRM_block->haloRcvTML);
    }

    ocrPrintf("BLOCK %ld finished.\n", PRM_block->id);
    fflush(0);

    ocrDbDestroy( depv[0].guid );
    return NULL_GUID;
}

ocrGuid_t myConnect( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    PRM_init3dEdt_t * PRM_initEdt = (PRM_init3dEdt_t *)paramv;
    comm_t * block = depv[0].ptr;
    int * rParams = depv[7].ptr;
    object * objects = NULL;
    if( !ocrGuidIsNull( depv[8].guid ) ) objects = depv[8].ptr;
    else ocrPrintf("Objects are null, for some reason!\n");

    block_t * PRM_block;
    ocrGuid_t blockDBK;

    ocrDbCreate( &blockDBK, (void **)&PRM_block, sizeof(block_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    u64 i;

    for( i = 0; i < 6; i++ )
    {
        connect_t * tmp = depv[i+1].ptr;
        memcpy( &PRM_block->rcv[i * 5], &block->rcv[i], sizeof(ocrGuid_t) );
        memcpy( &PRM_block->rRcv[i * 5], &block->rRcv[i], sizeof(ocrGuid_t) );
        memcpy( &PRM_block->snd[i * 5], &tmp->sEVT, sizeof(ocrGuid_t) );
        memcpy( &PRM_block->rSnd[i * 5], &tmp->rEVT, sizeof(ocrGuid_t) );
        PRM_block->neighborRefineLvls[i] = 0;
        ocrDbDestroy( depv[i+1].guid );
    }

    PRM_block->id = PRM_initEdt->id;
    PRM_block->rootId = PRM_block->id;
    PRM_block->timestep = 0;
    PRM_block->refLvl = 0;
    PRM_block->maxRefLvl = MAX_REF;
    PRM_block->refineFreq = (u32)rParams[17];
    ocrGuid_t blockGUID;

    u64 xDim = PRM_initEdt->edtGridDims[0];
    u64 yDim = PRM_initEdt->edtGridDims[1];

    u64 id = PRM_block->id;

    u64 xPos = id % xDim;
    u64 yPos = (id / xDim) % yDim;
    u64 zPos = id/ (xDim*yDim);

    double blockSize = (double)rParams[4];
    double halfSize = blockSize/2.0f;
    PRM_block->halfSize.x = PRM_block->halfSize.y = PRM_block->halfSize.z = halfSize;

    PRM_block->pos.x = (double)xPos * blockSize + halfSize;
    PRM_block->pos.y = (double)yPos * blockSize + halfSize;
    PRM_block->pos.z = (double)zPos * blockSize + halfSize;


    PRM_block->size.x = PRM_block->size.y = PRM_block->size.z = blockSize;

    PRM_block->numObjects = rParams[24];

    if( objects != NULL )
    {
        memcpy( PRM_block->objects, objects, sizeof(object) * PRM_block->numObjects );
    }


    srand( time(0) );
    PRM_block->data = (double)rand()/(double)RAND_MAX;

    ocrEdtTemplateCreate( &PRM_block->blockTML, blockEdt, 0, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block->refineTML, refineEdt, 0, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block->refineRcvTML, haloRefineRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block->refineCtrlTML, refineControlEdt, 0, 1);
    ocrEdtTemplateCreate( &PRM_block->communicateIntentTML, communicateIntentEdt, 0, 2 );
    ocrEdtTemplateCreate( &PRM_block->willRefineTML, willRefineEdt, 0, 2 );
    ocrEdtTemplateCreate( &PRM_block->updateIntentTML, updateIntentEdt, 0, EDT_PARAM_UNK );
    ocrEdtTemplateCreate( &PRM_block->stencilTML, stencilEdt, 0, 7 );
    ocrEdtTemplateCreate( &PRM_block->haloRcvTML, haloRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );

    PRM_block->parentEVT = NULL_GUID;


    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrAffinityGetCurrent( &currentAffinity );
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    ocrEdtCreate( &blockGUID, PRM_block->blockTML, EDT_PARAM_DEF, NULL, 1, NULL, EDT_PROP_NONE, &edtHNT, NULL );

    ocrDbRelease( depv[0].guid );
    ocrDbRelease( blockDBK );
    ocrAddDependence( blockDBK, blockGUID, 0, DB_MODE_RW );

    ocrDbDestroy( depv[0].guid );

    return NULL_GUID;
}

ocrGuid_t blockInit( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    PRM_init3dEdt_t * PRM_initEdt = (PRM_init3dEdt_t *)paramv;
    range_t * rStruct = (range_t *)depv[1].ptr;
    //if( ocrGuidIsNull( depv[2].guid ) ) ocrPrintf("Objects are null, for some reason!\n");

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
    ocrEdtTemplateCreate( &connectTML, myConnect, paramc, 9 );

    ocrGuid_t channelRcvs[6];

    memcpy( channelRcvs, comms->rcv, sizeof(ocrGuid_t) * 6);

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    for( dir = 0; dir < 6; dir++ ) {
        ocrEventCreateParams( &comms->rcv[dir], OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &comms->rRcv[dir], OCR_EVENT_CHANNEL_T, false, &params );
    }

    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrAffinityGetCurrent( &currentAffinity );

    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    ocrEdtCreate( &connectGUID, connectTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );

    for( i = 0; i < 6; i++ )
    {
        ocrAddDependence( rcvs[i], connectGUID, i+1, DB_MODE_RW );
    }
    for( dir = 0; dir < 6; dir++ )
    {
            ocrGuid_t DBKChannel;
            connect_t *dummy;

            ocrDbCreate( &DBKChannel, (void **)&dummy, sizeof(connect_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
            memcpy( &dummy->sEVT, &comms->rcv[dir], sizeof(ocrGuid_t) );
            memcpy( &dummy->rEVT, &comms->rRcv[dir], sizeof( ocrGuid_t ) );

            ocrDbRelease( DBKChannel );
            ocrEventSatisfy( sends[dir], DBKChannel );
    }

    ocrDbRelease( DBKComm );
    ocrDbRelease( depv[0].guid );
    ocrAddDependence( DBKComm, connectGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[0].guid, connectGUID, 7, DB_MODE_RW );
    ocrAddDependence( rStruct->DBKObjects, connectGUID, 8, DB_MODE_RW );
    ocrEdtTemplateDestroy( connectTML );

    return NULL_GUID;
}

ocrGuid_t realMainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    int i;
    //int i, ierr, object_num;
    int * params;
    //double * objs;
    ocrGuid_t DBKArgs;
    ocrGuid_t DBKmyRange;
    object *objects;

    range_t * rStruct;

    ocrDbCreate( &DBKArgs, (void **)&params, sizeof( int ) * 35, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    ocrDbCreate( &DBKmyRange, (void **)&rStruct, sizeof( range_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

#include "param.h"

    counter_malloc = 0;
    size_malloc = 0.0;

    /* set initial values*/

    void * programArgv = depv[0].ptr;
    u32 argc = ocrGetArgc(programArgv);
    u64 object_num = 0;

    for( i = 1; i < argc; i++ ) {
        char *str;
        str = ocrGetArgv(programArgv, i);

        if (!strcmp(str, "--max_blocks"))
            max_num_blocks = atoi( ocrGetArgv( programArgv ,++i) );
        else if (!strcmp(str, "--target_active"))
            target_active = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--target_max"))
            target_max = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--target_min"))
            target_min = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--num_refine"))
            num_refine = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--block_change"))
            block_change = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--uniform_refine"))
            uniform_refine = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--nx"))
            x_block_size = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--ny"))
            y_block_size = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--nz"))
            z_block_size = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--num_vars"))
            num_vars = atoi( ocrGetArgv( programArgv, ++i ) );
        else if (!strcmp(str, "--comm_vars"))
            comm_vars = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_x"))
            init_block_x = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_y"))
            init_block_y = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--init_z"))
            init_block_z = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--reorder"))
            reorder = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npx"))
            npx = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npy"))
            npy = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--npz"))
            npz = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--inbalance"))
            inbalance = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--lb_opt"))
            lb_opt = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--refine_freq"))
            refine_freq = atoi( ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--report_diffusion"))
            report_diffusion = atoi(ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--error_tol"))
            error_tol = atoi(ocrGetArgv( programArgv, ++i) );
        else if (!strcmp(str, "--num_tsteps"))
            num_tsteps = atoi(ocrGetArgv(programArgv, ++i) );
        else if (!strcmp(str, "--stages_per_ts"))
            stages_per_ts = atoi(ocrGetArgv(programArgv, ++i) );
        else if (!strcmp(str, "--checksum_freq"))
            checksum_freq = atoi(ocrGetArgv(programArgv, ++i) );
        else if (!strcmp(str, "--stencil"))
            stencil = atoi(ocrGetArgv(programArgv, ++i));
        else if (!strcmp(str, "--permute"))
            permute = 1;
        else if (!strcmp(str, "--report_perf"))
            report_perf = atoi(ocrGetArgv(programArgv, ++i));
        else if (!strcmp(str, "--plot_freq"))
            plot_freq = atoi(ocrGetArgv(programArgv, ++i));
        else if (!strcmp(str, "--code"))
            code = atoi(ocrGetArgv(programArgv, ++i));
        else if (!strcmp(str, "--blocking_send"))
            nonblocking = 0;
        else if (!strcmp(str, "--refine_ghost"))
            refine_ghost = 1;
        else if (!strcmp(str, "--num_objects"))
        {
            num_objects = atoi(ocrGetArgv(programArgv, ++i));

            ocrDbCreate( &rStruct->DBKObjects, (void **)&objects, sizeof(object)*num_objects, DB_PROP_NONE, NULL_HINT, NO_ALLOC );

            object_num = 0;
            ocrPrintf("objects %ld\n", num_objects);
        }
        else if( !strcmp(str, "--object"))
        {
            if( object_num >= num_objects )
            {
                ocrPrintf("object number greater than num_objects\n");
                ocrAssert(0);
            }
            objects[object_num].type = atoi(ocrGetArgv(programArgv, ++i));
            objects[object_num].bounce = atoi(ocrGetArgv(programArgv, ++i));
            objects[object_num].cen[0] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].cen[1] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].cen[2] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].move[0] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].move[1] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].move[2] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].size[0] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].size[1] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].size[2] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].inc[0] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].inc[1] = atof(ocrGetArgv(programArgv, ++i));
            objects[object_num].inc[2] = atof(ocrGetArgv(programArgv, ++i));
            object_num++;
        } else if( !strcmp(str, "--help"))
        {
            //print_help_message();
            ocrShutdown();
            return NULL_GUID;
        }

        if(!block_change)
            block_change = num_refine;
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

    u64 numRanks = npx * npy * npz;

    for( i = 0; i < 6; i++ )
        ocrGuidRangeCreate( &rStruct->range[i], numRanks, GUID_USER_EVENT_STICKY );

    ocrGuid_t *redChannels;
    ocrGuid_t DBKChannels;

    ocrDbCreate( &DBKChannels, (void **)&redChannels, sizeof(ocrGuid_t)*numRanks, DB_PROP_NONE, NULL_HINT, NO_ALLOC );


    //if(ocrGuidIsNull(DBKObjects)) ocrPrintf("objects are null, for some reason!\n");

    ocrDbRelease( rStruct->DBKObjects );
    ocrDbRelease( DBKArgs );
    ocrDbRelease( DBKmyRange );
    ocrGuid_t dv[3] = { DBKArgs, DBKmyRange };
    ocrPrintf("%ld, %ld, %ld\n", npx, npy, npz);
    fflush(0);
    u64 gridDims[3] = {npx, npy, npz};

    forkSpmdEdts_Cart3D( blockInit, gridDims, dv );

    return NULL_GUID;
}

ocrGuid_t wrapupEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrPrintf("miniAMR complete!\n");
    fflush(0);

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t realMainTML, realMainGUID, realMainEVT;
    ocrGuid_t wrapupTML, wrapupGUID;

    ocrEdtTemplateCreate( &realMainTML, realMainEdt, 0, 1 );
    ocrEdtTemplateCreate( &wrapupTML, wrapupEdt, 0, 1 );

    ocrEdtCreate( &realMainGUID, realMainTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_HINT, &realMainEVT );
    ocrEdtCreate( &wrapupGUID, wrapupTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );

    ocrEdtTemplateDestroy( realMainTML );
    ocrEdtTemplateDestroy( wrapupTML );

    ocrDbRelease( depv[0].guid );
    ocrAddDependence( realMainEVT, wrapupGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[0].guid, realMainGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}
