#define MAY_REFINE  0
#define WILL_REFINE 1
#define WONT_REFINE 2

#define MORE_REFINED -1
#define SAME_REFINED  0
#define LESS_REFINED  1

#define NEIGHBORS_MAY_REFINE rState->neighborDisps[i*4] == MAY_REFINE || rState->neighborDisps[(i*4)+1] == MAY_REFINE || rState->neighborDisps[(i*4)+2] == MAY_REFINE || rState->neighborDisps[(i*4)+3] == MAY_REFINE
#define OBJECT_DRIVEN

#include <stdlib.h>
#include <time.h>
#include <extensions/ocr-affinity.h>
#include "coarsenOCR.h"

typedef struct{
    ocrGuid_t channels[4];
    ocrGuid_t rChannels[4];
} bundle_t;

typedef struct{
    bundle_t    bundles[4];
    bool        setHere[4];
} bundleSet_t;

typedef struct{
    ocrGuid_t ctrl;
} ctrl_t;

typedef struct{
    ocrGuid_t channels[6][4][4];
    ocrGuid_t rChannels[6][4][4];
}subSets_t;

typedef struct{
    s64 disposition;
    u64 numSets[6];
}catalog_t;

typedef struct{
    u64 cCount;
    u64  disposition;
    u64  prevDisposition;
    s64  neighborDisps[24]; //directions in order of: left, right, up, down, front, back.
} refineState_t;

typedef struct{
    ocrGuid_t channels[4];
    u64 numChannels;
}send_t;

typedef struct{
    u64  intent;
}intent_t;

typedef struct{
    u64 intent[4];
}quadIntent_t;

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t haloNewChannelsSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
    ocrEventSatisfy( *sendChannel, depv[0].guid );

    return NULL_GUID;
}

ocrGuid_t haloNewChannelsRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        ocrGuid_t newDBK = NULL_GUID;

        if( depc == 4 )
        {
            u64 i;
            bundleSet_t * tmp;
            for( i = 0; i < 4; i++ )
            {
                if( ocrGuidIsNull( depv[i].guid ) ) continue;
                if( ocrGuidIsNull( newDBK ) )
                {
                    ocrDbCreate( &newDBK, (void **)&tmp, sizeof(bundleSet_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    u64 j;
                    for( j = 0; j < 4; j++ ) tmp->setHere[j] = false;
                }
                memcpy( &tmp->bundles[i], depv[i].ptr, sizeof(bundle_t) );
                tmp->setHere[i] = true;
                ocrDbDestroy( depv[i].guid );
            }
        }
        else if( depc == 1 )
        {
            bundle_t *tmp;
            ocrDbCreate( &newDBK, (void **)&tmp, sizeof(bundle_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
            memcpy( tmp, depv[0].ptr, sizeof(bundle_t) );
            ocrDbDestroy( depv[0].guid );
        }
        else ocrAssert(0); //WE SHOULD ONLY BE SEEING A DEPC OF 1 OR 4

        return newDBK;
}

ocrGuid_t haloRefineRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    ocrGuid_t newDBK;

    if( depc == 1 ){
        intent_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        memcpy( tmp, depv[0].ptr, sizeof(intent_t) );
        ocrDbDestroy(depv[0].guid);
        return newDBK;
    }
    else if( depc == 4 ) {
        u64 i;
        quadIntent_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof(quadIntent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

        for( i = 0; i < 4; i++ ){
            intent_t * inDBK = depv[i].ptr;
            if( ocrGuidIsNull( depv[i].guid ) )tmp->intent[i] = -7;
            else
            {
                tmp->intent[i] = inDBK->intent;
                ocrDbDestroy( depv[i].guid );
            }
        }
        return newDBK;
    }
    else ocrAssert(0);
    return NULL_GUID;
}

ocrGuid_t refineEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t * PRM_block = depv[depc-2].ptr;
    catalog_t *catalog = depv[0].ptr;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat  =   1;
    params.EVENT_CHANNEL.nbDeps =   1;

    ocrGuid_t relaunchGUID;
    ocrGuid_t coarsenTML;

    #ifdef NO_COARSEN
    memcpy( &coarsenTML, &PRM_block->blockTML, sizeof(ocrGuid_t) );
    #else
    ocrEdtTemplateCreate( &coarsenTML, coarsenControlEdt, 0, EDT_PARAM_UNK );
    #endif

    //we have a few cases we need to account for:
    //
    // 1:1
    //  - send refinement message to neighbor.
    //  - set up new channels in each direction. we will have a total of 4 * 6 non-local channels.
    //  -
    u64 i, j;
    ocrGuid_t internalChannels[6][4];
    ocrGuid_t rInternalChannels[6][4];

    //connect all new channels, if needed.
    for(i = 0; i < 6; i++) //posterity check for detecting whether new events were needed.
    {
        if( !ocrGuidIsNull(depv[i+1].guid) )
        {
            if( PRM_block->neighborRefineLvls[i] == PRM_block->refLvl || PRM_block->neighborRefineLvls[i] == PRM_block->refLvl+1 )
            {
                ocrAssert( depv[i+1].ptr != NULL );
                bundle_t *tBundle = depv[i+1].ptr;
                memcpy( &PRM_block->snd[(i*5)+1], &tBundle->channels[0], sizeof( ocrGuid_t ) * 4 );
                memcpy( &PRM_block->rSnd[(i*5)+1], &tBundle->rChannels[0], sizeof( ocrGuid_t ) * 4 );
            }
        }
    }

    /*
     * At this point, the new channels are connected to the parent's PRM_block-> This is exactly what we want, for a couple of reasons:
     *  Reason 1:
     *      we have a way to keep track of the channels once this block joins (coarsens) again, and just in case the neighbors have not coarsened.
     *  Reason 2:
     *      once those neighbors are also coarsened, we can destroy our send/rcvs (based on ownership).
     */

    switch( catalog->disposition ) {

        case WILL_REFINE:
        {
            fflush(0);
            ocrGuid_t coalesceGUID, coalesceTML;
            ocrEdtTemplateCreate( &coalesceTML, coalesceEdt, EDT_PARAM_UNK, EDT_PARAM_UNK );
            ocrEdtCreate( &coalesceGUID, coalesceTML, 0, NULL, 9, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
            ocrEdtTemplateDestroy( coalesceTML );
            for( i = 0; i < 6; i++ )
            {
                for( j = 0; j < 4; j++ )
                {
                    ocrEventCreateParams( &internalChannels[i][j], OCR_EVENT_CHANNEL_T, false, &params );
                    ocrEventCreateParams( &rInternalChannels[i][j], OCR_EVENT_CHANNEL_T, false, &params );
                }
            }

            for( i = 0; i < 8; i++ ) //create new child block.
            {
                block_t * childBlock;
                ocrGuid_t childBlockDBK;
                ocrDbCreate( &childBlockDBK, (void **)&childBlock, sizeof( block_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( childBlock, PRM_block, sizeof(block_t) );
                childBlock->parent = PRM_block->id;
                childBlock->refLvl = PRM_block->refLvl+1;
                childBlock->id = PRM_block->id * 8 * childBlock->refLvl + i;
                childBlock->size.x = PRM_block->size.x/2;
                childBlock->size.y = PRM_block->size.y/2;
                childBlock->size.z = PRM_block->size.z/2;
                childBlock->halfSize.x = PRM_block->halfSize.x/2;
                childBlock->halfSize.y = PRM_block->halfSize.y/2;
                childBlock->halfSize.z = PRM_block->halfSize.z/2;
                childBlock->data = PRM_block->data / 8;
                ocrEventCreate( &childBlock->parentEVT, OCR_EVENT_ONCE_T, 0 );
                ocrAddDependence( childBlock->parentEVT, coalesceGUID, i, DB_MODE_RW );

                //set channels.
                switch(i){
                    case 0:
                        {
                        u64 idx = 0;
                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx], &PRM_block->snd[(idx)+1], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rSnd[idx], &PRM_block->rSnd[(idx)+1], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock->rcv[idx], &PRM_block->rcv[(idx)+1], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rRcv[idx], &PRM_block->rRcv[(idx)+1], sizeof(ocrGuid_t) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        /*---------------------------------------------------------------------------------------*/

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5) + 1], sizeof(ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5) + 1], sizeof(ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5) + 1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5) + 1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof(ocrGuid_t) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][0], sizeof(ocrGuid_t) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][0], sizeof(ocrGuid_t) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx = idx+1;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[(idx)*5], &PRM_block->snd[(idx*5)+1], sizeof( ocrGuid_t ) ); //the channels had to be created in this direction, use the channels sent by this bundle.
                        memcpy( &childBlock->rSnd[(idx)*5], &PRM_block->rSnd[(idx*5)+1], sizeof( ocrGuid_t ) ); //the channels had to be created in this direction, use the channels sent by this bundle.
                        //rcv
                        memcpy( &childBlock->rcv[(idx)*5], &PRM_block->rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[(idx)*5], &PRM_block->rRcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        }
                        break;

                    case 1:
                        {
                        u64 idx = 0;
                        childBlock->pos.x = PRM_block->pos.x + childBlock->size.x;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        }
                        break;

                    case 2:
                        {
                        u64 idx = 0;

                        childBlock->pos.z = PRM_block->pos.z + childBlock->size.z;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][0], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        }
                        break;
                    case 3:
                        {
                        u64 idx = 0;

                        childBlock->pos.x = PRM_block->pos.x + childBlock->size.x;
                        childBlock->pos.z = PRM_block->pos.z + childBlock->pos.z;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        }
                        break;
                    case 4:
                        {
                        u64 idx = 0;

                        childBlock->pos.y = PRM_block->pos.y + childBlock->size.y;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        }
                        break;
                    case 5:
                        {
                        u64 idx = 0;

                        childBlock->pos.y = PRM_block->pos.y + childBlock->size.y;
                        childBlock->pos.x = PRM_block->pos.x + childBlock->size.x;
                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        }
                        break;
                    case 6:
                        {
                        u64 idx = 0;
                        childBlock->pos.y = PRM_block->pos.y + childBlock->size.y;
                        childBlock->pos.z = PRM_block->pos.z + childBlock->size.z;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        }
                        break;
                    case 7:
                        {
                        u64 idx = 0;
                        childBlock->pos.y = PRM_block->pos.y + childBlock->size.y;
                        childBlock->pos.x = PRM_block->pos.x + childBlock->size.x;
                        childBlock->pos.z = PRM_block->pos.z + childBlock->size.z;

                        //set left direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &PRM_block->snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &PRM_block->rSnd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &PRM_block->rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &PRM_block->rRcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = PRM_block->neighborRefineLvls[idx];
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock->snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rSnd[idx*5], &rInternalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock->rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        memcpy( &childBlock->rRcv[idx*5], &rInternalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock->neighborRefineLvls[idx] = childBlock->refLvl;
                        }
                        break;
                }



            u32 x;
            subSets_t * subSets = depv[depc-1].ptr;

            for( x = 0; x < 6; x++ )
            {
                u64 sNum = 42;
                if( childBlock->neighborRefineLvls[x] > childBlock->refLvl )
                {
                    //ocrPrintf("THIS IS OCCURING\n");
                    fflush(0);
                    switch(i)
                    {
                        case 0:
                            switch(x)
                            {
                                case 0:
                                    sNum = 0;
                                    break;
                                case 2:
                                    sNum = 0;
                                    break;
                                case 5:
                                    sNum = 0;
                                    break;
                                default:
                                    sNum = 42;
                                    break;
                            }
                            break;
                        case 1:
                            switch(x)
                            {
                                case 1:
                                    sNum = 0;
                                    break;
                                case 2:
                                    sNum = 1;
                                    break;
                                case 5:
                                    sNum = 1;
                                    break;
                                default:
                                    sNum = 42;
                                    break;
                                }
                            break;
                        case 2:
                            switch(x)
                            {
                                case 0:
                                    sNum = 1;
                                    break;
                                case 2:
                                    sNum = 2;
                                    break;
                                case 4:
                                    sNum = 0;
                                    break;
                                default:
                                    sNum = 42;
                                    break;
                            }
                            break;
                        case 3:
                                switch(x)
                                {
                                    case 1:
                                        sNum = 1;
                                        break;
                                    case 2:
                                        sNum = 3;
                                        break;
                                    case 4:
                                        sNum = 1;
                                        break;
                                    default:
                                        sNum = 42;
                                        break;
                                }
                                break;
                            case 4:
                                switch(x)
                                {
                                    case 0:
                                        sNum = 2;
                                        break;
                                    case 3:
                                        sNum = 0;
                                        break;
                                    case 5:
                                        sNum = 2;
                                        break;
                                    default:
                                        sNum = 42;
                                        break;
                                }

                                break;
                            case 5:
                                switch(x)
                                {
                                    case 1:
                                        sNum = 2;
                                        break;
                                    case 3:
                                        sNum = 1;
                                        break;
                                    case 5:
                                        sNum = 3;
                                        break;
                                    default:
                                        sNum = 42;
                                        break;
                                }
                                break;
                            case 6:
                                switch(x)
                                {
                                    case 0:
                                        sNum = 3;
                                        break;
                                    case 3:
                                        sNum = 2;
                                        break;
                                    case 4:
                                        sNum = 2;
                                        break;
                                    default:
                                        sNum = 42;
                                        break;
                                }
                                break;
                            case 7:
                                switch(x)
                                {
                                    case 1:
                                        sNum = 3;
                                        break;
                                    case 3:
                                        sNum = 3;
                                        break;
                                    case 4:
                                        sNum = 3;
                                        break;
                                    default:
                                        sNum = 42;
                                        break;
                                }
                                break;
                            default:
                                sNum = 42;
                                break;
                        }
                        if( !ocrGuidIsNull( depv[x+1].guid ) && (sNum != 42) )
                        {
                            bundleSet_t * bundleSet = depv[x+1].ptr;
                            if( (bundleSet->setHere[sNum] == true) )
                            {
                                    ocrAssert( !ocrGuidIsNull( subSets->channels[x][sNum][0] ) );
                                    ocrAssert( !ocrGuidIsNull( subSets->rChannels[x][sNum][0] ) );
                                    memcpy( &childBlock->rcv[(x*5) + 1], &subSets->channels[x][sNum][0], sizeof(ocrGuid_t)*4);
                                    memcpy( &childBlock->rRcv[(x*5) + 1], &subSets->rChannels[x][sNum][0], sizeof(ocrGuid_t)*4);

                                    memcpy( &childBlock->snd[(x*5) + 1], &bundleSet->bundles[sNum].channels[0], sizeof(ocrGuid_t)*4);
                                    memcpy( &childBlock->rSnd[(x*5) + 1], &bundleSet->bundles[sNum].rChannels[0], sizeof(ocrGuid_t)*4);
                            }
                            else
                            {
                                childBlock->neighborRefineLvls[x]--;
                            }
                        }
                    }
            }
            //Both for here and the WONT_REFINE case below,
            //end the grouping hint.
            //TODO
            //<\BALA>
            ocrEdtCreate( &relaunchGUID, coarsenTML, EDT_PARAM_DEF, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
            ocrDbRelease( childBlockDBK );
            ocrAddDependence( childBlockDBK, relaunchGUID, 0, DB_MODE_RW );

            }
            ocrDbRelease( depv[depc-2].guid );
            ocrAddDependence( depv[depc-2].guid, coalesceGUID, 8, DB_MODE_RW );
        }
           break;
        case WONT_REFINE:
            {
                ocrEdtCreate( &relaunchGUID, coarsenTML, EDT_PARAM_DEF, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                ocrAddDependence( depv[depc-2].guid, relaunchGUID, 0, DB_MODE_RW );
            }
            break;
        case MAY_REFINE:
            ocrPrintf("block fell to refine while still in the MAY_REFINE state.\n");
            ocrAssert(0);
            break;
        default:
            break;
    }
    ocrDbDestroy( depv[0].guid );
    if( !ocrGuidIsNull( depv[depc-1].guid ) ) ocrDbDestroy( depv[depc-1].guid );
    for( i = 1; i < 7; i++ )
    {
        if( !ocrGuidIsNull( depv[i].guid ) ) ocrDbDestroy( depv[i].guid );
    }

    #ifndef NO_COARSEN
    ocrEdtTemplateDestroy( coarsenTML );
    #endif

    return NULL_GUID;
}

ocrGuid_t establishNewConnections( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    block_t * PRM_block = depv[1].ptr;
    refineState_t *rState = depv[0].ptr;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat =    1;
    params.EVENT_CHANNEL.nbDeps =   1;

    ocrGuid_t channelListDBK;
    ocrGuid_t subSetDBK = NULL_GUID;

    u64 i, j;
    catalog_t *catalog;
    subSets_t *subs;

    ocrDbCreate( &channelListDBK, (void **)&catalog, sizeof(catalog_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &subSetDBK, (void **)&subs, sizeof(subSets_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrAffinityGetCurrent( &currentAffinity );
    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    ocrGuid_t refineGUID, refineTML, rcvTML;
    ocrEdtTemplateCreate( &refineTML, refineEdt, 0, 9 );
    ocrEdtCreate( &refineGUID, refineTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );

    ocrEdtTemplateCreate( &rcvTML, haloNewChannelsRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );


    for( i = 0; i < 6; i++ ) catalog->numSets[i] = 0;

    u64 prms[4];
    prms[0] = PRM_block->id;
    prms[2] = 0;

    if( rState->disposition == WILL_REFINE )
    {
        //create and send new channels where needed, NULL_GUIDs where not needed.
        bundle_t *bundle;
        ocrGuid_t bundleGUID;
        prms[2] = 1;
        prms[3] = PRM_block->timestep;
        #ifdef R_DEBUG
        ocrPrintf("%ldrefineT%ld neighborRefineLvls[ ", PRM_block->id, PRM_block->timestep);
        #endif
        for( i = 0; i < 6; i++ )
        {
            prms[1] = i;
            ocrGuid_t rcvGUID, rcvOUT;
            if( PRM_block->refLvl < PRM_block->neighborRefineLvls[i] ) // I am less refined.
            {
                ocrEdtCreate( &rcvGUID, rcvTML, 4, prms, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, refineGUID, i+1, DB_MODE_RW );
                #ifdef R_DEBUG
                ocrPrintf("%ld is refining because of neighbor %ld\n", PRM_block->id, i);
                #endif
                bool willRef = false;
                for( j = 0; j < 4; j++ )
                {
                    if( rState->neighborDisps[(i*4)+(j)] == WILL_REFINE )
                    {
                        u64 k;
                        ocrDbCreate( &bundleGUID, (void **)&bundle, sizeof(bundle_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                        willRef = true;
                        for( k = 0; k < 4; k++ )
                        {
                            ocrEventCreateParams(  &bundle->channels[k], OCR_EVENT_CHANNEL_T, false, &params );
                            ocrEventCreateParams( &bundle->rChannels[k], OCR_EVENT_CHANNEL_T, false, &params );
                        }
                        memcpy( subs->channels[i][j], bundle->channels, sizeof( ocrGuid_t ) * 4 );
                        memcpy( subs->rChannels[i][j], bundle->rChannels, sizeof( ocrGuid_t ) * 4 );
                        ocrDbRelease( bundleGUID );
                        ocrEventSatisfy( PRM_block->rSnd[(i * 5)+(j + 1)], bundleGUID );

                        ocrAddDependence( PRM_block->rRcv[(i*5)+(j+1)], rcvGUID, j, DB_MODE_RW );
                        catalog->numSets[i]++;
                    }
                    else
                    {
                        ocrAddDependence( NULL_GUID, rcvGUID, j, DB_MODE_RW );
                    }
                }
                if( willRef )
                {
                    #ifdef R_DEBUG
                    ocrPrintf("%ld+ ", PRM_block->neighborRefineLvls[i]);
                    #endif
                    PRM_block->neighborRefineLvls[i]++;
                }
                #ifdef R_DEBUG
                else
                {
                    ocrPrintf("%ld ", PRM_block->neighborRefineLvls[i]);
                }
                #endif
            }
            else if( PRM_block->refLvl == PRM_block->neighborRefineLvls[i] ) //we must exchange channels, here.
            {
                ocrDbCreate( &bundleGUID, (void **)&bundle, sizeof( bundle_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                ocrEdtCreate( &rcvGUID, rcvTML, 4, prms, 1, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                ocrAddDependence( rcvOUT, refineGUID, i+1, DB_MODE_RW );
                for( j = 0; j < 4; j++ )
                {
                    ocrEventCreateParams( &bundle->channels[j], OCR_EVENT_CHANNEL_T, false, &params );
                    ocrEventCreateParams( &bundle->rChannels[j], OCR_EVENT_CHANNEL_T, false, &params );
                }
                //memcpy rcv
                memcpy( &PRM_block->rcv[(i*5)+1], bundle->channels, sizeof( ocrGuid_t )*4 );
                //memcpy rrcv
                memcpy( &PRM_block->rRcv[(i*5)+1], bundle->rChannels, sizeof( ocrGuid_t )*4 );
                ocrDbRelease( bundleGUID );
                ocrEventSatisfy( PRM_block->rSnd[i*5], bundleGUID );

                ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );
                catalog->numSets[i]++;
                if( rState->neighborDisps[i*4] == WILL_REFINE )
                {
                    #ifdef R_DEBUG
                    ocrPrintf("%ld+ ", PRM_block->neighborRefineLvls[i]);
                    #endif
                    PRM_block->neighborRefineLvls[i]++;
                }
                #ifdef R_DEBUG
                else
                {
                    ocrPrintf("%ld ", PRM_block->neighborRefineLvls[i]);
                }
                #endif
            }
            else if( PRM_block->refLvl > PRM_block->neighborRefineLvls[i] )
            {
                if( rState->neighborDisps[i*4] == WILL_REFINE )
                {
                    ocrDbCreate( &bundleGUID, (void **)&bundle, sizeof(bundle_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    ocrEdtCreate( &rcvGUID, rcvTML, 4, prms, 1, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                    ocrAddDependence( rcvOUT, refineGUID, i+1, DB_MODE_RW );
                    for( j = 0; j < 4; j++ )
                    {
                        ocrEventCreateParams( &bundle->channels[j], OCR_EVENT_CHANNEL_T, false, &params );
                        ocrEventCreateParams( &bundle->rChannels[j], OCR_EVENT_CHANNEL_T, false, &params );
                    }
                    memcpy( &PRM_block->rcv[(i*5)+1], bundle->channels, sizeof( ocrGuid_t )*4 );
                    memcpy( &PRM_block->rRcv[(i*5)+1], bundle->rChannels, sizeof( ocrGuid_t )*4 );
                    ocrDbRelease( bundleGUID );
                    ocrEventSatisfy( PRM_block->rSnd[i*5], bundleGUID );

                    ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );
                    catalog->numSets[i]++;
                    #ifdef R_DEBUG
                    ocrPrintf("%ld+ ", PRM_block->neighborRefineLvls[i]);
                    #endif
                    PRM_block->neighborRefineLvls[i]++;
                }
                else
                {
                    ocrAddDependence( NULL_GUID, refineGUID, i+1, DB_MODE_RW );
                    #ifdef R_DEBUG
                    ocrPrintf("%ld ", PRM_block->neighborRefineLvls[i]);
                    #endif
                }
            }
        }
        #ifdef R_DEBUG
        ocrPrintf("] myLvl:%ld\n", PRM_block->refLvl);
        fflush(0);

        ocrPrintf("%ldrefineT%ld neighborIds[ ", PRM_block->id, PRM_block->timestep);
        for( i = 0; i < 6; i++ )
        {
            if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
            {
                ocrPrintf("%ld ", PRM_block->neighborIds[i*4]);
            }
            else
            {
                ocrPrintf("{ ");
                for( j = 0; j < 4; j++ )
                {
                    ocrPrintf("%ld ", PRM_block->neighborIds[(i*4)+j]);
                }
                ocrPrintf("} ");
            }
        }
        ocrPrintf("]\n");
        fflush(0);
        #endif
    }
    else if( rState->disposition == WONT_REFINE )
    {
        bundle_t *bundle;
        ocrGuid_t bundleGUID;
        ocrGuid_t rcvGUID, rcvOUT;
        for( i = 0; i < 6; i++ )
        {
            if( PRM_block->refLvl < PRM_block->neighborRefineLvls[i] )
            {
                for( j = 0; j < 4; j++ )
                {
                    if( rState->neighborDisps[(i*4)+(j)] == WILL_REFINE )
                    {
                        ocrPrintf("This state should not be allowed.\n");
                        ocrAssert(0);
                    }
                }
                ocrAddDependence( NULL_GUID, refineGUID, i+1, DB_MODE_RW );
            }
            else if( PRM_block->refLvl == PRM_block->neighborRefineLvls[i] )
            {
                if( rState->neighborDisps[i*4] == WILL_REFINE )
                {
                    ocrDbCreate( &bundleGUID, (void **)&bundle, sizeof( bundle_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    ocrEdtCreate( &rcvGUID, rcvTML, 4, prms, 1, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                    ocrAddDependence( rcvOUT, refineGUID, i+1, DB_MODE_RW );
                    for( j = 0; j < 4; j++ )
                    {
                        ocrEventCreateParams( &bundle->channels[j], OCR_EVENT_CHANNEL_T, false, &params );
                        ocrEventCreateParams( &bundle->rChannels[j], OCR_EVENT_CHANNEL_T, false, &params );
                    }
                    memcpy( &PRM_block->rcv[(i*5)+1], bundle->channels, sizeof( ocrGuid_t )*4 );
                    memcpy( &PRM_block->rRcv[(i*5)+1], bundle->rChannels, sizeof( ocrGuid_t )*4 );
                    ocrDbRelease( bundleGUID );
                    ocrEventSatisfy( PRM_block->rSnd[i*5], bundleGUID );
                    ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );

                    catalog->numSets[i]++;
                    PRM_block->neighborRefineLvls[i]++;
                }
                else
                {
                    ocrAddDependence( NULL_GUID, refineGUID, i+1, DB_MODE_RW );
                }
            }
            else if( PRM_block->refLvl > PRM_block->neighborRefineLvls[i] )
            {
                if( rState->neighborDisps[i*4] == WILL_REFINE ) PRM_block->neighborRefineLvls[i]++;
                ocrAddDependence( NULL_GUID, refineGUID, i+1, DB_MODE_RW );
            }
        }
    }

    catalog->disposition = rState->disposition;
    ocrAddDependence( channelListDBK, refineGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[1].guid, refineGUID, 7, DB_MODE_RW );
    ocrAddDependence( subSetDBK, refineGUID, 8, DB_MODE_RW );

    ocrDbDestroy( depv[0].guid );
    ocrEdtTemplateDestroy( rcvTML );
    ocrEdtTemplateDestroy( refineTML );

    return NULL_GUID;
}

ocrGuid_t updateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    bool stable = true;

    block_t * PRM_block = depv[depc-1].ptr;
    refineState_t *rState = (refineState_t *)depv[0].ptr;

    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrAffinityGetCurrent( &currentAffinity );
    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    u32 i;

    for( i = 1; i < depc - 1; i++ )
    {
        s32 difference = PRM_block->neighborRefineLvls[i-1] - PRM_block->refLvl;

        if( !ocrGuidIsNull(depv[i].guid) )
        {
            switch( difference )
            {
                case  1: //I am less refined.
                    {
                        u32 j;
                        quadIntent_t *tmp = (quadIntent_t *) depv[i].ptr;
                        u64 offs = (i-1)*4;
                        for( j = 0; j < 4; j++ )
                        {
                            if( rState->neighborDisps[offs+j] != MAY_REFINE )continue;
                            rState->neighborDisps[offs+j] = tmp->intent[j];
                            if( tmp->intent[j] == MAY_REFINE ) stable = false; //if any neighbor may refine, we must stay awake to know what they intend to do.
                        }
                    }
                    break;
                case  0:
                case -1:
                    {
                        intent_t *tmp = (intent_t *)depv[i].ptr;
                        rState->neighborDisps[(i-1) * 4] = tmp->intent;
                        if( tmp->intent == MAY_REFINE ) stable = false; //if any neighbor may refine, we must stay awake to know what they intend to do.
                    }
                    break;
                default:
                    break;
            }
            ocrDbDestroy( depv[i].guid );
        }
    }

    if( stable && rState->disposition != MAY_REFINE )
    {
        ocrGuid_t newConnectionsGUID, newConnectionsTML;
        ocrEdtTemplateCreate( &newConnectionsTML, establishNewConnections, 0, 2 );
        ocrEdtCreate( &newConnectionsGUID, newConnectionsTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );
        ocrEdtTemplateDestroy( newConnectionsTML );
        ocrAddDependence( depv[0].guid, newConnectionsGUID, 0, DB_MODE_RW );
        ocrAddDependence( depv[depc-1].guid, newConnectionsGUID, 1, DB_MODE_RW );
    }
    else
    {
        if( rState->disposition == MAY_REFINE )
        {
            u64 newDecision = WONT_REFINE;


            for( i = 0; i < 6; i++ )
            {
                u64 difference = PRM_block->neighborRefineLvls[i] - PRM_block->refLvl;
                u64 j;
                switch( difference )
                {
                    case 1: // I am less refined.
                        for( j = 0; j < 4; j++ )
                        {
                            if( rState->neighborDisps[(i*4)+j] == WILL_REFINE && newDecision != MAY_REFINE)
                            {
                                newDecision = WILL_REFINE;// when later portions implemented, replace this with WILL_REFINE;
                            }
                            else if( (rState->neighborDisps[(i*4)+j] == MAY_REFINE) )
                            {
                                newDecision = MAY_REFINE;
                            }
                        }
                        break;
                    case 0: //I don't really care about my neighbors that are my size or larger.
                    case -1:
                        break;
                    default:
                        break;
                }
            }

            rState->prevDisposition = rState->disposition;
            rState->disposition = newDecision;
        }
        else
        {
            rState->prevDisposition = rState->disposition;
        }
        ocrGuid_t commIntentGUID;
        ocrEdtCreate( &commIntentGUID, PRM_block->communicateIntentTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );

        rState->cCount++;
        ocrDbRelease( depv[0].guid );
        ocrDbRelease( depv[depc-1].guid );
        ocrAddDependence( depv[0].guid, commIntentGUID, 0, DB_MODE_RW );
        ocrAddDependence( depv[depc-1].guid, commIntentGUID, 1, DB_MODE_RW );
    }

    return NULL_GUID;
}

ocrGuid_t communicateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //stay inside of communicate intent until "stable."
    //This means that if any neighbor has identified itself as a block that "could be" refining, all of its
    //neighbors must await its decision.

    block_t * PRM_block = depv[1].ptr;
    refineState_t *rState = (refineState_t *)depv[0].ptr;

    u64 i;

    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrAffinityGetCurrent( &currentAffinity );
    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    ocrGuid_t updateIntentGUID;
    ocrEdtCreate( &updateIntentGUID, PRM_block->updateIntentTML, EDT_PARAM_DEF, NULL, 8, NULL, EDT_PROP_NONE, &edtHNT, NULL );

    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block->neighborRefineLvls[i] - PRM_block->refLvl;

        switch(difference)
        {
            case  1: //I am less refined.
                {
                    u64 offs = (i*5)+1;
                    u64 j;

                    if( rState->prevDisposition == MAY_REFINE )
                    {
                        for( j = 0; j < 4; j++ )
                        {
                            ocrGuid_t intentDBK;
                            intent_t *tmp;

                            ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                            tmp->intent = rState->disposition;
                            ocrAssert( !ocrGuidIsNull(intentDBK) );
                            ocrDbRelease( intentDBK );
                            ocrEventSatisfy( PRM_block->rSnd[offs + j], intentDBK );
                        }
                    }
                    else if( NEIGHBORS_MAY_REFINE )
                    {
                        for(j = 0; j < 4; j++)
                        {
                            if( rState->neighborDisps[(i*4)+j] == MAY_REFINE )
                            {
                                ocrGuid_t intentDBK;
                                intent_t *tmp;

                                ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                                tmp->intent = rState->disposition;

                                ocrAssert( !ocrGuidIsNull(intentDBK) );
                                ocrDbRelease( intentDBK );
                                ocrEventSatisfy( PRM_block->rSnd[(i*5)+(j+1)], intentDBK );
                            }
                        }
                    }
                }
                break;
            case  0:
            case -1:
                {
                    if( rState->prevDisposition == MAY_REFINE )
                    {
                        ocrGuid_t intentDBK;
                        intent_t *tmp;

                        ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                        tmp->intent = rState->disposition;

                        ocrAssert( !ocrGuidIsNull(intentDBK) );
                        ocrDbRelease( intentDBK );
                        ocrEventSatisfy( PRM_block->rSnd[i*5], intentDBK );
                    }
                    else if( rState->neighborDisps[i*4] == MAY_REFINE )
                    {

                        ocrGuid_t intentDBK;
                        intent_t *tmp;

                        ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                        tmp->intent = rState->disposition;

                        ocrAssert( !ocrGuidIsNull(intentDBK) );
                        ocrDbRelease( intentDBK );
                        ocrEventSatisfy( PRM_block->rSnd[i*5], intentDBK );
                    }
                }
                break;
            default:
                ocrPrintf("uhhh....\n");
                break;
        }
    }
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block->neighborRefineLvls[i] - PRM_block->refLvl;

        ocrGuid_t rcvGUID;
        ocrGuid_t rcvOUT;
        u64 rcvParams[2];

        rcvParams[0] = PRM_block->id;
        rcvParams[1] = i;

        switch( difference ) {
            case 1: //I am less refined.
                //set to receive from at most 4 receive channels (1 Edt, 4 rcv Evts).
                {
                    u64 offs = (i*5)+1;
                    u64 dCnt = 0;
                    u64 j;
                    u64 dIdcs[4];
                    if( rState->prevDisposition == MAY_REFINE )
                    {
                        dCnt = 4;
                        ocrEdtCreate( &rcvGUID, PRM_block->refineRcvTML, 2, rcvParams, dCnt, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                        ocrAddDependence( rcvOUT, updateIntentGUID, i+1, DB_MODE_RW );
                        for( j = 0; j < dCnt; j++ ) ocrAddDependence( PRM_block->rRcv[offs+j], rcvGUID, j, DB_MODE_RW );
                    }
                    else if( NEIGHBORS_MAY_REFINE )
                    {
                        ocrEdtCreate( &rcvGUID, PRM_block->refineRcvTML, 2, rcvParams, 4, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );
                        ocrAddDependence( rcvOUT, updateIntentGUID, i+1, DB_MODE_RW );
                        for( j = 0; j < 4; j++ ) //Catalog the necessary deps.
                        {
                            if( rState->neighborDisps[(i*4)+j] == MAY_REFINE )
                            {
                                ocrAddDependence( PRM_block->rRcv[(i*5)+(j+1)], rcvGUID, j, DB_MODE_RW );
                            }
                            else
                            {
                                ocrAddDependence( NULL_GUID, rcvGUID, j, DB_MODE_RW );
                            }
                        }
                        for( j = 0; j < dCnt; j++ ) ocrAddDependence( PRM_block->rRcv[dIdcs[j]], rcvGUID, j, DB_MODE_RW );
                    }
                    else ocrAddDependence( NULL_GUID, updateIntentGUID, i+1, DB_MODE_RW );

                }
                break;
            case 0: //I am at the same level.
            case -1: //I am more refined.
                { //set to receive from 1 receive channel (1 Edt, 1 receive Evt).

                    if( (rState->neighborDisps[i*4] == MAY_REFINE) || (rState->prevDisposition == MAY_REFINE)  )
                    {

                        ocrEdtCreate( &rcvGUID, PRM_block->refineRcvTML, 2, rcvParams, 1, NULL, EDT_PROP_NONE, &edtHNT, &rcvOUT );

                        ocrAddDependence( rcvOUT, updateIntentGUID, i+1, DB_MODE_RW );
                        ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );
                    }
                    else ocrAddDependence( NULL_GUID, updateIntentGUID, i+1, DB_MODE_RW );
                }
                break;
            default:
                ocrPrintf("EDT %ld has a relation that is %ld. This is not allowed!\n", PRM_block->id, difference );
                ocrShutdown();
                return NULL_GUID;
        }
    }

    ocrDbRelease( depv[0].guid );
    ocrDbRelease( depv[1].guid );
    ocrAddDependence( depv[0].guid, updateIntentGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[1].guid, updateIntentGUID, 7, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t willRefineEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t * PRM_block = depv[1].ptr;

    refineState_t *rState = (refineState_t *)depv[0].ptr;

    srand(time(NULL) * PRM_block->id);
    bool decision = false;
    u64 i;

    #ifdef OBJECT_DRIVEN

    for( i = 0; i < PRM_block->numObjects; i++ )
    {
        if(!decision)
        {
            double xMin = PRM_block->objects[i].cen[0]-(PRM_block->objects[i].size[0]/2.0f), xMax = PRM_block->objects[i].cen[0] + (PRM_block->objects[i].size[0]/2.0f);
            double yMin = PRM_block->objects[i].cen[1]-(PRM_block->objects[i].size[1]/2.0f), yMax = PRM_block->objects[i].cen[1] + (PRM_block->objects[i].size[1]/2.0f);
            double zMin = PRM_block->objects[i].cen[2]-(PRM_block->objects[i].size[2]/2.0f), zMax = PRM_block->objects[i].cen[2] + (PRM_block->objects[i].size[2]/2.0f);

            bool dX, dY, dZ;

            dX = ((xMin >= PRM_block->pos.x - PRM_block->halfSize.x) && (xMin < PRM_block->pos.x + PRM_block->halfSize.x)) ||
                                ((xMax >=PRM_block->pos.x - PRM_block->halfSize.x) && (xMax < PRM_block->pos.x + PRM_block->halfSize.x) );
            dY = ((yMin >= PRM_block->pos.y - PRM_block->halfSize.y) && (yMin < PRM_block->pos.y + PRM_block->halfSize.y)) ||
                                ((yMax >= PRM_block->pos.y - PRM_block->halfSize.y) && (yMax < PRM_block->pos.y + PRM_block->halfSize.y) );
            dZ = ((zMin >= PRM_block->pos.z - PRM_block->halfSize.z) && (zMin < PRM_block->pos.z + PRM_block->halfSize.z)) ||
                                ((zMax >= PRM_block->pos.z - PRM_block->halfSize.z) && (zMax < PRM_block->pos.z + PRM_block->halfSize.z) );

            decision = dX && dY && dZ;
            continue;
        }
        break;
    }
    #else //this is here for the specific reason of testing out uniform refinement up to 5 levels.
    if( PRM_block->id == 10 ) decision = true;
    else if( PRM_block->timestep == 100 && PRM_block->parent == 10 ) decision = true;
    else if( PRM_block->timestep == 200 && PRM_block->rootId == 10 ) decision = true;
    else if( PRM_block->timestep == 300 && PRM_block->rootId == 10 ) decision = true;
    else if( PRM_block->timestep == 400 && PRM_block->rootId == 10 ) decision = true;
    else decision = false;
    #endif

    for( i = 0; i < 24; i++ ) rState->neighborDisps[i] = MAY_REFINE; //set all neighbor states to MAY_REFINE.
    if( decision && (PRM_block->refLvl < PRM_block->maxRefLvl) )
    {
        //ocrPrintf("I WILL REFINE!\n");
        rState->disposition = WILL_REFINE;
    }
    else
    {
        bool mayRefine = false;
        for( i = 0; i < 6; i++ )
        {
            if( PRM_block->refLvl < PRM_block->neighborRefineLvls[i] )
            {
                mayRefine = true;
                break;
            }
        }

        if( mayRefine )
        {
            rState->disposition = MAY_REFINE;
        }
        else
        { //we have neighbors at a lower refinement level than us.
            rState->disposition = WONT_REFINE;
        }
    }
    rState->prevDisposition = MAY_REFINE;
    rState->cCount = 0;

    return depv[0].guid;
}

ocrGuid_t refineControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t * PRM_block = depv[0].ptr;

    if(PRM_block->id==0) ocrPrintf("----------------------------------------------------------------------------------------------------------------\n");

    ocrGuid_t intentGUID, rStateDBK;
    ocrGuid_t willRefineGUID, willRefineEVT;
    refineState_t *myState;

    ocrGuid_t currentAffinity;
    ocrHint_t edtHNT;
    ocrHintInit( &edtHNT, OCR_HINT_EDT_T );
    ocrAffinityGetCurrent( &currentAffinity );
    ocrSetHintValue( &edtHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );

    ocrDbCreate( &rStateDBK, (void **)&myState, sizeof(refineState_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrEdtCreate( &willRefineGUID, PRM_block->willRefineTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, &willRefineEVT ); //we care about the returnEVT of this Edt.
    ocrEdtCreate( &intentGUID, PRM_block->communicateIntentTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &edtHNT, NULL );

    ocrAddDependence( willRefineEVT, intentGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[0].guid, intentGUID, 1, DB_MODE_RW );

    ocrAddDependence( rStateDBK, willRefineGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[0].guid, willRefineGUID, 1, DB_MODE_RW );

    return NULL_GUID;
}
